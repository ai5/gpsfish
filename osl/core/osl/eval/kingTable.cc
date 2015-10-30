#include "osl/eval/kingTable.h"
using osl::MultiInt;

osl::CArray2d<MultiInt, osl::PTYPE_SIZE, 17 * 9> osl::eval::ml::KingPieceRelative::attack_table;
osl::CArray2d<MultiInt, osl::PTYPE_SIZE, 17 * 9> osl::eval::ml::KingPieceRelative::defense_table;

void osl::eval::ml::
KingPieceRelative::setUp(const Weights &weights, int stage)
{
  for (int i = PTYPE_PIECE_MIN; i <= PTYPE_MAX; ++i)
  {
    for (int y = 0; y <= 16; ++y)
    {
      for (int x = 0; x <= 8; ++x)
      {
	const int distance = x * 17 + y;
	attack_table[i][distance][stage] =
	  weights.value((i - PTYPE_PIECE_MIN) * 17 * 9 + distance);
	defense_table[i][distance][stage] =
	  weights.value((i - PTYPE_PIECE_MIN) * 17 * 9 + distance + TABLE_DIM);
      }
    }
  }
}

MultiInt osl::eval::ml::
KingPieceRelative::eval(const NumEffectState &state)
{
  MultiInt value;
  const Square b_king = state.kingSquare(BLACK);
  const Square w_king = state.kingSquare(WHITE);
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (!piece.isOnBoard())
      continue;
    const Ptype ptype = piece.ptype();
    const Square position = piece.square();
    if (piece.owner() == BLACK)
    {
      const int attack_index = index(state, BLACK,position, w_king);
      const int defense_index = index(state, BLACK,position, b_king);
      value += attack_table[ptype][attack_index];
      value += defense_table[ptype][defense_index];
    }
    else
    {
      const int attack_index = index(state, WHITE,position, b_king);
      const int defense_index = index(state, WHITE,position, w_king);
      value -= attack_table[ptype][attack_index];
      value -= defense_table[ptype][defense_index];
    }
  }
  return value;
}

template<osl::Player P>
MultiInt osl::eval::ml::
KingPieceRelative::evalWithUpdate(const NumEffectState &state,
				  Move moved, MultiInt const& last_value)
{
  if (moved.isPass())
    return last_value;

  if (moved.ptype() == osl::KING)
  {
    return eval(state);
  }

  MultiInt value(last_value);
  if (!moved.isDrop())
  {
    const PtypeO ptypeO = moved.oldPtypeO();
    const int attack_index = index(state, ptypeO, moved.from(), false);
    const int defense_index = index(state,ptypeO, moved.from(), true);
    if (P == BLACK)
    {
      value -= attack_table[moved.oldPtype()][attack_index];
      value -= defense_table[moved.oldPtype()][defense_index];
    }
    else
    {
      value += attack_table[moved.oldPtype()][attack_index];
      value += defense_table[moved.oldPtype()][defense_index];
    }
  }
  {
    const int attack_index = index(state, moved.ptypeO(), moved.to(), false);
    const int defense_index = index(state, moved.ptypeO(), moved.to(), true);
    if (P == BLACK)
    {
      value += attack_table[moved.ptype()][attack_index];
      value += defense_table[moved.ptype()][defense_index];
    }
    else
    {
      value -= attack_table[moved.ptype()][attack_index];
      value -= defense_table[moved.ptype()][defense_index];
    }
  }
  const Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const PtypeO ptypeO = moved.capturePtypeO();
    const int attack_index = index(state, ptypeO, moved.to(), false);
    const int defense_index = index(state,ptypeO, moved.to(), true);
    if (P == BLACK)
    {
      value += attack_table[captured][attack_index];
      value += defense_table[captured][defense_index];
    }
    else
    {
      value -= attack_table[captured][attack_index];
      value -= defense_table[captured][defense_index];
    }
  }
  return value;
}


osl::CArray<MultiInt, osl::eval::ml::KingPieceRelativeNoSupport::ONE_DIM>
osl::eval::ml::KingPieceRelativeNoSupport::table;

void osl::eval::ml::
KingPieceRelativeNoSupport::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <int Sign> inline
void osl::eval::ml::
KingPieceRelativeNoSupport::adjust(int attack, int defense, MultiInt& out)
{
  if(Sign>0)
    out += table[attack] + table[defense];
  else
    out -= table[attack] + table[defense];
}

MultiInt osl::eval::ml::
KingPieceRelativeNoSupport::eval(const NumEffectState &state)
{
  MultiInt result;
  const CArray<Square, 2> kings = {
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    };
  PieceMask black = (~state.effectedMask(BLACK)) & state.piecesOnBoard(BLACK);
  black.reset(KingTraits<BLACK>::index);
  while (black.any())
  {
    const Piece piece = state.pieceOf(black.takeOneBit());
    const int index_attack = index(BLACK, kings[WHITE],
				   piece);
    const int index_defense = index(BLACK, kings[BLACK],
				    piece) + ONE_DIM / 2;
    adjust<1>(index_attack, index_defense, result);
  }
  PieceMask white = (~state.effectedMask(WHITE)) & state.piecesOnBoard(WHITE);
  white.reset(KingTraits<WHITE>::index);
  while (white.any())
  {
    const Piece piece = state.pieceOf(white.takeOneBit());
    const int index_attack = index(WHITE, kings[BLACK],
				   piece);
    const int index_defense = index(WHITE, kings[WHITE],
				    piece) + ONE_DIM / 2;
    adjust<-1>(index_attack, index_defense, result);
  }
  return result;
}

MultiInt osl::eval::ml::
KingPieceRelativeNoSupport::evalWithUpdate(
  const NumEffectState &state,
  Move moved,
  const CArray<PieceMask, 2> &effected_mask,
  const MultiInt &last_values)
{
  if (moved.ptype() == KING)
    return eval(state);

  //const CArray<PieceMask, 2> new_mask = {{
  const CArray<PieceMask, 2> new_mask = {{
      state.effectedMask(BLACK),
      state.effectedMask(WHITE)
    }};
  const CArray<Square, 2> kings = {{ 
      state.kingSquare<BLACK>(),
      state.kingSquare<WHITE>(),
    }};

  MultiInt result(last_values);
  const Piece p = state.pieceAt(moved.to());
  if (!moved.isDrop())
  {
    if (!effected_mask[p.owner()].test(p.number()))
    {
      const int index_attack =
	index(p.owner(), kings[alt(p.owner())],
	      moved.oldPtype(), moved.from());
      const int index_defense =
	index(p.owner(), kings[p.owner()], moved.oldPtype(),
	      moved.from()) + ONE_DIM / 2;
      if (p.owner() == BLACK)
	adjust<-1>(index_attack, index_defense, result);
      else
	adjust<1>(index_attack, index_defense, result);
    }
  }
  const Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    PieceMask captured_mask =
      effected_mask[moved.player()] & (~state.piecesOnBoard(BLACK)) &
      (~state.piecesOnBoard(WHITE));
    
    if (!effected_mask[alt(moved.player())].test(captured_mask.takeOneBit()))
    {
      const int index_attack =
	index(alt(p.owner()), kings[p.owner()],
	      captured, moved.to());
      const int index_defense =
	index(alt(p.owner()), kings[alt(p.owner())], captured,
	      moved.to()) + ONE_DIM / 2;
      if (p.owner() == BLACK)
	adjust<1>(index_attack, index_defense, result);
      else
	adjust<-1>(index_attack, index_defense, result);
    }
  }
  if (!new_mask[p.owner()].test(p.number()))
  {
    const int index_attack =
      index(p.owner(), kings[alt(p.owner())],
	    moved.ptype(), moved.to());
    const int index_defense =
      index(p.owner(), kings[p.owner()], moved.ptype(),
	    moved.to()) + ONE_DIM / 2;
    if (p.owner() == BLACK)
      adjust<1>(index_attack, index_defense, result);
    else
      adjust<-1>(index_attack, index_defense, result);
  }
  PieceMask onboard_black = state.piecesOnBoard(BLACK);
  onboard_black.reset(KingTraits<BLACK>::index);
  // old 0, new, 1
  PieceMask black_old = (~effected_mask[0]) & new_mask[0] & onboard_black;
  black_old.reset(p.number());
  while (black_old.any())
  {
    const Piece piece = state.pieceOf(black_old.takeOneBit());
    const int index_attack =
      index(BLACK, kings[WHITE], piece);
    const int index_defense =
      index(BLACK, kings[BLACK], piece) + ONE_DIM / 2;
    adjust<-1>(index_attack, index_defense, result);
  }
  // old 1, new 0
  PieceMask black_new = effected_mask[0] & (~new_mask[0]) & onboard_black;
  black_new.reset(p.number());
  while (black_new.any())
  {
    const Piece piece = state.pieceOf(black_new.takeOneBit());
    const int index_attack =
      index(BLACK, kings[WHITE], piece);
    const int index_defense =
      index(BLACK, kings[BLACK], piece) + ONE_DIM / 2;
    adjust<1>(index_attack, index_defense, result);
  }

  // old 0, new, 1
  PieceMask onboard_white = state.piecesOnBoard(WHITE);
  onboard_white.reset(KingTraits<WHITE>::index);
  PieceMask white_old = (~effected_mask[1]) & new_mask[1] & onboard_white;
  white_old.reset(p.number());
  while (white_old.any())
  {
    const Piece piece = state.pieceOf(white_old.takeOneBit());
    const int index_attack =
      index(WHITE, kings[BLACK], piece);
    const int index_defense =
      index(WHITE, kings[WHITE], piece) + ONE_DIM / 2;
    adjust<1>(index_attack, index_defense, result);
  }
  // old 1, new 0
  PieceMask white_new = effected_mask[1] & (~new_mask[1]) & onboard_white;
  white_new.reset(p.number());
  while (white_new.any())
  {
    const Piece piece = state.pieceOf(white_new.takeOneBit());
    const int index_attack =
      index(WHITE, kings[BLACK], piece);
    const int index_defense =
      index(WHITE, kings[WHITE], piece) + ONE_DIM / 2;
    adjust<-1>(index_attack, index_defense, result);
  }

  return result;
}


