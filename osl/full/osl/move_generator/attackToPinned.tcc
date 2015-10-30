#ifndef _MOVE_GENERATOR_ATTACK_TO_PINNED_TCC
#define _MOVE_GENERATOR_ATTACK_TO_PINNED_TCC
#include "osl/move_generator/attackToPinned.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/bits/ptypeTable.h"

namespace osl
{
  namespace move_generator
  {
    namespace {
      template<Player P,Ptype T,class Action>
      void generatePtype(const NumEffectState& state,Action& action)
      {
	Square target=state.template kingSquare<alt(P)>();
	for(int num=PtypeTraits<T>::indexMin;num<PtypeTraits<T>::indexLimit;num++){
	  Piece p=state.pieceOf(num);
	  if(p.template isOnBoardByOwner<P>()){
	    Square from=p.square();
	    EffectContent effect=Ptype_Table.getEffect(newPtypeO(P,T),from,target);
	    if(effect.hasEffect()){
	      Offset offset=effect.offset();
	      assert(!offset.zero());
	      Piece p1;
	      Square pos=target-offset;
	      for(;(p1=state.pieceAt(pos)).isEmpty();pos-=offset) ;
	      if(p1.canMoveOn<P>() &&
		 state.hasEffectByPiece(p,pos)){
		AddEffectWithEffect<Action>::template generate<P,false>(state,pos,action);
	      }
	    }
	  }
	}
      }
    }
    template<Player P>
    template<class Action>
    void AttackToPinned<P>::
    generate(const NumEffectState& state,Action& action)
    {
      generatePtype<P,ROOK,Action>(state,action);
      generatePtype<P,BISHOP,Action>(state,action);
      generatePtype<P,LANCE,Action>(state,action);
    }

  }
}

#endif /* _MOVE_GENERATOR_ATTACK_TO_PINNED_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
