/* simpleState.cc
 */

#include "osl/simpleState.h"
#include "osl/simpleState.tcc"
#include "osl/csa.h"
#include "osl/bits/pieceTable.h"
#include "osl/bits/pieceStand.h"
#include <iostream>
#include <stdexcept>

osl::SimpleState::SimpleState() {
  init();
}

osl::SimpleState::SimpleState(Handicap h) {
  init(h);
}

void osl::SimpleState::initPawnMask(){
  for (Ptype ptype: PieceStand::order) {
    stand_count[BLACK][ptype - PTYPE_BASIC_MIN] = countPiecesOnStandBit(BLACK, ptype);
    stand_count[WHITE][ptype - PTYPE_BASIC_MIN] = countPiecesOnStandBit(WHITE, ptype);
  }

  pawnMask[0].clearAll();
  pawnMask[1].clearAll();
  for(int num=PtypeTraits<PAWN>::indexMin;
      num< PtypeTraits<PAWN>::indexLimit; num++){
    Piece p=pieceOf(num);
    Player player=p.owner();
    Square pos=p.square();
    if(!pos.isPieceStand() && !p.isPromotedNotKingGold()){
      if (isPawnMaskSet(player,pos.x()))
      {
	throw CsaIOError("2FU!");
      }
      pawnMask[player].set(pos);
    }
  }
  assert(isConsistent(true));
}

void osl::SimpleState::init() {
  player_to_move=BLACK;
  for (int ipos=0;ipos<Square::SIZE;ipos++) {
    setBoard(Square::nth(ipos),Piece::EDGE());
  }
  for (int y=1;y<=9;y++)
    for (int x=9;x>0;x--) {
      setBoard(Square(x,y),Piece::EMPTY());
    }
  //  promoteMask.clearAll();
  stand_mask[BLACK].resetAll();
  stand_mask[WHITE].resetAll();
  stand_count[BLACK].fill(0);
  stand_count[WHITE].fill(0);
  used_mask.resetAll();
  pawnMask[0].clearAll();
  pawnMask[1].clearAll();
  for (int num=0;num<Piece::SIZE;num++){
    pieces[num]=Piece(WHITE,Piece_Table.getPtypeOf(num),num,Square::STAND());
  }
}
  

void osl::SimpleState::init(Handicap h) {
  init();
  if (h != HIRATE) {
    std::cerr << "unsupported handicap\n";
    throw std::runtime_error("unsupported handicap");
  }
  // 歩
  for (int x=9;x>0;x--) {
    setPiece(BLACK,Square(x,7),PAWN);
    setPiece(WHITE,Square(x,3),PAWN);
  }
  // 
  setPiece(BLACK,Square(1,9),LANCE);
  setPiece(BLACK,Square(9,9),LANCE);
  setPiece(WHITE,Square(1,1),LANCE);
  setPiece(WHITE,Square(9,1),LANCE);
  //
  setPiece(BLACK,Square(2,9),KNIGHT);
  setPiece(BLACK,Square(8,9),KNIGHT);
  setPiece(WHITE,Square(2,1),KNIGHT);
  setPiece(WHITE,Square(8,1),KNIGHT);
  //
  setPiece(BLACK,Square(3,9),SILVER);
  setPiece(BLACK,Square(7,9),SILVER);
  setPiece(WHITE,Square(3,1),SILVER);
  setPiece(WHITE,Square(7,1),SILVER);
  //
  setPiece(BLACK,Square(4,9),GOLD);
  setPiece(BLACK,Square(6,9),GOLD);
  setPiece(WHITE,Square(4,1),GOLD);
  setPiece(WHITE,Square(6,1),GOLD);
  //
  setPiece(BLACK,Square(5,9),KING);
  setPiece(WHITE,Square(5,1),KING);
  //
  setPiece(BLACK,Square(8,8),BISHOP);
  setPiece(WHITE,Square(2,2),BISHOP);
  //
  setPiece(BLACK,Square(2,8),ROOK);
  setPiece(WHITE,Square(8,2),ROOK);

  initPawnMask();
}
  

