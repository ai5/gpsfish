/* numEffectState.cc
 */
#include "osl/numEffectState.h"
#include "osl/numEffectState.tcc"
#include "osl/simpleState.tcc"
#include "osl/bits/numSimpleEffect.tcc"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/safeMove.h"
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/move_action.h"
#include "osl/move_generator/effect_action.h"

#include <iostream>
#if (defined(__i386__) || defined(__x86_64__)) && !defined(OSL_NO_SSE)
#include <emmintrin.h>
typedef __v2di v2di;
#endif

bool osl::operator==(const NumEffectState& st1,
			    const NumEffectState& st2)
{
  assert(st1.isConsistent(true));
  assert(st2.isConsistent(true));
  if (!(st1.effects == st2.effects)) 
    return false;
  if (!(st1.pieces_onboard == st2.pieces_onboard)) 
    return false;
  if (!(st1.promoted == st2.promoted)) 
    return false;
  if (!(st1.pin_or_open == st2.pin_or_open)) 
    return false;
  if (!(st1.king_mobility == st2.king_mobility)) 
    return false;
  if (!(st1.king8infos == st2.king8infos)) 
    return false;
  return (static_cast<const SimpleState&>(st1)
	  == static_cast<const SimpleState&>(st2));
}

const osl::checkmate::King8Info osl::
NumEffectState::king8Info(Player king) const
{
  return King8Info(Iking8Info(king));
}

template<osl::Player P>
void osl::NumEffectState::makeKing8Info()
{
  const Player altP=alt(P);
#ifdef ALLOW_KING_ABSENCE
  if (kingSquare<P>().isPieceStand())
    return;
#endif
  king8infos[P]=King8Info::make<altP>(*this,kingSquare<P>()).uint64Value();
}

osl::
NumEffectState::NumEffectState(const SimpleState& st) 
  : SimpleState(st),effects(st)
{
  pieces_onboard[0].resetAll();
  pieces_onboard[1].resetAll();
  promoted.resetAll();
  effects.effected_mask[0].resetAll();
  effects.effected_mask[1].resetAll();
  effects.effected_changed_mask[0].resetAll();
  effects.effected_changed_mask[1].resetAll();
  for(int num=0;num<40;num++){
    Piece p=pieceOf(num);
    if (p.isOnBoard()){
      pieces_onboard[p.owner()].set(num);
      if (p.isPromoted())
	promoted.set(num);
      for(int i=0;i<2;i++){
	Player pl=indexToPlayer(i);
	if(hasEffectAt(pl,p.square()))
	{
	  effects.effected_mask[i].set(num);
	  effects.effected_changed_mask[i].set(num);
	}
      }
    }
  }
  makePinOpen(BLACK);
  makePinOpen(WHITE);
  if(kingSquare<BLACK>().isOnBoard())
    makeKing8Info<BLACK>();
  if(kingSquare<WHITE>().isOnBoard())
    makeKing8Info<WHITE>();
}
osl::
NumEffectState::~NumEffectState() 
{
}

const osl::Piece osl::
NumEffectState::selectCheapPiece(PieceMask effect) const
{
  if (! effect.any())
    return Piece::EMPTY();
  mask_t pieces = effect.selectBit<PAWN>(), ppieces;
  if (pieces.any()) 
  {
    ppieces = pieces & promoted.getMask<PAWN>();
    pieces &= ~ppieces;
    if (pieces.any())
      return pieceOf(pieces.bsf()+PtypeFuns<PAWN>::indexNum*32);
    return pieceOf(ppieces.bsf()+PtypeFuns<PAWN>::indexNum*32);
  }
  pieces = effect.selectBit<LANCE>();
  if (pieces.any()) 
  {
    ppieces = pieces & promoted.getMask<LANCE>();
    pieces &= ~ppieces;
    if (pieces.any())
      return pieceOf(pieces.bsf()+PtypeFuns<LANCE>::indexNum*32);
    return pieceOf(ppieces.bsf()+PtypeFuns<LANCE>::indexNum*32);
  }
  mask_t king = effect.selectBit<KING>();
  effect.clearBit<KING>();
  if (effect.none())
    return pieceOf(king.bsf()+PtypeFuns<KING>::indexNum*32);
  // depends on current piece numbers: <FU 0>, KE 18, GI 22, KI 26, <OU 30>, <KY 32>, KA 36, HI 38, 
  const int index = 0;
  ppieces = effect.getMask(index) & promoted.getMask(index);
  pieces = effect.getMask(index) & ~ppieces;
  if (pieces.none() || ppieces.none())
    return pieceOf(pieces.any() ? pieces.bsf() : ppieces.bsf());
  const int num = pieces.bsf(), nump = ppieces.bsf();
  if (Piece_Table.getPtypeOf(num) == Piece_Table.getPtypeOf(nump))
    return pieceOf(num);
  return pieceOf(std::min(num, nump));
}

const osl::Piece osl::
NumEffectState::findThreatenedPiece(Player P) const
{
  assert(! inCheck(P));
  PieceMask pieces = piecesOnBoard(P) & effectedMask(alt(P));
  PieceMask nolance = pieces; nolance.clearBit<LANCE>();
  int pp=-1, npp=-1, ret=-1;
  const int lance_index = PtypeFuns<LANCE>::indexNum; // 64bit: 0, 32bit: 1
  for (int i=lance_index; i>=0; --i) {
    mask_t all = nolance.getMask(i);
    mask_t promoted = all & promotedPieces().getMask(i);
    mask_t notpromoted = all & ~promoted;
    if (promoted.any()) {
      pp = promoted.bsr() + i*32;
      notpromoted &= ~Ptype_Table.getMaskLow(Piece_Table.getPtypeOf(pp));
    }
    if (notpromoted.any())
      npp = notpromoted.bsr() + i*32;
    ret = std::max(pp, npp);
    if (ret >= PtypeTraits<KNIGHT>::indexMin)
      return pieceOf(ret);  
  }
  mask_t lance = pieces.selectBit<LANCE>();
  if (lance.any()) {
    mask_t plance = lance & promotedPieces().getMask(lance_index);
    if (plance.any())
      return pieceOf(plance.bsr()+lance_index*32);
    return pieceOf(lance.bsr()+lance_index*32);
  }
  if (ret >= 0) {
    assert(Piece_Table.getPtypeOf(ret) == PAWN);
    return pieceOf(ret);
  }
  return Piece::EMPTY();
}

