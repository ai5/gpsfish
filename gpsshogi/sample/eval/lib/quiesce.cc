/* quiesce.cc
 */
#include "quiesce.h"
#include "osl/search/sortCaptureMoves.h"
#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/search/quiescenceGenerator.h"
#include "osl/move_probability/featureSet.h"
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/book/bookInMemory.h"
#include "osl/move_order/captureSort.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/eval/evalTraits.h"
#include "osl/eval/pieceEval.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/promote_.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/move_action.h"
#include "osl/move_classifier/check_.h"
#include "osl/eval/see.h"
#include "osl/rating/ratedMoveVector.h"
#include "osl/csa.h"
#include <iostream>

const size_t NoDebug = (size_t)-1;

// #difeni DEBUG_QUIESCE
#ifdef DEBUG_QUIESCE
const size_t debug_time = NoDebug;
// const size_t debug_time = NoDebug-1;
// const size_t debug_time = 240;
#endif

#define LEARN_PVS
#define FOLLOW_BOOK

const int MT_CAP_DRAGON = 955 + 647;
const int EFUTIL_MG1 = MT_CAP_DRAGON*2/8;
const int EFUTIL_MG2 = MT_CAP_DRAGON*2/8;

gpsshogi::
Table::Table()
{
}

gpsshogi::
Table::~Table()
{
}

gpsshogi::Record *gpsshogi::
Table::allocate(const HashKey& key)
{
  return &table[key];
}

void gpsshogi::
Table::clear()
{
  table.clear();
}


/* ------------------------------------------------------------------------- */

gpsshogi::
BigramTable::BigramTable()
{
}

gpsshogi::
BigramTable::~BigramTable()
{
}

void gpsshogi::
BigramTable::clear()
{
  table.clear();
}

/* ------------------------------------------------------------------------- */

gpsshogi::
Quiesce::Quiesce(Eval *e, int a, int q) 
  : eval(e), all_moves_depth(a), quiesce_depth(q),
    root_depth_left(0), root_history_size(0), time(0), pv(64), node_count(0)
{
  setDepth(a, q);
}

gpsshogi::
Quiesce::~Quiesce()
{
}

void gpsshogi::
Quiesce::clear()
{
  node_count = 0;
  table.clear();
  bigram_table.clear();
}
    
bool gpsshogi::
Quiesce::quiesce(NumEffectState& state,
		 int& value, PVVector& pv)
{
  return quiesce(state, value, pv, -infty(BLACK), infty(BLACK));
}

bool gpsshogi::
Quiesce::quiesce(NumEffectState& state,
		 int& value, PVVector& pv, int alpha, int beta)
{
  assert(alpha != beta);
  history = pv;

  const Player turn = state.turn();
  if (eval::betterThan(turn, alpha, beta))
    std::swap(alpha, beta);

  key = HashKey(state);
  history_state.setRoot(state);
  if (! eval_value)
    eval_value.reset(eval->newStack(state));
  else
    eval_value->reset(state);
  root_depth_left = all_moves_depth + quiesce_depth;
  if (! bonanzaCompatible())
    assert(root_depth_left < PvMaxDepth/2);
  else
    assert(root_depth_left < PvMaxDepth);
  value = search(alpha, beta, root_depth_left);
  assert(history.size() < this->pv.size());
  pv = this->pv[history.size()];
  root_history_size = history.size();
  killers.fill(Move());
  return true;
}

void gpsshogi::
Quiesce::selectSeePlus(const NumEffectState& state, const MoveVector& src, MoveVector& dst,
		       int threshold) const
{
  RatedMoveVector moves;
  for (size_t i=0; i<src.size(); ++i) {
    const int see = osl::eval::PieceEval::computeDiffAfterMoveForRP(state, src[i]);
    if (see < threshold)
      continue;
    moves.push_back(RatedMove(src[i], see));
  }
  moves.sort();
  if (! history.empty() && history.back().isNormal()) {
    const Square last_to = history.back().to();
    for (size_t i=0; i<moves.size(); ++i) {
      if (moves[i].move().to() == last_to) {
	dst.push_back(moves[i].move());
	std::rotate(moves.begin()+(int)i, moves.begin()+(int)i+1,
		    moves.end());
	assert(moves.back().move() == dst.back());
	moves.pop_back();
	break;
      }
    }
  }
  for (size_t i=0; i<moves.size(); ++i)
    dst.push_back(moves[i].move());
}

