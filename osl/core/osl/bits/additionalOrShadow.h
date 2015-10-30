/* additionalOrShadow.h
 */
#ifndef OSL_ADDITIONALORSHADOW_H
#define OSL_ADDITIONALORSHADOW_H

#include "osl/numEffectState.h"
#include "osl/bits/boardTable.h"

namespace osl
{
  namespace effect_util
  {
    struct AdditionalOrShadow
    {
      template <int count_max>
      static int count(const PieceVector& direct_pieces, 
		       const NumEffectState& state,
		       Square target, Player attack)
      {
	int result=0;
	for (Piece p: direct_pieces)
	{
	  const Square from = p.square();
	  int num = p.number();
	  const Direction long_d=Board_Table.getLongDirection<BLACK>(Offset32(target,from));
	  if(!isLong(long_d)) continue; // unpromoted Knightを除いておくのとどちらが得か?
	  Direction d=longToShort(long_d);
	  for(;;){
	    num=state.longEffectNumTable()[num][d];
	    if(Piece::isEmptyNum(num) || state.pieceOf(num).owner()!=attack)
	      break;
	    if (++result >= count_max)
	      return result;
	  }
	}
	return result;
      }

    };
  }
} // namespace osl

#endif /* OSL_ADDITIONALORSHADOW_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
