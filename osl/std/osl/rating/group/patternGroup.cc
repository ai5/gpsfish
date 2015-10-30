/* patternGroup.cc
 */
#include "osl/rating/group/patternGroup.h"
#include <sstream>

osl::rating::PatternGroup::PatternGroup(Direction d, Direction d2) 
  : Group(name(d, d2)), direction(d), direction2(d2)
{
  for (int attack=0; attack<3; ++attack) {
    for (int defense=0; defense<3; ++defense) {
      for (int s=PTYPE_PIECE_MIN; s<= PTYPE_MAX; ++s) {
	for (int t=PTYPE_PIECE_MIN; t<= PTYPE_MAX; ++t) {
	  push_back(new Pattern(d, d2, static_cast<Ptype>(s), static_cast<Ptype>(t), true, attack, defense));
	  push_back(new Pattern(d, d2, static_cast<Ptype>(s), static_cast<Ptype>(t), false, attack, defense));
	}
	push_back(new Pattern(d, d2, static_cast<Ptype>(s), PTYPE_EMPTY, true, attack, defense));
	push_back(new Pattern(d, d2, static_cast<Ptype>(s), PTYPE_EDGE, true, attack, defense)); // redundant
      }
    }
  }
  target_table.fill(0);
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      const Square src(x,y);
      const Square target_b = Pattern::nextSquare(BLACK, src, direction, direction2);
      const Square target_w = Pattern::nextSquare(WHITE, src, direction, direction2);
      target_table[playerToIndex(BLACK)][src.index()] = target_b.uintValue();
      target_table[playerToIndex(WHITE)][src.index()] = target_w.uintValue();
    }
  }
}

int osl::rating::PatternGroup::findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
{
  const Ptype self = move.ptype();
  const Square position
    = Square::makeDirect(target_table[playerToIndex(move.player())][move.to().index()]);
  assert(position == Pattern::nextSquare(move, direction, direction2));
  const Piece p = (position == move.from()) ? Piece::EMPTY() : state.pieceAt(position);
  const Ptype target = p.ptype();
  if (env.pattern_cache[position.index()] < 0)
    env.pattern_cache[position.index()] = CountEffect2::index(state, position, env) 
      * (PTYPE_MAX+1-PTYPE_PIECE_MIN) * ((PTYPE_MAX+1 - PTYPE_PIECE_MIN)*2 +2);
  const int base = env.pattern_cache[position.index()];

  int index = base + (self - PTYPE_PIECE_MIN)*((PTYPE_MAX+1 - PTYPE_PIECE_MIN)*2 +2);
  if (!isPiece(target)) {
    index += (PTYPE_MAX+1-PTYPE_PIECE_MIN)*2 + (target == PTYPE_EMPTY ? 0 : 1);
  }
  else {
    index += (target - PTYPE_PIECE_MIN)*2 + (p.owner() != move.player());
  }
  return index;
}

std::string osl::rating::PatternGroup::name(Direction direction, Direction direction2) 
{
  std::ostringstream ss;
  ss << "Pattern" << direction;
  if (direction2 != Pattern::INVALID)
      ss << direction2;
  return ss.str();
}

const osl::CArray<osl::Direction,4> osl::rating::PatternLongGroup::rook_direction4 = {{ U, L, D, R }};
const osl::CArray<osl::Direction,4> osl::rating::PatternLongGroup::bishop_direction4 = {{ UL, DL, DR, UR }};

std::string osl::rating::PatternLongGroup::name(int direction_id)
{
  std::ostringstream ss;
  ss << "PatLong" << direction_id;
  return ss.str();
}

osl::rating::PatternLongGroup::PatternLongGroup(int d) 
  : Group(name(d)), direction_id(d)
{
  const CArray<Ptype,5> self_list = {{ ROOK, PROOK, BISHOP, PBISHOP, LANCE }};
  for (int s=0; s<((d == 0) ? 5 : 4); ++s) {
    const Ptype self = self_list[s];
    const Direction direction = makeDirection(self);
    for (int attack=0; attack<3; ++attack) {
      for (int defense=0; defense<3; ++defense) {
	for (int t=PTYPE_PIECE_MIN; t<= PTYPE_MAX; ++t) {
	  push_back(new PatternLong(direction, self, LongTarget(static_cast<Ptype>(t), true,  true, attack, defense)));
	  push_back(new PatternLong(direction, self, LongTarget(static_cast<Ptype>(t), true,  false, attack, defense)));
	  push_back(new PatternLong(direction, self, LongTarget(static_cast<Ptype>(t), false, true, attack, defense)));
	  push_back(new PatternLong(direction, self, LongTarget(static_cast<Ptype>(t), false, false, attack, defense)));
	}
	push_back(new PatternLong(direction, self, LongTarget(PTYPE_EMPTY, true,  true, attack, defense)));
	push_back(new PatternLong(direction, self, LongTarget(PTYPE_EMPTY, false,  true, attack, defense)));
      }
    }
    push_back(new PatternLong(direction, self, LongTarget(PTYPE_EDGE, true, true, 0, 0)));
  }
}