osl::SimpleState::~SimpleState() {}

void osl::SimpleState::setPiece(Player player,Square pos,Ptype ptype) {
  int num;
  for (num=0;num<40;num++) {
    if (!used_mask.test(num) && Piece_Table.getPtypeOf(num)==unpromote(ptype)
	&& (ptype!=KING || 
	    num==PtypeTraits<KING>::indexMin+playerToIndex(player))) {
      used_mask.set(num);
	
      Piece p(player,ptype,num,pos);
      setPieceOf(num,p);
      if (pos.isPieceStand())
	stand_mask[player].set(num);
      else{
	setBoard(pos,p);
	if (ptype==PAWN)
	  pawnMask[player].set(pos);
      }
      return;
    }
  }
  std::cerr << "osl::SimpleState::setPiece! maybe too many pieces " 
	    << ptype << " " << pos << " " << player << "\n";
  abort();
}

void osl::SimpleState::setPieceAll(Player player) {
  for (int num=0;num<40;num++) {
    if (!used_mask.test(num)) {
      used_mask.set(num);
      stand_mask[player].set(num);
      Player pplayer = player;
      /* 片玉しかない問題のため */
      if (num==PtypeTraits<KING>::indexMin+playerToIndex(alt(player)))
      {
	pplayer=alt(player);
      }
      Piece p(pplayer,Piece_Table.getPtypeOf(num),num,Square::STAND());
      setPieceOf(num,p);
    }
  }
}
  
