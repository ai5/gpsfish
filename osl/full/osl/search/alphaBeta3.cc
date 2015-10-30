/* alphaBeta3.cc
 */
#include "osl/search/alphaBeta3.h"
#include "osl/search/searchRecorder.h"
#include "osl/search/bigramKillerMove.h"
#include "osl/search/killerMoveTable.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/search/moveWithComment.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/eval/see.h"
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/promote_.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_classifier/directCheck.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_order/captureEstimation.h"
#include "osl/move_order/captureSort.h"
#include "osl/move_order/cheapPtype.h"
#include "osl/csa.h"
#include "osl/stat/average.h"
#include "osl/stat/histogram.h"
#include "osl/repetitionCounter.h"
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <iomanip>
#include <unordered_map>
const int extended_futility_margin = 256*16, futility_margin = 128*16, table_record_limit = 400;
const int lmr_fullwidth = 4, lmr_reduce_limit = 200;
const bool best_move_extension_enabled = false;
const bool futility_pruning_enabled = true;
const bool extended_futility_pruning_enabled = true;
const bool cut_drop_move_in_frontier_node = true;
const bool lmr_enabled = true, lmr_verify_enabled = true;
const bool immediate_checkmate_enabled = true;
const bool decorate_csa_in_pv = false, show_height_in_pv = false;
/* ------------------------------------------------------------------------- */
namespace osl
{
  namespace search
  {
    inline Ptype promoteIf(Ptype ptype) 
    {
      return canPromote(ptype) ? promote(ptype) : ptype;
    }
    struct CompactRecord
    {
      Move best_move;
      int value, limit;
      enum ValueType { Exact, UpperBound, LowerBound };
      ValueType type;
      CompactRecord() : limit(-1000000)
      {
      }
      template <Player P>
      bool highFail(int height, int threshold) const 
      {
	return height <= limit && EvalTraits<P>::betterThan(value, threshold)
	  && (type == Exact || type == LowerBound);
      }
      template <Player P>
      bool lowFail(int height, int threshold) const 
      {
	return height <= limit && EvalTraits<P>::betterThan(threshold, value)
	  && (type == Exact || type == UpperBound);
      }
    };
    struct CompactHashTable	// todo: open hash
    {
      typedef std::unordered_map<HashKey, CompactRecord, std::hash<HashKey>> table_t;
      table_t table;
      mutable int probe_success, probe_fail;
      CompactHashTable() : probe_success(0), probe_fail(0)
      {
      }
      ~CompactHashTable()
      {
      }
      const CompactRecord probe(const HashKey& key) const
      {
	table_t::const_iterator p = table.find(key);
	if (p != table.end()) {
	  ++probe_success;
	  return p->second;
	}
	++probe_fail;
	return CompactRecord();
      }
      void store(const HashKey& key, const CompactRecord& value)
      {
	table[key] = value;
      }
      void clear()
      {
	table.clear();
	probe_success = probe_fail = 0;
      }
    };
  }
}
/* ------------------------------------------------------------------------- */
// TODO: make shared? object
namespace 
{
  boost::scoped_array<osl::search::AlphaBeta3::SearchInfo> tree;
  osl::search::CompactHashTable table;
  osl::stat::Average mpn, mpn_cut, last_alpha_update;
  osl::stat::Histogram alpha_update_type(1,8);
  osl::search::BigramKillerMove bigram_killers;
  osl::search::KillerMoveTable killer_moves;
  int eval_count;
  int max_node_depth, total_node_count, depth_node_count[osl::search::AlphaBeta3::MaxDepth];
  void init_node_count()
  {
    max_node_depth = total_node_count = 0;
    std::fill(depth_node_count, depth_node_count+sizeof(depth_node_count)/sizeof(int), 0);
  }
  inline void add_node_count(int depth)
  {
    max_node_depth = std::max(max_node_depth, depth);
    ++depth_node_count[depth];
    ++total_node_count;
  }
  osl::Player root_player;
  osl::RepetitionCounter repetition_counter;
}

/* ------------------------------------------------------------------------- */

osl::search::AlphaBeta3::
AlphaBeta3(const NumEffectState& s, checkmate_t& /*checker*/,
	   SimpleHashTable *t, CountRecorder& r)
  : state(s), depth(0), recorder(r), table_common(t)
{
  if (! tree) {
    rating::StandardFeatureSet::instance();    
    tree.reset(new SearchInfo[MaxDepth]);
  }
}

osl::search::AlphaBeta3::
~AlphaBeta3()
{
}