osl::CArray<MultiInt, 2592> osl::eval::ml::PtypeYY::table;

void osl::eval::ml::PtypeYY::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <int Sign>
inline
void osl::eval::ml::
PtypeYY::adjust(int black, int white, MultiInt &out)
{
  if(Sign>0)
    out += table[black] - table[white];
  else
    out -= table[black] - table[white];
}

MultiInt osl::eval::ml::
PtypeYY::eval(const NumEffectState &state)
{
  MultiInt result;
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    const int black_index = index<BLACK>(p, kings[BLACK]);
    const int white_index = index<WHITE>(p, kings[WHITE]);
    adjust<1>(black_index, white_index, result);
  }

  return result;
}

MultiInt osl::eval::ml::PtypeYY::evalWithUpdate(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values)
{
  if (moved.ptype() == KING && moved.to().y() != moved.from().y())
  {
    return eval(state);
  }
  MultiInt result(last_values);
  if (!moved.isDrop())
  {
    const int index_black = index<BLACK>(moved.oldPtypeO(), moved.from(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(moved.oldPtypeO(), moved.from(),
					 state.kingSquare<WHITE>());
    adjust<-1>(index_black, index_white, result);
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const PtypeO ptypeO = newPtypeO(alt(moved.player()), captured);
    const int index_black = index<BLACK>(ptypeO, moved.to(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(ptypeO, moved.to(),
					 state.kingSquare<WHITE>());
    adjust<-1>(index_black, index_white, result);
  }

  {
    const int index_black = index<BLACK>(moved.ptypeO(), moved.to(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(moved.ptypeO(), moved.to(),
					 state.kingSquare<WHITE>());
    adjust<1>(index_black, index_white, result);
  }
  return result;
}


osl::CArray<int, osl::eval::ml::King25Effect::DIM>
osl::eval::ml::King25Effect::table;

void osl::eval::ml::
King25Effect::setUp(const Weights &weights)
{
  table.fill(0);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}

void osl::eval::ml::King25Effect::countEffectAndPieces(
  const NumEffectState &state,
  const osl::Player attack,
  int &effect,
  int &piece)
{
  const Square king = state.kingSquare(alt(attack));
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);

  PieceMask mask;
  int count = 0;
  for (int y = min_y; y <= max_y; ++y)
  {
    for (int x = min_x; x <= max_x; ++x)
    {
      const Square target(x, y);
      count += state.countEffect(attack, target);
      mask |= state.effectSetAt(target);
    }
  }
  effect = std::min(255, count);
  mask = mask & state.piecesOnBoard(attack);
  piece = std::min(16,  mask.countBit());
}

int osl::eval::ml::King25Effect::index(int effect, int piece_count)
{
  return effect + 128 * piece_count;
}

int osl::eval::ml::King25Effect::eval(
  const NumEffectState &state)
{
  int black_effect, black_piece, white_effect, white_piece;
  countEffectAndPieces(state, osl::BLACK, black_effect, black_piece);
  countEffectAndPieces(state, osl::WHITE, white_effect, white_piece);
  return table[index(black_effect, black_piece)] - table[index(white_effect, white_piece)];
}


osl::CArray<MultiInt, osl::eval::ml::King25Effect2::ONE_DIM>
osl::eval::ml::King25Effect2::table;
osl::CArray<MultiInt, osl::eval::ml::King25EffectY2::ONE_DIM>
osl::eval::ml::King25EffectY2::table;

void osl::eval::ml::King25Effect2::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25EffectY2::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}


osl::CArray<MultiInt, osl::eval::ml::King25EffectSupported::ONE_DIM>
osl::eval::ml::King25EffectSupported::table;
osl::CArray<MultiInt, osl::eval::ml::King25EffectSupportedY::ONE_DIM>
osl::eval::ml::King25EffectSupportedY::table;

void osl::eval::ml::King25EffectSupported::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25EffectSupportedY::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}


osl::CArray<int, osl::eval::ml::King25EffectBoth::DIM/2>
osl::eval::ml::King25EffectBoth::attack_table;
osl::CArray<int, osl::eval::ml::King25EffectBoth::DIM/2>
osl::eval::ml::King25EffectBoth::defense_table;

void osl::eval::ml::
King25EffectBoth::setUp(const Weights &weights)
{
  attack_table.fill(0);
  defense_table.fill(0);
  for (size_t i = 0; i < DIM/2; ++i)
  {
    attack_table[i] = weights.value(i);
    defense_table[i] = weights.value(i+DIM/2);
  }
}

template <osl::Player Attack>
void
 osl::eval::ml::King25EffectBoth::countEffectAndPiecesBoth(
  const NumEffectState &state,
  PieceMask& mask,
  PieceMask& supported_mask,
  int &attack_effect,
  int &attack_piece,
  int &defense_effect,
  int &defense_piece,
  int &attack_piece_supported,
  CArray<int, 5> &verticals,
  CArray<int, 5> &king_verticals)
{
  king_verticals.fill(0);
  const Player Defense = alt(Attack);
  const Square king = state.kingSquare(Defense);
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  int mask_all=(1<<5)-1;
  verticals.fill(mask_all);
  const int y_mask_base=(1 << (Defense == BLACK ? (min_y-king.y()) + 2 : -(max_y-king.y()) + 2));
  if(Defense==BLACK)
    mask_all ^= ((1<<(max_y-king.y()+3))-y_mask_base);
  else
    mask_all ^= ((1<<(king.y()-min_y+3))-y_mask_base);
  mask.resetAll();
  int attack_count = 0;
  int defense_count = 0;
  for (int x = min_x; x <= max_x; ++x)
  {
    int vertical_x=mask_all;
    int king_vertical_x = 0;
    if(Defense==BLACK){
      int y_mask=y_mask_base;
      for (int y = min_y; y <= max_y; ++y, y_mask+=y_mask)
      {
	const Square target(x, y);
	const int count = state.countEffect(Attack, target);
	defense_count += state.countEffect(alt(Attack), target);
	mask |= state.effectSetAt(target);
	if(count>0){
	  attack_count += count;
	  vertical_x |= y_mask;
	}
	const Piece piece = state.pieceAt(target);
	if (count == 0 &&
	    (piece.isEmpty() || piece.owner() == Attack))
	{
	  king_vertical_x |= y_mask;
	}
      }
    }
    else{
      int y_mask=y_mask_base;
      for (int y = max_y; y >= min_y; --y, y_mask+=y_mask)
      {
	const Square target(x, y);
	const int count = state.countEffect(Attack, target);
	defense_count += state.countEffect(alt(Attack), target);
	mask |= state.effectSetAt(target);
	if(count>0){
	  attack_count += count;
	  vertical_x |= y_mask;
	}
	const Piece piece = state.pieceAt(target);
	if (count == 0 &&
	    (piece.isEmpty() || piece.owner() == Attack))
	{
	  king_vertical_x |= y_mask;
	}
      }
    }
    const int x_diff = king.x() - x;
    verticals[(Defense == BLACK ? 2 - x_diff : 2 + x_diff)] = vertical_x;
    king_verticals[(Defense == BLACK ? 2 - x_diff : 2 + x_diff)] =
      king_vertical_x;
  }
  attack_effect = std::min(127, attack_count);
  defense_effect = std::min(127, defense_count);
  PieceMask attack_mask = mask & state.piecesOnBoard(Attack);
  attack_piece = std::min(16,  attack_mask.countBit());
  PieceMask defense_mask = mask & state.piecesOnBoard(alt(Attack));
  defense_piece = std::min(16,  defense_mask.countBit());
  supported_mask = attack_mask & state.effectedMask(Attack);
  attack_piece_supported = std::min(16, supported_mask.countBit());
}


osl::CArray<int, osl::eval::ml::King25EffectY::DIM/2>
osl::eval::ml::King25EffectY::attack_table;
osl::CArray<int, osl::eval::ml::King25EffectY::DIM/2>
osl::eval::ml::King25EffectY::defense_table;

void osl::eval::ml::
King25EffectY::setUp(const Weights &weights)
{
  attack_table.fill(0);
  defense_table.fill(0);
  for (size_t i = 0; i < DIM/2; ++i)
  {
    attack_table[i] = weights.value(i);
    defense_table[i] = weights.value(i+DIM/2);
  }
}



void osl::eval::ml::
King25EmptySquareNoEffect::setUpBase(const Weights &weights, CArray<int, 15>& table)
{
  table.fill(0);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}

template <osl::Player defense>
int osl::eval::ml::
King25EmptySquareNoEffect::evalOne(const NumEffectState &state, const CArray<int, 15>& table)
{
  int result = 0;
  const Piece king_piece = state.kingPiece<defense>();
  const Square king = king_piece.square();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);

  PieceMask pieces=state.piecesOnBoard(defense);
  pieces.reset(KingTraits<defense>::index);

  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (state.pieceAt(target).isEmpty()
	  && ! /*state.hasEffectNotBy(defense, king_piece, target)*/
	  (pieces & state.effectSetAt(target)).any())
      {
	if (defense == BLACK)
	  result += table[index(x - king.x(), y - king.y())];
	else
	  result -= table[index(king.x() - x, king.y() - y)];
      }
    }
  }
  return result;
}

