#include "osl/eval/mobility.h"
#include "osl/mobility/rookMobility.h"
#include "osl/mobility/bishopMobility.h"
#include "osl/mobility/lanceMobility.h"

using osl::MultiInt;

osl::CArray<MultiInt, 18>
osl::eval::ml::RookMobilityAll::rook_vertical_table;
osl::CArray<MultiInt, 18>
osl::eval::ml::RookMobilityAll::rook_horizontal_table;
osl::CArray<MultiInt, 34>
osl::eval::ml::RookMobilityAll::sum_table;
osl::CArray<MultiInt, 324>
osl::eval::ml::RookMobilityAll::x_table;
osl::CArray<MultiInt, 324>
osl::eval::ml::RookMobilityAll::y_table;
osl::CArray<MultiInt, 17*9>
osl::eval::ml::RookMobilityAll::sumkingx_table;
osl::CArray<MultiInt, 9*2*5*9>
osl::eval::ml::RookMobilityAll::xkingx_table;

osl::CArray<MultiInt, 36>
osl::eval::ml::BishopMobilityAll::bishop_table;
osl::CArray<MultiInt, 18>
osl::eval::ml::BishopMobilityAll::each_table;

osl::CArray<MultiInt, 9>
osl::eval::ml::LanceMobility::lance_table;

void osl::eval::ml::
RookMobility::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < 9; ++i)
  {
    RookMobilityAll::rook_vertical_table[i][stage] = weights.value(i);
  }
  for (size_t i = 0; i < 9; ++i)
  {
    RookMobilityAll::rook_horizontal_table[i][stage] = weights.value(i + 9);
  }
  for (size_t i = 0; i < 9; ++i)
  {
    RookMobilityAll::rook_vertical_table[i+9][stage] = weights.value(i + 18);
  }
  for (size_t i = 0; i < 9; ++i)
  {
    RookMobilityAll::rook_horizontal_table[i+9][stage] = weights.value(i + 27);
  }
}

void osl::eval::ml::
RookMobilitySum::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      RookMobilityAll::sum_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
RookMobilityX::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      RookMobilityAll::x_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
RookMobilityY::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      RookMobilityAll::y_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
RookMobilityXKingX::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      RookMobilityAll::xkingx_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}
void osl::eval::ml::
RookMobilitySumKingX::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      RookMobilityAll::sumkingx_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <int Sign> inline
void osl::eval::ml::
RookMobilityAll::adjust(const NumEffectState& state,
			bool promoted, int vertical, int horizontal,
			Square position, MultiInt& value)
{
  int offset=0;
  if(promoted) offset=9;
  if(Sign>0)
  {
    if (! promoted) {
      const Square king = state.kingSquare<BLACK>();
      value += (xkingx_table[indexXKingX<Sign>(position, king, vertical, true)]
		+ xkingx_table[indexXKingX<Sign>(position, king, horizontal, false)]
		+ sumkingx_table[vertical + horizontal + 17*std::abs(king.x()-position.x())]
	);
    }
    value+= (rook_vertical_table[vertical+offset]+
	     rook_horizontal_table[horizontal+offset] +
	     sum_table[vertical+horizontal+(promoted ? 17 : 0)] +
	     x_table[indexX(position, promoted, vertical, true)] +
	     x_table[indexX(position, promoted, horizontal, false)] +
	     y_table[indexY<Sign>(position, promoted, vertical, true)] +
	     y_table[indexY<Sign>(position, promoted, horizontal, false)]);
  }
  else{ // Sign<0
    if (! promoted) {
      const Square king = state.kingSquare<WHITE>();
      value -= (xkingx_table[indexXKingX<Sign>(position, king, vertical, true)]
		+ xkingx_table[indexXKingX<Sign>(position, king, horizontal, false)]
		+ sumkingx_table[vertical + horizontal + 17*std::abs(king.x()-position.x())]
	);
    }
    value-= (rook_vertical_table[vertical+offset]+
	     rook_horizontal_table[horizontal+offset] +
	     sum_table[vertical+horizontal+(promoted ? 17 : 0)] +
	     x_table[indexX(position, promoted, vertical, true)] +
	     x_table[indexX(position, promoted, horizontal, false)] +
	     y_table[indexY<Sign>(position, promoted, vertical, true)] +
	     y_table[indexY<Sign>(position, promoted, horizontal, false)]);
  }
}

