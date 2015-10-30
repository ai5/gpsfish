/* enterKingUtil.h
 */
#ifndef _ENTER_KING_UTIL_H
#define _ENTER_KING_UTIL_H
#include "osl/numEffectState.h"

namespace osl
{
  namespace enter_king
  {
    int countEffectInRange(const NumEffectState& staet, Player Turn,
			   int x0, int x1, int y0, int y1);
    int countEffectInFrontOf(const NumEffectState& state, Player attack, 
			     Square target, Player defense);
    int countPiecePointsOnStand(const NumEffectState& state, Player Turn);
    template <Player Turn>
    int countPiecePointsInRange(const NumEffectState& state, int& num_pieces, 
				int x0, int x1, int y0, int y1);
    int countPiecePointsInRange(const NumEffectState& state, Player Turn, int& num_pieces, 
				int x0, int x1, int y0, int y1);
    template <Player Turn>
    int countPiecePointsOnRow(const NumEffectState& state,
			      int& num_pieces, int row);
    int countPiecePointsOnRow(const NumEffectState& state, Player Turn, 
			      int& num_pieces, int row);
  } //namespace enter_king
} //namespace osl
#endif /* _ENTER_KING_UTIL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
