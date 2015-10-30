#include "osl/eval/king8.h"
#include <cstdlib>
using osl::MultiInt;
using osl::MultiIntPair;

template <bool Opening>
osl::CArray<int, 32> osl::eval::ml::King8EffectEmptySquare<Opening>::table;
template <bool Opening>
osl::CArray<int, 32> osl::eval::ml::King8EffectDefenseSquare<Opening>::table;

osl::CArray<int, 32> osl::eval::ml::King8Effect::empty_table;
osl::CArray<int, 32> osl::eval::ml::King8Effect::defense_table;
osl::CArray<int, 288> osl::eval::ml::King8Effect::empty_y_table;
osl::CArray<int, 288> osl::eval::ml::King8Effect::defense_y_table;

void osl::eval::ml::
King8Effect::setUp(const Weights &weights)
{
  for (size_t i = 0; i < empty_table.size(); ++i)
  {
    empty_table[i] = weights.value(i);
  }
  for (size_t i = 0; i < defense_table.size(); ++i)
  {
    defense_table[i] = weights.value(i + empty_table.size());
  }
  for (size_t i = 0; i < empty_y_table.size(); ++i)
  {
    empty_y_table[i] = weights.value(i + empty_table.size() +
				     defense_table.size());
  }
  for (size_t i = 0; i < defense_y_table.size(); ++i)
  {
    defense_y_table[i] = weights.value(i + empty_table.size() +
				       defense_table.size() +
				       empty_y_table.size());
  }
}

int osl::eval::ml::King8Effect::eval(
  const NumEffectState &state)
{
  int result = 0;
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  for (int i = SHORT8_DIRECTION_MIN; i <= SHORT8_DIRECTION_MAX; ++i)
  {
    const Direction dir = static_cast<Direction>(i);
    {
      EffectState empty, defense;
      effectState(state, BLACK, dir, empty, defense);
      if (empty != NOT_EMPTY)
      {
	result -= empty_table[index(dir, empty)];
	result -= empty_y_table[indexY(black_king, dir, empty)];
      }
      if (defense != NOT_EMPTY)
      {
	result -= defense_table[index(dir, defense)];
	result -= defense_y_table[indexY(black_king, dir, defense)];
      }
    }
    {
      EffectState empty, defense;
      effectState(state, WHITE, dir, empty, defense);
      if (empty != NOT_EMPTY)
      {
	result += empty_table[index(dir, empty)];
	result += empty_y_table[indexY(white_king, dir, empty)];
      }
      if (defense != NOT_EMPTY)
      {
	result += defense_table[index(dir, defense)];
	result += defense_y_table[indexY(white_king, dir, defense)];
      }
    }
  }
  return result;
}

int osl::eval::ml::
King8Effect::index(const Direction dir,
		   EffectState state)
{
  return (dir * 4 + state);
}

int osl::eval::ml::
King8Effect::indexY(Piece king,
		    const Direction dir,
		    EffectState state)
{
  const int y = ((king.owner() == BLACK) ?
		 king.square().y() : 10 - king.square().y());
  return (dir * 4 + state) * 9 + y - 1;
}



void osl::eval::ml::
King8Effect::effectState(const NumEffectState &state,
			 const Player defenseP,
			 const Direction dir,
			 EffectState &empty,
			 EffectState &defense)
{
  const Square target =
    Board_Table.nextSquare(defenseP,
			     state.kingSquare(defenseP),
			     dir);
  if (!state.pieceAt(target).isEmpty())
  {
    empty = defense = NOT_EMPTY;
    return;
  }
  const int attack_count = state.countEffect(alt(defenseP), target);
  const int defense_count = state.countEffect(defenseP, target);
  if (attack_count == 0)
  {
    empty = NO_EFFECT;
    defense = NO_EFFECT;
  }
  else if (defense_count == 1)
  {
    empty = MORE_EFFECT_KING_ONLY;
  }
  else if (attack_count >= defense_count)
  {
    empty = MORE_EFFECT;
  }
  else
  {
    empty = LESS_EFFECT;
  }
  if (defense_count == 1 && attack_count > defense_count)
  {
    defense = MORE_EFFECT_KING_ONLY;
  }
  else if (attack_count > defense_count)
  {
    defense = MORE_EFFECT;
  }
  else
  {
    defense = LESS_EFFECT;
  }
}



