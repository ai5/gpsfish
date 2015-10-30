#ifndef OSL_GENERATE_PIECE_MOVES_TCC
#define OSL_GENERATE_PIECE_MOVES_TCC
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/move_action.h"
#include "osl/bits/king8Info.h"

namespace osl
{
  namespace move_generator
  {
    namespace piece_on_board
    {
      /**
       * ROOK, BISHOP, PROOK, PBISHOPのlong方向の手生成
       * CanPはNoPromoteかCanPromote, CheckPromoteのみ
       * NoPromoteはpromoteできない点からの後ろ，横のdirection
       * CanPromoteはpromoteできる点から
       * CheckPromoteはpromoteできない点からの前向き direction
       */
      template <Player P,class Action,PromoteType CanP,Direction Dir,bool notPromoteCapture>
      inline void 
      generateLong(NumEffectState const& state,Piece p,const Piece *ptr,Square from,Action& action,Int2Type<true>,Move moveBase,Ptype ptype)
      {
	int num=p.number();
	const Direction shortDir=longToShort(Dir);
	Square limit=state.mobilityOf((P==BLACK ? shortDir : inverse(shortDir)),num);
	const Piece *limitPtr=state.getPiecePtr(limit);
	assert(ptype!=LANCE);
	const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	assert(!offset.zero());
	ptr+=offset.intValue();
	Square to=from+offset;
	Move m=moveBase.newAddTo(offset);
	if(CanP==CheckPromoteType || CanP==CanPromoteType){
	  if(CanP==CheckPromoteType){
	    // promoteできない数
	    int count=(P==BLACK ? from.y1()-5 : 7-from.y1()); 
	    for(int i=0;i<count;i++){
	      if(ptr==limitPtr){
		Piece p1= *limitPtr;
		if(!notPromoteCapture && p1.canMoveOn<P>())
		  action.unknownMove(from,to,p1,ptype,false,P,m.newAddCapture(p1));
		return;
	      }
	      action.simpleMove(from,to,ptype,false,P,m);
	      ptr+=offset.intValue();
	      to+=offset; m=m.newAddTo(offset);
	    }
	  }
	  if(notPromoteCapture) return;
	  while(ptr!=limitPtr){
	    assert(from.canPromote<P>() || to.canPromote<P>());
	    action.simpleMove(from,to,::osl::promote(ptype),true,P,m.promote());
	    ptr+=offset.intValue();
	    to+=offset;
	    m=m.newAddTo(offset);
	  }
	  Piece p1= *limitPtr;
	  if(p1.canMoveOn<P>()){
	    m=m.newAddCapture(p1);
	    assert(from.canPromote<P>() || to.canPromote<P>());
	    action.unknownMove(from,to,p1,::osl::promote(ptype),true,P,m.promote());
	  }
	}
	else{ // NoPromote
	  while(ptr!=limitPtr){
	    action.simpleMove(from,to,ptype,false,P,m);
	    ptr+=offset.intValue();
	    to+=offset; m=m.newAddTo(offset);
	  }
	  if(notPromoteCapture) return;
	  Piece p1= *limitPtr;
	  if(p1.canMoveOn<P>()){
	    m=m.newAddCapture(p1);
	    action.unknownMove(from,to,p1,ptype,false,P,m);
	  }
	}
      }
      template <Player P,class Action,PromoteType CanP,Direction Dir,bool notPromoteCapture>
      inline void 
      generateLong(NumEffectState const&,Piece,const Piece *, Square,Action&,Int2Type<false>,Move,Ptype)
      {
      }

      template <Player P,Ptype T,class Action,PromoteType CanP,Direction Dir,bool notPromoteCapture>
      inline void
      generateLong(NumEffectState const& state,Piece p,const Piece *ptr, Square pos,Action& action,Move moveBase,Ptype ptype)
      {
	generateLong<P,Action,
	  (CanP != CheckPromoteType ? CanP : (DirectionTraits<Dir>::canPromoteTo ? CheckPromoteType : NoPromoteType)),
	  Dir,notPromoteCapture>(state,p,ptr,pos,action,
	       Int2Type<(PtypeTraits<T>::moveMask & DirectionTraits<Dir>::mask) !=0>(),moveBase,ptype);
    }

