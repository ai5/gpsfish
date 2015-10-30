#ifndef _GENERATE_DROP_MOVES_TCC
#define _GENERATE_DROP_MOVES_TCC
#include "osl/move_generator/drop.h"
namespace osl 
{
  namespace move_generator
  {
    namespace drop
    {
      /**
       * Nは有効なptypeの数
       * http://d.hatena.ne.jp/LS3600/200911 2009-11-10 参照
       */
      template<Player P,class Action,bool hasPawn,bool hasLance,bool hasKnight,int N>
      void 
      generateX(const NumEffectState& state,Action& action,int x,Move m1,Move m2,Move m3,Ptype t1,Ptype t2,Ptype t3)
      {
	assert(hasPawn || hasLance || hasKnight || N>0);
	if(P==BLACK){
	  if(N>0){
	    Square pos(x,1);
	    Piece p=state.pieceAt(pos);
	    if(p.isEmpty()){
	      if(N==4){
		action.dropMove(pos,ROOK,P);
		action.dropMove(pos,BISHOP,P);
		action.dropMove(pos,GOLD,P);
		action.dropMove(pos,SILVER,P);
	      }
	      else{
		if(N>=1) action.dropMove(pos,t1,P,m1.newAddTo(pos));
		if(N>=2) action.dropMove(pos,t2,P,m2.newAddTo(pos));
		if(N>=3) action.dropMove(pos,t3,P,m3.newAddTo(pos));
	      }
	    }
	  }
	  if(hasPawn || hasLance || N>0){
	    Square pos(x,2);
	    Piece p=state.pieceAt(pos);
	    if(p.isEmpty()){
	      if(N==4){
		action.dropMove(pos,ROOK,P);
		action.dropMove(pos,BISHOP,P);
		action.dropMove(pos,GOLD,P);
		action.dropMove(pos,SILVER,P);
	      }
	      else{
		if(N>=1) action.dropMove(pos,t1,P,m1.newAddTo(pos));
		if(N>=2) action.dropMove(pos,t2,P,m2.newAddTo(pos));
		if(N>=3) action.dropMove(pos,t3,P,m3.newAddTo(pos));
	      }
	      if(hasLance)
		action.dropMove(pos,LANCE,P);
	      if(hasPawn)
		action.dropMove(pos,PAWN,P);
	    }
	  }
	  for(int y=3;y<=9;y++){
	    Square pos(x,y);
	    Piece p=state.pieceAt(pos);
	    if(p.isEmpty()){
	      if(N==4){
		action.dropMove(pos,ROOK,P);
		action.dropMove(pos,BISHOP,P);
		action.dropMove(pos,GOLD,P);
		action.dropMove(pos,SILVER,P);
	      }
	      else{
		if(N>=1) action.dropMove(pos,t1,P,m1.newAddTo(pos));
		if(N>=2) action.dropMove(pos,t2,P,m2.newAddTo(pos));
		if(N>=3) action.dropMove(pos,t3,P,m3.newAddTo(pos));
	      }
	      if(hasKnight)
		action.dropMove(pos,KNIGHT,P);
	      if(hasLance)
		action.dropMove(pos,LANCE,P);
	      if(hasPawn)
		action.dropMove(pos,PAWN,P);
	    }
	  }
	}
	else{
	  if(N>0){
	    Square pos(x,9);
	    Piece p=state.pieceAt(pos);
	    if(p.isEmpty()){
	      if(N==4){
		action.dropMove(pos,ROOK,P);
		action.dropMove(pos,BISHOP,P);
		action.dropMove(pos,GOLD,P);
		action.dropMove(pos,SILVER,P);
	      }
	      else{
		if(N>=1) action.dropMove(pos,t1,P,m1.newAddTo(pos));
		if(N>=2) action.dropMove(pos,t2,P,m2.newAddTo(pos));
		if(N>=3) action.dropMove(pos,t3,P,m3.newAddTo(pos));
	      }
	    }
	  }
	  if(hasPawn || hasLance || N>0){
	    Square pos(x,8);
	    Piece p=state.pieceAt(pos);
	    if(p.isEmpty()){
	      if(N==4){
		action.dropMove(pos,ROOK,P);
		action.dropMove(pos,BISHOP,P);
		action.dropMove(pos,GOLD,P);
		action.dropMove(pos,SILVER,P);
	      }
	      else{
		if(N>=1) action.dropMove(pos,t1,P,m1.newAddTo(pos));
		if(N>=2) action.dropMove(pos,t2,P,m2.newAddTo(pos));
		if(N>=3) action.dropMove(pos,t3,P,m3.newAddTo(pos));
	      }
	      if(hasLance)
		action.dropMove(pos,LANCE,P);
	      if(hasPawn)
		action.dropMove(pos,PAWN,P);
	    }
	  }
	  for(int y=7;y>=1;y--){
	    Square pos(x,y);
	    Piece p=state.pieceAt(pos);
	    if(p.isEmpty()){
	      if(N==4){
		action.dropMove(pos,ROOK,P);
		action.dropMove(pos,BISHOP,P);
		action.dropMove(pos,GOLD,P);
		action.dropMove(pos,SILVER,P);
	      }
	      else{
		if(N>=1) action.dropMove(pos,t1,P,m1.newAddTo(pos));
		if(N>=2) action.dropMove(pos,t2,P,m2.newAddTo(pos));
		if(N>=3) action.dropMove(pos,t3,P,m3.newAddTo(pos));
	      }
	      if(hasKnight)
		action.dropMove(pos,KNIGHT,P);
	      if(hasLance)
		action.dropMove(pos,LANCE,P);
	      if(hasPawn)
		action.dropMove(pos,PAWN,P);
	    }
	  }
	}
      }

