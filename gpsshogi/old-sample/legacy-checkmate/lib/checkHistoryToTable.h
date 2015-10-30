/* checkHistoryToTable.h
 */
#ifndef OSL_CHECKMATE_CHECKHISTORYTOTABLE_H
#define OSL_CHECKMATE_CHECKHISTORYTOTABLE_H

#include "checkHashRecord.h"
#include "checkTableUtil.h"
#include "osl/repetitionCounter.h"
#include "osl/container/moveStack.h"

namespace osl
{
  namespace checkmate
  {
    struct CheckHistoryToTable
    {
      template <class Table>
      static void write(Table&, const RepetitionCounter&, const MoveStack&,
			const SimpleState& state, Player attack);
      template <class Table>
      static void undoWrite(Table&, const RepetitionCounter&, Player attack);
    };
  } // namespace checkmate
} // namespace osl

template <class Table>
void osl::checkmate::
CheckHistoryToTable::write(Table& table, 
			   const RepetitionCounter& counter,
			   const MoveStack& moves,
			   const SimpleState& state, Player attack)
{
  PieceStand white_stand(WHITE, state);
  CheckHashRecord *root = table.root();
  for (int i=0; i<counter.checkCount(attack); ++i)
  {
    const HashKey& key = counter.history().top(i);
    if (key != counter.history().top(0)) // ignore current state
    {
      const PathEncoding path(key.turn());

      CheckHashRecord *record = 0;
      CheckTableUtil::allocate(record, table, key, white_stand, path, root);
      if (! record->isVisited)
      {
	record->isVisited = true;
	table.incrementVisited();
      }
      if (record->findLoop(path, table.getTwinTable()) == 0)
      {
	record->setLoopDetection(path, record);
      }
    }
    assert(moves.hasLastMove(i+1)); // oops, different index
    if (! moves.hasLastMove(i+1))
      break;
    const Move last_move = moves.lastMove(i+1);
    if (last_move.isNormal())
      white_stand = white_stand.previousStand(WHITE, last_move);
  }
}

template <class Table>
void osl::checkmate::
CheckHistoryToTable::undoWrite(Table& table, 
			       const RepetitionCounter& counter,
			       Player attack)
{
  for (int i=0; i<counter.checkCount(attack); ++i)
  {
    const HashKey& key = counter.history().top(i);
    CheckHashRecord *record = table.find(key);
    assert(record);
    if (record)
      record->isVisited = false;
  }
}

#endif /* OSL_CHECKMATE_CHECKHISTORYTOTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
