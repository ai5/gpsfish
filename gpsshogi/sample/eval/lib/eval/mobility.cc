#include "eval/mobility.h"
#include "osl/mobility/rookMobility.h"
#include "osl/mobility/bishopMobility.h"
#include "osl/mobility/lanceMobility.h"

int gpsshogi::RookMobility::index(
  const osl::NumEffectState &state,
  const osl::Piece &rook, bool vertical) const
{
  int result = (vertical ? osl::mobility::RookMobility::countVerticalAll(
		  rook.owner(),
		  state, rook) :
		osl::mobility::RookMobility::countHorizontalAll(
		  rook.owner(),
		  state, rook));
  if (rook.isPromoted())
    result += 18;
  if (!vertical)
    result += 9;
  return result;
}

osl::MultiInt gpsshogi::RookMobility::eval(const osl::NumEffectState &state, const MultiWeights& weights,
					   CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
       i < osl::PtypeTraits<osl::ROOK>::indexLimit;
       ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
    {
      result += (piece.owner() == osl::BLACK) 
	? weights.value(index(state, piece, true)) 
	: -weights.value(index(state, piece, true));
      result += (piece.owner() == osl::BLACK)
	? weights.value(index(state, piece, false))
	: -weights.value(index(state, piece, false));
    }
  }
  return result;
}

void gpsshogi::RookMobility::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  osl::CArray<int, 18 * 2> mobility;
  mobility.fill(0);

  for (int i = osl::PtypeTraits<osl::ROOK>::indexMin;
       i < osl::PtypeTraits<osl::ROOK>::indexLimit;
       ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
    {
      const int vertcal_index = index(state, piece, true);
      const int horizontal_index = index(state, piece, false);
      mobility[vertcal_index] += (piece.owner() == osl::BLACK ? 1 : -1);
      mobility[horizontal_index] += (piece.owner() == osl::BLACK ? 1 : -1);
    }
  }
  for (size_t i = 0; i < mobility.size(); ++i)
  {
    if (mobility[i] != 0)
    {
      diffs.add(offset + i, mobility[i]);
    }
  }
}

void gpsshogi::
RookMobility::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {
  os << "HI v ";
  for (int i = 0; i < 9; ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl << "HI h ";
  for (int i = 9; i < 18; ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl << "RY v ";
  for (int i = 18; i < 27; ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl << "RY h ";
  for (int i = 27; i < 36; ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl;
  }
}

void gpsshogi::RookMobilityX::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
    {
      const int weight = (p.owner() == BLACK ? 1 : -1);
      features.add(index(p,
			 osl::mobility::RookMobility::countHorizontalAll(
			   p.owner(), state, p), false), weight);
      features.add(index(p,
			 osl::mobility::RookMobility::countVerticalAll(
			   p.owner(), state, p), true), weight);
    }
  }
}

void gpsshogi::RookMobilityXKingX::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && ! p.isPromoted())
    {
      const int weight = (p.owner() == BLACK ? 1 : -1);
      const Piece king = state.kingPiece(p.owner());
      features.add(index(p, king,
			 osl::mobility::RookMobility::countHorizontalAll(
			   p.owner(), state, p), false), weight);
      features.add(index(p, king,
			 osl::mobility::RookMobility::countVerticalAll(
			   p.owner(), state, p), true), weight);
    }
  }
}

void gpsshogi::RookMobilityY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
    {
      const int weight = (p.owner() == BLACK ? 1 : -1);
      features.add(index(p,
			 osl::mobility::RookMobility::countHorizontalAll(
			   p.owner(), state, p), false), weight);
      features.add(index(p,
			 osl::mobility::RookMobility::countVerticalAll(
			   p.owner(), state, p), true), weight);
    }
  }
}


void gpsshogi::RookMobilitySum::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
    {
      const int count =
	osl::mobility::RookMobility::countHorizontalAll(p.owner(), state, p) +
	osl::mobility::RookMobility::countVerticalAll(p.owner(), state, p);
      features.add(count + (p.isPromoted() ? 17 : 0),
		   p.owner() == BLACK ? 1 : -1);
    }
  }
}

