/* ntesukiTable.tcc
 */
#include "osl/ntesuki/ntesukiTable.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/apply_move/applyMoveWithPath.h"
#include <iterator>

using namespace osl;
using namespace osl::ntesuki;

template<class Search, class F> class
DoUndoMoveHelper
{
  Search* searcher;
  F& func;
  NumEffectState& state;
  NtesukiRecord *child;
public:
  DoUndoMoveHelper(Search* searcher,
		   F& func,
		   NumEffectState& state,
		   NtesukiRecord *child)
    : searcher(searcher), state(state), child(child)
  {
  }
	
  void operator()(Square last_to)
  {
    (*searcher).template forEachRecordFrom<F>(func, state, child);
  }
};


template <class F>
void
osl::ntesuki::NtesukiTable::Table::
forEachRecord(F& func)
{
  for (iterator it = begin(); it != end(); ++it)
  {
    for (NtesukiRecord::RecordList::iterator p = it->second.begin();
	 p != it->second.end(); ++p)
    {
      NtesukiRecord *r = &(*p);
      func(r);
    }
  }
}

template <class F>
void
osl::ntesuki::NtesukiTable::Table::
forEachRecordFrom(F& func,
		  NumEffectState& state,
		  NtesukiRecord *record)
{
  NtesukiMoveGenerator mg;
  NtesukiMoveList all_moves;
  mg.generateSlow(state.turn(), state, all_moves);

  func.enter(record);

  std::vector<NtesukiMove> moves;
  std::copy(all_moves.begin(), all_moves.end(),
	    std::back_insert_iterator<std::vector<NtesukiMove> >(moves));
  typename F::Compare c;
  std::sort(moves.begin(), moves.end(), c);
  for (std::vector<NtesukiMove>::const_iterator it = moves.begin();
       it != moves.end(); ++it)
  {
    const NtesukiMove& m = *it;
    NtesukiRecord *child = find(record->key.newHashWithMove(m.getMove()));
    if (child)
    {
      if (func.withChildMove(m, child))
      {
	DoUndoMoveHelper<Table, F> helper(func, state, child);
	ApplyMoveOfTurn::doUndoMove(state, m.getMove(), helper);
      }
    }
    else
    {
      func.noChildMove(m);
    }
  }
  func.exit();
}

template <class F>
void
osl::ntesuki::NtesukiTable::Table::
forEachRecordFromRoot(F& func)
{
  if (rootState.get() == NULL)
  {
    throw RootStateNotSet();
  }

  NumEffectState state(*rootState);
  forEachRecordFrom<F>(func, state, root);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
