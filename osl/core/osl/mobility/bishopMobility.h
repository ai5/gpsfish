/* bishopMobility.h
 */
#ifndef MOBILITY_BISHOP_MOBILITY_H
#define MOBILITY_BISHOP_MOBILITY_H
#include "osl/mobility/countMobility.h"
#include <cstdlib>

namespace osl
{
  namespace mobility
  {
    /**
     * 盤上の角および馬が動けるマスの数を数える
     */
    struct BishopMobility
    {
    public:
      /**
       * 斜め方向，
       * P : 駒pの持ち主
       * countAll : 利きに関係なく動けるマス
       * countSafe : 相手の利きがない動けるマス
       * 両方を求める
       */
      template<Player P>
      static void countBoth(const NumEffectState& state,Piece p,int& countAll,int& countSafe){
	assert(p.ptype()==BISHOP || p.ptype()==PBISHOP);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	const Square pos=p.square();
	countMobilityBoth<P,true,true>(state,pos,DirectionPlayerTraits<UL,P>::offset(),countAll,countSafe);
	countMobilityBoth<P,true,true>(state,pos,DirectionPlayerTraits<UR,P>::offset(),countAll,countSafe);
	countMobilityBoth<P,true,true>(state,pos,DirectionPlayerTraits<DL,P>::offset(),countAll,countSafe);
	countMobilityBoth<P,true,true>(state,pos,DirectionPlayerTraits<DR,P>::offset(),countAll,countSafe);
      }
      static void countBoth(Player pl,const NumEffectState& state,Piece p,int& countAll,int& countSafe){
	if(pl==BLACK)
	  countBoth<BLACK>(state,p,countAll,countSafe);
	else
	  countBoth<WHITE>(state,p,countAll,countSafe);
      }
      /**
       * 斜め方向，利きに関係なく動けるマスの数
       */
      template<Player P>
      static int countAll(const NumEffectState& state,int num){
	const Square posUL=state.mobilityOf(UL,num);
	const Square posUR=state.mobilityOf(UR,num);
	const Square posDL=state.mobilityOf(DL,num);
	const Square posDR=state.mobilityOf(DR,num);
	int count=posDR.y()-posUL.y()+
	  posDL.y()-posUR.y()-4+
	  (state.pieceAt(posUR).template canMoveOn<P>() ? 1 : 0)+
	  (state.pieceAt(posDR).template canMoveOn<P>() ? 1 : 0)+
	  (state.pieceAt(posUL).template canMoveOn<P>() ? 1 : 0)+
	  (state.pieceAt(posDL).template canMoveOn<P>() ? 1 : 0);
	return count;
      }
      template<Player P>
      static int countAll(const NumEffectState& state,Piece p){
	assert(p.ptype()==BISHOP || p.ptype()==PBISHOP);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	return countAll<P>(state,p.number());
      }
      static int countAll(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countAll<BLACK>(state,p);
	else
	  return countAll<WHITE>(state,p);
      }

      template<Player P, Direction Dir>
      static int countAllDir(const NumEffectState& state,Piece p){
	assert(p.ptype()==BISHOP || p.ptype()==PBISHOP);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	assert(Dir == UL || Dir == UR || Dir == DL || Dir == DR);
	Direction dir = (P == BLACK ? Dir : inverse(Dir));
	const Square pos = state.mobilityOf(dir, p.number());
	int count = std::abs(pos.y() - p.square().y())
	  - 1 + (state.pieceAt(pos).template canMoveOn<P>() ? 1 : 0);
	return count;
      }
      template <Direction dir>
      static int countAllDir(Player pl,const NumEffectState& state,Piece p){
	if(pl==BLACK)
	  return countAllDir<BLACK, dir>(state,p);
	else
	  return countAllDir<WHITE, dir>(state,p);
      }
      /**
       * 斜め方向，相手の利きがない動けるマスを求める
       */
      template<Player P>
      static int countSafe(const NumEffectState& state,Piece p){
	assert(p.ptype()==BISHOP || p.ptype()==PBISHOP);
	assert(p.isOnBoard());
	assert(p.owner()==P);
	const Square pos=p.square();
	return 
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<UL,P>::offset())+
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<UR,P>::offset())+
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<DL,P>::offset())+
	  countMobilitySafe(P,state,pos,DirectionPlayerTraits<DR,P>::offset());
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
#endif /* MOBILITY_BISHOP_MOBILITY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