struct osl::eval::ml::King8EffectBase::
    MakeEffectStateSimple
{
  EffectState operator()(const NumEffectState &state,
			 const Player defense,
			 const Direction dir) const
  {
    const Square target =
      Board_Table.nextSquare(defense,
			       state.kingSquare(defense),
			       dir);
    if (!state.pieceAt(target).isEmpty())
      return NOT_EMPTY;
	
    const int attack_count = state.countEffect(alt(defense), target);
    if (attack_count == 0)
      return NO_EFFECT;
    const int defense_count = state.countEffect(defense, target);
    if (defense_count == 1)
      return MORE_EFFECT_KING_ONLY;
    else if (attack_count >= defense_count)
      return MORE_EFFECT;
    else
      return LESS_EFFECT;
  }
};

struct osl::eval::ml::King8EffectBase::
    MakeEffectStateDefense
{
  EffectState operator()(const NumEffectState &state,
			 const Player defense,
			 const Direction dir) const
  {
    const Square target =
      Board_Table.nextSquare(defense,
			       state.kingSquare(defense),
			       dir);
    if (!state.pieceAt(target).isOnBoardByOwner(defense))
      return NOT_EMPTY;

    const int attack_count = state.countEffect(alt(defense), target);
    if (attack_count == 0)
      return NO_EFFECT;

    const int defense_count = state.countEffect(defense, target);
    if (defense_count == 1 && attack_count > defense_count)
      return MORE_EFFECT_KING_ONLY;
    else if (attack_count > defense_count)
      return MORE_EFFECT;
    else
      return LESS_EFFECT;
  }
};

template <class MakeEffectState>
const osl::CArray<int,2>
#if (defined __GNUC__) && (! defined GPSONE) && (! defined GPSUSIONE)
__attribute__ ((used))
#endif
 osl::eval::ml::
King8EffectBase::evalCommon(const NumEffectState &state, const table_t& table)
{
  MakeEffectState f;
  CArray<int,2> result = {{0, 0}};
  for (int i = SHORT8_DIRECTION_MIN; i <= SHORT8_DIRECTION_MAX; ++i)
  {
    const Direction dir = static_cast<Direction>(i);
    const EffectState black_effect_state = f(state, BLACK, dir);
    if (black_effect_state != NOT_EMPTY)
    {
      result[0] -= table[index(dir, black_effect_state)];
    }
    const EffectState white_effect_state = f(state, WHITE, dir);
    if (white_effect_state != NOT_EMPTY)
    {
      result[1] += table[index(dir, white_effect_state)];
    }
  }

  return result;
}

template <class MakeEffectState>
const osl::CArray<int,2>
#if (defined __GNUC__) && (! defined GPSONE) && (! defined GPSUSIONE)
__attribute__ ((used))
#endif
 osl::eval::ml::
King8EffectBase::evalWithUpdateCommon(const NumEffectState &new_state, Move last_move, 
				      const CArray<int,2>& last_value, const table_t& table)
{
  CArray<int,2> result = last_value;
  MakeEffectState f;
  BoardMask mask = new_state.changedEffects();
  mask.set(last_move.to()); mask.set(last_move.from());
  for (int z=0; z<2; ++z) 
  {
    const Player pl = indexToPlayer(z);
    const Square king = new_state.kingSquare(pl);
    bool update = mask.anyInRange(Board_Mask_Table3x3.mask(king));
    if (! update) 
      continue;
    result[z] = 0;
    for (int i = SHORT8_DIRECTION_MIN; i <= SHORT8_DIRECTION_MAX; ++i)
    {
      const Direction dir = static_cast<Direction>(i);
      const EffectState effect_state = f(new_state, pl, dir);
      if (effect_state != NOT_EMPTY)
      {
	result[z] -= table[index(dir, effect_state)];
      }
    }
    if (z == 1)
      result[1] = -result[1];
  }
  return result;
}

template <class MakeEffectState>
inline
std::pair<osl::CArray<int,2>, osl::CArray<int,2> > osl::eval::ml::
King8EffectBase::evalWithUpdateCommon(const NumEffectState &new_state, Move last_move, 
				      const CArray<int,2>& last_value_o, const CArray<int,2>& last_value_e, 
				      const table_t& table_o, const table_t& table_e)
{
  CArray<int,2> result_o = last_value_o, result_e = last_value_e;
  MakeEffectState f;
  BoardMask mask = new_state.changedEffects();
  mask.set(last_move.to()); mask.set(last_move.from());
  for (int z=0; z<2; ++z) 
  {
    const Player pl = indexToPlayer(z);
    const Square king = new_state.kingSquare(pl);
    bool update = mask.anyInRange(Board_Mask_Table3x3.mask(king));
    if (! update) 
      continue;
    result_o[z] = result_e[z] = 0;
    for (int i = SHORT8_DIRECTION_MIN; i <= SHORT8_DIRECTION_MAX; ++i)
    {
      const Direction dir = static_cast<Direction>(i);
      const EffectState effect_state = f(new_state, pl, dir);
      if (effect_state != NOT_EMPTY)
      {
	result_o[z] -= table_o[index(dir, effect_state)];
	result_e[z] -= table_e[index(dir, effect_state)];
      }
    }
    if (z == 1) 
    {
      result_o[1] = -result_o[1];
      result_e[1] = -result_e[1];
    }
  }
  return std::make_pair(result_o, result_e);
}

