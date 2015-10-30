/* piecePairKing.cc
 */
#include "eval/piecePairKing.h"

void gpsshogi::PiecePairKing::
featuresOneNonUniq(const NumEffectState &state,
		   index_list_t &feature_count) const
{
  FixedCapacityVector<Piece,38> pieces;
  if (state.kingSquare(BLACK).y() >= 7)
  {
    PieceMask black = state.piecesOnBoard(BLACK) & ~state.promotedPieces();
    black.clearBit<KING>();
    while (! black.none()) 
    {
      const Piece p = state.pieceOf(black.takeOneBit());
      if (p.square().y() >= 5)
	pieces.push_back(p);
    }
    bool flipx;
    const int index_king = indexKing(BLACK, state.kingSquare(BLACK), flipx);
    for (size_t i=0; i<pieces.size(); ++i)
    {
      const unsigned int i0 = indexPiece(BLACK, pieces[i].square(), pieces[i].ptype(), flipx);
      for (size_t j=i+1; j<pieces.size(); ++j)
      {
	const unsigned int i1 = indexPiece(BLACK, pieces[j].square(), pieces[j].ptype(), flipx);
	const unsigned int index = composeIndex(index_king, i0, i1);
	assert(index == indexBlack(state.kingSquare(BLACK), pieces[i], pieces[j]));
	assert(index < dimension());
	feature_count.add(index, 1);
      }
    }
  }
  if (state.kingSquare(WHITE).y() <= 3)
  {  
    PieceMask white = state.piecesOnBoard(WHITE) & ~state.promotedPieces();
    white.clearBit<KING>();
    pieces.clear();
    while (! white.none()) 
    {
      const Piece p = state.pieceOf(white.takeOneBit());
      if (p.square().y() <= 5)
	pieces.push_back(p);
    }
    bool flipx;
    const int index_king = indexKing(WHITE, state.kingSquare(WHITE), flipx);
    for (size_t i=0; i<pieces.size(); ++i)
    {
      const unsigned int i0 = indexPiece(WHITE, pieces[i].square(), pieces[i].ptype(), flipx);
      for (size_t j=i+1; j<pieces.size(); ++j)
      {
	const unsigned int i1 = indexPiece(WHITE, pieces[j].square(), pieces[j].ptype(), flipx);
	const unsigned int index = composeIndex(index_king, i0, i1);
	assert(index == indexWhite(state.kingSquare(WHITE), pieces[i], pieces[j]));
	assert(index < dimension());
	feature_count.add(index, -1);
      }
    }
  }
}

gpsshogi::PiecePairKingFlat::~PiecePairKingFlat()
{
}

int gpsshogi::PiecePairKingFlat::
eval(const NumEffectState& state) const
{
  index_list_t features;
  feature.featuresOneNonUniq(state, features);
  int ret=0;
  for (size_t i=0; i<features.size(); ++i)
    ret += value(features[i].first) * features[i].second;
  return ret;
}

int gpsshogi::PiecePairKingFlat::
add(const NumEffectState& state, Player player, Square to, Ptype ptype) const
{
  const Square king = state.kingSquare(player);
  bool flipx;
  const int index_king = feature.indexKing(player, king, flipx);
  const unsigned int i0 = feature.indexPiece(player, to, ptype, flipx);
  int sum = 0;
  PieceMask bitset = state.piecesOnBoard(player) & ~state.promotedPieces();
  bitset.clearBit<KING>();
  while (! bitset.none()) 
  {
    const Piece p = state.pieceOf(bitset.takeOneBit());
    if (p.square().squareForBlack(player).y() < 5)
      continue;
    const unsigned int i1 = feature.indexPiece(player, p.square(), p.ptype(), flipx);
    const unsigned int index = feature.composeIndex(index_king, i0, i1);
    sum += value(index);
  }
  sum -= value(feature.composeIndex(index_king, i0, i0));
  return (player == BLACK) ? sum : -sum;
}