int osl::search::AlphaBeta3::
evalValue() const
{
  ++eval_count;
  if ((eval_count % (1<<18) == 0) || stop_by_alarm)
    if (toSeconds(this->timeAssigned().standard) - this->elapsed() < 0.3 || stop_by_alarm)
      throw misc::NoMoreTime();
  return tree[depth].eval.value();
}

osl::Move osl::search::AlphaBeta3::
computeBestMoveIteratively(int limit, int /*step*/, int initial_limit, 
			   size_t /*node_limit*/, 
			   const TimeAssigned& assign,
			   MoveWithComment */*additional_info*/)
{
  this->setStartTime(clock::now());
  this->setTimeAssign(assign);

  mpn.clear();
  mpn_cut.clear();
  last_alpha_update.clear();
  bigram_killers.clear();
  table.clear();
  eval_count = 0;
  init_node_count();
  
  initial_limit = std::min(initial_limit, limit);

  // todo: iteration
  Move best_move;
  double consumed = 0;

  try {
    for (int i=0; i<=limit; i+=100) {
      double new_consumed = this->elapsed(), diff = new_consumed - consumed;
      consumed = new_consumed;
      if (table_common->verboseLevel() > 1)
	std::cerr << i << " sec " << diff << " " << new_consumed 
		  << " mpn " << mpn.average() << " " << mpn_cut.average() 
		  << " " << last_alpha_update.average() << "\n";
      best_move = searchRoot(i);

      if (hasSchedule()) {
	const double current_time_left = toSeconds(this->timeAssigned().standard)-this->elapsed();
	const double coef = nextIterationCoefficient();
	if (current_time_left < new_consumed * coef) {
	  if (table_common->verboseLevel() > 1)
	    std::cerr << "expected timeover\n";
	  break;
	}
      }
    }
  }
  catch (misc::NoMoreTime&) {
    if (table_common->verboseLevel() > 1)
      std::cerr << "timeover\n";
  }
  catch (NoMoreMemory&) {
    if (table_common->verboseLevel() > 1)
      std::cerr << "memory full\n";
  }
  double new_consumed = this->elapsed(), diff = new_consumed - consumed;
  consumed = new_consumed;
  if (table_common->verboseLevel() > 1) {
    std::cerr << "finish" << " sec " << diff << " " << new_consumed 
	      << " mpn " << mpn.average() << " " << mpn_cut.average() 
	      << " " << last_alpha_update.average() << "\n";
    std::cerr << "table " << table.table.size() << " " << table.probe_success << " " << table.probe_fail
	      << "\n";
    recorder.finishSearch(best_move, consumed, table_common->verboseLevel() > 1);
    // alpha_update_type.show(std::cerr);
    for (int i=0; i<=max_node_depth/4; ++i) {
      for (int j=0; j<4; ++j) {
	const int id = i + (max_node_depth/4)*j;
	fprintf(stderr, "   depth %2d %5.2f%%",
		id, 100.0*depth_node_count[id] / (double)total_node_count);
      }
      fprintf(stderr, "\n");
    }
  }
  return best_move;
}

bool osl::search::AlphaBeta3::
isReasonableMove(Move /*move*/, int /*pawn_sacrifice*/)
{
  return true;
}

void osl::search::AlphaBeta3::
setRootIgnoreMoves(const MoveVector * /*rim*/, bool)
{
}
void osl::search::AlphaBeta3::
setHistory(const MoveStack& /*h*/)
{
}

void osl::search::AlphaBeta3::
showNodeDepth(std::ostream&)
{
}
void osl::search::AlphaBeta3::
clearNodeDepth()
{
}
/* ------------------------------------------------------------------------- */

