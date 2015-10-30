#include "eval/pin.h"
#include "eval/indexCache.h"
#include "osl/effect_util/pin.h"
#include "osl/csa.h"

template <int DIM, class IndexFunc>
osl::MultiInt gpsshogi::PinBase<DIM, IndexFunc>::
eval(const osl::NumEffectState &state, const MultiWeights& weights,
     CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  osl::PieceMask black_mask = state.pin(BLACK);
  while (black_mask.any())
  {
    const osl::Piece piece = state.pieceOf(black_mask.takeOneBit());
    result += weights.value(f(state.kingSquare<BLACK>(), piece));
  }
  osl::PieceMask white_mask = state.pin(WHITE);
  while (white_mask.any())
  {
    const osl::Piece piece = state.pieceOf(white_mask.takeOneBit());
    result -= weights.value(f(state.kingSquare<WHITE>(), piece));
  }
  return result;
}

template <int DIM, class IndexFunc>
void gpsshogi::PinBase<DIM, IndexFunc>::
featuresNonUniq(const osl::NumEffectState &state, 
	 index_list_t&values,
	 int offset) const
{
  osl::PieceMask black_mask = state.pin(BLACK);
  while (black_mask.any())
  {
    const osl::Piece piece = state.pieceOf(black_mask.takeOneBit());
    const int idx = f(state.kingSquare<BLACK>(), piece);
    values.add(offset + idx, 1);
  }
  osl::PieceMask white_mask = state.pin(WHITE);
  while (white_mask.any())
  {
    const osl::Piece piece = state.pieceOf(white_mask.takeOneBit());
    const int idx = f(state.kingSquare<WHITE>(), piece);
    values.add(offset + idx, -1);
  }
}

template <int DIM, class IndexFunc>
void gpsshogi::
PinBase<DIM, IndexFunc>::showSummary(std::ostream& os, const MultiWeights&) const
{
  os << name() << " done" << std::endl;
}

void gpsshogi::Pin::showAll(std::ostream &os,
			    const MultiWeights& weights) const
{
  // TODO: quad?
  for (int stage = 0; stage < 3; ++stage)
  {
    os << name() << " " << stage << std::endl;
    for (int y_diff = 0; y_diff < 17; ++y_diff)
    {
      os << "Y: " << y_diff - 8 << std::endl;
      for (int ptype = PTYPE_MIN; ptype <= PTYPE_MAX; ++ptype)
      {
	if (!isPiece(static_cast<Ptype>(ptype)))
	  continue;
	os << static_cast<Ptype>(ptype);
	for (int x_diff = 0; x_diff < 9; ++x_diff)
	{
	  os << " "
	     << weights.value((ptype - PTYPE_PIECE_MIN) * 17 * 9 +
			      x_diff * 17 + y_diff)[stage];
	}
	os << std::endl;
      }
    }
  }
}

template <osl::Player Defense>
void gpsshogi::
PinPtype::featuresOneKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Square king = state.kingSquare<Defense>();
  const int weight = (Defense == BLACK ? 1 : -1);
  osl::PieceMask pin_mask = state.pin(Defense);
  while (pin_mask.any())
  {
    const Piece piece = state.pieceOf(pin_mask.takeOneBit());
    if (!state.hasEffectAt<Defense>(piece.square()))
      continue;
    if (king.y() == piece.square().y()) // rook h
    {
      features.add(piece.ptype() + PTYPE_SIZE * 1, weight);
    }
    else if (king.x() == piece.square().x())
    {
      if (state.hasEffectByPtypeStrict<LANCE>(alt(Defense),
					      piece.square())) // lance
      {
	features.add(piece.ptype() + PTYPE_SIZE * 4, weight);
      }
      else // rook v
      {
	features.add(piece.ptype() + PTYPE_SIZE * 0, weight);
      }
    }
    else // bishop
    {
      if ((Defense == BLACK && piece.square().y() < king.y()) ||
	  (Defense == WHITE && piece.square().y() > king.y())) // u
      {
	features.add(piece.ptype() + PTYPE_SIZE * 2, weight);
      }
      else
      {
	features.add(piece.ptype() + PTYPE_SIZE * 3, weight);
      }
    }
  }
}
void gpsshogi::
PinPtype::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  featuresOneKing<BLACK>(state, features);
  featuresOneKing<WHITE>(state, features);
}

