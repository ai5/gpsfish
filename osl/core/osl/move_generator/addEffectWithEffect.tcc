#ifndef _GENERATER_ADD_EFFECT_WITH_EFFECT_TCC
#define _GENERATER_ADD_EFFECT_WITH_EFFECT_TCC

#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_generator/open.h"
#include "osl/move_generator/open.tcc"
#include "osl/move_generator/pieceOnBoard.tcc"

//#define GENERATE_PAWNDROP_CHECKMATE

namespace osl
{
  namespace move_generator
  {
    namespace detail
    {
      /**
       * マスtoに移動可能な駒pを移動する手を生成する．
       * ptypeMaskで指定されたptypeになる場合以外は手を生成しない．
       * @param state - 盤面
       * @param p - 利きを持つコマ
       * @param to - 目的のマス
       * @param toP - 目的のマスに現在ある駒(又は空白)
       * @param action - 手生成のaction(典型的にはstoreかfilterつきstore)
       * @param ptypeMask - 移動後の駒のptypeに対応するbitが1なら手を生成する
       * should promoteは?
       * 呼び出す時はpinnedの場合のunsafeなdirectionは排除済み
       */
      template<osl::Player P,class Action>
      void generateMovePiecePtypeMask(const NumEffectState& state,Piece p,Square to,Piece toP,Action& action,unsigned int ptypeMask)
      {
	assert(p.isOnBoardByOwner<P>());
	assert(toP==state.pieceAt(to));
	Ptype ptype=p.ptype();
	Square from=p.square();
	if(canPromote(ptype) &&
	   (to.canPromote<P>() || from.canPromote<P>())){
	  Ptype pptype=osl::promote(ptype);
	  if(((1<<pptype)&ptypeMask)!=0)
	    action.unknownMove(from,to,toP,pptype,true,P);
	  if(Move::ignoreUnpromote<P>(ptype,from,to)) return;
	}
	// 
	if(((1<<ptype)&ptypeMask)!=0)
	  action.unknownMove(p.square(),to,toP,ptype,false,P);
      }
      /**
       * あるマスに利きを持つすべての駒の中で，
       * ptypeMaskで指定されたptypeになる場合は移動する手を生成する
       * @param state - 盤面
       * @param to - 目的のマス
       * @param toP - 目的のマスに現在ある駒(又は空白)
       * @param action - 手生成のaction(典型的にはstoreかfilterつきstore)
       * @param ptypeMask - 移動後の駒のptypeに対応するbitが1なら手を生成する
       * pinnedの場合は移動する手が1手もない場合もある．
       */
      template<osl::Player P,class Action>
      void generateMoveToPtypeMaskWithPieceMask(const NumEffectState& state,Square to,Piece toP,Action& action,unsigned int ptypeMask,PieceMask pieceMask)
      {
	if(pieceMask.test(KingTraits<P>::index)){
	  const Player altP=alt(P);
	  if(!state.hasEffectAt<altP>(to)){
	    action.unknownMove(state.kingSquare<P>(),to,toP,KING,false,P);
	  }
	  pieceMask.reset(KingTraits<P>::index);
	}
	while (pieceMask.any()){
	  const int num=pieceMask.takeOneBit();
	  Piece p=state.pieceOf(num);
	  if(state.pinOrOpen(P).test(num)){
	    Direction d=state.pinnedDir<P>(p);
	    Direction d1=Board_Table.template getShort8Unsafe<P>(p.square(),to);
	    if(primDir(d)!=primDirUnsafe(d1)) continue;
	  }
	  generateMovePiecePtypeMask<P,Action>(state,p,to,toP,action,ptypeMask);
	}
      }
      template<osl::Player P,class Action>
      void generateMoveToPtypeMask(const NumEffectState& state,Square to,Piece toP,Action& action,unsigned int ptypeMask)
      {
	PieceMask pieceMask=state.piecesOnBoard(P)&state.effectSetAt(to);
	const Player altP=alt(P);
	pieceMask.reset(KingTraits<P>::index); // 玉は除く
	pieceMask &= ~state.pinOrOpen(altP); // open atackからのものを除く
	generateMoveToPtypeMaskWithPieceMask<P,Action>(state,to,toP,action,ptypeMask,pieceMask);
      }
#ifndef GENERATE_PAWNDROP_CHECKMATE
      /**
       * 敵玉の前に歩を置いた場合に遮った利きで敵玉にlibertyが生まれるかどうか?
       */
      template<osl::Player P>
      bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      blockingU(const NumEffectState& state,Square pos)
      {
	const osl::Player altP=alt(P);
	NumBitmapEffect effect=state.effectSetAt(pos);
	mask_t mask=(effect.getMask(1)& NumBitmapEffect::longEffectMask());
	mask&=state.piecesOnBoard(P).getMask(1)<<8; // ROOK, BISHOPの利きのみのはず
	while(mask.any()){
	  int num=mask.takeOneBit()+NumBitmapEffect::longToNumOffset;
	  Square from=state.pieceOf(num).square();
	  if( (P==BLACK ? from.y()>=pos.y() : pos.y()>=from.y()) ){
	    Square shadowPos=pos+Board_Table.getShortOffset(Offset32(pos,from));
	    assert((P==BLACK ? shadowPos.y()<=pos.y() : pos.y()<=shadowPos.y()) );
	    Piece p=state.pieceAt(shadowPos);
	    if(p.canMoveOn<altP>() && !state.hasMultipleEffectAt(P,shadowPos)){
	      return true;
	    }
	  }
	}
	return false;
      }
#endif
      /**
       * int DirType : 0  - U
       *               1  - LRD
       *               2  - UL,UR,DL,DR
       * dirOffset = DirectionPlayerTraits<Dir,P>::offset()
       */
      template<osl::Player P,int DirType,class Action>
      void generateDir(const NumEffectState& state,Square target,Action& action,bool& hasPawnCheckmate,Offset dirOffset,Direction Dir,Direction primDir,int ptypeMaskNotKing)
      {
	const Player altP=alt(P);
	Square pos=target-dirOffset;
	if(!pos.isOnBoard()) return;
	Piece p=state.pieceAt(pos);
	if(p.isOnBoardByOwner<P>()){
	  if(DirType==0 && state.hasLongEffectAt<LANCE>(P,pos)){
	    PieceOnBoard<Action>::template generate<P,true>(state,p,action,1<<primDir);
	  }
	  return;
	}
	if((state.Iking8Info(altP)&(1ull<<(40+Dir)))!=0){
	  // - posに利きがある
	  // TODO safe moveではない
	  generateMoveToPtypeMask<P,Action>(state,pos,p,action,
					    ptypeMaskNotKing);
	}
	if(DirType !=0) return;
	if(p.isEmpty()){
	  Square pos1=state.kingMobilityOfPlayer(altP,Dir);
	  mask_t lance_mask=state.longEffectAt<LANCE>(pos1,P);
	  if(lance_mask.any()){
	    Piece p1=state.pieceAt(pos1);
	    if(p1.isOnBoardByOwner<P>()){
	      PieceOnBoard<Action>::template generate<P,true>(state,p1,action,1<<primDir);
	      // 
	      if(state.hasEffectByPiece(p1,pos)){
		PieceOnBoard<Action>::template generatePiece<P>(state,p1,pos,Piece::EMPTY(),action);
	      }
	    }
	    else if(p1.isOnBoardByOwner<altP>()){
	      assert(!lance_mask.hasMultipleBit());
	      int num=lance_mask.bsf()+PtypeFuns<LANCE>::indexNum*32;
	      Piece p2=state.pieceOf(num);
	      if(!state.pinOrOpen(P).test(num) ||
		 state.kingSquare<P>().isUD(p2.square())){
		action.unknownMove(p2.square(),pos1,p1,LANCE,false,P);
	      }
	    }
	  }
	  // - PAWN, LANCEはここで調べる?
	  //  + ただしPAWNはつみは禁止
	  if(! state.isPawnMaskSet<P>(target.x()) &&
	     state.hasPieceOnStand<PAWN>(P)){
	    // 利きをさえぎるパターンの検証
#ifndef GENERATE_PAWNDROP_CHECKMATE
	    if(((state.Iking8Info(altP)&(0xff00ull|(1ull<<(U+24))))^(1ull<<(U+24)))!=0 || blockingU<P>(state,pos))
	      action.dropMove(pos,PAWN,P);
	    else
	      hasPawnCheckmate=true;
#else
	    action.dropMove(pos,PAWN,P);
#endif
	  }
	  if(state.hasPieceOnStand<LANCE>(P)){
	    action.dropMove(pos,LANCE,P);
	    for(pos-=DirectionPlayerTraits<U,P>::offset();
		pos!=pos1;pos-=DirectionPlayerTraits<U,P>::offset()){
	      action.dropMove(pos,LANCE,P);
	    }
	  }
	}
      }