// check
bool osl::SimpleState::isConsistent(bool show_error) const
{
  // board上の要素のconsistency
  for (int y=1;y<=9;y++)
  {
    for (int x=9;x>=1;x--)
    {
      const Square pos(x,y);
      const Piece p0=pieceAt(pos);
      if (p0.isPiece())
      {
	if (p0.square()!=pos)
	{
	  if (show_error) {
	    std::cerr << p0 << " must be put at " << pos << std::endl;
	  }
	  return false;
	}
	int num=p0.number();
	if (! PieceTable::validNumber(num) || !used_mask.test(num)) {
	  if (show_error) std::cerr << "NotUsed, num=" << num << std::endl;
	  return false;
	}
	Piece p1=pieceOf(num);
	if (p0!=p1) {
	  if (show_error) std::cerr << "board[" << pos << "]!=" 
				    << "piece[" << num << "]" << std::endl;
	  return false;
	}
      }
    }
  }
  // piecesのconsistency
  for (int num0=0; num0<Piece::SIZE; num0++)
  {
    if(!usedMask().test(num0)) continue;
    if (isOnBoard(num0))
    {
      Piece p0=pieceOf(num0);
      Ptype ptype=p0.ptype();
      if (unpromote(ptype)!=Piece_Table.getPtypeOf(num0)) {
	if (show_error) std::cerr << "ptype of piece[" << num0 << "]=" 
				  << ptype << std::endl;
	return false;
      }
      if (!p0.isOnBoard()) {
	if (show_error) std::cerr << "mochigoma[" << num0 << "]=true" << std::endl;
	return false;
      }
      Square pos=p0.square();
      if (!pos.isOnBoard()) {
	if (show_error) std::cerr << "position " << pos << " is not onboard" << std::endl;
	return false;
      }
      Piece p1=pieceAt(pos);
      int num1=p1.number();
      if (num0 !=num1) {
	if (show_error) std::cerr << "pieces[" << num0 << "]=" << p0 << ",board[" << pos << "] is " << p1 << std::endl;
	return false;
      }
    }
    else
    {
      Piece p0=pieceOf(num0);
      Ptype ptype=p0.ptype();
#ifdef ALLOW_KING_ABSENCE
      if (p0.isEmpty() && Piece_Table.getPtypeOf(num0) == KING)
	continue;
#endif
      if (p0.number()!=num0) {
	if (show_error) 
	  std::cerr << "pieces[" << num0 << "] (" 
		    << Piece_Table.getPtypeOf(num0)  <<  ") ="
		    << p0 << std::endl;
	return false;
	  
      }
      if (ptype!=Piece_Table.getPtypeOf(num0)) {
	if (show_error) std::cerr << "ptype of piece[" << num0 << "]=" 
				  << ptype << std::endl;
	return false;
      }
      if (! p0.square().isPieceStand()) {
	if (show_error) std::cerr << p0 << " must be offboard" << std::endl;
	return false;
      }
    }
  }
  // mask
  for (Ptype ptype: PieceStand::order) {
    if (countPiecesOnStand(BLACK, ptype) 
	!= countPiecesOnStandBit(BLACK, ptype)) {
      if (show_error) std::cerr << "count stand BLACK " << ptype << " inconsistent\n"
				<< *this << countPiecesOnStand(BLACK, ptype)
				<< " " << countPiecesOnStandBit(BLACK, ptype) << std::endl;
      return false;
    }
    if (countPiecesOnStand(WHITE, ptype)
	!= countPiecesOnStandBit(WHITE, ptype)) {
      if (show_error) std::cerr << "count stand WHITE " << ptype << " inconsistent\n" 
				<< *this << countPiecesOnStand(WHITE, ptype)
				<< " " << countPiecesOnStandBit(WHITE, ptype) << std::endl;
      return false;
    }
  }
  // pawnMask;
  {
    CArray<BitXmask,2> pawnMask1;
    pawnMask1[0].clearAll();
    pawnMask1[1].clearAll();
    for (int num=PtypeTraits<PAWN>::indexMin;
	 num<PtypeTraits<PAWN>::indexLimit;num++){
      if (isOnBoard(num)){
	Piece p=pieceOf(num);
	if (!p.isPromotedNotKingGold()){
	  pawnMask1[playerToIndex(p.owner())].set(p.square());
	}
      }
    }
    if ((pawnMask[0]!=pawnMask1[0])
	|| (pawnMask[1]!=pawnMask1[1]))
    {
      if (show_error) 
	std::cerr << "pawnMask "
		  << pawnMask[0] << "!=" << pawnMask1[0] 
		  << " || " <<  pawnMask[1] << "!=" << pawnMask1[1]
		  << std::endl;
      return false;
    }
  }
  // illegal position for piece
  for (int i=0; i<nthLimit<PAWN>(); ++i) {
    const Piece pawn = nth<PAWN>(i);
    if (! pawn.isPromoted() && pawn.isOnBoard()
	&& pawn.square().squareForBlack(pawn.owner()).y() == 1) {
      if (show_error)
	std::cerr << "pawn " << pawn << std::endl;
      return false;
    } 
  }
  for (int i=0; i<nthLimit<LANCE>(); ++i) {
    const Piece lance = nth<LANCE>(i);
    if (! lance.isPromoted() && lance.isOnBoard()
	&& lance.square().squareForBlack(lance.owner()).y() == 1) {
      if (show_error)
	std::cerr << "lance " << lance << std::endl;
      return false;
    } 
  }
  for (int i=0; i<nthLimit<KNIGHT>(); ++i) {
    const Piece knight = nth<KNIGHT>(i);
    if (! knight.isPromoted() && knight.isOnBoard()
	&& knight.square().squareForBlack(knight.owner()).y() == 1) {
      if (show_error)
	std::cerr << "knight " << knight << std::endl;
      return false;
    } 
  }
  return true;
}

bool osl::SimpleState::isAlmostValidMove(Move move,bool show_error) const
{
  if (show_error)
  {
    const bool valid = isAlmostValidMove<true>(move);
    if (! valid)
      std::cerr << *this << " " << move << std::endl;
    return valid;
  }
  else
    return isAlmostValidMove<false>(move);
}