void gpsshogi::
Quiesce::generateTacticalMoves(MoveVector& out) const
{
#ifdef FOLLOW_BOOK
  if (! bonanzaCompatible()) {
    static const BookInMemory& book = BookInMemory::instance("joseki-wide.dat");
    book.find(key, out);
    if (out.size() > 2)
      out.resize(2);
    if (! out.empty())
      return;
  }
#endif
  const Player turn = state()->turn();
  MoveVector moves;
  {
    move_action::Store store(moves);
    // capture
    for (int i=0; i<Piece::SIZE; ++i) {
      const Piece piece = state()->pieceOf(i);
      if (! piece.isOnBoardByOwner(alt(turn)))
	continue;

      move_generator::GenerateCapture::generate(turn, *state(), piece.square(), 
						store);
    }
    // promote
    const int quiesce_height = history.size() - root_history_size - all_moves_depth;
    if (bonanzaCompatible() && quiesce_height > 6) {
      if (state()->turn() == BLACK)
	move_generator::Promote<BLACK>::generateMovesPtype<move_action::Store,PAWN>(*state(), store);
      else 
	move_generator::Promote<WHITE>::generateMovesPtype<move_action::Store,PAWN>(*state(), store);
    }
    else
      move_generator::GeneratePromote<true>::generate(turn, *state(), store);
  }
  selectSeePlus(*state(), moves, out);
}

void gpsshogi::
Quiesce::generateAllMoves(MoveVector& moves, int depth_left) const
{
  if (bonanzaCompatible()) {
    MoveVector all;
    state()->generateLegal(all);
    selectSeePlus(*state(), all, moves, -40000);
    return;
  }
#ifdef FOLLOW_BOOK
  {
    static const BookInMemory& book = BookInMemory::instance();
    book.find(key, moves);
    if (moves.size() > 4)
      moves.resize(4);
    if (! moves.empty())
      return;
  }
#endif
  MoveLogProbVector all;
#ifdef USE_OLD_PROBABILITY
  static const rating::FeatureSet& feature_set
    = rating::StandardFeatureSet::instance();
  rating::RatingEnv env;
  env.make(*state(), state()->pin(state()->turn()),
	   state()->pin(alt(state()->turn())),
	   eval_value->progress16());
  feature_set.generateLogProb(*state(), env, 800, all, false);
#else
  static const move_probability::StandardFeatureSet& feature_set
    = move_probability::StandardFeatureSet::instance();
  MoveStack h;
  if (! history.empty())
    h.push(history[history.size()-1]);
  Move threatmate = move_probability::StateInfo::findShortThreatmate(*state(), h.lastMove());
  move_probability::StateInfo info(*state(), eval_value->progress16(),
				   h, threatmate);
  feature_set.generateLogProb2(info, all);
#endif
  const size_t width = (depth_left == root_depth_left) ? 8 : 4;
  // const size_t width = (depth_left + all_moves_depth/2 >= root_depth_left) ? 8 : 4;
  for (size_t i=0; i<std::min(all.size(), width); ++i)
    moves.push_back(all[i].move());
}

void gpsshogi::
Quiesce::generateTakeBack(MoveVector& out, Square to) const
{
  if (to.isPieceStand())
    return;
  MoveVector moves;
  {
    move_action::Store store(moves);
    move_generator::GenerateCapture::generate(state()->turn(), *state(), to, store);
  }

  selectSeePlus(*state(), moves, out);
}

