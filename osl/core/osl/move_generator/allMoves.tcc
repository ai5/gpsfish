#ifndef OSL_GENERATE_ALL_MOVES_TCC
#define OSL_GENERATE_ALL_MOVES_TCC
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/pieceOnBoard.tcc"
#include "osl/move_generator/drop.h"
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    namespace all_moves
    {
      template<class Action,Player P,Ptype T>
      void
      generatePtype(const NumEffectState& state, Action& action){
	for(int num=PtypeTraits<T>::indexMin;num<PtypeTraits<T>::indexLimit;++num){
	  Piece p=state.pieceOf(num);
	  if(p.isOnBoardByOwner<P>()){
	    int dummy=0;
	    if(PtypeTraits<T>::canPromote && p.isPromoted()){
	      const Ptype PT=PtypeTraits<PtypeFuns<T>::promotePtype >::moveType;
	      PieceOnBoard<Action>::template generatePtype<P,PT,false>(state,p,action,dummy);
	    }
	    else{
	      PieceOnBoard<Action>::template generatePtype<P,T,false>
		(state,p,action,dummy);
	    }
	  }
	}
      }
    }
    using all_moves::generatePtype;
    /**
     * すべての手を生成する
     */
    template<class Action>
    template<Player P>
    void AllMoves<Action>::
    generateOnBoard(const NumEffectState& state, Action& action){
      generatePtype<Action,P,PAWN>(state,action);
      generatePtype<Action,P,LANCE>(state,action);
      generatePtype<Action,P,KNIGHT>(state,action);
      generatePtype<Action,P,SILVER>(state,action);
      generatePtype<Action,P,GOLD>(state,action);
      generatePtype<Action,P,BISHOP>(state,action);
      generatePtype<Action,P,ROOK>(state,action);
      int dummy=0;
      PieceOnBoard<Action>::template generatePtype<P,KING,false>
	(state,state.kingPiece<P>(),action,dummy);
    }
    /**
     * すべての手を生成する
     */
    template<class Action>
    template<Player P>
    void AllMoves<Action>::
    generate(const NumEffectState& state, Action& action){
      generateOnBoard<P>(state,action);
      Drop<Action>::template generate<P>(state,action);
    }


  } // namespace move_generator
} // namespace osl

#endif /* OSL_GENERATE_ALL_MOVES_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
