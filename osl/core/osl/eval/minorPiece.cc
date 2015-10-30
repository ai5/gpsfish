#include "osl/eval/minorPiece.h"
#include "osl/additionalEffect.h"
#include <iostream>
using osl::MultiInt;

void osl::eval::ml::PawnDropX::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PawnDropBoth::x_table[i][s] = weights.value(i+ONE_DIM*s);
  }
}

osl::CArray<osl::MultiInt, 9>
osl::eval::ml::PawnDropBoth::attack_table;
osl::CArray<osl::MultiInt, 9>
osl::eval::ml::PawnDropBoth::defense_table;
osl::CArray<osl::MultiInt, 81>
osl::eval::ml::PawnDropBoth::attack_y_table;
osl::CArray<osl::MultiInt, 81>
osl::eval::ml::PawnDropBoth::defense_y_table;
osl::CArray<osl::MultiInt, 90>
osl::eval::ml::PawnDropBoth::x_table;
osl::CArray<osl::MultiInt, 18>
osl::eval::ml::PawnDropBoth::stand_table;
osl::CArray<osl::MultiInt, 90>
osl::eval::ml::PawnDropBoth::x_stand_table;
osl::CArray<osl::MultiInt, 162>
osl::eval::ml::PawnDropBoth::y_stand_table;
osl::CArray<osl::MultiInt, 10>
osl::eval::ml::PawnDropBoth::drop_non_drop_table;
osl::CArray<osl::MultiInt, 36>
osl::eval::ml::PawnDropBoth::state_king_relative_table;
osl::CArray<osl::MultiInt, 1>
osl::eval::ml::SilverAdvance26::table;

void osl::eval::ml::PawnDrop::setUp(const Weights &weights,int stage)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    PawnDropBoth::defense_table[i][stage] = weights.value(i);
    PawnDropBoth::attack_table[i][stage] = weights.value(i + ONE_DIM);
  }
}

void osl::eval::ml::PawnDropY::setUp(const Weights &weights,int stage)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    PawnDropBoth::attack_y_table[i][stage] = weights.value(i);
    PawnDropBoth::defense_y_table[i][stage] = weights.value(i + ONE_DIM);
  }
}

void osl::eval::ml::PawnDropPawnStand::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PawnDropBoth::stand_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::PawnDropPawnStandX::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PawnDropBoth::x_stand_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}
void osl::eval::ml::PawnDropPawnStandY::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PawnDropBoth::y_stand_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::PawnDropNonDrop::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PawnDropBoth::drop_non_drop_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::PawnStateKingRelative::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PawnDropBoth::state_king_relative_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::PawnDropBoth::eval(
  const NumEffectState &state)
{
  osl::MultiInt result;
  CArray<Square, 2> kings = {{state.kingSquare<BLACK>(),
			       state.kingSquare<WHITE>()}};
  CArray<Piece, 2> king_piece = {{state.kingPiece<BLACK>(),
				  state.kingPiece<WHITE>()}};
  CArray<bool, 2> has_pawn = {{state.hasPieceOnStand<PAWN>(BLACK),
			       state.hasPieceOnStand<PAWN>(WHITE)}};
  for (int x = 1; x <= 9; ++x)
  {
    const bool black_on_board = state.isPawnMaskSet<BLACK>(x);
    const bool white_on_board = state.isPawnMaskSet<WHITE>(x);
    if (!black_on_board)
    {
      const int attack_index = index(kings[WHITE], x);
      const int defense_index = index(kings[BLACK], x);
      const int attack_index_x =
	indexX<true>(king_piece[WHITE], x);
      const int defense_index_x =
	indexX<false>(king_piece[BLACK], x);
      const int attack_index_y = indexY<WHITE>(king_piece[WHITE], x);
      const int defense_index_y = indexY<BLACK>(king_piece[BLACK], x);
      result += value(attack_index, defense_index,
		      attack_index_y, defense_index_y,
		      attack_index_x, defense_index_x);
      if (has_pawn[BLACK])
      {
	result += standValue(attack_index, defense_index,
			     attack_index_y, defense_index_y,
			     attack_index_x, defense_index_x);
      }
    }
    if (!white_on_board)
    {
      const int attack_index = index(kings[BLACK], x);
      const int defense_index = index(kings[WHITE], x);
      const int attack_index_x =
	indexX<true>(king_piece[BLACK], x);
      const int defense_index_x =
	indexX<false>(king_piece[WHITE], x);
      const int attack_index_y = indexY<BLACK>(king_piece[BLACK], x);
      const int defense_index_y = indexY<WHITE>(king_piece[WHITE], x);
      result -= value(attack_index, defense_index,
		      attack_index_y, defense_index_y,
		      attack_index_x, defense_index_x);
      if (has_pawn[WHITE])
      {
	result -= standValue(attack_index, defense_index,
			     attack_index_y, defense_index_y,
			     attack_index_x, defense_index_x);
      }
    }
    const int index_x = (x > 5 ? 10 - x : x);
    if (black_on_board && white_on_board)
    {
      result +=
	state_king_relative_table[std::abs(kings[BLACK].x() - x) +
				  BOTH_ON_BOARD * 9];
      result -=
	state_king_relative_table[std::abs(kings[WHITE].x() - x) +
				  BOTH_ON_BOARD * 9];
    }
    else if (black_on_board && !white_on_board)
    {
      result += drop_non_drop_table[index_x - 1];
      result -= drop_non_drop_table[index_x - 1 + 5];
      result +=
	state_king_relative_table[std::abs(kings[BLACK].x() - x) +
				  SELF_ON_BOARD * 9];
      result -=
	state_king_relative_table[std::abs(kings[WHITE].x() - x) +
				  OPP_ON_BOARD * 9];
    }
    else if (!black_on_board && white_on_board)
    {
      result += drop_non_drop_table[index_x - 1 + 5];
      result -= drop_non_drop_table[index_x - 1];
      result +=
	state_king_relative_table[std::abs(kings[BLACK].x() - x) +
				  OPP_ON_BOARD * 9];
      result -=
	state_king_relative_table[std::abs(kings[WHITE].x() - x) +
				  SELF_ON_BOARD * 9];
    }
    else
    {
      result +=
	state_king_relative_table[std::abs(kings[BLACK].x() - x) +
				  BOTH_ON_STAND * 9];
      result -=
	state_king_relative_table[std::abs(kings[WHITE].x() - x) +
				  BOTH_ON_STAND * 9];
    }
  }
  return result;
}



osl::MultiInt osl::eval::ml::NoPawnOnStand::weight;

void osl::eval::ml::
NoPawnOnStand::setUp(const Weights &weights,int stage)
{
  weight[stage] = weights.value(0);
}



osl::CArray<osl::MultiInt, 9> osl::eval::ml::PawnAdvance::table;

void osl::eval::ml::
PawnAdvance::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i) {
    table[i][stage] = weights.value(i);
  }
}

osl::MultiInt osl::eval::ml::PawnAdvance::eval(
  const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted() &&
	cantAdvance(state, pawn))
    {
      if (pawn.owner() == BLACK)
	result += table[index(BLACK, pawn.square())];
      else
	result -= table[index(WHITE, pawn.square())];
    }
  }
  return result;
}

template <osl::Player P> inline
void osl::eval::ml::
PawnAdvanceAll::adjust(int index, MultiInt& values)
{
  if(P==BLACK)
    values += PawnAdvance::table[index];
  else
    values -= PawnAdvance::table[index];
}

template <osl::Player P>
void osl::eval::ml::
PawnAdvanceAll::evalWithUpdateBang(const NumEffectState &state,
				   osl::Move moved, MultiInt& values)
{
  assert(moved.player() == P);
  if (moved.ptype() == PAWN)
  {
    if (cantAdvance(state, moved.ptypeO(), moved.to()))
    {
      adjust<P>(index(P, moved.to()), values);
      return;
    }
  }
  const Player Opponent = alt(P);
  Ptype captured = moved.capturePtype();
  if (captured == PAWN)
  {
    if (cantAdvance(state, moved.capturePtypeO(), moved.to()))
      adjust<P>(index(Opponent, moved.to()), values);
  }
  else if (captured != PTYPE_EMPTY)
  {
    const Piece piece = state.pieceAt(
      moved.to() + DirectionPlayerTraits<D, Opponent>::offset());
    if (piece.isPlayerPtype(Opponent,PAWN))
      adjust<P>(index(Opponent, piece.square()), values);
  }
  if (!moved.isDrop())
  {
    const Piece piece = state.pieceAt(
      moved.from() + DirectionPlayerTraits<D, P>::offset());
    if (piece.isPlayerPtype(P,PAWN))
      adjust<Opponent>(index(P, piece.square()), values);
  }
  {
    const Piece piece = state.pieceAt(
      moved.to()+DirectionPlayerTraits<D,P>::offset());
    if (piece.isPlayerPtype(P,PAWN))
      adjust<P>(index(P, piece.square()), values);
  }
}



osl::CArray<osl::MultiInt, 153>
osl::eval::ml::SilverFeatures::head_table;
osl::CArray<osl::MultiInt, 9>
osl::eval::ml::SilverFeatures::retreat_table;

void osl::eval::ml::
SilverHeadPawnKingRelative::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      head_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
SilverFeatures::eval(const NumEffectState &state)
{
  MultiInt result;
  CArray<Square, 2> kings = {{state.kingSquare<BLACK>(),
				state.kingSquare<WHITE>()}};
  for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit; ++i)
  {
    const Piece silver = state.pieceOf(i);
    if (!silver.isOnBoard() || silver.isPromoted()) continue;
    if (silver.owner()==BLACK){
      result += evalOne<BLACK>(state, silver, kings);
    }
    else{
      result -= evalOne<WHITE>(state, silver, kings);
    }
  }
  return result;
}

template<osl::Player P>
inline
bool osl::eval::ml::
SilverFeatures::canRetreat(const NumEffectState &state,
			   const osl::Piece silver)
{
  assert(silver.isOnBoard() && !silver.isPromoted());
  assert(silver.owner()==P);
  if ((P == BLACK && silver.square().y() != 9) ||
      (P == WHITE && silver.square().y() != 1))
  {
    Square dl = silver.square()+DirectionPlayerTraits<DL,P>::offset();
    Piece pdl = state.pieceAt(dl);
    if (!pdl.canMoveOn<P>() ||
	state.hasEffectAt(alt(P), dl))
    {
    Square dr = silver.square()+DirectionPlayerTraits<DR,P>::offset();
      Piece pdr = state.pieceAt(dr);
      if (!pdr.canMoveOn<P>() ||
	  state.hasEffectAt(alt(P), dr))
      {
	return false;
      }
    }    
  }
  return true;
}

void osl::eval::ml::
SilverRetreat::setUp(const Weights &weights, int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i) {
    retreat_table[i][stage] = weights.value(i);
  }
}


osl::CArray<osl::MultiInt, 153>
osl::eval::ml::GoldFeatures::knight_table;
osl::CArray<osl::MultiInt, 9>
osl::eval::ml::GoldFeatures::retreat_table;
osl::CArray<osl::MultiInt, 14>
osl::eval::ml::GoldFeatures::side_table;

void osl::eval::ml::
GoldKnightKingRelative::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      knight_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
GoldSideMove::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      side_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::MultiInt osl::eval::ml::
GoldFeatures::eval(const NumEffectState &state)
{
  MultiInt result;
  CArray<Square, 2> kings = {{state.kingSquare<BLACK>(),
				state.kingSquare<WHITE>()}};
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    const Piece gold = state.pieceOf(i);
    if (!gold.isOnBoard())
      continue;
    if (gold.owner() == BLACK)
    {
      result += evalOne<BLACK>(state, gold, kings);
    }
    else
    {
      result -= evalOne<WHITE>(state, gold, kings);
    }
  }
  return result;
}

template<osl::Player P>
inline
bool osl::eval::ml::
GoldFeatures::canRetreat(const NumEffectState &state,
				 const osl::Piece gold)
{
  assert(gold.isOnBoard());
  assert(P==gold.owner());

  if ((P == BLACK && gold.square().y() != 9) ||
      (P == WHITE && gold.square().y() != 1))
  {
    Square d = gold.square()+DirectionPlayerTraits<D,P>::offset();
    if ((state.pieceAt(d).isOnBoardByOwner(P) ||
	 state.hasEffectAt(alt(P), d)))
    {
      return false;
    }
  }
  return true;
}

void osl::eval::ml::
GoldRetreat::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i) {
    retreat_table[i][stage] = weights.value(i);
  }
}



osl::CArray<MultiInt, 9> osl::eval::ml::KnightAdvance::table;

template<osl::Player P>
inline
bool osl::eval::ml::
KnightAdvance::cantAdvance(const NumEffectState &state,
				  const osl::Piece knight)
{
  // knight が敵陣一段目にいないと仮定
  // もしいる場合はSquare(1,1)のUURが駒台に衝突
  assert(P==knight.owner());
  Square uul = knight.square()+DirectionPlayerTraits<UUL,P>::offset();
  const Piece puul = state.pieceAt(uul);
  if (!puul.canMoveOn<P>())
  {
    Square uur = knight.square()+DirectionPlayerTraits<UUR,P>::offset();
    const Piece puur = state.pieceAt(uur);
    if (!puur.canMoveOn<P>())
      return true;
  }
  return false;
}

