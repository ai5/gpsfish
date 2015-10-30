/* dominanceCheck.h
 */
#ifndef SEARCH_DOMINANCECHECK_H
#define SEARCH_DOMINANCECHECK_H

#include "osl/hash/hashKeyStack.h"
namespace osl
{
  namespace search
  {
    struct DominanceCheck
    {
      enum Result { NORMAL=0, WIN, LOSE };
      /**
       * 駒損するループの検出.
       * @return true なら直前の指手は指してはいけない
       * @param history 今までの局面
       * @param next_state 次の局面
       */
      static Result detect(const HashKeyStack& history, 
			   const HashKey& next_state)
      {
	const Player player = alt(next_state.turn());
	const PieceStand new_stand = next_state.blackStand();
	for (size_t i=3; i<history.size(); i+=4)
	{
	  // 4手が最小のループ. 6は銀があると発生するが頻度は不明
	  const HashKey& old_state = history.top(i);
	  assert(old_state.turn() == next_state.turn());
	  if (! old_state.isSameBoard(next_state))
	    continue;

	  const PieceStand old_stand = old_state.blackStand();
	  if (old_stand == new_stand)
	    return NORMAL;	// 千日手は別に検出
	  if (old_stand.hasMoreThan(player, new_stand))
	    return LOSE;
	  if (new_stand.hasMoreThan(player, old_stand))
	    return WIN;
	}
	return NORMAL;
      }
    };
  } // namespace search
} // namespace osl


#endif /* SEARCH_DOMINANCECHECK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