template <bool show_error>
bool osl::SimpleState::isAlmostValidMove(Move move) const
{
  assert(move.isValid());
  assert(turn() == move.player());
  assert(isValidMoveByRule(move, true));

  const Square from=move.from();
  if (from.isPieceStand()) // 打つ手
    return isAlmostValidDrop<show_error>(move);
  const Square to=move.to();
    
  if (! testValidityOtherThanEffect<show_error>(move))
    return false;

  const Piece from_piece = pieceAt(from);
  // その offsetの動きがptypeに関してvalidか?
  EffectContent effect=Ptype_Table.getEffect(from_piece.ptypeO(),from,to);
  if (!effect.hasUnblockableEffect())
  {
    const Offset o=effect.offset();
    if (o.zero()) {
      if (show_error) {
	std::cerr << " No such move2 : " << move << std::endl;
      }
      return false;
    }
    // 離れた動きの時に間が全部空いているか?
    for (Square p=from+o;p!=to;p+=o) {
      if (! pieceAt(p).isEmpty()) {
	if (show_error) 
	  std::cerr << " Not space to move : " << move << std::endl;
	return false;
      }
    }
  }

  assert(isValidMoveByRule(move, true));
  return true;
}

bool osl::SimpleState::isValidMoveByRule(Move move,bool show_error) 
{
  assert(move.isNormal());
  const Square from=move.from();
  const Square to=move.to();
  const Ptype ptype=move.ptype();
  const Player turn = move.player();
    
  if (from.isPieceStand()) // 打つ手
  { 
    // 動けない場所ではないか?
    if (! Ptype_Table.canDropTo(turn,ptype,to))
    {
      if (show_error) std::cerr << " can't drop to : " << move << std::endl;
      return false;
    }
  }
  else
  {
    if (isBasic(move.ptype()) && move.isPromotion()) 
    {
      if (show_error) std::cerr << " inconsistent promote " << move << std::endl;
      return false;
    }
    const PtypeO old_ptypeo = move.oldPtypeO();
    const EffectContent effect
      = Ptype_Table.getEffect(old_ptypeo, Offset32(to,from));
    // その offsetの動きがptypeに関してvalidか?
    if (!effect.hasUnblockableEffect())
    {
      const Offset o = effect.offset();
      if (o.zero()) {
	if (show_error) {
	  std::cerr << " No such move1 : " << move << std::endl;
	}
	return false;
      }
    }
    // promoteしている時にpromote可能か
    if (move.isPromotion())
    {
      if (! (canPromote(unpromote(move.ptype()))
	     && (to.canPromote(move.player()) 
		 || from.canPromote(move.player()))))
      {
	if (show_error) 
	  std::cerr << " illegal promote type or position : " << move << std::endl;
	return false;
      }
    }
    // promoteしていない時に強制promoteでないか?
    if ((! isPromoted(ptype)
	 && ! Ptype_Table.canDropTo(turn,getPtype(old_ptypeo),to)) 
	&& !move.isPromotion()) 
    {
      if (show_error) 
	std::cerr << " must promote to this position : " << move << std::endl;
      return false;
    }
  }
  return true;
}

bool osl::SimpleState::isValidMove(Move move,bool show_error) const
{
  if (turn() != move.player()) {
    if (show_error) {
      std::cerr << "invalid player move : " << move << std::endl;
      std::cerr << *this;
    }
    return false;
  }
  if (! isValidMoveByRule(move, show_error) || ! move.isValid())
    return false;
  return isAlmostValidMove(move, show_error);
}
  
#ifndef MINIMAL
bool osl::SimpleState::dump() const
{
  return static_cast<bool>(std::cerr << *this << "\n");
}
#endif
  
/**
 * from で表現されたPieceをnew_ownerの持駒にした局面を作る.
 */