void osl::eval::ml::
KnightAdvance::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i) {
    table[i][stage] = weights.value(i);
  }
}

MultiInt osl::eval::ml::KnightAdvance::eval(
  const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<KNIGHT>::indexMin;
       i < PtypeTraits<KNIGHT>::indexLimit; ++i)
  {
    const Piece knight = state.pieceOf(i);
    if (!knight.isOnBoard() || knight.isPromoted()) continue;
    if (knight.owner() == BLACK){
      if(cantAdvance<BLACK>(state,knight))
	result += table[index(BLACK, knight.square())];
    }
    else if(cantAdvance<WHITE>(state,knight)){
      result -= table[index(WHITE, knight.square())];
    }
  }
  return result;
}


MultiInt osl::eval::ml::AllGold::weight;

void osl::eval::ml::
AllGold::setUp(const Weights &weights,int stage)
{
  weight[stage] = weights.value(0);
}



osl::CArray<MultiInt, 144> osl::eval::ml::PtypeY::table;

void osl::eval::ml::
PtypeY::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i][stage] = weights.value(i);
  }
}

MultiInt osl::eval::ml::PtypeY::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    if (p.owner() == BLACK)
      result += table[index(BLACK,p.ptype(),p.square())];
    else
      result -= table[index(WHITE,p.ptype(),p.square())];
  }
  return result;
}

template<osl::Player P>
MultiInt osl::eval::ml::
PtypeY::evalWithUpdate(const NumEffectState &, Move moved,
		       MultiInt const& last_value)
{
  MultiInt result(last_value);

  if (!moved.isDrop())
  {
    if (P == BLACK)
      result -= table[index(BLACK, moved.oldPtype(), moved.from())];
    else
      result += table[index(WHITE, moved.oldPtype(), moved.from())];
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const MultiInt weight =
      table[index(alt(P), captured, moved.to())];
    if (P == BLACK)
      result += weight;
    else
      result -= weight;
  }
  {
    if (P == BLACK)
      result += table[index(BLACK, moved.ptype(), moved.to())];
    else
      result -= table[index(WHITE, moved.ptype(), moved.to())];
  }

  return result;
}


osl::CArray<MultiInt, 80> osl::eval::ml::PtypeX::table;

void osl::eval::ml::
PtypeX::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i][stage] = weights.value(i);
  }
}

MultiInt osl::eval::ml::PtypeX::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    if (p.owner() == BLACK)
      result += table[index(BLACK,p.ptype(),p.square())];
    else
      result -= table[index(WHITE,p.ptype(),p.square())];
  }
  return result;
}

template<osl::Player P>
MultiInt osl::eval::ml::
PtypeX::evalWithUpdate(const NumEffectState &, Move moved,
		       MultiInt const& last_value)
{
  MultiInt result(last_value);

  if (!moved.isDrop())
    {
      if (P == BLACK)
	result -= table[index(BLACK, moved.oldPtype(), moved.from())];
      else
	result += table[index(WHITE, moved.oldPtype(), moved.from())];
      Ptype captured = moved.capturePtype();
      if (captured != PTYPE_EMPTY)
	{
	  if (P == BLACK)
	    result += table[index(WHITE, captured, moved.to())];
	  else
	    result -= table[index(BLACK, captured, moved.to())];
	}
    }
  if (P == BLACK)
    result += table[index(BLACK, moved.ptype(), moved.to())];
  else
    result -= table[index(WHITE, moved.ptype(), moved.to())];
  return result;
}


MultiInt osl::eval::ml::KnightCheck::weight;
osl::CArray<MultiInt, 9> osl::eval::ml::KnightCheck::y_table;

void osl::eval::ml::KnightCheck::setUp(const Weights &weights,int stage)
{
  KnightCheck::weight[stage] = weights.value(0);
}

void osl::eval::ml::
KnightCheckY::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      KnightCheck::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

MultiInt osl::eval::ml::
KnightCheck::eval(const NumEffectState &state)
{
  MultiInt result;
  if (canCheck<BLACK>(state))
  {
    const int index_y = indexY<BLACK>(state.kingSquare<BLACK>().y());
    result += value(index_y);
  }
  if (canCheck<WHITE>(state))
  {
    const int index_y = indexY<WHITE>(state.kingSquare<WHITE>().y());
    result -= value(index_y);
  }
  return result;
}

osl::CArray<MultiInt, 1024> osl::eval::ml::PawnPtypeOPtypeO::table;
osl::CArray<MultiInt, 9216> osl::eval::ml::PawnPtypeOPtypeO::y_table;

void osl::eval::ml::
PawnPtypeOPtypeO::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
PawnPtypeOPtypeOY::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PawnPtypeOPtypeO::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

MultiInt osl::eval::ml::
PawnPtypeOPtypeO::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted())
    {
      const Square up = Board_Table.nextSquare(pawn.owner(),
						   pawn.square(), U);
      const Square up_up = Board_Table.nextSquare(pawn.owner(),
						      up, U);
      PtypeO up_p =
	(up.isOnBoard() ? state.pieceAt(up).ptypeO() : PTYPEO_EDGE);
      PtypeO up_up_p =
	(up_up.isOnBoard() ? state.pieceAt(up_up).ptypeO() : PTYPEO_EDGE);
      const int idx = index(pawn.owner(), up_p, up_up_p);
      const int idx_y = indexY(pawn.owner(), up_p, up_up_p,
			       pawn.square().y());
      if (pawn.owner() == BLACK)
	result += table[idx] + y_table[idx_y];
      else
	result -= table[idx] + y_table[idx_y];
    }
  }
  return result;
}

template<osl::Player P>
MultiInt
#if (defined __GNUC__ && ! defined __clang__)
    __attribute__((__flatten__))
#endif
 osl::eval::ml::
PawnPtypeOPtypeO::evalWithUpdate(const NumEffectState &state, Move moved,
				 const CArray2d<int, 2, 9> &pawns,
				 const MultiInt &last_value)
{
  assert(moved.player()==P);
  MultiInt result(last_value);
  if (!moved.isDrop())
  {
    if (moved.oldPtype() == PAWN)
    {
      const Square up_up = moved.to() + DirectionPlayerTraits<U,P>::offset();
      const PtypeO up_up_p =
	(up_up.isOnBoard() ? state.pieceAt(up_up).ptypeO() : PTYPEO_EDGE);
      const int i = index(P, moved.capturePtypeOSafe(), up_up_p);
      const int i_y = indexY(P, moved.capturePtypeOSafe(),
			     up_up_p, moved.from().y());
      if (P == BLACK)
	result -= table[i]+y_table[i_y];
      else
	result += table[i]+y_table[i_y];
    }
    if (pawns[BLACK][moved.from().x() - 1] != 0)
    {
      if (pawns[BLACK][moved.from().x() - 1] ==
	  moved.from().y() + 1)
      {
	const Square up_up = moved.from() + DirectionPlayerTraits<U,BLACK>::offset();
	const PtypeO up_up_p =
	  (up_up.isOnBoard() ? (up_up == moved.to() ? moved.capturePtypeOSafe() :
				state.pieceAt(up_up).ptypeO()) :
	   PTYPEO_EDGE);
	const int i = index(BLACK, moved.oldPtypeO(), up_up_p);
	const int i_y = indexY(BLACK, moved.oldPtypeO(), up_up_p,
			      moved.from().y() + 1);
	result -= table[i]+y_table[i_y];
	if (up_up != moved.to())
	{
	  const int new_i = index(BLACK, PTYPEO_EMPTY, up_up_p);
	  const int new_i_y = indexY(BLACK, PTYPEO_EMPTY, up_up_p,
				     moved.from().y() + 1);
	  result += table[new_i]+y_table[new_i_y];
	}
      }
      if (pawns[BLACK][moved.from().x() - 1] ==
	  moved.from().y() + 2)
      {
	const Square up = moved.from() + DirectionPlayerTraits<D,BLACK>::offset();
	const PtypeO up_p =
	  (up.isOnBoard() ? (up == moved.to() ? moved.capturePtypeOSafe() :
			     state.pieceAt(up).ptypeO()) : PTYPEO_EDGE);
	const int i = index(BLACK, up_p, moved.oldPtypeO());
	const int i_y = indexY(BLACK, up_p, moved.oldPtypeO(),
			       moved.from().y() + 2);
	result -= table[i]+y_table[i_y];
	if (moved.to() != up)
	{
	  const int new_i = index(BLACK, up_p, PTYPEO_EMPTY);
	  const int new_i_y = indexY(BLACK, up_p, PTYPEO_EMPTY,
				     moved.from().y() + 2);
	  result += table[new_i]+y_table[new_i_y];
	}
      }
    }
    if (pawns[WHITE][moved.from().x() - 1] != 0)
    {
      if (pawns[WHITE][moved.from().x() - 1] ==
	  moved.from().y() - 1)
      {
	const Square up_up = moved.from() + DirectionPlayerTraits<U,WHITE>::offset(); 	
	const PtypeO up_up_p =
	  (up_up.isOnBoard() ? (up_up == moved.to() ? moved.capturePtypeOSafe() :
				state.pieceAt(up_up).ptypeO()) :
	   PTYPEO_EDGE);
	const int i = index(WHITE, moved.oldPtypeO(), up_up_p);
	const int i_y = indexY(WHITE, moved.oldPtypeO(), up_up_p,
			       moved.from().y() - 1);
	result += table[i]+y_table[i_y];
	if (moved.to() != up_up)
	{
	  const int new_i = index(WHITE, PTYPEO_EMPTY, up_up_p);
	  const int new_i_y = indexY(WHITE, PTYPEO_EMPTY, up_up_p,
				     moved.from().y() - 1);
	  result -= table[new_i]+y_table[new_i_y];
	}
      }
      if (pawns[WHITE][moved.from().x() - 1] ==
	  moved.from().y() - 2)
      {
	const Square up = moved.from() + DirectionPlayerTraits<D,WHITE>::offset(); 	
	const PtypeO up_p =
	  (up.isOnBoard() ? (up == moved.to() ? moved.capturePtypeOSafe() :
			     state.pieceAt(up).ptypeO()) : PTYPEO_EDGE);
	const int i = index(WHITE, up_p, moved.oldPtypeO());
	const int i_y = indexY(WHITE, up_p, moved.oldPtypeO(),
			       moved.from().y() - 2);
	result += table[i]+y_table[i_y];
	if (moved.to() != up)
	{
	  const int new_i = index(WHITE, up_p, PTYPEO_EMPTY);
	  const int new_i_y = indexY(WHITE, up_p, PTYPEO_EMPTY,
				     moved.from().y() - 2);
	  result -= table[new_i]+y_table[new_i_y];
	}
      }
    }
  }
  Ptype captured = moved.capturePtype();
  if (captured == PAWN)
  {
    const Square up = moved.to() + DirectionPlayerTraits<D,P>::offset(); 	
    const Square up_up = up + DirectionPlayerTraits<D,P>::offset(); 	
    const PtypeO up_p =
      (up.isOnBoard() ? (up == moved.from() ? moved.oldPtypeO() :
			 state.pieceAt(up).ptypeO()) : PTYPEO_EDGE);
    const PtypeO up_up_p =
      (up_up.isOnBoard() ? (up_up == moved.from() ? moved.oldPtypeO() :
			    state.pieceAt(up_up).ptypeO()) : PTYPEO_EDGE);
    const int i = index(alt(P), up_p, up_up_p);
    const int i_y = indexY(alt(P), up_p, up_up_p,
			   moved.to().y());
    if (P == BLACK)
    {
      result += table[i]+y_table[i_y];
    }
    else
    {
      result -= table[i]+y_table[i_y];
    }
  }
  if (moved.ptype() == PAWN)
  {
    const Square up = moved.to() + DirectionPlayerTraits<U,P>::offset();
    const Square up_up = up + DirectionPlayerTraits<U,P>::offset();
    const PtypeO up_p =
      (up.isOnBoard() ? (up == moved.from() ? moved.oldPtypeO() :
			 state.pieceAt(up).ptypeO()) : PTYPEO_EDGE);
    const PtypeO up_up_p =
      (up_up.isOnBoard() ? (up_up == moved.from() ? moved.oldPtypeO() :
			    state.pieceAt(up_up).ptypeO()) : PTYPEO_EDGE);
    const int i = index(P, up_p, up_up_p);
    const int i_y = indexY(P, up_p, up_up_p, moved.to().y());
    if (P == BLACK)
    {
      result += table[i]+y_table[i_y];
    }
    else
    {
      result -= table[i]+y_table[i_y];
    }
  }
  if (pawns[BLACK][moved.to().x() - 1] != 0)
  {
    if (pawns[BLACK][moved.to().x() - 1] ==
	moved.to().y() + 1)
    {
      const Square up_up = moved.to() + DirectionPlayerTraits<U,BLACK>::offset();
      const PtypeO up_up_p =
	(up_up.isOnBoard() ? state.pieceAt(up_up).ptypeO() :
	 PTYPEO_EDGE);
      const int i = index(BLACK, moved.ptypeO(), up_up_p);
      const int i_y = indexY(BLACK, moved.ptypeO(), up_up_p,
			     moved.to().y() + 1);
      result += table[i]+y_table[i_y];
      if (moved.isDrop() || moved.from() != up_up)
      {
	const int old_i = index(BLACK, moved.capturePtypeOSafe(), up_up_p);
	const int old_i_y = indexY(BLACK, moved.capturePtypeOSafe(),
				   up_up_p, moved.to().y() + 1);
	result -= table[old_i]+y_table[old_i_y];
      }
    }
    if (pawns[BLACK][moved.to().x() - 1] ==
	moved.to().y() + 2)
    {
      const Square up = moved.to() + DirectionPlayerTraits<D,BLACK>::offset();
      const PtypeO up_p =
	(up.isOnBoard() ? state.pieceAt(up).ptypeO() : PTYPEO_EDGE);
      const int i = index(BLACK, up_p, moved.ptypeO());
      const int i_y = indexY(BLACK, up_p, moved.ptypeO(), moved.to().y() + 2);
      result += table[i]+y_table[i_y];
      if (moved.isDrop() || up != moved.from())
      {
	const int old_i = index(BLACK, up_p, moved.capturePtypeOSafe());
	const int old_i_y = indexY(BLACK, up_p, moved.capturePtypeOSafe(),
				   moved.to().y() + 2);
	result -= table[old_i]+y_table[old_i_y];
      }
    }
  }
  if (pawns[WHITE][moved.to().x() - 1] != 0)
  {
    if (pawns[WHITE][moved.to().x() - 1] ==
	moved.to().y() - 1)
    {
      const Square up_up = moved.to() + DirectionPlayerTraits<U,WHITE>::offset();
      const PtypeO up_up_p =
	(up_up.isOnBoard() ? state.pieceAt(up_up).ptypeO() :
	 PTYPEO_EDGE);
      const int i = index(WHITE, moved.ptypeO(), up_up_p);
      const int i_y = indexY(WHITE, moved.ptypeO(), up_up_p,
			     moved.to().y() - 1);
      result -= table[i]+y_table[i_y];
      if (up_up != moved.from())
      {
	const int old_i = index(WHITE, moved.capturePtypeOSafe(), up_up_p);
	const int old_i_y = indexY(WHITE, moved.capturePtypeOSafe(), up_up_p,
				   moved.to().y() - 1);
	result += table[old_i]+y_table[old_i_y];
      }
    }
    if (pawns[WHITE][moved.to().x() - 1] ==
	moved.to().y() - 2)
    {
      const Square up = moved.to() + DirectionPlayerTraits<D,WHITE>::offset();
      const PtypeO up_p =
	(up.isOnBoard() ? state.pieceAt(up).ptypeO() : PTYPEO_EDGE);
      const int i = index(WHITE, up_p, moved.ptypeO());
      const int i_y = indexY(WHITE, up_p, moved.ptypeO(), moved.to().y() - 2);
      result -= table[i]+y_table[i_y];
      if (moved.isDrop() || up != moved.from())
      {
	const int old_i = index(WHITE, up_p, moved.capturePtypeOSafe());
	const int old_i_y = indexY(WHITE, up_p, moved.capturePtypeOSafe(),
				   moved.to().y() - 2);
	result += table[old_i]+y_table[old_i_y];
      }
    }
  }
  return result;
}



