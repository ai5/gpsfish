/* shouldPromoteCut.h
 */
#ifndef _SEARCH_SHOULDPROMOTECUT_H
#define _SEARCH_SHOULDPROMOTECUT_H

#include "osl/basic_type.h"
#include "osl/bits/ptypeTable.h"
namespace osl
{
    /**
     * 探索で前向き枝刈して良い条件を一時的に書いておく
     *
     * 効果を把握したら手生成の段階で cut するのが better 
       * TODO: あと，2段目への香は必ず成るチェックをいれる
     */
    struct ShouldPromoteCut
    {
      template <Player P>
      static bool canIgnore(Ptype ptype, Square from, Square to)
      {
	assert(! from.isPieceStand());
	return (ptype==LANCE && (P==BLACK ? to.y()==2 : to.y()==8)) ||
	  (isBasic(ptype) && Ptype_Table.isBetterToPromote(ptype)
	   && (to.canPromote<P>() || from.canPromote<P>()));
      }
      /**
       * dropでないことが確定している場合
       */
      template <Player Moving>
      static bool canIgnoreMove(Move move)
      {
	assert(! move.isDrop());
	return canIgnore<Moving>(move.ptype(), move.from(), move.to());
      }
      /**
       * drop は通すチェック有り
       */
      template <Player Moving>
      static bool canIgnoreAndNotDrop(Move move)
      {
	return (! move.isDrop()) && canIgnoreMove<Moving>(move);
      }
      static bool canIgnoreAndNotDrop(Move move)
      {
	if (move.player() == BLACK)
	  return canIgnoreAndNotDrop<BLACK>(move);
	else
	  return canIgnoreAndNotDrop<WHITE>(move);
      }
    };
    

} // osl

#endif /* _SHOULDPROMOTECUT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