      template<osl::Player P,int DirType,class Action,Direction Dir>
      void generateDir(const NumEffectState& state,Square target,Action& action,bool& hasPawnCheckmate)
      {
	generateDir<P,DirType,Action>(state,target,action,hasPawnCheckmate,
				      DirectionPlayerTraits<Dir,P>::offset(),Dir,DirectionTraits<Dir>::primDir,DirectionTraits<Dir>::ptypeMaskNotKing);
      }
      /**
       * int DirType : 0  - U
       *               1  - LRD
       *               2  - UL,UR,DL,DR
       * dirOffset = DirectionPlayerTraits<Dir,P>::offset()
       */
      template<osl::Player P,int DirType,class Action>
      void generateDirNotKing(const NumEffectState& state,Square target,Action& action,CArray<unsigned char,8>& pieceMobility, int& spaces, PieceMask const& notPieceMask,Offset dirOffset,Direction Dir,Direction primDir,int ptypeMask,Direction dirByBlack
	)
      {
	const Player altP=alt(P);
	Square pos=target-dirOffset;
	if(!pos.isOnBoard()){
	  pieceMobility[dirByBlack]=pos.uintValue();
	  return;
	}
	Piece p=state.pieceAt(pos);
	if(p.canMoveOn<P>()){
	  // - posに利きがある
	  const PieceMask pieceMask=state.piecesOnBoard(P)&state.effectSetAt(pos)&notPieceMask & ~state.effectSetAt(target);
	  if(pieceMask.any())
	    detail:: template generateMoveToPtypeMaskWithPieceMask<P,Action>(state,pos,p,action,
									     ptypeMask,pieceMask);
	}
	Square nextSquare=pos;
	if(p.isEmpty()){
	  spaces|=(1u<<Dir);
	  if(DirType==0 && ! state.isPawnMaskSet<P>(target.x()) &&
	     state.hasPieceOnStand<PAWN>(P))
	    action.dropMove(pos,PAWN,P);
	  do{
	    pos-=dirOffset;
	    p=state.pieceAt(pos);
	  } while(p.isEmpty());
	}
	if(p.isOnBoardByOwner<P>() && state.hasEffectByPiece(p,target)){
	  for(;;){
	    Piece p1=state.findLongAttackAt(P,p,inverse(Dir));
	    if(!p1.isOnBoardByOwner<P>()){
	      break;
	    }
	    p=p1;
	  } 
	  pos=p.square()-dirOffset;
	  while((p=state.pieceAt(pos)).isEmpty())
	    pos-=dirOffset;
	}
	else if (p.isOnBoardByOwner<altP>() && state.hasEffectByPiece(p,target)){
	  // shadowは1つだけ見る
	  Piece p1=state.findLongAttackAt(altP,p,Dir);
	  if(p1.isOnBoardByOwner<P>()){
	    if(pos!=nextSquare){
	      if(p1.ptype()==LANCE){
		int num=p1.number();
		if(!state.pinOrOpen(P).test(num) ||
		   p1.square().isUD(state.kingSquare<P>())){
		  action.unknownMove(p1.square(),pos,p,LANCE,false,P);
		}
	      }
	      else
		PieceOnBoard<Action>::template generatePiece<P>(state,p1,pos,p,action);
	    }
	    pos=p1.square();
	    p=p1;
	  }
	  else{
	    pos=p.square()-dirOffset;
	    while((p=state.pieceAt(pos)).isEmpty())
	      pos-=dirOffset;
	  }
	}
      	pieceMobility[dirByBlack]=pos.uintValue();
	if(p.isOnBoardByOwner<P>()){
	  Piece p1=state.findLongAttackAt(P,p,inverse(Dir));
	  if(p1.isOnBoardByOwner<P>()){
	    Open<Action>::template generate<P>(state,p,action,target,primDir);
	  }
	}
	else if(p.isOnBoardByOwner<altP>() && pos!=nextSquare){
	  if(DirType==0){
	    mask_t lance_mask=state.longEffectAt<LANCE>(pos,P);
	    if(lance_mask.any()){
	      assert(!lance_mask.hasMultipleBit());
	      int num=lance_mask.bsf()+PtypeFuns<LANCE>::indexNum*32;
	      Piece p2=state.pieceOf(num);
	      if(!state.pinOrOpen(P).test(num) || 
		 state.kingSquare<P>().isUD(p2.square())){
		action.unknownMove(p2.square(),pos,p,LANCE,false,P);
	      }
	    }
	  }
	  if(DirType <= 1){
	    mask_t rook_mask=state.allEffectAt<ROOK>(P,pos);
	    while(rook_mask.any()){
	      int num=rook_mask.takeOneBit()+PtypeFuns<ROOK>::indexNum*32;
	      Piece p2=state.pieceOf(num);
	      if(p2.square()==target) continue;
	      PieceOnBoard<Action>::template generatePiece<P>(state,p2,pos,p,action);
	    }
	  }
	  if(DirType == 2){
	    mask_t bishop_mask=state.allEffectAt<BISHOP>(P,pos);
	    // 利きをチェックする必要あり
	    while(bishop_mask.any()){
	      int num=bishop_mask.takeOneBit()+PtypeFuns<BISHOP>::indexNum*32;
	      Piece p2=state.pieceOf(num);
	      if(p2.square()==target) continue;
	      PieceOnBoard<Action>::template generatePiece<P>(state,p2,pos,p,action);
	    }
	  }
	}

	// - PAWN, LANCEはここで調べる?
	//  + ただしPAWNはつみは禁止
	if(DirType == 0){
	  if(state.hasPieceOnStand<LANCE>(P)){
	    for(pos+=DirectionPlayerTraits<U,P>::offset();
		pos!=target;pos+=DirectionPlayerTraits<U,P>::offset()){
	      if(state.pieceAt(pos).isEmpty())
		action.dropMove(pos,LANCE,P);
	    }
	  }
	}
      }