osl::CArray<MultiInt, 9> osl::eval::ml::PromotedMinorPieces::table;
osl::CArray<MultiInt, 162> osl::eval::ml::PromotedMinorPieces::y_table;

void osl::eval::ml::
PromotedMinorPieces::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
PromotedMinorPiecesY::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PromotedMinorPieces::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <int Sign>
inline void osl::eval::ml::
PromotedMinorPieces::adjust(int index, int index_attack, int index_defense,
			    MultiInt &result)
{
  if(Sign>0)
    result+= table[index] + y_table[index_attack] + y_table[index_defense];
  else
    result-= table[index] + y_table[index_attack] + y_table[index_defense];
}
template <osl::Player P>
void osl::eval::ml::
PromotedMinorPieces::evalOne(const NumEffectState &state,
			     const PieceMask promoted,
			     MultiInt &result)
{
  PieceMask attack = promoted & state.piecesOnBoard(P);
  const Square king = state.kingSquare<alt(P)>();
  const Square self_king = state.kingSquare<P>();
  int min_left = -10;
  int min_right = 10;
  while (attack.any())
  {
    const Piece p = state.pieceOf(attack.takeOneBit());
    const int x_diff = (P == BLACK ? p.square().x() - king.x() :
			king.x() - p.square().x());
    if (x_diff <= 0)
    {
      if (x_diff > min_left)
      {
	if (min_left != -10)
	{
	  if (P == BLACK)
	    adjust<1>(-min_left, indexY<true, P>(king, -min_left),
		      indexY<false, P>(self_king, -min_left), result);
	  else
	    adjust<-1>(-min_left, indexY<true, P>(king, -min_left),
		       indexY<false, P>(self_king, -min_left), result);
	}
	min_left = x_diff;
      }
      else
      {
	if (P == BLACK)
	  adjust<1>(-x_diff, indexY<true, P>(king, -x_diff),
		    indexY<false, P>(self_king, -x_diff),
		    result);
	else
	  adjust<-1>(-x_diff, indexY<true, P>(king, -x_diff),
		     indexY<false, P>(self_king, -x_diff),
		     result);
      }
    }
    if (x_diff >= 0)
    {
      if (x_diff < min_right)
      {
	if (min_right != 10)
	{
	  if (P == BLACK)
	    adjust<1>(min_right, indexY<true, P>(king, min_right),
		      indexY<false, P>(self_king, min_right),
		      result);
	  else
	    adjust<-1>(min_right, indexY<true, P>(king, min_right),
		       indexY<false, P>(self_king, min_right),
		       result);
	}
	min_right = x_diff;
      }
      else if (x_diff != 0)
      {
	if (P == BLACK)
	  adjust<1>(x_diff, indexY<true, P>(king, x_diff),
		    indexY<false, P>(self_king, x_diff),
		    result);
	else
	  adjust<-1>(x_diff, indexY<true, P>(king, x_diff),
		     indexY<false, P>(self_king, x_diff),
		     result);
      }
    }
  }
}

MultiInt osl::eval::ml::
PromotedMinorPieces::eval(const NumEffectState &state)
{
  MultiInt result;
  PieceMask promoted_pieces = state.promotedPieces();
  promoted_pieces.clearBit<ROOK>();
  promoted_pieces.clearBit<BISHOP>();
  if (promoted_pieces.none())
    return result;

  evalOne<BLACK>(state, promoted_pieces, result);
  evalOne<WHITE>(state, promoted_pieces, result);
  return result;
}

MultiInt osl::eval::ml::
PromotedMinorPieces::evalWithUpdate(const NumEffectState &state,
				    Move moved,
				    const MultiInt &last_values)
{
  Ptype captured = moved.capturePtype();
  if (moved.ptype() == KING ||
      (isPromoted(moved.ptype()) && !isMajor(moved.ptype())) ||
      (captured != PTYPE_EMPTY && isPromoted(captured) &&
       !isMajor(captured)))
    return eval(state);

  return last_values;
}


osl::CArray<MultiInt, 64> osl::eval::ml::NonPawnAttacked::table;
osl::CArray<MultiInt, 19584> osl::eval::ml::NonPawnAttacked::king_table;

void osl::eval::ml::NonPawnAttacked::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::NonPawnAttackedKingRelative::setUp(
  const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      NonPawnAttacked::king_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int x_diff=0;x_diff<9;x_diff++)
    for(int y_diff= -8;y_diff<=8;y_diff++)
      for(int has_support=0;has_support<2;has_support++)
	for(int same_turn=0;same_turn<2;same_turn++)
	  for(int ptype=0;ptype<PTYPE_SIZE;ptype++){
	    int index=((ptype + (same_turn ? 0 : PTYPE_SIZE) +
			(has_support ? 0 : PTYPE_SIZE*2))* 9 + x_diff) * 17 +
	      y_diff + 8;
	    int index0=ptype + (same_turn ? 0 : PTYPE_SIZE) +
	      (has_support ? 0 : PTYPE_SIZE * 2);
	    NonPawnAttacked::king_table[index] += NonPawnAttacked::table[index0];
	  }
}

template <int Sign>
void osl::eval::ml::
NonPawnAttacked::adjust(int black_turn_king_attack,
			int black_turn_king_defense,
			int white_turn_king_attack,
			int white_turn_king_defense,
			MultiIntPair &result)
{
  if(Sign>0){
    result[BLACK] += king_table[black_turn_king_attack] +
      king_table[black_turn_king_defense];
    result[WHITE] += king_table[white_turn_king_attack] +
      king_table[white_turn_king_defense];
  }
  else{
    result[BLACK] -= king_table[black_turn_king_attack] +
      king_table[black_turn_king_defense];
    result[WHITE] -= king_table[white_turn_king_attack] +
      king_table[white_turn_king_defense];
  }
}

void osl::eval::ml::
NonPawnAttacked::eval(const NumEffectState &state, MultiIntPair& result)
{
  result = MultiIntPair();
  CArray<Square, 2> kings = {{state.kingSquare<BLACK>(),
				state.kingSquare<WHITE>()}};
  PieceMask black_attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
  black_attacked.reset(KingTraits<BLACK>::index);
  mask_t black_ppawn = state.promotedPieces().getMask<PAWN>() & black_attacked.selectBit<PAWN>();
  black_attacked.clearBit<PAWN>();
  black_attacked.orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  PieceMask black_with_support = state.effectedMask(BLACK) & black_attacked;
  PieceMask black_without_support = (~state.effectedMask(BLACK)) & black_attacked;
  while (black_with_support.any())
  {
    const Piece piece = state.pieceOf(black_with_support.takeOneBit());
    const int index_king_black_turn_attack =
      indexK<true>(kings[WHITE], true, true, piece);
    const int index_king_white_turn_attack =
      indexK<true>(kings[WHITE], false, true, piece);
    const int index_king_black_turn_defense =
      indexK<false>(kings[BLACK], true, true, piece);
    const int index_king_white_turn_defense =
      indexK<false>(kings[BLACK], false, true, piece);
    adjust<1>(index_king_black_turn_attack, index_king_black_turn_defense,
	      index_king_white_turn_attack, index_king_white_turn_defense,
	      result);
  }
  while (black_without_support.any())
  {
    const Piece piece = state.pieceOf(black_without_support.takeOneBit());
    const int index_king_black_turn_attack =
      indexK<true>(kings[WHITE], true, false, piece);
    const int index_king_white_turn_attack =
      indexK<true>(kings[WHITE], false, false, piece);
    const int index_king_black_turn_defense =
      indexK<false>(kings[BLACK], true, false, piece);
    const int index_king_white_turn_defense =
      indexK<false>(kings[BLACK], false, false, piece);
    adjust<1>(index_king_black_turn_attack, index_king_black_turn_defense,
	      index_king_white_turn_attack, index_king_white_turn_defense,
	      result);
  }

  PieceMask white_attacked = state.effectedMask(BLACK) & state.piecesOnBoard(WHITE);
  white_attacked.reset(KingTraits<WHITE>::index);
  mask_t white_ppawn = state.promotedPieces().getMask<PAWN>() & white_attacked.selectBit<PAWN>();
  white_attacked.clearBit<PAWN>();
  white_attacked.orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
  PieceMask white_with_support = state.effectedMask(WHITE) & white_attacked;
  PieceMask white_without_support = (~state.effectedMask(WHITE)) & white_attacked;
  while (white_with_support.any())
  {
    const Piece piece = state.pieceOf(white_with_support.takeOneBit());
    const int index_king_black_turn_attack =
      indexK<true>(kings[BLACK], false, true, piece);
    const int index_king_white_turn_attack =
      indexK<true>(kings[BLACK], true, true, piece);
    const int index_king_black_turn_defense =
      indexK<false>(kings[WHITE], false, true, piece);
    const int index_king_white_turn_defense =
      indexK<false>(kings[WHITE], true, true, piece);
    adjust<-1>(index_king_black_turn_attack, index_king_black_turn_defense,
	       index_king_white_turn_attack, index_king_white_turn_defense,
	       result);
  }
  while (white_without_support.any())
  {
    const Piece piece = state.pieceOf(white_without_support.takeOneBit());
    const int index_king_black_turn_attack =
      indexK<true>(kings[BLACK], false, false, piece);
    const int index_king_white_turn_attack =
      indexK<true>(kings[BLACK], true, false, piece);
    const int index_king_black_turn_defense =
      indexK<false>(kings[WHITE], false, false, piece);
    const int index_king_white_turn_defense =
      indexK<false>(kings[WHITE], true, false, piece);
    adjust<-1>(index_king_black_turn_attack, index_king_black_turn_defense,
	       index_king_white_turn_attack, index_king_white_turn_defense,
	       result);
  }
}