bool osl::
NumEffectState::wasCheckEvasion(Move last_move) const
{
  if (! last_move.isNormal())
    return false;
  const Square from = last_move.from(), to = last_move.to();
  if (last_move.ptype() == KING) {
    if (last_move.isCapture()
	&& hasEffectIf(last_move.capturePtypeO(), to, from))
      return true;
    return hasEffectAt(turn(), from);
  }
  if (last_move.isCapture())
    return hasEffectIf(last_move.capturePtypeO(), to,
		       kingSquare(alt(turn())))
      && !Board_Table.isBetweenSafe(from, to, 
				    kingSquare(alt(turn())));
  const Piece piece = pieceOnBoard(to);
  if (! pin(alt(turn())).test(piece.number()))
    return false;
  if (last_move.isDrop() || last_move.oldPtype() == KNIGHT)
    return true;
  const Direction d=pinnedDir(piece);
  return primDir(d)
    !=primDirUnsafe(Board_Table.getShort8Unsafe(piece.owner(), from,to));
}

void osl::NumEffectState::makeMove(Move move)
{
  assert(turn() == move.player());
  if (move.isPass()) {
    makeMovePass();
    return;
  }

  assert(isAlmostValidMove(move));
  const Square from=move.from();
  const Square to=move.to();
  if (from.isPieceStand())
  {
    doDropMove(to,move.ptype());
  }
  else
  {
    const Piece captured = pieceOnBoard(to);
    if (captured != Piece::EMPTY())
    {
      doCaptureMove(from,to,captured,move.promoteMask());
    }
    else
    {
      doSimpleMove(from,to,move.promoteMask());
    }
  }
  changeTurn();
}

void osl::NumEffectState::
doSimpleMove(Square from, Square to, int promoteMask)
{
  Piece oldPiece;
  int num;
  PtypeO oldPtypeO, newPtypeO;
  CArray<PieceMask,2> pin_or_open_backup;
  KingMobility king_mobility_backup;
  PieceMask promoted_backup;
  CArray<PieceMask,2> effected_mask_backup;
  CArray<PieceMask,2> effected_changed_mask_backup;
  CArray<uint64_t,2> king8infos_backup;
  mobility::MobilityTable mobilityTable;
  if (turn()==BLACK){
    prologueSimple(Player2Type<BLACK>(), from, to, promoteMask, 
		   oldPiece, num, oldPtypeO, newPtypeO, 
		   pin_or_open_backup, king_mobility_backup,
		   promoted_backup, effected_mask_backup, effected_changed_mask_backup,king8infos_backup,mobilityTable);
  }
  else{
    prologueSimple(Player2Type<WHITE>(), from, to, promoteMask, 
		   oldPiece, num, oldPtypeO, newPtypeO, 
		   pin_or_open_backup, king_mobility_backup,
		   promoted_backup, effected_mask_backup, effected_changed_mask_backup,king8infos_backup,mobilityTable);
  }
  if (promoteMask!=0 && num < PtypeTraits<PAWN>::indexLimit)
    clearPawn(turn(),from);
}
void osl::NumEffectState::
doCaptureMove(Square from, Square to, Piece target, int promoteMask)
{
  Piece oldPiece;
  PtypeO oldPtypeO, capturePtypeO, newPtypeO;
  int num0, num1, num1Index;
  mask_t num1Mask;
  CArray<PieceMask,2> pin_or_open_backup;
  KingMobility king_mobility_backup;
  PieceMask promoted_backup;
  CArray<PieceMask,2> effected_mask_backup;
  CArray<PieceMask,2> effected_changed_mask_backup;
  CArray<uint64_t,2> king8infos_backup;
  mobility::MobilityTable mobilityTable;
  if(turn()==BLACK){
    prologueCapture(Player2Type<BLACK>(), from, to, target, promoteMask, oldPiece, oldPtypeO, 
		    capturePtypeO, newPtypeO, num0, num1, num1Index,num1Mask, 
		    pin_or_open_backup, king_mobility_backup,
		    promoted_backup, effected_mask_backup, effected_changed_mask_backup,king8infos_backup,mobilityTable);
  }
  else{
    prologueCapture(Player2Type<WHITE>(), from, to, target, promoteMask, oldPiece, oldPtypeO, 
		    capturePtypeO, newPtypeO, num0, num1, num1Index,num1Mask, 
		    pin_or_open_backup, king_mobility_backup,
		    promoted_backup, effected_mask_backup, effected_changed_mask_backup,king8infos_backup,mobilityTable);
  }
  const Ptype capturePtype=target.ptype();
  if (capturePtype==PAWN)
    clearPawn(alt(turn()),to);
  if (promoteMask!=0 && num0<PtypeTraits<PAWN>::indexLimit)
    clearPawn(turn(),from);
}

void osl::NumEffectState::
doDropMove(Square to,Ptype ptype)
{
  Piece oldPiece;
  PtypeO ptypeO;
  int num, numIndex;
  mask_t numMask;
  CArray<PieceMask,2> pin_or_open_backup;
  KingMobility king_mobility_backup;
  CArray<PieceMask,2> effected_mask_backup;
  CArray<PieceMask,2> effected_changed_mask_backup;
  CArray<uint64_t,2> king8infos_backup;
  mobility::MobilityTable mobilityTable;
  if(turn()==BLACK){
    prologueDrop(Player2Type<BLACK>(), to, ptype, oldPiece, num, ptypeO, numIndex, numMask, 
		 pin_or_open_backup, king_mobility_backup,
		 effected_mask_backup, effected_changed_mask_backup,king8infos_backup,mobilityTable);
  }
  else{
    prologueDrop(Player2Type<WHITE>(), to, ptype, oldPiece, num, ptypeO, numIndex, numMask, 
		 pin_or_open_backup, king_mobility_backup,
		 effected_mask_backup, effected_changed_mask_backup,king8infos_backup,mobilityTable);
  }
  if (ptype==PAWN)
    setPawn(turn(),to);
}

