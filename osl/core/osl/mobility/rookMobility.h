/* rookMobility.h
 */
#ifndef MOBILITY_ROOK_MOBILITY_H
#define MOBILITY_ROOK_MOBILITY_H
#include "osl/mobility/countMobility.h"
#include "osl/bits/boardTable.h"

namespace osl
{
  namespace mobility
  {
    /**
     * 盤上の飛車および竜の動けるマス
     */
    struct RookMobility
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
      static void countVerticalBoth(const NumEffectState& state,Piece p,int& countAll,int& countSafe){
	assert(p.ptype()==ROOK || p.ptype()==PROOK);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	const Square pos=p.square();
	countMobilityBoth(P,state,pos,DirectionPlayerTraits<U,P>::offset(),countAll,countSafe);
	countMobilityBoth(P,state,pos,DirectionPlayerTraits<D,P>::offset(),countAll,countSafe);
      }
      static void countVerticalBoth(Player pl,const NumEffectState& state,Piece p,int& countAll,int& countSafe){
	if(pl==BLACK)
	  countVerticalBoth<BLACK>(state,p,countAll,countSafe);
	else
	  countVerticalBoth<WHITE>(state,p,countAll,countSafe);
      }
      /**
       * 縦方向，利きに関係なく動けるマスの数
       */
      template<Player P>
      static int countVerticalAll(const NumEffectState& state,int num){
	//	const Square pos=p.square();
	const Square posU=state.mobilityOf(U,num);
	const Square posD=state.mobilityOf(D,num);
	int count=posD.y()-posU.y()-2+
	  (state.pieceAt(posU).template canMoveOn<P>() ? 1 : 0)+
	  (state.pieceAt(posD).template canMoveOn<P>() ? 1 : 0);
	return count;
      }
      template<Player P>
      static int countVerticalAll(const NumEffectState& state,Piece p){
	return countVerticalAll<P>(state,p.number());
      }
      static int countVerticalAll(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countVerticalAll<BLACK>(state,p);
	else
	  return countVerticalAll<WHITE>(state,p);
      }
      /**
       * 縦方向，相手の利きがない動けるマスを求める
       */
      template<Player P>
      static int countVerticalSafe(const NumEffectState& state,Piece p){
	const Square pos=p.square();
	return 
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<U,P>::offset())+
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<D,P>::offset());
      }
      static int countVerticalSafe(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countVerticalSafe<BLACK>(state,p);
	else
	  return countVerticalSafe<WHITE>(state,p);
      }
      /**
       * 横方向，
       * P : 駒pの持ち主
       * countAll : 利きに関係なく動けるマス
       * countSafe : 相手の利きがない動けるマス
       * 両方を求める
       */
      template<Player P>
      static void countHorizontalBoth(const NumEffectState& state,Piece p,int& countAll,int& countSafe){
	assert(p.ptype()==ROOK || p.ptype()==PROOK);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	const Square pos=p.square();
	countMobilityBoth(P,state,pos,DirectionPlayerTraits<L,P>::offset(),countAll,countSafe);
	countMobilityBoth(P,state,pos,DirectionPlayerTraits<R,P>::offset(),countAll,countSafe);
      }
      static void countHorizontalBoth(Player pl,const NumEffectState& state,Piece p,int& countAll,int& countSafe){
	if(pl==BLACK)
	  countHorizontalBoth<BLACK>(state,p,countAll,countSafe);
	else
	  countHorizontalBoth<WHITE>(state,p,countAll,countSafe);
      }

      template<Player P>
      static int countHorizontalAll(const NumEffectState& state,int num)
      {
	const Square posR=state.mobilityOf(R,num);
	const Square posL=state.mobilityOf(L,num);
	int count=(posL.x()-posR.x()-2)+
	  (state.pieceAt(posR).template canMoveOn<P>() ? 1 : 0)+
	  (state.pieceAt(posL).template canMoveOn<P>() ? 1 : 0);
	return count;
      }
      /**
       * 横方向，利きに関係なく動けるマスの数
       */
      template<Player P>
      static int countHorizontalAll(const NumEffectState& state,Piece p){
	return countHorizontalAll<P>(state,p.number());
      }
      static int countHorizontalAll(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countHorizontalAll<BLACK>(state,p);
	else
	  return countHorizontalAll<WHITE>(state,p);
      }
      /**
       * 横方向，相手の利きがない動けるマスを求める
       */
      template<Player P>
      static int countHorizontalSafe(const NumEffectState& state,Piece p){
	const Square pos=p.square();
	return 
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<L,P>::offset())+
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<R,P>::offset());
      }
      static int countHorizontalSafe(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countHorizontalSafe<BLACK>(state,p);
	else
	  return countHorizontalSafe<WHITE>(state,p);
      }
    };
  }
}
#endif /* MOBILITY_ROOK_MOBILITY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