template<osl::Player P>
void osl::eval::ml::
NonPawnAttacked::evalWithUpdateBang(
  const NumEffectState &state,
  Move moved,
  const CArray<PieceMask, 2> &effected,
  MultiIntPair &result)
{
  if (moved.ptype() == KING)
  {
    eval(state, result);
    return;
  }

  CArray<PieceMask, 2> effected_mask = effected;
  effected_mask[0].clearBit<KING>();
  effected_mask[1].clearBit<KING>();
  CArray<PieceMask, 2> new_mask = {{
      state.effectedMask(BLACK),
      state.effectedMask(WHITE)
    }};

  mask_t black_ppawn =
    new_mask[0].selectBit<PAWN>() & state.promotedPieces().template getMask<PAWN>();
  mask_t white_ppawn =
    new_mask[1].template selectBit<PAWN>() & state.promotedPieces().template getMask<PAWN>();
  new_mask[0].clearBit<PAWN>();
  new_mask[1].clearBit<PAWN>();
  new_mask[0].orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  new_mask[1].orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
  new_mask[0].clearBit<KING>();
  new_mask[1].clearBit<KING>();
  CArray<Square, 2> kings = {{ state.kingSquare<BLACK>(),
				 state.kingSquare<WHITE>() }};
  const Piece p = state.pieceAt(moved.to());
  assert(p.owner()==P);
  if (!moved.isDrop())
  {
    if (effected_mask[alt(P)].test(p.number()))
    {
      const bool has_support = effected_mask[P].test(p.number());
      const int index_king_black_turn_attack =
	indexK<true>(kings[alt(P)], BLACK == P,
		     has_support, moved.from(), P, moved.oldPtype());
      const int index_king_white_turn_attack =
	indexK<true>(kings[alt(P)], WHITE == P,
		     has_support, moved.from(), P, moved.oldPtype());
      const int index_king_black_turn_defense =
	indexK<false>(kings[P], BLACK == P,
		      has_support, moved.from(), P, moved.oldPtype());
      const int index_king_white_turn_defense =
	indexK<false>(kings[P], WHITE == P,
		      has_support, moved.from(), P, moved.oldPtype());
      if (P == BLACK)
	adjust<-1>(index_king_black_turn_attack, index_king_black_turn_defense,
		   index_king_white_turn_attack, index_king_white_turn_defense,
		   result);
      else
	adjust<1>(index_king_black_turn_attack, index_king_black_turn_defense,
		  index_king_white_turn_attack, index_king_white_turn_defense,
		  result);
    }
  }
  if (new_mask[alt(P)].test(p.number()))
  {
    const bool has_support = new_mask[P].test(p.number());
    const int index_king_black_turn_attack =
      indexK<true>(kings[alt(P)], BLACK == P,
		   has_support, p);
    const int index_king_white_turn_attack =
      indexK<true>(kings[alt(P)], WHITE == P,
		   has_support, p);
    const int index_king_black_turn_defense =
      indexK<false>(kings[P], BLACK == P,
		    has_support, p);
    const int index_king_white_turn_defense =
      indexK<false>(kings[P], WHITE == P,
		    has_support, p);
    if (P == BLACK)
      adjust<1>(index_king_black_turn_attack, index_king_black_turn_defense,
		index_king_white_turn_attack, index_king_white_turn_defense,
		result);
    else
      adjust<-1>(index_king_black_turn_attack, index_king_black_turn_defense,
		 index_king_white_turn_attack, index_king_white_turn_defense,
		 result);
  }
  const Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY && captured != PAWN)
  {
    PieceMask captured_mask =
      effected_mask[P] & (~state.piecesOnBoard(BLACK)) &
      (~state.piecesOnBoard(WHITE));
    
    const bool has_support = effected_mask[alt(P)].test(captured_mask.takeOneBit());
    const int index_king_black_turn_attack =
      indexK<true>(kings[P], WHITE == P,
		   has_support, moved.to(), alt(P), captured);
    const int index_king_white_turn_attack =
      indexK<true>(kings[P], BLACK == P,
		   has_support, moved.to(), alt(P), captured);
    const int index_king_black_turn_defense =
      indexK<false>(kings[alt(P)], WHITE == P,
		    has_support, moved.to(), alt(P), captured);
    const int index_king_white_turn_defense =
      indexK<false>(kings[alt(P)], BLACK == P,
		    has_support, moved.to(), alt(P), captured);
    if (P == BLACK)
      adjust<1>(index_king_black_turn_attack, index_king_black_turn_defense,
		index_king_white_turn_attack, index_king_white_turn_defense,
		result);
    else
      adjust<-1>(index_king_black_turn_attack, index_king_black_turn_defense,
		 index_king_white_turn_attack, index_king_white_turn_defense,
		 result);
  }

  updateEffectChanged<BLACK>(state, effected_mask, new_mask, p.number(),
			     result);
  updateEffectChanged<WHITE>(state, effected_mask, new_mask, p.number(),
			     result);
}


osl::CArray<MultiInt, 9> osl::eval::ml::KnightHead::table;
osl::CArray<MultiInt, 144> osl::eval::ml::KnightHead::opp_table;

void osl::eval::ml::
KnightHead::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
KnightHeadOppPiecePawnOnStand::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      KnightHead::opp_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

MultiInt osl::eval::ml::
KnightHead::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<KNIGHT>::indexMin;
       i < PtypeTraits<KNIGHT>::indexLimit;
       ++i)
  {
    const Piece knight = state.pieceOf(i);
    if (knight.isOnBoard() && !knight.isPromoted())
    {
      const Square up = Board_Table.nextSquare(knight.owner(),
					       knight.square(), U);
      const Piece up_piece = state.pieceAt(up);
      if ((up_piece.isEmpty() && state.hasPieceOnStand<PAWN>(alt(knight.owner())) &&
	   !state.isPawnMaskSet(alt(knight.owner()), knight.square().x()) &&
	   state.countEffect(knight.owner(), up) <=
	   state.countEffect(alt(knight.owner()), up)) ||
	  (state.hasEffectByPtypeStrict<PAWN>(alt(knight.owner()), up) &&
	   (up_piece.isEmpty() || up_piece.owner() == knight.owner()) &&
	   state.countEffect(knight.owner(), up) <
	   state.countEffect(alt(knight.owner()), up)))
      {
	const int y = knight.square().y();
	if (knight.owner() == BLACK)
	{
	  result += table[y - 1];
	}
	else
	{
	  result -= table[9 - y];
	}
      }
      else if (up_piece.isPiece() && up_piece.owner() != knight.owner() &&
	       state.hasPieceOnStand<PAWN>(up_piece.owner()))
      {
	const int y = (knight.owner() == BLACK ? knight.square().y() :
		       10 - knight.square().y());
	const int index = up_piece.ptype() * 9 + y - 1;
	if (knight.owner() == BLACK)
	{
	  result += opp_table[index];
	}
	else
	{
	  result -= opp_table[index];
	}
      }
    }
  }
  return result;
}


osl::CArray<MultiInt, 1024> osl::eval::ml::NonPawnAttackedPtype::table;

void osl::eval::ml::
NonPawnAttackedPtype::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
NonPawnAttackedPtype::eval(const NumEffectState &state,
			   CArray<PieceMask, 40> &attacked_mask,
			   MultiIntPair &result)
{
  result = MultiIntPair();
  PieceMask black_attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
  black_attacked.reset(KingTraits<BLACK>::index);
  mask_t black_ppawn = state.promotedPieces().getMask<PAWN>() & black_attacked.selectBit<PAWN>();
  black_attacked.clearBit<PAWN>();
  black_attacked.orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  while (black_attacked.any())
  {
    const Piece piece = state.pieceOf(black_attacked.takeOneBit());
    const bool with_support = state.effectedMask(BLACK).test(piece.number());
    PieceMask attacking =
      state.effectSetAt(piece.square()) & state.piecesOnBoard(WHITE);
    attacked_mask[piece.number()] = attacking;
    
    while (attacking.any())
    {
      const Piece attack = state.pieceOf(attacking.takeOneBit());
      const int index_black_turn = index(true, with_support,
					 piece.ptype(), attack.ptype());
      const int index_white_turn = index(false, with_support,
					 piece.ptype(), attack.ptype());
      adjust<1>(index_black_turn, index_white_turn, result);
    }
  }
  PieceMask white_attacked = state.effectedMask(BLACK) & state.piecesOnBoard(WHITE);
  white_attacked.reset(KingTraits<WHITE>::index);
  mask_t white_ppawn = state.promotedPieces().getMask<PAWN>() & white_attacked.selectBit<PAWN>();
  white_attacked.clearBit<PAWN>();
  white_attacked.orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
  while (white_attacked.any())
  {
    const Piece piece = state.pieceOf(white_attacked.takeOneBit());
    const bool with_support = state.effectedMask(WHITE).test(piece.number());
    PieceMask attacking =
      state.effectSetAt(piece.square()) & state.piecesOnBoard(BLACK);
    attacked_mask[piece.number()] = attacking;
    while (attacking.any())
    {
      const Piece attack = state.pieceOf(attacking.takeOneBit());
      const int index_black_turn = index(false, with_support,
					 piece.ptype(), attack.ptype());
      const int index_white_turn = index(true, with_support,
					 piece.ptype(), attack.ptype());
      adjust<-1>(index_black_turn, index_white_turn, result);
    }
  }
}

template<osl::Player P>
void osl::eval::ml::
NonPawnAttackedPtype::evalWithUpdateBang(
  const NumEffectState &state,
  Move moved,
  const CArray<PieceMask, 2> &effected,
  CArray<PieceMask, 40> &attacked_mask,
  MultiIntPair &result)
{
  CArray<PieceMask, 2> effected_mask = effected;
  effected_mask[0].clearBit<KING>();
  effected_mask[1].clearBit<KING>();
  CArray<PieceMask, 2> new_mask = {{
      state.effectedMask(BLACK),
      state.effectedMask(WHITE)
    }};
  mask_t black_ppawn =
    new_mask[0].selectBit<PAWN>() & state.promotedPieces().template getMask<PAWN>();
  mask_t white_ppawn =
    new_mask[1].selectBit<PAWN>() & state.promotedPieces().template getMask<PAWN>();
  new_mask[0].clearBit<PAWN>();
  new_mask[1].clearBit<PAWN>();
  new_mask[0].orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  new_mask[1].orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
  new_mask[0].clearBit<KING>();
  new_mask[1].clearBit<KING>();
  const Piece p = state.pieceAt(moved.to());
  assert(p.owner()==P);
  assert(moved.player()==P);
  const Ptype captured = moved.capturePtype();
  int captured_number = -1;
  if (captured != PTYPE_EMPTY && captured != PAWN)
  {
    PieceMask captured_mask =
      effected_mask[P] & (~state.piecesOnBoard(BLACK)) &
      (~state.piecesOnBoard(WHITE));
    captured_number = captured_mask.takeOneBit();
  }
  if (!moved.isDrop() && moved.oldPtype() != PAWN)
  {
    if (effected_mask[alt(P)].test(p.number()))
    {
      const bool has_support = effected_mask[P].test(p.number());
      PieceMask attacking = attacked_mask[p.number()];
      if (captured_number != -1)
      {
	if (attacking.test(captured_number))
	{
	  if (P == BLACK)
	  {
	    evalOnePiece<false>(P, moved.oldPtype(), captured,
				has_support, result);
	  }
	  else
	  {
	    evalOnePiece<true>(P, moved.oldPtype(), captured,
			       has_support, result);
	  }
	  attacking.reset(captured_number);
	}
      }
      while (attacking.any())
      {
	const Piece attack = state.pieceOf(attacking.takeOneBit());
	if (P == BLACK)
	{
	  evalOnePiece<false>(P, moved.oldPtype(), attack.ptype(),
			      has_support, result);
	}
	else
	{
	  evalOnePiece<true>(P, moved.oldPtype(), attack.ptype(),
			      has_support, result);
	}
      }
    }
  }
  if (new_mask[alt(P)].test(p.number()))
  {
    const bool has_support = new_mask[P].test(p.number());
    PieceMask attacking =
      state.effectSetAt(moved.to()) & state.piecesOnBoard(alt(P));
    attacked_mask[p.number()] = attacking;
    while (attacking.any())
    {
      const Piece attack = state.pieceOf(attacking.takeOneBit());
      if (P == BLACK)
      {
	evalOnePiece<true>(P, p.ptype(), attack.ptype(),
			   has_support, result);
      }
      else
      {
	evalOnePiece<false>(P, p.ptype(), attack.ptype(),
			    has_support, result);
      }
    }
  }
  if (captured_number != -1)
  {
    const bool has_support = effected_mask[alt(P)].test(captured_number);
    PieceMask attacking = attacked_mask[captured_number];
    if (attacking.test(p.number()))
    {
      if (P == BLACK)
      {
	evalOnePiece<true>(alt(P), captured, moved.oldPtype(),
			   has_support, result);
      }
      else
      {
	evalOnePiece<false>(alt(P), captured, moved.oldPtype(),
			   has_support, result);
      }
      attacking.reset(p.number());
    }
    while (attacking.any())
    {
      const Piece attack = state.pieceOf(attacking.takeOneBit());
      if (P == BLACK)
      {
	evalOnePiece<true>(alt(P), captured, attack.ptype(),
			   has_support, result);
      }
      else
      {
	evalOnePiece<false>(alt(P), captured, attack.ptype(),
			   has_support, result);
      }
    }
  }
  updateChanged<BLACK>(state, p, moved, captured_number,
		       effected_mask, new_mask, attacked_mask, result);
  updateChanged<WHITE>(state, p, moved, captured_number,
		       effected_mask, new_mask, attacked_mask, result);
}

