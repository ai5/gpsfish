#ifndef OSL_MOVE_GENERATOR_PROMOTE_TCC
#define OSL_MOVE_GENERATOR_PROMOTE_TCC

#include "osl/move_generator/promote_.h"
#include "osl/basic_type.h"
#include "osl/bits/directionTraits.h"
#include "osl/bits/ptypeTraits.h"
namespace osl
{
  namespace move_generator
  {
    namespace promote
    {
      template<Player P,Ptype T,bool noCapture,Direction Dir>
      class AllPromoteDir
      {
	template <class Action>
	static void generateIfValid(const NumEffectState& state,Piece piece, Action& action, 
				    Int2Type<true> /*isLong*/, Int2Type<true>)
	{
	  const Square from = piece.square();
	  const Direction black_direction = longToShort(DirectionPlayerTraits<Dir,P>::directionByBlack);
	  Square last = state.mobilityOf(black_direction, piece.number());
	  const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	  assert(! last.isPieceStand());
	  assert(!offset.zero());
	  for (Square to=from+offset; to!=last; to+=offset) {
	    assert(state.pieceAt(to).isEmpty());
	    action.simpleMove(from,to,PtypeFuns<T>::promotePtype,true,P);
	  }
	  const Piece last_piece = state.pieceAt(last);
	  if (!noCapture && last_piece.canMoveOn<P>()){
	    action.unknownMove(from,last,last_piece,PtypeFuns<T>::promotePtype,true,P);
	  }
	}
	// short move
	template <class Action>
	static void generateIfValid(const NumEffectState& state,Piece p, Action& action, 
				    Int2Type<false> /*isLong*/,Int2Type<true>){
	  Square pos=p.square();
	  const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	  Square toPos=pos+offset;
	  Piece p1=state.pieceAt(toPos);
	  if (p1.isEmpty()){
	    action.simpleMove(pos,toPos,PtypeFuns<T>::promotePtype,true,P);
	  }
	  else if (!noCapture && p1.canMoveOn<P>()){
	    action.unknownMove(pos,toPos,p1,PtypeFuns<T>::promotePtype,true,P);
	  }
	}
	template <class Action>
	static void generateIfValid(const NumEffectState&, Piece, Action&, Int2Type<true>,Int2Type<false>){
	}
	template <class Action>
	static void generateIfValid(const NumEffectState&, Piece, Action&, Int2Type<false>,Int2Type<false>){
	}
      public:
	template<class Action>
	static void generate(NumEffectState const& state,Piece p,Action& action){
	  generateIfValid(state,p,action,
			  Int2Type<DirectionTraits<Dir>::isLong>(),
			  Int2Type<(PtypeTraits<T>::moveMask 
				    & DirectionTraits<Dir>::mask) !=0>());
	}
      };
      /**
       * 指定した駒が常にpromote可能な場合にpromoteする動きを作る
       */
      template<Player P,Ptype T,bool noCapture>
      class AllPromote{
      public:
	template<class Action>
	static void generate(NumEffectState const& state,Piece p,Action& action){
	  AllPromoteDir<P,T,noCapture,UL>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,U>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,UR>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,L>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,R>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,DL>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,D>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,DR>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,UUL>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,UUR>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_UL>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_U>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_UR>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_L>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_R>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_DL>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_D>::generate(state,p,action);
	  AllPromoteDir<P,T,noCapture,LONG_DR>::generate(state,p,action);
	}
      };
      template<Player P,Ptype T,bool noCapture,Direction Dir>
      class MayPromoteDir
      {
	template <class Action>
	static void generateIfValid(const NumEffectState& state,Piece piece, Action& action, 
				    Int2Type<true> /*isLong*/, Int2Type<true>)
	{
	  const Square from = piece.square();
	  const Direction black_direction = longToShort(DirectionPlayerTraits<Dir,P>::directionByBlack);
	  Square last = state.mobilityOf(black_direction, piece.number());
	  const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	  assert(! last.isPieceStand());
	  assert(! offset.zero());

	  const Piece last_piece = state.pieceAt(last);
	  if (!noCapture && last_piece.canMoveOn<P>()){
	    if (! last.canPromote<P>())
	      return;
	    action.unknownMove(from,last,last_piece,PtypeFuns<T>::promotePtype,true,P);
	  }
	  for (Square to=last-offset; to!=from; to-=offset) {
	    assert(state.pieceAt(to).isEmpty());
	    if (! to.canPromote<P>())
	      return;
	    action.simpleMove(from,to,PtypeFuns<T>::promotePtype,true,P);
	  }
	}
	// short move
	template <class Action>
	static void generateIfValid(const NumEffectState& state,Piece p, Action& action, Int2Type<false>,Int2Type<true>){
	  Square pos=p.square();
	  const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	  Square toPos=pos+offset;
	  Piece p1=state.pieceAt(toPos);
	  if (p1.isEmpty()){
	    action.simpleMove(pos,toPos,PtypeFuns<T>::promotePtype,true,P);
	  }
	  else if (!noCapture && p1.canMoveOn<P>()){
	    action.unknownMove(pos,toPos,p1,PtypeFuns<T>::promotePtype,true,P);
	  }
	}
	template <class Action>
	static void generateIfValid(const NumEffectState&, Piece, Action&, Int2Type<true>,Int2Type<false>){
	}
	template <class Action>
	static void generateIfValid(const NumEffectState&, Piece, Action&, Int2Type<false>,Int2Type<false>){
	}
      public:
	template<class Action>
	static void generate(NumEffectState const& state,Piece p,Action& action){
	  generateIfValid(state,p,action,
			  Int2Type<DirectionTraits<Dir>::isLong>(),
			  Int2Type<(PtypeTraits<T>::moveMask 
				    & DirectionTraits<Dir>::mask) !=0>());
	}
      };
      /**
       * 指定した駒が移動先によってはpromoteできる場合
       */
      template<Player P,Ptype T,bool noCapture>
      class MayPromote{
      public:
	template<class Action>
	static void generate(NumEffectState const& state,Piece p,Action& action){
	  MayPromoteDir<P,T,noCapture,UL>::generate(state,p,action);
	  MayPromoteDir<P,T,noCapture,U>::generate(state,p,action);
	  MayPromoteDir<P,T,noCapture,UR>::generate(state,p,action);
	  MayPromoteDir<P,T,noCapture,UUL>::generate(state,p,action);
	  MayPromoteDir<P,T,noCapture,UUR>::generate(state,p,action);
	  MayPromoteDir<P,T,noCapture,LONG_UL>::generate(state,p,action);
	  MayPromoteDir<P,T,noCapture,LONG_U>::generate(state,p,action);
	  MayPromoteDir<P,T,noCapture,LONG_UR>::generate(state,p,action);
	}
      };