int gpsshogi::PiecePairKingFlat::
sub(const NumEffectState& state, Player player, Square from, Ptype ptype) const
{
  const Square king = state.kingSquare(player);
  bool flipx;
  const int index_king = feature.indexKing(player, king, flipx);
  const unsigned int i0 = feature.indexPiece(player, from, ptype, flipx);
  int sum = 0;
  PieceMask bitset = state.piecesOnBoard(player) & ~state.promotedPieces();
  bitset.clearBit<KING>();
  while (! bitset.none()) 
  {
    const Piece p = state.pieceOf(bitset.takeOneBit());
    if (p.square().squareForBlack(player).y() < 5)
      continue;
    const unsigned int i1 = feature.indexPiece(player, p.square(), p.ptype(), flipx);
    const unsigned int index = feature.composeIndex(index_king, i0, i1);
    sum -= value(index);
  }
  return (player == BLACK) ? sum : -sum;
}

int gpsshogi::PiecePairKingFlat::
addSub(const NumEffectState& state, Player player, Square to, Ptype ptype, Square from) const
{
  const Square king = state.kingSquare(player);
  bool flipx;
  const int index_king = feature.indexKing(player, king, flipx);
  const unsigned int i0 = feature.indexPiece(player, to, ptype, flipx);
  const unsigned int s0 = feature.indexPiece(player, from, ptype, flipx);
  int sum = 0;
  PieceMask bitset = state.piecesOnBoard(player) & ~state.promotedPieces();
  bitset.clearBit<KING>();
  FixedCapacityVector<Piece,38> pieces;
  while (! bitset.none()) 
  {
    const Piece p = state.pieceOf(bitset.takeOneBit());
    if (p.square().squareForBlack(player).y() < 5)
      continue;
    const unsigned int i1 = feature.indexPiece(player, p.square(), p.ptype(), flipx);
    const unsigned int index = feature.composeIndex(index_king, i0, i1);
    sum += value(index);
    const unsigned int sub_index = feature.composeIndex(index_king, s0, i1);
    sum -= value(sub_index);
  }
  sum -= value(feature.composeIndex(index_king, i0, i0));
  sum += value(feature.composeIndex(index_king, s0, i0));
  return (player == BLACK) ? sum : -sum;
}

int gpsshogi::PiecePairKingFlat::
evalWithUpdate(const NumEffectState& state, Move moved, int last_value) const
{
  if (moved.isPass())
    return last_value;
  const Player player = moved.player();
  if (moved.ptype() == KING)	// osl ではking別に分けると手間が半分に
    return PiecePairKingFlat::eval(state);
  const Square rking = state.kingSquare(player).squareForBlack(player);
  if (rking.y() < 7) 
  {
    if (moved.capturePtype() != PTYPE_EMPTY && ! isPromoted(moved.capturePtype()))
    {
      const Square roking = state.kingSquare(alt(player)).squareForBlack(alt(player));
      if (roking.y() >= 7 && moved.to().squareForBlack(alt(player)).y() >= 5)
	return last_value + sub(state, alt(player), moved.to(), moved.capturePtype());
    }
    return last_value;
  }
  const Square rto = moved.to().squareForBlack(player);
  if (moved.isDrop())
  {
    if (rto.y() < 5)
      return last_value;
    return last_value + add(state, player, moved.to(), moved.ptype());
  }
  const Square rfrom = moved.from().squareForBlack(player);
  const Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY && ! isPromoted(captured))
  {
    const Player opponent = alt(player);
    const Square roking = state.kingSquare(opponent).squareForBlack(opponent);
    if (roking.y() >= 7 && moved.to().squareForBlack(opponent).y() >= 5)
      last_value += sub(state, opponent, moved.to(), moved.capturePtype());
  }

  if (isPromoted(moved.oldPtype()))
    return last_value;
  if (rfrom.y() < 5)
  {
    if (rto.y() < 5 || isPromoted(moved.ptype()))
      return last_value;
    return last_value + add(state, player, moved.to(), moved.ptype());
  }
  if (rto.y() < 5 || isPromoted(moved.ptype()))
    return last_value + sub(state, player, moved.from(), moved.oldPtype());
  assert(! moved.isPromotion());
  return last_value + addSub(state, player, moved.to(), moved.ptype(), moved.from());
}

void gpsshogi::PiecePairKingFlat::
featuresNonUniq(const NumEffectState& state, index_list_t& out, int offset) const
{
  index_list_t tmp;
  feature.featuresOneNonUniq(state, tmp);
  for (size_t i=0; i<tmp.size(); ++i)
    out.add(tmp[i].first+offset, tmp[i].second);
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
