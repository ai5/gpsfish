/* alphaBeta2.cc
 */
#include "osl/search/alphaBeta2.h"
#ifdef OSL_SMP
#  include "osl/search/alphaBeta2Parallel.h"
#endif
#include "osl/search/simpleHashRecord.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/dominanceCheck.h"
#include "osl/search/moveGenerator.h"
#include "osl/search/realizationProbability.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/realizationProbability.h"
#include "osl/search/moveWithComment.h"
#include "osl/search/moveStackRejections.h"
#include "osl/search/searchMonitor.h"
#include "osl/search/usiReporter.h"
#include "osl/search/limitToCheckCount.h"
#include "osl/eval/see.h"
#include "osl/eval/pieceEval.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/csa.h"
#include "osl/record/ki2.h"
#include "osl/record/kanjiCode.h"
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/additionalEffect.h"
#include "osl/misc/eucToLang.h"
#include "osl/stat/ratio.h"
#include "osl/enterKing.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

#ifdef _WIN32
#include <chrono>
#include <thread>
#endif

#define search_assert(x, m) assert((x) || SearchState2::abort(m))

typedef osl::search::RealizationProbability Probabilities_t;

#ifdef CHECKMATE_COUNT
static size_t root_checkmate = 0, checkmate_before = 0, checkmate_after = 0,
  count_threatmate = 0, quiesce_checkmate=0;
#endif

// #define EXPERIMENTAL_QUIESCE

/* ------------------------------------------------------------------------- */
void osl::search::AlphaBeta2SharedRoot::
showLastPv(int limit) const
{
  for (int i=last_pv.size()-1; i>=0 && last_pv[i].depth == limit; --i) {
    std::cerr << last_pv[i].eval << ' ';
    for (size_t j=0; j<std::min((size_t)2, last_pv[i].pv.size()); ++j)
      std::cerr << csa::show(last_pv[i].pv[j]);
    std::cerr << "  ";
  }
  std::cerr << "\n";
}

/* ------------------------------------------------------------------------- */
/*	Constructors							     */
/* ------------------------------------------------------------------------- */

#ifndef MINIMAL
template <class EvalT>
osl::CArray<int, osl::search::SearchState2Core::MaxDepth> osl::search::AlphaBeta2Tree<EvalT>::depth_node_count;
#endif

template <class EvalT>
osl::search::AlphaBeta2Tree<EvalT>::
AlphaBeta2Tree(const NumEffectState& s, checkmate_t& c, 
	       SimpleHashTable *t, CountRecorder& r)
  : SearchBase<EvalT,SimpleHashTable,CountRecorder,RealizationProbability>(r, t), 
    SearchState2(s, c), AlphaBeta2Common<EvalT>(s), node_count(0), shared_root(new AlphaBeta2SharedRoot)
{
#ifdef OSL_SMP
  for (int i=0; i<4; ++i) {
    try 
    {
      shared.reset(new AlphaBeta2Parallel<EvalT>(this));
      break;
    }
    catch (std::bad_alloc&)
    {
      std::cerr << "panic " << i << " allocation of AlphaBeta2Parallel failed\n";
#ifdef _WIN32
      std::this_thread::sleep_for(std::chrono::seconds(1));
#else
      sleep(1);
#endif
    }
  }
#endif
}

template <class EvalT>
osl::search::AlphaBeta2Tree<EvalT>::
AlphaBeta2Tree(const AlphaBeta2Tree<EvalT>& src, AlphaBeta2Parallel<EvalT> *)
  : SearchBase<EvalT,SimpleHashTable,CountRecorder,RealizationProbability>(src), 
    SearchState2(src), SearchTimer(src), AlphaBeta2Common<EvalT>(src), 
    node_count(0), shared(src.shared), shared_root(src.shared_root)
{
  for (PVVector& p: pv)
    p.clear();
}

template <class EvalT>
osl::search::AlphaBeta2Tree<EvalT>::
~AlphaBeta2Tree()
{
  for (MoveGenerator *p: generators)
    dealloc(p);
#ifdef OSL_SMP
  if (shared && shared.use_count() == 1)
    shared.reset();
#endif
}

