/* immediateCheckmate.tcc
 */
#ifndef OSL_CHECKMATE_IMMEDIATE_CHECKMATE_TCC
#define OSL_CHECKMATE_IMMEDIATE_CHECKMATE_TCC
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/immediateCheckmateTable.h"
#include "osl/move_classifier/kingOpenMove.h"
#include "osl/bits/directionTraits.h"
#include "osl/bits/pieceTable.h"
#include "osl/bits/mask.h"

namespace osl
{
  namespace checkmate
  {
    namespace detail {
      using osl::misc::BitOp;
      template<Player P>
      bool blockingVerticalAttack(NumEffectState const& state,Square pos)
      {
	PieceMask effect=state.effectSetAt(pos)&
	  state.effectSetAt(pos+DirectionPlayerTraits<U,P>::offset());
	mask_t mask=effect.getMask(1);  // longは常に1
	mask&=(state.piecesOnBoard(P).getMask(1)<<8);
	if((mask&mask_t::makeDirect(PtypeFuns<LANCE>::indexMask<<8)).none()){
	  mask&=mask_t::makeDirect(PtypeFuns<ROOK>::indexMask<<8);
	  while(mask.any()){
	    int num=mask.takeOneBit()+NumBitmapEffect::longToNumOffset;
	    Square from=state.pieceOf(num).square();
	    assert(from.isOnBoard());
	    if(from.isU<P>(pos)) goto found;
	  }
	  return false;
	found:;
	}
	const Offset offset=DirectionPlayerTraits<U,P>::offset();
	pos+=offset;
	const Player altP=alt(P);
	for(int i=0;i<3;i++,pos+=offset){
	  Piece p=state.pieceAt(pos);
	  if(p.canMoveOn<altP>()){ // 自分の駒か空白
	    if(state.countEffect(P,pos)==1) return true;
	    if(!p.isEmpty()) return false;
	  }
	  else return false;
	}
	return false;
      }
      template<Player P>
      bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      blockingDiagonalAttack(NumEffectState const& state,Square pos,Square target,
			     King8Info canMoveMask)
      {
	const Player altP=alt(P);
	Square to=target-DirectionPlayerTraits<U,P>::offset();
	// Uに相手の駒がある
	if((canMoveMask.uint64Value()&(0x10000<<U))==0) return false;
	PieceMask effect=state.effectSetAt(to)&state.effectSetAt(pos);
	mask_t mask=effect.getMask(1);  // longは常に1
	mask&=(state.piecesOnBoard(P).getMask(1)<<8);
	mask&=mask_t::makeDirect(PtypeFuns<BISHOP>::indexMask<<8);
	while(mask.any()){
	  int num=mask.takeOneBit()+NumBitmapEffect::longToNumOffset;
	  Square from=state.pieceOf(num).square();
	  assert(from.isOnBoard());
	  Offset offset=Board_Table.getShort8OffsetUnsafe(to,from);
	  if(to+offset != pos) continue;
	  if(state.countEffect(P,to)==1) return true;
	  // Uがspaceだと絡んでくる
	  if(!state.pieceAt(to).isEmpty()) return false;
	  Square pos1=to-offset;
	  // BISHOPの利き一つで止めていた
	  Piece p=state.pieceAt(pos1);
	  if(p.canMoveOn<altP>() &&
	     state.countEffect(P,pos1)==1){
	    return true;
	  }
	}
	return false;
      }
      template<Player P,bool canDrop,bool setBestMove>
      bool hasKnightCheckmate(NumEffectState const& state, 
			      Square target, 
			      Square pos,
			      King8Info canMoveMask,
			      Move& bestMove, mask_t mask1)
      {
	if(!pos.isOnBoard()) return false;
	const Player altP=alt(P);
	Piece p=state.pieceAt(pos);
	if(p.canMoveOn<P>() && 
	   !state.hasEffectByNotPinned(altP,pos)
	  ){
	  mask_t mask=state.effectSetAt(pos).getMask<KNIGHT>()&mask1;
	  if(mask.any()){
	    if(blockingVerticalAttack<P>(state,pos) ||
	       blockingDiagonalAttack<P>(state,pos,target,canMoveMask)) return false;
	    if(setBestMove){
	      int num=mask.takeOneBit()+(PtypeFuns<KNIGHT>::indexNum<<5);
	      Piece p1=state.pieceOf(num);
	      Square from=p1.square();
	      bestMove=Move(from,pos,KNIGHT,p.ptype(),false,P);
	    }
	    return true;
	  }
	  else if(canDrop && p.isEmpty()){
	    if(blockingVerticalAttack<P>(state,pos) ||
	       blockingDiagonalAttack<P>(state,pos,target,canMoveMask)) return false;
	    if(setBestMove)
	      bestMove=Move(pos,KNIGHT,P);
	    return true;
	  }
	}
	return false;
      }
      // KNIGHT
      // KNIGHTのdropは利きを遮ることはない
      template<Player P,bool setBestMove>
      bool hasCheckmateMoveKnight(NumEffectState const& state, Square target, 
				  King8Info canMoveMask,Move& bestMove)
      {
	// 8近傍に移動できる時は桂馬の一手詰めはない
	if((canMoveMask.uint64Value()&0xff00)!=0) return false;
	mask_t mask=mask_t::makeDirect(PtypeFuns<KNIGHT>::indexMask);
	mask&=state.piecesOnBoard(P).getMask<KNIGHT>();
	mask&= ~state.promotedPieces().getMask<KNIGHT>();
	mask&= ~state.pinOrOpen(P).getMask<KNIGHT>();
	if(state.hasPieceOnStand<KNIGHT>(P)){
	  Square pos=target-DirectionPlayerTraits<UUR,P>::offset();
	  if(hasKnightCheckmate<P,true,setBestMove>(state,target,pos,canMoveMask,bestMove,mask))
	    return true;
	  pos=target-DirectionPlayerTraits<UUL,P>::offset();
	  return hasKnightCheckmate<P,true,setBestMove>(state,target,pos,canMoveMask,bestMove,mask);
	}
	else{
	  Square pos=target-DirectionPlayerTraits<UUR,P>::offset();
	  if(hasKnightCheckmate<P,false,setBestMove>(state,target,pos,canMoveMask,bestMove,mask))
	    return true;
	  pos=target-DirectionPlayerTraits<UUL,P>::offset();
	  return hasKnightCheckmate<P,false,setBestMove>(state,target,pos,canMoveMask,bestMove,mask);
	}
	return false;
      }
      template<Player P,bool setBestMove>
      bool slowCheckDrop(NumEffectState const& state,Square target,
			 Ptype ptype,King8Info canMoveMask,Move& bestMove)
      {
	unsigned int dropMask=(canMoveMask.uint64Value()&0xff)
	  &Immediate_Checkmate_Table.ptypeDropMask(ptype,canMoveMask);
	// dropMaskが0ならここに来ない
	assert(dropMask!=0);
	while(dropMask!=0){
	  int i=BitOp::takeOneBit(dropMask);
	  Direction d=static_cast<Direction>(i);
	  unsigned int blockingMask=Immediate_Checkmate_Table.blockingMask(ptype,d) &
	    (canMoveMask.uint64Value()>>16);
	  Square drop=target-Board_Table.getOffset<P>(d);
	  if(blockingMask!=0){
	    NumBitmapEffect effect=state.effectSetAt(drop);
	    mask_t longEffect=effect.getMask(1)&NumBitmapEffect::longEffectMask();
	    longEffect&=(state.piecesOnBoard(P).getMask(1)<<8);
	    if(longEffect.any()){
	      do{
		int j=BitOp::takeOneBit(blockingMask);
		Direction d1=static_cast<Direction>(j);
		Square pos=target-Board_Table.getOffset<P>(d1);
		NumBitmapEffect effect1=state.effectSetAt(pos);
		if(effect1.countEffect(P)>1) continue;
		mask_t longEffect1=effect1.getMask(1)&longEffect;
		if(!longEffect1.any()) continue;
		//
		int num=longEffect1.takeOneBit()+NumBitmapEffect::longToNumOffset;
		if(Board_Table.isBetween(drop,state.pieceOf(num).square(),pos))
		  goto tryNext;
	      }while(blockingMask!=0);
	    }
	  }
	  // blockingMaskの点がすべてOKならOK
	  if(setBestMove)
	    bestMove=Move(drop,ptype,P);
	  return true;
	tryNext:;
	}
	return false;
      }
    } // detail
  } // checkmate
} // osl