      /**
       * 短い利きの動き
       * CanPromoteType - promote可能な動きの時
       * MustPromoteType - 2段目の歩，3,4段目の桂馬
       */
      template <Player P,class Action,PromoteType CanP,Direction Dir,bool notPromoteCapture>
      void 
      generateShort(const Piece *ptr,Square from,Action& action,Int2Type<true>,Move moveBase,Ptype ptype)
      {
	const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
	Piece p1=ptr[offset.intValue()];	
	Square to=from+offset;
	Move m=moveBase.newAddTo(offset).newAddCapture(p1);
	if ((notPromoteCapture ? p1.isEmpty() : p1.canMoveOn<P>())){
	  if (!notPromoteCapture && (CanP==CanPromoteType || CanP==MustPromoteType))
	    action.unknownMove(from,to,p1,::osl::promote(ptype),true,P,m.promote());
	  if (CanP!=MustPromoteType)
	    action.unknownMove(from,to,p1,ptype,false,P,m);
	}
      }

      template <Player P,class Action,PromoteType CanP,Direction Dir,bool notPromoteCapture>
    void 
      generateShort(const Piece */*ptr*/,Square /*from*/,Action& /*action*/,Int2Type<false>,Move /*moveBase*/,Ptype /*ptype*/)
    {
    }

      template <Player P,Ptype T,class Action,PromoteType CanP,Direction Dir,bool notPromoteCapture>
    void 
      generateShort(const Piece *ptr,Square from,Action& action,Move moveBase,Ptype /*ptype*/)
    {
      generateShort<P,Action,
	(CanP != CheckPromoteType ? CanP : (DirectionTraits<Dir>::canPromoteTo ? CanPromoteType : NoPromoteType)),
	Dir,notPromoteCapture>(ptr,from,action,
	     Int2Type<(PtypeTraits<T>::moveMask & DirectionTraits<Dir>::mask) !=0>(),
	     moveBase,T);
  }