osl::CArray<osl::MultiInt, 512*512>
osl::eval::ml::NonPawnAttackedPtypePair::table;
void osl::eval::ml::NonPawnAttackedPtypePair::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for (int i=0; i<PTYPE_SIZE*2*PTYPE_SIZE; ++i)
    for (int j=i+1; j<PTYPE_SIZE*2*PTYPE_SIZE; ++j) {
      table[index2(j,i)] = table[index2(i,j)];
    }
}
template <osl::Player Owner>
osl::MultiInt osl::eval::ml::NonPawnAttackedPtypePair::
evalOne(const NumEffectState &state)
{
  MultiInt result;
  PieceMask attacked = state.effectedMask(alt(Owner)) & state.piecesOnBoard(Owner);
  attacked.reset(state.kingPiece<Owner>().number());
  mask_t ppawn = state.promotedPieces().getMask<PAWN>() & attacked.selectBit<PAWN>();
  attacked.clearBit<PAWN>();
  attacked.orMask(PtypeFuns<PAWN>::indexNum, ppawn);
  PieceVector pieces;
  while (attacked.any())
  {
    const Piece piece = state.pieceOf(attacked.takeOneBit());
    pieces.push_back(piece);
  }
  for (size_t i=0; i+1<pieces.size(); ++i) {
    const int i0 = index1(state, pieces[i]);
    for (size_t j=i+1; j<pieces.size(); ++j) {
      const int i1 = index1(state, pieces[j]);
      if (Owner == BLACK)
	result += table[index2(i0, i1)];
      else
	result -= table[index2(i0, i1)];
    }
  }
  return result;
}

osl::MultiInt osl::eval::ml::NonPawnAttackedPtypePair::
eval(const NumEffectState &state)
{
  return evalOne<BLACK>(state) + evalOne<WHITE>(state);
}


osl::CArray<MultiInt, 160>
osl::eval::ml::PtypeCount::table;
osl::CArray<MultiInt, 2240>
osl::eval::ml::PtypeCount::xy_table;
osl::CArray<MultiInt, 2240>
osl::eval::ml::PtypeCount::xy_attack_table;
osl::CArray<MultiInt, 2240>
osl::eval::ml::PtypeCount::xy_table_diff;
osl::CArray<MultiInt, 2240>
osl::eval::ml::PtypeCount::xy_attack_table_diff;
void osl::eval::ml::PtypeCount::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::PtypeCountXY::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PtypeCount::xy_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int i=PTYPE_BASIC_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    int indexMin=Ptype_Table.getIndexMin(ptype);
    int size=Ptype_Table.getIndexLimit(ptype)-indexMin;
    for(int x=0;x<5;x++){
      for(int j=0;j<size;j++){
	for(int k=0;k<160;k+=40){
	  PtypeCount::xy_table[(indexMin+j+k)*5+x]+=PtypeCount::table[indexMin+j+k];
	}
      }
    }
  }
  for(int i=PTYPE_BASIC_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    int indexMin=Ptype_Table.getIndexMin(ptype);
    int size=Ptype_Table.getIndexLimit(ptype)-indexMin;
    for(int x=0;x<5;x++){
      for(int k=0;k<160;k+=40)
	PtypeCount::xy_table_diff[(indexMin+k)*5+x]=PtypeCount::xy_table[(indexMin+k)*5+x];
      for(int j=1;j<size;j++){
	for(int k=0;k<160;k+=40)
	  PtypeCount::xy_table_diff[(indexMin+k+j)*5+x]=PtypeCount::xy_table[(indexMin+k+j)*5+x]-PtypeCount::xy_table[(indexMin+k+j-1)*5+x];
      }
    }
    for(int y=0;y<9;y++){
      for(int k=0;k<160;k+=40)
	PtypeCount::xy_table_diff[800+(indexMin+k)*9+y]=PtypeCount::xy_table[800+(indexMin+k)*9+y];
      for(int j=1;j<size;j++){
	for(int k=0;k<160;k+=40)
	  PtypeCount::xy_table_diff[800+(indexMin+k+j)*9+y]=PtypeCount::xy_table[800+(indexMin+k+j)*9+y]-PtypeCount::xy_table[800+(indexMin+k+j-1)*9+y];
      }
    }
  }
}

void osl::eval::ml::PtypeCountXYAttack::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      PtypeCount::xy_attack_table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for(int i=PTYPE_BASIC_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    int indexMin=Ptype_Table.getIndexMin(ptype);
    int size=Ptype_Table.getIndexLimit(ptype)-indexMin;
    for(int x=0;x<5;x++){
      for(int k=0;k<160;k+=40)
	PtypeCount::xy_attack_table_diff[(indexMin+k)*5+x]=PtypeCount::xy_attack_table[(indexMin+k)*5+x];
      for(int j=1;j<size;j++){
	for(int k=0;k<160;k+=40)
	  PtypeCount::xy_attack_table_diff[(indexMin+k+j)*5+x]=PtypeCount::xy_attack_table[(indexMin+k+j)*5+x]-PtypeCount::xy_attack_table[(indexMin+k+j-1)*5+x];
      }
    }
    for(int y=0;y<9;y++){
      for(int k=0;k<160;k+=40)
	PtypeCount::xy_attack_table_diff[800+(indexMin+k)*9+y]=PtypeCount::xy_attack_table[800+(indexMin+k)*9+y];
      for(int j=1;j<size;j++){
	for(int k=0;k<160;k+=40)
	  PtypeCount::xy_attack_table_diff[800+(indexMin+k+j)*9+y]=PtypeCount::xy_attack_table[800+(indexMin+k+j)*9+y]-PtypeCount::xy_attack_table[800+(indexMin+k+j-1)*9+y];
      }
    }
  }
}

template<osl::Player P,osl::Ptype T>
MultiInt osl::eval::ml::PtypeCount::
evalPlayerPtype(const osl::CArray2d<int, 2, osl::PTYPE_SIZE> &ptype_count,
		const osl::CArray2d<int, 2, osl::PTYPE_SIZE> &ptype_board_count,
		const osl::CArray<int,2> &kings_x,
		const osl::CArray<int,2> &kings_y)
{
  MultiInt out;
  int i=playerToIndex(P);
  int j=static_cast<int>(T);
  if (ptype_count[i][j] != 0)
  {
    const int index_x = indexCountX<T>(ptype_count[i][j], kings_x[i]);
    const int index_y = indexCountY<T>(ptype_count[i][j], kings_y[i]);
    const int index_x_attack =
      indexCountX<T>(ptype_count[i][j], kings_x[1-i]);
    const int index_y_attack =
      indexCountY<T>(ptype_count[i][j], kings_y[1-i]);
    if (P == BLACK)
    {
      out += xy_table[index_x] + xy_table[index_y];
      out += xy_attack_table[index_x_attack] +
	xy_attack_table[index_y_attack];
    }
    else
    {
      out -= (xy_table[index_x] + xy_table[index_y]);
      out -= (xy_attack_table[index_x_attack] +
	      xy_attack_table[index_y_attack]);
    }
    if (ptype_board_count[i][j] != 0)
    {
      const int index_x =
	indexBoardCountX<T>(ptype_board_count[i][j], kings_x[i]);
      const int index_y =
	indexBoardCountY<T>(ptype_board_count[i][j], kings_y[i]);
      const int index_x_attack =
	indexBoardCountX<T>(ptype_board_count[i][j], kings_x[(i + 1) & 1]);
      const int index_y_attack =
	indexBoardCountY<T>(ptype_board_count[i][j], kings_y[(i + 1) & 1]);
      if (P == BLACK)
      {
	out += xy_table[index_x] + xy_table[index_y];
	out += xy_attack_table[index_x_attack] +
	  xy_attack_table[index_y_attack];
      }
      else
      {
	out -= (xy_table[index_x] + xy_table[index_y]);
	out -= (xy_attack_table[index_x_attack] +
		xy_attack_table[index_y_attack]);
      }
    }
  }
  return out;
}

void
#if (defined __GNUC__ && ! defined __clang__)
    __attribute__((__flatten__))
#endif
 osl::eval::ml::PtypeCount::eval(
  const NumEffectState &state,
  const CArray2d<int, 2, PTYPE_SIZE> &ptype_count,
  const CArray2d<int, 2, PTYPE_SIZE> &ptype_board_count,
  MultiInt &out)
{
  out.clear();
  CArray<int, 2> kings_x = {{ state.kingSquare<BLACK>().x(),
			      state.kingSquare<WHITE>().x() }};
  CArray<int, 2> kings_y = {{ state.kingSquare<BLACK>().y(),
			      10 - state.kingSquare<WHITE>().y() }};
  if (kings_x[0] > 5)
    kings_x[0] = 10 - kings_x[0];
  if (kings_x[1] > 5)
    kings_x[1] = 10 - kings_x[1];
  out = 
    evalPlayerPtype<BLACK,PPAWN>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,PLANCE>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,PKNIGHT>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,PSILVER>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,PBISHOP>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,PROOK>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,GOLD>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,PAWN>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,LANCE>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,KNIGHT>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,SILVER>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,BISHOP>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<BLACK,ROOK>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,PPAWN>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,PLANCE>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,PKNIGHT>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,PSILVER>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,PBISHOP>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,PROOK>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,GOLD>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,PAWN>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,LANCE>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,KNIGHT>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,SILVER>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,BISHOP>(ptype_count,ptype_board_count,kings_x,kings_y)+
    evalPlayerPtype<WHITE,ROOK>(ptype_count,ptype_board_count,kings_x,kings_y);
}

template<osl::Player P>
void osl::eval::ml::PtypeCount::evalWithUpdateBang(
  const NumEffectState &state,
  Move last_move,
  CArray2d<int, 2, PTYPE_SIZE> &ptype_count,
  CArray2d<int, 2, PTYPE_SIZE> &ptype_board_count,
  MultiInt &last_value_and_out,
  unsigned int &ptypeo_mask)
{
  assert(last_move.player()==P);
  const Player altP=alt(P);
  CArray<int, 2> kings_x = {{ state.kingSquare<BLACK>().x(),
			      state.kingSquare<WHITE>().x() }};
  CArray<int, 2> kings_y = {{ state.kingSquare<BLACK>().y(),
			      10 - state.kingSquare<WHITE>().y() }};
  if (kings_x[0] > 5)
    kings_x[0] = 10 - kings_x[0];
  if (kings_x[1] > 5)
    kings_x[1] = 10 - kings_x[1];

  if (last_move.ptype() == KING)
  {
    const Ptype capturedPtype = last_move.capturePtype();
    if (capturedPtype != PTYPE_EMPTY)
    {
      const PtypeO capturedPtypeO = last_move.capturePtypeO();
      if(--ptype_count[altP][capturedPtype]==0)
	ptypeo_mask &= ~(1<<(last_move.capturePtypeO()-PTYPEO_MIN));
      --ptype_board_count[altP][capturedPtype];
      const Ptype base_captured = unpromote(capturedPtype);
      ++ptype_count[P][base_captured];
      ptypeo_mask |= (1<<(captured(capturedPtypeO)-PTYPEO_MIN));
    }
    eval(state, ptype_count, ptype_board_count, last_value_and_out);
    return;
  }
  
  MultiInt sum;
  if (last_move.isDrop())
  {
    const int count = ++ptype_board_count[P][last_move.ptype()];
    sum = valueBoardAll(last_move.ptype(),count,kings_x[P],kings_y[P],kings_x[altP],kings_y[altP]);
  }
  else{
    Ptype capturedPtype = last_move.capturePtype();
    if (capturedPtype != PTYPE_EMPTY)
    {
      const int count = --ptype_count[altP][capturedPtype];
      if(count==0)
	ptypeo_mask &= ~(1<<(last_move.capturePtypeO()-PTYPEO_MIN));
      const int board_count = --ptype_board_count[altP][capturedPtype];
      const Ptype base_captured = unpromote(capturedPtype);
      const int c_count = ++ptype_count[P][base_captured];
      ptypeo_mask |= 1<<(captured(last_move.capturePtypeO())-PTYPEO_MIN);
      sum=valueAll(capturedPtype,count+1,kings_x[altP],kings_y[altP],kings_x[P],kings_y[P])+
	valueBoardAll(capturedPtype,board_count+1,kings_x[altP],kings_y[altP],kings_x[P],kings_y[P])+
	valueAll(base_captured,c_count,kings_x[P],kings_y[P],kings_x[altP],kings_y[altP]);
    }
    if (last_move.isPromotion())
    {
      const Ptype old_ptype = last_move.oldPtype();
      const Ptype new_ptype = last_move.ptype();
      const int base_count = --ptype_count[P][old_ptype];
      const int base_board_count = --ptype_board_count[P][old_ptype];
      const int count = ++ptype_count[P][new_ptype];
      const int board_count = ++ptype_board_count[P][new_ptype];
      if(base_count==0)
	ptypeo_mask &= ~(1<<(last_move.oldPtypeO()-PTYPEO_MIN));
      ptypeo_mask |= (1<<(last_move.ptypeO()-PTYPEO_MIN));
      sum+=valueAll(new_ptype,count,kings_x[P],kings_y[P],kings_x[altP],kings_y[altP])+
	valueBoardAll(new_ptype,board_count,kings_x[P],kings_y[P],kings_x[altP],kings_y[altP])-
	valueAll(old_ptype,base_count+1,kings_x[P],kings_y[P],kings_x[altP],kings_y[altP])-
	valueBoardAll(old_ptype,base_board_count+1,kings_x[P],kings_y[P],kings_x[altP],kings_y[altP]);
    }
  }
  if(P==BLACK) last_value_and_out+= sum;
  else last_value_and_out-= sum;
}

void osl::eval::ml::
LanceEffectPieceKingRelative::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::CArray<MultiInt, 9792>
osl::eval::ml::LanceEffectPieceKingRelative::table;