      template<Player P,class Action,bool hasPawn,bool hasLance,bool hasKnight,int N>
      void 
      generate(const NumEffectState& state,Action& action,Move m1,Move m2,Move m3,Ptype t1,Ptype t2,Ptype t3)
      {
	if(hasPawn || hasLance || hasKnight || N>0){
	  if(hasPawn){
	    if(hasLance || hasKnight || N>0){
	      for(int x=9;x>0;x--){
		if(state.isPawnMaskSet<P>(x))
		  generateX<P,Action,false,hasLance,hasKnight,N>(state,action,x,m1,m2,m3,t1,t2,t3);
		else
		  generateX<P,Action,true,hasLance,hasKnight,N>(state,action,x,m1,m2,m3,t1,t2,t3);
	      }
	    }
	    else{
	      for(int x=9;x>0;x--){
		if(!state.isPawnMaskSet<P>(x))
		  generateX<P,Action,true,hasLance,hasKnight,N>(state,action,x,m1,m2,m3,t1,t2,t3);
	      }
	    }
	  }
	  else{ // pawnなし
	    for(int x=9;x>0;x--){
	      generateX<P,Action,false,hasLance,hasKnight,N>(state,action,x,m1,m2,m3,t1,t2,t3);
	    }
	  }
	}
      }


