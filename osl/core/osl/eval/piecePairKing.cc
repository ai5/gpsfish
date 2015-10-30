/* piecePairKing.cc
 */
#include "osl/eval/piecePairKing.h"
#include "osl/eval/weights.h"
#include <cstdint>

osl::CArray<int16_t, 1488375> osl::eval::ml::PiecePairKing::table;

void osl::eval::ml::
PiecePairKing::setUp(const Weights &weights)
{
  for (size_t i=0; i<weights.dimension(); ++i)
    table[i] = weights.value(i);

  for (int x=1; x<=5; ++x)
  {
    for (int y=1; y<=3; ++y)
    {
      bool flipx;
      const int king = indexKing(WHITE, Square(x,y), flipx);
      for (int i=0; i<45*7; ++i)
	for (int j=i+1; j<45*7; ++j)
	  table[composeIndex(king, j, i)] = table[composeIndex(king, i, j)];
    }
  }
}

osl::CArray<int,2> osl::eval::ml::
PiecePairKing::eval(const NumEffectState& state)
{
  CArray<int,2> ret;
  ret[BLACK] = evalOne<BLACK>(state);
  ret[WHITE] = evalOne<WHITE>(state);
  return ret;
}

template <osl::Player King>
int osl::eval::ml::
PiecePairKing::evalOne(const NumEffectState& state)
{
  FixedCapacityVector<Piece,38> pieces;
  if (state.template kingSquare<King>().template squareForBlack<King>().y() < 7)
    return 0;

  PieceMask bitset = state.piecesOnBoard(King) & ~state.promotedPieces();
  bitset.clearBit<KING>();
  while (! bitset.none()) 
  {
    const Piece p = state.pieceOf(bitset.takeOneBit());
    if (p.square().squareForBlack<King>().y() >= 5)
      pieces.push_back(p);
  }
  int sum = 0;
  bool flipx;
  const int index_king = indexKing(King, state.kingSquare(King), flipx);
  if (flipx)
  {
    for (size_t i=0; i<pieces.size(); ++i)
    {
      const unsigned int i0 = indexPiece<true>(King, pieces[i].square(), pieces[i].ptype());
      for (size_t j=i+1; j<pieces.size(); ++j)
      {
	const unsigned int i1 = indexPiece<true>(King, pieces[j].square(), pieces[j].ptype());
	const unsigned int index = composeIndex(index_king, i0, i1);
	sum += table[index];
      }
    }
  }
  else
  {
    for (size_t i=0; i<pieces.size(); ++i)
    {
      const unsigned int i0 = indexPiece<false>(King, pieces[i].square(), pieces[i].ptype());
      for (size_t j=i+1; j<pieces.size(); ++j)
      {
	const unsigned int i1 = indexPiece<false>(King, pieces[j].square(), pieces[j].ptype());
	const unsigned int index = composeIndex(index_king, i0, i1);
	sum += table[index];
      }
    }
  }
  return (King == BLACK) ? sum : -sum;
}