MultiInt osl::eval::ml::
LanceEffectPieceKingRelative::eval(const NumEffectState &state)
{
  MultiInt result;
  for (int i = PtypeTraits<LANCE>::indexMin;
       i < PtypeTraits<LANCE>::indexLimit;
       ++i)
  {
    const Piece lance = state.pieceOf(i);
    if (lance.isOnBoard() && !lance.isPromoted())
    {
      const Square self_king = state.kingSquare(lance.owner());
      const Square opp_king = state.kingSquare(alt(lance.owner()));
      Square p = state.mobilityOf(lance.owner() == BLACK ? U : D,
				     lance.number());
      if (!p.isOnBoard())
      {
	const int index1 = 0 + 0 + (PTYPEO_EDGE - PTYPEO_MIN) * 17 * 9;
	const int index2 = 0 + 0 + (PTYPEO_EDGE - PTYPEO_MIN) * 17 * 9 + 4896;
	if (lance.owner() == BLACK)
	{
	  result += table[index1];
	  result += table[index2];
	}
	else
	{
	  result -= table[index1];
	  result -= table[index2];
	}
      }
      else
      {
	const int index1 = index(lance.owner(), p, opp_king,
				 state.pieceAt(p).ptypeO(), true);
	const int index2 = index(lance.owner(), p, self_king,
				 state.pieceAt(p).ptypeO(), false);
	if (lance.owner() == BLACK)
	{
	  result += table[index1];
	  result += table[index2];
	}
	else
	{
	  result -= table[index1];
	  result -= table[index2];
	}
      }
    }
  }
  return result;
}

osl::CArray<MultiInt, 1440>
osl::eval::ml::PtypeYPawnY::table;

void osl::eval::ml::PtypeYPawnY::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s = 0; s < NStages; ++s)
    {
      table[i][s] = weights.value(i + ONE_DIM*s);
    }
  }
}

osl::MultiInt osl::eval::ml::
PtypeYPawnY::eval(const NumEffectState &state,
		  const CArray2d<int, 2, 9> &pawns)
{
  MultiInt result;
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    // only skip pawn, not ppawns
    if (piece.ptype() == PAWN)
      continue;
    if (!piece.isOnBoard())
      continue;

    const int idx = index(piece.owner(), piece.ptype(), piece.square().y(),
			  pawns[piece.owner()][piece.square().x() - 1]);
    if (piece.owner() == BLACK)
    {
      result += table[idx];
    }
    else
    {
      result -= table[idx];
    }
  }

  return result;
}

template<osl::Player P>
void osl::eval::ml::
PtypeYPawnY::evalWithUpdateBang(const NumEffectState &state,
				Move moved,
				const CArray2d<int, 2, 9> &pawns,
				MultiInt& last_value)
{
  Ptype captured = moved.capturePtype();
  assert(P==moved.player());

  if (moved.oldPtype() == PAWN)
  {
    const int x = moved.to().x();
    const int old_pawn_y = (moved.isDrop() ? 0 : moved.from().y());
    const int new_pawn_y = pawns[P][moved.to().x() - 1];
    for (int y = 1; y <= 9; ++y)
    {
      const Piece p = state.pieceAt(Square(x, y));
      if (y == moved.to().y())
      {
	if (p.ptype() == PPAWN)
	{
	  const int idx_new = index(P, p.ptype(), y, new_pawn_y);
	  if (P == BLACK)
	  {
	    last_value += table[idx_new];
	  }
	  else
	  {
	    last_value -= table[idx_new];
	  }   
	}
      }
      else if (!p.isEmpty() && p.owner() == P)
      {
	const int idx_old = index(P, p.ptype(), y, old_pawn_y);
	const int idx_new = index(P, p.ptype(), y, new_pawn_y);
	if (P == BLACK)
	{
	  last_value -= table[idx_old];
	  last_value += table[idx_new];
	}
	else
	{
	  last_value += table[idx_old];
	  last_value -= table[idx_new];
	}   
      }
    }
  }
  else
  {
    if (!moved.isDrop())
    {
      const int pawn_y = pawns[P][moved.from().x() - 1];
      const int idx = index(P, moved.oldPtype(), moved.from().y(),
			    pawn_y);
      if (P == BLACK)
      {
	last_value -= table[idx];
      }
      else
      {
	last_value += table[idx];
      }
    }
    {
      const int pawn_y = pawns[P][moved.to().x() - 1];
      const int idx = index(P, moved.ptype(), moved.to().y(),
			    pawn_y);
      if (P == BLACK)
      {
	last_value += table[idx];
      }
      else
      {
	last_value -= table[idx];
      }
    }
  }

  if (captured != PTYPE_EMPTY)
  {
    if (captured == PAWN)
    {
      const int old_pawn_y = moved.to().y();
      const int new_pawn_y = 0;
      const int x = moved.to().x();
      for (int y = 1; y <= 9; ++y)
      {
	const Piece p = state.pieceAt(Square(x, y));
	if (!p.isEmpty() && p.owner() == alt(P))
	{
	  const int idx_old = index(alt(P), p.ptype(), y,
				    old_pawn_y);
	  const int idx_new = index(alt(P), p.ptype(), y,
				    new_pawn_y);
	  if (P == BLACK)
	  {
	    last_value += table[idx_old];
	    last_value -= table[idx_new];
	  }
	  else
	  {
	    last_value -= table[idx_old];
	    last_value += table[idx_new];
	  }   
	}
      }
    }
    else
    {
      const int pawn_y = pawns[alt(P)][moved.to().x() - 1];
      const int idx = index(alt(P), captured, moved.to().y(),
			    pawn_y);
      if (P == BLACK)
      {
	last_value += table[idx];
      }
      else
      {
	last_value -= table[idx];
      }
    }
  }
}

osl::CArray<osl::MultiInt, 1215>
osl::eval::ml::GoldAndSilverNearKing::table;
osl::CArray<osl::MultiInt, 9720>
osl::eval::ml::GoldAndSilverNearKing::combination_table;

void osl::eval::ml::
GoldAndSilverNearKing::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
GoldAndSilverNearKingCombination::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      GoldAndSilverNearKing::combination_table[i][s] =
	weights.value(i + ONE_DIM*s);
  }
}

template <osl::Player P>
osl::MultiInt osl::eval::ml::
GoldAndSilverNearKing::evalOne(const NumEffectState &state,
			       const CArray2d<int, 2, 3> &gs_count)
{
  MultiInt result;
  int total = 0;
  const Square king = state.kingSquare<P>();
  for (size_t i = 0; i < gs_count[0].size(); ++i)
  {
    total += gs_count[P][i];
    if (total != 0)
    {
      result += table[index<P>(king, i, total)];
    }
  }
  result += combination_table[
    indexCombination<P>(king, gs_count[P][0],
			gs_count[P][1], gs_count[P][2])];
  return P == BLACK ? result : -result;
}

osl::MultiInt osl::eval::ml::
GoldAndSilverNearKing::eval(const NumEffectState &state,
			    const CArray2d<int, 2, 3> &gs_count)
{
  return evalOne<BLACK>(state, gs_count) + evalOne<WHITE>(state, gs_count);
}


osl::CArray<osl::MultiInt, 8192>
osl::eval::ml::PtypeCombination::table;

void osl::eval::ml::
PtypeCombination::setUp(const Weights &weights)
{
  static CArray<MultiInt, 8192> orig_table;
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s = 0; s < NStages; ++s)
    {
      orig_table[i][s] = weights.value(i + ONE_DIM*s);
    }
  }
  for(int i=0;i<8192;i++){
    int pawn=(i>>12)&1;
    int ppawn=(i>>6)&1;
    int lance=(i>>11)&1;
    int plance=(i>>5)&1;
    int knight=(i>>10)&1;
    int pknight=(i>>4)&1;
    int silver=(i>>9)&1;
    int psilver=(i>>3)&1;
    int bishop=(i>>8)&1;
    int pbishop=(i>>2)&1;
    int rook=(i>>7)&1;
    int prook=(i>>1)&1;
    int gold=(i>>0)&1;
    int newIndex=ppawn|(plance<<1)|(pknight<<2)|(psilver<<3)|(pbishop<<4)|
      (prook<<5)|(gold<<6)|(pawn<<7)|(lance<<8)|(knight<<9)|(silver<<10)|
      (bishop<<11)|(rook<<12);
    table[newIndex]=orig_table[i];
  }
}

osl::MultiInt osl::eval::ml::
PtypeCombination::eval(unsigned int ptypeo_mask)
{
  return evalOne<BLACK>(ptypeo_mask) + evalOne<WHITE>(ptypeo_mask);
}


osl::CArray<osl::MultiInt, 5*2>
osl::eval::ml::SilverFork::table;
inline
std::pair<int,int> osl::eval::ml::
SilverFork::matchRook(const NumEffectState& state, Piece rook,
		      const CArray<bool,2>& has_silver,
		      Square& silver_drop)
{
  const Square sq = rook.square();
  if (rook.isPromoted() || sq.isPieceStand())
    return std::make_pair(0,0);
  const Player owner = rook.owner();
  if (! has_silver[alt(owner)] || ! sq.canPromote(alt(owner)))
    return std::make_pair(0,0);
  const CArray<Offset,2> offset = {{
      Board_Table.getOffset(owner, UL), Board_Table.getOffset(owner, UR)
    }};
  for (size_t i=0; i<offset.size(); ++i) {
    const Square next = sq+offset[i], next2 = next+offset[i];
    if (! state.pieceAt(next).isEmpty() || state.hasEffectAt(owner, next))
      continue;
    const Piece p = state.pieceAt(next2);
    if (! p.isOnBoardByOwner(owner)) 
      continue;
    silver_drop = next;
    if (p.ptype() == ROOK)
      return std::make_pair(sign(owner), 0);
    if (p.ptype() == GOLD)
      return std::make_pair(sign(owner), state.hasEffectAt(owner, next2) ? 1 : 2);
  }
  return std::make_pair(0,0);
}
inline
std::pair<int,int> osl::eval::ml::
SilverFork::matchGold(const NumEffectState& state, Piece gold,
		      const CArray<bool,2>& has_silver, Square& silver_drop)
{
  const Square sq = gold.square();
  if (sq.isPieceStand())
    return std::make_pair(0,0);
  const Player owner = gold.owner();
  if (! has_silver[alt(owner)] || ! sq.canPromote(alt(owner)))
    return std::make_pair(0,0);
  const CArray<Offset,2> offset = {{
      Board_Table.getOffset(BLACK, L), Board_Table.getOffset(BLACK, R)
    }};
  const bool guarded = state.hasEffectAt(owner, sq);
  for (size_t i=0; i<offset.size(); ++i) {
    const Square next = sq+offset[i], next2 = next+offset[i];
    const Piece np = state.pieceAt(next);
    if (np.isEdge())
      continue;
    const Square next_down = next + Board_Table.getOffset(owner, D);
    if (! state.pieceAt(next_down).isEmpty() || state.hasEffectAt(owner, next_down))
      continue;
    const Piece p = state.pieceAt(next2);
    if (! p.isOnBoardByOwner(owner))
      continue;
    if (p.ptype() == ROOK || p.ptype() == GOLD) {
      silver_drop = next_down;
      const bool recaputure = guarded
	|| (p.ptype() == GOLD && state.hasEffectAt(owner, next2))
	|| (np.canMoveOn(owner) && ! state.hasEffectAt(alt(owner), next));
      return std::make_pair(sign(owner), 3 + recaputure);
    }
  }
  return std::make_pair(0,0);
}

osl::MultiIntPair osl::eval::ml::
SilverFork::eval(const NumEffectState& state, CArray<std::pair<Square,int>,2>& silver_drop)
{
  silver_drop.fill(std::make_pair(Square(),0));
  MultiIntPair result;		// by turn
  const CArray<bool,2> has_silver = {{ 
      state.hasPieceOnStand<SILVER>(BLACK), 
      state.hasPieceOnStand<SILVER>(WHITE),
    }};
  if (! has_silver[BLACK] && ! has_silver[WHITE])
    return result;
  Square drop;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit; ++i) 
  {
    const Piece rook = state.pieceOf(i);
    std::pair<int,int> match = matchRook(state, rook, has_silver, drop);
    if (match.first) {
      const MultiInt value_attack = table[match.second*2];
      const Player attack = (match.first > 0) ? WHITE : BLACK;
      if (-value_attack[0] > silver_drop[attack].second) {
	silver_drop[attack].second = -value_attack[0];
	silver_drop[attack].first = drop;
      }
      if (match.first > 0)	// owner is black
      {
	result[BLACK] += table[match.second*2+1];
	result[WHITE] += value_attack;
      }
      else if (match.first < 0)	// owner is white
      {
	result[BLACK] -= value_attack;
	result[WHITE] -= table[match.second*2+1];
      }
    }
  }

  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i) 
  {
    const Piece gold = state.pieceOf(i);
    std::pair<int,int> match = matchGold(state, gold, has_silver, drop);
    if (match.first) {
      const MultiInt value_attack = table[match.second*2];
      const Player attack = (match.first > 0) ? WHITE : BLACK;
      if (-value_attack[0] > silver_drop[attack].second) {
	silver_drop[attack].second = -value_attack[0];
	silver_drop[attack].first = drop;
      }
      if (match.first > 0)
      {
	result[BLACK] += table[match.second*2+1];
	result[WHITE] += value_attack;
      }
      else if (match.first < 0)
      {
	result[BLACK] -= value_attack;
	result[WHITE] -= table[match.second*2+1];
      }
    }
  }
  return result;
}