template<osl::Player P>
void osl::NumEffectState::
prologueSimple(Player2Type<P>, Square from, Square to, int promoteMask,
	       Piece& oldPiece, int& num, 
	       PtypeO& oldPtypeO, PtypeO& new_ptypeo,
	       CArray<PieceMask,2>& pin_or_open_backup,
	       KingMobility& king_mobility_backup,
	       PieceMask& promoted_backup,
	       CArray<PieceMask,2>& effected_mask_backup,
	       CArray<PieceMask,2>& effected_changed_mask_backup,
	       CArray<uint64_t,2>& king8infos_backup,
	       MobilityTable &mobility_backup)
{
  mobility_backup = effects.mobilityTable;
  pin_or_open_backup = pin_or_open;
  king_mobility_backup = king_mobility;
  effected_mask_backup = effects.effected_mask;
  effected_changed_mask_backup = effects.effected_changed_mask;
  king8infos_backup=king8infos;

  oldPiece=pieceAt(from);
  Piece newPiece=oldPiece.promoteWithMask(promoteMask);
  newPiece+=(to-from);
  num=oldPiece.number();

  oldPtypeO=oldPiece.ptypeO();
  new_ptypeo=newPiece.ptypeO();
  // 自分自身の効きを外す
  setPieceOf(num,newPiece);
  effects.clearChangedEffects();
  effects.clearEffectedChanged();
  effects.template doEffect<NumBitmapEffect::Sub,true>(*this,oldPtypeO,from,num);
  // 自分自身がブロックしていたpromote?の延長
  // あるいは自分自身のブロック
  effects.effectedNumTable[num].clear();
  setBoard(to,newPiece);
  effects.template doBlockAt<NumBitmapEffect::Sub,true>(*this,to,num);
  setBoard(from,Piece::EMPTY());
  effects.template doBlockAt<NumBitmapEffect::Add,true>(*this,from,num);
  effects.template doEffect<NumBitmapEffect::Add,true>(*this,new_ptypeo,to,num);

  if (oldPtypeO == newPtypeO(P,KING))
    makePinOpen(P);
  else {
    Direction lastD=UL;
    pin_or_open[P].reset(num);
    recalcPinOpen(from,lastD,P);
    recalcPinOpen(to,lastD,P);
  }
  {
    Direction lastD=UL;
    pin_or_open[alt(P)].reset(num);
    recalcPinOpen(from,lastD,alt(P));
    recalcPinOpen(to,lastD,alt(P));
  }
  promoted_backup = promoted;
  if (promoteMask)
    promoted.set(num);
  if(hasEffectAt(BLACK,to))
    effects.effected_mask[BLACK].set(num);
  else
    effects.effected_mask[BLACK].reset(num);
  if(hasEffectAt(WHITE,to))
    effects.effected_mask[WHITE].set(num);
  else
    effects.effected_mask[WHITE].reset(num);
  effects.effected_changed_mask[BLACK].set(num);
  effects.effected_changed_mask[WHITE].set(num);
  {
    BoardMask changed=changedEffects(BLACK)|changedEffects(WHITE);
    changed.set(from);
    changed.set(to);
    if(changed.anyInRange(Board_Mask_Table3x3.mask(kingSquare<BLACK>()))
       || pin_or_open[BLACK]!=pin_or_open_backup[BLACK])
      makeKing8Info<BLACK>();
    if(changed.anyInRange(Board_Mask_Table3x3.mask(kingSquare<WHITE>()))
       || pin_or_open[WHITE]!=pin_or_open_backup[WHITE])
      makeKing8Info<WHITE>();
  }
}

void osl::NumEffectState::
epilogueSimple(Square from, Square to, Piece oldPiece, 
	       int num, PtypeO oldPtypeO, PtypeO newPtypeO,
	       const CArray<PieceMask,2>& pin_or_open_backup,
	       const KingMobility& king_mobility_backup,
	       const PieceMask& promoted_backup,
	       const CArray<PieceMask,2>& effected_mask_backup,
	       const CArray<PieceMask,2>& effected_changed_mask_backup,
	       const CArray<uint64_t,2>& king8infos_backup,
	       const MobilityTable & mobility_backup)
{
  setPieceOf(num,oldPiece);
  effects.doEffect<NumBitmapEffect::Sub,false>(*this,newPtypeO,to,num);
  setBoard(from,oldPiece);
  effects.effectedNumTable[num].clear();
  effects.doBlockAt<NumBitmapEffect::Sub,false>(*this,from,num);
  setBoard(to,Piece::EMPTY());
  effects.doBlockAt<NumBitmapEffect::Add,false>(*this,to,num);
  effects.doEffect<NumBitmapEffect::Add,false>(*this,oldPtypeO,from,num);
  effects.invalidateChangedEffects();
  pin_or_open = pin_or_open_backup;
  king_mobility = king_mobility_backup;
  promoted = promoted_backup;
  effects.effected_mask = effected_mask_backup;
  effects.effected_changed_mask = effected_changed_mask_backup;
  effects.mobilityTable = mobility_backup;
  king8infos = king8infos_backup;
}