void gpsshogi::RookMobilitySumKingX::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && ! p.isPromoted())
    {
      const int count =
	osl::mobility::RookMobility::countHorizontalAll(p.owner(), state, p) +
	osl::mobility::RookMobility::countVerticalAll(p.owner(), state, p);
      const Square king = state.kingSquare(p.owner());
      const int diff_x = std::abs(p.square().x() - king.x());
      features.add(count + 17 * diff_x,
		   p.owner() == BLACK ? 1 : -1);
    }
  }
}



int gpsshogi::BishopMobility::index(
  const osl::NumEffectState &state,
  const osl::Piece &bishop) const
{
  int result = osl::mobility::BishopMobility::countAll(bishop.owner(),
						       state, bishop);
  if (bishop.isPromoted())
    result += 18;
  return result;
}

osl::MultiInt gpsshogi::BishopMobility::eval(const osl::NumEffectState &state, const MultiWeights& weights,
					     CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
       i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
       ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
    {
      result += (piece.owner() == osl::BLACK)
	? weights.value(index(state, piece))
	: -weights.value(index(state, piece));
    }
  }
  return result;
}

void gpsshogi::BishopMobility::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  osl::CArray<int, 18 * 2> mobility;
  mobility.fill(0);

  for (int i = osl::PtypeTraits<osl::BISHOP>::indexMin;
       i < osl::PtypeTraits<osl::BISHOP>::indexLimit;
       ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
    {
      mobility[index(state, piece)] += (piece.owner() == osl::BLACK ? 1 : -1);
    }
  }
  for (size_t i = 0; i < mobility.size(); ++i)
  {
    if (mobility[i] != 0)
    {
      diffs.add(offset + i, mobility[i]);
    }
  }
}

void gpsshogi::
BishopMobility::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {
  os << "KA ";
  for (int i = 0; i < 18; ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl << "UM ";
  for (int i = 18; i < 36; ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl;
  }
}

void gpsshogi::BishopMobilityEach::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
    {
      const int weight = p.owner() == BLACK ? 1 : -1;
      features.add(
	osl::mobility::BishopMobility::countAllDir<UL>(p.owner(), state, p) +
	osl::mobility::BishopMobility::countAllDir<DR>(p.owner(), state, p) +
	(p.isPromoted() ? 9 : 0), weight);
      features.add(
	osl::mobility::BishopMobility::countAllDir<UR>(p.owner(), state, p) +
	osl::mobility::BishopMobility::countAllDir<DL>(p.owner(), state, p) +
	(p.isPromoted() ? 9 : 0), weight);
    }
  }
}



osl::MultiInt gpsshogi::LanceMobility::eval(const osl::NumEffectState &state, const MultiWeights& weights,
					    CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = osl::PtypeTraits<osl::LANCE>::indexMin;
       i < osl::PtypeTraits<osl::LANCE>::indexLimit;
       ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && !piece.isPromoted())
    {
      result += (piece.owner() == osl::BLACK)
	? weights.value(osl::mobility::LanceMobility::countAll(piece.owner(), state, piece))
	: -weights.value(osl::mobility::LanceMobility::countAll(piece.owner(), state, piece));
    }
  }
  return result;
}

void gpsshogi::LanceMobility::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  osl::CArray<int, 9> mobility;
  mobility.fill(0);

  for (int i = osl::PtypeTraits<osl::LANCE>::indexMin;
       i < osl::PtypeTraits<osl::LANCE>::indexLimit;
       ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && !piece.isPromoted())
    {
      mobility[osl::mobility::LanceMobility::countAll(piece.owner(),
						      state, piece)] +=
	(piece.owner() == osl::BLACK ? 1 : -1);
    }
  }
  for (size_t i = 0; i < mobility.size(); ++i)
  {
    if (mobility[i] != 0)
    {
      diffs.add(offset + i, mobility[i]);
    }
  }
}

void gpsshogi::
LanceMobility::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {
  os << "KY ";
  for (int i = 0; i < 9; ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl;
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