osl::Move osl::search::AlphaBeta3::
searchRoot(int limit)
{
  depth = 0;
  SearchInfo& root = tree[0];
  root.moved = Move::PASS(alt(state.turn()));
  root.hash_key = HashKey(state);
  root.height = limit;
  root.path = PathEncoding(state.turn(), 0);
  root.eval = eval_t(state);
  root.moves.clear();
  recorder.resetNodeCount();
  root_player = state.turn();
  repetition_counter.clear();
  repetition_counter.push(root.hash_key, state);
#if 1
  RatedMoveVector moves;
  {
    const rating::StandardFeatureSet& features = rating::StandardFeatureSet::instance();
    RatingEnv env;
    env.make(state);
    features.generateRating(state, env, 2000, moves);
    for (const RatedMove& move: moves)
      root.moves.push_back(move.move());
  }
#else
  state.generateLegal(root.moves);
#endif
  
  Move best_move;
  const Player turn = state.turn();
  int best_value = minusInfty(turn);
  root.alpha = best_value + eval::delta(turn);
  root.beta = -minusInfty(turn) - eval::delta(turn);
  root.node_type = PvNode;
  
  CompactRecord record = table.probe(root.hash_key);
  if (record.best_move.isNormal()) {
    MoveVector::iterator p
      =std::find(root.moves.begin(), root.moves.end(), record.best_move);
    if (p != root.moves.end())
      std::swap(*root.moves.begin(), *p);
  }

  for (Move move: root.moves) {
    if (best_move.isNormal())
      root.node_type = AllNode;
    assert(!ShouldPromoteCut::canIgnoreAndNotDrop(move));
    if (best_move.isNormal())
      continue;
    const int value = (turn == BLACK)
      ? makeMoveAndSearch<BLACK>(move, 100)
      : makeMoveAndSearch<WHITE>(move, 100);
    if (eval::betterThan(turn, value, best_value)) {
      root.pv.setPV(move, root, tree[depth+1].pv);
      if (limit && table_common->verboseLevel()) {
	std::cerr << "  " << csa::show(move) << " " << std::setw(6) << value << " " << std::setw(3) << root.pv.size() << "  ";
	for (size_t i=1; i<root.pv.size(); ++i) {
	  std::cerr << csa::show(root.pv[i].move);
	  if (decorate_csa_in_pv) {
	    if (i && root.pv[i-1].move.to() == root.pv[i].move.to()) std::cerr << '!';
	    else if (root.pv[i].move.capturePtype()) std::cerr << 'x' << csa::show(root.pv[i].move.capturePtype());
	    if (root.pv[i].move.isPromotion()) std::cerr << '*';
	    if (root.pv[i].in_check) std::cerr << '#';
	    if (show_height_in_pv) std::cerr << "(" << root.pv[i].height/10 << ")";
	  }
	}
	std::cerr << std::endl;
      }
      best_value = value;
      best_move = move;
      root.alpha = best_value + eval::delta(turn);
      SimpleHashRecord *record = table_common->allocate(root.hash_key, limit);
      if (record)
	record->setLowerBound(turn, limit, MoveLogProb(best_move,100), best_value);
    }
  }
  record.best_move = best_move;
  record.value = best_value;
  record.type = CompactRecord::Exact;
  record.limit = root.height;
  table.store(root.hash_key, record);
  return best_move;
}

template <osl::Player P>
struct osl::search::AlphaBeta3::CallSearch
{
  AlphaBeta3 *search;
  explicit CallSearch(AlphaBeta3 *s) : search(s) {}
  void operator()(Square) const { search->template presearch<P>(); }
};

template <osl::Player P>
struct osl::search::AlphaBeta3::CallQuiesce
{
  AlphaBeta3 *search;
  explicit CallQuiesce(AlphaBeta3 *s) : search(s) {}
  void operator()(Square) const { search->template quiesce<alt(P)>(); }
};

template <osl::Player P>
int osl::search::AlphaBeta3::
makeMoveAndSearch(Move move, int consume)
{
  ++depth;
  SearchInfo &node = tree[depth], &parent = tree[depth-1];
  node.moved = move;
  node.hash_key = tree[depth-1].hash_key.newHashWithMove(move);
  node.path = parent.path;
  node.height = parent.height - consume;
  node.alpha = parent.beta;
  node.beta = parent.alpha;
  node.node_type = (NodeType)-(parent.node_type);
  node.eval = parent.eval;
  node.pv.clear();
  node.extended = 0;

  // 千日手確認
  if (0)
  {
    const Sennichite next_sennichite
      = repetition_counter.isAlmostSennichite(node.hash_key);
    if (node.moved.isNormal() && next_sennichite.isDraw())
      return this->drawValue();
    if (next_sennichite.hasWinner())
      return this->winByFoul(next_sennichite.winner());
  }
  // repetition_counter.push(node.hash_key, state);
  
  CallSearch<P> f(this);
  node.path.pushMove(move);
  state.makeUnmakeMove(Player2Type<P>(), move, f);
  node.path.popMove(move);

  // repetition_counter.pop();
  --depth;

  return tree[depth+1].search_value;
}

inline
bool osl::search::AlphaBeta3::
reductionOk() const
{
  const SearchInfo& node = tree[depth];
  const Move m = node.moved;
  if (m.isCaptureOrPromotion())
    return false;
  if (node.in_check || (depth > 0 && tree[depth-1].in_check))
    return false;
  return true;
}

