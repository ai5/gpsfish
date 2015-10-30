/* sacrificeCheck.h
 */
#ifndef OSL_SACRIFICECHECK_H
#define OSL_SACRIFICECHECK_H

#include "osl/search/simpleHashRecord.h"
#include "osl/container/moveStack.h"

namespace osl
{
  namespace search
  {
    struct SacrificeCheck
    {
      template<class RecordStack>
      static int count2(const RecordStack& record_stack,
			 const MoveStack& history,
			 int history_max)
      {
	int i=1;
	while (history.hasLastMove(i+1) && (i+1 <= history_max))
	{
	  // 王手回避で駒得
	  assert(record_stack.hasLastRecord(i));
	  const SimpleHashRecord *last_record = record_stack.lastRecord(i);
	  if ((! last_record) || (! last_record->inCheck()))
	    break;
	  const Move last_move = history.lastMove(i);
	  if (! last_move.isCapture())
	    break;
	  if (static_cast<int>(record_stack.size()) <= i)
	    break;
	  // ただで取られた王手
	  const Move last_last_move = history.lastMove(i+1);
	  if ((last_last_move.to() != last_move.to())
	      || (last_last_move.isCapture())
	      || (unpromote(last_last_move.ptype()) == PAWN))
	    break;
	  i+=2;
	}
	return i/2;
      }
    };
  } // namespace search
} // namespace osl

#endif /* OSL_SACRIFICECHECK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