// not KNIGHT
template<osl::Player P,bool setBestMove>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateDrop(NumEffectState const& state, Square target,
		 King8Info canMoveMask,Move& bestMove)
{
  typedef misc::GeneralMask<unsigned short> mask_t;
  mask_t dropPtypeMask=mask_t::makeDirect(Immediate_Checkmate_Table.dropPtypeMask(canMoveMask));
  while(dropPtypeMask.any()){
    Ptype ptype=static_cast<Ptype>(dropPtypeMask.takeOneBit()+PTYPE_BASIC_MIN);
    if(state.hasPieceOnStand(P,ptype) &&
       detail::slowCheckDrop<P,setBestMove>(state,target,ptype,canMoveMask,
					    bestMove))
      return true;
  }
  return false;
}

template<osl::Player P,bool setBestMove>
bool osl::checkmate::ImmediateCheckmate::
slowHasCheckmateMoveDirPiece(NumEffectState const& state, Square target,
			     King8Info canMoveMask,Direction d,Square pos,Piece p,Ptype ptype,Move& bestMove){
  const Player altP=alt(P);
  // ptypeがPROOKの時は，更なるチェックが必要
  if(ptype==PROOK){
    int dx=target.x()-pos.x();
    int dy=target.y()-pos.y();
    if(abs(dx)==1 && abs(dy)==1){
      {
	Square pos1=pos+Offset(dx,0);
	Piece p1=state.pieceAt(pos1);
	if(!p1.isEmpty()){
	  {
	    //  * -OU *
	    // (A)(B)(D)
	    //  * (C) *
	    // (E) *  *
	    // +RY (C) -> (A), (E) -> (A)
	    // -?? - (B)
	    // (D) - 竜以外の利きなし 
	    Square pos2=pos+Offset(2*dx,0);
	    if(state.pieceAt(pos2).template canMoveOn<altP>()){
	      NumBitmapEffect effect2=state.effectSetAt(pos2);
	      if(effect2.countEffect(P)==0 ||
		 (effect2.countEffect(P)==1 &&
		  effect2.test(p.number())))
		return false;
	    }
	  }
	  {
	    //  * -OU *
	    // (A)(B) *
	    //  * (C) *
	    // +RY (C) -> (A)
	    // -?? - (B)竜でpinされているが実はAへの利き持つ
	    if(p.square()==target-Offset(0,2*dy) &&
	       state.hasEffectByPiece(p1,pos))
	      return false;
	  }
	}
      }
      {
	Square pos1=pos+Offset(0,dy);
	Piece p1=state.pieceAt(pos1);
	if(!p1.isEmpty()){
	  Square pos2=pos+Offset(0,2*dy);
	  {
	    if(state.pieceAt(pos2).template canMoveOn<altP>()){
	      NumBitmapEffect effect2=state.effectSetAt(pos2);
	      if(effect2.countEffect(P)==0 ||
		 (effect2.countEffect(P)==1 &&
		  effect2.test(p.number())))
		return false;

	    }
	    {
	      // (C)(B)-OU
	      //  * (A) *
	      // +RY (C) -> (A)
	      // -?? - (B)竜でpinされているが実はAへの利き持つ
	      if(p.square()==target-Offset(2*dx,0) &&
		 state.hasEffectByPiece(p1,pos))
		return false;
	    }
	  }
	}
      }
    }
  }
  // 元々2つの利きがあったマスが，
  // block & 自分の利きの除去で利きがなくなることはあるか?
  // -> ある．
  // +KA  *   *
  //  *  (A) +KI
  //  *  -OU (B)
  //  *   *   *
  // で金がAに移動して王手をかけると，Bの利きが2から0になる．
  mask_t mask=mask_t::makeDirect((canMoveMask.uint64Value()>>16)&Immediate_Checkmate_Table.noEffectMask(ptype,d));
  if(mask.any()){
    int num=p.number();
    NumBitmapEffect effect2=state.effectSetAt(pos);
    effect2.reset(num+8);
    mask_t longEffect2=effect2.getMask(1)&NumBitmapEffect::longEffectMask();
    longEffect2&=(state.piecesOnBoard(P).getMask(1)<<8);
    do {
      Direction d1=static_cast<Direction>(mask.takeOneBit());
      Square pos1=target-Board_Table.getOffset<P>(d1);
      NumBitmapEffect effect1=state.effectSetAt(pos1);
      int count=effect1.countEffect(P);
      // 自分の利きの除去
      if(effect1.test(num)) count--;
      if(count==0) return false;
      // blockしている利きの除去
      mask_t longEffect1=effect1.getMask(1)&longEffect2;
      while(longEffect1.any()){
	int num1=longEffect1.takeOneBit()+NumBitmapEffect::longToNumOffset;
	if(Board_Table.isBetween(pos,state.pieceOf(num1).square(),pos1))
	  count--;
	if(count==0) return false;
      }
    } while (mask.any());
  }
  // 自殺手でないことのチェックを入れる
  if(move_classifier::KingOpenMove<P>::isMember(state,ptype,p.square(),pos)) return false;
  if(setBestMove){
    bestMove=Move(p.square(),pos,ptype,
		  state.pieceAt(pos).ptype(),
		  ptype!=p.ptype(),P);
  }
  return true;
}