template <osl::Player P>
void osl::search::AlphaBeta3::
presearch()
{
  SearchInfo& node = tree[depth];
  assert(state.turn() == alt(P));
  const Player turn = alt(P);
  if (state.hasEffectAt(turn, state.kingSquare(alt(turn)))) {
    node.search_value = winByFoul(turn);
    return;
  }
  node.in_check = state.hasEffectAt(alt(turn), state.kingSquare(turn));
  node.eval.update(state, node.moved);

  // heuristic extension
#if 0
  if (depth > 1 && tree[depth-1].in_check && tree[depth-1].moves.size() == 1) {	// one reply
    const int ext = 50;
    node.extended += ext;
    node.height += ext;
  }
#endif
  if (node.in_check) {
    const int ext = (node.alpha != node.beta
		     || (depth > 2 && tree[depth-1].moved.ptype() == KING))
      ? 100 : 100;
    node.extended += ext;
    node.height += ext;		
  } 
  else if (depth > 1 && node.moved.to() == tree[depth-1].moved.to() && ! node.moved.isPass()) {
    const int ext = (node.alpha != node.beta
		     || tree[depth-1].moved.isCapture())
      ? 50 : 25;
    node.extended += ext;
    node.height += ext;
  }

  // null move pruning
  if (node.moved.isPass()) { // need verify?
    const int ext = (node.height >= 500) ? -200 : -100;
    node.height += ext;
    node.extended = ext;
  }

  // null window search
  const int org_alpha = node.alpha, org_height = node.height;
  const NodeType org_node_type = node.node_type;
  const bool pv_in_pvs = node.node_type == CutNode && node.alpha != node.beta;
  int lmr_reduce = 0;
  if (pv_in_pvs)
    node.alpha = node.beta;

  if (node.alpha == node.beta) {
    if (lmr_enabled && ! node.extended && reductionOk()
	&& (!pv_in_pvs || node.height >= lmr_reduce_limit+100)
	&& depth > 0) {
      if (pv_in_pvs)
	lmr_reduce = tree[depth-1].moves_tried / lmr_fullwidth * 50;
      else
	lmr_reduce = tree[depth-1].moves_tried / lmr_fullwidth * 75;
      lmr_reduce = std::min(400, lmr_reduce);
      node.height -= lmr_reduce;
      if (pv_in_pvs && node.height < lmr_reduce_limit)
	node.height = lmr_reduce_limit;
    }
    search<alt(P)>();
    if (EvalTraits<P>::betterThan(node.beta, node.search_value)) // note: beta cut for opponent
      return;
    node.height = org_height;
    node.alpha = org_alpha;
    node.node_type = org_node_type;
    // verification search not in pv
    if (! pv_in_pvs) {
      if (lmr_verify_enabled && lmr_reduce) {
	node.height -= lmr_reduce/2;
	if (lmr_reduce >= 100 || node.height >= 400)
	  search<alt(P)>();
      }
      return;
    }
    node.node_type = PvNode;
  }
  // now node is pv
  assert(node.node_type == PvNode);
  if (node.height >= table_record_limit)
  {
    CompactRecord record = table.probe(node.hash_key);
    // iid if hash-move is not available
    if (! record.best_move.isNormal()) {
      const int height = node.height;
      for (int i=200; i+100<height; i+=200) {
	node.height = i;
	search<alt(P)>(); 
	node.alpha = org_alpha;
	node.node_type = PvNode;
      }
      node.height = height;
    }
  }
  // main search
  const bool best_move_extension_candidate
    = best_move_extension_enabled && root_player == P
    && node.height >= 150 && node.extended < 50;
  const bool skip_main_search 
    = best_move_extension_candidate && pv_in_pvs;
  if (! skip_main_search)
    search<alt(P)>();
  // best move ext --- ?
  if (best_move_extension_candidate
      && EvalTraits<P>::betterThan(node.search_value, node.beta)) // alpha value update for P
  {
    node.node_type = PvNode;
    node.alpha = org_alpha;
    const int ext = 50;
    node.height += ext; node.extended += ext;
    search<alt(P)>();
  }  
}