template<osl::Player P>
void osl::NumEffectState::
prologueDrop(Player2Type<P>, Square to, Ptype ptype,
	     Piece& oldPiece, int& num, PtypeO& ptypeO, 
	     int& numIndex, mask_t& numMask,
	     CArray<PieceMask,2>& pin_or_open_backup,
	     KingMobility& king_mobility_backup,
	     CArray<PieceMask,2>& effected_mask_backup,
	     CArray<PieceMask,2>& effected_changed_mask_backup,
	     CArray<uint64_t,2>& king8infos_backup,
	     MobilityTable &mobility_backup)
{
  king8infos_backup = king8infos;
  mobility_backup = effects.mobilityTable;
  pin_or_open_backup = pin_or_open;
  king_mobility_backup = king_mobility;
  effected_mask_backup = effects.effected_mask;
  effected_changed_mask_backup = effects.effected_changed_mask;
#if OSL_WORDSIZE == 64
  numIndex=0;
#elif OSL_WORDSIZE == 32
  numIndex=Ptype_Table.getIndex(ptype);
#endif
  const mask_t ownMochigoma=
    standMask(P).getMask(numIndex) & Ptype_Table.getMaskLow(ptype);
  assert(ownMochigoma.any());
  numMask=ownMochigoma.lowestBit();
  int numLow = ownMochigoma.bsf();
  num = numLow|(numIndex<<5);
  oldPiece=pieceOf(num);
  Piece newPiece=oldPiece;
  newPiece+=to-Square::STAND();
  ptypeO=newPiece.ptypeO();
  setPieceOf(num,newPiece);
  effects.clearChangedEffects();
  effects.clearEffectedChanged();
  effects.template doBlockAt<NumBitmapEffect::Sub,true>(*this,to,num);
  effects.template doEffect<NumBitmapEffect::Add,true>(*this,ptypeO,to,num);
  setBoard(to,newPiece);
  standMask(P).xorMask(numIndex,numMask);
  stand_count[P][ptype-PTYPE_BASIC_MIN]--;
  pieces_onboard[P].xorMask(numIndex,numMask);
  {
    Direction lastD=UL;
    recalcPinOpen(to,lastD,P);
  }
  {
    Direction lastD=UL;
    recalcPinOpen(to,lastD,alt(P));
  }
  if(hasEffectAt(BLACK,to))
    effects.effected_mask[BLACK].set(num);
  else
    effects.effected_mask[BLACK].reset(num);
  if (hasEffectAt(WHITE,to))
    effects.effected_mask[WHITE].set(num);
  else
    effects.effected_mask[WHITE].reset(num);
  effects.effected_changed_mask[BLACK].set(num);
  effects.effected_changed_mask[WHITE].set(num);
  {
    BoardMask changed=changedEffects(BLACK)|changedEffects(WHITE);
    changed.set(to);
    if(changed.anyInRange(Board_Mask_Table3x3.mask(kingSquare<BLACK>()))
       || pin_or_open[BLACK]!=pin_or_open_backup[BLACK])
      makeKing8Info<BLACK>();
    if(changed.anyInRange(Board_Mask_Table3x3.mask(kingSquare<WHITE>()))
       || pin_or_open[WHITE]!=pin_or_open_backup[WHITE])
      makeKing8Info<WHITE>();
  }
}

template<osl::Player P>
void osl::NumEffectState::
epilogueDrop(Player2Type<P>, Square to, Ptype ptype, Piece oldPiece, 
	     int num, PtypeO ptypeO, int numIndex, mask_t numMask,
	     const CArray<PieceMask,2>& pin_or_open_backup,
	     const KingMobility& king_mobility_backup,
	     const CArray<PieceMask,2>& effected_mask_backup,
	     const CArray<PieceMask,2>& effected_changed_mask_backup,
	     const CArray<uint64_t,2>& king8infos_backup,
	     const MobilityTable& mobility_backup)
{
  standMask(P).xorMask(numIndex,numMask);
  stand_count[P][ptype-PTYPE_BASIC_MIN]++;
  pieces_onboard[P].xorMask(numIndex,numMask);
  setBoard(to,Piece::EMPTY());
  effects.template doEffect<NumBitmapEffect::Sub,false>(*this,ptypeO,to,num);
  effects.template doBlockAt<NumBitmapEffect::Add,false>(*this,to,num);
  setPieceOf(num,oldPiece);
  effects.effectedNumTable[num].clear();
  effects.invalidateChangedEffects();
  pin_or_open = pin_or_open_backup;
  king_mobility = king_mobility_backup;
  effects.effected_mask = effected_mask_backup;
  effects.effected_changed_mask = effected_changed_mask_backup;
  effects.mobilityTable = mobility_backup;
  king8infos = king8infos_backup;
}

template<osl::Player P>
void osl::NumEffectState::
prologueCapture(Player2Type<P>, Square from, Square to, Piece target, 
		int promoteMask,
		Piece& oldPiece, PtypeO& oldPtypeO, PtypeO& capturePtypeO, 
		PtypeO& new_ptypeo, int& num0, int& num1, 
		int& num1Index, mask_t& num1Mask,
		CArray<PieceMask,2>& pin_or_open_backup,
		KingMobility& king_mobility_backup,
		PieceMask& promoted_backup,
		CArray<PieceMask,2>& effected_mask_backup,
		CArray<PieceMask,2>& effected_changed_mask_backup,
		CArray<uint64_t,2>& king8infos_backup,
		MobilityTable &mobility_backup)
{
  mobility_backup = effects.mobilityTable;
  pin_or_open_backup = pin_or_open;
  king_mobility_backup = king_mobility;
  effected_mask_backup = effects.effected_mask;
  effected_changed_mask_backup = effects.effected_changed_mask;
  king8infos_backup = king8infos;

  num1=target.number();
  num1Index=PieceMask::numToIndex(num1);
  num1Mask=PieceMask::numToMask(num1);
  pieces_onboard[alt(P)].xorMask(num1Index,num1Mask);
  standMask(P).xorMask(num1Index,num1Mask);
  oldPiece=pieceAt(from);
  Piece newPiece=oldPiece.promoteWithMask(promoteMask);
  newPiece+=(to-from);
  num0=oldPiece.number();
  setPieceOf(num0,newPiece);
  setPieceOf(num1,target.captured());
      
  oldPtypeO=oldPiece.ptypeO();
  new_ptypeo=newPiece.ptypeO();
  capturePtypeO=target.ptypeO();
  stand_count[P][unpromote(getPtype(capturePtypeO))-PTYPE_BASIC_MIN]++;
  effects.clearChangedEffects();
  effects.clearEffectedChanged();
  effects.setChangedPieces(effectSetAt(to));
  effects.template doEffect<NumBitmapEffect::Sub,true>(*this,capturePtypeO,to,num1);
  effects.template doEffect<NumBitmapEffect::Sub,true>(*this,oldPtypeO,from,num0);
  setBoard(from,Piece::EMPTY());
  effects.template doBlockAt<NumBitmapEffect::Add,true>(*this,from,num0);
  effects.effectedNumTable[num0]=effects.effectedNumTable[num1];
  effects.effectedNumTable[num1].clear();
  setBoard(to,newPiece);
  effects.template doEffect<NumBitmapEffect::Add,true>(*this,new_ptypeo,to,num0);

  if (oldPtypeO == newPtypeO(P,KING))
    makePinOpen(P);
  else {
    Direction lastD=UL;
    pin_or_open[P].reset(num0);
    pin_or_open[P].reset(num1); // captured is not pin
    recalcPinOpen(from,lastD,P);
    recalcPinOpen(to,lastD,P);
  }
  {
    Direction lastD=UL;
    pin_or_open[alt(P)].reset(num0);
    pin_or_open[alt(P)].reset(num1); // captured is not pin
    recalcPinOpen(from,lastD,alt(P));
    recalcPinOpen(to,lastD,alt(P));
  }
  promoted_backup = promoted;
  promoted.reset(num1);
  effects.effected_mask[BLACK].reset(num1);
  effects.effected_mask[WHITE].reset(num1);
  if (promoteMask)
    promoted.set(num0);
  if(hasEffectAt(BLACK,to))
    effects.effected_mask[BLACK].set(num0);
  else
    effects.effected_mask[BLACK].reset(num0);
  if(hasEffectAt(WHITE,to))
    effects.effected_mask[WHITE].set(num0);
  else
    effects.effected_mask[WHITE].reset(num0);
  effects.effected_changed_mask[BLACK].set(num0);
  effects.effected_changed_mask[WHITE].set(num0);
  {
    BoardMask changed=changedEffects(BLACK)|changedEffects(WHITE);
    changed.set(from);
    changed.set(to);
    if(changed.anyInRange(Board_Mask_Table3x3.mask(kingSquare<BLACK>()))
       || pin_or_open[BLACK]!=pin_or_open_backup[BLACK])
      makeKing8Info<BLACK>();
    if(changed.anyInRange(Board_Mask_Table3x3.mask(kingSquare<WHITE>()))
       || pin_or_open[WHITE]!=pin_or_open_backup[WHITE])
      makeKing8Info<WHITE>();
  }
}