int osl::rating::PatternLongGroup::findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
{  
  const size_t unit = ((PTYPE_MAX+1-PTYPE_PIECE_MIN)*4+2)*9+1;
  const Ptype self = move.ptype();
  int base = 0;
  switch (self) {
  case ROOK:
    break;
  case PROOK:
    base += unit; break;
  case BISHOP:
    base += unit*2; break;
  case PBISHOP:
    base += unit*3; break;
  case LANCE:
    if (direction_id != 0)
      return -1;
    base += unit*4; break;
  default:
    return -1;
  }
  const Direction direction = makeDirection(self);
  const PieceSquare pp = PatternLong::find(state, move, direction);

  int index = base;
  if (pp.first.isEdge()) {
    index += unit - 1;
  } else {
    index += ((PTYPE_MAX+1-PTYPE_PIECE_MIN)*4+2)*CountEffect2::index(state, pp.second, env);
    if (pp.first.isEmpty()) {
      index += (PTYPE_MAX+1-PTYPE_PIECE_MIN)*4;
      index += ! LongTarget::isPromotable(move, pp.second);
    }
    else {
      assert(pp.first.isPiece());
      index += (pp.first.ptype()-PTYPE_PIECE_MIN)*4;
      index += (! LongTarget::isPromotable(move, pp.second))*2;
      index += (pp.first.owner() != move.player());
    }
  }
  return index;
}

std::string osl::rating::PatternLongGroup2::name(int direction_id)
{
  std::ostringstream ss;
  ss << "PatLong2" << direction_id;
  return ss.str();
}

osl::rating::PatternLongGroup2::PatternLongGroup2(int d) 
  : Group(name(d)), direction_id(d)
{
  const CArray<Ptype,5> self_list = {{ ROOK, PROOK, BISHOP, PBISHOP, LANCE }};
  for (int s=0; s<((d == 0) ? 5 : 4); ++s) {
    const Ptype self = self_list[s];
    const Direction direction = makeDirection(self);
    for (int t=PTYPE_PIECE_MIN; t<= PTYPE_MAX; ++t) {
      push_back(new PatternLong2(direction, self, LongTarget2(static_cast<Ptype>(t), true)));
      push_back(new PatternLong2(direction, self, LongTarget2(static_cast<Ptype>(t), false)));
    }
    push_back(new PatternLong2(direction, self, LongTarget2(PTYPE_EDGE, true)));
  }
}

int osl::rating::PatternLongGroup2::findMatch(const NumEffectState& state, Move move, const RatingEnv& ) const
{  
  const size_t unit = (PTYPE_MAX+1-PTYPE_PIECE_MIN)*2+1;
  const Ptype self = move.ptype();
  int base = 0;
  switch (self) {
  case ROOK:
    break;
  case PROOK:
    base += unit; break;
  case BISHOP:
    base += unit*2; break;
  case PBISHOP:
    base += unit*3; break;
  case LANCE:
    if (direction_id != 0)
      return -1;
    base += unit*4; break;
  default:
    return -1;
  }
  const Direction direction = makeDirection(self);
  const Piece p = PatternLong2::find(state, move, direction);

  int index = base;
  if (! p.isPiece()) {
    index += unit - 1;
  } else {
    assert(p.isPiece());
    index += (p.ptype()-PTYPE_PIECE_MIN)*2;
    index += (p.owner() != move.player());
  }
  return index;
}

osl::rating::
PatternBlockGroup::PatternBlockGroup(Ptype a) 
  : Group(std::string("PatternBlock")+Ptype_Table.getCsaName(a)), attacker(a)
{
  assert(a == LANCE || a == ROOK || a == BISHOP);
  for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s) {
    const Ptype self = static_cast<Ptype>(s);
    for (int attack=0; attack<3; ++attack) {
      for (int defense=0; defense<3; ++defense) {
	for (int t=PTYPE_PIECE_MIN; t<= PTYPE_MAX; ++t) {
	  push_back(new PatternBlock(self, a, LongTarget(static_cast<Ptype>(t), true,  true, attack, defense)));
	  push_back(new PatternBlock(self, a, LongTarget(static_cast<Ptype>(t), true,  false, attack, defense)));
	  push_back(new PatternBlock(self, a, LongTarget(static_cast<Ptype>(t), false, true, attack, defense)));
	  push_back(new PatternBlock(self, a, LongTarget(static_cast<Ptype>(t), false, false, attack, defense)));
	}
	push_back(new PatternBlock(self, a, LongTarget(PTYPE_EMPTY, true,  true, attack, defense)));
	push_back(new PatternBlock(self, a, LongTarget(PTYPE_EMPTY, false,  true, attack, defense)));
      }
    }
  }
}

int osl::rating::
PatternBlockGroup::findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
{  
  const size_t unit = ((PTYPE_MAX+1-PTYPE_PIECE_MIN)*4+2)*9;
  const PieceSquare pp = PatternBlock::find(state, move, attacker);
  if (pp.first.isEdge())
    return -1;

  int index = (move.ptype() - PTYPE_PIECE_MIN)*unit;
  index += ((PTYPE_MAX+1-PTYPE_PIECE_MIN)*4+2)*CountEffect2::index(state, pp.second, env);
  if (pp.first.isEmpty()) {
    index += (PTYPE_MAX+1-PTYPE_PIECE_MIN)*4;
    index += ! pp.second.canPromote(alt(state.turn()));
  }
  else {
    assert(pp.first.isPiece());
    index += (pp.first.ptype()-PTYPE_PIECE_MIN)*4;
    index += (! pp.second.canPromote(pp.first.isPiece() ? alt(pp.first.owner()) : alt(move.player())))*2;
    index += (pp.first.owner() != move.player());
  }
  return index;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