template <bool Opening>
void osl::eval::ml::
King8EffectEmptySquare<Opening>::setUp(const Weights &weights)
{
  table.fill(0);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}

template <bool Opening>
const osl::CArray<int,2> osl::eval::ml::
King8EffectEmptySquare<Opening>::eval(const NumEffectState &state)
{
  return evalCommon<MakeEffectStateSimple>(state, table);
}
template <bool Opening>
const osl::CArray<int,2> osl::eval::ml::
King8EffectEmptySquare<Opening>::evalWithUpdate(const NumEffectState &new_state, Move last_move, 
					  const CArray<int,2>& last_value)
{
  return evalWithUpdateCommon<MakeEffectStateSimple>
    (new_state, last_move, last_value, table);
}

std::pair<osl::CArray<int,2>, osl::CArray<int,2> >
osl::eval::ml::King8EffectEmptySquareBoth::
evalWithUpdate(const NumEffectState &new_state, Move last_move, 
	       const CArray<int,2>& last_value_opening,
	       const CArray<int,2>& last_value_ending)
{
  return evalWithUpdateCommon<MakeEffectStateSimple>
    (new_state, last_move, last_value_opening, last_value_ending, 
     King8EffectEmptySquare<true>::table, King8EffectEmptySquare<false>::table);
}	


template <bool Opening>
void osl::eval::ml::
King8EffectDefenseSquare<Opening>::setUp(const Weights &weights)
{
  table.fill(0);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}
template <bool Opening>
const osl::CArray<int,2> osl::eval::ml::
King8EffectDefenseSquare<Opening>::eval(const NumEffectState &state)
{
  return evalCommon<MakeEffectStateDefense>(state, table);
}
template <bool Opening>
const osl::CArray<int,2> osl::eval::ml::
King8EffectDefenseSquare<Opening>::evalWithUpdate(const NumEffectState &new_state, Move last_move, 
					  const CArray<int,2>& last_value)
{
  return evalWithUpdateCommon<MakeEffectStateDefense>
    (new_state, last_move, last_value, table);
}


std::pair<osl::CArray<int,2>, osl::CArray<int,2> >
osl::eval::ml::King8EffectDefenseSquareBoth::
evalWithUpdate(const NumEffectState &new_state, Move last_move, 
	       const CArray<int,2>& last_value_opening,
	       const CArray<int,2>& last_value_ending)
{
  return evalWithUpdateCommon<MakeEffectStateDefense>
    (new_state, last_move, last_value_opening, last_value_ending, 
     King8EffectDefenseSquare<true>::table, King8EffectDefenseSquare<false>::table);
}	



osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::base_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::u_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::d_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::l_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::r_table;

osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::base_defense_piece_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::u_defense_piece_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::d_defense_piece_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::l_defense_piece_table;
osl::CArray<int, osl::eval::ml::King8EffectAll::ONE_DIM>
osl::eval::ml::King8EffectAll::r_defense_piece_table;

void osl::eval::ml::
King8EffectAll::setUp(const Weights &weights)
{
  base_table.fill(0);
  u_table.fill(0);
  d_table.fill(0);
  l_table.fill(0);
  r_table.fill(0);
  base_defense_piece_table.fill(0);
  u_defense_piece_table.fill(0);
  d_defense_piece_table.fill(0);
  l_defense_piece_table.fill(0);
  r_defense_piece_table.fill(0);
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    base_table[i] = weights.value(i);
    u_table[i] = weights.value(i+ONE_DIM);
    d_table[i] = weights.value(i+ONE_DIM*2);
    l_table[i] = weights.value(i+ONE_DIM*3);
    r_table[i] = weights.value(i+ONE_DIM*4);
    base_defense_piece_table[i] = weights.value(i+ONE_DIM*5);
    u_defense_piece_table[i] = weights.value(i+ONE_DIM*6);
    d_defense_piece_table[i] = weights.value(i+ONE_DIM*7);
    l_defense_piece_table[i] = weights.value(i+ONE_DIM*8);
    r_defense_piece_table[i] = weights.value(i+ONE_DIM*9);
  }
}