template<osl::Player P,bool setBestMove>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMoveDirPiece(NumEffectState const& state, Square target,
			 King8Info canMoveMask,Direction d,Square pos,Piece p,Move& bestMove){
  Square from=p.square();
  Ptype ptype=p.ptype();
  // 相手の利きが伸びてしまって移動後も利きがついてくる可能性
  {
    const Player altP=alt(P);
    Direction d1=Board_Table.getShort8Unsafe<P>(from,pos);
    if(d1!=DIRECTION_INVALID_VALUE){ // not knight move
      int num=state.longEffectNumTable()[p.number()][P==BLACK ? d1 : inverse(d1)];
      if(num != EMPTY_NUM && state.pieceOf(num).isOnBoardByOwner<altP>())
	return false;
    }
  }
  if(canPromote(ptype) &&
     (from.canPromote<P>() || pos.canPromote<P>())){
    Ptype pptype=promote(ptype);
    if((((canMoveMask.uint64Value()>>8)|0x100)&
	Immediate_Checkmate_Table.noEffectMask(pptype,d))==0){
      if(slowHasCheckmateMoveDirPiece<P,setBestMove>(state,target,canMoveMask,d,pos,p,pptype,bestMove)) return true;
    }
    if (ptype==PAWN || /*basic because canpromote*/isMajorBasic(ptype)) 
      return false;
  }
  if((((canMoveMask.uint64Value()>>8)|0x100)&
      Immediate_Checkmate_Table.noEffectMask(ptype,d))==0){
    if(slowHasCheckmateMoveDirPiece<P,setBestMove>(state,target,canMoveMask,d,pos,p,ptype,bestMove)) return true;
  }
  return false;
}

