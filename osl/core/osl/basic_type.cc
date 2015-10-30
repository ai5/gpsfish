#include "osl/basic_type.h"
#include "osl/bits/boardTable.h"
#include "osl/bits/ptypeTable.h"
#include "osl/simpleState.h"
#include "osl/bits/squareCompressor.h"
// #include "move-phash.c"
#include <iostream>

bool osl::isValid(Player player)
{
  return player==BLACK || player==WHITE; 
}

std::ostream& osl::operator<<(std::ostream& os,Player player)
{
  if(player==BLACK)
    return os << "+";
  else
    return os << "-";
}



bool osl::isValid(Ptype ptype)
{
  return static_cast<int>(ptype)>=PTYPE_MIN 
    && static_cast<int>(ptype)<=PTYPE_MAX;
}

bool osl::isValidPtypeO(int ptypeO)
{
  return (ptypeO >= PTYPEO_MIN) && (ptypeO <= PTYPEO_MAX);
}

std::istream& osl::operator>>(std::istream& is, osl::Ptype& ptype)
{
  std::string s;
  is >> s;
  if (s == "PTYPE_EMPTY")
    ptype = PTYPE_EMPTY;
  else if (s == "PTYPE_EDGE")
    ptype = PTYPE_EDGE;
  else if (s == "PPAWN")
    ptype = PPAWN;
  else if (s == "PLANCE")
    ptype = PLANCE;
  else if (s == "PKNIGHT")
    ptype = PKNIGHT;
  else if (s == "PSILVER")
    ptype = PSILVER;
  else if (s == "PBISHOP")
    ptype = PBISHOP;
  else if (s == "PROOK")
    ptype = PROOK;
  else if (s == "KING")
    ptype = KING;
  else if (s == "GOLD")
    ptype = GOLD;
  else if (s == "PAWN")
    ptype = PAWN;
  else if (s == "LANCE")
    ptype = LANCE;
  else if (s == "KNIGHT")
    ptype = KNIGHT;
  else if (s == "SILVER")
    ptype = SILVER;
  else if (s == "BISHOP")
    ptype = BISHOP;
  else if (s == "ROOK")
    ptype = ROOK;
  else{
    std::cerr << "Incorrect input : " << s << std::endl;
    ptype = PTYPE_EMPTY;
  }
  return is;
}

std::ostream& osl::operator<<(std::ostream& os,const osl::Ptype ptype)
{
    return os << Ptype_Table.getName(ptype);
}
  
std::ostream& osl::operator<<(std::ostream& os,const osl::PtypeO ptypeO)
{
  if (isPiece(ptypeO))
    return os << "PtypeO(" << getOwner(ptypeO) << "," 
	      << getPtype(ptypeO) << ")";
  return os << "PtypeO(" << (int)ptypeO << "," << getPtype(ptypeO) << ")";
}



bool osl::isValid(Direction d){
  return DIRECTION_MIN<=d && d<=DIRECTION_MAX;
}

std::ostream& osl::operator<<(std::ostream& os,const Direction d){
  static const char* names[]={
    "UL","U","UR","L",
    "R","DL","D","DR",
    "UUL","UUR","LONG_UL",
    "LONG_U","LONG_UR","LONG_L",
    "LONG_R","LONG_DL","LONG_D","LONG_DR"
  };
  return os << names[static_cast<int>(d)];
}

#define OFFSET_INDEX(dx,dy) ((dx*BOARD_HEIGHT + dy) - OFFSET_MIN)

osl::Offset::Offset(Player player, Direction direction)
{
  *this = Board_Table.getOffset(player, direction);
}

/** 
 * Offsetから一般に dxは求まらないので,
 * ここでの入力は12近傍のみとする
 */
int osl::Offset::dx() const
{
  switch (index())
  {
  case OFFSET_INDEX(-1,-2): return -1;
  case OFFSET_INDEX(1,-2): return 1;
  case OFFSET_INDEX(-1,-1): return -1;
  case OFFSET_INDEX(0,-1): return 0;
  case OFFSET_INDEX(1,-1): return 1;
  case OFFSET_INDEX(-1,0): return -1;
  case OFFSET_INDEX(1,0): return 1;
  case OFFSET_INDEX(-1,1): return -1;
  case OFFSET_INDEX(0,1): return 0;
  case OFFSET_INDEX(1,1): return 1;
  case OFFSET_INDEX(-1,2): return -1;
  case OFFSET_INDEX(1,2): return 1;
  default: 
    std::cerr << index() << " " << ZERO().index() << "\n";
    assert(0);
  }
  return 0;
}

/** 
 * Offsetから一般に dyは求まらないので,
 * ここでの入力は12近傍のみとする
 */
int osl::Offset::dy() const
{
  switch (index())
  {
  case OFFSET_INDEX(-1,-2): return -2;
  case OFFSET_INDEX(1,-2): return -2;
  case OFFSET_INDEX(-1,-1): return -1;
  case OFFSET_INDEX(0,-1): return -1;
  case OFFSET_INDEX(1,-1): return -1;
  case OFFSET_INDEX(-1,0): return 0;
  case OFFSET_INDEX(1,0): return 0;
  case OFFSET_INDEX(-1,1): return 1;
  case OFFSET_INDEX(0,1): return 1;
  case OFFSET_INDEX(1,1): return 1;
  case OFFSET_INDEX(-1,2): return 2;
  case OFFSET_INDEX(1,2): return 2;
  default: assert(0);
  }
  return 0;
}

