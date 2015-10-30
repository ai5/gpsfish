/* searchState2.cc
 */
#include "osl/search/searchState2.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/sacrificeCheck.h"
#include "osl/csa.h"
#include "osl/misc/milliSeconds.h"
#include <iostream>

/* ------------------------------------------------------------------------- */
osl::search::
RecordStack2::RecordStack2()
{
  clear();
}

void osl::search::
RecordStack2::clear()
{
  data.clear();
  data.push_back(0);
}

#ifndef MINIMAL
void osl::search::
RecordStack2::dump() const 
{
  std::cerr << "RecordStack\n";
  for (size_t i=0; i<data.size(); ++i) {
    std::cerr << data[i];
    std::cerr << std::endl;
  }
}
#endif
/* ------------------------------------------------------------------------- */

osl::search::
SearchState2Shared::SearchState2Shared()
{
}

osl::search::
SearchState2Shared::~SearchState2Shared()
{
}


#ifndef MINIMAL
osl::CArray<int, osl::search::SearchState2Core::MaxDepth> 
osl::search::SearchState2Core::depth_node_count_quiesce;
#endif

osl::search::
SearchState2Core::SearchState2Core(const NumEffectState& s, checkmate_t& c)
  : current_state(s), checkmate_searcher(&c), 
    current_path(s.turn()), root_depth(0), 
    stop_tree(false)
{
  setState(s);			// initialize shared
  assert(hasLastRecord());
}
osl::search::
SearchState2Core::~SearchState2Core()
{
}

void osl::search::
SearchState2Core::setState(const NumEffectState& s)
{
  if (&root_state != &s)
    root_state = s;
  restoreRootState();
  try 
  {
    shared.reset();
    shared.reset(new SearchState2Shared());
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of SearchState2Shared failed\n";
  }
}

void osl::search::
SearchState2Core::restoreRootState()
{
  current_state = root_state;
  current_path = PathEncoding(current_state.turn(), move_history.size());
  repetition_counter.clear();
  const HashKey key(current_state);
  repetition_counter.push(key, current_state);
  move_history.clear();
  record_stack.clear();
  root_depth = 0;
}

void osl::search::
SearchState2Core::setHistory(const MoveStack& h)
{
  move_history = h;
  current_path = PathEncoding(current_path.turn(), h.size());
  root_depth = history().size();
}

void osl::search::
SearchState2Core::setBigramKillerMove(const BigramKillerMove& killers)
{
  try 
  {
    shared.reset();
    shared.reset(new SearchState2Shared());
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "panic. allocation of SearchState2Shared failed\n";
  }
  shared->bigram_killers = killers;
}

bool osl::search::
SearchState2Core::abort() const
{
  return abort(Move());
}

bool osl::search::
SearchState2Core::abort(Move best_move) const
{
  std::cerr << state();
#ifndef MINIMAL
  history().dump();
  const SimpleHashRecord *record = record_stack.lastRecord();
  std::cerr << "best move " << csa::show(best_move)
	    << "\n";
  std::cerr << "record " << record << "\n";
  if (record)
  {
    record->dump(std::cerr);
  }
  record_stack.dump();
  repetition_counter.history().dump();
#endif
  return false;
}

void osl::search::SearchState2Core::makePV(PVVector& parent, Move m, PVVector& pv) const
{
  parent.clear();
  parent.push_back(m);
  parent.push_back(pv.begin(), pv.end());
#ifdef DEBUG_PV
  NumEffectState s = state();
  for (Move p: parent) {
    if (! p.isPass() && ! s.isValidMove(p)) {
      std::cerr << "best move error " << p << " " << i << "\n";
      std::cerr << state();
      for (Move q: parent)
	std::cerr << q << " ";
      std::cerr << "\n";
      ::abort();
      break;
    }
    ApplyMoveOfTurn::doMove(s, p);
  }
#endif
}

#ifndef NDEBUG
void osl::search::
SearchState2Core::makeMoveHook(Move)
{
  // history().dump();
}
#endif

/* ------------------------------------------------------------------------- */

osl::search::
SearchState2::SearchState2(const NumEffectState& s, checkmate_t& c)
  : SearchState2Core(s, c), root_limit(0), cur_limit(0)
{
}

osl::search::
SearchState2::~SearchState2()
{
}

void osl::search::
SearchState2::setState(const NumEffectState& s)
{
  SearchState2Core::setState(s);
  root_limit = cur_limit = 0;
}

int osl::search::
SearchState2::countSacrificeCheck2(int history_max) const
{
  return SacrificeCheck::count2(recordHistory(), history(), history_max);
}

bool osl::search::
SearchState2::abort(Move best_move) const
{
  std::cerr << "cur limit " << cur_limit
	    << " root limit " << root_limit << "\n";
  SearchState2Core::abort(best_move);
  return false;
}

void osl::search::
SearchState2::checkPointSearchAllMoves()
{
  // debug code can be written here
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