template<osl::Player P>
void osl::NumEffectState::
epilogueCapture(Player2Type<P>, Square from, Square to, Piece target, 
		Piece oldPiece, PtypeO oldPtypeO, PtypeO capturePtypeO, 
		PtypeO newPtypeO, int num0, int num1, 
		int num1Index, mask_t num1Mask,
		const CArray<PieceMask,2>& pin_or_open_backup,
		const KingMobility& king_mobility_backup,
		const PieceMask& promoted_backup,
		const CArray<PieceMask,2>& effected_mask_backup,
		const CArray<PieceMask,2>& effected_changed_mask_backup,
		const CArray<uint64_t,2>& king8infos_backup,
		const MobilityTable &mobility_backup)
{
  standMask(P).xorMask(num1Index,num1Mask);
  stand_count[P][unpromote(getPtype(capturePtypeO))-PTYPE_BASIC_MIN]--;
  pieces_onboard[alt(P)].xorMask(num1Index,num1Mask);
  effects.effectedNumTable[num1]=effects.effectedNumTable[num0];
  effects.effectedNumTable[num0].clear();
  setPieceOf(num0,oldPiece);
  setPieceOf(num1,target);
  effects.template doEffect<NumBitmapEffect::Sub,false>(*this,newPtypeO,to,num0);
  setBoard(from,oldPiece);
  setBoard(to,target);
  effects.template doBlockAt<NumBitmapEffect::Sub,false>(*this,from,num0);
  effects.template doEffect<NumBitmapEffect::Add,false>(*this,capturePtypeO,to,num1);
  effects.template doEffect<NumBitmapEffect::Add,false>(*this,oldPtypeO,from,num0);
  effects.invalidateChangedEffects();
  pin_or_open = pin_or_open_backup;
  king_mobility = king_mobility_backup;
  promoted = promoted_backup;
  effects.effected_mask = effected_mask_backup;
  effects.effected_changed_mask = effected_changed_mask_backup;
  effects.mobilityTable = mobility_backup;
  king8infos = king8infos_backup;
}


#ifndef MINIMAL
bool osl::NumEffectState::isConsistent(bool showError) const
{
  if (!SimpleState::isConsistent(showError)) 
  {
    if (showError)
      std::cerr << "error before effect\n";
    return false;
  }
  effect::NumSimpleEffectTable effects1(*this);
  if (!(effects1==effects))
  {
    if (showError)
    {
      std::cerr << "Effect error 1" << std::endl;
      std::cerr << *this;
      for(int y=1;y<=9;y++)
	for(int x=9;x>0;x--)
	{
	  Square pos(x,y);
	  if (!(effects1.effectSetAt(pos)==effects.effectSetAt(pos)))
	  {
	    std::cerr << pos << ",real=" << effects.effectSetAt(pos) << ",ideal=" << effects1.effectSetAt(pos) << std::endl;
	  }
	}
      for(int num=0;num<=39;num++){
	for(int i=0;i<8;i++){
	  Direction d=static_cast<Direction>(i);
	  if(effects.effectedNumTable[num][d]!=effects1.effectedNumTable[num][d]){
	    std::cerr << "piece=" << pieceOf(num) << ",num=" << num << ",d=" << d << ",v1=" << effects.effectedNumTable[num][d] << ",v2=" << effects1.effectedNumTable[num][d] << std::endl;
	  }
	}
      }
      std::cerr << effects.effectedNumTable << std::endl;
    }
    return false;
  }
  for (int z=0; z<2; ++z) {
    const Player p = indexToPlayer(z);
#ifdef ALLOW_KING_ABSENCE
    if (kingSquare(p).isPieceStand())
      continue;
#endif
#if 0
    const PieceMask pin2 = effect_util::Pin::make(*this, p);
    if (pin(p) != pin2) {
      if (showError)
	std::cerr << "pin for " << p << " differs " << pin(p) << " " << pin2 << "\n";
      return false;
    }
#endif
    King8Info king8info2 = King8Info::make(alt(p), *this);
    if (King8Info(Iking8Info(p)).uint64Value() != king8info2.uint64Value()) {
      if (showError)
	std::cerr << "king8info for " << p << " differs \n" << King8Info(Iking8Info(p)) << "\n" << king8info2 << "\n";
      return false;
    }      
  }
  for (int i=0; i<Piece::SIZE; ++i) {
    const Piece p = pieceOf(i);
    if (p.isOnBoard()) {
      if (promoted.test(i) != p.isPromoted()) {
	if (showError)
	  std::cerr << "promoted differs " << p << " " << promoted << " " << promoted.test(i) << "\n";
	return false;
      }
    }
  }
  return true;
}
#endif