void osl::eval::ml::SilverFork::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

osl::CArray<osl::MultiInt, 256*2*2>
osl::eval::ml::BishopRookFork::table;
void osl::eval::ml::BishopRookFork::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for (int i=0; i<PTYPE_SIZE; ++i)
    for (int j=i+1; j<PTYPE_SIZE; ++j)
    {
      table[bishopIndex((Ptype)j,(Ptype)i)*2]   = table[bishopIndex((Ptype)i,(Ptype)j)*2];
      table[bishopIndex((Ptype)j,(Ptype)i)*2+1] = table[bishopIndex((Ptype)i,(Ptype)j)*2+1];
      table[rookIndex((Ptype)j,(Ptype)i)*2]     = table[rookIndex((Ptype)i,(Ptype)j)*2];
      table[rookIndex((Ptype)j,(Ptype)i)*2+1]   = table[rookIndex((Ptype)i,(Ptype)j)*2+1];
    }
}
inline
const osl::Square osl::eval::ml::BishopRookFork::
findDropInLine(const NumEffectState& state, Player defense, 
	       const Square a, const Square b, Piece king)
{
  Offset offset = Board_Table.getShortOffset(Offset32(b,a));
  Square drop_position;
  Square sq=a+offset;
  for (Piece p=state.pieceAt(sq); p.isEmpty(); sq+=offset, p=state.pieceAt(sq))
  {
    if (! drop_position.isPieceStand())
      continue;
    if (! state.hasEffectAt(defense, sq)
	|| (state.hasEffectAt(alt(defense), sq)
	    && ! state.hasEffectNotBy(defense, king, sq)))
      drop_position = sq;
  }
  return (sq == b) ? drop_position : Square();
}
inline
bool osl::eval::ml::BishopRookFork::
testCenter(const NumEffectState& state, Player defense, 
	   const Square a, const Square b, Piece king,
	   Square center, bool maybe_empty)
{
  const Piece p = state.pieceAt(center);
  if (! p.isEmpty() 
      || (state.hasEffectAt(defense, center)
	  && (! state.hasEffectAt(alt(defense), center)
	      || state.hasEffectNotBy(defense, king, center))))
    return false;
  return state.isEmptyBetween(center, a, !maybe_empty)
    && state.isEmptyBetween(center, b, !maybe_empty);
}

const osl::Square osl::eval::ml::
BishopRookFork::isBishopForkSquare(const NumEffectState& state, Player defense, 
				   const Square a, const Square b,
				   bool maybe_empty)
{
  const Piece king = state.kingPiece(defense);
  const int cx = b.x() - a.x(), cy = b.y() - a.y();
  if ((cx + cy) % 2)
    return Square();
  const int p = (cx+cy)/2, q = (cx-cy)/2;
  if (p == 0 || q == 0)
    return findDropInLine(state, defense, a, b, king);

  const CArray<Square,2> centers = {{
      b + Offset(-p,-p), b + Offset(-q,q)
    }};
  
  for (size_t i=0; i<centers.size(); ++i) {
    if (! centers[i].isOnBoardRegion())
      continue;
    if (testCenter(state, defense, a, b, king, centers[i], maybe_empty))
      return centers[i];
  }
  return Square();
}

inline
const osl::Square osl::eval::ml::
BishopRookFork::isRookForkSquare(const NumEffectState& state, Player defense, 
				   const Square a, const Square b)
{
  const Piece king = state.kingPiece(defense);
  const CArray<Square,2> centers = {{
      Square(a.x(), b.y()), Square(b.x(), a.y())
    }};
  if (centers[0] == a || centers[0] == b)
    return findDropInLine(state, defense, a, b, king);
  for (size_t i=0; i<centers.size(); ++i) 
  {
    assert(centers[i].isOnBoardRegion());
    if (testCenter(state, defense, a, b, king, centers[i])) 
      return centers[i];
  }
  return Square();
}

template <osl::Player Defense>
osl::MultiIntPair osl::eval::ml::
BishopRookFork::evalOne(const NumEffectState &state, const PieceVector& target,
			std::pair<Square,int>& bishop_drop,
			std::pair<Square,int>& rook_drop)
{
  MultiIntPair result;
  for (size_t i=0; i<target.size(); ++i) 
  {
    const Piece pi = target[i];
    assert(pi.isOnBoardByOwner(Defense));
    for (size_t j=i+1; j<target.size(); ++j) 
    {
      const Piece pj = target[j];
      assert(pj.isOnBoardByOwner(Defense));
      if (state.hasPieceOnStand<BISHOP>(alt(Defense)))
      {
	const Square center
	  = isBishopForkSquare(state, Defense, pi.square(), pj.square());
	if (! center.isPieceStand()) {
	  const int index = bishopIndex(pi.ptype(), pj.ptype())*2;
	  const MultiInt value_attack = table[index];
	  if (-value_attack[0] > bishop_drop.second) { // negative value is better for attacker
	    bishop_drop.second = -value_attack[0];
	    bishop_drop.first  = center;
	  }
	  if (Defense == BLACK)
	  {
	    result[BLACK] += table[index+1];
	    result[WHITE] += value_attack;
	  }
	  else
	  {
	    result[BLACK] -= value_attack;
	    result[WHITE] -= table[index+1];
	  }
	}
      }
      if (state.hasPieceOnStand<ROOK>(alt(Defense)))
      {
	const Square center
	  = isRookForkSquare(state, Defense, pi.square(), pj.square());
	if (! center.isPieceStand()) {
	  const int index = rookIndex(pi.ptype(), pj.ptype())*2;
	  const MultiInt value_attack = table[index];
	  if (-value_attack[0] > rook_drop.second) { // negative value is better for attacker
	    rook_drop.second = -value_attack[0];
	    rook_drop.first  = center;
	  }	
	  if (Defense == BLACK)
	  {
	    result[BLACK] += table[index+1];
	    result[WHITE] += value_attack;
	  }
	  else
	  {
	    result[BLACK] -= value_attack;
	    result[WHITE] -= table[index+1];
	  }
	}
      }
    }
  }
  assert(bishop_drop.second == 0 || ! bishop_drop.first.isPieceStand());
  return result;
}

osl::MultiIntPair osl::eval::ml::
BishopRookFork::eval(const NumEffectState &state, 
		     CArray<std::pair<Square,int>,2>& bishop_drop,
		     CArray<std::pair<Square,int>,2>& rook_drop)
{
  bishop_drop.fill(std::make_pair(Square(),0));
  rook_drop.fill(std::make_pair(Square(),0));
  MultiIntPair result;
  const CArray<bool,2> has_bishop = {{ 
      state.hasPieceOnStand<BISHOP>(BLACK), 
      state.hasPieceOnStand<BISHOP>(WHITE),
    }};
  const CArray<bool,2> has_rook = {{ 
      state.hasPieceOnStand<ROOK>(BLACK), 
      state.hasPieceOnStand<ROOK>(WHITE),
    }};
  if (has_bishop[BLACK] + has_bishop[WHITE]
      + has_rook[BLACK] + has_rook[WHITE] == 0)
    return result;
  PieceMask notcovered = ~state.effectedMask(BLACK); 
  notcovered &= ~state.effectedMask(WHITE);
  notcovered.clearBit<PAWN>();
  notcovered.setBit<KING>();
  if (has_bishop[WHITE] + has_rook[WHITE]) {
    PieceVector pieces;
    PieceMask target = notcovered & state.piecesOnBoard(BLACK);
    while (target.any())
      pieces.push_back(state.pieceOf(target.takeOneBit()));
    result += evalOne<BLACK>(state, pieces, bishop_drop[WHITE], rook_drop[WHITE]);
  }
  if (has_bishop[BLACK] + has_rook[BLACK]) {
    PieceVector pieces;
    PieceMask target = notcovered & state.piecesOnBoard(WHITE);
    while (target.any())
      pieces.push_back(state.pieceOf(target.takeOneBit()));
    result += evalOne<WHITE>(state, pieces, bishop_drop[BLACK], rook_drop[BLACK]);
  }
  return result;
}