template <osl::Player defense>
std::pair<int,int> osl::eval::ml::
King25EmptySquareNoEffect::evalOne(const NumEffectState &state, 
				     const CArray<int, 15>& opening, const CArray<int, 15>& ending)
{
  int result_o = 0, result_e = 0;
  const Piece king_piece = state.kingPiece<defense>();
  const Square king = king_piece.square();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);

  PieceMask pieces=state.piecesOnBoard(defense);
  pieces.reset(KingTraits<defense>::index);

  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (state.pieceAt(target).isEmpty()
	  && ! /*state.hasEffectNotBy(defense, king_piece, target)*/
	  (pieces & state.effectSetAt(target)).any())
      {
	if (defense == BLACK) 
	{
	  result_o += opening[index(x - king.x(), y - king.y())];
	  result_e += ending[index(x - king.x(), y - king.y())];
	}
	else
	{
	  result_o -= opening[index(king.x() - x, king.y() - y)];
	  result_e -= ending[index(king.x() - x, king.y() - y)];
	}
      }
    }
  }
  return std::make_pair(result_o, result_e);
}

std::pair<osl::CArray<int,2>, osl::CArray<int,2> > osl::eval::ml::
King25EmptySquareNoEffect::eval(const NumEffectState &state, 
				  const CArray<int, 15>& opening, const CArray<int, 15>& ending)
{
  std::pair<int,int> b = evalOne<BLACK>(state, opening, ending);
  std::pair<int,int> w = evalOne<WHITE>(state, opening, ending);
  CArray<int,2> result_o = {{ b.first, w.first }};
  CArray<int,2> result_e = {{ b.second, w.second }};
  return std::make_pair(result_o, result_e);
}

std::pair<osl::CArray<int,2>, osl::CArray<int,2> > osl::eval::ml::
King25EmptySquareNoEffect::evalWithUpdate(const NumEffectState &state, Move last_move,
					    const CArray<int, 15>& opening, const CArray<int, 15>& ending,
					    const CArray<int, 2>& last_opening_value, 
					    const CArray<int, 2>& last_ending_value)
{
  BoardMask mb = state.changedEffects(BLACK), mw = state.changedEffects(WHITE);
  mb.set(Square(last_move.to())); mb.set(Square(last_move.from()));
  mw.set(Square(last_move.to())); mw.set(Square(last_move.from()));
  const Square kb = state.kingSquare<BLACK>(), kw = state.kingSquare<WHITE>();
  const bool update_black = mb.anyInRange(Board_Mask_Table5x5.mask(kb));
  const bool update_white = mw.anyInRange(Board_Mask_Table5x5.mask(kw));
  std::pair<int,int> b = update_black 
    ? evalOne<BLACK>(state, opening, ending)
    : std::make_pair(last_opening_value[0], last_ending_value[0]);
  std::pair<int,int> w = update_white
    ? evalOne<WHITE>(state, opening, ending)
    : std::make_pair(last_opening_value[1], last_ending_value[1]);
  CArray<int,2> result_o = {{ b.first, w.first }};
  CArray<int,2> result_e = {{ b.second, w.second }};
  return std::make_pair(result_o, result_e);
}


osl::CArray<int, 15>
osl::eval::ml::King25EmptySquareNoEffectOpening::table;
osl::CArray<int, 15>
osl::eval::ml::King25EmptySquareNoEffectEnding::table;

const osl::CArray<int,2> osl::eval::ml::
King25EmptySquareNoEffectOpening::eval(const NumEffectState &state)
{
  CArray<int, 2> result = {{ evalOne<BLACK>(state, table), evalOne<WHITE>(state, table) }};
  return result;
}

const osl::CArray<int,2> osl::eval::ml::
King25EmptySquareNoEffectEnding::eval(const NumEffectState &state)
{
  CArray<int, 2> result = {{ evalOne<BLACK>(state, table), evalOne<WHITE>(state, table) }};
  return result;
}



template <int Stage>
osl::CArray<int, 5 * 3 * 8 * 3>
osl::eval::ml::King25EffectEach<Stage>::table;

template <int Stage>
void osl::eval::ml::
King25EffectEach<Stage>::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}

template <int Stage>
template <osl::Player Defense>
osl::eval::ml::EffectState
osl::eval::ml::King25EffectEach<Stage>::effectState(
  const NumEffectState &state,
  Square target)
{
  if (!state.hasEffectAt(alt(Defense), target))
  {
    return static_cast<EffectState>(std::min(2, state.countEffect(Defense, target)));
  }
  const int diff = state.countEffect(Defense, target) -
    state.countEffect(alt(Defense), target);
  return static_cast<EffectState>(std::max(-2, std::min(2, diff)) + ATTACK_DIFF_0);
}

template <int Stage>
template <osl::Player Defense>
int osl::eval::ml::King25EffectEach<Stage>::index(
  const NumEffectState &state, Square king,
  Square target)
{
  const Piece piece = state.pieceAt(target);
  // [0, 2]
  const int rel_x = std::abs(king.x() - target.x());
  // [-2, +2]
  const int rel_y = (target.y() - king.y()) * (Defense == BLACK ? 1 : -1);
  int piece_owner;
  if (piece.isEmpty())
    piece_owner = 0;
  else if (piece.owner() == Defense)
    piece_owner = 1;
  else
    piece_owner = 2;

  int val = rel_y + 2 + rel_x * 5 +
    effectState<Defense>(state, target) * 5 * 3 +
    piece_owner * 5 * 3 * 8;
  return val;
}

template <int Stage>
template <osl::Player Defense>
int osl::eval::ml::King25EffectEach<Stage>::evalOne(
  const NumEffectState &state)
{
  int result = 0;
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      result += table[index<Defense>(state, king, target)];
    }
  }
  if (Defense == BLACK)
    return result;
  else
    return -result;
}

template <int Stage>
int osl::eval::ml::
King25EffectEach<Stage>::eval(const NumEffectState &state)
{
  return evalOne<BLACK>(state) + evalOne<WHITE>(state);
}

osl::CArray<MultiInt, 5 * 3 * 8 * 3>
osl::eval::ml::King25EffectEachBoth::table;
osl::CArray<MultiInt, 3000>
osl::eval::ml::King25EffectEachBoth::x_table;
osl::CArray<MultiInt, 3240>
osl::eval::ml::King25EffectEachBoth::y_table;
osl::CArray<MultiInt, 27000>
osl::eval::ml::King25EffectEachBoth::xy_table;
osl::CArray<int, 256>
osl::eval::ml::King25EffectEachBoth::effect_state_table;