bool osl::NumEffectState::isConsistent(const NumEffectState& prev, Move moved, bool show_error) const
{
  // test changedEffects
  const CArray<BoardMask,2> changed_squares
    = {{ changedEffects(BLACK), changedEffects(WHITE) }};
  const BoardMask changed_all = changed_squares[BLACK] | changed_squares[WHITE];
  CArray<BoardMask, Piece::SIZE> each_effect, prev_effect;
  for (int i=0; i<Piece::SIZE; ++i) {
    each_effect[i].clear();
    prev_effect[i].clear();
  }
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      const Square sq(x, y);
      for (int i=0; i<Piece::SIZE; ++i) {
	if (effectSetAt(sq).test(i))
	  each_effect[i].set(sq);
	if (prev.effectSetAt(sq).test(i))
	  prev_effect[i].set(sq);
      }
      if (! changed_all.test(sq))
      {
	if (effectSetAt(sq) != prev.effectSetAt(sq)) {
#ifndef MINIMAL
	  if (show_error)
	    std::cerr << "changedEffects unset\n" << *this << moved << sq << "\n";	  
#endif
	  return false;
	}
      }
      for (int i=0; i<2; ++i) 
      {
	const Player pl = indexToPlayer(i);
	if (! changed_squares[pl].test(sq))
	{
	  if ((effectSetAt(sq) & piecesOnBoard(pl))
	      != (prev.effectSetAt(sq) & prev.piecesOnBoard(pl))) {
#ifndef MINIMAL
	    if (show_error)
	      std::cerr << "changedEffects unset for " << pl << "\n" << *this << moved << sq << "\n";
#endif
	    return false;
	  }
	}
      }
    }
  }
  // test changedPieces()
  const NumBitmapEffect changed_effect_pieces = changedPieces(); 
  for (int i=0; i<Piece::SIZE; ++i) {
    if (each_effect[i] == prev_effect[i])
      continue;
    if (! changed_effect_pieces.test(i)) {
#ifndef MINIMAL
      if (show_error)
	std::cerr << "changedPieces() unset\n" << *this << moved << i 
		  << " " << each_effect[i] << " != " <<  prev_effect[i] << "\n";
#endif
      return false;
    }
  }
  // test effectedChanged(Player pl)
  for (int i=0; i<Piece::SIZE; ++i) 
  {
    for (int j=0; j<2; ++j) 
    {
      const Player pl = indexToPlayer(j);
      if (prev.pieceOf(i).square() == moved.to())
	continue;		// captured
      if (prev.effectedMask(pl).test(i) != effectedMask(pl).test(i)) {
	if (! effectedChanged(pl).test(i)) {
#ifndef MINIMAL
	  if (show_error)
	    std::cerr << "effectedChanged(" << pl << ") unset\n" << *this << moved << i 
		      << " " << prev.effectedChanged(pl) << " != " << prev.effectedChanged(WHITE) << "\n";
#endif
	  return false;
	}
      }
    }
  }
  return true;
}

template <bool show_error>
bool
#if (defined __GNUC__) && (! defined GPSONE) && (! defined GPSUSIONE)
__attribute__ ((used,noinline))
#endif
osl::NumEffectState::isAlmostValidMove(Move move) const{
  assert(move.isValid());
  assert(move.isNormal());
  assert(this->turn() == move.player());
  assert(isValidMoveByRule(move, true));

  const Square from=move.from();
  if (from.isPieceStand()) // 打つ手
    return isAlmostValidDrop<show_error>(move);
  const Square to=move.to();
  const Piece from_piece = this->pieceAt(from);
    
  if (! testValidityOtherThanEffect<show_error>(move))
    return false;
  if(!hasEffectByPiece(from_piece,to)){
    if (show_error) {
      std::cerr << " No such move2 : " << move << std::endl;
    }
    return false;
  }
  return true;
}

bool osl::NumEffectState::
isAlmostValidMove(Move move,bool show_error) const{
#ifdef MINIMAL
  show_error=false;
#endif
  if(show_error)
    return isAlmostValidMove<true>(move);
  else
    return isAlmostValidMove<false>(move);
}

#ifndef MINIMAL
void osl::NumEffectState::showEffect(std::ostream& os) const
{
  os<< static_cast<SimpleState const&>(*this);
  for(int y=1;y<=9;y++){
    os << 'P' << y;  
    for(int x=9;x>0;x--){
      Square pos(x,y);
      os << csa::show(pieceAt(pos)) << effectSetAt(pos);
    }
    os << std::endl;
  }
  // 持ち駒の表示
  for(int num=0;num<Piece::SIZE;num++){
    if (standMask(BLACK).test(num)){
      os << "P+00" << csa::show(Piece_Table.getPtypeOf(num))
	 << std::endl;
    }
    else if (standMask(WHITE).test(num)){
      os << "P-00" << csa::show(Piece_Table.getPtypeOf(num))
	 << std::endl;
    }
  }
}
#endif

osl::PieceMask osl::NumEffectState::
makePinOpen(osl::Square target,osl::Player defense)
{
  PieceMask pins;
  if(target.isPieceStand()) return pins;
  PieceMask mask=piecesOnBoard(alt(defense));
  makePinOpenDir<UL>(target,pins,mask,defense);
  makePinOpenDir<U>(target,pins,mask,defense);
  makePinOpenDir<UR>(target,pins,mask,defense);
  makePinOpenDir<L>(target,pins,mask,defense);
  makePinOpenDir<R>(target,pins,mask,defense);
  makePinOpenDir<DL>(target,pins,mask,defense);
  makePinOpenDir<D>(target,pins,mask,defense);
  makePinOpenDir<DR>(target,pins,mask,defense);
  return pins;
}

void osl::NumEffectState::
makePinOpen(osl::Player defense)
{
  pin_or_open[defense]=makePinOpen(kingSquare(defense),defense);
}

const osl::mask_t osl::NumEffectState::
allEffectAt(Player attack, Ptype ptype, Square target) const
{
  switch (ptype) {
  case PAWN: case PPAWN:
    return allEffectAt<PAWN>(attack, target);
  case LANCE: case PLANCE:
    return allEffectAt<LANCE>(attack, target);
  case KNIGHT: case PKNIGHT:
    return allEffectAt<KNIGHT>(attack, target);
  case SILVER: case PSILVER:
    return allEffectAt<SILVER>(attack, target);
  case GOLD:
    return allEffectAt<GOLD>(attack, target);
  case BISHOP: case PBISHOP:
    return allEffectAt<BISHOP>(attack, target);
  case ROOK: case PROOK:
    return allEffectAt<ROOK>(attack, target);
  case KING:
    return allEffectAt<KING>(attack, target);
  default:
    assert(0);
  }
  return mask_t();
}