void
osl::eval::ml::King8EffectAll::effectState(
  const NumEffectState &state,
  const Player defense,
  const Direction dir,
  EffectState &empty, EffectState &defense_effect)
{
  empty = NOT_EMPTY;
  defense_effect = NOT_EMPTY;
  const Square target =
    Board_Table.nextSquare(defense,
			     state.kingSquare(defense),
			     dir);
  const Piece piece = state.pieceAt(target);
  if (!target.isOnBoard() ||
      piece.isOnBoardByOwner(alt(defense)))
    return;

  const int attack_count = state.countEffect(alt(defense), target);
  const int defense_count = state.countEffect(defense, target);

  if (piece.isEmpty())
  {
    if (attack_count == 0)
      empty = NO_EFFECT;
    else if (defense_count == 1)
      empty = MORE_EFFECT_KING_ONLY;
    else if (attack_count >= defense_count)
      empty = MORE_EFFECT;
    else
      empty = LESS_EFFECT;
  }
  else
  {
    if (attack_count == 0)
      defense_effect = NO_EFFECT;
    else if (defense_count == 1 && attack_count > defense_count)
      defense_effect = MORE_EFFECT_KING_ONLY;
    else if (attack_count > defense_count)
      defense_effect = MORE_EFFECT;
    else
      defense_effect = LESS_EFFECT;
  }
}

int osl::eval::ml::
King8EffectAll::index(const Direction dir, EffectState state)
{
  return dir * 4 + state;
}

int osl::eval::ml::
King8EffectAll::eval(const NumEffectState &state,
		     PieceMask /*black_mask*/, PieceMask /*white_mask*/)
{
  int result = 0;
  osl::checkmate::King8Info black_king(state.Iking8Info(BLACK));
  const int black_liberty = black_king.liberty();
  const bool black_u_blocked =
    (black_liberty & ((DirectionTraits<UL>::mask |
		       DirectionTraits<U>::mask |
		       DirectionTraits<UR>::mask))) == 0;
  const bool black_d_blocked =
    (black_liberty & ((DirectionTraits<DL>::mask |
		       DirectionTraits<D>::mask |
		       DirectionTraits<DR>::mask))) == 0;
  const bool black_l_blocked =
    (black_liberty & ((DirectionTraits<UL>::mask |
		       DirectionTraits<L>::mask |
		       DirectionTraits<DL>::mask))) == 0;
  const bool black_r_blocked =
    (black_liberty & ((DirectionTraits<UR>::mask |
		       DirectionTraits<R>::mask |
		       DirectionTraits<DR>::mask))) == 0;
  osl::checkmate::King8Info white_king(state.Iking8Info(WHITE));
  const int white_liberty = white_king.liberty();
  const bool white_u_blocked =
    (white_liberty & ((DirectionTraits<UL>::mask |
		       DirectionTraits<U>::mask |
		       DirectionTraits<UR>::mask))) == 0;
  const bool white_d_blocked =
    (white_liberty & ((DirectionTraits<DL>::mask |
		       DirectionTraits<D>::mask |
		       DirectionTraits<DR>::mask))) == 0;
  const bool white_l_blocked =
    (white_liberty & ((DirectionTraits<UL>::mask |
		       DirectionTraits<L>::mask |
		       DirectionTraits<DL>::mask))) == 0;
  const bool white_r_blocked =
    (white_liberty & ((DirectionTraits<UR>::mask |
		       DirectionTraits<R>::mask |
		       DirectionTraits<DR>::mask))) == 0;

  for (int i = SHORT8_DIRECTION_MIN; i <= SHORT8_DIRECTION_MAX; ++i)
  {
    const Direction dir = static_cast<Direction>(i);
    EffectState black_empty_effect_state,
      black_defense_effect_state;
    effectState(state, BLACK, dir,
		black_empty_effect_state, black_defense_effect_state);
    if (black_empty_effect_state != NOT_EMPTY)
    {
      const int idx = index(dir, black_empty_effect_state);
      result -= base_table[idx];
      if (black_u_blocked)
	result -= u_table[idx];
      if (black_d_blocked)
	result -= d_table[idx];
      if (black_l_blocked)
	result -= l_table[idx];
      if (black_r_blocked)
	result -= r_table[idx];
    }
    if (black_defense_effect_state != NOT_EMPTY)
    {
      const int idx = index(dir, black_defense_effect_state);
      result -= base_defense_piece_table[idx];
      if (black_u_blocked)
	result -= u_defense_piece_table[idx];
      if (black_d_blocked)
	result -= d_defense_piece_table[idx];
      if (black_l_blocked)
	result -= l_defense_piece_table[idx];
      if (black_r_blocked)
	result -= r_defense_piece_table[idx];
    }
    EffectState white_empty_effect_state,
      white_defense_effect_state;
    effectState(state, WHITE, dir,
		white_empty_effect_state, white_defense_effect_state);
    if (white_empty_effect_state != NOT_EMPTY)
    {
      const int idx = index(dir, white_empty_effect_state);
      result += base_table[idx];
      if (white_u_blocked)
	result += u_table[idx];
      if (white_d_blocked)
	result += d_table[idx];
      if (white_l_blocked)
	result += l_table[idx];
      if (white_r_blocked)
	result += r_table[idx];
    }
    if (white_defense_effect_state != NOT_EMPTY)
    {
      const int idx = index(dir, white_defense_effect_state);
      result += base_defense_piece_table[idx];
      if (white_u_blocked)
	result += u_defense_piece_table[idx];
      if (white_d_blocked)
	result += d_defense_piece_table[idx];
      if (white_l_blocked)
	result += l_defense_piece_table[idx];
      if (white_r_blocked)
	result += r_defense_piece_table[idx];
    }
  }

  return result;
}