template <osl::Player P>
void osl::search::AlphaBeta3::
search()
{
  using namespace move_classifier;
  add_node_count(depth);

  SearchInfo& node = tree[depth];
  assert(state.turn() == P);
  recorder.addNodeCount();  

  if (node.height < 0) {
    quiesceRoot<P>();
    return;
  }

  CompactRecord record = node.height >= table_record_limit 
    ? table.probe(node.hash_key)
    : CompactRecord();  
  if (node.alpha == node.beta) {
    if (record.highFail<P>(node.height, node.beta)) {
      node.search_value = record.value;
      return;
    }
    if (record.lowFail<P>(node.height, node.alpha)) {
      node.search_value = record.value;
      return;
    }
  }
  const bool frontier_node = futility_pruning_enabled && node.height < 100;
  const bool extended_frontier_node = (! frontier_node) && extended_futility_pruning_enabled && node.height < 200;
  const bool in_pv = node.alpha != node.beta;
  node.move_type = Initial;
  node.moves_tried = 0;
  const int initial_value = minusInfty(P)+depth*EvalTraits<P>::delta*2;
  int best_value = initial_value, last_alpha_update=0;
  if (EvalTraits<P>::betterThan(best_value, node.alpha)) {
    node.alpha = best_value + EvalTraits<P>::delta;
    if (EvalTraits<P>::betterThan(best_value, node.beta)) {
      node.search_value = best_value;
      return;
    }
  }
  const int initial_alpha = node.alpha;
  if (record.best_move.isNormal()) {
    const Move move = record.best_move;
    int value = makeMoveAndSearch<P>(move, 100);
    if (EvalTraits<P>::betterThan(value, best_value)) {
      best_value = value;
      if (EvalTraits<P>::betterThan(value, node.alpha)) {
	if (in_pv)
	  node.pv.setPV(move, node, tree[depth+1].pv);
	node.alpha = value + EvalTraits<P>::delta;
	last_alpha_update = node.moves_tried+1;
	alpha_update_type.add(node.move_type);
	if (EvalTraits<P>::betterThan(value, node.beta)) {
	  mpn_cut.add(node.moves_tried+1);
	  goto done;
	}
      }
    }    
    node.moves_tried++;
  }
  if (immediate_checkmate_enabled && ! node.in_check && (frontier_node || extended_futility_pruning_enabled) 
      && ImmediateCheckmate::hasCheckmateMove<P>(state)) {
    node.search_value = winByCheckmate(P);
    return;
  }
  for (Move move=nextMove<P>(); !move.isInvalid(); move=nextMove<P>(), node.moves_tried++) {
    if (node.moves_tried == 1)
      node.node_type = AllNode;
    if (move == record.best_move)
      continue;
    if (! node.in_check && node.node_type != PvNode) {
      if (frontier_node && node.move_type > Pass) {
	const int futility = evalValue()
	  + (move.capturePtype() ? eval_t::captureValue(move.capturePtypeO()) : 0)
	  + futility_margin*EvalTraits<P>::delta;
	if (EvalTraits<P>::betterThan(best_value, futility)
	    && (!tree[depth-1].in_check || !PlayerMoveAdaptor<DirectCheck>::isMember(state,move)))
	  continue;
      } 
      else if (extended_frontier_node && node.move_type > Killer) {
	const int futility_base = evalValue()+ extended_futility_margin*EvalTraits<P>::delta;
	if ((move.capturePtype() 
	     && EvalTraits<P>::betterThan(best_value, futility_base+node.eval.captureValue(move.capturePtypeO())))
	    || EvalTraits<P>::betterThan(best_value, futility_base+See::see(state, move)))
	  if (!tree[depth-1].in_check || !PlayerMoveAdaptor<DirectCheck>::isMember(state,move))
	    continue;
      }
    }
    int value = makeMoveAndSearch<P>(move, 100);
    if (EvalTraits<P>::betterThan(value, best_value)) {
      best_value = value;
      record.best_move = move;
      if (EvalTraits<P>::betterThan(value, node.alpha)) {
	if (in_pv)
	  node.pv.setPV(move, node, tree[depth+1].pv);
	node.alpha = value + EvalTraits<P>::delta;
	last_alpha_update = node.moves_tried+1;
	alpha_update_type.add(node.move_type);
	if (EvalTraits<P>::betterThan(value, node.beta)) {
	  mpn_cut.add(node.moves_tried+1);
	  goto done;
	}
      }
    }
  }
  mpn.add(node.moves_tried);
  if (last_alpha_update)
    ::last_alpha_update.add(last_alpha_update);
done:
  if (last_alpha_update && node.move_type > Killer) {
    bigram_killers.setMove(node.moved, record.best_move);
    killer_moves.setMove(depth, record.best_move);
    // history_table.setMove(depth, record.best_move);
  }
  if (node.height >= table_record_limit) {
    record.value = best_value;
    record.limit = node.height;
    if (EvalTraits<P>::betterThan(initial_alpha, best_value))
      record.type = CompactRecord::UpperBound;
    else if (EvalTraits<P>::betterThan(node.beta, best_value))
      record.type = CompactRecord::Exact;
    else
      record.type = CompactRecord::LowerBound;
    table.store(node.hash_key, record);
  }
  node.search_value = best_value;
}

