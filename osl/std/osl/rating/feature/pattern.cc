/* pattern.cc
 */
#include "osl/rating/feature/pattern.h"
#include <sstream>

const std::string osl::rating::Pattern::name(Direction d, Direction d2, Ptype self, Ptype target, bool same) 
{
  std::ostringstream os;
  os << d;
  if (d2 != Pattern::INVALID)
      os << d2;
  os << "-" << Ptype_Table.getCsaName(self) << "-" 
     << Ptype_Table.getCsaName(target) << (same ? "=" : "!");
  return os.str();
}

const std::string osl::rating::LongTarget::name() const
{
  std::ostringstream os;
  os << Ptype_Table.getCsaName(target)
     << (same ? "=" : "!") << (promotable ? "p" : "-");
  return os.str() + CountEffect2::name(attack, defense);
}
const std::string osl::rating::LongTarget2::name() const
{
  std::ostringstream os;
  os << Ptype_Table.getCsaName(target)
     << (same ? "=" : "!");
  return os.str();
}

osl::rating::
PatternLong::PatternLong(Direction d, Ptype s, LongTarget t) 
  : Feature(name(d, s)+"-"+t.name()), direction(d), self(s), target(t)
{
  assert(unpromote(s) == LANCE || unpromote(s) == BISHOP || unpromote(s) == ROOK);
}

const osl::rating::PieceSquare osl::rating::
PatternLong::nextPieceOrEnd(const SimpleState& state, Square start, Offset diff) 
{
  Square cur = start;
  assert(cur.isOnBoard());
  assert(! diff.zero());
  cur += diff;
  while (state.pieceAt(cur) == Piece::EMPTY())
    cur += diff;
  const Piece p = state.pieceAt(cur);
  if (! p.isEdge())
    return std::make_pair(p, cur);
  cur -= diff;
  assert(cur.isOnBoard());
  if (cur == start)
    return std::make_pair(p, cur); // EDGE
  return std::make_pair(state.pieceOnBoard(cur), cur); // EMPTY
}

const osl::rating::PieceSquare osl::rating::
PatternLong::nextPieceOrEnd(const SimpleState& state, Square start, Player player, Direction direction) 
{
  const Offset diff = Board_Table.getOffset(player, direction);
  return nextPieceOrEnd(state, start, diff);
}

const osl::rating::PieceSquare osl::rating::
PatternLong::find(const NumEffectState& state, Move move, Direction direction) 
{
  PieceSquare p = nextPieceOrEnd(state, move.to(), move.player(), direction);
  if (p.second == move.from())
    p = nextPieceOrEnd(state, p.second, move.player(), direction);
  return p;
}

const std::string osl::rating::PatternLong::name(Direction d, Ptype self)
{
  std::ostringstream os;
  os << d << "-" << Ptype_Table.getCsaName(self);
  return os.str();
}


osl::rating::
PatternLong2::PatternLong2(Direction d, Ptype s, LongTarget2 t2) 
  : Feature(name(d, s)+"--"+t2.name()), direction(d), self(s), target2(t2)
{
  assert(unpromote(s) == LANCE || unpromote(s) == BISHOP || unpromote(s) == ROOK);
}

const std::string osl::rating::
PatternLong2::name(Direction d, Ptype self)
{
  std::ostringstream os;
  os << d << "-" << Ptype_Table.getCsaName(self);
  return os.str();
}

osl::rating::
PatternBlock::PatternBlock(Ptype s, Ptype a, LongTarget t) 
  : Feature(std::string(Ptype_Table.getCsaName(s))/*+"-"+Ptype_Table.getCsaName(a)+">"*/+t.name()), 
    self(s), attack(a), target(t)
{
}

const osl::rating::PieceSquare osl::rating::
PatternBlock::find(const NumEffectState& state, Move move, Ptype ap) 
{
  Piece attack;
  if (ap == LANCE) {
    attack = state.findAttackAt<LANCE>(alt(state.turn()), move.to());
    if (attack.ptype() == LANCE) 
      return PatternLong::nextPieceOrEnd
	(state, move.to(), 
	 Board_Table.getShortOffset(Offset32(move.to(), attack.square())));
  } else if (ap == BISHOP) {
    attack = state.findAttackAt<BISHOP>(alt(state.turn()), move.to());
    if (attack.isPiece()) 
      return PatternLong::nextPieceOrEnd
	(state, move.to(), 
	 Board_Table.getShortOffset(Offset32(move.to(), attack.square())));
  } else if (ap == ROOK) {
    attack = state.findAttackAt<ROOK>(alt(state.turn()), move.to());
    if (attack.isPiece()) 
      return PatternLong::nextPieceOrEnd
	(state, move.to(), 
	 Board_Table.getShortOffset(Offset32(move.to(), attack.square())));
  }
  return std::make_pair(Piece::EDGE(), Square::STAND());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