osl::CArray<MultiInt, osl::eval::ml::KingXBothBlocked::ONE_DIM>
osl::eval::ml::KingXBothBlocked::table;
osl::CArray<MultiInt, osl::eval::ml::KingXBothBlockedY::ONE_DIM>
osl::eval::ml::KingXBothBlockedY::table;

void osl::eval::ml::
KingXBothBlocked::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
KingXBothBlockedY::setUp(const Weights &weights)
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
KingXBothBlocked::adjust(int index, int index_y, MultiInt &out)
{
  if(Sign>0)
    out += KingXBothBlocked::table[index] +KingXBothBlockedY::table[index_y];
  else
    out -= KingXBothBlocked::table[index] +KingXBothBlockedY::table[index_y];
}

MultiIntPair osl::eval::ml::
KingXBothBlocked::eval(const NumEffectState &state)
{
  MultiIntPair result;
  King8Info black(state.Iking8Info(BLACK));
  if ((black.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask |
			  DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    const Square black_king = state.kingSquare<BLACK>();
    adjust<1>(index(black_king),
	      indexY<BLACK>(black_king),
	      result[BLACK]);
  }

  King8Info white(state.Iking8Info(WHITE));
  if ((white.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask |
			  DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    const Square white_king = state.kingSquare<WHITE>();
    adjust<-1>(index(white_king),
	       indexY<WHITE>(white_king),
	       result[WHITE]);
  }

  return result;
}

template <osl::Player P>
int osl::eval::ml::KingXBlockedBase::index(
  Square king, int diff)
{
  const int king_x = king.x();
  if (P == BLACK)
  {
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 6)
      x_diff = -x_diff;
    return target_x - 1 + ((x_diff == 1) ? 0 : 5);
  }
  else
  {
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 5)
      x_diff = -x_diff;
    return target_x - 1 + ((x_diff == 1) ? 0 : 5);
  }
}

template <osl::Player P>
bool osl::eval::ml::KingXBlockedBase::isBlocked(
  const NumEffectState &state,
  int diff)
{
#if 1
  const King8Info info(state.Iking8Info(P));
  if ((diff == 1) ^ (P == BLACK))
    return (info.liberty() & (DirectionTraits<UR>::mask 
			      | DirectionTraits<R>::mask 
			      | DirectionTraits<DR>::mask)) == 0;
  assert((diff == 1 && P == BLACK) || (diff == -1 && P == WHITE));
  return (info.liberty() & (DirectionTraits<UL>::mask 
			    | DirectionTraits<L>::mask 
			    | DirectionTraits<DL>::mask)) == 0;
#else
  const Square pos = state.kingSquare<P>();
  const int target_x = pos.x() + diff;
  for (int y = pos.y() - 1; y <= pos.y() + 1; ++y)
  {
    Square target(target_x, y);
    Piece p(state.pieceAt(target));
    if ((!p.isEdge()) && ! p.isOnBoardByOwner<P>() &&
	!state.hasEffectAt<alt(P)>(target))
    {
      return false;
    }
  }
  return true;
#endif
}

