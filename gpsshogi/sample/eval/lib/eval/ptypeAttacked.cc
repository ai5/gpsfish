/* ptypeAttacked.cc
 */
#include "eval/ptypeAttacked.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/eval/evalTraits.h"
#include <iostream>
#include <iomanip>

int gpsshogi::
MajorGoldSilverAttacked::index(const NumEffectState &state, Piece piece) const
{
  return piece.ptype() + (state.turn() == piece.owner() ? 0 : PTYPE_SIZE);
}

template <osl::Ptype PTYPE>
int gpsshogi::
MajorGoldSilverAttacked::evalOne(const NumEffectState &state) const
{
  int result = 0;
  for (int i = PtypeTraits<PTYPE>::indexMin;
       i < PtypeTraits<PTYPE>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() &&
	state.hasEffectAt(alt(piece.owner()), piece.square()))
    {
      const int weight = value(index(state, piece));
      if (piece.owner() == BLACK)
	result += weight;
      else
	result -= weight;
    }
  }
  return result;
}

int gpsshogi::MajorGoldSilverAttacked::eval(const NumEffectState &state) const
{
  int result = 0;
  result += evalOne<ROOK>(state);
  result += evalOne<BISHOP>(state);
  result += evalOne<GOLD>(state);
  result += evalOne<SILVER>(state);

  return result;
}

template <osl::Ptype PTYPE>
void gpsshogi::
MajorGoldSilverAttacked::featureOne(const NumEffectState &state,
				    CArray<int, PTYPE_SIZE * 2> &features) const
{
  for (int i = PtypeTraits<PTYPE>::indexMin;
       i < PtypeTraits<PTYPE>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() &&
	state.hasEffectAt(alt(piece.owner()), piece.square()))
    {
      if (piece.owner() == BLACK)
	features[index(state, piece)] += 1;
      else
	features[index(state, piece)] -= 1;
    }
  }
}
void gpsshogi::MajorGoldSilverAttacked::features(
  const NumEffectState &state, 
  index_list_t &diffs, int offset) const
{
  CArray<int, PTYPE_SIZE * 2> feature_count;
  feature_count.fill(0);
  featureOne<ROOK>(state, feature_count);
  featureOne<BISHOP>(state, feature_count);
  featureOne<GOLD>(state, feature_count);
  featureOne<SILVER>(state, feature_count);
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset + i, feature_count[i]);
    }
  }
}

void gpsshogi::MajorGoldSilverAttacked::showSummary(std::ostream &os) const
{
  os << name() << std::endl;
  for (int i = 0; i < PTYPE_SIZE; ++i)
  {
    const Ptype ptype = static_cast<Ptype>(i);
    const Ptype basic = unpromote(ptype);
    if (isMajorBasic(basic) || basic == GOLD || basic == SILVER)
    {
      os << ptype << " " << value(i)
	 << " " << value(i + PTYPE_SIZE) << std::endl;
    }
  }
}

void gpsshogi::
NonPawnAttacked::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask black_attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
  black_attacked.reset(state.kingPiece<BLACK>().number());
  mask_t black_ppawn = state.promotedPieces().getMask<PAWN>() & black_attacked.selectBit<PAWN>();
  black_attacked.clearBit<PAWN>();
  while (black_attacked.any())
  {
    const Piece piece = state.pieceOf(black_attacked.takeOneBit());
    features.add(index(state, piece), 1);
  }
  while (black_ppawn.any())
  {
    const Piece piece = state.pieceOf(black_ppawn.takeOneBit());
    features.add(index(state, piece), 1);
  }
  PieceMask white_attacked = state.effectedMask(BLACK) & state.piecesOnBoard(WHITE);
  white_attacked.reset(state.kingPiece<WHITE>().number());
  mask_t white_ppawn = state.promotedPieces().getMask<PAWN>() & white_attacked.selectBit<PAWN>();
  white_attacked.clearBit<PAWN>();
  while (white_attacked.any())
  {
    const Piece piece = state.pieceOf(white_attacked.takeOneBit());
    features.add(index(state, piece), -1);
  }
  while (white_ppawn.any())
  {
    const Piece piece = state.pieceOf(white_ppawn.takeOneBit());
    features.add(index(state, piece), -1);
  }
}