template <osl::Player P>
osl::Move osl::search::AlphaBeta3::nextMove()
{
  SearchInfo& node = tree[depth];
  switch (node.move_type) {
  case Initial:
    node.move_index = 0;
    node.moves.clear();
    if (node.in_check) {
      move_generator::GenerateEscape<P>::
	generate(state,state.kingPiece<P>(),node.moves);
      node.move_type = KingEscape; // fall through
    } // fall through
  case KingEscape:
    if (! node.moves.empty()) {
      if (node.move_index < node.moves.size())
	return node.moves[node.move_index++];
      return Move();
    }
    node.move_type = Pass;	// fall through
    node.move_index = 0;
  case Pass:
    if (node.move_index++ == 0 && node.node_type != PvNode && !node.in_check)
      return Move::PASS(P);
    node.move_type = TakeBack; // fall through
    node.move_index = 0;
    if (node.moved.isNormal()) {
      move_generator::GenerateCapture::generate(P,state, node.moved.to(), node.moves);
      // move_order::MoveSorter::sort(node.moves, move_order::CheapPtype());
    }
  case TakeBack:
    if (node.move_index == 0 && node.moves.size()) 
      return node.moves[node.move_index++];
    node.move_type = Capture;	// fall through
    node.move_index = 0;
    generateCapture<P>(state, node);
  case Capture:
    if (node.move_index < node.moves.size())
      return node.moves[node.move_index++];
    node.move_type = Killer;	// fall through
    node.move_index = 0;
    node.moves.clear();
    bigram_killers.getMove(state, node.moved, node.moves);
    killer_moves.getMove(state, depth, node.moves);
  case Killer:
    if (node.move_index < node.moves.size())
      return node.moves[node.move_index++];
    node.move_type = CaptureAll;	// fall through
    node.move_index = 0;
    generateCaptureAll<P>(state, node);
  case CaptureAll:
    if (node.move_index < node.moves.size())
      return node.moves[node.move_index++];
    node.move_type = All;	// fall through
    node.move_index = 0;
    generateAllMoves<P>(state, tree[depth-1], node);
  case All:
    if (node.move_index < node.moves.size())
      return node.moves[node.move_index++];
  }
  return Move();
}

template <osl::Player P>
void osl::search::AlphaBeta3::
generateAllMoves(const NumEffectState& state, const SearchInfo& parent, SearchInfo& node)
{
  node.moves.clear();
  if (cut_drop_move_in_frontier_node 
      && ! parent.in_check
      && ! node.in_check && node.node_type != PvNode) {
    if ((futility_pruning_enabled && node.height < 100)
	|| (extended_futility_pruning_enabled && node.height < 200
	    && EvalTraits<P>::betterThan(node.alpha, node.eval.value() + extended_futility_margin*EvalTraits<P>::delta))) {
      // generation considering futility pruning
      GenerateAllMoves::generateOnBoard<P>(state, node.moves);
    }
  }
#if 1
#  if 1
  if (node.alpha != node.beta || node.height >= 800) { 
    RatedMoveVector moves;
    const rating::StandardFeatureSet& features = rating::StandardFeatureSet::instance();
    RatingEnv env;
    env.make(state);
    features.generateRating(state, env, 2000, moves);
    for (const RatedMove& move: moves)
      if (move.move().isDrop() || ! seePlusLight<P>(state, move.move()))
	node.moves.push_back(move.move());
    return;
  }
#  endif
  GenerateAllMoves::generate<P>(state, node.moves);

  if (node.alpha != node.beta || node.height > 300)
    std::sort(node.moves.begin(), node.moves.end(), move_order::CaptureEstimation(state));
#endif
}

