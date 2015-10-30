#include "osl/eval/pieceStand.h"
#include "osl/bits/pieceStand.h"

osl::CArray<osl::MultiInt, osl::Piece::SIZE>
osl::eval::ml::PieceStand::table;

void osl::eval::ml::
PieceStand::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i][stage] = weights.value(i);
  }
}

osl::MultiInt osl::eval::ml::PieceStand::eval(
  const NumEffectState &state)
{
  MultiInt result;
  for (Ptype ptype: osl::PieceStand::order)
  {
    const int black_count =
      state.countPiecesOnStand(BLACK, ptype);
    const int white_count =
      state.countPiecesOnStand(WHITE, ptype);
    for (int j = 0; j < black_count; ++j)
    {
      result += table[Ptype_Table.getIndexMin(ptype) + j];
    }
    for (int j = 0; j < white_count; ++j)
    {
      result -= table[Ptype_Table.getIndexMin(ptype) + j];
    }
  }
  return result;
}



osl::CArray<osl::MultiInt, 21>
osl::eval::ml::NonPawnPieceStand::table;

void osl::eval::ml::
NonPawnPieceStand::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i][stage] = weights.value(i);
  }
}

osl::MultiInt osl::eval::ml::
NonPawnPieceStand::eval(int black_count, int white_count)
{
  return table[black_count] - table[white_count];
}


osl::CArray<osl::MultiInt, 5625> osl::eval::ml::NonPawnPieceStandCombination::table;
osl::CArray<osl::MultiInt, 5625> osl::eval::ml::NonPawnPieceStandCombination::check_table;

osl::MultiInt osl::eval::ml::
NonPawnPieceStandCombination::sumUp(const CArray<int, 6> &indices,
				    const CArray<MultiInt, 5625> &values)
{
  osl::MultiInt result;
  for (int rook = 0; rook <= indices[0]; ++rook)
  {
    for (int bishop = 0; bishop <= indices[1]; ++bishop)
    {
      for (int gold = 0; gold <= indices[2]; ++gold)
      {
	for (int silver = 0; silver <= indices[3]; ++silver)
	{
	  for (int knight = 0; knight <= indices[4]; ++knight)
	  {
	    for (int lance = 0; lance <= indices[5]; ++lance)
	    {
	      if (rook + bishop + gold + silver + knight + lance == 0)
	      {
		continue;
	      }
	      result += values[index(rook, bishop,
				     gold, silver, knight, lance)];
	    }
	  }
	}
      }
    }
  }
  return result;
}

void osl::eval::ml::
NonPawnPieceStandCombination::setUp(const Weights &weights)
{
  CArray<MultiInt, 5625> orig_table;
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
    {
      orig_table[i][s] = weights.value(i + ONE_DIM*s);
    }
  }
  CArray<int, 6> indices;
  for (indices[0] = 0; indices[0] <= 2; ++indices[0])
  {
    for (indices[1] = 0; indices[1] <= 2; ++indices[1])
    {
      for (indices[2] = 0; indices[2] <= 4; ++indices[2])
      {
	for (indices[3] = 0; indices[3] <= 4; ++indices[3])
	{
	  for (indices[4] = 0; indices[4] <= 4; ++indices[4])
	  {
	    for (indices[5] = 0; indices[5] <= 4; ++indices[5])
	    {
	      table[index(indices[0],
			  indices[1],
			  indices[2],
			  indices[3],
			  indices[4],
			  indices[5])] = sumUp(indices, orig_table);
	    }
	  }
	}
      }
    }
  }
  table[0] = orig_table[0];
}