void gpsshogi::
NonPawnAttackedKingRelatve::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask black_attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
  black_attacked.reset(state.kingPiece<BLACK>().number());
  black_attacked.clearBit<PAWN>();
  while (black_attacked.any())
  {
    const Piece piece = state.pieceOf(black_attacked.takeOneBit());
    features.add(index(state, piece, true), 1);
    features.add(index(state, piece, false), 1);
  }
  PieceMask white_attacked = state.effectedMask(BLACK) & state.piecesOnBoard(WHITE);
  white_attacked.reset(state.kingPiece<WHITE>().number());
  white_attacked.clearBit<PAWN>();
  while (white_attacked.any())
  {
    const Piece piece = state.pieceOf(white_attacked.takeOneBit());
    features.add(index(state, piece, true), -1);
    features.add(index(state, piece, false), -1);
  }
}

void gpsshogi::
NonPawnAttackedPtype::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask black_attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
  black_attacked.reset(state.kingPiece<BLACK>().number());
  mask_t black_ppawn = state.promotedPieces().getMask<PAWN>() & black_attacked.selectBit<PAWN>();
  black_attacked.clearBit<PAWN>();
  black_attacked.orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  while (black_attacked.any())
  {
    const Piece piece = state.pieceOf(black_attacked.takeOneBit());
    PieceMask attacking =
      state.effectSetAt(piece.square()) & state.piecesOnBoard(WHITE);
    while (attacking.any())
    {
      const Piece attack = state.pieceOf(attacking.takeOneBit());
      assert(attack.owner() == WHITE);
      features.add(index(state, piece, attack.ptype()), 1);
    }
  }
  PieceMask white_attacked = state.effectedMask(BLACK) & state.piecesOnBoard(WHITE);
  white_attacked.reset(state.kingPiece<WHITE>().number());
  mask_t white_ppawn = state.promotedPieces().getMask<PAWN>() & white_attacked.selectBit<PAWN>();
  white_attacked.clearBit<PAWN>();
  white_attacked.orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
  while (white_attacked.any())
  {
    const Piece piece = state.pieceOf(white_attacked.takeOneBit());
    PieceMask attacking =
      state.effectSetAt(piece.square()) & state.piecesOnBoard(BLACK);
    while (attacking.any())
    {
      const Piece attack = state.pieceOf(attacking.takeOneBit());
      assert(attack.owner() == BLACK);
      features.add(index(state, piece, attack.ptype()), -1);
    }
  }
}

void gpsshogi::
NonPawnAttackedPtypePair::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask black_attacked = state.effectedMask(WHITE) & state.piecesOnBoard(BLACK);
  black_attacked.reset(state.kingPiece<BLACK>().number());
  mask_t black_ppawn = state.promotedPieces().getMask<PAWN>() & black_attacked.selectBit<PAWN>();
  black_attacked.clearBit<PAWN>();
  black_attacked.orMask(PtypeFuns<PAWN>::indexNum, black_ppawn);
  PieceVector pieces;
  while (black_attacked.any())
  {
    const Piece piece = state.pieceOf(black_attacked.takeOneBit());
    pieces.push_back(piece);
  }
  for (size_t i=0; i+1<pieces.size(); ++i) {
    const int i0 = index1(state, pieces[i]);
    for (size_t j=i+1; j<pieces.size(); ++j) {
      const int i1 = index1(state, pieces[j]);
      features.add(index2(i0, i1), 1);
    }
  }
  pieces.clear();

  PieceMask white_attacked = state.effectedMask(BLACK) & state.piecesOnBoard(WHITE);
  white_attacked.reset(state.kingPiece<WHITE>().number());
  mask_t white_ppawn = state.promotedPieces().getMask<PAWN>() & white_attacked.selectBit<PAWN>();
  white_attacked.clearBit<PAWN>();
  white_attacked.orMask(PtypeFuns<PAWN>::indexNum, white_ppawn);
  while (white_attacked.any())
  {
    const Piece piece = state.pieceOf(white_attacked.takeOneBit());
    pieces.push_back(piece);
  }
  for (size_t i=0; i+1<pieces.size(); ++i) {
    const int i0 = index1(state, pieces[i]);
    for (size_t j=i+1; j<pieces.size(); ++j) {
      const int i1 = index1(state, pieces[j]);
      features.add(index2(i0, i1), -1);
    }
  }
}
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