template <osl::Player P>
void osl::search::AlphaBeta3::
generateCapture(const NumEffectState& state, SearchInfo& node)
{
  node.moves.clear();
  MoveVector all;
  for (size_t i=0; i+1<PieceStand::order.size(); ++i) {	// except for pawn
    const Ptype ptype = PieceStand::order[i];
    all.clear();
    move_action::Store store(all);
    for (int j=Ptype_Table.getIndexMin(ptype); j<Ptype_Table.getIndexLimit(ptype); ++j) {
      const Piece p = state.pieceOf(j);
      if (! p.isOnBoardByOwner<alt(P)>())
	continue;
      move_generator::GenerateCapture::generate(P,state, p.square(), store);
    }
    for (Move move: all) {
      if (See::see(state, move) > 0) {
	node.moves.push_back(move);
      }
    }
    if (! node.moves.empty())
      return;
  }
  // promote
  all.clear();
  move_generator::Promote<P>::generate(state, all);
  for (Move move: all) {
    if (See::see(state, move) > 0) {
      node.moves.push_back(move);
    }
  }
  if (! node.moves.empty())
    return;
  // pawn
  all.clear();
  {
    move_action::Store store(all);
    for (int j=Ptype_Table.getIndexMin(PAWN); j<Ptype_Table.getIndexLimit(PAWN); ++j) {
      const Piece p = state.pieceOf(j);
      if (! p.isOnBoardByOwner<alt(P)>())
	continue;
      move_generator::GenerateCapture::generate(P,state, p.square(), store);
    }
  }
  for (Move move: all) {
    if (See::see(state, move) > 0) {
      node.moves.push_back(move);
    }
  }
}
template <osl::Player P>
inline
bool osl::search::AlphaBeta3::
seePlusLight(const NumEffectState& state, Move m)
{
  assert(P == m.player());
  assert(P == state.turn());
  assert(! m.isDrop());
  if (state.countEffect(P, m.to()) > state.countEffect(P, m.to()))
    return true;
  return eval::Ptype_Eval_Table.value(m.capturePtype()) >= eval::Ptype_Eval_Table.value(m.oldPtype());
}

template <osl::Player P>
void osl::search::AlphaBeta3::
generateCaptureAll(const NumEffectState& state, SearchInfo& node)
{
  node.moves.clear();
  MoveVector all;
  {
    move_action::Store store(all);
    for (size_t i=0; i+1<PieceStand::order.size(); ++i) {
      const Ptype ptype = PieceStand::order[i];
      for (int j=Ptype_Table.getIndexMin(ptype); j<Ptype_Table.getIndexLimit(ptype); ++j) {
	const Piece p = state.pieceOf(j);
	if (! p.isOnBoardByOwner<alt(P)>())
	  continue;
	move_generator::GenerateCapture::generate(P,state, p.square(), store);
      }
    }
    move_generator::Promote<P>::generateMoves(state, store);
    for (int j=PtypeTraits<PAWN>::indexMin; j<PtypeTraits<PAWN>::indexLimit; ++j) {
      const Piece p = state.pieceOf(j);
      if (! p.isOnBoardByOwner<alt(P)>())
	continue;
      move_generator::GenerateCapture::generate(P,state, p.square(), store);
    }
  }
  for (Move move: all)
    if (seePlusLight<P>(state, move))
      node.moves.push_back(move);
  std::sort(node.moves.begin(), node.moves.end(), move_order::CaptureEstimation(state));
}

template <osl::Player P>
void osl::search::AlphaBeta3::
quiesceRoot()
{
  SearchInfo& node = tree[depth];
  assert(! state.hasEffectAt(P, state.kingSquare(alt(P))));
  assert(node.in_check == state.hasEffectAt(alt(P), state.kingSquare(P)));

  node.search_value = evalValue();
  const int static_value = node.search_value;
  int best_value = static_value;
  if (node.in_check) {
    node.moves.clear();
    move_generator::GenerateEscape<P>::
      generate(state,state.kingPiece<P>(),node.moves);
    best_value = threatmatePenalty(P)+depth*EvalTraits<P>::delta*2;
    
    for (Move move: node.moves) {
      int value = makeMoveAndQuiesce<P>(move);
      if (EvalTraits<P>::betterThan(value, best_value)) {
	best_value = value;
	if (EvalTraits<P>::betterThan(value, node.alpha)) {
	  if (node.node_type == PvNode)
	    node.pv.setPV(move, node, tree[depth+1].pv);
	  node.alpha = value + EvalTraits<P>::delta;
	  if (EvalTraits<P>::betterThan(value, node.beta))
	    goto done;
	}
      }
    }
    goto done;
  } // end of in check
  if (EvalTraits<P>::betterThan(best_value, node.beta)) 
    goto done;
  if (immediate_checkmate_enabled && ImmediateCheckmate::hasCheckmateMove<P>(state)) {
    node.search_value = winByCheckmate(P);
    return;
  }
  for (Ptype ptype: PieceStand::order) {
    const int expected = static_value + node.eval.captureValue(newPtypeO(alt(P), promoteIf(ptype)));
    if (EvalTraits<P>::betterThan(node.alpha, expected))
      break;
    for (int j=Ptype_Table.getIndexMin(ptype); j<Ptype_Table.getIndexLimit(ptype); ++j) {
      const Piece p = state.pieceOf(j);
      if (! p.isOnBoardByOwner<alt(P)>())
	continue;
      node.moves.clear();
      move_generator::GenerateCapture::generate(P,state, p.square(), node.moves);
      for (Move move: node.moves) {
	if (See::see(state, move) < 0)
	  continue;
	int value = makeMoveAndQuiesce<P>(move);
	if (EvalTraits<P>::betterThan(value, best_value)) {
	  best_value = value;
	  if (EvalTraits<P>::betterThan(value, node.alpha)) {
	    if (node.node_type == PvNode)
	      node.pv.setPV(move, node, tree[depth+1].pv);
	    node.alpha = value + EvalTraits<P>::delta;
	    if (EvalTraits<P>::betterThan(value, node.beta))
	      goto done;
	  }
	}
      }
    }
  }
done:
  node.search_value = best_value;  
}