void osl::eval::ml::
CanCheckNonPawnPieceStandCombination::setUp(const Weights &weights)
{
  CArray<MultiInt, 5625> orig_table;
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
    {
      orig_table[i][s] = weights.value(i + ONE_DIM*s);
    }
  }
  CArray<int, 6> indices;
  for (indices[0] = 0; indices[0] <= 2; ++indices[0])
  {
    for (indices[1] = 0; indices[1] <= 2; ++indices[1])
    {
      for (indices[2] = 0; indices[2] <= 4; ++indices[2])
      {
	for (indices[3] = 0; indices[3] <= 4; ++indices[3])
	{
	  for (indices[4] = 0; indices[4] <= 4; ++indices[4])
	  {
	    for (indices[5] = 0; indices[5] <= 4; ++indices[5])
	    {
	      NonPawnPieceStandCombination::check_table[
		NonPawnPieceStandCombination::index(indices[0],
						    indices[1],
						    indices[2],
						    indices[3],
						    indices[4],
						    indices[5])] =
		NonPawnPieceStandCombination::sumUp(indices, orig_table);
	    }
	  }
	}
      }
    }
  }
  NonPawnPieceStandCombination::check_table[0] = orig_table[0];
}

osl::MultiInt osl::eval::ml::
NonPawnPieceStandCombination::eval(const NumEffectState &state,
				   const CArray<bool, 2> &can_check)
{
  const int black_index = index(state.countPiecesOnStand<ROOK>(BLACK),
				state.countPiecesOnStand<BISHOP>(BLACK),
				state.countPiecesOnStand<GOLD>(BLACK),
				state.countPiecesOnStand<SILVER>(BLACK),
				state.countPiecesOnStand<KNIGHT>(BLACK),
				state.countPiecesOnStand<LANCE>(BLACK));
  const int white_index = index(state.countPiecesOnStand<ROOK>(WHITE),
				state.countPiecesOnStand<BISHOP>(WHITE),
				state.countPiecesOnStand<GOLD>(WHITE),
				state.countPiecesOnStand<SILVER>(WHITE),
				state.countPiecesOnStand<KNIGHT>(WHITE),
				state.countPiecesOnStand<LANCE>(WHITE));
  MultiInt result;
  result = table[black_index] - table[white_index];
  if (can_check[WHITE])
  {
    result += check_table[black_index];
  }
  if (can_check[BLACK])
  {
    result -= check_table[white_index];
  }
  return result;
}

osl::MultiInt osl::eval::ml::
NonPawnPieceStandCombination::evalWithUpdate(
  const NumEffectState &state,
  Move moved,
  const MultiInt &last_value,
  const CArray<bool, 2> &could_check,
  const CArray<bool, 2> &can_check)
{
  if (!moved.isDrop() && ! moved.isCapture() &&
      could_check[0] == can_check[0] && could_check[1] == can_check[1])
  {
    return last_value;
  }
  return eval(state, can_check);
}


osl::CArray<osl::MultiInt, 44> osl::eval::ml::NonPawnPieceStandTurn::table;

void osl::eval::ml::
NonPawnPieceStandTurn::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
NonPawnPieceStandTurn::eval(const NumEffectState &state, MultiIntPair& result)
{
  result = MultiIntPair();
  for (Ptype ptype: osl::PieceStand::order)
  {
    if (ptype == PAWN)
      continue;
    const int black_count = state.countPiecesOnStand(BLACK, ptype);
    const int white_count = state.countPiecesOnStand(WHITE, ptype);
    for (int j = 0; j < black_count; ++j)
    {
      const int index_black = index(BLACK, BLACK, ptype, j);
      const int index_white = index(BLACK, WHITE, ptype, j);
      result[BLACK] += table[index_black];
      result[WHITE] += table[index_white];
    }
    for (int j = 0; j < white_count; ++j)
    {
      const int index_black = index(WHITE, BLACK, ptype, j);
      const int index_white = index(WHITE, WHITE, ptype, j);
      result[BLACK] -= table[index_black];
      result[WHITE] -= table[index_white];
    }
  }
}