const MultiIntPair osl::eval::ml::
KingXBlockedBase::eval(const NumEffectState &state, const table_t& table)
{
  MultiIntPair val;
  const Square black_king = state.kingSquare<BLACK>();
  const Square white_king = state.kingSquare<WHITE>();
  const int b = playerToIndex(BLACK), w = playerToIndex(WHITE);
  if (isBlocked<BLACK>(state, 1))
    val[b] += table[index<BLACK>(black_king, 1)];
  if (isBlocked<BLACK>(state, -1))
    val[b] += table[index<BLACK>(black_king, -1)];

  if (isBlocked<WHITE>(state, 1))
    val[w] -= table[index<WHITE>(white_king, 1)];
  if (isBlocked<WHITE>(state, -1))
    val[w] -= table[index<WHITE>(white_king, -1)];
  return val;
}

const MultiIntPair osl::eval::ml::
KingXBlockedYBase::eval(const NumEffectState &state,
			const table_t& table)
{
  MultiIntPair val;
  const Square black_king = state.kingSquare<BLACK>();
  const Square white_king = state.kingSquare<WHITE>();
  const int b = playerToIndex(BLACK), w = playerToIndex(WHITE);
  const bool black_r_blocked = KingXBlockedBase::isBlocked<BLACK>(state, 1);
  const bool black_l_blocked = KingXBlockedBase::isBlocked<BLACK>(state, -1);
  if (black_r_blocked)
    val[b] += table[index<BLACK>(black_king, 1)];
  if (black_l_blocked)
    val[b] += table[index<BLACK>(black_king, -1)];

  const bool white_r_blocked = KingXBlockedBase::isBlocked<WHITE>(state, 1);
  const bool white_l_blocked = KingXBlockedBase::isBlocked<WHITE>(state, -1);
  if (white_r_blocked)
    val[w] -= table[index<WHITE>(white_king, 1)];
  if (white_l_blocked)
    val[w] -= table[index<WHITE>(white_king, -1)];
  return val;
}

#if 0
inline
std::pair<osl::CArray<int,2>,osl::CArray<int,2> > 
osl::eval::ml::
KingXBlockedBase::evalWithUpdate(const NumEffectState &new_state, Move last_move,
				 const CArray<int,2>& last_value_o, const CArray<int,2>& last_value_e,
				 const table_t& table_o, const table_t& table_e)
{
  CArray<int,2> val_o = last_value_o;
  CArray<int,2> val_e = last_value_e;
  const Square black_king = new_state.kingSquare<BLACK>();
  const Square white_king = new_state.kingSquare<WHITE>();
  BoardMask mask = new_state.changedEffects();
  mask.set(last_move.from()); mask.set(last_move.to());
  if (mask.anyInRange(Board_Mask_Table3x3.mask(black_king))) 
  {
    const int b = playerToIndex(BLACK);
    val_o[b] = val_e[b]= 0;
    if (isBlocked<BLACK>(new_state, 1)) {
      val_o[b] += table_o[index<BLACK>(black_king, 1)];
      val_e[b] += table_e[index<BLACK>(black_king, 1)];
    }
    if (isBlocked<BLACK>(new_state, -1)) {
      val_o[b] += table_o[index<BLACK>(black_king, -1)];
      val_e[b] += table_e[index<BLACK>(black_king, -1)];
    }
  }
  if (mask.anyInRange(Board_Mask_Table3x3.mask(white_king)))
  {
    const int w = playerToIndex(WHITE);
    val_o[w] = val_e[w]= 0;
    if (isBlocked<WHITE>(new_state, 1)) {
      val_o[w] -= table_o[index<WHITE>(white_king, 1)];
      val_e[w] -= table_e[index<WHITE>(white_king, 1)];
    }
    if (isBlocked<WHITE>(new_state, -1)) {
      val_o[w] -= table_o[index<WHITE>(white_king, -1)];
      val_e[w] -= table_e[index<WHITE>(white_king, -1)];
    }
  }
  return std::make_pair(val_o, val_e);
}
#endif

template <int Sign>
inline
void osl::eval::ml::
KingXBlockedYBase::adjust(int index, int index_y, MultiInt &out)
{
  if(Sign>0)
    out += KingXBlocked::table[index]+ KingXBlockedY::table[index_y];
  else
    out -= KingXBlocked::table[index]+ KingXBlockedY::table[index_y];
}