template <osl::Player Defense>
void gpsshogi::
PinPtypeDistance::featuresOneKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Square king = state.kingSquare<Defense>();
  const int weight = (Defense == BLACK ? 1 : -1);
  osl::PieceMask pin_mask = state.pin(Defense);
  while (pin_mask.any())
  {
    const Piece piece = state.pieceOf(pin_mask.takeOneBit());
    if (!state.hasEffectAt<Defense>(piece.square()))
      continue;
    if (king.y() == piece.square().y()) // rook h
    {
      features.add((piece.ptype() + PTYPE_SIZE * 1) * 7 +
		   std::abs(king.x() - piece.square().x()) - 1, weight);
    }
    else if (king.x() == piece.square().x())
    {
      if (state.hasEffectByPtypeStrict<LANCE>(alt(Defense),
					      piece.square())) // lance
      {
	features.add((piece.ptype() + PTYPE_SIZE * 4) * 7 +
		     std::abs(king.y() - piece.square().y()) - 1, weight);
      }
      else // rook v
      {
	features.add((piece.ptype() + PTYPE_SIZE * 0) * 7 +
		     std::abs(king.y() - piece.square().y()) - 1, weight);
      }
    }
    else // bishop
    {
      if ((Defense == BLACK && piece.square().y() < king.y()) ||
	  (Defense == WHITE && piece.square().y() > king.y())) // u
      {
	features.add((piece.ptype() + PTYPE_SIZE * 2) * 7 +
		     std::abs(king.x() - piece.square().x()) - 1, weight);
      }
      else
      {
	features.add((piece.ptype() + PTYPE_SIZE * 3) * 7 +
		     std::abs(king.x() - piece.square().x()) - 1, weight);
      }
    }
  }
}
void gpsshogi::
PinPtypeDistance::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  featuresOneKing<BLACK>(state, features);
  featuresOneKing<WHITE>(state, features);
}

template <osl::Player Defense>
void gpsshogi::
PinPtypePawnAttack::featuresOneKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Square king = state.kingSquare<Defense>();
  const int weight = (Defense == BLACK ? 1 : -1);
  osl::PieceMask pin_mask = state.pin(Defense);
  while (pin_mask.any())
  {
    const Piece piece = state.pieceOf(pin_mask.takeOneBit());
    if (!state.hasEffectAt<Defense>(piece.square()))
      continue;
    const Square up =
      piece.square() + DirectionPlayerTraits<U, Defense>::offset();
    if (!up.isOnBoard() ||
	(!state.hasEffectByPtypeStrict<PAWN>(alt(Defense),
					     up) &&
	 (state.isPawnMaskSet(alt(Defense),
			      piece.square().x()) ||
	  !state.pieceAt(up).isEmpty())))
    {
      continue;
    }

    if (king.y() == piece.square().y()) // rook h
    {
      features.add(piece.ptype() + PTYPE_SIZE * 0, weight);
    }
    else if (king.x() == piece.square().x())
    {
      // do nothing
    }
    else // bishop
    {
      if ((Defense == BLACK && piece.square().y() < king.y()) ||
	  (Defense == WHITE && piece.square().y() > king.y())) // u
      {
	features.add(piece.ptype() + PTYPE_SIZE * 1, weight);
      }
      else
      {
	features.add(piece.ptype() + PTYPE_SIZE * 2, weight);
      }
    }
  }
}

void gpsshogi::
PinPtypePawnAttack::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  featuresOneKing<BLACK>(state, features);
  featuresOneKing<WHITE>(state, features);
}


template <osl::Player Defense>
void gpsshogi::
CheckShadowPtype::featuresOneKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Square king = state.kingSquare<Defense>();
  const int weight = (Defense == BLACK ? 1 : -1);
  osl::PieceMask open_mask = state.checkShadow(alt(Defense));
  while (open_mask.any())
  {
    const Piece piece = state.pieceOf(open_mask.takeOneBit());
    if (king.y() == piece.square().y()) // rook h
    {
      features.add(piece.ptype() + PTYPE_SIZE * 1, weight);
    }
    else if (king.x() == piece.square().x())
    {
	if (state.hasEffectByPtypeStrict<LANCE>(alt(Defense),
						piece.square())) // lance
      {
	features.add(piece.ptype() + PTYPE_SIZE * 4, weight);
      }
      else // rook v
      {
	features.add(piece.ptype() + PTYPE_SIZE * 0, weight);
      }
    }
    else // bishop
    {
      if ((Defense == BLACK && piece.square().y() < king.y()) ||
	  (Defense == WHITE && piece.square().y() > king.y())) // u
      {
	features.add(piece.ptype() + PTYPE_SIZE * 2, weight);
      }
      else
      {
	features.add(piece.ptype() + PTYPE_SIZE * 3, weight);
      }
    }
  }
}
void gpsshogi::
CheckShadowPtype::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  featuresOneKing<BLACK>(state, features);
  featuresOneKing<WHITE>(state, features);
}


namespace gpsshogi
{
  template class PinBase<2142, PinF>;
  template class PinBase<10206, PinYF>;
}