void osl::eval::ml::
RookMobilityAll::eval(const NumEffectState& state, MultiInt& out)
{
  out.clear();
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece rook = state.pieceOf(i);
    if (! rook.isOnBoard())
      continue;
    if (rook.owner() == BLACK)
    {
      const int vertical = osl::mobility::RookMobility::countVerticalAll<BLACK>(state,i);
      const int horizontal = osl::mobility::RookMobility::countHorizontalAll<BLACK>(
	state, i);
      adjust<1>(state, rook.isPromoted(), vertical, horizontal, rook.square(), out);
    }
    else
    {
      const int vertical = osl::mobility::RookMobility::countVerticalAll<WHITE>(state,i);
      const int horizontal = osl::mobility::RookMobility::countHorizontalAll<WHITE>(
	state, i);
      adjust<-1>(state, rook.isPromoted(), vertical, horizontal, rook.square(), out);
    }
  }
}




void osl::eval::ml::
BishopMobility::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < 18; ++i)
  {
    BishopMobilityAll::bishop_table[i][stage] = weights.value(i);
  }
  for (size_t i = 0; i < 18; ++i)
  {
    BishopMobilityAll::bishop_table[i+18][stage] = weights.value(i + 18);
  }
}

void osl::eval::ml::
BishopMobilityEach::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      BishopMobilityAll::each_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <int Sign> inline
void osl::eval::ml::
BishopMobilityAll::adjust(bool promoted, int mobility1, int mobility2,
			  MultiInt& value)
{
  int count=0;
  int each_offset = 0;
  if(promoted)
  {
    count=18;
    each_offset = 9;
  }
  if(Sign>0)
  {
    value += (bishop_table[mobility1 + mobility2 + count] +
	      each_table[mobility1 + each_offset] +
	      each_table[mobility2 + each_offset]);
  }
  else
  {
    value -= (bishop_table[mobility1 + mobility2 + count] +
	      each_table[mobility1 + each_offset] +
	      each_table[mobility2 + each_offset]);
  }
}

void osl::eval::ml::
BishopMobilityAll::eval(const NumEffectState& state, MultiInt& out)
{
  out.clear();
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece bishop = state.pieceOf(i);
    if (! bishop.isOnBoard())
      continue;
    if (bishop.owner() == BLACK)
    {
      const int mobility1 =
	mobility::BishopMobility::countAllDir<BLACK, UL>(state, bishop) +
	mobility::BishopMobility::countAllDir<BLACK, DR>(state, bishop);
      const int mobility2 =
	mobility::BishopMobility::countAllDir<BLACK, UR>(state, bishop) +
	mobility::BishopMobility::countAllDir<BLACK, DL>(state, bishop);
      adjust<1>(bishop.isPromoted(), mobility1, mobility2, out);
    }
    else
    {
      const int mobility1 =
	mobility::BishopMobility::countAllDir<WHITE, UL>(state, bishop) +
	mobility::BishopMobility::countAllDir<WHITE, DR>(state, bishop);
      const int mobility2 =
	mobility::BishopMobility::countAllDir<WHITE, UR>(state, bishop) +
	mobility::BishopMobility::countAllDir<WHITE, DL>(state, bishop);
      adjust<-1>(bishop.isPromoted(), mobility1, mobility2, out);
    }
  }
}



void osl::eval::ml::
LanceMobility::setUp(const Weights &weights,int stage)
{
  for (size_t i = 0; i < 9; ++i)
  {
    lance_table[i][stage] = weights.value(i);
  }
}

template <int Sign> inline
void osl::eval::ml::
LanceMobilityAll::adjust(int mobility, MultiInt& value)
{
  if(Sign>0)
    value += LanceMobility::lance_table[mobility];
  else
    value -= LanceMobility::lance_table[mobility];
}

void osl::eval::ml::
LanceMobilityAll::eval(const NumEffectState &state, MultiInt& out)
{
  out.clear();
  for (int i = PtypeTraits<LANCE>::indexMin;
       i < PtypeTraits<LANCE>::indexLimit;
       ++i)
  {
    const Piece lance = state.pieceOf(i);
    if (!lance.isOnBoardNotPromoted())
      continue;
    if (lance.pieceIsBlack())
    {
      const int mobility = osl::mobility::LanceMobility::countAll<BLACK>(
	state, lance.square(),i);
      adjust<1>(mobility, out);
    }
    else
    {
      const int mobility = osl::mobility::LanceMobility::countAll<WHITE>(
	state, lance.square(),i);
      adjust<-1>(mobility, out);
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