template<osl::Player P,bool setBestMove>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMoveDir(NumEffectState const& state, Square target,
		    King8Info canMoveMask,Direction d,Move& bestMove){
  Square pos=target-Board_Table.getOffset<P>(d);
  if(state.countEffect(P,pos)<2 &&
     !effect_util::AdditionalEffect::hasEffect(state,pos,P)) return false;
  PieceMask pieceMask=state.piecesOnBoard(P)&state.effectSetAt(pos);
  assert(pos.isOnBoard());
  // 玉で王手をかけない
  pieceMask.reset(KingTraits<P>::index);
  for(int i=0;i<=PieceMask::numToIndex(40);i++){
    mask_t mask=pieceMask.getMask(i);
    while (mask.any()){
      const int num=mask.takeOneBit()+i*32;
      if(hasCheckmateMoveDirPiece<P,setBestMove>(state,target,canMoveMask,d,pos,state.pieceOf(num),bestMove)) return true;
    }
  }
  return false;
}

// not KNIGHT
template<osl::Player P,bool setBestMove>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMove(NumEffectState const& state, Square target,
		 King8Info canMoveMask,Move& bestMove)
{
  assert(! state.inCheck());
  typedef misc::GeneralMask<unsigned int> mask_t;
  mask_t mask2=mask_t::makeDirect((canMoveMask.uint64Value()>>24)&0xff);
  while(mask2.any()){
    Direction d=static_cast<Direction>(mask2.takeOneBit());
    if(hasCheckmateMoveDir<P,setBestMove>(state,target,canMoveMask,d,bestMove)) return true;
  }
  return false;
}

template<osl::Player P>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMove(NumEffectState const& state, King8Info canMoveMask)
{
  const Player altP=alt(P);
  const Square target=state.kingSquare(altP);
  assert(target.isOnBoard());
  // 相手からの王手がかかっていない
  Move dummy;
  if(hasCheckmateMove<P,false>(state,target,canMoveMask,dummy)) return true;
  if(detail::hasCheckmateMoveKnight<P,false>(state,target,canMoveMask,dummy)) return true;
  return hasCheckmateDrop<P,false>(state,target,canMoveMask,dummy);
}

template<osl::Player P>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMove(NumEffectState const& state)
{
  const Player altP=alt(P);
#ifndef NDEBUG
  const Square target=state.kingSquare(altP);
#endif
  assert(target.isOnBoard());
  King8Info canMoveMask(state.Iking8Info(altP));
  return hasCheckmateMove<P>(state, canMoveMask);
}

template<osl::Player P>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMove(NumEffectState const& state, King8Info canMoveMask,
		 Square target, Move& bestMove)
{
  assert(! state.inCheck());
  assert(target.isOnBoard());

  if(hasCheckmateMove<P,true>(state,target,canMoveMask,bestMove)) return true;
  if(detail::hasCheckmateMoveKnight<P,true>(state,target,canMoveMask,bestMove)) return true;
  return  hasCheckmateDrop<P,true>(state,target,canMoveMask,bestMove);
}

template<osl::Player P>
bool osl::checkmate::ImmediateCheckmate::
hasCheckmateMove(NumEffectState const& state,Move& bestMove)
{
  const Player altP=alt(P);
  const Square target=state.kingSquare(altP);
  King8Info canMoveMask(state.Iking8Info(altP));
  return hasCheckmateMove<P>(state, canMoveMask, target, bestMove);
}

#endif /* OSL_CHECKMATE_IMMEDIATE_CHECKMATE_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