void osl::eval::ml::
King25EffectEachBothOpening::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    King25EffectEachBoth::table[i][0] = weights.value(i);
  }
}

void osl::eval::ml::
King25EffectEachBothMidgame::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    King25EffectEachBoth::table[i][1] = weights.value(i);
  }
}

void osl::eval::ml::
King25EffectEachBothMidgame2::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    King25EffectEachBoth::table[i][2] = weights.value(i);
  }
}

void osl::eval::ml::
King25EffectEachBothEnding::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    King25EffectEachBoth::table[i][EndgameIndex] = weights.value(i);
  }
}
void osl::eval::ml::
King25EffectEachXY::setUp(const Weights &weights)
{
  for(int rel_y_2=0;rel_y_2<5;rel_y_2++)
    for(int x_diff_2=0;x_diff_2<5;x_diff_2++)
      for(int es=0;es<8;es++)
	for(int po=0;po<3;po++)
	  for(int king_x_1=0;king_x_1<5;king_x_1++){
	    int oldIndex=(rel_y_2+x_diff_2*5+es*5*5+po*5*5*8)*5+king_x_1;
	    int newIndex=po+3*(es+8*(rel_y_2+5*(x_diff_2+5*king_x_1)));
	    for (int s=0; s<NStages; ++s)
	      King25EffectEachBoth::x_table[newIndex][s] = weights.value(oldIndex + X_DIM*s);
	  }
  for(int rel_y_2=0;rel_y_2<5;rel_y_2++)
    for(int rel_x=0;rel_x<3;rel_x++)
      for(int es=0;es<8;es++)
	for(int po=0;po<3;po++)
	  for(int king_y_1=0;king_y_1<9;king_y_1++){
	    int oldIndex=(rel_y_2+rel_x*5+es*5*3+po*5*3*8)*9+king_y_1;
	    int newIndex=po+3*(es+8*(rel_y_2+5*(rel_x+3*king_y_1)));
	    for (int s=0; s<NStages; ++s)
	      King25EffectEachBoth::y_table[newIndex][s] = weights.value(oldIndex+ X_DIM * EvalStages + Y_DIM*s)+King25EffectEachBoth::table[oldIndex/9][s];
	  }
  for(int d_effect=0;d_effect<16;d_effect++){
    for(int a_effect=0;a_effect<16;a_effect++){
      if(a_effect==0){
	King25EffectEachBoth::effect_state_table[a_effect*16+d_effect]=3*(std::min(2, d_effect));
      }
      else{
	int diff=d_effect-a_effect;
	King25EffectEachBoth::effect_state_table[a_effect*16+d_effect]=
	  3*(std::max(-2, std::min(2, diff)) + ATTACK_DIFF_0);
      }
    }
  }
}

void osl::eval::ml::
King25EffectEachKXY::setUp(const Weights &weights)
{
  for(int rel_y_2=0;rel_y_2<5;rel_y_2++)
    for(int x_diff_2=0;x_diff_2<5;x_diff_2++){
      int rel_x=std::abs(x_diff_2-2);
      for(int es=0;es<8;es++)
	for(int po=0;po<3;po++)
	  for(int king_x_1=0;king_x_1<5;king_x_1++)
	    for (int king_y_1=0;king_y_1<9;king_y_1++) {
	      int oldIndex=((rel_y_2+x_diff_2*5+es*5*5+po*5*5*8)*9+king_y_1)*5 + king_x_1;
	      int newIndexX=po+3*(es+8*(rel_y_2+5*(x_diff_2+5*king_x_1)));
	      int newIndexY=po+3*(es+8*(rel_y_2+5*(rel_x+3*king_y_1)));
	      int newIndex=po+3*(es+8*(rel_y_2+5*(x_diff_2+5*(king_x_1+5*king_y_1))));
	      for (int s=0; s<NStages; ++s)
		King25EffectEachBoth::xy_table[newIndex][s] = weights.value(oldIndex + ONE_DIM*s)+King25EffectEachBoth::x_table[newIndexX][s]+King25EffectEachBoth::y_table[newIndexY][s];
	  }
    }
}

template <osl::Player Defense>
int
osl::eval::ml::King25EffectEachBoth::effectStateIndex3(
  const NumEffectState &state, Square target)
{
  NumBitmapEffect effect=state.effectSetAt(target);
  const int d_effect=effect.countEffect(Defense);
  const int a_effect=effect.countEffect(alt(Defense));
  return effect_state_table[a_effect*16+d_effect];
}

template <osl::Player Defense>
void osl::eval::ml::King25EffectEachBoth::index(
  const NumEffectState &state, 
  Square target, int &index_xy,
  int rel_y, int king_x, int king_y, int x_diff)
{
  const Piece piece = state.pieceAt(target);
  // piece_owner: 0 - empty, 1 - defense, 2 - attack
  int piece_owner;
  PtypeO ptypeO=piece.ptypeO();
  if(Defense==BLACK){
#ifdef __INTEL_COMPILER
    piece_owner = (unsigned int)((int)(ptypeO)>>30);
    piece_owner &= 0x2;
    piece_owner |= (ptypeO+14)>>4;
#else
    piece_owner=((ptypeO+14)>>4)|(((unsigned int)ptypeO>>30)&0x2);
#endif
  }
  else{
    piece_owner=(((ptypeO+14)>>3)&0x2)|((unsigned int)ptypeO>>31);
  }
  assert(piece_owner >= 0 && piece_owner < 3);
  int effect_state_index = effectStateIndex3<Defense>(state, target);

  index_xy=piece_owner+effect_state_index+3*(8*((rel_y+2)+5*((x_diff+2)+5*(king_x-1+5*(king_y-1)))));
}

template <osl::Player Defense>
void osl::eval::ml::King25EffectEachBoth::evalOne(
  const NumEffectState &state, MultiInt &out)
{
  out.clear();
  const Square king = state.kingSquare<Defense>();
  const int min_dx = std::max(1, king.x() - 2)-king.x();
  const int max_dx = std::min(9, king.x() + 2)-king.x();
  const int min_dy = std::max(1, king.y() - 2)-king.y();
  const int max_dy = std::min(9, king.y() + 2)-king.y();
  const int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
  const int king_y = (Defense == BLACK ? king.y() : 10 - king.y());
  if ((Defense == BLACK && king.x() >= 6) ||
      (Defense == WHITE && king.x() >= 5)){
    for (int dx = min_dx; dx <= max_dx; ++dx)
      {
	// [0, 2]
	// const int rel_x = std::abs(dx);
	// [-2, 2]
	int x_diff = dx;
	for (int dy = min_dy; dy <= max_dy; ++dy)
	  {
	    const Square target(king.x()+dx, king.y()+dy);
	    // [-2, +2]
	    const int rel_y = dy * (Defense == BLACK ? 1 : -1);
	    int index_xy;
	    index<Defense>(state, target, index_xy,
			   rel_y,king_x,king_y,x_diff);
	    out += xy_table[index_xy];
	  }
      }
  }
  else {
    for (int dx = min_dx; dx <= max_dx; ++dx)
      {
	// [0, 2]
	// const int rel_x = std::abs(dx);
	// [-2, 2]
	int x_diff = -dx;
	for (int dy = min_dy; dy <= max_dy; ++dy)
	  {
	    const Square target(king.x()+dx, king.y()+dy);
	    // [-2, +2]
	    const int rel_y = dy * (Defense == BLACK ? 1 : -1);
	    int index_xy;
	    index<Defense>(state, target, index_xy,
			   rel_y,king_x,king_y,x_diff);
	    out +=  xy_table[index_xy];
	  }
      }
  }
  if (Defense != BLACK)
    {
      out = -out;
    }
}

void osl::eval::ml::
King25EffectEachBoth::eval(const NumEffectState &state,
			   MultiIntPair &out)
{
  evalOne<BLACK>(state, out[BLACK]);
  evalOne<WHITE>(state, out[WHITE]);
}

void
osl::eval::ml::King25EffectEachBoth::evalWithUpdate(
  const NumEffectState &state, Move last_move,
  MultiIntPair &values)
{
  BoardMask mb = state.changedEffects(BLACK), mw = state.changedEffects(WHITE);
  mb.set(Square(last_move.to())); mb.set(Square(last_move.from()));
  mw.set(Square(last_move.to())); mw.set(Square(last_move.from()));
  const Square kb = state.kingSquare<BLACK>(), kw = state.kingSquare<WHITE>();
  const bool update_black = mb.anyInRange(Board_Mask_Table5x5.mask(kb)) ||
    mw.anyInRange(Board_Mask_Table5x5.mask(kb));
  const bool update_white = mw.anyInRange(Board_Mask_Table5x5.mask(kw)) ||
    mb.anyInRange(Board_Mask_Table5x5.mask(kw));
  if (update_black)
  {
    evalOne<BLACK>(state, values[BLACK]);
  }
  if (update_white)
  {
    evalOne<WHITE>(state, values[WHITE]);
  }
}