template <osl::Player P>
int osl::search::AlphaBeta3::
makeMoveAndQuiesce(Move move)
{
  ++depth;
  tree[depth] = tree[depth-1];
  tree[depth].moved = move;
  tree[depth].hash_key = tree[depth-1].hash_key.newHashWithMove(move);
  tree[depth].height -= 1;
  std::swap(tree[depth].alpha, tree[depth].beta);
  tree[depth].pv.clear();

  CallQuiesce<P> f(this);
  tree[depth].path.pushMove(move);
  state.makeUnmakeMove(move, f);
  tree[depth].path.popMove(move);

  --depth;

  return tree[depth+1].search_value;
}

template <osl::Player P>
void osl::search::AlphaBeta3::
quiesce()
{
  add_node_count(depth);

  assert(state.turn() == P);
  recorder.addQuiescenceCount();
  SearchInfo& node = tree[depth];
  if (state.hasEffectAt(P, state.kingSquare(alt(P)))) {
    node.search_value = winByFoul(P);
    return;
  }
  node.eval.update(state, node.moved);
  node.in_check = state.hasEffectAt(alt(P), state.kingSquare(P));

  const int static_value = evalValue();
  int best_value = static_value;

  if (node.in_check) {
    node.moves.clear();
    move_generator::GenerateEscape<P>::
      generate(state,state.kingPiece<P>(),node.moves);

    best_value = threatmatePenalty(P)+depth*EvalTraits<P>::delta*2;
    
    for (Move move: node.moves) {
      int value = makeMoveAndQuiesce<P>(move);
      if (EvalTraits<P>::betterThan(value, best_value)) {
	best_value = value;
	if (EvalTraits<P>::betterThan(value, node.alpha)) {
	  node.alpha = value + EvalTraits<P>::delta;
	  if (EvalTraits<P>::betterThan(value, node.beta))
	    goto done;
	}
      }
    }
    goto done;
  } // end of in check

  // leaf
  if (EvalTraits<P>::betterThan(best_value, node.beta)) 
    goto done;
  if (immediate_checkmate_enabled && node.alpha != node.beta && ImmediateCheckmate::hasCheckmateMove<P>(state)) {
    node.search_value = winByCheckmate(P);
    return;
  }
  for (size_t i=0; i<PieceStand::order.size(); ++i) {
    const Ptype ptype = PieceStand::order[i];
    const int expected = static_value + node.eval.captureValue(newPtypeO(alt(P), promoteIf(ptype)));
    if (EvalTraits<P>::betterThan(node.alpha, expected))
      break;
    for (int j=Ptype_Table.getIndexMin(ptype); j<Ptype_Table.getIndexLimit(ptype); ++j) {
      const Piece p = state.pieceOf(j);
      if (! p.isOnBoardByOwner<alt(P)>())
	continue;
      node.moves.clear();
      move_generator::GenerateCapture::generate(P,state, p.square(), node.moves);

      for (size_t k=0; k<std::min((size_t)1, node.moves.size()); ++k) {
	const Move move = node.moves[k];
	const int see = See::see(state, move);
	int value = static_value + see*eval_t::seeScale()*EvalTraits<P>::delta;
	if (EvalTraits<P>::betterThan(value, best_value)) {
	  if (node.node_type == PvNode)
	    node.pv.setPV(move, node, tree[depth+1].pv);
	  best_value = value;
	  if (i < 6 || EvalTraits<P>::betterThan(value, node.beta))
	    goto done;
	}
      }
    }
  }
done:
  node.search_value = best_value;  
}

/* ------------------------------------------------------------------------- */
osl::search::AlphaBeta3::
SearchInfo::SearchInfo() : eval((NumEffectState(SimpleState(HIRATE)))), pv()
{
}

void osl::search::AlphaBeta3::
PVVector::setPV(Move m, const SearchInfo& node, const PVVector& child)
{
  clear();
  const PVInfo info = { m, node.height, node.in_check, };
  push_back(info);
  push_back(child.begin(), child.end());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