void osl::NumEffectState::copyFrom(const NumEffectState& src)
{
#ifndef MINIMAL
  (*this).used_mask=src.used_mask;
#endif
  (*this).stand_mask=src.stand_mask;
#if (defined(__i386__) || defined(__x86_64__)) && !defined(OSL_NO_SSE)
  {  
    v2di b16=*((v2di*)&src.board[16]);
    v2di b20=*((v2di*)&src.board[20]);
    v2di b24=*((v2di*)&src.board[24]);
    v2di b32=*((v2di*)&src.board[32]);
    v2di b36=*((v2di*)&src.board[36]);
    v2di b40=*((v2di*)&src.board[40]);
    v2di b48=*((v2di*)&src.board[48]);
    v2di b52=*((v2di*)&src.board[52]);
    v2di b56=*((v2di*)&src.board[56]);

    *((v2di*)&(*this).board[16])=b16;
    *((v2di*)&(*this).board[20])=b20;
    *((v2di*)&(*this).board[24])=b24;
    *((v2di*)&(*this).board[32])=b32;
    *((v2di*)&(*this).board[36])=b36;
    *((v2di*)&(*this).board[40])=b40;
    *((v2di*)&(*this).board[48])=b48;
    *((v2di*)&(*this).board[52])=b52;
    *((v2di*)&(*this).board[56])=b56;


    v2di b64=*((v2di*)&src.board[64]);
    v2di b68=*((v2di*)&src.board[68]);
    v2di b72=*((v2di*)&src.board[72]);

    v2di b80=*((v2di*)&src.board[80]);
    v2di b84=*((v2di*)&src.board[84]);
    v2di b88=*((v2di*)&src.board[88]);

    v2di b96=*((v2di*)&src.board[96]);
    v2di b100=*((v2di*)&src.board[100]);
    v2di b104=*((v2di*)&src.board[104]);


    *((v2di*)&(*this).board[64])=b64;
    *((v2di*)&(*this).board[68])=b68;
    *((v2di*)&(*this).board[72])=b72;

    *((v2di*)&(*this).board[80])=b80;
    *((v2di*)&(*this).board[84])=b84;
    *((v2di*)&(*this).board[88])=b88;

    *((v2di*)&(*this).board[96])=b96;
    *((v2di*)&(*this).board[100])=b100;
    *((v2di*)&(*this).board[104])=b104;

    v2di b112=*((v2di*)&src.board[112]);
    v2di b116=*((v2di*)&src.board[116]);
    v2di b120=*((v2di*)&src.board[120]);

    v2di b128=*((v2di*)&src.board[128]);
    v2di b132=*((v2di*)&src.board[132]);
    v2di b136=*((v2di*)&src.board[136]);

    v2di b144=*((v2di*)&src.board[144]);
    v2di b148=*((v2di*)&src.board[148]);
    v2di b152=*((v2di*)&src.board[152]);

    *((v2di*)&(*this).board[112])=b112;
    *((v2di*)&(*this).board[116])=b116;
    *((v2di*)&(*this).board[120])=b120;

    *((v2di*)&(*this).board[128])=b128;
    *((v2di*)&(*this).board[132])=b132;
    *((v2di*)&(*this).board[136])=b136;

    *((v2di*)&(*this).board[144])=b144;
    *((v2di*)&(*this).board[148])=b148;
    *((v2di*)&(*this).board[152])=b152;

    v2di p0=*((v2di*)&src.pieces[0]);
    v2di p4=*((v2di*)&src.pieces[4]);
    v2di p8=*((v2di*)&src.pieces[8]);
    v2di p12=*((v2di*)&src.pieces[12]);
    v2di p16=*((v2di*)&src.pieces[16]);
    v2di p20=*((v2di*)&src.pieces[20]);
    v2di p24=*((v2di*)&src.pieces[24]);
    v2di p28=*((v2di*)&src.pieces[28]);
    v2di p32=*((v2di*)&src.pieces[32]);
    v2di p36=*((v2di*)&src.pieces[36]);
    *((v2di*)&(*this).pieces[0])=p0;
    *((v2di*)&(*this).pieces[4])=p4;
    *((v2di*)&(*this).pieces[8])=p8;
    *((v2di*)&(*this).pieces[12])=p12;
    *((v2di*)&(*this).pieces[16])=p16;
    *((v2di*)&(*this).pieces[20])=p20;
    *((v2di*)&(*this).pieces[24])=p24;
    *((v2di*)&(*this).pieces[28])=p28;
    *((v2di*)&(*this).pieces[32])=p32;
    *((v2di*)&(*this).pieces[36])=p36;
  }
#else
  for(int x=1;x<=9;x++)
    for(int y=1;y<=9;y++)
      (*this).board[Square(x,y).index()]=src.board[Square(x,y).index()];
  (*this).pieces=src.pieces;
#endif
  (*this).pawnMask=src.pawnMask;
  this->stand_count = src.stand_count;
  this->player_to_move=src.player_to_move;
  effects.copyFrom(src.effects);
  this->pieces_onboard=src.pieces_onboard;
  (*this).promoted=src.promoted;
  (*this).pin_or_open=src.pin_or_open;
  (*this).king_mobility=src.king_mobility;
  (*this).king8infos=src.king8infos;
}

void osl::NumEffectState::copyFrom(const SimpleState& src)
{
  copyFrom(NumEffectState(src));
}

bool osl::NumEffectState::isSafeMove(Move move) const
{
  using namespace move_classifier;
  return ConditionAdaptor<SafeMove>::isMember(*this, move);
}
bool osl::NumEffectState::isCheck(Move move) const
{
  using namespace move_classifier;
  return PlayerMoveAdaptor<Check>::isMember(*this, move);
}
bool osl::NumEffectState::isPawnDropCheckmate(Move move) const
{
  using namespace move_classifier;
  return PlayerMoveAdaptor<PawnDropCheckmate>::isMember(*this, move);
}
bool osl::NumEffectState::isDirectCheck(Move move) const
{
  using namespace move_classifier;
  return PlayerMoveAdaptor<DirectCheck>::isMember(*this, move);
}