template <class EvalT>
osl::search::MoveGenerator *
osl::search::AlphaBeta2Tree<EvalT>::alloc()
{
  try 
  {
    return new MoveGenerator;
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of MoveGenerator failed\n";
    throw TableFull();		// stop search anyway
  }
  return 0;
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::dealloc(MoveGenerator *p)
{
  delete p;
}

template <class EvalT>
osl::search::MoveGenerator& osl::search::AlphaBeta2Tree<EvalT>::makeGenerator()
{
  const size_t cur_depth = this->curDepth();
  while (generators.size() <= cur_depth)
    generators.push_back(0);
  if (generators[cur_depth] == 0)
    generators[cur_depth] = alloc();
  return *generators[cur_depth];
}

/* ------------------------------------------------------------------------- */
/*	Methods for alpha beta search					     */
/* ------------------------------------------------------------------------- */

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
alphaBetaSearchAfterMove(const MoveLogProb& moved, Window w,
			 bool in_pv)
{
  assert(w.alpha(P) % 2);
  assert(w.beta(P) % 2);
  assert(alt(P) == state().turn());
  assert(P == moved.player());
  assert(eval::notLessThan(P, w.beta(P), w.alpha(P)));

  // Pが指した後でPの王に利きがある => 直前の手が非合法手
  if (state().inCheck(P)) {
    return this->minusInfty(P);
  }
  this->eval.update(state(), moved.move());
  const Player Turn = alt(P);
  const size_t previous_node_count = nodeCount();

  pv[this->curDepth()].clear();

  int result;
  // 局面表が利用不可能の場合にこの関数と子孫で利用するrecord
  // TODO: class に max_depth だけ持たせるべき
  std::unique_ptr<SimpleHashRecord> record_if_unavailable;
  int alloc_limit = curLimit(), memory1000 = lastMemoryUseRatio1000();
  const SimpleHashRecord *parent = hasLastRecord(1) ? lastRecord(1) : 0;
  const uint64_t table_use = this->table->memoryUse();
  if (table_use*8 > OslConfig::memoryUseLimit()
      && memory1000 > 300 && (root_limit >= 1600 || memory1000 > 500)
      && ! in_pv && ! state().inCheck() && (!parent||! parent->inCheck()))
  {
    if (table_use*6 > OslConfig::memoryUseLimit())
      alloc_limit -= std::max(root_limit - 1400, 200);
    else
      alloc_limit -= std::max((root_limit - 1400)*3/4, 0);
    if (memory1000 > 900)
      alloc_limit -= 400;
    else if (root_limit >= 1600 && memory1000 > 800)
      alloc_limit -= 200;
    else if (root_limit >= 1600 && memory1000 > 700)
      alloc_limit -= 100;
    alloc_limit = std::max(0, alloc_limit);
  }
  SimpleHashRecord *record 
    = this->table->allocate(currentHash(), alloc_limit);
  const bool has_table_record = record;
  if (! record) {
    record_if_unavailable.reset(new SimpleHashRecord());
    record = record_if_unavailable.get(); // memorize してはいけない
  }
  setCurrentRecord(record);
  record->setInCheck(state().inCheck());
#if 0
  // harmful now
  if (pass_count.loopByBothPass()) {
    return quiesce<Turn>(w);
  }
#endif
  ++node_count;
  int consumption = moved.logProb();
  
  // hash の手がない場合のiid
  if (in_pv 
      && (! record->bestMove().isNormal())) {
    assert(node_type[this->curDepth()] == PvNode);
    for (int limit = curLimit() - 200+1; limit > consumption+200;
	 limit -= 200) {
      searchAllMoves<Turn>(moved.move(), limit, 
			   record, w);
      if (! record->bestMove().validMove()) {
	Move quiesce_best = record->qrecord.bestMove();
	if (quiesce_best.isNormal())
	  record->setBestMove(quiesce_best, 200);
      }
    }
  }
  if (! in_pv) {
    // null window search
    if (node_type[this->curDepth()] == PvNode)
      node_type[this->curDepth()] = AllNode;
    result = searchAllMoves<Turn>
      (moved.move(), consumption, record, 
       Window(w.alpha(P), w.alpha(P)));
  } else {
    // normal search
    assert(node_type[this->curDepth()] == PvNode);
    result = searchAllMoves<Turn>(moved.move(), consumption, 
				  record, w);
  }
  bool extended = false;
  // extension if the alpha value is updated
  if (eval::betterThan(P, result, w.alpha(P))) {
    const SimpleHashRecord *parent = lastRecord(1);
    int consumption_here = consumption+1;
    const int re_search = 100;
    if (! w.null() && (! in_pv || consumption > re_search))
      consumption_here = std::min(consumption, re_search);
    else if (consumption > re_search
	     && (record->threatmate().status(P).status() == ThreatmateState::CHECK_AFTER_THREATMATE
		 || record->threatmate().mayHaveCheckmate(P)))
      consumption_here = re_search;
    else if (consumption > 150
	     && ((parent && parent->inCheck())
		 || state().hasEffectAt(P, state().kingSquare(alt(P)))))
      consumption_here = 150;
    if (consumption_here <= consumption) {
      node_type[this->curDepth()] = PvNode;
      extended = true;
      ext_limit.add(consumption - consumption_here);
      result = searchAllMoves<Turn>(moved.move(), consumption_here, record, w);
    }
  }
  ext.add(extended);

  if (has_table_record)
    record->addNodeCount(nodeCount() - previous_node_count);
  return result;
}

template <class EvalT>
template <osl::Player Turn>
int osl::search::AlphaBeta2Tree<EvalT>::
searchAllMoves(Move m, int limit_consumption, SimpleHashRecord *record, 
	       Window w)
{
  if (! w.null())
    assert(node_type[this->curDepth()] == PvNode);
  const Player P = alt(Turn);
  this->recorder.tryMove(MoveLogProb(m, limit_consumption),
		   w.alpha(P), curLimit());
  subLimit(limit_consumption);

  const int result = searchAllMoves<Turn>(record, w);

  addLimit(limit_consumption);
  this->recorder.recordValue(MoveLogProb(m, limit_consumption),
			     result,eval::betterThan(P, result, w.alpha(P)),
		       curLimit());
  return result;
}

template <class EvalT>
template <osl::Player P>
void osl::search::AlphaBeta2Tree<EvalT>::
testThreatmate(SimpleHashRecord *record, bool in_pv)
{
  if ((! record->inCheck())
      && (! (record && record->threatmate().isThreatmate(P)))
      && (in_pv || (this->curDepth() > 0 
		    && this->node_type[this->curDepth()-1] != CutNode))
      && tryThreatmate())
  {
    int threatmate_limit = 0;
    const SimpleHashRecord *parent = lastRecord(1);
    size_t node_count = record->nodeCount();
    if (parent)
      node_count = std::max(node_count, parent->nodeCount()/32);
    threatmate_limit = 4500-this->curDepth()*1000;
    int threatmate_max = 0;
    if ((node_count >= 1000 && this->recorder.checkmateRatio() < 0.5)
	|| (node_count >= 200 
	    && (state().king8Info(P).libertyCount() == 0
		|| state().king8Info(P).dropCandidate()
		|| state().king8Info(P).template hasMoveCandidate<alt(P)>(state()))))
      threatmate_max = 100;
    threatmate_limit = std::max(threatmate_limit, threatmate_max);
    if (! in_pv)
      threatmate_limit /= 2;
    if (root_limit < 800)
      threatmate_limit /= 2;
#ifdef EXPERIMENTAL_QUIESCE
    if (curLimit() <= 400)
      threatmate_limit = 1;
    else if (curLimit() <= 500)
      threatmate_limit /= 16;
    else if (curLimit() <= 600)
      threatmate_limit /= 4;
#endif

    if (curLimit() >= this->table->minimumRecordLimit())
    {
      threatmate_limit = record->qrecord.threatmateNodesLeft(threatmate_limit);
    }
    else
    {
      threatmate_limit /= 2;		// for safety against multiple calls
    }

    Move threatmate_move;
    this->recorder.gotoCheckmateSearch(state(), threatmate_limit);
#ifdef CHECKMATE_COUNT
    size_t count = checkmateSearcher().totalNodeCount();
#endif
    bool threatmate
      = isThreatmateState<P>(threatmate_limit, threatmate_move);
#ifdef CHECKMATE_COUNT
    count_threatmate += checkmateSearcher().totalNodeCount() - count;
#endif
    if (threatmate_limit > 100) {
      updateCheckmateCount();
      testStop();
    }
    this->recorder.backFromCheckmateSearch();
    if (!threatmate && threatmate_limit == 0
	&& record->qrecord.threatmateNodesLeft(2)) {
      threatmate = isThreatmateStateShort<P>(2, threatmate_move);
    }
    if (threatmate)
    {
      record->threatmate().setThreatmate(P, threatmate_move);
    }
  }
}

template <class EvalT>
template <osl::Player P>
bool osl::search::AlphaBeta2Tree<EvalT>::
tryCheckmate(SimpleHashRecord *record, bool in_pv, Move& checkmate_move)
{
  int checkmate_limit = 1;
  if (! in_pv) {
    const SimpleHashRecord *parent = lastRecord(1);
    if (! (record->threatmate().mayHaveCheckmate(alt(P))
	   || (parent && parent->threatmate().maybeThreatmate(alt(P)))))
      return false;
    // simulation only
  }
  if (in_pv && root_limit >= 500+this->rootLimitBias()) {
    int depth = this->curDepth();
    if (root_limit >= 700+this->rootLimitBias() && depth <= 3) {
      if (/*record_memorize &&*/ depth <= 1)
	checkmate_limit = checkmate::limitToCheckCount(3500);
      else if (/* record_memorize &&*/ depth == 2)
	checkmate_limit = 1000;
      else if (/* record_memorize &&*/ depth == 3)
	checkmate_limit = 200;
    }
    else if (((root_limit - curLimit()) <= 500) || (depth <= 5))
    {
      assert(static_cast<unsigned int>(curLimit()) < 4096);
      checkmate_limit = checkmate::limitToCheckCount(std::max(0,curLimit()-200-this->rootLimitBias()));
    }
    const SimpleHashRecord *parent = lastRecord(1);
    if (record->threatmate().mayHaveCheckmate(alt(P))
	|| (parent && parent->threatmate().maybeThreatmate(alt(P))))
      checkmate_limit += std::max(100, checkmate_limit);
    else
      checkmate_limit = std::min((long)record->nodeCount()/2, (long)checkmate_limit);
    if (root_limit < 800)
      checkmate_limit /= 2;
  }
  if (curLimit() >= this->table->minimumRecordLimit())
  {
    // 詰将棋はある程度深さが増えたときだけ呼ぶ
    checkmate_limit = record->qrecord.checkmateNodesLeft(checkmate_limit);
    if (checkmate_limit <= 0)
      return false;
  }
  else
  {
    checkmate_limit /= 2;		// for safety against multiple calls
  }
  
  // 相手を詰ますことを考える 
#ifdef CHECKMATE_COUNT
  size_t count = checkmateSearcher().totalNodeCount();
#endif
  this->recorder.gotoCheckmateSearch(state(), checkmate_limit);
  const bool win = isWinningState<P>
    (checkmate_limit, checkmate_move);
  if (checkmate_limit > 100)
    updateCheckmateCount();
  this->recorder.backFromCheckmateSearch();
#ifdef CHECKMATE_COUNT
  checkmate_before += checkmateSearcher().totalNodeCount() - count;
#endif
  if (this->root_limit >= 1600 && checkmate_limit >= 100)
    this->checkmate_searcher->runGC(this->table->isVerbose(),
				    lastMemoryUseRatio1000());
  return win;
}

template <class EvalT>
template <osl::Player P>
bool osl::search::AlphaBeta2Tree<EvalT>::
tryCheckmateAgain(SimpleHashRecord *record, Move& checkmate_move,
		  int node_count, int best_value)
{
  int checkmate_limit = 1;
  if (record->threatmate().maybeThreatmate(P)) {
    if (EvalTraits<P>::betterThan(this->eval.captureValue(newPtypeO(P,KING)), best_value))
      checkmate_limit = node_count / 2; // この局面は必死 or 詰将棋以外のalt(P)の勝
    else 
      checkmate_limit = node_count / 8;
    checkmate_limit += 100;
    if (this->recorder.checkmateRatio() < 0.5) {
      int checkmate_importance_wrt_root = this->recorder.searchNodeCount()/2
	+ this->recorder.checkmateCount()/8;
      for (int i=0; i<this->curDepth(); ++i) {
	if (this->in_pv[i])
	  checkmate_importance_wrt_root = checkmate_importance_wrt_root*7/8;
	else
	  checkmate_importance_wrt_root /= 7;
      }
      checkmate_limit = std::max(checkmate_limit, checkmate_importance_wrt_root);
    }
  } else if (record->threatmate().mayHaveCheckmate(alt(P))) {
    checkmate_limit = countCheckAfterThreatmate(alt(P),2)*320 + 100;
#ifdef MORE_CHECKMATE_IF_CAPTURE_MAJOR
    if (lastMove().isNormal() && isMajorNonPieceOK(lastMove().capturePtype())
	&& ! state().hasEffectAt(P, lastMove().to()))
      checkmate_limit += 20000;
#endif
  }
  if (curDepth() == 1 && hasLastRecord(1)) {
    const SimpleHashRecord *parent = lastRecord(1); // root
    if (parent->inCheck() || parent->threatmate().maybeThreatmate(alt(P)))
      checkmate_limit = std::max(checkmate_limit, (int)parent->nodeCount()/2+parent->checkmateNodes());
  }

  // adjustment by time
  int checkmate_afford = this->nodeAffordable();
#ifdef OSL_SMP
  checkmate_afford *= 1.5 / shared->max_threads;
#endif
  if (checkmate_limit > 100 && checkmate_limit > checkmate_afford) {
#ifndef NDEBUG
    if (checkmate_afford > 0 && toSeconds(this->timeAssigned().standard) >= 10.0)
      std::cerr << "adjust checkmate near timeover " << checkmate_limit << " => " << checkmate_afford << "\n";
#endif
    checkmate_limit = checkmate_afford;
  }

  if (true /*record_memorize*/)
    checkmate_limit = record->qrecord.checkmateNodesLeft(checkmate_limit);

#ifdef CHECKMATE_COUNT
  size_t count = checkmateSearcher().totalNodeCount();
#endif
  this->recorder.gotoCheckmateSearch(state(), checkmate_limit);
  const bool win = isWinningState<P>
    (checkmate_limit, checkmate_move);
  if (checkmate_limit > 100)
    updateCheckmateCount();
  this->recorder.backFromCheckmateSearch();
#ifdef CHECKMATE_COUNT
  checkmate_after += checkmateSearcher().totalNodeCount() - count;
#endif
  if (this->root_limit >= 1600 && checkmate_limit >= 100)
    this->checkmate_searcher->runGC(this->table->isVerbose(),
				    lastMemoryUseRatio1000());
  return win;
}

template <class EvalT>
bool osl::search::
AlphaBeta2Tree<EvalT>::tryPass(SimpleHashRecord *record, Player P) const
{
  return ! record->inCheck()
    && ! record->threatmate().maybeThreatmate(P);
}

template <class EvalT>
template <osl::Player P>
const osl::MoveLogProb osl::search::
AlphaBeta2Tree<EvalT>::nextMove()
{
  MoveGenerator& generator = makeGenerator();
  SimpleHashRecord *record = lastRecord();
  switch (this->move_type[this->curDepth()]) {
  case common_t::HASH:
  {
    if (curLimit() < this->leafLimit()) {
      this->move_type[this->curDepth()] = common_t::FINISH;
      break;
    }
    this->move_type[this->curDepth()] = common_t::TACTICAL;
    MoveLogProb best_move_in_table = record->bestMove();
    assert(curLimit() > 0);
    generator.init<eval_t>(curLimit(), record, this->eval, state(), 
			   node_type[this->curDepth()] != CutNode,
			   best_move_in_table.move());
    if (best_move_in_table.validMove() && 
	this->validTableMove(state(), best_move_in_table, curLimit())) {
      if (this->in_pv[this->curDepth()] 
	  || best_move_in_table.move().capturePtype())
	best_move_in_table.setLogProbAtMost(RealizationProbability::TableMove);
      else 
	best_move_in_table.setLogProbAtMost(state().inCheck() ? 120 : 150);
      return best_move_in_table;
    }
  }
  // fall through
  // TODO: 打歩詰めはここでチェックすると早そう
  case common_t::TACTICAL:
  {
    MoveLogProb m = generator.nextTacticalMove<P>(*this);
    if (m.validMove())
      return m;
    // fall through
    this->move_type[this->curDepth()] = common_t::KILLER;
    this->killers[this->curDepth()].clear();
    if ((! record->inCheck())
	&& ! record->threatmate().maybeThreatmate(P)
	&& (curLimit() >= 300)) {
      MoveVector killer_moves;	// TODO: 効率化
      getKillerMoves(killer_moves);
      for (Move move: killer_moves) {
	assert(this->killers[this->curDepth()].size() < this->killers[this->curDepth()].capacity());
	this->killers[this->curDepth()].push_back(move);
      }
      std::reverse(this->killers[this->curDepth()].begin(), this->killers[this->curDepth()].end());
    }
  }
  case common_t::KILLER:
  {
    typename common_t::killer_t& killers = this->killers[this->curDepth()];
    if (! killers.empty()) {
      Move m = killers[killers.size()-1];
      killers.pop_back();
      return MoveLogProb(m, 300);
    }
    // fall through
    this->move_type[this->curDepth()] = common_t::PASS;
  }
  case common_t::PASS:
    assert(record->inCheck() == state().inCheck());
    this->move_type[this->curDepth()] = common_t::ALL;
    if (tryPass(record, P)) {
      const int pass_consumption = (curLimit() >= 800) ? 300 : 200;
      return MoveLogProb(Move::PASS(P), pass_consumption);
    }
    // TODO: pass の後の最善手
    // fall through
  case common_t::ALL:
  {
    MoveLogProb m = generator.nextMove<P>(*this);
    if (m.validMove())
      return m;
    this->move_type[this->curDepth()] = common_t::FINISH;
  }
  default:
    assert(this->move_type[this->curDepth()] == common_t::FINISH);
  }
  return MoveLogProb();
}
  
template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
searchAllMoves(SimpleHashRecord *record, Window w)
{
#ifndef NDEBUG
  checkPointSearchAllMoves();
#endif
  assert(P == state().turn());
  search_assert(w.isConsistent(), lastMove());
  assert(curLimit() >= 0);

  assert(hasLastRecord(1));
  const SimpleHashRecord *parent = lastRecord(1);
#ifndef DONT_USE_CHECKMATE
  const int node_count_at_beginning = nodeCount();
#endif
#if (! defined OSL_USE_RACE_DETECTOR) && (! defined MINIMAL)
  depth_node_count[this->curDepth()]++;
#endif
  this->move_type[this->curDepth()] = common_t::INITIAL;
  const bool in_pv = this->in_pv[this->curDepth()] = ! w.null();

  // テーブルにある値を調べる
  if (record) {
    if (in_pv) {
      if (record->hasLowerBound(SearchTable::HistorySpecialDepth)) {
	int lower_bound = record->lowerBound();
	if (this->isWinValue(P, lower_bound)
	    || (record->hasUpperBound(SearchTable::HistorySpecialDepth)
		&& record->upperBound() == lower_bound))
	  return lower_bound;
      }
      if (record->hasUpperBound(SearchTable::HistorySpecialDepth)) {
	int upper_bound = record->upperBound();
	if (this->isWinValue(alt(P), upper_bound))
	  return upper_bound;
      }
    }
    else {			// ! in_pv
      int table_value = 0;
      if (record->hasGreaterLowerBound<P>(curLimit(), w.alpha(P), 
					  table_value)) {
	assert(eval::isConsistentValue(table_value));
	w.alpha(P) = table_value + EvalTraits<P>::delta;
	if (EvalTraits<P>::betterThan(table_value, w.beta(P))) {
	  this->recorder.tableHitLowerBound(P, table_value, w.beta(P), curLimit());
	  return table_value;
	}
      } 
      if (record->hasLesserUpperBound<P>(curLimit(), w.beta(P), table_value)) {
	assert(eval::isConsistentValue(table_value));
	w.beta(P) = table_value - EvalTraits<P>::delta;
	if (EvalTraits<P>::betterThan(w.alpha(P), table_value))
	{
	  this->recorder.tableHitUpperBound(P, table_value, w.alpha(P), curLimit());
	  return table_value;
	}
      }
    }

    Move checkmate_move=Move::INVALID();
    if ((!record->inCheck())
	&& record->qrecord.checkmateNodesLeft(1)
	&& isWinningStateShort<P>(2, checkmate_move))
    {
      this->recordWinByCheckmate(P, record, checkmate_move);
      return this->winByCheckmate(P);
    }    
#ifndef DONT_USE_CHECKMATE
    assert(record);
    // try simulation or simple checkmate search
    int additional_limit = 0;		// simulation only
    if (parent && parent->threatmate().maybeThreatmate(alt(P)))
    {
      additional_limit = std::max(100, parent->qrecord.threatmateNodes()/8);
      additional_limit = record->qrecord.checkmateNodesLeft(additional_limit);
    }
    this->recorder.gotoCheckmateSearch(state(), additional_limit);
    const bool win = isWinningState<P>(additional_limit, checkmate_move);
    updateCheckmateCount();
    this->recorder.backFromCheckmateSearch();
    if (win) {      
      assert(checkmate_move.isValid());
      this->recordWinByCheckmate(P, record, checkmate_move);
      return this->winByCheckmate(P);
    }
#endif
  }

  search_assert(w.isConsistent(), lastMove());
  const int initial_alpha = w.alpha(P);

#ifndef DONT_USE_CHECKMATE
  // 詰めろを考える
  testThreatmate<P>(record, in_pv);
#endif
  // 探索前に ThreatmateState を設定
  record->qrecord.updateThreatmate(P, (parent ? &(parent->threatmate()) : 0), 
				   state().inCheck());

  MoveLogProb best_move;	// invalidated
  int best_value = this->minusInfty(P);
  int tried_moves = 0;
  int alpha_update = 0;
  int last_alpha_update = 0;
#if (defined OSL_SMP) 
  int last_smp_idle = 0;
#  if (! defined OSL_SMP_NO_SPLIT_INTERNAL)
#    if (! defined NDEBUG)
  bool already_split = false;
#    endif
#  endif
#endif

  // the first move
  MoveLogProb m = nextMove<P>();
  ++tried_moves;
  if (! m.validMove() || m.logProb() > curLimit()) {
    goto move_generation_failure;
  }
#if (defined OSL_SMP) && (! defined OSL_SMP_NO_SPLIT_INTERNAL)
  int first_move_node;
#endif
  {
#if (defined OSL_SMP) && (! defined OSL_SMP_NO_SPLIT_INTERNAL)
    const int previous_node_count = nodeCount();
#endif
    assert(eval::betterThan(P, w.alpha(P), best_value));
    const int result = alphaBetaSearch<P>(m, w, in_pv);
    if (eval::betterThan(P, result, best_value)) {
      best_value = result;
      best_move = m;
      if (eval::betterThan(P, best_value, w.alpha(P))) {
	w.alpha(P) = result + EvalTraits<P>::delta;;
	assert(best_move.validMove());
	++alpha_update;
	last_alpha_update = 1;
	if (eval::betterThan(P, result, w.beta(P))) {
	  mpn_cut.add(tried_moves);
	  if (this->move_type[this->curDepth()] >= common_t::ALL) {
	    setKillerMove(best_move.move());
	    if (best_move.isNormal()
		&& ! best_move.move().isCapture())
	    {
	      const int d = (curLimit()+200)/100;
	      this->historyTable().add(best_move.move(), d*d);
	    }
	  }
	  assert(best_move.validMove());
	  assert(! this->isWinValue(alt(P), best_value));
	  goto register_table;
	} else {
	  if (in_pv) 
	    makePV(m.move());
	}
      }
    }
#if (defined OSL_SMP) && (! defined OSL_SMP_NO_SPLIT_INTERNAL)
    first_move_node = nodeCount() - previous_node_count;
#endif
  }
  testStop();

#ifndef DONT_USE_CHECKMATE
  if (in_pv)
  {
    Move checkmate_move;
    if (tryCheckmate<P>(record, in_pv, checkmate_move)) {
      assert(checkmate_move.isValid());
      best_value= this->winByCheckmate(P);
      this->recordWinByCheckmate(P, record, checkmate_move);
      return best_value;
    }
  }
#endif
  search_assert(w.isConsistent(), lastMove());
  if (curLimit() < this->leafLimit())
    goto move_generation_failure;
  while (true) {
#if (defined OSL_SMP) && (! defined OSL_SMP_NO_SPLIT_INTERNAL)
    const bool prefer_split = shared && curLimit() >= shared->split_min_limit
      && (curLimit() >= 600+this->rootLimitBias()
	  // || (curLimit() >= 400 && this->curDepth() < 2)
	  || (first_move_node >= 30000));
    try {
      if (prefer_split) {
	int cur_smp_idle; 
	{
# ifdef OSL_USE_RACE_DETECTOR
	  std::lock_guard<std::mutex> lk(shared->lock_smp);
#endif
	  cur_smp_idle = shared->smp_idle;
	}
	if (cur_smp_idle > last_smp_idle) {
	  last_smp_idle = cur_smp_idle;
	  assert(! already_split);
# if (! defined NDEBUG)
	  already_split = true;
# endif
	  if (examineMovesOther<P>(w, best_move, best_value, tried_moves,
				   alpha_update, last_alpha_update)) {
	    assert(best_move.validMove());
	    assert(best_move.player() == P);
	    if (this->move_type[this->curDepth()] >= common_t::ALL) {
	      setKillerMove(best_move.move());
	      if (best_move.isNormal()
		  && ! best_move.move().isCapture())
	      {
		const int d = (curLimit()+200)/100;
		this->historyTable().add(best_move.move(), d*d);
	      }
	    }
	    mpn_cut.add(tried_moves);    
	    goto register_table;
	  }
	  goto all_moves_done;
	}
      }
    }
    catch(AlphaBeta2ParallelCommon::SplitFailed&) {
# if (! defined NDEBUG)
      already_split = false;
# endif
      // fall through
    }
#endif
    MoveLogProb m = nextMove<P>(); 
    if (! m.validMove())
      break;
    ++tried_moves;

    assert(eval::betterThan(P, w.alpha(P), best_value));
    const int result = alphaBetaSearch<P>(m, w, in_pv && ! best_move.validMove());
    if (eval::betterThan(P, result, best_value)) {
      best_value = result;
      best_move = m;
      if (eval::betterThan(P, best_value, w.alpha(P))) {
	w.alpha(P) = result + EvalTraits<P>::delta;;
	assert(best_move.validMove());
	++alpha_update;
	last_alpha_update = tried_moves;
	if (eval::betterThan(P, result, w.beta(P))) {
	  assert(best_move.validMove());
	  if (this->move_type[this->curDepth()] >= common_t::ALL) {
	    setKillerMove(best_move.move());
	    if (best_move.isNormal()
		&& ! best_move.move().isCapture())
	    {
	      const int d = (curLimit()+200)/100;
	      this->historyTable().add(best_move.move(), d*d);
	    }
	  }
	  mpn_cut.add(tried_moves);
	  goto register_table;
	} else {
	  if (in_pv)
	    makePV(m.move());
	}
      }
    }
  }
#if (defined OSL_SMP) && (! defined OSL_SMP_NO_SPLIT_INTERNAL)
all_moves_done:
#endif
  if (tried_moves == 1 && tryPass(record, P)) {
    // goto quiescence search if tried move is only null move
    goto move_generation_failure;
  }
  mpn.add(tried_moves);
  if (alpha_update) {
    this->alpha_update.add(alpha_update);
    this->last_alpha_update.add(last_alpha_update);
  }
  // 宣言勝
  if (((this->curDepth() % 2) == 0 || OslConfig::usiMode())
      && EnterKing::canDeclareWin<P>(state())) {
    best_value = this->brinkmatePenalty(alt(P), std::max(1,16-this->curDepth())*256)
      + this->eval.value();
    record->setAbsoluteValue(Move::DeclareWin(), best_value,
			     SearchTable::CheckmateSpecialDepth);
    return best_value;
  }
  if (record) {
    // もう一度，相手を詰ますことを考える
#ifndef DONT_USE_CHECKMATE
    Move checkmate_move=Move::INVALID();
    if (tryCheckmateAgain<P>(record, checkmate_move, 
			     nodeCount() - node_count_at_beginning,
			     best_value)) {
      assert(checkmate_move.isValid());
      best_value= this->winByCheckmate(P);
      this->recordWinByCheckmate(P, record, checkmate_move);
      return best_value;
    }
#endif
  }
register_table:
  assert(best_value == this->minusInfty(P) || best_move.validMove());
  assert(eval::isConsistentValue(best_value));
  if (this->isWinValue(alt(P), best_value))
  {
    // TODO: 直前の着手が(やけくそ)王手の連続だった場合
    // 必死ではなくて詰扱いの方が良い可能性はある
    best_value = this->brinkmatePenalty(P, std::max(1,16-this->curDepth())*256) + this->eval.value();

    // (この深さでは)上限==下限
    record->setAbsoluteValue(best_move, best_value, curLimit());
    return best_value;
  }
  else if (EvalTraits<P>::betterThan(w.alpha(P), initial_alpha)) {
    if (best_move.validMove()) {
      assert(best_value % 2 == 0);
      record->setLowerBound(P, curLimit(), best_move, best_value);
    }
  }
  if (EvalTraits<P>::betterThan(w.beta(P), best_value)) {
    if (best_move.validMove())
      record->setUpperBound(P, curLimit(), best_move, best_value);
  }
  return best_value;
move_generation_failure:
  pv[this->curDepth()].clear();
  // 手を生成できなかった
  // limit が 200 未満だった場合と，以上だったが move generator が limit 以下の手を生成できなかった場合ここに来る
  best_value = quiesce<P>(w);
  if (record)
  {
    if (EvalTraits<P>::betterThan(best_value, initial_alpha)) {
      if (EvalTraits<P>::betterThan(w.beta(P), best_value)) {
	record->setAbsoluteValue(MoveLogProb(), best_value, curLimit());
      } else {
	record->setLowerBound(P, curLimit(), MoveLogProb(), best_value);
      }
    }
    else 
    {
      assert(EvalTraits<P>::betterThan(w.beta(P), best_value));
      record->setUpperBound(P, curLimit(), MoveLogProb(), best_value);
    }
  }
  assert(eval::isConsistentValue(best_value));
  return best_value;
}

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
quiesce(Window w)
{
#ifdef EXPERIMENTAL_QUIESCE
  return quiesceExp<P>(w);
#else
  return quiesceStable<P>(w);
#endif
}

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
quiesceStable(Window w)
{
  testStop();
  initPV();
  
  typedef QuiescenceSearch2<eval_t> qsearcher_t;
  qsearcher_t qs(*this, *this->table);
  Move last_move = lastMove();
  if (last_move.isInvalid())
    last_move = Move::PASS(alt(P));
  assert(w.alpha(P) % 2);
  assert(w.beta(P) % 2);
#ifdef CHECKMATE_COUNT
  size_t count = checkmateSearcher().totalNodeCount();
#endif
  const int result = qs.template search<P>(w.alpha(P), w.beta(P), this->eval, last_move, 4);
  node_count += qs.nodeCount();
  this->recorder.addQuiescenceCount(qs.nodeCount());
#ifdef CHECKMATE_COUNT
  quiesce_checkmate += checkmateSearcher().totalNodeCount() - count;
#endif

  assert(result % 2 == 0);
  return result;
}

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
quiesceExp(Window w)
{
  testStop();

  SimpleHashRecord *record = lastRecord();
  assert(record);
  Move best_move;
  const int qdepth = 4;
  const int previous_node_count = nodeCount();

  int result =
    quiesceRoot<P>(w, qdepth, best_move, record->threatmate());
  
  const size_t qnode = nodeCount() - previous_node_count;
  this->recorder.addQuiescenceCount(qnode);
  record->qrecord.setLowerBound(qdepth, result, best_move);
  return result;
}

template <class EvalT>
template <osl::Player P>
struct osl::search::AlphaBeta2Tree<EvalT>::NextQMove
{
  AlphaBeta2Tree *searcher;
  Window window;
  const int depth;
  int *result;
  DualThreatmateState threatmate;
  NextQMove(AlphaBeta2Tree *s, Window w, int d, int *r,
	    DualThreatmateState t)
    : searcher(s), window(w), depth(d), result(r), threatmate(t) {
  }
  void operator()(Square /*last_to*/) {
    searcher->eval.update(searcher->state(), searcher->lastMove());
    *result = 
      searcher->quiesce<P>(window, depth, threatmate);
  }
};

template <class EvalT>
template <osl::Player P>
bool osl::search::AlphaBeta2Tree<EvalT>::
quiesceWithMove(Move move, Window& w, int depth_left, Move& best_move, int& best_value, 
		const DualThreatmateState& threatmate)
{
  // TODO: futility margin
  const bool in_pv = ! w.null();
  int result;
  typedef NextQMove<alt(P)> next_t;
  next_t helper(this, w, depth_left, &result, threatmate);

  const HashKey new_hash = currentHash().newHashWithMove(move);
  const eval_t old_eval = this->eval;
  doUndoMoveOrPass<P,next_t>(new_hash, move, helper);
  this->eval = old_eval;

  if (eval::betterThan(P, result, best_value)) {
    best_value = result;
    best_move = move;
    if (eval::betterThan(P, best_value, w.alpha(P))) {
      w.alpha(P) = result + EvalTraits<P>::delta;
      if (in_pv)
	makePV(best_move);
      if (eval::betterThan(P, result, w.beta(P))) {
	return true;
      }
    }
  }
  return false;
}

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
quiesceRoot(Window w, int depth_left, Move& best_move, DualThreatmateState threatmate)
{
  assert(! state().inCheck(alt(P)));

  initPV();
  // depth_node_count_quiesce[this->curDepth()]++;
  // ++node_count;

  SimpleHashRecord& record = *lastRecord();
  assert(record.inCheck() == state().inCheck());
  assert(depth_left > 0);
  
  int best_value = this->minusInfty(P);
  // stand pat 
  if (! record.inCheck()) {
    if (! threatmate.maybeThreatmate(P)) {
      best_value = this->eval.value();
    } else {
      const int value = this->eval.value() + this->threatmatePenalty(P);
      best_value = EvalTraits<P>::max(best_value, value);
    }
    best_move = Move::PASS(P);
    if (EvalTraits<P>::betterThan(best_value, w.alpha(P))) {
      if (EvalTraits<P>::betterThan(best_value, w.beta(P)))
	return best_value;
      w.alpha(P) = best_value + EvalTraits<P>::delta;
    }
  }

  Move prev_best = record.qrecord.bestMove();
  MoveGenerator& generator = makeGenerator();
  generator.init(200, &record, this->eval, state(), 
		 w.alpha(P) == w.beta(P),
		 prev_best, true);
  int tried_moves = 0;
  // previous 最善手を試す
  if (prev_best.isNormal()) {
    ++tried_moves;
    if (quiesceWithMove<P>(prev_best, w, depth_left-1, best_move, best_value,
			   threatmate))
      goto finish;
  }


  // captures or king escape
  for (MoveLogProb m = generator.nextTacticalMove<P>(*this); 
       m.validMove(); m = generator.nextTacticalMove<P>(*this)) {
    ++tried_moves;
    if (quiesceWithMove<P>(m.move(), w, depth_left-1, best_move, best_value,
			   threatmate))
      goto finish;
  } 
  for (MoveLogProb m = generator.nextMove<P>(*this); 
       m.validMove(); m = generator.nextMove<P>(*this)) {
    ++tried_moves;
    if (quiesceWithMove<P>(m.move(), w, depth_left-1, best_move, best_value,
			   threatmate))
      goto finish;
  } 

  // pawn drop foul?
  if (record.inCheck()) {
    if (tried_moves == 0) {
      if (lastMove().isNormal() && lastMove().ptype() == PAWN && lastMove().isDrop())
	return this->winByFoul(P);
      return this->winByCheckmate(alt(P));
    }
    goto finish;
  }  
finish:
  return best_value;
}

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
quiesce(Window w, int depth_left, DualThreatmateState parent_threatmate)
{
  if (state().inCheck(alt(P))) {
    return this->minusInfty(alt(P));
  }

  initPV();
#ifndef MINIMAL
  depth_node_count_quiesce[this->curDepth()]++;
#endif
  ++node_count;

  SimpleHashRecord record;
  record.setInCheck(state().inCheck());

  DualThreatmateState threatmate;
  threatmate.updateInLock(P, &parent_threatmate, record.inCheck());

  int best_value = this->minusInfty(P);
  // TODO: 玉の回りのtakeback延長
  if (depth_left <= 0) {
    if (record.inCheck()) {
      if (lastMove().isCapture())
	depth_left +=2;
      else
	depth_left = 0;
    }
    else if (threatmate.maybeThreatmate(P)) {
      if (threatmate.mayHaveCheckmate(alt(P))) {
	Move checkmate_move;
	bool win = isWinningState<P>(10, checkmate_move);
	if (win)
	  return this->winByCheckmate(P);
      }
      return this->eval.value() + this->threatmatePenalty(P);
    }
    else {
      if (threatmate.mayHaveCheckmate(alt(P)))
	return this->eval.value() + this->threatmatePenalty(alt(P));
      if (ImmediateCheckmate::hasCheckmateMove<P>(state()))
	return this->winByCheckmate(P);
      if (ImmediateCheckmate::hasCheckmateMove<alt(P)>(state()))
	return this->eval.value() + this->threatmatePenalty(P);
      return this->eval.value();
    }
  }

  if (! record.inCheck()) {
    if (ImmediateCheckmate::hasCheckmateMove<P>(state())) {
      return this->winByCheckmate(P);
    }
  }
  if (threatmate.mayHaveCheckmate(alt(P))) {
    Move checkmate_move;
    bool win = isWinningState<P>(10, checkmate_move);
    if (win)
      return this->winByCheckmate(P);
  }
  MoveGenerator& generator = makeGenerator();
  generator.init(200, &record, this->eval, state(), 
		 w.alpha(P) == w.beta(P),
		 Move(), true);
  int tried_moves = 0;
  Move best_move;
  // stand pat
  if (! record.inCheck()) {
    if (! threatmate.maybeThreatmate(P)) {
      best_value = this->eval.value();
    } else {
      const int value = this->eval.value() + this->threatmatePenalty(P);
      best_value = EvalTraits<P>::max(best_value, value);
    }
    best_move = Move::PASS(P);
    if (EvalTraits<P>::betterThan(best_value, w.alpha(P))) {
      if (EvalTraits<P>::betterThan(best_value, w.beta(P)))
	return best_value;
      w.alpha(P) = best_value + EvalTraits<P>::delta;
    }
  }

  // captures or king escape
  for (MoveLogProb m = generator.nextTacticalMove<P>(*this); 
       m.validMove(); m = generator.nextTacticalMove<P>(*this)) {
    ++tried_moves;
    if (quiesceWithMove<P>(m.move(), w, depth_left-1, best_move, best_value,
			   threatmate))
      goto finish;
  } 
  for (MoveLogProb m = generator.nextMove<P>(*this); 
       m.validMove(); m = generator.nextMove<P>(*this)) {
    ++tried_moves;
    if (quiesceWithMove<P>(m.move(), w, depth_left-1, best_move, best_value,
			   threatmate))
      goto finish;
  } 

  // pawn drop foul?
  if (record.inCheck()) {
    if (tried_moves == 0) {
      if (lastMove().ptype() == PAWN && lastMove().isDrop()) 
	return this->winByFoul(P);
      return this->winByCheckmate(alt(P));
    }
    goto finish;
  }  
finish:
  return best_value;
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::updateCheckmateCount()
{
#ifdef OSL_SMP
  if (shared) {
    const size_t new_count = shared->checkmateCount();
    this->recorder.setCheckmateCount(new_count);
    return;
  }
#endif
  this->recorder.setCheckmateCount
    (checkmateSearcher().totalNodeCount());
}

template <class EvalT>
int 
osl::search::AlphaBeta2Tree<EvalT>::
rootAlpha(Player P, int last_value, Progress16 progress)
{
  int pawns = 3;
  if (eval::betterThan(P, last_value, eval_t::captureValue(newPtypeO(alt(P),KING))))
  {
    pawns = 10;
  }
  else if (progress.value() <= 1)
  {
    pawns = 3;
  }
  else if (progress.value() <= 7)
  {
    pawns = 4;
  }
  else if (progress.value() <= 9)
  {
    pawns = 5;
  }
  else if (progress.value() <= 10)
  {
    pawns = 6;
  }
  else
  {
    pawns = 7;
  }
  const int width = eval_t::captureValue(newPtypeO(alt(P),PAWN))*pawns/2;
  return last_value - width - eval::delta(P);
}

template <class EvalT>
int 
osl::search::AlphaBeta2Tree<EvalT>::
stableThreshold(Player P, int last_value)
{
  int pawns = 3;
  if (eval::betterThan(P, last_value, eval_t::captureValue(newPtypeO(alt(P),KING))))
    pawns = 10;
  else if (eval::betterThan(P, eval_t::captureValue(newPtypeO(alt(P),PAWN))*2, last_value)
	   && eval::betterThan(P, last_value, eval_t::captureValue(newPtypeO(P,PAWN))*2))
    pawns = 2;
  const int width = eval_t::captureValue(newPtypeO(alt(P),PAWN))*pawns/2;
  return last_value - width - eval::delta(P);
}

namespace osl
{
  namespace
  {
    void find_threatmate(const SimpleHashTable& table, HashKey key,
			 const search::SearchState2::PVVector& pv,
			 CArray<bool, search::SearchState2::MaxDepth>& threatmate)
    {
      for (size_t i=0; i<pv.size(); ++i) {
	key = key.newMakeMove(pv[i]);
	const SimpleHashRecord *record = table.find(key);
	threatmate[i] = record
	  && record->threatmate().isThreatmate(key.turn());
      }
    }
  }
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::
updateRootPV(Player P, std::ostream& os, int result, Move m)
{
  std::lock_guard<std::mutex> lk(OslConfig::lock_io);
  this->makePV(m);
  const int last_root_value = shared_root->root_values_for_iteration.size() ? shared_root->root_values_for_iteration.back() : 0;
  const int threshold = stableThreshold(P, last_root_value);
  bool new_stable = eval::betterThan(P, result, threshold);
  shared_root->last_root_value_update = result;

  if (new_stable && m != shared_root->last_root_move
      && (See::see(state(), m) < -eval::Ptype_Eval_Table.value(KNIGHT)*2
	  || eval::betterThan(P, result, eval_t::captureValue(newPtypeO(alt(P),KING))))) {
    new_stable = false;
  }
  if (new_stable && shared_root->root_values_for_iteration.size() > 1) {
    const int last_root_value2 = shared_root->root_values_for_iteration[shared_root->root_values_for_iteration.size()-2];
    const int threshold2 = stableThreshold(P, last_root_value2);
    if (eval::betterThan(P, threshold2, result)
	&& eval::betterThan(P, last_root_value2, last_root_value))
      new_stable = false;
  }
  this->shared_root->last_pv.push_back(RootPV(root_limit, pv[0], result));
  this->setStable(new_stable);
#ifndef GPSONE
  if (this->hasMonitor() && !this->prediction_for_speculative_search) {
    const double scale = OslConfig::usiOutputPawnValue()*2.0
      / eval_t::captureValue(newPtypeO(alt(P),PAWN));
    CArray<bool, MaxDepth> threatmate = {{ 0 }};
    find_threatmate(*this->table, currentHash(), pv[0], threatmate);
    for (const auto& monitor: this->monitors())
      monitor->showPV(root_limit/200, this->recorder.allNodeCount(),
		      this->elapsed(), static_cast<int>(result*scale), 
		      m, &*pv[0].begin(), &*pv[0].end(),
		      &threatmate[0], &threatmate[0]+pv[0].size());
  }
#endif
  if (this->table->isVerbose()) {
    showPV(os, result, m, new_stable ? ' ' : '*');
  }
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::
addMultiPV(Player P, int result, Move m)
{
  std::lock_guard<std::mutex> lk(OslConfig::lock_io);
  this->makePV(m);
  this->shared_root->last_pv.push_back(RootPV(root_limit, pv[0], result));
  std::swap(*this->shared_root->last_pv.rbegin(), *(this->shared_root->last_pv.rbegin()+1));

  if (this->hasMonitor() && !this->prediction_for_speculative_search) {
    const double scale = OslConfig::usiOutputPawnValue()*2.0
      / eval_t::captureValue(newPtypeO(alt(P),PAWN));
    CArray<bool, MaxDepth> threatmate = {{ 0 }};
    find_threatmate(*this->table, currentHash(), pv[0], threatmate);
    for (const auto& monitor:this->monitors())
      monitor->showPV(root_limit/200, this->recorder.allNodeCount(),
		      this->elapsed(), static_cast<int>(result*scale), 
		      m, &*pv[0].begin(), &*pv[0].end(),
		      &threatmate[0], &threatmate[0]+pv[0].size());
  }

  if (this->table->isVerbose()) {
    showPV(std::cerr, result, m, '&');
  }
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::
showFailLow(int result, Move m) const
{
  if (this->root_ignore_moves)
    std::cerr << "[" << this->root_ignore_moves->size() << "] ";
  std::cerr << " <" << std::setfill(' ') << std::setw(5) 
	    << static_cast<int>(result*200.0/this->eval.captureValue(newPtypeO(WHITE,PAWN)))
	    << " " << csa::show(m) << "\n";
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::
showPV(std::ostream& os, int result, Move m, char stable_char) const
{
  assert(m.isNormal()); // do not pass at root
  if (this->root_ignore_moves)
    os << "[" << this->root_ignore_moves->size() << "] ";
  os << stable_char;
  os << " " << std::setfill(' ') << std::setw(5) 
     << static_cast<int>(result*200.0/this->eval.captureValue(newPtypeO(WHITE,PAWN))) << " ";
  for (Move m: pv[0]) {
    os << csa::show(m);
  }
  const double elapsed = this->elapsed();
  if (elapsed > 1.0)
    os << " (" << elapsed << "s, " << OslConfig::memoryUseRatio()*100.0 << "%)";
  os << std::endl;
#ifndef MINIMAL
#ifndef _WIN32
  if (! OslConfig::usiMode())
  {
    NumEffectState state = this->state();
    std::string str; str.reserve(200); str = "        ";
    for (size_t i=0; i<pv[0].size(); ++i) {
      str += ki2::show(pv[0][i], state, i ? pv[0][i-1] : Move());
      state.makeMove(pv[0][i]);

      // threatmate?
      const SimpleHashRecord *record
	= this->table->find(HashKey(state));
      if (record && 
	  record->threatmate().isThreatmate(state.turn()))
	str += "(" K_TSUMERO ")";
    }
    std::string converted = misc::eucToLang(str);
    if (! converted.empty())
      os << converted << std::endl;
  }
#endif
#endif

#ifdef DEBUG_PV
  NumEffectState s = state();
  for (size_t i=0; i<pv[0].size(); ++i) {
    if (! pv[0][i].isPass() && ! s.isValidMove(pv[0][i])) {
      std::cerr << "root pv error " << pv[0][i] << " " << i << "\n";
      break;
    }
    ApplyMoveOfTurn::doMove(s, pv[0][i]);
  }
#endif
}

template <class EvalT>
template <osl::Player P>
struct osl::search::AlphaBeta2Tree<EvalT>::NextMove
{
  AlphaBeta2Tree *searcher;
  const MoveLogProb& moved;
  Window window;
  int *result;
  bool in_pv;
  NextMove(AlphaBeta2Tree *s, const MoveLogProb& md, Window w, int *r,
	   bool p)
    : searcher(s), moved(md), window(w), result(r), in_pv(p) {
    assert(P == md.player());
  }
  void operator()(Square /*last_to*/) {
#ifndef NDEBUG
    const int cur_limit = searcher->curLimit();
#endif
    *result = 
      searcher->alphaBetaSearchAfterMove<P>(moved, window, in_pv);
    assert(cur_limit == searcher->curLimit() || searcher->SearchState2Core::abort());
  }
};

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2Tree<EvalT>::
alphaBetaSearch(const MoveLogProb& search_move, Window w, bool in_pv)
{
  assert(w.alpha(P) % 2);
  assert(w.beta(P) % 2);
  const Move move = search_move.move();
  assert(P == move.player());
  assert(P == state().turn());
  assert(eval::notLessThan(P, w.beta(P), w.alpha(P)));

  testStop();
  pv[curDepth()+1].clear();
  // TODO: more efficient way to find foul
  if (! move.isPass() ){
    if(MoveStackRejections::probe<P>(state(),history(),curDepth(),move,w.alpha(P),repetitionCounter().checkCount(alt(P)))){
      return this->winByLoop(alt(P));
    }
    if (move_classifier::MoveAdaptor<move_classifier::PawnDropCheckmate<P> >
	::isMember(state(), move))
      return this->winByFoul(alt(P));
  }

  const HashKey new_hash = currentHash().newHashWithMove(move);
  assert(P == move.player());

  if (move.isPass())
    this->pass_count.inc(P);

  // 千日手確認
  if (! this->pass_count.loopByBothPass()) {
    const Sennichite next_sennichite
      = repetition_counter.isAlmostSennichite(new_hash);
    if (next_sennichite.isDraw())
      return this->drawValue();
    if (next_sennichite.hasWinner())
      return this->winByFoul(next_sennichite.winner());
    assert(next_sennichite.isNormal());
  }
  
  if (! move.isPass()) {
    // 優越関係確認
    const DominanceCheck::Result has_dominance
      = DominanceCheck::detect(repetition_counter.history(), new_hash);
    if (has_dominance == DominanceCheck::LOSE)
      return this->winByLoop(alt(P));
    if (has_dominance == DominanceCheck::WIN)
      return this->winByLoop(P);
    // 連続王手駒捨て
    if (! move.isCapture()) {
      const int sacrifice_count = countSacrificeCheck2(this->curDepth());
      if (sacrifice_count == 2) {
	// 3回目は指さない
	const Square to = move.to();
	int offence = state().countEffect(P, to) + (move.isDrop() ? 1 : 0);
	const int deffense = state().hasEffectAt(alt(P), to); // max1
	if (offence <= deffense)
	  offence += AdditionalEffect::count2(state(), to, P);
	if (offence <= deffense) {
	  return this->winByLoop(alt(P));
	}
      }
    }
  }
  // 探索
  int result;
  NextMove<P> helper(this, search_move, w, &result, in_pv);

  this->recorder.addNodeCount();  
  const eval_t old_eval = this->eval;
  doUndoMoveOrPass<P,NextMove<P> >(new_hash, move, helper);
  this->eval = old_eval;
  if (move.isPass())
    this->pass_count.dec(P);

  return result;
}

template <class EvalT>
template <osl::Player P>
void osl::search::AlphaBeta2Tree<EvalT>::
examineMovesRoot(const MoveLogProbVector& moves, size_t i, Window window,
		 MoveLogProb& best_move, int& best_value)
{
  for (;i<moves.size(); ++i) {
    testStop();

#if (defined OSL_SMP) && (! defined OSL_SMP_NO_SPLIT_ROOT)
    if (shared && i > 8
	&& moves.size() > i+1) {
      int smp_idle;
      {
#  ifdef OSL_USE_RACE_DETECTOR
	std::lock_guard<std::mutex> lk(shared->lock_smp);
#  endif
	smp_idle = shared->smp_idle;
      }
      if (smp_idle) {
	try {
	  examineMovesRootPar<P>(moves, i, window, best_move, best_value);
	  break;
	} catch (AlphaBeta2ParallelCommon::SplitFailed&) {
	}
      }
    }
#endif

    const MoveLogProb& m = moves[i];
#ifndef GPSONE
    if (this->elapsed() > 1.0)
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      for (const auto& monitor:this->monitors())
	monitor->rootMove(m.move());
    }
    if (this->multi_pv) {
      int width = this->multi_pv*this->eval.captureValue(newPtypeO(P, PAWN))/200;
      if (width % 2 == 0) 
	width -= EvalTraits<P>::delta;
      window.alpha(P) = best_value + width;
    }
#endif
    const int result = alphaBetaSearch<P>(m, window, false);
    if (eval::betterThan(P, result, best_value)) 
    {
      window.alpha(P) = result + EvalTraits<P>::delta;
      best_move = m;
      best_value = result;
      updateRootPV(P, std::cerr, result, m.move());
      if (eval::betterThan(P, result, window.beta(P))) {
	assert(! this->isWinValue(alt(P), result));
	break;
      }
    } 
#ifndef GPSONE
    else if (this->multi_pv && eval::betterThan(P, result, window.alpha(P)))
    {
      addMultiPV(P, result, m.move());
    }
#endif
    if (this->root_limit >= 1600)
      this->checkmate_searcher->runGC(this->table->isVerbose(),
				      lastMemoryUseRatio1000());
  }
}

/* ------------------------------------------------------------------------- */

template <class EvalT>
osl::search::AlphaBeta2<EvalT>::
AlphaBeta2(const NumEffectState& s, checkmate_t& c, 
	   SimpleHashTable *t, CountRecorder& r)
  : AlphaBeta2Tree<EvalT>(s, c, t, r)
{
  MoveGenerator::initOnce();
}

template <class EvalT>
osl::search::AlphaBeta2<EvalT>::
~AlphaBeta2()
{
}

template <class EvalT>
typename osl::search::AlphaBeta2<EvalT>::PVCheckmateStatus osl::search::AlphaBeta2<EvalT>::
findCheckmateInPV(int verify_node, CArray<bool,2>& king_in_threat)
{
  king_in_threat.fill(false);
  if (this->shared_root->last_pv.empty())
    return PVStable;
  const SearchState2::PVVector& pv = this->shared_root->last_pv.back().pv;
  NumEffectState state = this->state();
  PathEncoding path = this->path();
  PVCheckmateStatus found = PVStable;
  SearchState2::checkmate_t *checkmate_searcher = this->checkmate_searcher;
  if (this->node_count < verify_node*pv.size())
    verify_node = this->node_count/(pv.size()+1)/4;
  for (size_t i=0; i<pv.size(); ++i)
  {
    this->checkmate_searcher->runGC(this->table->isVerbose(),
				    this->lastMemoryUseRatio1000());
    assert(pv[i].isPass() || state.isValidMove(pv[i]));
    if (! pv[i].isPass() && ! state.isValidMove(pv[i]))
    {
      std::cerr << "pv error " << pv[i] << "\n" << state;
      return PVStable;
    }
    state.makeMove(pv[i]);
    path.pushMove(pv[i]);
    if (state.inCheck())
      continue;
    const HashKey key(state);
    SimpleHashRecord *record = this->table->allocate(key, 2000);
    if (! record)
      break;
    Move checkmate_move, threatmate_move;
    const bool old_win = this->isWinningState
      (*checkmate_searcher, state, key, path, 
       0, checkmate_move, pv[i]);
    if (! old_win) 
    {
      const bool new_win = this->isWinningState
	(*checkmate_searcher, state, key, path, 
	 verify_node, checkmate_move, pv[i], true);
      if (new_win)
      {
	found = PVCheckmate;
	this->recordWinByCheckmate(state.turn(), record, checkmate_move);
	king_in_threat[alt(state.turn())] = true;
	if (this->table->isVerbose())
	  std::cerr << "  pv checkmate " << csa::show(pv[i])
		    << "(" << i << ")\n";
      }
    }
    state.changeTurn();
    const Player T = state.turn();
    const bool old_threatmate_in_record = record->threatmate().isThreatmate(alt(T));
    const bool old_threatmate = this->isWinningState
      (*checkmate_searcher, state, HashKey(state), PathEncoding(T), 
       1, threatmate_move, Move::PASS(alt(T)));
    if (! old_threatmate)
    {
      const bool new_threatmate = this->isWinningState
	(*checkmate_searcher, state, HashKey(state), PathEncoding(T), 
	 verify_node, threatmate_move, Move::PASS(alt(T)), this->root_limit >= 1000 + this->rootLimitBias());
      if (new_threatmate)
      {
	record->threatmate().setThreatmate(alt(T), threatmate_move);
	king_in_threat[alt(T)] = true;
	if (! old_threatmate_in_record) 
	  found = PVThreatmate;
	else if (found == PVStable) 
	  found = PVThreatmateNotRecord;
	if (this->table->isVerbose())
	  std::cerr << "  pv threatmate " << csa::show(pv[i])
		    << "(" << i << ")\n";
      }
    }
    state.changeTurn();
  }
  this->checkmate_searcher->runGC(this->table->isVerbose(),
				  this->lastMemoryUseRatio1000());
  this->updateCheckmateCount();
  return found;
}

template <class EvalT>
int osl::search::AlphaBeta2<EvalT>::
alphaBetaSearchRoot(MoveLogProb& best_move, int limit)
{
  const Player Turn = this->state().turn();
  Window root_window = this->fullWindow(Turn);
  return alphaBetaSearchRoot(root_window, best_move, limit);
}

template <class EvalT>
osl::Move osl::search::AlphaBeta2<EvalT>::
computeBestMoveIteratively(int limit, const int step, 
			   int initial_limit, size_t node_limit,
			   const TimeAssigned& assign,
			   MoveWithComment *additional_info)
{
  this->setStartTime(clock::now());
  this->setTimeAssign(assign);
  if (this->table->verboseLevel() > 2)
  {
    const time_t now = time(0);
    char ctime_buf[64];
    std::cerr << "AlphaBeta2 " << ctime_r(&now, ctime_buf);
  }
  if (this->table->isVerbose()) {
    std::cerr << " time assign/max " << toSeconds(this->timeAssigned().standard)
	      << "/" << toSeconds(this->timeAssigned().max)
	      << " multipv " << this->multi_pv
	      << " iteration " << this->nextIterationCoefficient()
	      << " mem " << std::fixed << std::setprecision(2)
	      << OslConfig::memoryUseRatio()*100.0 << "%";
    std::cerr << "\n";
  }
  initial_limit = std::min(initial_limit, limit);
  
  this->recorder.resetNodeCount();

  double last_iteration_consumed = 0;
  double total_consumed = 0;
  int limit_iterative = initial_limit;
  Move last_best_move = Move::INVALID();
  this->shared_root->last_pv.clear();

#ifdef OSL_SMP
#  ifdef SPLIT_STAT
  if (this->shared) {
    this->shared->parallel_splits = 0;
    this->shared->cancelled_splits.setValue(0);
    this->shared->parallel_abort.setValue(0);
  }
#  endif
#endif
  try
  {
    if (this->table->verboseLevel() > 1)
    {
      MoveVector moves;
      this->state().generateLegal(moves);
      for (Move move: moves) {
	HashKey key = this->currentHash().newHashWithMove(move);
	const SimpleHashRecord *record = this->table->find(key);
	if (! record || record->lowerLimit() < SearchTable::HistorySpecialDepth)
	  continue;
	std::cerr << "prebound value " << csa::show(move)
		  << " " << record->lowerBound() << " " << record->upperBound() << "\n";
      }
    }

    MoveLogProb search_move;
    this->shared_root->root_values.push_back(alphaBetaSearchRoot(search_move, 0));
    this->shared_root->last_root_move = search_move.move();
    this->shared_root->best_move_for_iteration.push_back(search_move.move());
    if (this->table->verboseLevel() > 1)
      std::cerr << "=> quiesce "
		<< csa::show(search_move.move()) << "\n";
    while (limit_iterative < limit && ! this->stopping())
    {
      if (this->table->verboseLevel() > 1)
	std::cerr << "=> iteration " << limit_iterative 
		  << " (" << last_iteration_consumed << ", " << total_consumed << " sec)"
		  << " mem " << OslConfig::memoryUseRatio()*100.0 << "%\n";
      this->recorder.startSearch(limit_iterative);
      const int previous_node_count = this->nodeCount();
      try {
	for (int i=0; i<8; ++i)
	{
	  this->shared_root->root_values.push_back(alphaBetaSearchRoot(search_move, limit_iterative+this->rootLimitBias()));
	  this->shared_root->last_root_move = search_move.move();
	  last_best_move = search_move.move();
	  if (this->stopping())
	    break;
	  PVCheckmateStatus need_more_verify = PVStable;
	  CArray<bool, 2> king_in_threat;
	  int verify_node_limit = limit <= (1200 + this->rootLimitBias()) ? 10000 : 40000;
	  if (toSeconds(this->timeAssigned().standard) < 20)
	    verify_node_limit /= 4;
#ifdef DONT_USE_CHECKMATE
	  break;
#endif
	  need_more_verify = findCheckmateInPV(verify_node_limit, king_in_threat);
	  if (need_more_verify == PVStable
	      || (i > 0 && need_more_verify == PVThreatmateNotRecord))
	    break;
	  if (this->isStableNow())
	    this->setStable(i > 0 && king_in_threat[this->state().turn()] == false);
	} 
      } catch (...) {
	last_iteration_consumed = this->elapsed() - total_consumed;
	total_consumed += last_iteration_consumed;
	this->updateCheckmateCount();
	this->recorder.finishSearch(search_move.move(), total_consumed,
			      this->table->verboseLevel());
	throw;
      }

      last_iteration_consumed = this->elapsed() - total_consumed;
      total_consumed += last_iteration_consumed;
      this->shared_root->best_move_for_iteration.push_back(last_best_move);
      this->shared_root->root_values_for_iteration.push_back
	(this->shared_root->root_values.back());

      this->updateCheckmateCount();
      if (this->table->verboseLevel() > 2) {
	std::cerr << "<= " 
		  << csa::show(search_move.move());
	std::cerr << std::setprecision(4) << "  mpn " << this->mpn.average() 
		  << " cut " << this->mpn_cut.average()
		  << " alpha " << this->alpha_update.average()
		  << " last " << this->last_alpha_update.average()
		  << " ext " << 100.0*this->ext.average() << "%"
		  << " ext_limit " << this->ext_limit.average()
		  << " mem " << OslConfig::memoryUseRatio()*100.0;
#ifdef OSL_SMP
#  ifdef SPLIT_STAT
	if (this->shared) {
	  std::cerr << " split " << this->shared->parallel_splits << " cancel " << this->shared->cancelled_splits.value() 
		    << " abort " << this->shared->parallel_abort.value();
	}
#  endif
#endif
	std::cerr << "\n";
      }
      bool time_over = false;
      if (this->hasSchedule()) {
	const double elapsed = this->elapsed();
	const double current_time_left = toSeconds(this->timeAssigned().standard) - elapsed;
	double coef = this->nextIterationCoefficient();
	if (! this->isStableNow())
	  coef = std::min(0.5, coef);
	else {
	  const int same_best_moves = this->shared_root->sameBestMoves();
	  if (same_best_moves == 0) {
	    if (this->table->verboseLevel() > 2 && coef > 0.75)
	      std::cerr << "info: " << coef << " -> 0.75 by bestmove update\n";
	    coef = std::min(0.75, coef);
	  }
	  else if (same_best_moves >= 3) {
	    const Move last_move = this->lastMove();
	    if (last_move.isNormal() && last_best_move.isNormal()
		&& last_move.to() == last_best_move.to()
		&& isMajor(last_best_move.capturePtype())
		&& isMajorNonPieceOK(last_move.capturePtype())) {
	      if (coef < 5.0 && this->table->verboseLevel() > 2)
		std::cerr << "info: " << coef << " -> 5.0 by takeback major piece\n";
	      coef = std::max(5.0, coef);
	    }
	  }
	}
	if (current_time_left 
	    < last_iteration_consumed * coef)
	  time_over = true;
	if (! time_over) {
	  SimpleHashRecord *record
	    = this->table->find(this->currentHash());
	  if (record) {
	    record->addNodeCount(this->nodeCount() - previous_node_count);
	  }
	}
      }
      bool node_limit_over = (this->recorder.nodeCount() *4 > node_limit);
      this->recorder.finishSearch(search_move.move(), 
			    total_consumed,
			    (time_over || node_limit_over) && this->table->verboseLevel());
      if (time_over || node_limit_over || this->stopping()) {
	if (this->table->isVerbose()) {
	  const char *reason = "other reason";
	  if (this->stopReason() == SearchTimerCommon::NoMoreMemory)
	    reason = "memory full";
	  else if (time_over || this->stopReason() == SearchTimerCommon::NoMoreTime)
	    reason = "time";
	  else if (node_limit_over)
	    reason = "node count";
	  else if (this->stopReason() == SearchTimerCommon::StopByOutside)
	    reason = "outside";
	  std::cerr << "iteration stop at " << limit_iterative << " by "
		    << reason << "\n";
	}
	goto finish;
      }
      this->testStop();
      limit_iterative += step;
    }
    if (this->table->verboseLevel() > 1)
      std::cerr << "=> final iteration " << limit_iterative 
		<< " (" << last_iteration_consumed << ", " << total_consumed << " sec)"
		<< " mem " << OslConfig::memoryUseRatio()*100.0 << "%\n";
    while (true) {
      this->recorder.startSearch(limit);
      try {
	for (int i=0; i<8; ++i)
	{
	  this->shared_root->root_values.push_back(alphaBetaSearchRoot(search_move, limit+this->rootLimitBias()));
	  this->shared_root->last_root_move = search_move.move();
	  last_best_move = search_move.move();
	  if (this->stopping())
	    break;
	  PVCheckmateStatus need_more_verify = PVStable;
	  CArray<bool, 2> king_in_threat;
	  int verify_node_limit = limit <= (1200 + this->rootLimitBias()) ? 10000 : 40000;
	  if (toSeconds(this->timeAssigned().standard) < 20)
	    verify_node_limit /= 4;
#ifdef DONT_USE_CHECKMATE
	  break;
#endif
	  need_more_verify = findCheckmateInPV(verify_node_limit, king_in_threat);
	  if (need_more_verify == PVStable
	      || (i > 0 && need_more_verify == PVThreatmateNotRecord))
	    break;
	  if (this->isStableNow())
	    this->setStable(i > 0 && king_in_threat[this->state().turn()] == false);
	}
      } catch (...) {
	last_iteration_consumed = this->elapsed() - total_consumed;
	total_consumed += last_iteration_consumed;
	this->updateCheckmateCount();
	this->recorder.finishSearch(search_move.move(), total_consumed,
			      this->table->verboseLevel());
	throw;
      }
      last_iteration_consumed = this->elapsed() - total_consumed;
      total_consumed += last_iteration_consumed;
      this->updateCheckmateCount();
      this->recorder.finishSearch(search_move.move(), total_consumed,
			    this->table->verboseLevel());
      this->shared_root->best_move_for_iteration.push_back(last_best_move);
      this->shared_root->root_values_for_iteration.push_back
	(this->shared_root->root_values.back());

      if (last_best_move.isNormal())
	break;
      this->testStop();

      // ほっておくと投了
      if (limit >= 2000 || this->root_ignore_moves)
	break;

      limit += 200;
      if (this->table->isVerbose())
	std::cerr << "  extend limit to " << limit << " before resign\n";
    }
  }
  catch (std::exception& e)
  {
    if (! OslConfig::usiMode())
      std::cerr << "std exception " << e.what() << "\n";
  }
  catch (...)
  {
    std::cerr << "unknown exception\n";
#ifndef NDEBUG
    throw;
#endif
  }
finish:
  if (this->table->verboseLevel() > 1) {
    std::cerr << "<= " << csa::show(last_best_move);
    std::cerr << std::setprecision(4) << "  mpn " << this->mpn.average()
	      << " cut " << this->mpn_cut.average()
	      << " alpha " << this->alpha_update.average()
	      << " last " << this->last_alpha_update.average()
	      << " ext " << this->ext.average()
	      << " ext_limit " << this->ext_limit.average()
	      << " mem " << OslConfig::memoryUseRatio()*100.0;
#ifdef OSL_SMP
#  ifdef SPLIT_STAT
    if (this->shared) {
      std::cerr << " split " << this->shared->parallel_splits << " cancel " << this->shared->cancelled_splits.value() 
		<< " abort " << this->shared->parallel_abort.value();
    }
#  endif
#endif
    std::cerr << "\n";
  }

  if (additional_info) {
    additional_info->node_count = this->nodeCount();
    additional_info->elapsed = this->elapsed();
    additional_info->moves.clear();
    additional_info->root_limit = this->root_limit;
  }
  if (additional_info && this->shared_root->root_values.size() > 1) { // last_root_value[0] is for quiesce
    assert(last_best_move == this->shared_root->last_root_move);
    additional_info->move = last_best_move;
    const double scale = 200.0/this->eval.captureValue(newPtypeO(WHITE,PAWN));
    additional_info->value = static_cast<int>(this->shared_root->last_root_value_update * scale);
    if (!this->shared_root->last_pv.empty()) {
      for (size_t i=1; i<this->shared_root->last_pv.back().pv.size(); ++i) {
	additional_info->moves.push_back(this->shared_root->last_pv.back().pv[i]);
      }
    }
  }
#ifndef GPSONE
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    for (const auto& monitor:this->monitors())
      monitor->searchFinished();
  }
#endif
  return last_best_move;
}

template <class EvalT>
template <osl::Player P>
int osl::search::AlphaBeta2<EvalT>::
alphaBetaSearchRoot(Window window, MoveLogProb& best_move, int limit)
{
#ifndef GPSONE
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    for (const auto& monitor:this->monitors())
      monitor->newDepth(limit/200);
  }
#endif
  assert(P == this->state().turn());
  assert(window.alpha(P) % 2);
  assert(window.beta(P) % 2);
  setRoot(limit);
  assert(this->curDepth() == 0);
  this->node_type[this->curDepth()] = base_t::PvNode;
  this->checkmate_searcher->setRootPlayer(P);
#ifdef OSL_SMP
  if (this->shared)
    this->shared->threadStart();
#endif
  // まずテーブルを牽く
  SimpleHashRecord *record_in_table
    = this->table->allocate(this->currentHash(), limit);
  SimpleHashRecord *record = record_in_table;
  std::unique_ptr<SimpleHashRecord> record_if_not_allocated;
  if (! record)
  {
    record_if_not_allocated.reset(new SimpleHashRecord());
    record = record_if_not_allocated.get();
  }
  assert(record);
  this->setRootRecord(record);
  assert(this->rootRecord() == record);
  assert(this->hasLastRecord() && this->lastRecord() == record);
  record->setInCheck(this->state().inCheck());

  if (limit == 0) {
    int result = this->template quiesce<P>(fullWindow(P));
    best_move = MoveLogProb(record->qrecord.bestMove(), 100);
    if (this->root_ignore_moves
	&& this->root_ignore_moves->isMember(best_move.move()))
      best_move = MoveLogProb();
#ifndef GPSONE
    else if (this->hasMonitor() && !this->prediction_for_speculative_search) 
    {
      const double scale = OslConfig::usiOutputPawnValue()*2.0
	/ this->eval.captureValue(newPtypeO(alt(P),PAWN));
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      for (const auto& monitor:this->monitors())
	monitor->showPV(1, this->recorder.allNodeCount(),
			this->elapsed(), static_cast<int>(result*scale), 
			best_move.move(), 0, 0, 0, 0);
    }
#endif
    return result;
  }
  if (record_in_table) {
    int table_value = 0;
    const MoveLogProb m = record_in_table->bestMove();
    if (! m.isNormal())
      record_in_table->resetValue();
    else if (record->hasGreaterLowerBound<P>(this->curLimit(), window.beta(P), 
					     table_value)) {
      if (! this->root_ignore_moves 
	  || ! this->root_ignore_moves->isMember(m.move())) {
	best_move = m;
	return table_value;
      }
    }
  }

  // gather all moves
  MoveLogProbVector moves;
  MoveGenerator& generator = this->makeGenerator();
  const MoveLogProb last_best_move = record->bestMove();
  {
    MoveLogProbVector raw_moves;
    assert(this->curLimit() > 0);
    const Move hash_move = last_best_move.isNormal()
      ? last_best_move.move() : record->qrecord.bestMove();
    generator.init(this->curLimit()+200, record, this->eval, this->state(), true, hash_move);
    if (last_best_move.isNormal())
      raw_moves.push_back(last_best_move);
    else if (record->qrecord.bestMove().isNormal())
      raw_moves.push_back(MoveLogProb(record->qrecord.bestMove(), 100));
    generator.generateAll<P>(*this, raw_moves);

    // clean up losing moves
    for (size_t i=0; i<raw_moves.size(); ++i) {
      const Move m = raw_moves[i].move();
      if (i > 0 && m == hash_move)
	continue;
      const HashKey key = this->currentHash().newHashWithMove(m);
      const SimpleHashRecord *record = this->table->find(key);
      assert(this->state().isValidMove(m));
      if (record) {
	if (record->hasUpperBound(SearchTable::HistorySpecialDepth)
	    && this->isWinValue(alt(P), record->upperBound()))
	  continue;
      }
      if (this->root_ignore_moves && this->root_ignore_moves->isMember(m))
	continue;
      if (! m.isDrop() && m.ptype() != KING
	  && move_classifier::KingOpenMove<P>::isMember(this->state(), m.ptype(), m.from(), m.to()))
	continue;
      if (move_classifier::MoveAdaptor<move_classifier::PawnDropCheckmate<P> >
	  ::isMember(this->state(), m))
        continue;
      raw_moves[i].setLogProbAtMost(limit);
      moves.push_back(raw_moves[i]);
    }
  }

  if (! OslConfig::searchExactValueInOneReply()) {
    if (moves.size() == 1 
	|| (moves.size() == 2 && moves[0].move() == moves[1].move()))
    {
      best_move = moves[0];
#ifndef GPSONE
      if (this->hasMonitor() && !this->prediction_for_speculative_search) {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	for (const auto& monitor:this->monitors())
	  monitor->rootForcedMove(best_move.move());
      }
#endif
      return 0;	// XXX
    }
  }

#ifndef DONT_USE_CHECKMATE
  // 詰将棋を呼んでみる root では沢山呼んでも問題ない
  int checkmate_node = 0;
  if (! this->prediction_for_speculative_search) {
    int checkmate_max = 30000*std::max(limit - 300 - this->rootLimitBias(), 0)/100;
    if (limit >= 1000 + this->rootLimitBias())
      checkmate_max = std::min(400000, 60000*(limit - 800 - this->rootLimitBias())/100);
    if (toSeconds(this->timeAssigned().standard) < 20) {
      checkmate_node /= 4;
      if (toSeconds(this->timeAssigned().standard) < 10)
	checkmate_node /= 2;
    }
    checkmate_node = record->qrecord.checkmateNodesLeft(checkmate_max);
#ifdef CHECKMATE_COUNT
    std::cerr << "limit " << limit << " checkmate " << checkmate_node << "\n";
#endif
  }
  if (checkmate_node > 0)
  {
    const bool my_king_in_check
      = this->state().hasEffectAt(alt(P),this->state().kingSquare(P));
    if (my_king_in_check)
    {
      // 相手から王手がかかっている
      this->recorder.gotoCheckmateSearch(this->state(), checkmate_node/8);
      const bool lose = this->template isLosingState<P>(checkmate_node/8);
      this->recorder.backFromCheckmateSearch();
      this->updateCheckmateCount();
      if (lose)
      {
	best_move = MoveLogProb(Move::INVALID(),100);
	this->recordLoseByCheckmate(P, record);
	this->shared_root->last_pv.clear();
	this->shared_root->last_root_move = Move();
	this->shared_root->last_root_value_update = this->winByCheckmate(alt(P));
#ifndef GPSONE
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	for (const auto& monitor:this->monitors())
	  monitor->rootLossByCheckmate();
#endif
	return this->winByCheckmate(alt(P));
      }
    }
    // 詰まされなければ，相手を詰ますことを考える 
    {
      Move checkmate_move;
#ifdef CHECKMATE_COUNT
      size_t count = this->checkmateSearcher().totalNodeCount();
#endif
      this->recorder.gotoCheckmateSearch(this->state(), checkmate_node);
      const bool win = this->template isWinningState<P>
	(checkmate_node, checkmate_move, limit >= 1000 + this->rootLimitBias());
      this->recorder.backFromCheckmateSearch();
      this->updateCheckmateCount();
#ifdef CHECKMATE_COUNT
      root_checkmate += this->checkmateSearcher().totalNodeCount() - count;
#endif
      if (win)
      {
	best_move = MoveLogProb(checkmate_move,100);
	this->recordWinByCheckmate(P, record, checkmate_move);
	this->shared_root->last_pv.clear();
	this->shared_root->last_root_move = checkmate_move;
	this->shared_root->last_root_value_update = this->winByCheckmate(P);
	this->pv[1].clear();
	this->updateRootPV(P, std::cerr, this->winByCheckmate(P), checkmate_move);
	return this->winByCheckmate(P);
      }
    }
    // 詰めろを考える
    if ((! my_king_in_check)
	&& (! (record->threatmate().isThreatmate(P))))
    {
      Move threatmate_move;
#ifdef CHECKMATE_COUNT
      size_t count = this->checkmateSearcher().totalNodeCount();
#endif
      this->recorder.gotoCheckmateSearch(this->state(), checkmate_node);
      const bool threatmate 
	= this->template isThreatmateState<P>
	(checkmate_node, threatmate_move, limit >= 1000 + this->rootLimitBias());
#ifdef CHECKMATE_COUNT
      root_checkmate += this->checkmateSearcher().totalNodeCount() - count;
#endif
      this->recorder.backFromCheckmateSearch();
      this->updateCheckmateCount();
      if (threatmate)
      {
	if (record)
	  record->threatmate().setThreatmate(P, threatmate_move);
	if (this->table->verboseLevel() > 1)
	  std::cerr << "  root threatmate " << threatmate_move << "\n";
      }
      for (Ptype ptype: PieceStand::order)
      {
	this->testStop();
	if (! this->state().hasPieceOnStand(P, ptype))
	  continue;
	NumEffectState state(this->state().emulateHandPiece(P, alt(P), ptype));
	state.setTurn(alt(P));
	Move hand_move;
	this->template isWinningState<alt(P)>
	  (*this->checkmate_searcher, state, HashKey(state), PathEncoding(alt(P)),
	   checkmate_node, hand_move, Move::PASS(P), limit >= 1000 + this->rootLimitBias());
      }
    }
    this->testStop();
  }
  this->checkmate_searcher->runGC(this->table->isVerbose(),
				  this->lastMemoryUseRatio1000());
#endif
  const int ValueNone = window.alpha(P) - EvalTraits<P>::delta;
  int best_value = ValueNone;
  try {
    // first move
    size_t i=0;
    if (limit >= 1000 && ! moves.empty() && window == fullWindow(P))
    {
      // try aspiration window if we have sufficient limit
      const int root_alpha =
	this->rootAlpha(P, this->shared_root->root_values.size() ? this->shared_root->root_values.back() : 0,
			this->eval.progress16());
      if (EvalTraits<P>::betterThan(root_alpha, window.alpha(P))) {
	const Window window_copy = window;
	window.alpha(P) = root_alpha;
#ifndef GPSONE
	{
	  std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	  for (const auto& monitor:this->monitors())
	    monitor->rootFirstMove(moves[0].move());
	}
#endif
	const int result = this->template alphaBetaSearch<P>(moves[0], window, true);
	if (EvalTraits<P>::betterThan(result, root_alpha))
	{
	  window.alpha(P) = result + EvalTraits<P>::delta;
	  best_move = moves[0];
	  best_value = result;
	  this->updateRootPV(P, std::cerr, result, moves[0].move());
	  ++i;
	} 
	else
	{
	  if (this->table->isVerbose())
	    this->showFailLow(result, moves[0].move());
#ifndef GPSONE
	  if (this->hasMonitor() && !this->prediction_for_speculative_search) {
	    const double scale = OslConfig::usiOutputPawnValue()*2.0
	      / this->eval.captureValue(newPtypeO(alt(P),PAWN));
	    for (const auto& monitor:this->monitors())
	      monitor->showFailLow(this->root_limit/200, this->recorder.allNodeCount(),
				   this->elapsed(),static_cast<int>(result*scale),
				   moves[0].move());
	  }
#endif
	  this->setStable(false);
	  window = window_copy;
	}
	this->checkmate_searcher->runGC(this->table->isVerbose(),
					this->lastMemoryUseRatio1000());
      }
    }
    for (; i<moves.size() && best_value == ValueNone
	   && window == fullWindow(P); ++i) {
      const MoveLogProb& m = moves[i];
#ifndef GPSONE
      {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	for (const auto& monitor:this->monitors())
	  monitor->rootMove(m.move());
      }
#endif
      const int result = this->template alphaBetaSearch<P>(m, window, true);
      if (eval::betterThan(P, result, best_value)) {
	window.alpha(P) = result + EvalTraits<P>::delta;
	best_move = m;
	best_value = result;
	this->updateRootPV(P, std::cerr, result, m.move());
	if (eval::betterThan(P, result, window.beta(P))) {
	  assert(! this->isWinValue(alt(P), result));
	}
      }
      else if (result == ValueNone)
	this->setStable(false);
      this->checkmate_searcher->runGC(this->table->isVerbose(),
				      this->lastMemoryUseRatio1000());
    }
    // other moves
    if (! eval::betterThan(P, window.alpha(P), window.beta(P))) {
      this->template examineMovesRoot<P>(moves, i, window, best_move, best_value);
    }
    if (best_move.isNormal()) {
      if (best_value != ValueNone) {
	assert(! this->shared_root->last_pv.empty());
	assert(best_move.move() == this->shared_root->last_pv.back().pv[0]);
      }
    }
#ifndef GPSONE
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      for (const auto& monitor:this->monitors())
	monitor->depthFinishedNormally(limit/200);
    }
#endif
  } catch (std::runtime_error& e) {
    if (this->table->isVerbose())
      std::cerr << e.what() << "\n";
    assert(best_value % 2 == 0);
    this->stopNow();
    this->restoreRootState();
    if (best_value != ValueNone)
      record->setLowerBound(P, this->curLimit(), best_move, best_value);
    if (best_move.validMove()
	&& best_move.move() != last_best_move.move()) {
      if (this->table->verboseLevel() > 1) {
	std::cerr << "! use better move than the last best move\n";
	if (best_value != ValueNone) {
	  assert(! this->shared_root->last_pv.empty() &&
		 ! this->shared_root->last_pv.back().pv.empty());
	  assert(best_move.move() == this->shared_root->last_pv.back().pv[0]);
	}
      }
    }
    else {
#ifdef OSL_SMP
      if (this->shared)
	this->shared->waitAll();
#endif      
      throw;
    }
  }
  
  assert(best_value % 2 == 0);
  if (best_value != ValueNone)
    record->setLowerBound(P, this->curLimit(), best_move, best_value);
#ifdef OSL_SMP
  if (this->shared)
    this->shared->waitAll();
#endif      
#ifndef GPSONE
  if (best_value == ValueNone
      && this->hasMonitor() && !this->prediction_for_speculative_search) 
  {
    const double scale = OslConfig::usiOutputPawnValue()*2.0
      / this->eval.captureValue(newPtypeO(alt(P),PAWN));
    const int value = this->winByCheckmate(alt(P));
    for (const auto& monitor:this->monitors())
      monitor->showPV(limit/200, this->recorder.allNodeCount(),
		      this->elapsed(), static_cast<int>(value*scale), 
		      Move::INVALID(), 0, 0, 0, 0);
  }
#endif
  return best_value;
}

template <class EvalT>
void osl::search::AlphaBeta2<EvalT>::setRoot(int limit)
{
  SearchState2::setRoot(limit);
  SimpleHashRecord *record = this->table->allocate(this->currentHash(), std::max(1000,limit));
  assert(record);
  this->setRootRecord(record);  
  this->move_type[this->curDepth()] = base_t::INITIAL;
}

template <class EvalT>
void osl::search::AlphaBeta2<EvalT>::makeMove(Move move)
{
  assert(this->state().isValidMove(move));
  SearchState2::makeMove(move);
  this->eval.update(this->state(), move);

  SimpleHashRecord *record 
    = this->table->allocate(this->currentHash(), this->curLimit());
  assert(record);
  this->move_type[this->curDepth()] = base_t::INITIAL;
  record->setInCheck(this->state().inCheck());
  this->setCurrentRecord(record);
}

template <class EvalT>
bool osl::search::AlphaBeta2<EvalT>::
isReasonableMove(Move /*move*/, int /*pawn_sacrifice*/)
{
  return true;
}

template <class EvalT>
void osl::search::AlphaBeta2<EvalT>::
showNodeDepth(std::ostream& os)
{
#ifndef MINIMAL
  int max_depth=0;
  for (int i=base_t::MaxDepth-1; i>=0; --i) {
    if (base_t::depth_node_count[i] || base_t::depth_node_count_quiesce[i]) {
      max_depth = i;
      break;
    }
  }
  int max_count=0;
  for (int i=0; i<=max_depth; i+=2) {
    max_count = std::max(max_count, 
			 base_t::depth_node_count[i]+base_t::depth_node_count_quiesce[i]);
  }

  int unit = std::max(max_count/79, 100);
  for (int i=0; i<=max_depth; i+=2) {
    os << std::setw(3) << i << " " 
       << std::string(base_t::depth_node_count[i]/unit, '*')
       << std::string(base_t::depth_node_count_quiesce[i]/unit, '+')
       << std::endl;
  }
#  ifdef CHECKMATE_COUNT
  std::cerr << "checkmate root " << root_checkmate << " quiesce " << quiesce_checkmate
	    << "\nnormal before " << checkmate_before 
	    << " after " << checkmate_after << " threatmate " << count_threatmate
	    << "\n";
# endif
#endif
}

template <class EvalT>
void osl::search::AlphaBeta2<EvalT>::
clearNodeDepth()
{
#ifndef MINIMAL
  base_t::depth_node_count.fill(0);
  base_t::depth_node_count_quiesce.fill(0);
#endif
}

namespace osl
{
  namespace search
  {
#ifndef MINIMAL
    template class AlphaBeta2<eval::ProgressEval>;
    template class AlphaBeta2Tree<eval::ProgressEval>;
    template
    void AlphaBeta2Tree<eval::ProgressEval>::examineMovesRoot<BLACK>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
    template
    void AlphaBeta2Tree<eval::ProgressEval>::examineMovesRoot<WHITE>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
#endif
    template class AlphaBeta2<eval::ml::OpenMidEndingEval>;
    template class AlphaBeta2Tree<eval::ml::OpenMidEndingEval>;
    template
    void AlphaBeta2Tree<eval::ml::OpenMidEndingEval>::examineMovesRoot<BLACK>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
    template
    void AlphaBeta2Tree<eval::ml::OpenMidEndingEval>::examineMovesRoot<WHITE>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