osl::CArray<osl::MultiInt, 256*2*2>
osl::eval::ml::KnightFork::table;
void osl::eval::ml::KnightFork::setUp(const Weights &weights)
{
  for (int i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
  for (int i=0; i<PTYPE_SIZE; ++i)
    for (int j=i+1; j<PTYPE_SIZE; ++j) {
      table[index((Ptype)j,(Ptype)i)*2] = table[index((Ptype)i,(Ptype)j)*2];
      table[index((Ptype)j,(Ptype)i)*2+1] = table[index((Ptype)i,(Ptype)j)*2+1];
      table[(index((Ptype)j,(Ptype)i)+DROP_DIM)*2] = table[(index((Ptype)i,(Ptype)j)+DROP_DIM)*2];
      table[(index((Ptype)j,(Ptype)i)+DROP_DIM)*2+1] = table[(index((Ptype)i,(Ptype)j)+DROP_DIM)*2+1];
    }
}

template <osl::Player Defense>
osl::MultiIntPair osl::eval::ml::
KnightFork::evalOne(const NumEffectState &state, bool has_knight, 
		    BoardMask& knight_fork_squares, 
		    std::pair<Square,int>& knight_drop)
{
  knight_fork_squares.clear();
  const int z = playerToIndex(Defense);
  const int y_min = 3-z*2, y_max = 9-z*2;
  CArray<PieceVector,10> pieces;
  {
    PieceMask target = state.piecesOnBoard(Defense);
    target.clearBit<PAWN>();
    target.clearBit<LANCE>();
    target.clearBit<KNIGHT>();
    while (target.any()) {
      const Piece p = state.pieceOf(target.takeOneBit());
      const int y = p.square().y();
      pieces[y].push_back(p);
    }
  }
  MultiIntPair result;
  for (int y=y_min; y<=y_max; ++y){
    if (pieces[y].size() < 2)
      continue;
    const int y_drop = y - sign(Defense)*2;
    for (size_t i=0; i<pieces[y].size(); ++i) 
    {
      const Piece pi = pieces[y][i];
      assert(pi.isOnBoardByOwner(Defense));
      assert(pi.square().y() == y);
      const int xi = pi.square().x();
      for (size_t j=i+1; j<pieces[y].size(); ++j) 
      {
	const Piece pj = pieces[y][j];
	assert(pj.isOnBoardByOwner(Defense));
	assert(pj.square().y() == y);
	const int xj = pj.square().x();
	if (abs(xi -xj) != 2)
	  continue;
	const Square drop = Square((xi+xj)/2, y_drop);
	knight_fork_squares.set(drop); 
	if (! state[drop].isEmpty() || state.hasEffectAt(Defense, drop))
	  continue;
 	int found = index(pi.ptype(), pj.ptype());
	if (! has_knight)
	  found += DROP_DIM;
	found *= 2;
	const MultiInt value_attack = table[found];
	if (Defense == BLACK)
	{
	  result[BLACK] += table[found+1];
	  result[WHITE] += value_attack;
	}
	else
	{
	  result[BLACK] -= value_attack;
	  result[WHITE] -= table[found+1];
	}
	if (has_knight && -value_attack[0] > knight_drop.second) {
	  knight_drop.second = -value_attack[0];
	  knight_drop.first = Square((pi.square().x()+pj.square().x())/2, y_drop);
	}
      }
    }
  }
  return result;
}

osl::MultiIntPair osl::eval::ml::
KnightFork::eval(const NumEffectState &state,
		 CArray<BoardMask,2>& knight_fork_squares, 
		 CArray<std::pair<Square,int>,2>& knight_drop)
{
  knight_drop.fill(std::make_pair(Square(),0));
  MultiIntPair result;
  const CArray<bool,2> has_knight = {{ 
      state.hasPieceOnStand<KNIGHT>(BLACK), 
      state.hasPieceOnStand<KNIGHT>(WHITE),
    }};
  
  const CArray<bool,2> may_have_knight = {{ 
      has_knight[BLACK] 
      || (state.effectedMask(BLACK).selectBit<KNIGHT>() 
	  & ~state.effectedMask(WHITE).selectBit<KNIGHT>() 
	  & state.piecesOnBoard(WHITE).getMask(PtypeFuns<KNIGHT>::indexNum)).any(),
      has_knight[WHITE]
      || (state.effectedMask(WHITE).selectBit<KNIGHT>() 
	  & ~state.effectedMask(BLACK).selectBit<KNIGHT>() 
	  & state.piecesOnBoard(BLACK).getMask(PtypeFuns<KNIGHT>::indexNum)).any(),
    }};
  if (has_knight[BLACK] + has_knight[WHITE]
      + may_have_knight[BLACK] + may_have_knight[WHITE] == 0) {
    knight_fork_squares[BLACK].invalidate();
    knight_fork_squares[WHITE].invalidate();
    return result;
  }
  {
    const Player Defense = BLACK;
    if (has_knight[alt(Defense)] + may_have_knight[alt(Defense)] > 0)
      result += evalOne<Defense>(state, has_knight[alt(Defense)],
				 knight_fork_squares[alt(Defense)],
				 knight_drop[alt(Defense)]);
    else
      knight_fork_squares[alt(Defense)].invalidate();
  }
  {
    const Player Defense = WHITE;
    if (has_knight[alt(Defense)] + may_have_knight[alt(Defense)] > 0)
      result += evalOne<Defense>(state, has_knight[alt(Defense)],
				 knight_fork_squares[alt(Defense)],
				 knight_drop[alt(Defense)]);
    else
      knight_fork_squares[alt(Defense)].invalidate();
  }
  return result;
}

template <osl::Player P, osl::Player Defense>
void osl::eval::ml::
KnightFork::updateSquares(const NumEffectState& state, Move moved,
			  BoardMask& knight_fork_squares)
{
  assert(! knight_fork_squares.isInvalid());
  const Square to = moved.to();
  if (P != Defense) {
    if (! moved.isCapture())
      return;
    if ((Defense == BLACK && to.y() >= 3)
	|| (Defense == WHITE && to.y() <= 7)) {
      knight_fork_squares.reset(to.neighbor<Defense,UUL>());
      knight_fork_squares.reset(to.neighbor<Defense,UUR>());
    }
    return;
  }
  if (! moved.isDrop()) {
    if ((P == BLACK && moved.from().y() >= 3)
	|| (P == WHITE && moved.from().y() <= 7)) {
      knight_fork_squares.reset(moved.from().neighbor<P,UUL>());
      knight_fork_squares.reset(moved.from().neighbor<P,UUR>());
    }
  }
  if (! isTarget(moved.ptype())
      || (P == BLACK && to.y() < 3) || (P == WHITE && to.y() > 7))
    return;
  if (to.x() <= 7)
  {
    const Square l = to.neighbor<BLACK,L>(), l2 = l.neighbor<BLACK,L>();
    if (state[l2].isOnBoardByOwner<P>()) {
      knight_fork_squares.set(l.neighbor<P,U>().template neighbor<P,U>());
    }
  }
  if (to.x() >= 3)
  {
    const Square r = to.neighbor<BLACK,R>(), r2 = r.neighbor<BLACK,R>();
    if (state[r2].isOnBoardByOwner<P>()){
      knight_fork_squares.set(r.neighbor<P,U>().template neighbor<P,U>());
    }
  }
}

template <osl::Player Defense>
osl::MultiIntPair osl::eval::ml::
KnightFork::accumulate(const NumEffectState& state,
		       bool has_knight,
		       const BoardMask& knight_fork_squares,
		       std::pair<Square,int>& knight_drop)
{
  MultiIntPair result;
  BoardMask mask = knight_fork_squares;
  while (mask.any()) {
    Square sq = mask.takeOneBit();
    if (! state[sq].isEmpty() || state.hasEffectAt(Defense, sq))
      continue;
    const Piece pi = state[sq.back<Defense,UUL>()];
    const Piece pj = state[sq.back<Defense,UUR>()];
    if (! pi.isOnBoardByOwner<Defense>() || ! pj.isOnBoardByOwner<Defense>())
      std::cerr << state << Defense << ' ' << pi << ' ' << pj << "\n";
    assert(pi.isOnBoardByOwner<Defense>());
    assert(pj.isOnBoardByOwner<Defense>());
    int found = index(pi.ptype(), pj.ptype());
    if (! has_knight)
      found += DROP_DIM;
    found *= 2;
    const MultiInt value_attack = table[found];
    if (Defense == BLACK)
    {
      result[BLACK] += table[found+1];
      result[WHITE] += value_attack;
    }
    else
    {
      result[BLACK] -= value_attack;
      result[WHITE] -= table[found+1];
    }
    if (has_knight && -value_attack[0] > knight_drop.second) {
      knight_drop.second = -value_attack[0];
      knight_drop.first = sq;
    }    
  }
  return result;
}

template <osl::Player P>
osl::MultiIntPair osl::eval::ml::
KnightFork::evalWithUpdate(const NumEffectState &state, Move moved,
			   CArray<BoardMask,2>& knight_fork_squares, 
			   CArray<std::pair<Square,int>,2>& knight_drop)
{
  knight_drop.fill(std::make_pair(Square(),0));
  MultiIntPair result;
  const CArray<bool,2> has_knight = {{ 
      state.hasPieceOnStand<KNIGHT>(BLACK), 
      state.hasPieceOnStand<KNIGHT>(WHITE),
    }};  
  const CArray<bool,2> may_have_knight = {{ 
      has_knight[BLACK] 
      || (state.effectedMask(BLACK).selectBit<KNIGHT>() 
	  & ~state.effectedMask(WHITE).selectBit<KNIGHT>() 
	  & state.piecesOnBoard(WHITE).getMask(PtypeFuns<KNIGHT>::indexNum)).any(),
      has_knight[WHITE]
      || (state.effectedMask(WHITE).selectBit<KNIGHT>() 
	  & ~state.effectedMask(BLACK).selectBit<KNIGHT>() 
	  & state.piecesOnBoard(BLACK).getMask(PtypeFuns<KNIGHT>::indexNum)).any(),
    }};
  if (has_knight[BLACK] + has_knight[WHITE]
      + may_have_knight[BLACK] + may_have_knight[WHITE] == 0) {
    knight_fork_squares[BLACK].invalidate();
    knight_fork_squares[WHITE].invalidate();
    return result;
  }
  {
    const Player Defense = BLACK;
    if (has_knight[alt(Defense)] + may_have_knight[alt(Defense)] > 0) {
      if (knight_fork_squares[alt(Defense)].isInvalid())
	result += evalOne<Defense>(state, has_knight[alt(Defense)],
				   knight_fork_squares[alt(Defense)],
				   knight_drop[alt(Defense)]);
      else {
	updateSquares<P,Defense>(state, moved, knight_fork_squares[alt(Defense)]);
	result += accumulate<Defense>(state, has_knight[alt(Defense)],
				      knight_fork_squares[alt(Defense)],
				      knight_drop[alt(Defense)]);
      }
    }
    else
      knight_fork_squares[alt(Defense)].invalidate();
  }
  {
    const Player Defense = WHITE;
    if (has_knight[alt(Defense)] + may_have_knight[alt(Defense)] > 0) {
      if (knight_fork_squares[alt(Defense)].isInvalid())
	result += evalOne<Defense>(state, has_knight[alt(Defense)],
				   knight_fork_squares[alt(Defense)],
				   knight_drop[alt(Defense)]);
      else {
	updateSquares<P,Defense>(state, moved, knight_fork_squares[alt(Defense)]);
	result += accumulate<Defense>(state, has_knight[alt(Defense)],
				      knight_fork_squares[alt(Defense)],
				      knight_drop[alt(Defense)]);
      }
    }
    else
      knight_fork_squares[alt(Defense)].invalidate();
  }
  return result;
}


void osl::eval::ml::SilverAdvance26::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}
osl::MultiInt osl::eval::ml::
SilverAdvance26::eval(const NumEffectState &state) 
{
  const CArray<std::pair<Square,Ptype>,5> pattern = {{
      std::make_pair( Square(2,6), SILVER ),
      std::make_pair( Square(1,5), PAWN ),
      std::make_pair( Square(3,7), KNIGHT ),
      std::make_pair( Square(2,5), PAWN ),
      std::make_pair( Square(3,6), PAWN ),
    }};
  MultiInt sum;
  bool match = state.kingSquare(BLACK).x() >= 5;
  if (match) {
    for (size_t i=0; i<pattern.size(); ++i) {
      const Piece p = state.pieceAt(pattern[i].first);
      if (p.ptype() != pattern[i].second || p.owner() != BLACK) {
	match = false;
	break;
      }
    }
    if (match)
      sum += table[0];
  }
  match = state.kingSquare(WHITE).x() <= 5;
  if (match) {
    for (size_t i=0; i<pattern.size(); ++i) {
      const Piece p = state.pieceAt(pattern[i].first.rotate180());
      if (p.ptype() != pattern[i].second || p.owner() != WHITE) {
	match = false;
	break;
      }
    }
    if (match)
      sum += -table[0];
  }
  return sum;
}



osl::CArray<osl::MultiInt, osl::PTYPE_SIZE>
osl::eval::ml::Promotion37::table;
void osl::eval::ml::Promotion37::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <osl::Player P>
osl::MultiInt osl::eval::ml::
Promotion37::evalOne(const NumEffectState &state, int rank) 
{
  CArray<int,PTYPE_SIZE> count = {{ 0 }};
  for (int x=1; x<=9; ++x) {
    const Square target(x, rank);
    if (! state[target].isEmpty())
      continue;
    int a = state.countEffect(P, target);
    const int d = state.countEffect(alt(P), target);
    if (a > 0 && a == d)
      a += AdditionalEffect::hasEffect(state, target, P);
    if (a <= d)
      continue;
    const Ptype ptype = state.findCheapAttack(P, target).ptype();
    if (isPiece(ptype) && ! isPromoted(ptype))
      count[ptype]++;
  }
  MultiInt ret;
  for (int p=PTYPE_BASIC_MIN; p<=PTYPE_MAX; ++p) {
    if (count[p] > 0)
      ret += table[p]*sign(P);
    if (count[p] > 1)
      ret += table[p-8]*(sign(P)*(count[p]-1));
  }
  return ret;
}

osl::MultiInt osl::eval::ml::
Promotion37::eval(const NumEffectState &state) 
{
  return evalOne<BLACK>(state, 3) + evalOne<WHITE>(state, 7);
}

template <osl::Player P>
osl::MultiInt osl::eval::ml::
Promotion37::evalWithUpdate(const NumEffectState& state, Move moved,
			    MultiInt const& last_value)
{
  if (moved.isPass())
    return last_value;
  // todo changedEffects
  return eval(state);
}


namespace osl
{
  namespace eval
  {
    namespace ml
    {
      template void PawnAdvanceAll::
      evalWithUpdateBang<BLACK>(const NumEffectState &, Move,MultiInt&);
      template void PawnAdvanceAll::
      evalWithUpdateBang<WHITE>(const NumEffectState &, Move,MultiInt&);
      template MultiInt PtypeY::
      evalWithUpdate<BLACK>(const NumEffectState &, Move, MultiInt const&);
      template MultiInt PtypeY::
      evalWithUpdate<WHITE>(const NumEffectState &, Move, MultiInt const&);
      template MultiInt PtypeX::
      evalWithUpdate<BLACK>(const NumEffectState &, Move, MultiInt const&);
      template MultiInt PtypeX::
      evalWithUpdate<WHITE>(const NumEffectState &, Move, MultiInt const&);
      template MultiInt PawnPtypeOPtypeO::
      evalWithUpdate<BLACK>(const NumEffectState &, Move, const CArray2d<int, 2, 9> &, MultiInt const&);
      template MultiInt PawnPtypeOPtypeO::
      evalWithUpdate<WHITE>(const NumEffectState &, Move, const CArray2d<int, 2, 9> &, MultiInt const&);

      template void osl::eval::ml::NonPawnAttacked::
      evalWithUpdateBang<BLACK>(const NumEffectState &state,
				Move moved,
				const CArray<PieceMask, 2> &effected,
				MultiIntPair &result);
      template void osl::eval::ml::NonPawnAttacked::
      evalWithUpdateBang<WHITE>(const NumEffectState &state,
				Move moved,
				const CArray<PieceMask, 2> &effected,
				MultiIntPair &result);
      template void osl::eval::ml::NonPawnAttackedPtype::
      evalWithUpdateBang<BLACK>(
	const NumEffectState &state,
	Move moved,
	const CArray<PieceMask, 2> &effected,
	CArray<PieceMask, 40> &attacked_mask,
	MultiIntPair &result);
      template void osl::eval::ml::NonPawnAttackedPtype::
      evalWithUpdateBang<WHITE>(
	const NumEffectState &state,
	Move moved,
	const CArray<PieceMask, 2> &effected,
	CArray<PieceMask, 40> &attacked_mask,
	MultiIntPair &result);
      template void osl::eval::ml::PtypeYPawnY::
      evalWithUpdateBang<BLACK>(const NumEffectState &state,
				Move moved,
				const CArray2d<int, 2, 9> &pawns,
				MultiInt& last_value);
      template void osl::eval::ml::PtypeYPawnY::
      evalWithUpdateBang<WHITE>(const NumEffectState &state,
				Move moved,
				const CArray2d<int, 2, 9> &pawns,
				MultiInt& last_value);
      template void PtypeCount::
      evalWithUpdateBang<BLACK>(const NumEffectState &state,Move last_move,
				CArray2d<int, 2, PTYPE_SIZE> &ptype_count,
				CArray2d<int, 2, PTYPE_SIZE> &ptype_board_count,
				MultiInt &last_value_and_out,
				unsigned int &ptypeo_mask);
      template void PtypeCount::
      evalWithUpdateBang<WHITE>(const NumEffectState &state,Move last_move,
				CArray2d<int, 2, PTYPE_SIZE> &ptype_count,
				CArray2d<int, 2, PTYPE_SIZE> &ptype_board_count,
				MultiInt &last_value_and_out,
				unsigned int &ptypeo_mask);

      template MultiIntPair KnightFork::
      evalWithUpdate<BLACK>(const NumEffectState&, Move, CArray<BoardMask,2>&,
			    CArray<std::pair<Square,int>,2>&);
      template MultiIntPair KnightFork::
      evalWithUpdate<WHITE>(const NumEffectState&, Move, CArray<BoardMask,2>&,
			    CArray<std::pair<Square,int>,2>&);
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