      template <Player P,Ptype T,class Action,PromoteType CanP,bool useDirMask,bool notPromoteCapture>
    inline void
    generatePtypePromote(const NumEffectState& state,Piece p, Action& action,Square from,int dirMask)
    {
      const Ptype ptype=(T==GOLD ? p.ptype() : T);
      Move moveBase=Move(from,from,ptype,(Ptype)0,false,P);
      const Piece *ptr=state.getPiecePtr(from);
      if(!useDirMask || (dirMask&(1<<UL))==0){
	generateShort<P,T,Action,CanP,UL,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateShort<P,T,Action,CanP,DR,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_UL,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_DR,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
      }
      if(!useDirMask || (dirMask&(1<<UR))==0){
	generateShort<P,T,Action,CanP,UR,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateShort<P,T,Action,CanP,DL,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_UR,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_DL,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
      }
      if(!useDirMask || (dirMask&(1<<U))==0){
	generateShort<P,T,Action,CanP,U,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateShort<P,T,Action,CanP,D,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_U,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_D,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
      }
      if(!useDirMask || (dirMask&(1<<L))==0){
	generateShort<P,T,Action,CanP,L,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateShort<P,T,Action,CanP,R,notPromoteCapture>(ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_L,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
	generateLong<P,T,Action,CanP,LONG_R,notPromoteCapture>(state,p,ptr,from,action,moveBase,ptype);
      }
      generateShort<P,T,Action,CanP,UUL,notPromoteCapture>(ptr,from,action,moveBase,ptype);
      generateShort<P,T,Action,CanP,UUR,notPromoteCapture>(ptr,from,action,moveBase,ptype);
    }
      
      template <Player P,Direction Dir,class Action,bool notPromoteCapture>
  inline void
  generateKingDir(const Piece *ptr, Square from,Action& action,unsigned int liberty,Move const& moveBase)
  {
    if((liberty&(1<<Dir))!=0){
      const Offset offset=DirectionPlayerTraits<Dir,P>::offset();
      Move m=moveBase.newAddTo(offset);
      Square to=from+offset;
      Piece p1=ptr[offset.intValue()];
      assert(p1.canMoveOn<P>());
      if(notPromoteCapture && !p1.isEmpty()) return;
      m=m.newAddCapture(p1);
      action.unknownMove(from,to,p1,KING,false,P,m);
    }
  }

      template <Player P,class Action,bool useDirMask,bool notPromoteCapture>
  inline void
  generateKing(const NumEffectState& state, Action& action,Square pos,int dirMask)
  {
    King8Info king8info(state.Iking8Info(P));
    unsigned int liberty=king8info.liberty();
    Move moveBase(pos,pos,KING,(Ptype)0,false,P);
    const Piece *ptr=state.getPiecePtr(pos);
    if(!useDirMask || (dirMask&(1<<UL))==0){
      generateKingDir<P,UL,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
      generateKingDir<P,DR,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
    }
    if(!useDirMask || (dirMask&(1<<U))==0){
      generateKingDir<P,U,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
      generateKingDir<P,D,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
    }
    if(!useDirMask || (dirMask&(1<<UR))==0){
      generateKingDir<P,UR,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
      generateKingDir<P,DL,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
    }
    if(!useDirMask || (dirMask&(1<<L))==0){
      generateKingDir<P,L,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
      generateKingDir<P,R,Action,notPromoteCapture>(ptr,pos,action,liberty,moveBase);
    }
  }

      template <Player P,class Action,bool useDirMask,bool notPromoteCapture>
  inline void
  generateLance(const NumEffectState& state, Piece p,Action& action,Square from,int dirMask)
  {
    if(!useDirMask || (dirMask&(1<<U))==0){
      const Offset offset=DirectionPlayerTraits<U,P>::offset();
      Square limit=state.mobilityOf((P==BLACK ? U : D),p.number());
      Square to=limit;
      Piece p1=state.pieceAt(to);
      int limity=(P==BLACK ? to.y() : 10-to.y());
      int fromy=(P==BLACK ? from.y() : 10-from.y());
      int ycount=fromy-limity-1;
      Move m(from,to,LANCE,(Ptype)0,false,P);
      switch(limity){
      case 4: case 5: case 6: case 7: case 8: case 9:{
	if(!notPromoteCapture && p1.canMoveOn<P>())
	  action.unknownMove(from,to,p1,LANCE,false,P,m.newAddCapture(p1));
	m=m.newAddTo(-offset); to-=offset;
	goto escape4;
      }
      case 3:
	if(!notPromoteCapture && p1.canMoveOn<P>()){
	  Move m1=m.newAddCapture(p1);
	  action.unknownMove(from,to,p1,PLANCE,true,P,m1.promote());
	  action.unknownMove(from,to,p1,LANCE,false,P,m1);
	}
	m=m.newAddTo(-offset); to-=offset;
	goto escape4;
      case 2:
	if(!notPromoteCapture && p1.canMoveOn<P>()){
	  Move m1=m.newAddCapture(p1);
	  action.unknownMove(from,to,p1,PLANCE,true,P,m1.promote());
	}
	if(fromy==3) return;
	m=m.newAddTo(-offset); to-=offset;
	ycount=fromy-4;
	goto escape2;
      case 0: 
	m=m.newAddTo(-offset); to-=offset;
	if(!notPromoteCapture)
	  action.simpleMove(from,to,PLANCE,true,P,m.promote());
	goto join01;
      case 1: 
	if(!notPromoteCapture && p1.canMoveOn<P>()){
	  action.unknownMove(from,to,p1,PLANCE,true,P,m.newAddCapture(p1).promote());
	}
      join01:
	if(fromy==2) return;
	m=m.newAddTo(-offset); to-=offset;
	if(fromy==3){
	  if(!notPromoteCapture)
	    action.simpleMove(from,to,PLANCE,true,P,m.promote());
	  return;
	}
	ycount=fromy-4;
	goto escape01;
      default: assert(0);
      }
    escape01:
      if(!notPromoteCapture)
	action.simpleMove(from,to,PLANCE,true,P,m.promote());
      m=m.newAddTo(-offset); to-=offset;
    escape2:
      if(!notPromoteCapture)
	action.simpleMove(from,to,PLANCE,true,P,m.promote());
      action.simpleMove(from,to,LANCE,false,P,m);
      m=m.newAddTo(-offset); to-=offset;
    escape4:
      while(ycount-->0){
	action.simpleMove(from,to,LANCE,false,P,m);
	m=m.newAddTo(-offset);
	to-=offset;
      }
    }
    return;
  }

      template <Player P,class Action,bool useDirMask,bool notPromoteCapture>
  inline void
  generatePawn(const NumEffectState& state, Piece p,Action& action,Square from,int dirMask)
  {
    assert(from == p.square());
    if(!useDirMask || (dirMask&(1<<U))==0){
      if(notPromoteCapture && (P==BLACK ? from.yLe<4>() : from.yGe<6>())) return;
      const Offset offset=DirectionPlayerTraits<U,P>::offset();
      Square to=from+offset;
      Piece p1=state.pieceAt(to);
      if(notPromoteCapture){
	if(p1.isEmpty())
	  action.simpleMove(from,to,PAWN,false,P);
	return;
      }
      if(p1.canMoveOn<P>()){
	if(P==BLACK ? to.yLe<3>() : to.yGe<7>()){ // canPromote
	  if(notPromoteCapture) return;
	  Move m(from,to,PPAWN,PTYPE_EMPTY,true,P);
	  action.unknownMove(from,to,p1,PPAWN,true,P,
			     m.newAddCapture(p1));
	}
	else{
	  Move m(from,to,PAWN,PTYPE_EMPTY,false,P);
	  action.unknownMove(from,to,p1,PAWN,false,P,m.newAddCapture(p1));
	}
      }
    }
  }
}
    template <class Action,bool notPromoteCapture>
  template <Player P,Ptype T,bool useDirMask>
    void PieceOnBoard<Action,notPromoteCapture>::
  generatePtypeUnsafe(const NumEffectState& state,Piece p, Action& action,int dirMask)
  {
    using piece_on_board::generatePtypePromote;
    using piece_on_board::generateKing;
    using piece_on_board::generateLance;
    using piece_on_board::generatePawn;
    const Square from=p.square();
    if(T==KING){
      generateKing<P,Action,useDirMask,notPromoteCapture>(state,action,from,dirMask);
    }
    else if(T==LANCE){
      generateLance<P,Action,useDirMask,notPromoteCapture>(state,p,action,from,dirMask);
    }
    else if(T==PAWN){
      generatePawn<P,Action,useDirMask,notPromoteCapture>(state,p,action,from,dirMask);
    }
    else if(canPromote(T)){
      if(PtypePlayerTraits<T,P>::mustPromote(from))
	generatePtypePromote<P,T,Action,MustPromoteType,useDirMask,notPromoteCapture>(state,p,action,from,dirMask);
      else if(PtypePlayerTraits<T,P>::canPromote(from))
	generatePtypePromote<P,T,Action,CanPromoteType,useDirMask,notPromoteCapture>(state,p,action,from,dirMask);
      else if(PtypePlayerTraits<T,P>::checkPromote(from))
	generatePtypePromote<P,T,Action,CheckPromoteType,useDirMask,notPromoteCapture>(state,p,action,from,dirMask);
      else
	generatePtypePromote<P,T,Action,NoPromoteType,useDirMask,notPromoteCapture>(state,p,action,from,dirMask);
    }
    else
      generatePtypePromote<P,T,Action,NoPromoteType,useDirMask,notPromoteCapture>(state,p,action,from,dirMask);
  }