inline
void
osl::eval::ml::
KingXBlockedYBase::evalWithUpdateBang(const NumEffectState &new_state,
				      Move last_move,
				      MultiIntPair& values)
{
  const Square black_king = new_state.kingSquare<BLACK>();
  const Square white_king = new_state.kingSquare<WHITE>();
  BoardMask mask = new_state.changedEffects();
  mask.set(last_move.from()); mask.set(last_move.to());
  if (mask.anyInRange(Board_Mask_Table3x3.mask(black_king))) 
  {
    values[BLACK].clear();
    const bool black_r_blocked = KingXBlockedBase::isBlocked<BLACK>(new_state, 1);
    const bool black_l_blocked = KingXBlockedBase::isBlocked<BLACK>(new_state, -1);
    if (black_r_blocked) {
      adjust<1>(KingXBlockedBase::index<BLACK>(black_king, 1),
		index<BLACK>(black_king, 1),
		values[BLACK]);
    }
    if (black_l_blocked) {
      adjust<1>(KingXBlockedBase::index<BLACK>(black_king, -1),
		index<BLACK>(black_king, -1),
		values[BLACK]);
    }
    if (black_r_blocked && black_l_blocked)
    {
      KingXBothBlocked::adjust<1>(KingXBothBlocked::index(black_king),
				  KingXBothBlocked::indexY<BLACK>(black_king),
				  values[BLACK]);
    }
  }
  if (mask.anyInRange(Board_Mask_Table3x3.mask(white_king)))
  {
    values[WHITE].clear();
    const bool white_r_blocked = KingXBlockedBase::isBlocked<WHITE>(new_state, 1);
    const bool white_l_blocked = KingXBlockedBase::isBlocked<WHITE>(new_state, -1);
    if (white_r_blocked) {
      adjust<-1>(KingXBlockedBase::index<WHITE>(white_king, 1),
		 index<WHITE>(white_king, 1),
		 values[WHITE]);
    }
    if (white_l_blocked) {
      adjust<-1>(KingXBlockedBase::index<WHITE>(white_king, -1),
		 index<WHITE>(white_king, -1),
		 values[WHITE]);
    }
    if (white_r_blocked && white_l_blocked)
    {
      KingXBothBlocked::adjust<-1>(KingXBothBlocked::index(white_king),
				  KingXBothBlocked::indexY<WHITE>(white_king),
				  values[WHITE]);
    }
  }
}

template <osl::Player P> inline
int osl::eval::ml::KingXBlockedYBase::index(
  Square king, int diff)
{
  const int king_x = king.x();
  if (P == BLACK)
  {
    const int king_y = king.y();
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 6)
      x_diff = -x_diff;
    return (target_x - 1 + ((x_diff == 1) ? 0 : 5)) * 9 + king_y - 1;
  }
  else
  {
    const int king_y = 10 - king.y();
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 5)
      x_diff = -x_diff;
    return (target_x - 1 + ((x_diff == 1) ? 0 : 5)) * 9 + king_y - 1;
  }
}

osl::CArray<MultiInt, 10> osl::eval::ml::KingXBlocked::table;
osl::CArray<MultiInt, 90> osl::eval::ml::KingXBlockedY::table;

void osl::eval::ml::
KingXBlocked::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < table.size(); ++i)
  {
    table[i][stage] = weights.value(i);
  }
}
void
osl::eval::ml::
KingXBlockedBoth::evalWithUpdateBang(const NumEffectState &new_state, Move last_move,
				     MultiIntPair& last_values)
{
  KingXBlockedYBase::evalWithUpdateBang
    (new_state, last_move, last_values);
}

void osl::eval::ml::
KingXBlockedY::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < table.size(); ++i)
  {
    table[i][stage] = weights.value(i);
  }
}


osl::CArray<MultiInt, 80> osl::eval::ml::KingXBlocked3::table;
osl::CArray<MultiInt, 720> osl::eval::ml::KingXBlocked3::y_table;

void osl::eval::ml::KingXBlocked3::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::KingXBlocked3Y::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      KingXBlocked3::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int x=1;x<=5;x++)
    for(int y=1;y<=9;y++)
      for(int is_l=0;is_l<2;is_l++)
	for(int u_blocked=0;u_blocked<2;u_blocked++)
	  for(int opp_u_blocked=0;opp_u_blocked<2;opp_u_blocked++)
	    for(int opp_blocked=0;opp_blocked<2;opp_blocked++){
	      int indexY=x - 1 + 5 * (y - 1 + 9 * ((is_l ? 1 : 0) + 2 * ((u_blocked ? 1 : 0) + 2 * ((opp_u_blocked ? 1  : 0) + 2 * (opp_blocked ? 1 : 0)))));
	      int index0=x - 1 + 5 * ((is_l ? 1 : 0) + 2 * ((u_blocked ? 1 : 0) + 2 * ((opp_u_blocked ? 1  : 0) + 2 * (opp_blocked ? 1 : 0))));
	      KingXBlocked3::y_table[indexY]+=KingXBlocked3::table[index0];
	    }
}