      template<osl::Player P,int DirType,class Action,Direction Dir>
      void generateDirNotKing(const NumEffectState& state,Square target,Action& action,CArray<unsigned char,8>& pieceMobility, int& spaces, PieceMask const& notPieceMask)
      {
	generateDirNotKing<P,DirType,Action>(state,target,action,pieceMobility,spaces,notPieceMask,
					     DirectionPlayerTraits<Dir,P>::offset(),Dir,DirectionTraits<Dir>::primDir,DirectionTraits<Dir>::ptypeMask,DirectionPlayerTraits<Dir,P>::directionByBlack);
	
      }
      template<osl::Player P,osl::Direction Dir,class Action,bool hasKnight>
      void generateKnightDir(const NumEffectState& state,Square target,Action& action)
      {
	Square pos=target-DirectionPlayerTraits<Dir,P>::offset();
	if(!pos.isOnBoard()) return;
	Piece p=state.pieceAt(pos);
	if(!p.canMoveOn<P>()) return;
	mask_t mask=state.allEffectAt<KNIGHT>(P, pos);
	mask &= ~state.promotedPieces().getMask<KNIGHT>();
	// pinnedなknightは動けない
	mask &= ~state.pinOrOpen(P).getMask(PtypeFuns<KNIGHT>::indexNum);
	while(mask.any()){
	  const int num = mask.takeOneBit()+PtypeFuns<KNIGHT>::indexNum*32;
	  Piece p1=state.pieceOf(num);
	  action.unknownMove(p1.square(),pos,p,KNIGHT,false,P);
	}
	if(hasKnight && p.isEmpty()){
	  action.dropMove(pos,KNIGHT,P);
	}
      }
      template<osl::Player P,class Action>
      void generateKnightAll(const NumEffectState& state,Square target,Action& action)
      {
	if(state.hasPieceOnStand<KNIGHT>(P)){
	  detail::generateKnightDir<P,UUL,Action,true>(state,target,action);
	  detail::generateKnightDir<P,UUR,Action,true>(state,target,action);
	}
	else{
	  detail::generateKnightDir<P,UUL,Action,false>(state,target,action);
	  detail::generateKnightDir<P,UUR,Action,false>(state,target,action);
	}
      }
      template <osl::Player P,class Action>
      void generateDrop(Square target,Action& action,int spaceMask,osl::Ptype T,int dirMask,Offset offset)
      {
	if((spaceMask&dirMask)!=0){
	  Square pos=target-offset;
	  action.dropMove(pos,T,P);
	}
      }
      template <osl::Player P,class Action,Direction Dir>
      void generateDropDir(Square target,Action& action,int spaceMask,osl::Ptype T)
      {
	generateDrop<P,Action>(target,action,spaceMask,T,(1<<Dir),DirectionPlayerTraits<Dir,P>::offset());
      }
      template<Player P,class Action,bool mustCareSilver>
      void generateOpenOrCapture(const NumEffectState& state,Square target,Piece p,int num,Action& action)
      {
	// TODO: pin, captureを作る
	Direction d=Board_Table.template getShort8<P>(p.square(),target);
	Square mid=state.mobilityOf((P==BLACK ? d : inverse(d)),num);
	assert(mid.isOnBoard());
	const Player altP=alt(P);
	Square mid1=state.kingMobilityOfPlayer(altP,d);
	if(mid==mid1){
	  Piece p1=state.pieceAt(mid);
	  assert(p1.isPiece());
	  Square target_next=target-Board_Table.getShort8OffsetUnsafe(p.square(),target);
	  if((P==BLACK ? p1.pieceIsBlack() : !p1.pieceIsBlack())){
	    // open attack
	    PieceOnBoard<Action>::template generate<P,true>(state,p1,action,(1<<primDir(d)));
	    // p1がtarget_nextに利きを持つ
	    if(state.hasEffectByPiece(p1,target_next)){
	      // silverが斜め下に利きを持つ場合は「成らず」しか生成しない
	      if(mustCareSilver && p1.ptype()==SILVER && 
		 (P==BLACK ? target.y()>mid.y() : target.y()<mid.y())){
		// pinの場合は動ける可能性はない 
		if(!state.pinOrOpen(P).test(p1.number())){
		  action.unknownMove(mid,target_next,Piece::EMPTY(),SILVER,false,P);
		}
	      }
	      else
		PieceOnBoard<Action>::template generatePiece<P>(state,p1,target_next,Piece::EMPTY(),action);
	    }
	  }
	  else{
	    // 隣の場合はすでに作っている
	    if(mid==target_next)
	      return;
	    PieceOnBoard<Action>::template generatePiece<P>(state,p,mid,p1,action);
	  }
	}
      }