bool osl::NumEffectState::isOpenCheck(Move move) const
{
  using namespace move_classifier;
  return ConditionAdaptor<OpenCheck>::isMember(*this, move);
}

#ifndef MINIMAL
void osl::NumEffectState::generateAllUnsafe(MoveVector& out) const
{
  move_action::Store store(out);
  move_generator::AllMoves<move_action::Store>::generate(turn(), *this, store);
}
void osl::NumEffectState::generateLegal(MoveVector& moves) const
{
  if (inCheck()) {
    // 王手がかかっている時は防ぐ手のみを生成, 王手回避は不成も生成
    GenerateEscapeKing::generate(*this, moves);
  }
  else {
    // そうでなければ全ての手を生成
    MoveVector all_moves;
    GenerateAllMoves::generate(turn(), *this, all_moves);
    // この指手は，玉の素抜きがあったり，打歩詰の可能性があるので
    // 確認が必要
    std::copy_if(all_moves.begin(), all_moves.end(), std::back_inserter(moves),
		 [&](Move m){
		   return this->isSafeMove(m) && ! this->isPawnDropCheckmate(m);
		 });
  }
}

void osl::NumEffectState::generateWithFullUnpromotions(MoveVector& moves) const
{
  generateLegal(moves);
  if (inCheck())
    return;
  for (int i=0, iend=moves.size(); i<iend; ++i) {
    const Move move = moves[i];
    if (move.hasIgnoredUnpromote())
      moves.push_back(move.unpromote());
  }
}
#endif

void osl::NumEffectState::
findEffect(Player P, Square target, PieceVector& out) const
{
  effect_action::StorePiece store(&out);
  forEachEffect(P, target, store);
}

namespace osl
{
  // explicit template instantiation

  template bool NumEffectState:: 
  hasEffectByWithRemove<BLACK>(Square, Square) const;
  template bool NumEffectState:: 
  hasEffectByWithRemove<WHITE>(Square, Square) const;
  template void NumEffectState::makeKing8Info<BLACK>();
  template void NumEffectState::makeKing8Info<WHITE>();


  template void NumEffectState::
  prologueSimple(Player2Type<BLACK>, Square, Square, int, Piece&, int&, 
		 PtypeO&, PtypeO&, CArray<PieceMask,2>&, KingMobility&,
		 PieceMask&, CArray<PieceMask,2>&, CArray<PieceMask,2>&,
		 CArray<uint64_t,2>&, MobilityTable&);
  template void NumEffectState::
  prologueSimple(Player2Type<WHITE>, Square, Square, int, Piece&, int&, 
		 PtypeO&, PtypeO&, CArray<PieceMask,2>&, KingMobility&,
		 PieceMask&, CArray<PieceMask,2>&, CArray<PieceMask,2>&,
		 CArray<uint64_t,2>&, MobilityTable&);

  template void NumEffectState::
  prologueCapture(Player2Type<BLACK>, Square, Square, Piece, int, Piece&,
		  PtypeO&, PtypeO&, PtypeO&, int&, int&, int&, mask_t&,
		  CArray<PieceMask,2>&, KingMobility&, PieceMask&,
		  CArray<PieceMask,2>&, CArray<PieceMask,2>&,
		  CArray<uint64_t,2>&, MobilityTable&);
  template void NumEffectState::
  prologueCapture(Player2Type<WHITE>, Square, Square, Piece, int, Piece&,
		  PtypeO&, PtypeO&, PtypeO&, int&, int&, int&, mask_t&,
		  CArray<PieceMask,2>&, KingMobility&, PieceMask&,
		  CArray<PieceMask,2>&, CArray<PieceMask,2>&,
		  CArray<uint64_t,2>&, MobilityTable&);

  template void NumEffectState::
  prologueDrop(Player2Type<BLACK>, Square, Ptype, Piece&, int&, PtypeO&, 
	       int&, mask_t&, CArray<PieceMask,2>&, KingMobility&,
	       CArray<PieceMask,2>&, CArray<PieceMask,2>&,
	       CArray<uint64_t,2>&, MobilityTable&);
  template void NumEffectState::
  prologueDrop(Player2Type<WHITE>, Square, Ptype, Piece&, int&, PtypeO&, 
	       int&, mask_t&, CArray<PieceMask,2>&, KingMobility&,
	       CArray<PieceMask,2>&, CArray<PieceMask,2>&,
	       CArray<uint64_t,2>&, MobilityTable&);

  template void NumEffectState::
  epilogueCapture(Player2Type<BLACK>, Square, Square, Piece, Piece, PtypeO, PtypeO, 
		  PtypeO, int, int, int, mask_t, const CArray<PieceMask,2>&,
		  const KingMobility&, const PieceMask&, const CArray<PieceMask,2>&,
		  const CArray<PieceMask,2>&, const CArray<uint64_t,2>&,
		  const MobilityTable&);
  template void NumEffectState::
  epilogueCapture(Player2Type<WHITE>, Square, Square, Piece, Piece, PtypeO, PtypeO, 
		  PtypeO, int, int, int, mask_t, const CArray<PieceMask,2>&,
		  const KingMobility&, const PieceMask&, const CArray<PieceMask,2>&,
		  const CArray<PieceMask,2>&, const CArray<uint64_t,2>&,
		  const MobilityTable&);
  template void NumEffectState::
  epilogueDrop(Player2Type<BLACK>, Square, Ptype, Piece, int, PtypeO, int, mask_t,
	       const CArray<PieceMask,2>&, const KingMobility&, const CArray<PieceMask,2>&,
	       const CArray<PieceMask,2>&, const CArray<uint64_t,2>&, const MobilityTable&);
  template void NumEffectState::
  epilogueDrop(Player2Type<WHITE>, Square, Ptype, Piece, int, PtypeO, int, mask_t,
	       const CArray<PieceMask,2>&, const KingMobility&, const CArray<PieceMask,2>&,
	       const CArray<PieceMask,2>&, const CArray<uint64_t,2>&, const MobilityTable&);  

#ifndef DFPNSTATONE
  template Piece 
  NumEffectState::safeCaptureNotByKing<BLACK>(Square, Piece) const;
  template Piece 
  NumEffectState::safeCaptureNotByKing<WHITE>(Square, Piece) const;
#endif
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
