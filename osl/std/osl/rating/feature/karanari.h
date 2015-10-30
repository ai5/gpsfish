/* karanari.h
 */
#ifndef _KARANARI_H
#define _KARANARI_H

#include "osl/rating/feature.h"
#include "osl/effect_util/neighboring8Direct.h"
namespace osl
{
  namespace rating
  {
    class Karanari : public Feature
    {
      bool bishop, can_promote_area;
    public:
      Karanari(bool b, bool c) : Feature(b ? "Bishop" : "Rook"), bishop(b), can_promote_area(c) {}
      static bool matchGeneral(const NumEffectState& state, Move move)
      {
	if (! (move.isPromotion() && move.capturePtype() == PTYPE_EMPTY
	       && move.from().canPromote(state.turn())))
	  return false;
	const Square op_king = state.kingSquare(alt(state.turn()));
	if (! Neighboring8Direct::hasEffect(state, move.oldPtypeO(), move.from(), op_king)
	    && Neighboring8Direct::hasEffect(state, move.ptypeO(), move.to(), op_king))
	  return false;
	return true;
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	if (! (move.ptype() == (bishop ? PBISHOP : PROOK)
	       && matchGeneral(state, move)))
	  return false;
	if (can_promote_area)
	  return move.to().canPromote(move.player());
	const Square my_king = state.kingSquare(state.turn());
	if (bishop && move.to().isNeighboring8(my_king))
	  return false;
	return true;
      }
      static int index(const NumEffectState& state, Move move)
      {
	int base;
	switch (move.ptype()) {
	case PBISHOP:
	  base = 2;
	  break;
	case PROOK:
	  base = 0;
	  break;
	default:
	  return -1;
	}
	if (! matchGeneral(state, move))
	  return -1;
	if (move.to().canPromote(move.player()))
	  return base;
	const Square my_king = state.kingSquare(state.turn());
	if (/*bishop*/ base && move.to().isNeighboring8(my_king))
	  return -1;
	return base + 1;
      }
    };
  }
}

#endif /* _KARANARI_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