const osl::SimpleState 
osl::SimpleState::emulateCapture(Piece from, Player new_owner) const {
  osl::SimpleState newState;
  for(int i=0;i<40;i++){
    Piece p=pieceOf(i);
    if(p==from){
      newState.setPiece(new_owner,Square::STAND(),unpromote(p.ptype()));
    }
    else{
      newState.setPiece(p.owner(),p.square(),p.ptype());
    }
  }
  newState.setTurn(turn());
  newState.initPawnMask();
  return newState;
}

/**
 * from からto に ptypeの持駒を一枚渡した局面を作る.
 */
const osl::SimpleState 
osl::SimpleState::emulateHandPiece(Player from, Player to, Ptype ptype) const {
  assert(hasPieceOnStand(from, ptype));
  assert(from==alt(to));
  osl::SimpleState newState;
  bool done=false;
  for(int i=0;i<40;i++){
    if(!usedMask().test(i)) continue;
    Piece p=pieceOf(i);
    if(!done &&
       p.owner()==from &&
       !p.isOnBoard() &&
       p.ptype()==ptype){
      newState.setPiece(to,Square::STAND(),ptype);
      done=true;
    }
    else{
      newState.setPiece(p.owner(),p.square(),p.ptype());
    }
  }
  assert(done);
  newState.setTurn(turn());
  newState.initPawnMask();
  return newState;
}

const osl::SimpleState osl::SimpleState::rotate180() const
{
  SimpleState ret;
  for (int i=0; i<40; ++i) {
    if(!usedMask().test(i)) continue;
    const Piece p = pieceOf(i);
    ret.setPiece(alt(p.owner()), p.square().rotate180Safe(), p.ptype());
  }
  ret.setTurn(alt(turn()));
  ret.initPawnMask();
  return ret;
}

const osl::SimpleState osl::SimpleState::flipHorizontal() const
{
  SimpleState ret;
  for (int i=0; i<40; ++i) {
    if(!usedMask().test(i)) continue;
    const Piece p = pieceOf(i);
    ret.setPiece(p.owner(), p.square().flipHorizontal(), p.ptype());
  }
  ret.setTurn(turn());
  ret.initPawnMask();
  return ret;
}

bool osl::operator==(const SimpleState& st1,const SimpleState& st2)
{
  assert(st1.isConsistent(false));
  assert(st2.isConsistent(false));
  if (st1.turn()!=st2.turn()) 
    return false;
  if (st1.pawnMask[0]!=st2.pawnMask[0]) return false;
  if (st1.pawnMask[1]!=st2.pawnMask[1]) return false;
  for (int y=1;y<=9;y++)
    for (int x=9;x>0;x--) {
      Piece p1=st1.pieceAt(Square(x,y));
      Piece p2=st2.pieceAt(Square(x,y));
      if (p1.ptypeO()!=p2.ptypeO()) return false;
    }
  return true;
      
}

namespace osl
{
  namespace 
  {
    void showStand(std::ostream& os, Player player, PieceStand stand)
    {
      if (! stand.any())
	return;
      
      os << "P" << csa::show(player);
      for (Ptype ptype: PieceStand::order) {
	for (unsigned int j=0; j<stand.get(ptype); ++j)
	{
	  os << "00" << csa::show(ptype);
	}
      }
      os << "\n";
    }
  } // anonymous namespace
} // namespace osl

std::ostream& osl::operator<<(std::ostream& os,const SimpleState& state)
{
  for (int y=1;y<=9;y++) {
    os << 'P' << y;  
    for (int x=9;x>0;x--) {
      os << csa::show(state.pieceOnBoard(Square(x,y)));
    }
    os << std::endl;
  }
  // 持ち駒の表示
  const PieceStand black_stand(BLACK, state);
  const PieceStand white_stand(WHITE, state);
  showStand(os, BLACK, black_stand);
  showStand(os, WHITE, white_stand);
  
  os << state.turn() << std::endl;
  return os;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