osl::Move gpsshogi::
Quiesce::makeTakeBack(Move last_move) const
{
  if (! last_move.isNormal())
    return Move();
  const NumEffectState& state = *this->state();
  const Player turn = state.turn();
  Piece p = state.findCheapAttack(turn, last_move.to());
  if (! p.isPiece() || state.pin(turn).test(p.number()))
    return Move();
  const bool promotable = last_move.to().canPromote(turn)
    && canPromote(p.ptype());
  return Move(p.square(), last_move.to(),
	      promotable ? promote(p.ptype()) : p.ptype(),
	      last_move.ptype(), promotable, turn);
}

void gpsshogi::
Quiesce::generateEscapeFromLastMove(MoveVector& moves, Move last_move) const
{
  typedef osl::PieceEval eval_t;
  MoveVector all;
  if (state()->turn() == BLACK)
    search::QuiescenceGenerator<BLACK>::escapeFromLastMove<eval_t>(*state(), last_move, all);
  else
    search::QuiescenceGenerator<WHITE>::escapeFromLastMove<eval_t>(*state(), last_move, all);
  for (size_t i=0; i<std::min(all.size(), (size_t)2); ++i)
    moves.push_back(all[i]);
}

int gpsshogi::
Quiesce::search(int alpha, int beta, int depth_left)
{
  ++node_count;
  const bool in_pv = alpha != beta;
  const size_t my_time = ++time;
  const int full_search_depth_left
    = std::max(0, depth_left - (root_depth_left - all_moves_depth));
  const bool is_king_in_check = state()->inCheck();
  const bool consider_standpat
    = (bonanzaCompatible() || ! is_king_in_check) && ! full_search_depth_left;
#ifdef DEBUG_QUIESCE
  if (my_time == debug_time) {
    Record *record = table.allocate(key);
    std::cerr << "debug_time node " << my_time << (in_pv ? " pv" : "") << "\n"
	      << *state() << "[" << alpha << " " << beta << "] " << depth_left << "\n";
    std::cerr << "record [" << record->lower_bound << " " << record->lower_depth 
	      << "  " << record->upper_bound << " " << record->upper_depth << "] at "
	      << record->update_time << " " << record->best_move << "\n";
  }
  if (debug_time != NoDebug && time >= debug_time+1)
    std::cerr << "\nnew node " << (in_pv ? "pv " : "") << depth_left << " t " 
	      << my_time << " [" << alpha << " " << beta << "] " << history;
#endif
  const Player turn = state()->turn();
  assert(! eval::betterThan(turn, alpha, beta));
  if (state()->inCheck(alt(turn)))
    return infty(turn);
  if (history.size() > PvMaxDepth/8*7) {
    std::cerr << history_state.initialState();
    std::cerr << "warn long pv (" << alpha << ',' << beta << ")"
	      << ' ' << eval_value->value() << ' '
	      << (bonanzaCompatible() ? "B" : "")
	      << (consider_standpat ? "S" : "");
    for (Move move: history)
      std::cerr << ' ' << csa::show(move);
    std::cerr << "\n";
    return state()->inCheck() ? infty(alt(turn)) : infty(turn);
  }

  const int initial_alpha = alpha;
  int best_value = -infty(turn);
  Move best_move;
  assert(history.size() < this->pv.size());
  PVVector& cur_pv = pv[history.size()];
  cur_pv = history;
  // TODO: tsumero?
  const int stand_pat = eval_value->value();
  if (consider_standpat) {
    // assert(eval_value->value() == eval->eval(*state()));
#ifdef DEBUG_QUIESCE
    if (my_time == debug_time)
      std::cerr << "stand_pat " << stand_pat << "\n";
#endif
    if (eval::notLessThan(turn, stand_pat, beta)
	|| history.size()+1 >= history.capacity()) {
      return stand_pat;
    }
    if (eval::betterThan(turn, stand_pat, best_value)) {
      best_value = stand_pat;
      best_move = Move::PASS(turn);
      alpha = eval::max(turn, alpha, best_value);
    }
  }
  // delayed futility pruning 
  const Move last_move = history.empty() ? Move() : history.back();
  if (bonanzaCompatible()
      && ! state()->inCheck() && ! state()->wasCheckEvasion(last_move)) {
    if (full_search_depth_left == 1) {
      const int value = stand_pat - EFUTIL_MG1 * eval::delta(turn);
      if (eval::betterThan(turn, value, beta))
	return value;
    }
    else if (full_search_depth_left == 2) {
      const int value = stand_pat - EFUTIL_MG2 * eval::delta(turn);
      if (eval::betterThan(turn, value, beta))
	return value;
    }    
  }

  // table
  Record *record = table.allocate(key);
  if (! in_pv) {
    int previous_visit = std::max(record->lower_depth, record->upper_depth);
    if (previous_visit > depth_left)
      return infty(turn);
  }
  const int checkmate_special_depth = 100;

  if (! in_pv && record->lower_depth >= depth_left) {
    if (eval::betterThan(turn, record->lower_bound, best_value)) {
      best_move = record->best_move;
#ifdef DEBUG_QUIESCE
      if (debug_time != NoDebug)
	std::cerr << "lower_bound hit " << record->lower_bound << " [" << alpha << " " << beta 
		  << "] at " << my_time << " " << record->update_time << "\n";
#endif
      if (eval::betterThan(turn, record->lower_bound, alpha)) {
	if (eval::notLessThan(turn, record->lower_bound, beta)) {
	  return record->lower_bound;
	}
	alpha = record->lower_bound;
      }
      best_value = record->lower_bound;
    }
  }

  Move checkmate_move;
  if (! is_king_in_check
      && ImmediateCheckmate::hasCheckmateMove(turn, *state(), checkmate_move)) {
    record->best_move = checkmate_move;
    record->setValue(my_time, checkmate_special_depth, infty(turn));
    cur_pv.push_back(checkmate_move);
#ifdef DEBUG_QUIESCE
    if (my_time == debug_time)
      std::cerr << "checkmate at " << my_time << " " << record->lower_bound << " " << record->lower_depth
		<< " " << record->best_move << "\npv " << cur_pv;
#endif
    return infty(turn);
  }

  if ((! in_pv) && (record->upper_depth >= depth_left)) {
    if (eval::betterThan(turn, beta, record->upper_bound)) {
#ifdef DEBUG_QUIESCE
      if (debug_time != NoDebug)
	std::cerr << "upper_bound hit " << record->upper_bound << " [" << alpha << " " << beta 
		  << "] at " << my_time << " " << record->update_time << " in_pv " << in_pv << "\n";
#endif
      if (eval::notLessThan(turn, alpha, record->upper_bound)) {
	return record->upper_bound;
      }
      beta = record->upper_bound;
    }
  }

  // search specials
  FixedCapacityVector<Move,4> first_moves;
  if (record->best_move.isNormal())
    if (full_search_depth_left 
	|| (! bonanzaCompatible() && depth_left > 0
	    && (history.size() - root_history_size <= root_depth_left+2)
	    && (eval::See::see(*state(), record->best_move) >= 0)))
      first_moves.push_back(record->best_move);
  if (full_search_depth_left && !history.empty()) {
    Move bigram_killer = bigram_table.find(*state(), history.back());
    if (bigram_killer.isNormal() && !first_moves.isMember(bigram_killer))
      first_moves.push_back(bigram_killer);
  }
  Move takeback = makeTakeBack(last_move);
  if (takeback.isNormal()
      && eval::See::see(*state(), takeback) >= 0
      && ! first_moves.isMember(takeback)) {
    assert(state()->isAlmostValidMove<false>(takeback));
    first_moves.push_back(takeback);
  }
  Move killer_move = killers[history.size()-root_history_size];
  if (killer_move.isNormal()
      && state()->isAlmostValidMove<false>(killer_move)
      && (full_search_depth_left 
	  || (! bonanzaCompatible() && depth_left > 0 
	      && eval::See::see(*state(), killer_move) > 0)))
    if (! first_moves.isMember(killer_move))
      first_moves.push_back(killer_move);
  for (size_t i=0; i<first_moves.size(); ++i) {
    const Move m = first_moves[i];
    const int reduction = m.isPass() ? 2 : 1;
    assert(eval::notLessThan(turn, alpha, best_value));
    assert(eval::notLessThan(turn, beta, alpha));
    const bool extension = full_search_depth_left > 0
      && ! history.empty()
      && history.size() < PvMaxDepth/4
      && history[history.size()-1].isNormal()
      && m.to() == history[history.size()-1].to()
      && eval::See::see(*state(), m) > 0;

    int value;
    {
      const HashKey old_hash = key;
      DoUndoMoveLock lock(history_state, m);
      history.push_back(m);
      
      key = old_hash.newHashWithMove(m);
      eval_value->push(*state(), m);
#ifdef LEARN_PVS
      if (in_pv && i > 0 && root_depth_left == depth_left) 
      {
	value = search(alpha+eval::delta(turn), alpha, depth_left-reduction+extension);
	if (eval::betterThan(turn, value, alpha)) {
	  value = search(beta, alpha, depth_left-reduction+extension);
	}
      }
      else 
#endif
      {
	value = search(beta, alpha, depth_left-reduction+extension);
      }
      key = old_hash;
      eval_value->pop();
      history.pop_back();
    }
    if (eval::betterThan(turn, value, best_value)) {
      assert(history.size()+1 < this->pv.size());
      cur_pv = pv[history.size()+1];
      if (eval::betterThan(turn, value, alpha)) {
	killers[history.size()-root_history_size] = m;
	if (eval::notLessThan(turn, value, beta)) {
	  record->setLowerBound(my_time, depth_left, value, in_pv);
	  record->best_move = m;
	  return value;
	}
	alpha = value;
      }
      best_value = value;
      best_move = m;
    }
  }

  // move generation
  MoveVector moves;
  if (is_king_in_check && ! bonanzaCompatible()) {
    const bool need_selection = history.size() >= 2
      && history[history.size()-1].isNormal()
      && history[history.size()-2].isNormal()
      && history[history.size()-1].to() == history[history.size()-2].to()
      && history[history.size()-2].isDrop();
    MoveVector all;
    GenerateEscapeKing::generateCheap(*state(), all);
    for (Move move: all) {
      if (need_selection && move.isDrop()
	  && ! state()->hasEffectAt(state()->turn(), move.to()))
	continue;		// successive sacrifice
      moves.push_back(move);
    }
    if (moves.empty()) {
      record->setValue(my_time, checkmate_special_depth, -infty(turn));
      return -infty(turn);
    }
    move_order::CaptureSort::sort(moves.begin(), moves.end());
  }
  else {
    if (full_search_depth_left) {
      generateAllMoves(moves, depth_left);
    }
    else if (depth_left > 0) {
      generateTacticalMoves(moves);
      if (! bonanzaCompatible()
	  && root_depth_left == all_moves_depth + depth_left
	  && ! history.empty())
	generateEscapeFromLastMove(moves, last_move);
    } else if (! bonanzaCompatible()) {
      generateTakeBack(moves, history[history.size()-1].to());
    }
  }
#ifdef DEBUG_QUIESCE
  if (my_time == debug_time-1)
    std::cerr << "time " << my_time << " best_move " << best_move << " " << record->best_move << "\n";
#endif  
  // search all
  const int reduction = (is_king_in_check && moves.size() == 1) ? 0 : 1;
  for (MoveVector::const_iterator p=moves.begin(); p!=moves.end(); ++p) {
    if (first_moves.isMember(*p))
      continue;
    if (osl::ShouldPromoteCut::canIgnoreAndNotDrop(*p))
      continue;
    const bool extension = full_search_depth_left > 0
      && reduction > 0 && ! history.empty()
      && history.size() < PvMaxDepth/4
      && history[history.size()-1].isNormal()
      && p->to() == history[history.size()-1].to()
      && See::see(*state(), *p) > 0;
#ifdef DEBUG_QUIESCE
    if (my_time == debug_time)
      std::cerr << "try " << *p << " " << cur_pv;
#endif
    assert(eval::notLessThan(turn, alpha, best_value));
    assert(eval::notLessThan(turn, beta, alpha));

    int value;
    {
      const HashKey old_hash = key;
      DoUndoMoveLock lock(history_state, *p);
      history.push_back(*p);

      key = old_hash.newHashWithMove(*p);
      eval_value->push(*state(), *p);
#ifdef LEARN_PVS
      if (in_pv && root_depth_left == depth_left) 
      {
	value = search(alpha+eval::delta(turn), alpha, depth_left-reduction+extension);
	if (eval::betterThan(turn, value, alpha)) {
	  value = search(beta, alpha, depth_left-reduction+extension);
	}
      }
      else 
#endif
      {
	value = search(beta, alpha, depth_left-reduction+extension);
      }
#ifdef DEBUG_QUIESCE
      if (debug_time != NoDebug && time >= debug_time+1)
	std::cerr << "depth " << depth_left << " got " << value << " " << *p << " " << alpha << " " << beta << " at " 
		  << my_time << " " << time << "\n";
#endif
      key = old_hash;
      eval_value->pop();
      history.pop_back();
    }
    if (eval::betterThan(turn, value, best_value)) {
      assert(history.size()+1 < this->pv.size());
      cur_pv = pv[history.size()+1];
      if (eval::betterThan(turn, value, alpha)) {
	killers[history.size()-root_history_size] = *p;
#if 1
	if (! history.empty())
	  bigram_table.add(history.back(), *p);
#endif
	if (eval::notLessThan(turn, value, beta)) {
	  record->setLowerBound(my_time, depth_left, value, in_pv);
	  record->best_move = *p;
#ifdef DEBUG_QUIESCE
	  if (my_time == debug_time)
	    std::cerr << "debug_time beta cut " << value << " " << record->update_time
		      << " " << record->lower_bound << " " << record->lower_depth << "\n";
#endif
	  return value;
	}
	alpha = value;
      }
      best_value = value;
      best_move = *p;
#ifdef DEBUG_QUIESCE
      if (debug_time != NoDebug && my_time == debug_time)
	std::cerr << "time " << my_time << " best move updated " << best_move << "\n";
#endif
    }
  }
  record->setUpperBound(my_time, depth_left, best_value, in_pv);
  if (eval::betterThan(turn, best_value, initial_alpha)) {
    assert(in_pv || eval::notLessThan(turn, best_value, record->lower_bound) 
	   || record->lower_depth < depth_left);
    record->setLowerBound(my_time, depth_left, best_value, in_pv);
  }
  if (in_pv
      || record->best_move.isInvalid()) {
    record->best_move = best_move;
    record->update_time = my_time;
  }
  if (in_pv && ! (record->lower_bound == best_value || record->upper_bound == best_value)) {
    std::cerr << "time " << my_time << "\n" 
	      << *state() << history << record->lower_bound << " " << record->upper_bound
	      << " " << best_value << " " << initial_alpha << " " << alpha << " " << beta << "\n";
  }
#ifdef DEBUG_QUIESCE
  if (my_time == debug_time)
    std::cerr << "time " << my_time << " " << record->best_move << " " << best_move
	      << " " << best_value
	      << " [" << record->lower_bound << " " << record->upper_bound << "]"
	      << record->lower_depth << " " << record->upper_depth << "\npv " << cur_pv;
#endif
  assert(! in_pv || record->lower_bound == best_value || record->upper_bound == best_value);
  return best_value;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