      template<typename Action,Player P,Ptype T,bool noCapture>
      struct EachOnBoard
      {
	const NumEffectState& state;
	Action& action;
	EachOnBoard(const NumEffectState& state,Action& action):state(state),action(action){}
	void operator()(Piece p){
	  assert(! p.isPromoted());
	  if (PtypePlayerTraits<T,P>::mayPromote(p.square())){
	    if (p.square().template canPromote<P>()){
	      AllPromote<P,T,noCapture>::generate(state,p,action);
	    }
	    else{
	      MayPromote<P,T,noCapture>::generate(state,p,action);
	    }
	  }
	}
      };
    }

    template<Player P, bool noCapture>
    template <class Action, Ptype T>
    void Promote<P,noCapture>::
    generateMovesPtype(const NumEffectState& state, Action& action){
      typedef promote::EachOnBoard<Action,P,T,noCapture> each_t;
      each_t eachOnBoard(state,action);
      state.template forEachOnBoardPtypeStrict<P,T,each_t>(eachOnBoard);
    }

    template<Player P,bool noCapture>
    template <class Action>
    void Promote<P,noCapture>::
    generateMoves(const NumEffectState& state, Action& action){
      // promoteの価値の高い順に生成してみる
      // PAWNは600-100=500
      generateMovesPtype<Action,PAWN>(state,action);
      // ROOKは1300-950=350
      generateMovesPtype<Action,ROOK>(state,action);
      // BISHOPは1150-800=350
      generateMovesPtype<Action,BISHOP>(state,action);
      // LANCEは600-400=200
      generateMovesPtype<Action,LANCE>(state,action);
      // KNIGHTは600-400=200
      generateMovesPtype<Action,KNIGHT>(state,action);
      // SILVERは600-550=50
      generateMovesPtype<Action,SILVER>(state,action);
      // GOLD,KINGはpromoteしないので除く
    }
  }
}
#endif /* OSL_MOVE_GENERATOR_PROMOTE_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