template <osl::Player King>
int osl::eval::ml::
PiecePairKing::add(const NumEffectState& state, Square to, Ptype ptype)
{
  const Square king = state.kingSquare(King);
  bool flipx;
  const int index_king = indexKing(King, king, flipx);
  int sum = 0;
  PieceMask bitset = state.piecesOnBoard(King) & ~state.promotedPieces();
  bitset.clearBit<KING>();
  unsigned int i0;
  if (flipx)
  {
    i0 = indexPiece<true>(King, to, ptype);
    while (! bitset.none()) 
    {
      const Piece p = state.pieceOf(bitset.takeOneBit());
      if (p.square().squareForBlack(King).y() < 5)
	continue;
      const unsigned int i1 = indexPiece<true>(King, p.square(), p.ptype());
      const unsigned int index = composeIndex(index_king, i0, i1);
      sum += table[index];
    }
  }
  else
  {
    i0 = indexPiece<false>(King, to, ptype);
    while (! bitset.none()) 
    {
      const Piece p = state.pieceOf(bitset.takeOneBit());
      if (p.square().squareForBlack(King).y() < 5)
	continue;
      const unsigned int i1 = indexPiece<false>(King, p.square(), p.ptype());
      const unsigned int index = composeIndex(index_king, i0, i1);
      sum += table[index];
    }
  }
  sum -= table[composeIndex(index_king, i0, i0)];
  return (King == BLACK) ? sum : -sum;
}
template <osl::Player King>
int osl::eval::ml::
PiecePairKing::sub(const NumEffectState& state, Square from, Ptype ptype)
{
  const Square king = state.kingSquare(King);
  bool flipx;
  const int index_king = indexKing(King, king, flipx);
  int sum = 0;
  PieceMask bitset = state.piecesOnBoard(King) & ~state.promotedPieces();
  bitset.clearBit<KING>();
  if (flipx)
  {
    const unsigned int i0 = indexPiece<true>(King, from, ptype);
    while (! bitset.none()) 
    {
      const Piece p = state.pieceOf(bitset.takeOneBit());
      if (p.square().squareForBlack(King).y() < 5)
	continue;
      const unsigned int i1 = indexPiece<true>(King, p.square(), p.ptype());
      const unsigned int index = composeIndex(index_king, i0, i1);
      sum -= table[index];
    }
  }
  else
  {
    const unsigned int i0 = indexPiece<false>(King, from, ptype);
    while (! bitset.none()) 
    {
      const Piece p = state.pieceOf(bitset.takeOneBit());
      if (p.square().squareForBlack(King).y() < 5)
	continue;
      const unsigned int i1 = indexPiece<false>(King, p.square(), p.ptype());
      const unsigned int index = composeIndex(index_king, i0, i1);
      sum -= table[index];
    }
  }
  return (King == BLACK) ? sum : -sum;
}
template <osl::Player King>
int osl::eval::ml::
PiecePairKing::addSub(const NumEffectState& state, Square to, Ptype ptype, Square from)
{
  const Square king = state.kingSquare(King);
  bool flipx;
  const int index_king = indexKing(King, king, flipx);
  unsigned int i0, s0;
  int sum = 0;
  PieceMask bitset = state.piecesOnBoard(King) & ~state.promotedPieces();
  bitset.clearBit<KING>();
  FixedCapacityVector<Piece,38> pieces;
  if (flipx)
  {
    i0 = indexPiece<true>(King, to, ptype);
    s0 = indexPiece<true>(King, from, ptype);
    while (! bitset.none()) 
    {
      const Piece p = state.pieceOf(bitset.takeOneBit());
      if (p.square().squareForBlack(King).y() < 5)
	continue;
      const unsigned int i1 = indexPiece<true>(King, p.square(), p.ptype());
      const unsigned int index = composeIndex(index_king, i0, i1);
      sum += table[index];
      const unsigned int sub_index = composeIndex(index_king, s0, i1);
      sum -= table[sub_index];
    }
  }
  else
  {
    i0 = indexPiece<false>(King, to, ptype);
    s0 = indexPiece<false>(King, from, ptype);
    while (! bitset.none()) 
    {
      const Piece p = state.pieceOf(bitset.takeOneBit());
      if (p.square().squareForBlack(King).y() < 5)
	continue;
      const unsigned int i1 = indexPiece<false>(King, p.square(), p.ptype());
      const unsigned int index = composeIndex(index_king, i0, i1);
      sum += table[index];
      const unsigned int sub_index = composeIndex(index_king, s0, i1);
      sum -= table[sub_index];
    }
  }
  sum -= table[composeIndex(index_king, i0, i0)];
  sum += table[composeIndex(index_king, s0, i0)];
  return (King == BLACK) ? sum : -sum;
}

template <osl::Player P>
void osl::eval::ml::
PiecePairKing::evalWithUpdateBang(const NumEffectState& state, Move moved, CArray<int,2>& last_value)
{
  assert(P == moved.player());
  if (moved.isPass())
    return;
  const Player Opponent = alt(P);
  const Ptype captured = moved.capturePtype();
  bool adjust_capture = (captured != PTYPE_EMPTY)
    && ! isPromoted(captured)
    && moved.to().squareForBlack(alt(P)).y() >= 5;
  if (adjust_capture)
  {
    const Square roking = state.kingSquare(alt(P)).squareForBlack(alt(P));
    adjust_capture = roking.y() >= 7;
  }
  if (moved.ptype() == KING)
  {
    last_value[P] = evalOne<P>(state);
    if (adjust_capture)
      last_value[alt(P)] += sub<Opponent>(state, moved.to(), captured);
    return;
  }
  const Square rking = state.kingSquare(P).squareForBlack(P);
  if (rking.y() < 7) 
  {
    if (adjust_capture)
      last_value[alt(P)] += sub<Opponent>(state, moved.to(), captured);
    return;
  }
  const Square rto = moved.to().squareForBlack(P);
  if (moved.isDrop())
  {
    if (rto.y() >= 5)
      last_value[P] += add<P>(state, moved.to(), moved.ptype());
    return;
  }
  const Square rfrom = moved.from().squareForBlack(P);
  if (adjust_capture)
    last_value[alt(P)] += sub<Opponent>(state, moved.to(), captured);

  if (isPromoted(moved.oldPtype()))
    return;
  if (rfrom.y() < 5)
  {
    if (rto.y() >= 5 && ! isPromoted(moved.ptype()))
      last_value[P] += add<P>(state, moved.to(), moved.ptype());
    return;
  }
  if (rto.y() < 5 || isPromoted(moved.ptype()))
    last_value[P] += sub<P>(state, moved.from(), moved.oldPtype());
  else
    last_value[P] += addSub<P>(state, moved.to(), moved.ptype(), moved.from());
}

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      template void PiecePairKing::evalWithUpdateBang<BLACK>(const NumEffectState&, Move, CArray<int,2>&);
      template void PiecePairKing::evalWithUpdateBang<WHITE>(const NumEffectState&, Move, CArray<int,2>&);
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
