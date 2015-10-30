/* pawnCheckmateMoves.h
 */
#ifndef _PAWNCHECKMATEMOVES_H
#define _PAWNCHECKMATEMOVES_H

#include "osl/basic_type.h"
namespace osl
{
  namespace checkmate
  {
    struct PawnCheckmateMoves
    {
      /**
       * 指手は打歩詰の時以外は試さなくて良い
       * TODO: 敵陣2段目の香も打歩詰以外は成るべき
       */
      template <Player P>
      static bool effectiveOnlyIfPawnCheckmate(Ptype ptype, 
					       Square from, Square to)
      {
	return ((ptype == PAWN) || (ptype == ROOK) || (ptype == BISHOP))
	  && (from.canPromote<P>() || to.canPromote<P>());
      }
      static bool effectiveOnlyIfPawnCheckmate(Player a, Ptype ptype, 
					       Square from, Square to)
      {
	return ((ptype == PAWN) || (ptype == ROOK) || (ptype == BISHOP))
	  && (from.canPromote(a) || to.canPromote(a));
      }
      static bool effectiveOnlyIfPawnCheckmate(Move m)
      {
	return effectiveOnlyIfPawnCheckmate(m.player(), m.ptype(),
					    m.from(), m.to());
      }
      
      static bool hasParingNoPromote(bool isPromote, Ptype ptype)
      {
	return isPromote
	  && ((ptype == PPAWN) || (ptype == PROOK) || (ptype == PBISHOP));
      }
      /**
       * m を不成にした指手は打歩詰の時以外は試さなくて良い.
       * m を不成にした指手が王手とは限らない
       */
      static bool hasParingNoPromote(Move m)
      {
	return hasParingNoPromote(m.isPromotion(), m.ptype());
      }
    };
  } // namespace checkmate
} // namespace osl

#endif /* _PAWNCHECKMATEMOVES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