template<osl::Player P>
void osl::eval::ml::
NonPawnPieceStandTurn::evalWithUpdateBang(
  const NumEffectState &state,
  Move moved, MultiIntPair &result)
{
  assert(P==moved.player());
  if (!moved.isDrop() && ! moved.isCapture())
    return;

  if (moved.isDrop())
  {
    const Ptype ptype = moved.ptype();
    if (ptype == PAWN)
      return;
    const int count =
      state.countPiecesOnStand(P, moved.ptype());
    const int index_black = index(P, BLACK, moved.ptype(), count);
    const int index_white = index(P, WHITE, moved.ptype(), count);
    if(P==BLACK){
      result[BLACK] -= table[index_black];
      result[WHITE] -= table[index_white];
    }
    else{
      result[BLACK] += table[index_black];
      result[WHITE] += table[index_white];
    }
  }
  if (moved.isCapture() &&
      unpromote(moved.capturePtype()) != PAWN)
  {
    Ptype ptype = unpromote(moved.capturePtype());
    const int count = state.countPiecesOnStand(P, ptype) - 1;
    const int index_black = index(P, BLACK, ptype, count);
    const int index_white = index(P, WHITE, ptype, count);
    if(P==BLACK){
      result[BLACK] += table[index_black];
      result[WHITE] += table[index_white];
    }
    else{
      result[BLACK] -= table[index_black];
      result[WHITE] -= table[index_white];
    }
  }
}


osl::CArray<osl::MultiInt, 360> osl::eval::ml::PieceStandY::y_attack_table;
osl::CArray<osl::MultiInt, 360> osl::eval::ml::PieceStandY::y_defense_table;;
osl::CArray<osl::MultiInt, 9*7*19> osl::eval::ml::PieceStandY::y_attack_table_sum;
osl::CArray<osl::MultiInt, 9*7*19> osl::eval::ml::PieceStandY::y_defense_table_sum;

void osl::eval::ml::
PieceStandY::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s) 
    {
      y_attack_table[i][s] = weights.value(i + ONE_DIM * 2 * s);
      y_defense_table[i][s] = weights.value(i + ONE_DIM * 2 * s + ONE_DIM);
    }
  }
  for (int i=0;i<7;i++){
    Ptype ptype=osl::PieceStand::order[i];
    int ptypeSize=Ptype_Table.getIndexLimit(ptype)-Ptype_Table.getIndexMin(ptype);
    for(int king_y=1;king_y<=9;king_y++){
      MultiInt attack_sum, defense_sum;
      for(int count=0;count<=ptypeSize;count++){
#if 0
	int oldIndex=(king_y - 1) * 40 + Ptype_Table.getIndexMin(ptype) + count;
	int newIndex=(king_y - 1) * 7*19 + i*19 + count;
#else
	int oldIndex=index(ptype,BLACK,Square(5,king_y),count);
	int newIndex=index(i,BLACK,Square(5,king_y),count);
#endif
	y_attack_table_sum[newIndex]=attack_sum;
	y_defense_table_sum[newIndex]=defense_sum;
	if(count==ptypeSize) break;
	attack_sum += y_attack_table[oldIndex];
	defense_sum += y_defense_table[oldIndex];
      }
    }
  }
}

inline 
void osl::eval::ml::PieceStandY::updateResult(NumEffectState const& state,osl::MultiInt &result,int i, osl::Ptype ptype, osl::CArray<osl::Square,2> const&kings)
{
  const int black_count = state.countPiecesOnStand(BLACK, ptype);
  const int white_count = state.countPiecesOnStand(WHITE, ptype);
  const int attack_index_1 = PieceStandY::index(i, BLACK, kings[WHITE], black_count);
  const int attack_index_2 = PieceStandY::index(i, WHITE, kings[BLACK], white_count);
  const int defense_index_1 = PieceStandY::index(i, BLACK, kings[BLACK], black_count);
  const int defense_index_2 = PieceStandY::index(i, WHITE, kings[WHITE], white_count);
  result += y_attack_table_sum[attack_index_1] - y_attack_table_sum[attack_index_2] +
    y_defense_table_sum[defense_index_1] - y_defense_table_sum[defense_index_2];
}

osl::MultiInt osl::eval::ml::
PieceStandY::eval(const NumEffectState &state)
{
  MultiInt result;
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  updateResult(state,result,0,ROOK,kings);
  updateResult(state,result,1,BISHOP,kings);
  updateResult(state,result,2,GOLD,kings);
  updateResult(state,result,3,SILVER,kings);
  updateResult(state,result,4,KNIGHT,kings);
  updateResult(state,result,5,LANCE,kings);
  updateResult(state,result,6,PAWN,kings);
  return result;
}