MultiInt osl::eval::ml::
KingXBlocked3::eval(const NumEffectState &state)
{
  MultiInt result;
  King8Info black(state.Iking8Info(BLACK));
  if ((black.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask)) == 0) 
  {
    adjust<1>(
      indexY<BLACK>(state.kingSquare<BLACK>(),
		    true,
		    (black.liberty() & DirectionTraits<U>::mask) == 0,
		    (black.liberty() & DirectionTraits<UR>::mask) == 0,
		    (black.liberty() & DirectionTraits<R>::mask) == 0),
      result);
  }
  if ((black.liberty() & (DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    adjust<1>(
      indexY<BLACK>(state.kingSquare<BLACK>(),
		    false,
		    (black.liberty() & DirectionTraits<U>::mask) == 0,
		    (black.liberty() & DirectionTraits<UL>::mask) == 0,
		    (black.liberty() & DirectionTraits<L>::mask) == 0),
      result);
  }
  King8Info white(state.Iking8Info(WHITE));
  if ((white.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask)) == 0)
  {
    adjust<-1>(
      indexY<WHITE>(state.kingSquare<WHITE>(),
		    true,
		    (white.liberty() & DirectionTraits<U>::mask) == 0,
		    (white.liberty() & DirectionTraits<UR>::mask) == 0,
		    (white.liberty() & DirectionTraits<R>::mask) == 0),
      result);
  }
  if ((white.liberty() & (DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    adjust<-1>(
      indexY<WHITE>(state.kingSquare<WHITE>(),
		    false,
		    (white.liberty() & DirectionTraits<U>::mask) == 0,
		    (white.liberty() & DirectionTraits<UL>::mask) == 0,
		    (white.liberty() & DirectionTraits<L>::mask) == 0),
      result);
  }
  return result;
}



osl::CArray<MultiInt, 4> osl::eval::ml::AnagumaEmpty::table;


void osl::eval::ml::
AnagumaEmpty::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < table.size(); ++i)
  {
    table[i][stage] = weights.value(i);
  }
}


int osl::eval::ml::AnagumaEmpty::index(
  Square king, Square target)
{
  return std::abs(king.x() - target.x()) + std::abs(king.y() - target.y()) * 2;
}

template <osl::Player Defense>
MultiInt osl::eval::ml::AnagumaEmpty::evalOne(const NumEffectState &state)
{
  MultiInt result;
  const Square king = state.kingSquare<Defense>();
  if ((king.x() == 1 || king.x() == 9) &&
      ((Defense == BLACK && king.y() == 9) ||
       (Defense == WHITE && king.y() == 1))){
    const int x = (king.x() == 1 ? 2 : 8);
    const int y = (Defense == BLACK ? 8 : 2);
    if(Defense==BLACK){
      if (state.pieceAt(Square(king.x(), y)).isEmpty())
	result +=table[index(king, Square(king.x(), y))];
      if (state.pieceAt(Square(x, y)).isEmpty())
	result +=table[index(king, Square(x, y))];
      if (state.pieceAt(Square(x, king.y())).isEmpty())
	result +=table[index(king, Square(x, king.y()))];
    }
    else{
      if (state.pieceAt(Square(king.x(), y)).isEmpty())
	result -=table[index(king, Square(king.x(), y))];
      if (state.pieceAt(Square(x, y)).isEmpty())
	result -=table[index(king, Square(x, y))];
      if (state.pieceAt(Square(x, king.y())).isEmpty())
	result -=table[index(king, Square(x, king.y()))];
    }
  }
  return result;
}

MultiInt osl::eval::ml::
AnagumaEmpty::eval(const NumEffectState &state)
{
  return evalOne<BLACK>(state) + evalOne<WHITE>(state);
}



namespace osl
{
  namespace eval
  {
    namespace ml
    {
      template class King8EffectEmptySquare<true>;
      template class King8EffectEmptySquare<false>;
      template class King8EffectDefenseSquare<true>;
      template class King8EffectDefenseSquare<false>;
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
