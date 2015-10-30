/* countMobility.h
 */
#ifndef MOBILITY_COUNT_MOBILITY_H
#define MOBILITY_COUNT_MOBILITY_H
#include "osl/numEffectState.h"

namespace osl
{
  namespace mobility
  {
    /**
     * P : 駒pの持ち主
     * All : countAllを求めるかどうか?
     * Safe : countAllを求めるかどうか?
     * countAll : 利きに関係なく動けるマス
     * countSafe : 相手の利きがない動けるマス
     * 両方を求める
     */
    template<Player P,bool All,bool Safe>
    inline void countMobilityBoth(const NumEffectState& state,Square pos,Offset o,int& countAll,int& countSafe){
      assert(pos.isOnBoard());
      assert(!o.zero());
      Piece p;
      for(pos+=o;(p=state.pieceAt(pos)).isEmpty();pos+=o){
	if(All) countAll++;
	if(Safe && !state.hasEffectAt<alt(P)>(pos)) 
	  countSafe++;
      }
      if(p.canMoveOn<P>()){
	if(All) countAll++;
	if(Safe && !state.hasEffectAt<alt(P)>(pos)) 
	  countSafe++;
      }
    }
    inline void countMobilityBoth(Player P,const NumEffectState& state,Square pos,Offset o,int& countAll,int& countSafe){
      if(P==BLACK)
	countMobilityBoth<BLACK,true,true>(state,pos,o,countAll,countSafe);
      else
	countMobilityBoth<WHITE,true,true>(state,pos,o,countAll,countSafe);
    }
    /**
     * 利きに関係なく動けるマスの数
     */
    inline int countMobilityAll(Player pl,const NumEffectState& state,Square pos,Offset o)
    {
      int ret=0,dummy=0;
      if(pl==BLACK) 
	countMobilityBoth<BLACK,true,false>(state,pos,o,ret,dummy);
      else
	countMobilityBoth<WHITE,true,false>(state,pos,o,ret,dummy);
      return ret;
    }
    /**
     * 相手の利きがない動けるマスを求める
     */
    inline int countMobilitySafe(Player pl,const NumEffectState& state,Square pos,Offset o)
    {
      int ret=0,dummy=0;
      if(pl==BLACK) 
	countMobilityBoth<BLACK,false,true>(state,pos,o,dummy,ret);
      else
	countMobilityBoth<WHITE,false,true>(state,pos,o,dummy,ret);
      return ret;
    }
  }
}
#endif /* MOBILITY_ROOK_MOBILITY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