template<osl::Player P>
osl::MultiInt osl::eval::ml::
PieceStandY::evalWithUpdate(
  const NumEffectState &state,
  Move moved, const MultiInt &last_value)
{
  if (moved.ptype() == KING)
    return eval(state);

  MultiInt result(last_value);
  if (moved.isDrop())
  {
    const Ptype ptype = moved.ptype();
    const int count =
      state.countPiecesOnStand(P, ptype);
    const int attack_index = index(ptype, P,
				   state.kingSquare(alt(P)),
				   count);
    const int defense_index = index(ptype, P,
				    state.kingSquare(P),
				    count); 
    if(P==BLACK)
      result -= y_attack_table[attack_index] +y_defense_table[defense_index];
    else
      result += y_attack_table[attack_index] +y_defense_table[defense_index];
  }
  if (moved.isCapture())
  {
    Ptype ptype = unpromote(moved.capturePtype());
    const int count = state.countPiecesOnStand(P, ptype)-1;
    const int attack_index = index(ptype, P,
				   state.kingSquare(alt(P)),
				   count);
    const int defense_index = index(ptype, P,
				    state.kingSquare(P),
				    count);
    if(P==BLACK)
      result += y_attack_table[attack_index] +y_defense_table[defense_index];
    else
      result -= y_attack_table[attack_index] +y_defense_table[defense_index];
  }
  return result;
}

osl::CArray<osl::MultiInt, 16384> osl::eval::ml::PieceStandCombinationBoth::table;

void osl::eval::ml::
PieceStandCombinationBoth::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    int low = (i & 0x7F);
    int high = (i >> 7);
    if (low == high)
      continue;
    for (int s = 0; s < NStages; ++s)
    {
      table[i][s] = weights.value(i + ONE_DIM*s);
      if (high > low)
      {
	table[(low << 7) | high][s] = -table[i][s];
      }
    }
  }
}

osl::MultiInt osl::eval::ml::
PieceStandCombinationBoth::eval(const NumEffectState &state)
{
  int black_index = 0;
  int white_index = 0;
  black_index |= ((state.hasPieceOnStand<ROOK>(BLACK) ? 1 : 0) << 6);
  black_index |= ((state.hasPieceOnStand<BISHOP>(BLACK) ? 1 : 0) << 5);
  black_index |= ((state.hasPieceOnStand<GOLD>(BLACK) ? 1 : 0) << 4);
  black_index |= ((state.hasPieceOnStand<SILVER>(BLACK) ? 1 : 0) << 3);
  black_index |= ((state.hasPieceOnStand<KNIGHT>(BLACK) ? 1 : 0) << 2);
  black_index |= ((state.hasPieceOnStand<LANCE>(BLACK) ? 1 : 0) << 1);
  black_index |= ((state.hasPieceOnStand<PAWN>(BLACK) ? 1 : 0) << 0);
  white_index |= ((state.hasPieceOnStand<ROOK>(WHITE) ? 1 : 0) << 6);
  white_index |= ((state.hasPieceOnStand<BISHOP>(WHITE) ? 1 : 0) << 5);
  white_index |= ((state.hasPieceOnStand<GOLD>(WHITE) ? 1 : 0) << 4);
  white_index |= ((state.hasPieceOnStand<SILVER>(WHITE) ? 1 : 0) << 3);
  white_index |= ((state.hasPieceOnStand<KNIGHT>(WHITE) ? 1 : 0) << 2);
  white_index |= ((state.hasPieceOnStand<LANCE>(WHITE) ? 1 : 0) << 1);
  white_index |= ((state.hasPieceOnStand<PAWN>(WHITE) ? 1 : 0) << 0);
  return table[(black_index << 7) | white_index];
}


namespace osl
{
  namespace eval
  {
    namespace ml
    {
      template void NonPawnPieceStandTurn::evalWithUpdateBang<BLACK>(const NumEffectState &, Move, MultiIntPair &);
      template void NonPawnPieceStandTurn::evalWithUpdateBang<WHITE>(const NumEffectState &, Move, MultiIntPair &);
      template MultiInt PieceStandY::evalWithUpdate<BLACK>(const NumEffectState &, Move, const MultiInt &);
      template MultiInt PieceStandY::evalWithUpdate<WHITE>(const NumEffectState &, Move, const MultiInt &);
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