      template<osl::Player P,class Action>
      void generateRookLongMove(const NumEffectState& state,Square target,Action& action)
      {
	const Player altP=alt(P);
	for(int num=PtypeTraits<ROOK>::indexMin;num<PtypeTraits<ROOK>::indexLimit;num++){
	  // pinの場合はすでに作っている
	  if(state.pinOrOpen(altP).test(num)) continue;
	  Piece p=state.pieceOf(num);
	  if(!p.isOnBoardByOwner<P>()) continue;
	  if(target.isULRD(p.square())){
	    generateOpenOrCapture<P,Action,false>(state,target,p,num,action);
	    continue;
	  }
	  int target_x=target.x();
	  int target_y=target.y();
	  int rook_x=p.square().x();
	  int rook_y=p.square().y();
	  if(p.isPromoted()){
	    if((unsigned int)(target_x-rook_x+1)>2u){ // abs(target_x-rook_x)>1
	      if((unsigned int)(target_y-rook_y+1)>2u){ // abs(target_y-rook_y)>1
		{
		  Square pos(rook_x,target_y);
		  Piece p1=state.pieceAt(pos);
		  if(state.effectSetAt(pos).test(num) &&
		     p1.canMoveOn<P>() &&
		     state.kingMobilityAbs(altP,R).uintValue() >= pos.uintValue() &&
		     pos.uintValue() >= state.kingMobilityAbs(altP,L).uintValue() &&
		     (!state.pinOrOpen(P).test(num) ||
		      p.square().isUD(state.kingSquare<P>()))
		    ){
		    action.unknownMove(p.square(),pos,p1,PROOK,false,P);
		  }
		}
		{
		  Square pos(target_x,rook_y);
		  Piece p1=state.pieceAt(pos);
		  if(state.effectSetAt(pos).test(num) &&
		     p1.canMoveOn<P>() &&
		     state.kingMobilityAbs(altP,U).uintValue() >= pos.uintValue() &&
		     pos.uintValue() >= state.kingMobilityAbs(altP,D).uintValue() &&
		     (!state.pinOrOpen(P).test(num) ||
		      p.square().isLR(state.kingSquare<P>()))
		    ){
		    action.unknownMove(p.square(),pos,p1,PROOK,false,P);
		  }
		}
	      }
	      else{ // (abs(target_x-rook_x)>1 && abs(target_y-rook_y)==1
		int min_x=state.kingMobilityAbs(altP,L).x();
		int max_x=state.kingMobilityAbs(altP,R).x();
		if(target_x>rook_x) max_x=target_x-2;
		else min_x=target_x+2;
		min_x=std::max(min_x,rook_x-1);
		max_x=std::min(max_x,rook_x+1);
		for(int x=min_x;x<=max_x;x++){
		  Square pos=Square::makeNoCheck(x,target_y);
		  Piece p1=state.pieceAt(pos);
		  if(p1.canMoveOn<P>())
		    PieceOnBoard<Action>::template generatePiecePtype<P,PROOK>(state,p,pos,p1,action);
		}
	      }
	    }
	    else if((unsigned int)(target_y-rook_y+1)>2u){ // abs(target_y-rook_y)>1, abs(target_x-rook_x)==1
	      int min_y=state.kingMobilityAbs(altP,D).y();
	      int max_y=state.kingMobilityAbs(altP,U).y();
	      if(target_y>rook_y) max_y=target_y-2;
	      else min_y=target_y+2;
	      min_y=std::max(min_y,rook_y-1);
	      max_y=std::min(max_y,rook_y+1);
	      for(int y=min_y;y<=max_y;y++){
		Square pos=Square::makeNoCheck(target_x,y);
		Piece p1=state.pieceAt(pos);
		if(p1.canMoveOn<P>())
		  PieceOnBoard<Action>::template generatePiecePtype<P,PROOK>(state,p,pos,p1,action);
	      }
	    }
	  }
	  else{ // ROOK
	    // vertical move
	    if((unsigned int)(target_x-rook_x+1)>2u){ // abs(target_x-rook_x)>1
	      Square pos(rook_x,target_y);
	      Piece p1=state.pieceAt(pos);
	      if(state.effectSetAt(pos).test(num) &&
		 p1.canMoveOn<P>() &&
		 state.kingMobilityAbs(altP,R).uintValue() >= pos.uintValue() &&
		 pos.uintValue() >= state.kingMobilityAbs(altP,L).uintValue() &&
		 (!state.pinOrOpen(P).test(num) ||
		  p.square().isUD(state.kingSquare<P>()))
		){
		if(Square::canPromoteY<P>(rook_y) || Square::canPromoteY<P>(target_y)){
		  action.unknownMove(p.square(),pos,p1,PROOK,true,P);
		}
		else action.unknownMove(p.square(),pos,p1,ROOK,false,P);
	      }
	    }
	    // horizontal move
	    if((unsigned int)(target_y-rook_y+1)>2u){ // abs(target_y-rook_y)>1
	      Square pos(target_x,rook_y);
	      Piece p1=state.pieceAt(pos);
	      if(state.effectSetAt(pos).test(num) &&
		 p1.canMoveOn<P>() &&
		 state.kingMobilityAbs(altP,U).uintValue() >= pos.uintValue() &&
		 pos.uintValue() >= state.kingMobilityAbs(altP,D).uintValue() &&
		 (!state.pinOrOpen(P).test(num) ||
		  p.square().isLR(state.kingSquare<P>()))
		){
		if(Square::canPromoteY<P>(rook_y)){
		  action.unknownMove(p.square(),pos,p1,PROOK,true,P);
		}
		else
		  action.unknownMove(p.square(),pos,p1,ROOK,false,P);
	      }
	    }
	  }
	}
      }
      template<osl::Player P,class Action>
      void generateRookLongMoveNotKing(const NumEffectState& state,Square target,Action& action,CArray<unsigned char,8> const& pieceMobility)
      {
	for(int num=PtypeTraits<ROOK>::indexMin;num<PtypeTraits<ROOK>::indexLimit;num++){
	  Piece p=state.pieceOf(num);
	  if(!p.isOnBoardByOwner<P>()) continue;
	  if(target.isULRD(p.square())){
	    continue;
	  }
	  int dirMask=0; // can move to all direction 
	  if(state.pin(P).test(num)){
	    Direction d=state.pinnedDir<P>(p);
	    dirMask=(~(1<<primDir(d)));
	  }
	  int target_x=target.x();
	  int target_y=target.y();
	  int rook_x=p.square().x();
	  int rook_y=p.square().y();
	  if(p.isPromoted()){
	    if((unsigned int)(target_x-rook_x+1)>2u){ // abs(target_x-rook_x)>1
	      if((unsigned int)(target_y-rook_y+1)>2u){ // abs(target_y-rook_y)>1
		{
		  Square pos(rook_x,target_y);
		  Piece p1=state.pieceAt(pos);
		  if(p1.canMoveOn<P>() &&
		     pieceMobility[R] > pos.uintValue() &&
		     pos.uintValue() > pieceMobility[L] &&
		     (dirMask&(1<<U))==0 &&
		     state.effectSetAt(pos).test(num)
		    ){
		    action.unknownMove(p.square(),pos,p1,PROOK,false,P);
		  }
		}
		{
		  Square pos(target_x,rook_y);
		  Piece p1=state.pieceAt(pos);
		  if(p1.canMoveOn<P>() &&
		     pieceMobility[U] > pos.uintValue() &&
		     pos.uintValue() > pieceMobility[D] &&
		     (dirMask&(1<<L))==0 &&
		     state.effectSetAt(pos).test(num)){
		    action.unknownMove(p.square(),pos,p1,PROOK,false,P);
		  }
		}
	      }
	      else{ // (abs(target_x-rook_x)>1 && abs(target_y-rook_y)==1
		int min_x=Square::makeDirect(pieceMobility[L]).x()+1;
		int max_x=Square::makeDirect(pieceMobility[R]).x()-1;
		if(target_x>rook_x) max_x=target_x-2;
		else min_x=target_x+2;
		min_x=std::max(min_x,rook_x-1);
		max_x=std::min(max_x,rook_x+1);
		for(int x=min_x;x<=max_x;x++){
		  Square pos=Square::makeNoCheck(x,target_y);
		  if(((1<<primDirUnsafe(Board_Table.getShort8Unsafe<P>(p.square(),pos)))&dirMask)!=0) continue;
		  Piece p1=state.pieceAt(pos);
		  if(p1.canMoveOn<P>())
		    action.unknownMove(p.square(),pos,p1,PROOK,false,P);
		}
	      }
	    }
	    else if((unsigned int)(target_y-rook_y+1)>2u){ // abs(target_y-rook_y)>1, abs(target_x-rook_x)==1
	      int min_y=Square::makeDirect(pieceMobility[D]).y()+1;
	      int max_y=Square::makeDirect(pieceMobility[U]).y()-1;
	      if(target_y>rook_y) max_y=target_y-2;
	      else min_y=target_y+2;
	      min_y=std::max(min_y,rook_y-1);
	      max_y=std::min(max_y,rook_y+1);
	      for(int y=min_y;y<=max_y;y++){
		Square pos=Square::makeNoCheck(target_x,y);
		if(((1<<primDirUnsafe(Board_Table.getShort8Unsafe<P>(p.square(),pos)))&dirMask)!=0) continue;
		Piece p1=state.pieceAt(pos);
		if(p1.canMoveOn<P>())
		  action.unknownMove(p.square(),pos,p1,PROOK,false,P);
	      }
	    }
	  }
	  else{ // ROOK
	    // vertical move
	    if((unsigned int)(target_x-rook_x+1)>2u){ // abs(target_x-rook_x)>1
	      Square pos(rook_x,target_y);
	      Piece p1=state.pieceAt(pos);
	      if(p1.canMoveOn<P>() &&
		 pieceMobility[R] > pos.uintValue() &&
		 pos.uintValue() > pieceMobility[L] &&
		 (dirMask&(1<<U))==0 &&
		 state.effectSetAt(pos).test(num)
		){
		if(Square::canPromoteY<P>(rook_y) || Square::canPromoteY<P>(target_y)){
		  action.unknownMove(p.square(),pos,p1,PROOK,true,P);
		}
		else
		  action.unknownMove(p.square(),pos,p1,ROOK,false,P);
	      }
	    }
	    // horizontal move
	    if((unsigned int)(target_y-rook_y+1)>2u){ // abs(target_y-rook_y)>1
	      Square pos(target_x,rook_y);
	      Piece p1=state.pieceAt(pos);
	      if(p1.template canMoveOn<P>() &&
		 pieceMobility[U] > pos.uintValue() &&
		 pos.uintValue() > pieceMobility[D] &&
		 (dirMask&(1<<L))==0 &&
		 state.effectSetAt(pos).test(num)
		){
		if(Square::canPromoteY<P>(rook_y)){
		  action.unknownMove(p.square(),pos,p1,PROOK,true,P);
		}
		else
		  action.unknownMove(p.square(),pos,p1,ROOK,false,P);
	      }
	    }
	  }
	}
      }
      template<Player P,Ptype T,class Action>
      void generateBishopLongMove(const NumEffectState& state,Square target,Action& action,Piece p,int num)
      {
	const Player altP=alt(P);
	int target_x=target.x();
	int target_y=target.y();
	int target_xPy=target_x+target_y;
	int target_xMy=target_x-target_y;
	int bishop_x=p.square().x();
	int bishop_y=p.square().y();
	int bishop_xPy=bishop_x+bishop_y;
	int bishop_xMy=bishop_x-bishop_y;
	if(((target_xPy^bishop_xPy)&1)!=0){
	  if(T==BISHOP) return;
	  // 市松模様のparityの違う場合も，隣ならOK?
	  if((unsigned int)(target_xPy-bishop_xPy+1)<=2u){ // abs(target_xPy-bishop_xPy)==1
	    Square ul=state.kingMobilityAbs(altP,UL);
	    Square dr=state.kingMobilityAbs(altP,DR);
	    int min_xMy=ul.x()-ul.y();
	    int max_xMy=dr.x()-dr.y();
	    if(target_xMy>bishop_xMy) max_xMy=target_xMy-4;
	    else min_xMy=target_xMy+4;
	    min_xMy=std::max(min_xMy,bishop_xMy-1);
	    max_xMy=std::min(max_xMy,bishop_xMy+1);
	    for(int xMy=min_xMy;xMy<=max_xMy;xMy+=2){
	      int pos_x=(target_xPy+xMy)>>1;
	      int pos_y=(target_xPy-xMy)>>1;
	      Square pos=Square::makeNoCheck(pos_x,pos_y);
	      Piece p1=state.pieceAt(pos);
	      if(p1.canMoveOn<P>())
		PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	  }
	  else if((unsigned int)(target_xMy-bishop_xMy+1)<=2u){ // abs(target_xMy-bishop_xMy)==1
	    Square dl=state.kingMobilityAbs(altP,DL);
	    Square ur=state.kingMobilityAbs(altP,UR);
	    int min_xPy=dl.x()+dl.y();
	    int max_xPy=ur.x()+ur.y();
	    if(target_xPy>bishop_xPy) max_xPy=target_xPy-4;
	    else min_xPy=target_xPy+4;
	    min_xPy=std::max(min_xPy,bishop_xPy-1);
	    max_xPy=std::min(max_xPy,bishop_xPy+1);
	    for(int xPy=min_xPy;xPy<=max_xPy;xPy+=2){
	      int pos_x=(xPy+target_xMy)>>1;
	      int pos_y=(xPy-target_xMy)>>1;
	      Square pos=Square::makeNoCheck(pos_x,pos_y);
	      Piece p1=state.pieceAt(pos);
	      if(p1.canMoveOn<P>())
		PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	  }
	  return;
	}
	//  / 方向(dx==dy)から王手をかける
	if((unsigned int)(target_xPy-bishop_xPy+2)>4u){ // abs(target_xPy-bishop_xPy)>2
	  int pos_x=(bishop_xPy+target_xMy)>>1;
	  int pos_y=(bishop_xPy-target_xMy)>>1;
	  Square pos=Square::makeNoCheck(pos_x,pos_y);
	  if(pos.isOnBoard()){
	    Piece p1=state.pieceAt(pos);
	    if(state.effectSetAt(pos).test(num) &&
	       p1.canMoveOn<P>() &&
	       state.kingMobilityAbs(altP,UR).uintValue() >= pos.uintValue() &&
	       pos.uintValue() >= state.kingMobilityAbs(altP,DL).uintValue()
	      ){
	      PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	  }
	}
	else if(target_xPy==bishop_xPy){
	  generateOpenOrCapture<P,Action,true>(state,target,p,num,action);
	  return;
	}
	//  \ 方向(dx== -dy)から王手をかける
	if((unsigned int)(target_xMy-bishop_xMy+2)>4u){ // abs(target_xMy-bishop_xMy)>2
	  int pos_x=(target_xPy+bishop_xMy)>>1;
	  int pos_y=(target_xPy-bishop_xMy)>>1;
	  Square pos=Square::makeNoCheck(pos_x,pos_y);
	  if(pos.isOnBoard()){
	    Piece p1=state.pieceAt(pos);
	    if(state.effectSetAt(pos).test(num) &&
	       p1.canMoveOn<P>() &&
	       state.kingMobilityAbs(altP,DR).uintValue() >= pos.uintValue() &&
	       pos.uintValue() >= state.kingMobilityAbs(altP,UL).uintValue()
	      ){
	      PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	  }
	}
	else if(target_xMy==bishop_xMy){
	  generateOpenOrCapture<P,Action,true>(state,target,p,num,action);
	  return;
	}

      }
      template<osl::Player P,Ptype T,class Action>
      void generateBishopLongMoveNotKing(const NumEffectState& state,Square target,Action& action,CArray<unsigned char,8> const& pieceMobility,Piece p,int num)
      {
	int target_x=target.x();
	int target_y=target.y();
	int target_xPy=target_x+target_y;
	int target_xMy=target_x-target_y;
	int bishop_x=p.square().x();
	int bishop_y=p.square().y();
	int bishop_xPy=bishop_x+bishop_y;
	int bishop_xMy=bishop_x-bishop_y;
	if(((target_xPy^bishop_xPy)&1)!=0){
	  if(T!=PBISHOP) return;
	  // 市松模様のparityの違う場合も，隣ならOK?
	  if((unsigned int)(target_xPy-bishop_xPy+1)<=2u){ // abs(target_xPy-bishop_xPy)==1
	    Square ul=Square::makeDirect(pieceMobility[UL]);
	    Square dr=Square::makeDirect(pieceMobility[DR]);
	    int min_xMy=ul.x()-ul.y()+2;
	    int max_xMy=dr.x()-dr.y()-2;
	    if(target_xMy>bishop_xMy) max_xMy=target_xMy-4;
	    else min_xMy=target_xMy+4;
	    min_xMy=std::max(min_xMy,bishop_xMy-1);
	    max_xMy=std::min(max_xMy,bishop_xMy+1);
	    for(int xMy=min_xMy;xMy<=max_xMy;xMy+=2){
	      int pos_x=(target_xPy+xMy)>>1;
	      int pos_y=(target_xPy-xMy)>>1;
	      Square pos=Square::makeNoCheck(pos_x,pos_y);
	      Piece p1=state.pieceAt(pos);
	      if(p1.canMoveOn<P>())
		PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	    return;
	  }
	  else if((unsigned int)(target_xMy-bishop_xMy+1)<=2u){ // abs(target_xMy-bishop_xMy)==1
	    Square dl=Square::makeDirect(pieceMobility[DL]);
	    Square ur=Square::makeDirect(pieceMobility[UR]);
	    int min_xPy=dl.x()+dl.y()+2;
	    int max_xPy=ur.x()+ur.y()-2;
	    if(target_xPy>bishop_xPy) max_xPy=target_xPy-4;
	    else min_xPy=target_xPy+4;
	    min_xPy=std::max(min_xPy,bishop_xPy-1);
	    max_xPy=std::min(max_xPy,bishop_xPy+1);
	    for(int xPy=min_xPy;xPy<=max_xPy;xPy+=2){
	      int pos_x=(xPy+target_xMy)>>1;
	      int pos_y=(xPy-target_xMy)>>1;
	      Square pos=Square::makeNoCheck(pos_x,pos_y);
	      Piece p1=state.pieceAt(pos);
	      if(p1.canMoveOn<P>())
		PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	  }
	  return;
	}
	//  / 方向(dx==dy)から王手をかける
	if((unsigned int)(target_xPy-bishop_xPy+2)>4u){ // abs(target_xPy-bishop_xPy)>2
	  int pos_x=(bishop_xPy+target_xMy)>>1;
	  int pos_y=(bishop_xPy-target_xMy)>>1;
	  Square pos=Square::makeNoCheck(pos_x,pos_y);
	  if(pos.isOnBoard()){
	    if(pieceMobility[UR] > pos.uintValue() &&
	       pos.uintValue() > pieceMobility[DL] &&
	       state.effectSetAt(pos).test(num)){
	      Piece p1=state.pieceAt(pos);
	      if(p1.canMoveOn<P>())
		PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	  }
	}
	//  \ 方向(dx== -dy)から王手をかける
	if((unsigned int)(target_xMy-bishop_xMy+2)>4u){ // abs(target_xMy-bishop_xMy)>2
	  int pos_x=(target_xPy+bishop_xMy)>>1;
	  int pos_y=(target_xPy-bishop_xMy)>>1;
	  Square pos=Square::makeNoCheck(pos_x,pos_y);
	  if(pos.isOnBoard()){
	    if(pieceMobility[DR] > pos.uintValue() &&
	       pos.uintValue() > pieceMobility[UL] &&
	       state.effectSetAt(pos).test(num)
	      ){
	      Piece p1=state.pieceAt(pos);
	      if(p1.canMoveOn<P>())
		PieceOnBoard<Action>::template generatePiecePtype<P,T>(state,p,pos,p1,action);
	    }
	  }
	}
      }
    
      template<Player P,class Action>
      void generateDropGold(const NumEffectState& state,Square target,Action& action,int spaces)
      {
	if(!state.hasPieceOnStand<GOLD>(P)) return;
	unsigned int gold_mask=spaces&((1<<U)|(1<<UR)|(1<<UL)|(1<<L)|(1<<R)|(1<<D));
	if(gold_mask==0) return;
	generateDropDir<P,Action,U>(target,action,gold_mask,GOLD);
	generateDropDir<P,Action,UL>(target,action,gold_mask,GOLD);
	generateDropDir<P,Action,UR>(target,action,gold_mask,GOLD);
	generateDropDir<P,Action,L>(target,action,gold_mask,GOLD);
	generateDropDir<P,Action,R>(target,action,gold_mask,GOLD);
	generateDropDir<P,Action,D>(target,action,gold_mask,GOLD);
      }
      template<Player P,class Action>
      void generateDropSilver(const NumEffectState& state,Square target,Action& action,int spaces)
      {
	if(!state.hasPieceOnStand<SILVER>(P)) return;
	unsigned int silver_mask=spaces&((1<<U)|(1<<UR)|(1<<UL)|(1<<DL)|(1<<DR));
	if(silver_mask ==0) return;

	generateDropDir<P,Action,DL>(target,action,silver_mask,SILVER);
	generateDropDir<P,Action,DR>(target,action,silver_mask,SILVER);
	generateDropDir<P,Action,U>(target,action,silver_mask,SILVER);
	generateDropDir<P,Action,UL>(target,action,silver_mask,SILVER);
	generateDropDir<P,Action,UR>(target,action,silver_mask,SILVER);
      }
      /**
       * allEmpty - shadow attackを生成する場合は，posがemptyでないこともある．
       */
      template<Player P,class Action,bool allEmpty>
      void generateDropBishop(const NumEffectState& state,Square target,Action& action,Square ul,Square dr,Square ur,Square dl)
      {
	for(Square pos=dl+DirectionPlayerTraits<DL,P>::offset();
	    pos!=target;pos+=DirectionPlayerTraits<DL,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,BISHOP,P);
	for(Square pos=dr-DirectionPlayerTraits<UL,P>::offset();
	    pos!=target;pos-=DirectionPlayerTraits<UL,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,BISHOP,P);
	for(Square pos=ul+DirectionPlayerTraits<UL,P>::offset();
	    pos!=target;pos+=DirectionPlayerTraits<UL,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,BISHOP,P);
	for(Square pos=ur-DirectionPlayerTraits<DL,P>::offset();
	    pos!=target;pos-=DirectionPlayerTraits<DL,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,BISHOP,P);
      }

      template<Player P,class Action,bool allEmpty>
      void generateDropRook(const NumEffectState& state,Square target,Action& action,Square l,Square r,Square d,Square u)
      {
	for(Square pos=u-DirectionPlayerTraits<D,P>::offset();
	    pos!=target;pos-=DirectionPlayerTraits<D,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,ROOK,P);
	for(Square pos=l+DirectionPlayerTraits<L,P>::offset();
	    pos!=target;pos+=DirectionPlayerTraits<L,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,ROOK,P);
	for(Square pos=r-DirectionPlayerTraits<L,P>::offset();
	    pos!=target;pos-=DirectionPlayerTraits<L,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,ROOK,P);
	for(Square pos=d+DirectionPlayerTraits<D,P>::offset();
	    pos!=target;pos+=DirectionPlayerTraits<D,P>::offset())
	  if(allEmpty || state.pieceAt(pos).isEmpty())
	    action.dropMove(pos,ROOK,P);
      }
      template<osl::Player P,class Action>
      void generateKing(const NumEffectState& state,Square target,Action& action,bool &hasPawnCheckmate)
      {
	
	const Player altP=alt(P);
	assert(target==state.kingSquare(altP));
	generateDir<P,0,Action,U>(state,target,action,hasPawnCheckmate);
	generateKnightAll<P,Action>(state,target,action);
	generateDir<P,2,Action,UL>(state,target,action,hasPawnCheckmate);
	generateDir<P,2,Action,UR>(state,target,action,hasPawnCheckmate);
	generateDir<P,1,Action,L>(state,target,action,hasPawnCheckmate);
	generateDir<P,1,Action,R>(state,target,action,hasPawnCheckmate);
	generateDir<P,1,Action,D>(state,target,action,hasPawnCheckmate);
	generateDir<P,2,Action,DL>(state,target,action,hasPawnCheckmate);
	generateDir<P,2,Action,DR>(state,target,action,hasPawnCheckmate);
	detail::generateRookLongMove<P,Action>(state,target,action);
	for(int num=PtypeTraits<BISHOP>::indexMin;num<PtypeTraits<BISHOP>::indexLimit;num++){
	  // pinの場合はすでに作っている
	  if(state.pinOrOpen(altP).test(num)) continue;
	  Piece p=state.pieceOf(num);
	  if(!p.isOnBoardByOwner<P>()) continue;
	  if(p.isPromoted())
	    generateBishopLongMove<P,PBISHOP,Action>(state,target,action,p,num);
	  else
	    generateBishopLongMove<P,BISHOP,Action>(state,target,action,p,num);
	}
	int spaces=King8Info(state.Iking8Info(altP)).spaces();
	generateDropGold<P,Action>(state,target,action,spaces);
	generateDropSilver<P,Action>(state,target,action,spaces);
	// bishop
	if(state.hasPieceOnStand<BISHOP>(P)){
	  generateDropBishop<P,Action,true>(state,target,action,
					    state.kingMobilityOfPlayer(altP,UL),
					    state.kingMobilityOfPlayer(altP,DR),
					    state.kingMobilityOfPlayer(altP,UR),
					    state.kingMobilityOfPlayer(altP,DL));
	}
	if(state.hasPieceOnStand<ROOK>(P)){
	  Square l,r,d,u;
	  l=state.kingMobilityOfPlayer(altP,L);
	  r=state.kingMobilityOfPlayer(altP,R);
	  d=state.kingMobilityOfPlayer(altP,D);
	  u=state.kingMobilityOfPlayer(altP,U);
	  generateDropRook<P,Action,true>(state,target,action,l,r,d,u);
	}
      }
      template<osl::Player P,class Action>
      void generateNotKing(const NumEffectState& state,Square target,Action& action)
      {
	int spaces=0;
	CArray<unsigned char,8> pieceMobility;
	PieceMask notPieceMask;
	notPieceMask.setAll();
	int num=state.pieceAt(target).number();
	if(num != EMPTY_NUM){
	  notPieceMask.reset(num);
	}
	generateDirNotKing<P,0,Action,U>(state,target,action,pieceMobility,spaces,notPieceMask);
	generateKnightAll<P,Action>(state,target,action);
	generateDirNotKing<P,2,Action,UL>(state,target,action,pieceMobility,spaces,notPieceMask);
	generateDirNotKing<P,2,Action,UR>(state,target,action,pieceMobility,spaces,notPieceMask);
	generateDirNotKing<P,1,Action,L>(state,target,action,pieceMobility,spaces,notPieceMask);
	generateDirNotKing<P,1,Action,R>(state,target,action,pieceMobility,spaces,notPieceMask);
	generateDirNotKing<P,1,Action,D>(state,target,action,pieceMobility,spaces,notPieceMask);
	generateDirNotKing<P,2,Action,DL>(state,target,action,pieceMobility,spaces,notPieceMask);
	generateDirNotKing<P,2,Action,DR>(state,target,action,pieceMobility,spaces,notPieceMask);
	// rookが移動する手
	generateRookLongMoveNotKing<P,Action>(state,target,action,pieceMobility);
	// bishopが移動する手
	for(int num=PtypeTraits<BISHOP>::indexMin;num<PtypeTraits<BISHOP>::indexLimit;num++){
	  Piece p=state.pieceOf(num);
	  if(!p.isOnBoardByOwner<P>()) continue;
	  if(p.isPromoted())
	    generateBishopLongMoveNotKing<P,PBISHOP,Action>(state,target,action,pieceMobility,p,num);
	  else
	    generateBishopLongMoveNotKing<P,BISHOP,Action>(state,target,action,pieceMobility,p,num);
	}
	generateDropGold<P,Action>(state,target,action,spaces);
	generateDropSilver<P,Action>(state,target,action,spaces);
	if(state.hasPieceOnStand<BISHOP>(P)){
	  Square ul,dr,dl,ur;
	  ul=Square::makeDirect(pieceMobility[P==BLACK ? UL : DR]);
	  dr=Square::makeDirect(pieceMobility[P==BLACK ? DR : UL]);
	  ur=Square::makeDirect(pieceMobility[P==BLACK ? UR : DL]);
	  dl=Square::makeDirect(pieceMobility[P==BLACK ? DL : UR]);
	  generateDropBishop<P,Action,false>(state,target,action,ul,dr,ur,dl);
	}
	if(state.hasPieceOnStand<ROOK>(P)){
	  Square l,r,d,u;
	  l=Square::makeDirect(pieceMobility[P==BLACK ? L : R]);
	  r=Square::makeDirect(pieceMobility[P==BLACK ? R : L]);
	  d=Square::makeDirect(pieceMobility[P==BLACK ? D : U]);
	  u=Square::makeDirect(pieceMobility[P==BLACK ? U : D]);
	  generateDropRook<P,Action,false>(state,target,action,l,r,d,u);
	}
      }
    } // namespace detail
    template <class Action>
    template <osl::Player P,bool isAttackToKing>
    void osl::move_generator::AddEffectWithEffect<Action>::
    generate(const NumEffectState& state,Square target,Action& action,bool &hasPawnCheckmate)
    {
      if(!isAttackToKing){
	detail::template generateNotKing<P,Action>(state,target,action);
      }
      else{
	detail::template generateKing<P,Action>(state,target,action,hasPawnCheckmate);
      }
    }
    template<bool isAttackToKing>
    void osl::move_generator::GenerateAddEffectWithEffect::
    generate(Player player, const NumEffectState& state, Square target, 
	     move_action::Store& store)
    {
      using namespace osl::move_action;
      bool dummy;
      if(player==BLACK){
	AddEffectWithEffect<Store>::generate<BLACK,isAttackToKing>(state,target,store,dummy);
      }
      else{
	AddEffectWithEffect<Store>::generate<WHITE,isAttackToKing>(state,target,store,dummy);
      }
    }

  } // namespace move_generator
} // namespace osl
#endif /* _GENERATER_ADD_EFFECT_WITH_EFFECT_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