    template <class Action,bool notPromoteCapture>
template <Player P,Ptype T,bool useDirMask>
    void PieceOnBoard<Action,notPromoteCapture>::
generatePtype(const NumEffectState& state,Piece p, Action& action,int dirMask)
{
  int num=p.number();
//  if(T==SILVER) std::cerr << "p=" << p << std::endl;
  if(state.pin(P).test(num)){
    if(T==KNIGHT) return;
    Direction d=state.pinnedDir<P>(p);
    dirMask|=(~(1<<primDir(d)));
//    std::cerr << "pinned direction=" << d << ",dirMask=" << dirMask << std::endl;
    generatePtypeUnsafe<P,T,true>(state,p,action,dirMask);
  }
  else{
    generatePtypeUnsafe<P,T,useDirMask>(state,p,action,dirMask);
  }
}
    template <class Action,bool notPromoteCapture>
template <Player P,bool useDirmask>
    void PieceOnBoard<Action,notPromoteCapture>::
generate(const NumEffectState& state,Piece p, Action& action,int dirMask)
{
      
  switch(p.ptype()){
  case PPAWN: case PLANCE: case PKNIGHT: case PSILVER: case GOLD:
    generatePtype<P,GOLD,useDirmask>(state,p,action,dirMask); break;
  case PAWN: 
    generatePtype<P,PAWN,useDirmask>(state,p,action,dirMask); break;
  case LANCE: 
    generatePtype<P,LANCE,useDirmask>(state,p,action,dirMask); break;
  case KNIGHT: 
    generatePtype<P,KNIGHT,useDirmask>(state,p,action,dirMask); break;
  case SILVER: 
    generatePtype<P,SILVER,useDirmask>(state,p,action,dirMask); break;
  case BISHOP: 
    generatePtype<P,BISHOP,useDirmask>(state,p,action,dirMask); break;
  case PBISHOP: 
    generatePtype<P,PBISHOP,useDirmask>(state,p,action,dirMask); break;
  case ROOK: 
    generatePtype<P,ROOK,useDirmask>(state,p,action,dirMask); break;
  case PROOK: 
    generatePtype<P,PROOK,useDirmask>(state,p,action,dirMask); break;
  case KING: 
    generatePtype<P,KING,useDirmask>(state,p,action,dirMask); break;
  default: break;
  }
}
} // namespace move_generator
} // namespace osl

#endif /* _GENERATE_PIECE_MOVES_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