#ifndef MINIMAL
std::ostream& osl::operator<<(std::ostream& os, Offset offset)
{
  return os << "offset(" << offset.intValue() << ')';
}
#endif



static_assert(sizeof(osl::Square) == 4, "square size");

bool osl::Square::isOnBoardSlow() const 
{
    return (1<=x() && x() <=9 
	    && 1<=y() && y() <=9);
}

bool osl::Square::isValid() const
{
  return isPieceStand() || isOnBoard();
}


const osl::Square osl::
Square::neighbor(Player P, Direction D) const
{
  return Board_Table.nextSquare(P, *this, D);
}

const osl::Square osl::
Square::back(Player P, Direction D) const
{
  return Board_Table.nextSquare(alt(P), *this, D);
}

bool osl::Square::isNeighboring8(Square to) const {
  return (*this != to)
    && (to == *this+Board_Table.getShortOffsetNotKnight(Offset32(to,*this)));
}

std::ostream& osl::operator<<(std::ostream& os, Square square)
{
  if (square.isPieceStand())
    return os << "OFF";
  return os << "Square(" << square.x() << square.y() << ")";
}

static_assert(sizeof(osl::Piece) == 4, "piece size");

std::ostream& osl::operator<<(std::ostream& os,const Piece piece)
{
  if (piece.isPiece())
    os << "Piece(" << piece.owner() << "," << piece.ptype() 
	       << ",num=" << piece.number() 
	       << "," << piece.square() << ')';
  else if (piece == Piece::EMPTY())
    os << "PIECE_EMPTY";
  else if (piece == Piece::EDGE())
    os << "PIECE_EDGE";
  else
    os << "unkown piece?!";
  return os;
}

const osl::Piece osl::Piece::makeKing(Player owner, Square position)
{
  const int number = ((owner == BLACK)
		      ? (int)KingTraits<BLACK>::index
		      : (int)KingTraits<WHITE>::index);
  return Piece(owner, KING, number, position);
}



namespace osl
{
  static_assert(sizeof(Move) == 4, "move size");
} //namespace osl

bool osl::Move::isValid() const
{
  if (! isNormal())
    return false;
  const Square from = this->from();
  if (! from.isValid())
    return false;
  const Square to = this->to();
  if (! to.isOnBoard())
    return false;
  return osl::isValid(ptype())
    && osl::isValid(capturePtype())
    && capturePtype()!=KING
    && osl::isValid(player());
}

const osl::Move osl::Move::rotate180() const
{
  if (isPass())
    return Move::PASS(alt(player()));
  if (! isNormal())
    return *this;
  return Move(from().rotate180Safe(), to().rotate180(), ptype(),
	      capturePtype(), isPromotion(), alt(player()));
}

std::ostream& osl::operator<<(std::ostream& os,const Move move)
{
  if (move == Move::DeclareWin())
    return os << "MOVE_DECLARE_WIN";
  if (move.isInvalid())
    return os << "MOVE_INVALID";
  if (move.isPass())
    return os << "MOVE_PASS";
  const Player turn = move.player();
  if (move.isValid())
  {
    if (move.from().isPieceStand()) 
    {
      os << "Drop(" << turn << "," << move.ptype() << "," << move.to() << ")";
    }
    else
    {
      const Ptype capture_ptype=move.capturePtype();
      os << "Move(" << turn << "," << move.ptype() << "," 
	 << move.from() << "->" << move.to() ;
      if (move.promoteMask())
	os << ",promote";
      if (capture_ptype != PTYPE_EMPTY)
	os << ",capture=" << capture_ptype;
      os << ")";
    }
  }
  else
  {
    os << "InvalidMove " << move.from() << " " << move.to() 
       << " " << move.ptypeO() << " " << move.oldPtypeO()
       << " " << move.promoteMask()
       << " " << move.capturePtype() << "\n";
  }
  return os;
}

unsigned int osl::Move::hash() const
{
  assert(capturePtype() == PTYPE_EMPTY);
  // return move_phash(intValue());
  return intValue();
}

const osl::Move osl::Move::
fromMove16(Move16 move16, const SimpleState& state)
{
  if (move16==MOVE16_NONE)
    return Move();
  Player turn=state.turn();
  Square to=SquareCompressor::melt((move16>>8)&0x7f);
  if((move16&0x80)!=0){
    Ptype ptype=(Ptype)(move16-0x80);
    return Move(to,ptype,turn);
  }
  Square from=SquareCompressor::melt(move16&0x7f);
  Ptype ptype=state[from].ptype();
  Ptype capture_ptype=state[to].ptype();
  bool is_promote=(move16&0x8000)!=0;
  if(is_promote)
    return Move(from,to,::osl::promote(ptype),capture_ptype,true,turn);
  else
    return Move(from,to,ptype,capture_ptype,false,turn);
}
osl::Move16 osl::Move::toMove16() const
{
  if (isInvalid())
    return MOVE16_NONE;
  if (isDrop())
    return Move16(0x80+(uint16_t)ptype()+((SquareCompressor::compress(to()))<<8));
  if (isPromotion())
    return Move16(SquareCompressor::compress(from())+(SquareCompressor::compress(to())<<8)+0x8000);
  return Move16(SquareCompressor::compress(from())+(SquareCompressor::compress(to())<<8));
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