      template<Player P,class Action,bool hasPawn,bool hasLance,bool hasKnight>
      static void checkSilver(const NumEffectState& state,Action& action)
      {
	if(state.template hasPieceOnStand<SILVER>(P)){
	  if(state.template hasPieceOnStand<GOLD>(P)){
	    if(state.template hasPieceOnStand<BISHOP>(P)){
	      if(state.template hasPieceOnStand<ROOK>(P))
		generate<P,Action,hasPawn,hasLance,hasKnight,4>(
		  state,action,
		  Move::makeDirect(0),Move::makeDirect(0),Move::makeDirect(0),
		  PTYPE_EMPTY,PTYPE_EMPTY,PTYPE_EMPTY);
	      else
		generate<P,Action,hasPawn,hasLance,hasKnight,3>(
		  state,action,
		  Move(Square::STAND(),BISHOP,P),
		  Move(Square::STAND(),GOLD,P),
		  Move(Square::STAND(),SILVER,P),
		  BISHOP,GOLD,SILVER);
	    }
	    else if(state.template hasPieceOnStand<ROOK>(P))
	      generate<P,Action,hasPawn,hasLance,hasKnight,3>(
		state,action,
		Move(Square::STAND(),ROOK,P),
		Move(Square::STAND(),GOLD,P),
		Move(Square::STAND(),SILVER,P),
		ROOK,GOLD,SILVER);
	    else
	      generate<P,Action,hasPawn,hasLance,hasKnight,2>(
		state,action,
		Move(Square::STAND(),GOLD,P),
		Move(Square::STAND(),SILVER,P),
		Move::makeDirect(0),
		GOLD,SILVER,PTYPE_EMPTY);
	  } 
	  else if(state.template hasPieceOnStand<BISHOP>(P)){
	    if(state.template hasPieceOnStand<ROOK>(P))
	      generate<P,Action,hasPawn,hasLance,hasKnight,3>(
		state,action,
		Move(Square::STAND(),ROOK,P),
		Move(Square::STAND(),BISHOP,P),
		Move(Square::STAND(),SILVER,P),
		ROOK,BISHOP,SILVER);
	    else
	      generate<P,Action,hasPawn,hasLance,hasKnight,2>(
		state,action,
		Move(Square::STAND(),BISHOP,P),
		Move(Square::STAND(),SILVER,P),
		Move::makeDirect(0),
		BISHOP,SILVER,PTYPE_EMPTY);
	    }
	  else if(state.template hasPieceOnStand<ROOK>(P))
	    generate<P,Action,hasPawn,hasLance,hasKnight,2>(
	      state,action,
	      Move(Square::STAND(),ROOK,P),
	      Move(Square::STAND(),SILVER,P),
	      Move::makeDirect(0),
	      ROOK,SILVER,PTYPE_EMPTY);
	  else
	    generate<P,Action,hasPawn,hasLance,hasKnight,1>(
	      state,action,
	      Move(Square::STAND(),SILVER,P),
	      Move::makeDirect(0),
	      Move::makeDirect(0),
	      SILVER,PTYPE_EMPTY,PTYPE_EMPTY);
	}
	else if(state.template hasPieceOnStand<GOLD>(P)){
	  if(state.template hasPieceOnStand<BISHOP>(P)){
	    if(state.template hasPieceOnStand<ROOK>(P))
	      generate<P,Action,hasPawn,hasLance,hasKnight,3>(
		state,action,
		Move(Square::STAND(),ROOK,P),
		Move(Square::STAND(),BISHOP,P),
		Move(Square::STAND(),GOLD,P),
		ROOK,BISHOP,GOLD);
	    else
	      generate<P,Action,hasPawn,hasLance,hasKnight,2>(
		state,action,
		Move(Square::STAND(),BISHOP,P),
		Move(Square::STAND(),GOLD,P),
		Move::makeDirect(0),
		BISHOP,GOLD,PTYPE_EMPTY);
	  }
	  else if(state.template hasPieceOnStand<ROOK>(P))
	    generate<P,Action,hasPawn,hasLance,hasKnight,2>(
	      state,action,
	      Move(Square::STAND(),ROOK,P),
	      Move(Square::STAND(),GOLD,P),
	      Move::makeDirect(0),
	      ROOK,GOLD,PTYPE_EMPTY);
	  else
	    generate<P,Action,hasPawn,hasLance,hasKnight,1>(
	      state,action,
	      Move(Square::STAND(),GOLD,P),
	      Move::makeDirect(0),
	      Move::makeDirect(0),
	      GOLD,PTYPE_EMPTY,PTYPE_EMPTY);
	} 
	else if(state.template hasPieceOnStand<BISHOP>(P)){
	  if(state.template hasPieceOnStand<ROOK>(P))
	    generate<P,Action,hasPawn,hasLance,hasKnight,2>(
	      state,action,
	      Move(Square::STAND(),ROOK,P),
	      Move(Square::STAND(),BISHOP,P),
	      Move::makeDirect(0),
	      ROOK,BISHOP,PTYPE_EMPTY);
	  else
	    generate<P,Action,hasPawn,hasLance,hasKnight,1>(
	      state,action,
	      Move(Square::STAND(),BISHOP,P),
	      Move::makeDirect(0),
	      Move::makeDirect(0),
	      BISHOP,PTYPE_EMPTY,PTYPE_EMPTY);
	}
	else if(state.template hasPieceOnStand<ROOK>(P))
	  generate<P,Action,hasPawn,hasLance,hasKnight,1>(
	    state,action,
	    Move(Square::STAND(),ROOK,P),
	    Move::makeDirect(0),
	    Move::makeDirect(0),
	    ROOK,PTYPE_EMPTY,PTYPE_EMPTY);
	else
	  generate<P,Action,hasPawn,hasLance,hasKnight,0>(
	    state,action,
	    Move::makeDirect(0),
	    Move::makeDirect(0),
	    Move::makeDirect(0),
	    PTYPE_EMPTY,PTYPE_EMPTY,PTYPE_EMPTY);
      }

      template<Player P,class Action,bool hasPawn,bool hasLance>
      static void checkKnight(const NumEffectState& state,Action& action)
      {
	if(state.template hasPieceOnStand<KNIGHT>(P))
	  checkSilver<P,Action,hasPawn,hasLance,true>(state,action);
	else
	  checkSilver<P,Action,hasPawn,hasLance,false>(state,action);
      }

      template<Player P,class Action,bool hasPawn>
      static void checkLance(const NumEffectState& state,Action& action)
      {
	if(state.template hasPieceOnStand<LANCE>(P))
	  checkKnight<P,Action,hasPawn,true>(state,action);
	else
	  checkKnight<P,Action,hasPawn,false>(state,action);
      }

    } // namespace drop
    using drop::checkLance;

    template<class Action>
    template<osl::Player P>
    void osl::move_generator::Drop<Action>::
    generate(const NumEffectState& state,Action& action)
    {
      if(state.template hasPieceOnStand<PAWN>(P))
	checkLance<P,Action,true>(state,action);
      else
	checkLance<P,Action,false>(state,action);
    }
  } // namespace move_generator
} // namespace osl

#endif /* _GENERATE_DROP_MOVES_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