template <bool Opening>
osl::CArray<int, 1125> osl::eval::ml::King25EmptyAbs<Opening>::table;

template <bool Opening>
void osl::eval::ml::
King25EmptyAbs<Opening>::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}

template <bool Opening>
template <osl::Player player>
int osl::eval::ml::King25EmptyAbs<Opening>::index(Square king,
							    Square target)
{
  int x, target_x;
  if ((player == BLACK && king.x() >= 6) ||
      (player == WHITE && king.x() >= 5))
  {
    x = 10 - king.x();
    target_x = 10 - target.x();
  }
  else
  {
    x = king.x();
    target_x = target.x();
  }
  const int y = (player == BLACK ? king.y() : 10 - king.y());
  const int target_y = (player == BLACK ? target.y() : 10 - target.y());

  return target_y - y + 2 + (target_x - x + 2 ) * 5 + (y - 1) * 5 * 5
    + (x - 1) * 5 * 5 * 9;
}

template <bool Opening>
template <osl::Player Defense>
int osl::eval::ml::King25EmptyAbs<Opening>::evalOne(
  const NumEffectState &state)
{
  int result = 0;
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (target.isOnBoard() && state.pieceAt(target).isEmpty())
      {
	result += table[index<Defense>(king, target)];
      }
    }
  }
  if (Defense == BLACK)
    return result;
  else
    return -result;
}

template <bool Opening>
int osl::eval::ml::King25EmptyAbs<Opening>::eval(
  const NumEffectState &state)
{
  return evalOne<BLACK>(state) + evalOne<WHITE>(state);
}

template <bool Opening>
int osl::eval::ml::King25EmptyAbs<Opening>::evalWithUpdate(
  const NumEffectState &state, osl::Move moved,
  int last_value)
{
  if (moved.ptype() == osl::KING)
  {
    return eval(state);
  }
  const osl::Square self_king = state.kingSquare(moved.player());
  const osl::Square opp_king = state.kingSquare(alt(moved.player()));
  int result = last_value;

  if (!moved.isDrop())
  {
    const Square from = moved.from();
    if (std::abs(self_king.x() - from.x()) <= 2 &&
	std::abs(self_king.y() - from.y()) <= 2)
    {
      result += table[index(self_king, moved.from(), moved.player())] *
	(moved.player() == BLACK ? 1 : -1);
    }

    if (std::abs(opp_king.x() - from.x()) <= 2 &&
	std::abs(opp_king.y() - from.y()) <= 2)
    {
      result -= table[index(opp_king, from, alt(moved.player()))] *
	(moved.player() == BLACK ? 1 : -1);
    }
  }

  Ptype captured = moved.capturePtype();
  if (captured == PTYPE_EMPTY)
  {
    const Square to = moved.to();
    if (std::abs(self_king.x() - to.x()) <= 2 &&
	std::abs(self_king.y() - to.y()) <= 2)
    {
      result -= table[index(self_king, to, moved.player())] *
	(moved.player() == BLACK ? 1 : -1);
    }
    if (std::abs(opp_king.x() - to.x()) <= 2 &&
	std::abs(opp_king.y() - to.y()) <= 2)
    {
      result += table[index(opp_king, to, alt(moved.player()))] *
	(moved.player() == BLACK ? 1 : -1);
    }
  }
  return result;

}

osl::CArray<MultiInt, 3072> osl::eval::ml::King3Pieces::table;
osl::CArray<MultiInt, 15360> osl::eval::ml::King3Pieces::x_table;
osl::CArray<MultiInt, 27648> osl::eval::ml::King3Pieces::y_table;

void osl::eval::ml::King3Pieces::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King3PiecesXY::setUp(const Weights &weights)
{
  for (int i = 0; i < X_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King3Pieces::x_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for (int i = 0; i < Y_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King3Pieces::y_table[i][s] = weights.value(i + ONE_DIM*s + X_DIM);
  }
}

template <osl::Player King>
void osl::eval::ml::King3Pieces::evalOne(const NumEffectState &state,
						   MultiInt &result)
{
  const Square king = state.kingSquare<King>();
  const int vertical_index =
    index<King, VERTICAL>(
      state.pieceAt(king + DirectionPlayerTraits<U, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<D, King>::offset()).ptypeO());
  const int vertical_index_x =
    indexX<King, VERTICAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<U, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<D, King>::offset()).ptypeO());
  const int vertical_index_y =
    indexY<King, VERTICAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<U, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<D, King>::offset()).ptypeO());
  const int horizontal_index =
    index<King, HORIZONTAL>(
      state.pieceAt(king + DirectionPlayerTraits<L, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<R, King>::offset()).ptypeO());
  const int horizontal_index_x =
    indexX<King, HORIZONTAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<L, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<R, King>::offset()).ptypeO());
  const int horizontal_index_y =
    indexY<King, HORIZONTAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<L, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<R, King>::offset()).ptypeO());
  const int diagonal_index1 =
    index<King, DIAGONAL>(
      state.pieceAt(king + DirectionPlayerTraits<UL, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<DR, King>::offset()).ptypeO());
  const int diagonal_index2 =
    index<King, DIAGONAL>(
      state.pieceAt(king + DirectionPlayerTraits<UR, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<DL, King>::offset()).ptypeO());
  const int diagonal_index1_x =
    indexX<King, DIAGONAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<UL, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<DR, King>::offset()).ptypeO());
  const int diagonal_index2_x=
    indexX<King, DIAGONAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<UR, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<DL, King>::offset()).ptypeO());
  const int diagonal_index1_y =
    indexY<King, DIAGONAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<UL, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<DR, King>::offset()).ptypeO());
  const int diagonal_index2_y =
    indexY<King, DIAGONAL>(
      king,
      state.pieceAt(king + DirectionPlayerTraits<UR, King>::offset()).ptypeO(),
      state.pieceAt(king + DirectionPlayerTraits<DL, King>::offset()).ptypeO());
  const MultiInt v = value(vertical_index, horizontal_index,
			    diagonal_index1, diagonal_index2,
			    vertical_index_x,  horizontal_index_x,
			    diagonal_index1_x, diagonal_index2_x,
			    vertical_index_y , horizontal_index_y,
			    diagonal_index1_y, diagonal_index2_y);
  if (King == BLACK)
  {
    result += v;
  }
  else
  {
    result -= v;
  }
}

MultiInt
osl::eval::ml::King3Pieces::eval(const NumEffectState &state)
{
  MultiInt result;
  evalOne<BLACK>(state, result);
  evalOne<WHITE>(state, result);
  return result;
}

MultiInt
osl::eval::ml::King3Pieces::evalWithUpdate(
  const NumEffectState &state,
  Move last_move,
  MultiInt &last_value)
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  if ((std::abs(last_move.to().x() - kings[0].x()) <= 1 &&
       std::abs(last_move.to().y() - kings[0].y()) <= 1) ||
      (std::abs(last_move.to().x() - kings[1].x()) <= 1 &&
       std::abs(last_move.to().y() - kings[1].y()) <= 1))
    return eval(state);
  if (!last_move.isDrop())
  {
    if ((std::abs(last_move.from().x() - kings[0].x()) <= 1 &&
	 std::abs(last_move.from().y() - kings[0].y()) <= 1) ||
	(std::abs(last_move.from().x() - kings[1].x()) <= 1 &&
	 std::abs(last_move.from().y() - kings[1].y()) <= 1))
      return eval(state);
  }
  return last_value;
}



osl::CArray<int, 17 * 128> osl::eval::ml::King25EffectAttack::table;
osl::CArray<MultiInt, 17 * 128> osl::eval::ml::King25EffectDefense::table;

osl::CArray<int, 17 * 128 * 9> osl::eval::ml::King25EffectYAttack::table;
osl::CArray<MultiInt, 17 * 128 * 9> osl::eval::ml::King25EffectYDefense::table;


osl::CArray<MultiInt, 3240> osl::eval::ml::KingMobility::table;
osl::CArray<MultiInt, 3240> osl::eval::ml::KingMobility::rook_table;
osl::CArray<MultiInt, 3240> osl::eval::ml::KingMobility::bishop_table;
osl::CArray<MultiInt, 3240> osl::eval::ml::KingMobility::rook_bishop_table;

