#include "osl/eval/pin.h"
using osl::MultiInt;

osl::CArray<int, osl::PTYPE_SIZE>
osl::eval::ml::SimplePin::table;

osl::CArray2d<MultiInt, osl::PTYPE_SIZE, 17 * 9>
osl::eval::ml::Pin::table;

void osl::eval::ml::
SimplePin::setUp(const Weights &weights)
{
  table.fill(0);
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}

int osl::eval::ml::
SimplePin::eval(const NumEffectState &state,
		PieceMask black_mask, PieceMask white_mask) const
{
  int value = 0;
  while (black_mask.any())
  {
    const osl::Piece piece = state.pieceOf(black_mask.takeOneBit());
    value += table[piece.ptype()];
  }
  while (white_mask.any())
  {
    const osl::Piece piece = state.pieceOf(white_mask.takeOneBit());
    value -= table[piece.ptype()];
  }
  return value;
}



void osl::eval::ml::
Pin::setUp(const Weights &weights,int stage)
{
  for (int i = PTYPE_PIECE_MIN; i <= PTYPE_MAX; ++i)
  {
    for (int y = 0; y <= 16; ++y)
    {
      for (int x = 0; x <= 8; ++x)
      {
	const int distance = x * 17 + y;
	table[i][distance][stage] =
	  weights.value((i - PTYPE_PIECE_MIN) * 17 * 9 + distance);
      }
    }
  }
}

MultiInt osl::eval::ml::
Pin::eval(const NumEffectState &state,
	  PieceMask black_mask, PieceMask white_mask)
{
  MultiInt value;
  const Square black_king = state.kingSquare<BLACK>();
  const Square white_king = state.kingSquare<WHITE>();
  while (black_mask.any())
  {
    const osl::Piece piece = state.pieceOf(black_mask.takeOneBit());
    value += table[piece.ptype()][index(black_king, piece)];
  }
  while (white_mask.any())
  {
    const osl::Piece piece = state.pieceOf(white_mask.takeOneBit());
    value -= table[piece.ptype()][index(white_king, piece)];
  }
  return value;
}

osl::CArray<MultiInt, 80>
osl::eval::ml::PinPtypeAll::table;
osl::CArray<MultiInt, 48>
osl::eval::ml::PinPtypeAll::pawn_table;
osl::CArray<MultiInt, 560>
osl::eval::ml::PinPtypeAll::distance_table;

void osl::eval::ml::
PinPtype::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
PinPtypeDistance::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      distance_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

void osl::eval::ml::
PinPtypePawnAttack::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      pawn_table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <osl::Player Defense>
osl::MultiInt osl::eval::ml::
PinPtypeAll::evalOne(const NumEffectState &state)
{
  MultiInt result;
  const Square king = state.kingSquare<Defense>();
  PieceMask pin_mask = state.pin(Defense);
  while (pin_mask.any())
  {
    const Piece piece = state.pieceOf(pin_mask.takeOneBit());
    if (!state.hasEffectAt<Defense>(piece.square()))
      continue;
    if (king.y() == piece.square().y()) // rook h
    {
      result +=
	(distance_table[(piece.ptype() + PTYPE_SIZE * 1) * 7 +
			std::abs(king.x() - piece.square().x()) - 1] +
	 table[(piece.ptype() + PTYPE_SIZE * 1)]);
      if (pawnAttack<Defense>(state, piece))
      {
	result += pawn_table[(piece.ptype() + PTYPE_SIZE * 0)];
      }
    }
    else if (king.x() == piece.square().x())
    {
      if (state.hasEffectByPtypeStrict<LANCE>(alt(Defense),
					      piece.square())) // lance
      {
	result +=
	  (distance_table[(piece.ptype() + PTYPE_SIZE * 4) * 7 +
			  std::abs(king.y() - piece.square().y()) - 1] +
	   table[piece.ptype() + PTYPE_SIZE * 4]);
      }
      else // rook v
      {
	result +=
	  (distance_table[(piece.ptype() + PTYPE_SIZE * 0) * 7 +
			  std::abs(king.y() - piece.square().y()) - 1] +
	   table[piece.ptype() + PTYPE_SIZE * 0]);
      }
    }
    else // bishop
    {
      if ((Defense == BLACK && piece.square().y() < king.y()) ||
	  (Defense == WHITE && piece.square().y() > king.y())) // u
      {
	result +=
	  (distance_table[(piece.ptype() + PTYPE_SIZE * 2) * 7 +
			  std::abs(king.x() - piece.square().x()) - 1] +
	   table[piece.ptype() + PTYPE_SIZE * 2]);
	if (pawnAttack<Defense>(state, piece))
	{
	  result += pawn_table[(piece.ptype() + PTYPE_SIZE * 1)];
	}
      }
      else
      {
	result +=
	  (distance_table[(piece.ptype() + PTYPE_SIZE * 3) * 7 +
			  std::abs(king.x() - piece.square().x()) - 1] +
	   table[piece.ptype() + PTYPE_SIZE * 3]);
	if (pawnAttack<Defense>(state, piece))
	{
	  result += pawn_table[(piece.ptype() + PTYPE_SIZE * 2)];
	}
      }
    }
  }
  return result;
}

osl::MultiInt osl::eval::ml::
PinPtypeAll::eval(const NumEffectState &state)
{
  return evalOne<BLACK>(state) - evalOne<WHITE>(state);
}


osl::CArray<MultiInt, 80>
osl::eval::ml::CheckShadowPtype::table;

void osl::eval::ml::
CheckShadowPtype::setUp(const Weights &weights)
{
  for (size_t i = 0; i < ONE_DIM; ++i)
  {
    for (int s=0; s<NStages; ++s)
      table[i][s] = weights.value(i + ONE_DIM*s);
  }
}

template <osl::Player Defense>
osl::MultiInt osl::eval::ml::
CheckShadowPtype::evalOne(const NumEffectState &state) 
{
  MultiInt result;
  const Square king = state.kingSquare<Defense>();
  PieceMask open_mask = state.checkShadow(alt(Defense));
  while (open_mask.any())
  {
    const Piece piece = state.pieceOf(open_mask.takeOneBit());
    if (king.y() == piece.square().y()) // rook h
    {
      result += table[piece.ptype() + PTYPE_SIZE * 1];
    }
    else if (king.x() == piece.square().x())
    {
      if (state.hasEffectByPtypeStrict<LANCE>(alt(Defense),
						piece.square())) // lance
      {
	result += table[piece.ptype() + PTYPE_SIZE * 4];
      }
      else // rook v
      {
	result += table[piece.ptype() + PTYPE_SIZE * 0];
      }
    }
    else // bishop
    {
      if ((Defense == BLACK && piece.square().y() < king.y()) ||
	  (Defense == WHITE && piece.square().y() > king.y())) // u
      {
	result += table[piece.ptype() + PTYPE_SIZE * 2];
      }
      else
      {
	result += table[piece.ptype() + PTYPE_SIZE * 3];
      }
    }
  }
  return result;
}

osl::MultiInt osl::eval::ml::
CheckShadowPtype::eval(const NumEffectState &state)
{
  return evalOne<BLACK>(state) - evalOne<WHITE>(state);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
