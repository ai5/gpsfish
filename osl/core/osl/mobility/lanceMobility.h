/* lanceMobility.h
 */
#ifndef MOBILITY_LANCE_MOBILITY_H
#define MOBILITY_LANCE_MOBILITY_H
#include "osl/mobility/countMobility.h"

namespace osl
{
  namespace mobility
  {
    /**
     * 盤上の香車の動けるマス
     */
    struct LanceMobility
    {
    public:
      /**
       * 縦方向，
       * P : 駒pの持ち主
       * countAll : 利きに関係なく動けるマス
       * countSafe : 相手の利きがない動けるマス
       * 両方を求める
       */
      template<Player P>
      static void countBoth(const NumEffectState& state,Piece p,int& countAll,int& countSafe){
	assert(p.ptype()==LANCE);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	const Square pos=p.square();
	countMobilityBoth(P,state,pos,DirectionPlayerTraits<U,P>::offset(),countAll,countSafe);
      }
      static void countBoth(Player pl,const NumEffectState& state,Piece p,int& countAll,int &countSafe){
	if(pl==BLACK)
	  countBoth<BLACK>(state,p,countAll,countSafe);
	else
	  countBoth<WHITE>(state,p,countAll,countSafe);
      }
      /**
       * 縦方向，利きに関係なく動けるマスの数
       */
      template<Player P>
      static int countAll(const NumEffectState& state,Square pos,int num){
	const Square pos1=state.mobilityOf(DirectionPlayerTraits<U,P>::directionByBlack,num);
	int count=(P==BLACK ? pos.y()-pos1.y() : pos1.y()- pos.y())-1+
	  (state.pieceAt(pos1).template canMoveOn<P>() ? 1 : 0);
	return count;
      }
      template<Player P>
      static int countAll(const NumEffectState& state,Piece p){
	assert(p.ptype()==LANCE);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	return countAll<P>(state,p.square(),p.number());
      }
      static int countAll(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countAll<BLACK>(state,p);
	else
	  return countAll<WHITE>(state,p);
      }
      /**
       * 縦方向，相手の利きがない動けるマスを求める
       */
      template<Player P>
      static int countSafe(const NumEffectState& state,Piece p){
	assert(p.ptype()==LANCE);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	const Square pos=p.square();
	return 
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<U,P>::offset());
      }
      static int countSafe(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countSafe<BLACK>(state,p);
	else
	  return countSafe<WHITE>(state,p);
      }
    };
  }
}
#endif /* MOBILITY_LANCE_MOBILITY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