void osl::eval::ml::KingMobility::setUp(const Weights &weights)
{
  static CArray<MultiInt, 3240> old_table;
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      old_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int king_x=0;king_x<5;king_x++)
    for(int king_y=0;king_y<9;king_y++)
      for(int dir=0;dir<8;dir++)
	for(int mobility=0;mobility<9;mobility++){
	  int oldIndex=king_x + 5 * (king_y + 9 * (dir + 8 * mobility));
	  int newIndex=mobility+9*(dir+8*(king_y+9*king_x));
	  for (int s=0; s<NStages; ++s)
	    table[newIndex][s]=old_table[oldIndex][s];
	}
}

void osl::eval::ml::
KingMobilityWithRook::setUp(const Weights &weights)
{
  static CArray<MultiInt, 3240> old_table;
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      old_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int king_x=0;king_x<5;king_x++)
    for(int king_y=0;king_y<9;king_y++)
      for(int dir=0;dir<8;dir++)
	for(int mobility=0;mobility<9;mobility++){
	  int oldIndex=king_x + 5 * (king_y + 9 * (dir + 8 * mobility));
	  int newIndex=mobility+9*(dir+8*(king_y+9*king_x));
	  for (int s=0; s<NStages; ++s)
	    KingMobility::rook_table[newIndex][s]=old_table[oldIndex][s];
	}
}

void osl::eval::ml::
KingMobilityWithBishop::setUp(const Weights &weights)
{
  static CArray<MultiInt, 3240> old_table;
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      old_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int king_x=0;king_x<5;king_x++)
    for(int king_y=0;king_y<9;king_y++)
      for(int dir=0;dir<8;dir++)
	for(int mobility=0;mobility<9;mobility++){
	  int oldIndex=king_x + 5 * (king_y + 9 * (dir + 8 * mobility));
	  int newIndex=mobility+9*(dir+8*(king_y+9*king_x));
	  for (int s=0; s<NStages; ++s)
	    KingMobility::bishop_table[newIndex][s]=old_table[oldIndex][s];
	}
  for(int i=0;i<3240;i++){
    KingMobility::rook_bishop_table[i]=
      KingMobility::table[i]+KingMobility::rook_table[i]+KingMobility::bishop_table[i];
    KingMobility::rook_table[i]+=KingMobility::table[i];
    KingMobility::bishop_table[i]+=KingMobility::table[i];
  }
}

template <osl::Player P>
osl::MultiInt osl::eval::ml::
KingMobility::evalOne(const NumEffectState &state)
{
  MultiInt result;
  const Square king = state.kingSquare<P>();
  const int king_x = (king.x() > 5 ? 10 - king.x() : king.x()) - 1;
  const int king_y = (P == BLACK ? king.y() : 10 - king.y()) - 1;
  int indexBase=9*8*(king_y+9*king_x);
  if(P==BLACK){
    if (state.hasPieceOnStand<ROOK>(alt(P)))
    {
      if(state.hasPieceOnStand<BISHOP>(alt(P))){
	result =
	  rook_bishop_table[indexBase+0*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	  rook_bishop_table[indexBase+1*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	  rook_bishop_table[indexBase+2*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	  rook_bishop_table[indexBase+3*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	  rook_bishop_table[indexBase+4*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	  rook_bishop_table[indexBase+5*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	  rook_bishop_table[indexBase+6*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	  rook_bishop_table[indexBase+7*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))];
      }
      else{
	result =
	  rook_table[indexBase+0*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	  rook_table[indexBase+1*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	  rook_table[indexBase+2*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	  rook_table[indexBase+3*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	  rook_table[indexBase+4*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	  rook_table[indexBase+5*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	  rook_table[indexBase+6*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	  rook_table[indexBase+7*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))];
      }
    }
    else if(state.hasPieceOnStand<BISHOP>(alt(P))){
      result = 
	bishop_table[indexBase+0*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	bishop_table[indexBase+1*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	bishop_table[indexBase+2*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	bishop_table[indexBase+3*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	bishop_table[indexBase+4*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	bishop_table[indexBase+5*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	bishop_table[indexBase+6*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	bishop_table[indexBase+7*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))];
    }
    else{
      result = 
	table[indexBase+0*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	table[indexBase+1*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	table[indexBase+2*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	table[indexBase+3*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	table[indexBase+4*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	table[indexBase+5*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	table[indexBase+6*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	table[indexBase+7*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))];
    }
  }
  else{
    if (state.hasPieceOnStand<ROOK>(alt(P)))
    {
      if(state.hasPieceOnStand<BISHOP>(alt(P))){
	result = -(
	  rook_bishop_table[indexBase+7*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	  rook_bishop_table[indexBase+6*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	  rook_bishop_table[indexBase+5*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	  rook_bishop_table[indexBase+4*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	  rook_bishop_table[indexBase+3*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	  rook_bishop_table[indexBase+2*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	  rook_bishop_table[indexBase+1*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	  rook_bishop_table[indexBase+0*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))]);
      }
      else{
	result = -(
	  rook_table[indexBase+7*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	  rook_table[indexBase+6*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	  rook_table[indexBase+5*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	  rook_table[indexBase+4*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	  rook_table[indexBase+3*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	  rook_table[indexBase+2*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	  rook_table[indexBase+1*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	  rook_table[indexBase+0*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))]);
      }
    }
    else if(state.hasPieceOnStand<BISHOP>(alt(P))){
      result = -(
	bishop_table[indexBase+7*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	bishop_table[indexBase+6*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	bishop_table[indexBase+5*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	bishop_table[indexBase+4*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	bishop_table[indexBase+3*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	bishop_table[indexBase+2*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	bishop_table[indexBase+1*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	bishop_table[indexBase+0*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))]);
    }
    else{
      result = -(
	table[indexBase+7*9+mobilityDir<UL>(king,state.kingMobilityAbs(P, UL))]+
	table[indexBase+6*9+mobilityDir<U>(king,state.kingMobilityAbs(P, U))]+
	table[indexBase+5*9+mobilityDir<UR>(king,state.kingMobilityAbs(P, UR))]+
	table[indexBase+4*9+mobilityDir<L>(king,state.kingMobilityAbs(P, L))]+
	table[indexBase+3*9+mobilityDir<R>(king,state.kingMobilityAbs(P, R))]+
	table[indexBase+2*9+mobilityDir<DL>(king,state.kingMobilityAbs(P, DL))]+
	table[indexBase+1*9+mobilityDir<D>(king,state.kingMobilityAbs(P, D))]+
	table[indexBase+0*9+mobilityDir<DR>(king,state.kingMobilityAbs(P, DR))]);
    }
  }
  return result;
}  

osl::MultiInt osl::eval::ml::
KingMobility::eval(const NumEffectState &state)
{
  MultiInt result = evalOne<BLACK>(state) + evalOne<WHITE>(state);
  return result;
}  


osl::CArray<MultiInt, 45*33> osl::eval::ml::KingMobilitySum::table;

void osl::eval::ml::KingMobilitySum::setUp(const Weights &weights)
{
  CArray<MultiInt, 2925> old_table;
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      old_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int king_x=0;king_x<5;king_x++)
    for(int king_y=0;king_y<9;king_y++)
      for(int mobility=0;mobility<=32;mobility++){
	int oldIndex=king_x+5*(king_y+9*(mobility+8));
	int newIndex=mobility+33*(king_y+9*king_x);
	for (int s=0; s<NStages; ++s)
	  table[newIndex]=old_table[oldIndex];
      }
}

template <osl::Player P>
osl::MultiInt osl::eval::ml::
KingMobilitySum::evalOne(const NumEffectState &state)
{
  MultiInt result;
  const Square king = state.kingSquare<P>();
  int sum=state.kingMobilityAbs(P, UL).y()+state.kingMobilityAbs(P, U).y()+
    state.kingMobilityAbs(P, UR).y()+state.kingMobilityAbs(P, R).x()-
    state.kingMobilityAbs(P, DL).y()-state.kingMobilityAbs(P, D).y()-
    state.kingMobilityAbs(P, DR).y()-state.kingMobilityAbs(P, L).x();
  const int king_x = (king.x() > 5 ? 10 - king.x() : king.x()) - 1;
  const int king_y = (P == BLACK ? king.y() : 10 - king.y()) - 1;
  int mobility=sum-8;
  result = table[mobility+33*(king_y+9*king_x)];
  if (P == BLACK)
  {
    return result;
  }
  else
  {
    return -result;
  }
}  

osl::MultiInt osl::eval::ml::
KingMobilitySum::eval(const NumEffectState &state)
{
  MultiInt result = evalOne<BLACK>(state) + evalOne<WHITE>(state);
  return result;
}  


osl::CArray<MultiInt, 8192>
osl::eval::ml::King25BothSide::table;
osl::CArray<MultiInt, 40960>
osl::eval::ml::King25BothSide::x_table;
osl::CArray<MultiInt, 73728>
osl::eval::ml::King25BothSide::y_table;

void osl::eval::ml::King25BothSide::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25BothSideX::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King25BothSide::x_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25BothSideY::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King25BothSide::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int king_y=1;king_y<=9;king_y++)
    for(int effect1=0;effect1<32;effect1++)
      for(int effect2=0;effect2<32;effect2++)
	for(int i=0;i<8;i++){
	  int index0=effect1 + 32 * (effect2 + 32 * i);
	  int index1=king_y - 1 + 9 *(effect1 + 32 * (effect2 + 32 * i));
	  King25BothSide::y_table[index1] += King25BothSide::table[index0];
	}
}

template<osl::Player P>
osl::MultiInt osl::eval::ml::
King25BothSide::evalOne(const NumEffectState &state,
			const CArray<int, 5> &effects)
{
  const Square king=state.kingSquare<P>();
  const int king_y = (P==BLACK ? king.y() : 10 - king.y());
  const int king_x = (king.x() >= 6 ? 10 - king.x() : king.x());
  if ((P== BLACK && king.x() > 5) || (P==WHITE && king.x() < 5)){
    return
      x_table[indexX(king_x,effects[2],effects[0],(2*3)+1)]+ // (0,2)
      y_table[indexY(king_y,effects[0],effects[2],(0*3)+0)]+
      x_table[indexX(king_x,effects[3],effects[0],(1*3)+2)]+ // (0,3)
      y_table[indexY(king_y,effects[0],effects[3],(0*3)+1)]+
      x_table[indexX(king_x,effects[4],effects[0],(0*3)+2)]+ // (0,4)
      y_table[indexY(king_y,effects[0],effects[4],(0*3)+2)]+
      x_table[indexX(king_x,effects[2],effects[1],(2*3)+0)]+ // (1,2)
      y_table[indexY(king_y,effects[1],effects[2],(1*3)+0)]+
      x_table[indexX(king_x,effects[3],effects[1],(1*3)+1)]+ // (1,3)
      y_table[indexY(king_y,effects[1],effects[3],(1*3)+1)]+
      x_table[indexX(king_x,effects[4],effects[1],(0*3)+1)]+ // (1,4)
      y_table[indexY(king_y,effects[1],effects[4],(1*3)+2)]+
      x_table[indexX(king_x,effects[3],effects[2],(1*3)+0)]+ // (2,3)
      y_table[indexY(king_y,effects[2],effects[3],(2*3)+0)]+
      x_table[indexX(king_x,effects[4],effects[2],(0*3)+0)]+ // (2,4)
      y_table[indexY(king_y,effects[2],effects[4],(2*3)+1)];
  }
  else{
    return
      x_table[indexX(king_x,effects[0],effects[2],(0*3)+0)]+ // (0,2)
      y_table[indexY(king_y,effects[0],effects[2],(0*3)+0)]+
      x_table[indexX(king_x,effects[0],effects[3],(0*3)+1)]+ // (0,3)
      y_table[indexY(king_y,effects[0],effects[3],(0*3)+1)]+
      x_table[indexX(king_x,effects[0],effects[4],(0*3)+2)]+ // (0,4)
      y_table[indexY(king_y,effects[0],effects[4],(0*3)+2)]+
      x_table[indexX(king_x,effects[1],effects[2],(1*3)+0)]+ // (1,2)
      y_table[indexY(king_y,effects[1],effects[2],(1*3)+0)]+
      x_table[indexX(king_x,effects[1],effects[3],(1*3)+1)]+ // (1,3)
      y_table[indexY(king_y,effects[1],effects[3],(1*3)+1)]+
      x_table[indexX(king_x,effects[1],effects[4],(1*3)+2)]+ // (1,4)
      y_table[indexY(king_y,effects[1],effects[4],(1*3)+2)]+
      x_table[indexX(king_x,effects[2],effects[3],(2*3)+0)]+ // (2,3)
      y_table[indexY(king_y,effects[2],effects[3],(2*3)+0)]+
      x_table[indexX(king_x,effects[2],effects[4],(2*3)+1)]+ // (2,4)
      y_table[indexY(king_y,effects[2],effects[4],(2*3)+1)];
  }
}

osl::MultiInt osl::eval::ml::
King25BothSide::eval(const NumEffectState &state,
		     const CArray<int, 5> &black,
		     const CArray<int, 5> &white)
{
  return evalOne<BLACK>(state,black)-evalOne<WHITE>(state,white);
}

osl::CArray<MultiInt, 2400>
osl::eval::ml::King25Effect3::table;
osl::CArray<MultiInt, 21600>
osl::eval::ml::King25Effect3::y_table;

void osl::eval::ml::King25Effect3::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25Effect3Y::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King25Effect3::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <osl::Player Attack>
osl::MultiInt osl::eval::ml::
King25Effect3::evalOne(const NumEffectState &state,
		       PieceMask king25)
{
  king25 = king25 & state.piecesOnBoard(Attack);
  const PieceMask promoted_mask = (king25 & state.promotedPieces());
  const bool with_knight =
    (king25 & ~state.promotedPieces()).template selectBit<KNIGHT>().any();
  king25.clearBit<KNIGHT>();
  king25.clearBit<LANCE>();
  king25.clearBit<PAWN>();
  king25 = king25 | promoted_mask;
  const int piece_count = std::min(9, king25.countBit());
  const int stand_count = std::min(9,
				   state.countPiecesOnStand<ROOK>(Attack) +
				   state.countPiecesOnStand<BISHOP>(Attack) +
				   state.countPiecesOnStand<GOLD>(Attack) +
				   state.countPiecesOnStand<SILVER>(Attack));
  const bool stand_with_knight = state.hasPieceOnStand<KNIGHT>(Attack);
  const Player Defense = alt(Attack);
  PieceMask attacked =
    state.effectedMask(Attack) & state.piecesOnBoard(Defense);
  attacked.clearBit<KNIGHT>();
  attacked.clearBit<LANCE>();
  attacked.clearBit<PAWN>();
  PieceMask attacking;
  while (attacked.any())
  {
    const Piece piece = state.pieceOf(attacked.takeOneBit());
    attacking = attacking | state.effectSetAt(piece.square());
  }
  attacking = (attacking & state.piecesOnBoard(Attack) & ~king25);
  const int attacked_count = std::min(5, attacking.countBit());
  if (Attack == BLACK)
  {
    return table[index(piece_count, with_knight,
		       stand_count, stand_with_knight, attacked_count)] +
      y_table[indexY(piece_count, with_knight,
		     stand_count, stand_with_knight, attacked_count,
		     state.kingSquare<WHITE>().y())];
  }
  else
  {
    return -(table[index(piece_count, with_knight,
			 stand_count, stand_with_knight, attacked_count)] +
	     y_table[indexY(piece_count, with_knight,
			    stand_count, stand_with_knight, attacked_count,
			    10 - state.kingSquare<BLACK>().y())]);
  }
}

osl::MultiInt osl::eval::ml::
King25Effect3::eval(const NumEffectState &state,
		    const CArray<PieceMask, 2> &king25_mask)
{
  return evalOne<BLACK>(state, king25_mask[WHITE]) +
    evalOne<WHITE>(state, king25_mask[BLACK]);
}


osl::CArray<MultiInt, 4096>
osl::eval::ml::King25Mobility::table;
osl::CArray<MultiInt, 20480>
osl::eval::ml::King25Mobility::x_table;
osl::CArray<MultiInt, 36864>
osl::eval::ml::King25Mobility::y_table;

void osl::eval::ml::King25Mobility::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25MobilityX::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King25Mobility::x_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25MobilityY::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King25Mobility::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
King25Mobility::eval(const NumEffectState &state,
		     const CArray<int, 5> &black,
		     const CArray<int, 5> &white)
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare<BLACK>(),
      state.kingSquare<WHITE>(),
    }};
  MultiInt result;
  for (size_t i = 1; i < black.size(); ++i)
  {
    result += (table[index(black[i - 1], black[i], i - 1)] +
	       x_table[indexX<BLACK>(kings[BLACK],
				     black[i - 1], black[i], i - 1)] +
	       y_table[indexY<BLACK>(kings[BLACK],
				     black[i - 1], black[i], i - 1)]);
    result -= (table[index(white[i - 1], white[i], i - 1)] +
	       x_table[indexX<WHITE>(kings[WHITE],
				     white[i - 1], white[i], i - 1)] +
	       y_table[indexY<WHITE>(kings[WHITE],
				     white[i - 1], white[i], i - 1)]);
  }
  return result;
}


osl::CArray<MultiInt, 100>
osl::eval::ml::King25EffectCountCombination::table;
osl::CArray<MultiInt, 900>
osl::eval::ml::King25EffectCountCombination::y_table;

void osl::eval::ml::King25EffectCountCombination::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::King25EffectCountCombinationY::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      King25EffectCountCombination::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <osl::Player Attack>
osl::MultiInt osl::eval::ml::
King25EffectCountCombination::evalOne(const NumEffectState &state,
				      PieceMask king25)
{
  const Player Defense = alt(Attack);
  PieceMask attack = king25 & state.piecesOnBoard(Attack);
  PieceMask defense =
    king25 & state.piecesOnBoard(Defense);
  const PieceMask attack_promoted_mask = (attack & state.promotedPieces());
  const PieceMask defense_promoted_mask = (defense & state.promotedPieces());
  attack.clearBit<KNIGHT>();
  attack.clearBit<LANCE>();
  attack.clearBit<PAWN>();
  attack = attack | attack_promoted_mask;
  defense.clearBit<KNIGHT>();
  defense.clearBit<LANCE>();
  defense.clearBit<PAWN>();
  defense = defense | defense_promoted_mask;
  const int attack_count = std::min(9,
				    attack.countBit() +
				    state.countPiecesOnStand<ROOK>(Attack) +
				    state.countPiecesOnStand<BISHOP>(Attack) +
				    state.countPiecesOnStand<GOLD>(Attack) +
				    state.countPiecesOnStand<SILVER>(Attack));
  const int defense_count = std::min(9,
				    defense.countBit() +
				    state.countPiecesOnStand<ROOK>(Defense) +
				    state.countPiecesOnStand<BISHOP>(Defense) +
				    state.countPiecesOnStand<GOLD>(Defense) +
				    state.countPiecesOnStand<SILVER>(Defense));
  const int y = (Attack == BLACK ? state.kingSquare<Defense>().y() :
		 10 - state.kingSquare<Defense>().y());
  MultiInt result = table[attack_count + 10 * defense_count] +
    y_table[y - 1 + 9 * (attack_count + 10 * defense_count)];
  if (Attack == BLACK)
    return result;
  else
    return -result;
}

osl::MultiInt osl::eval::ml::
King25EffectCountCombination::eval(const NumEffectState &state,
				   const CArray<PieceMask, 2> &king25_mask)
{
  return evalOne<BLACK>(state, king25_mask[WHITE]) +
    evalOne<WHITE>(state, king25_mask[BLACK]);
}


osl::CArray<int, 18*81*(45*2)*3> osl::eval::ml::BishopExchangeSilverKing::table;
void osl::eval::ml::
BishopExchangeSilverKing::setUp(const Weights & weights)
{
  for (size_t i=0; i<weights.dimension(); ++i)
    table[i] = weights.value(i);
}

int osl::eval::ml::
BishopExchangeSilverKing::eval(const NumEffectState &state)
{
  if (state.promotedPieces().any())
    return 0;
  PieceMask stand_all = state.standMask(BLACK) | state.standMask(WHITE);
  stand_all.clearBit<BISHOP>();
  stand_all.clearBit<PAWN>();
  if (stand_all.any())
    return 0;
  if (state.nth<BISHOP>(0).owner() == state.nth<BISHOP>(1).owner())
    return 0;
  if (state.nth<BISHOP>(0).isOnBoard() != state.nth<BISHOP>(1).isOnBoard())
    return 0;
  int offset = 0;
  if (state.nth<BISHOP>(0).isOnBoard()) {
    offset += BISHOP_ONE_DIM;
    if (state.hasEffectByPiece(state.nth<BISHOP>(0),
			       state.nth<BISHOP>(1).square()))
      offset += BISHOP_ONE_DIM;
  }
  return evalOne<BLACK>(state, offset) + evalOne<WHITE>(state, offset);
}

template <osl::Player KingOwner>
int osl::eval::ml::
BishopExchangeSilverKing::evalOne(const NumEffectState &state, int offset)
{
  Square king = state.kingSquare(KingOwner);
  int king_index = indexKing(king.squareForBlack(alt(KingOwner))); // rotate if king is black
  if (king_index < 0)
    return 0;
  CArray<Piece,2> rook = {{
      state.nth<ROOK>(0), state.nth<ROOK>(1),
    }};
  if (rook[0].owner() == rook[1].owner())
    return 0;
  if (rook[0].owner() == KingOwner)
    std::swap(rook[0], rook[1]);
  int rook_index0 = indexKing(rook[0].square().squareForBlack(KingOwner)); // rotate if attaking rook is black
  if (rook_index0 < 0)
    return 0;
  int rook_index1 = indexKing(rook[1].square().squareForBlack(alt(KingOwner))); // rotate if defending rook is black
  if (rook_index1 < 0)
    return 0;  
  FixedCapacityVector<Square, 4> silver;
  for (int i=0; i<state.nthLimit<SILVER>(); ++i) {
    Piece p = state.nth<SILVER>(i);
    assert(p.isOnBoard());
    if (p.owner() == KingOwner)
      continue;
    silver.push_back(p.square().squareForBlack(KingOwner)); // rotate if silver is black
  }
  if (silver.size() != 2 || silver[0].x() == silver[1].x())
    return 0;
  int silver_index
    = indexSilver((silver[0].x() > silver[1].x()) ? silver[0] : silver[1]);
  int index = offset + (king_index*81+silver_index)*90;
  return table[index + rook_index0] * sign(KingOwner)
    + table[index + 45 + rook_index1] * sign(KingOwner);
}




template <osl::Player KingOwner>
int osl::eval::ml::
EnterKingDefense::evalOne(const NumEffectState &state)
{
  Square king = state.kingSquare(KingOwner);
  if (king.y() < 4 || king.y() > 6) // target: [4,6]
    return 0;
  CArray2d<int, 2, 2> count = {{{{ 0 }}}};
  for (int x=std::max(1, king.x()-2); x<=std::min(9, king.x()+2); ++x) {
    for (int y=king.y()-2*sign(KingOwner); 1; y-=sign(KingOwner)) {
      const Piece p = state.pieceAt(Square(x, y));
      if (p.isEdge())
	break;
      if (p.isEmpty())
	continue;
      count[p.owner() == KingOwner][p.ptype() == PAWN]++;
    }
  }
  const int c = king.squareForBlack(KingOwner).y() - 4;
  return table[c*(std::min(7, count[0][0]))] * sign(KingOwner)
    + table[c*(8 +std::min(7, count[0][1]))] * sign(KingOwner)
    + table[c*(16+std::min(7, count[1][0]))] * sign(KingOwner)
    + table[c*(24+std::min(7, count[1][1]))] * sign(KingOwner);
}

int osl::eval::ml::
EnterKingDefense::eval(const NumEffectState &state)
{
  return evalOne<BLACK>(state) + evalOne<WHITE>(state);
}

osl::CArray<int, (8+8+8+8)*3> osl::eval::ml::EnterKingDefense::table;
void osl::eval::ml::
EnterKingDefense::setUp(const Weights & weights)
{
  for (size_t i=0; i<weights.dimension(); ++i)
    table[i] = weights.value(i);
}



namespace osl
{
  namespace eval
  {
    namespace ml
    {
      template MultiInt KingPieceRelative::
      evalWithUpdate<BLACK>(const NumEffectState &state,
			    Move moved, MultiInt const& last_value);
      template MultiInt KingPieceRelative::
      evalWithUpdate<WHITE>(const NumEffectState &state,
			    Move moved, MultiInt const& last_value);

      template class King25EffectEach<0>;
      template class King25EffectEach<1>;
      template class King25EffectEach<2>;
      template class King25EmptyAbs<true>;
      template class King25EmptyAbs<false>;

      template void King25EffectBoth::countEffectAndPiecesBoth<BLACK>(const NumEffectState&, PieceMask&, PieceMask&, int&, int&, int&, int&, int&, CArray<int, 5>&, CArray<int, 5>&);
      template void King25EffectBoth::countEffectAndPiecesBoth<WHITE>(const NumEffectState&, PieceMask&, PieceMask&, int&, int&, int&, int&, int&, CArray<int, 5>&, CArray<int, 5>&);

      template MultiInt King25BothSide::
      evalOne<BLACK>(const NumEffectState &state, const CArray<int, 5> &effects);
      template MultiInt King25BothSide::
      evalOne<WHITE>(const NumEffectState &state, const CArray<int, 5> &effects);
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
