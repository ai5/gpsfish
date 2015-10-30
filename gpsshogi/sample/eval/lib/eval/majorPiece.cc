#include "eval/majorPiece.h"
#include "eval/indexCache.h"
#include "osl/mobility/rookMobility.h"
#include "osl/csa.h"
#include <algorithm>

template <osl::Ptype MajorPiece>
int gpsshogi::MajorPieceYBase<MajorPiece>::eval(const NumEffectState &state) const
{
  int result = 0;
  for (int i = PtypeTraits<MajorPiece>::indexMin;
       i < PtypeTraits<MajorPiece>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && isTarget(piece))
    {
      const int y = (piece.owner() == BLACK ? piece.square().y() : 10 - piece.square().y());
      if (piece.owner() == BLACK)
	result += value(y - 1);
      else
	result -= value(y - 1);
    }
  }
  return result;
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorPieceYBase<MajorPiece>::featuresNonUniq(const osl::NumEffectState &state, 
		    index_list_t &diffs,
		    int offset) const
{
  CArray<int, 9> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<MajorPiece>::indexMin;
       i < PtypeTraits<MajorPiece>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && isTarget(piece))
    {
      const int y = (piece.owner() == BLACK ? piece.square().y() : 10 - piece.square().y());
      if (piece.owner() == BLACK)
	++feature_count[y-1];
      else
	--feature_count[y-1];
    }
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorPieceYBase<MajorPiece>::showAll(std::ostream &os) const
{
  os << name() << std::endl;
  for (size_t i = 0; i < dimension(); ++i)
  {
    os << value(i) << " ";
  }
  os << std::endl;
}

int gpsshogi::RookPawn::eval(const NumEffectState &state) const
{
  int result = 0;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && !piece.square().canPromote(piece.owner()) &&
	!state.isPawnMaskSet(piece.owner(), piece.square().x()))
    {
      if (piece.owner() == BLACK)
	result += value(0);
      else
	result -= value(0);
    }
  }
  return result;
}

void gpsshogi::RookPawn::featuresNonUniq(
  const osl::NumEffectState &state,
  index_list_t &diffs,
  int offset) const
{
  int count = 0;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && !piece.square().canPromote(piece.owner()) &&
	!state.isPawnMaskSet(piece.owner(), piece.square().x()))
    {
      if (piece.owner() == BLACK)
	++count;
      else
	--count;
    }
  }
  if (count != 0)
  {
    diffs.add(offset, count);
  }
}

void gpsshogi::RookPawn::showSummary(std::ostream &os) const
{
  os << name() << " " << value(0) << std::endl;
}


int gpsshogi::RookPawnY::index(const NumEffectState &state,
			       const Piece rook) const
{
  const int rook_y =
    (rook.owner() == BLACK ? rook.square().y() : 10 - rook.square().y());
  int pawn_y = 0;
  if (!state.isPawnMaskSet(rook.owner(), rook.square().x()))
  {
    pawn_y = 0;
  }
  else
  {
    for (int y = 1; y <= 9; ++y)
    {
      const Piece piece = state.pieceAt(Square(rook.square().x(), y));
      if (piece.isOnBoardByOwner(rook.owner()) &&
	  piece.ptype() == PAWN)
      {
	pawn_y = y;
	break;
      }
    }
    if (rook.owner() == WHITE)
    {
      pawn_y = 10 - pawn_y;
    }
  }
  return (rook_y - 1) * 10 + pawn_y + (rook.isPromoted() ? 90 : 0);
}

void gpsshogi::RookPawnY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
    {
      features.add(index(state, piece), piece.owner() == BLACK ? 1 : -1);
    }
  }
}


int gpsshogi::RookPawnYX::index(const NumEffectState &state,
				const Piece rook, bool attack) const
{
  const Square king =
    state.kingSquare(attack ? alt(rook.owner()) : rook.owner());
  const int x_diff = std::abs(rook.square().x() - king.x());
  const int rook_y =
    (rook.owner() == BLACK ? rook.square().y() : 10 - rook.square().y());
  int pawn_y = 0;
  if (!state.isPawnMaskSet(rook.owner(), rook.square().x()))
  {
    pawn_y = 0;
  }
  else
  {
    for (int y = 1; y <= 9; ++y)
    {
      const Piece piece = state.pieceAt(Square(rook.square().x(), y));
      if (piece.isOnBoardByOwner(rook.owner()) &&
	  piece.ptype() == PAWN)
      {
	pawn_y = y;
	break;
      }
    }
    if (rook.owner() == WHITE)
    {
      pawn_y = 10 - pawn_y;
    }
  }
  return x_diff * 10 * 9 + (rook_y - 1) * 10 + pawn_y + (rook.isPromoted() ? 810 : 0) + (attack ? 0 : 1620);
}

void gpsshogi::RookPawnYX::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
    {
      features.add(index(state, piece, true),
		   piece.owner() == BLACK ? 1 : -1);
      features.add(index(state, piece, false),
		   piece.owner() == BLACK ? 1 : -1);
    }
  }
}



osl::MultiInt gpsshogi::AllMajor::eval(const NumEffectState &state, const MultiWeights& weights,
				       CArray<MultiInt,2>& /*saved_state*/) const
{
  int black_major_count = 0;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.owner() == BLACK)
      ++black_major_count;
  }
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.owner() == BLACK)
      ++black_major_count;
  }
  if (black_major_count == 0)
    return -weights.value(0);
  else if (black_major_count == 4)
    return weights.value(0);

  return MultiInt();
}

void gpsshogi::AllMajor::featuresNonUniq(
  const osl::NumEffectState &state,
  index_list_t &diffs,
  int offset) const
{
  int black_major_count = 0;
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.owner() == BLACK)
      ++black_major_count;
  }
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.owner() == BLACK)
      ++black_major_count;
  }
  if (black_major_count == 0)
  {
    diffs.add(offset, -1);
  }
  else if (black_major_count == 4)
  {
    diffs.add(offset, 1);
  }
}

void gpsshogi::AllMajor::showSummary(std::ostream &os, const MultiWeights& weights) const
{  
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {
    os << name() << " " << weights.value(0)[s] << std::endl;
  }
}


int gpsshogi::RookEffect::index(Player p, Square pos, Square king,
				bool horizontal, bool promoted)
{
  const int y_diff = (p == BLACK ? king.y() - pos.y() : pos.y() - king.y());
  const int x_diff = std::abs(king.x() - pos.x());
  return y_diff + 8 + x_diff * 17 + (horizontal ? 153 : 0)
    + (promoted ? 306 : 0);
}

template <osl::Player RookOwner>
osl::MultiInt gpsshogi::RookEffect::evalOne(const NumEffectState &state,
				  const Square king,
				  const Piece rook,
				  const Offset dir,
				  const MultiWeights& weights)
{
  assert(rook.owner() == RookOwner);
  MultiInt value;
  for (Square pos = rook.square() + dir; state.pieceAt(pos).isEmpty();
       pos += dir)
  {
    const MultiInt weight =
      weights.value(index(RookOwner, king, pos, dir.dy() == 0,
			rook.isPromoted()));
    value += (RookOwner == BLACK ? weight : -weight);
  }
  return value;
}

osl::MultiInt gpsshogi::RookEffect::
evalOnePiece(int piece_number, 
	     const osl::NumEffectState &state, const MultiWeights& weights) const
{
  MultiInt value;
  const Piece p = state.pieceOf(piece_number);
  if (p.isOnBoard())
  {
    const Square king =
      state.kingSquare(is_attack ? alt(p.owner()) : p.owner());
    if (p.owner() == BLACK) 
    {
      value += evalOne<BLACK>(state, king, p,
			      Board_Table.getOffset(p.owner(), L), weights);
      value += evalOne<BLACK>(state, king, p,
			      Board_Table.getOffset(p.owner(), R), weights);
      value += evalOne<BLACK>(state, king, p,
			      Board_Table.getOffset(p.owner(), U), weights);
      value += evalOne<BLACK>(state, king, p,
			      Board_Table.getOffset(p.owner(), D), weights);
    }
    else 
    {
      value += evalOne<WHITE>(state, king, p,
			      Board_Table.getOffset(p.owner(), L), weights);
      value += evalOne<WHITE>(state, king, p,
			      Board_Table.getOffset(p.owner(), R), weights);
      value += evalOne<WHITE>(state, king, p,
			      Board_Table.getOffset(p.owner(), U), weights);
      value += evalOne<WHITE>(state, king, p,
			      Board_Table.getOffset(p.owner(), D), weights);
    }
  }
  return value;
}

osl::MultiInt gpsshogi::RookEffect::eval(const NumEffectState &state, const MultiWeights& weights,
					 CArray<MultiInt,2>& saved_state) const
{
  for (int i = PtypeTraits<ROOK>::indexMin; i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    saved_state[i - PtypeTraits<ROOK>::indexMin] = evalOnePiece(i, state, weights);
  }
  return saved_state[0] + saved_state[1];
}

osl::MultiInt gpsshogi::RookEffect::evalWithUpdate(
    const NumEffectState &state,
    Move moved, MultiInt last_value, const MultiWeights& weights,
    CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_value;
  for (int i = PtypeTraits<ROOK>::indexMin; i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    if (! state.changedPieces().test(i))
    {
      if (moved.ptype() != KING)
	continue;
      const Piece p = state.pieceOf(i);
      if ((is_attack ? alt(p.owner()) : p.owner()) != moved.player())
	continue;
    }
    saved_state[i-PtypeTraits<ROOK>::indexMin] = evalOnePiece(i, state, weights);
  }
  return saved_state[0] + saved_state[1];
}

void gpsshogi::RookEffect::featureOne(const NumEffectState &state,
				      const Square king,
				      const Piece rook,
				      const Offset dir,
				      CArray<int, 612>& feature) const
{
  for (Square pos = rook.square() + dir; state.pieceAt(pos).isEmpty();
       pos += dir)
  {
    feature[index(rook.owner(), king, pos, dir.dy() == 0, rook.isPromoted())]
      += (rook.owner() == BLACK ? 1 : -1);
  }
}

void gpsshogi::RookEffect::featuresNonUniq(
  const NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  CArray<int, 612> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<ROOK>::indexMin; i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
    {
      const Square king =
	state.kingSquare(is_attack ? alt(p.owner()) : p.owner());
      featureOne(state, king, p,
		 Board_Table.getOffset(p.owner(), L), feature_count);
      featureOne(state, king, p,
		 Board_Table.getOffset(p.owner(), R), feature_count);
      featureOne(state, king, p,
		 Board_Table.getOffset(p.owner(), U), feature_count);
      featureOne(state, king, p,
		 Board_Table.getOffset(p.owner(), D), feature_count);
    }
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

void gpsshogi::RookEffect::showAll(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << "Rook V: " << std::endl;
    for (int y = 0; y < 17; ++y)
    {
      for (int x = 0; x < 9; ++x)
      {
        os << weights.value(y + x * 17)[s] << " ";
      }
      os << std::endl;
    }
    os << "H: " << std::endl;
    for (int y = 0; y < 17; ++y)
    {
      for (int x = 0; x < 9; ++x)
      {
        os << weights.value(y + x * 17 + 153)[s] << " ";
      }
      os << std::endl;
    }
  }
}


int gpsshogi::BishopEffect::index(Player p, Square pos, Square king,
				  bool ur, bool promoted) const
{
  const int y_diff = (p == BLACK ? king.y() - pos.y() : pos.y() - king.y());
  const int x_diff = std::abs(king.x() - pos.x());
  if (p == WHITE)
    ur = !ur;
  if (pos.x() > king.x() || (p == WHITE && pos.x() == king.x()))
    ur = !ur;
  return y_diff + 8 + x_diff * 17 + (ur ? 153 : 0) + (promoted ? 306 : 0);
}

osl::MultiInt gpsshogi::BishopEffect::evalOne(
    const NumEffectState &state, const Square king,
    const Piece bishop, Direction dir, const MultiWeights& weights) const
{
  MultiInt value;
  const Offset offset = Board_Table.getOffset(bishop.owner(), dir);
  for (Square pos = bishop.square() + offset;
       state.pieceAt(pos).isEmpty();
       pos += offset)
  {
    const MultiInt weight =
      weights.value(index(bishop.owner(), king, pos, (dir == UR || dir == DL),
			bishop.isPromoted()));
    value += (bishop.owner() == BLACK ? weight : -weight);
  }
  return value;
}

osl::MultiInt gpsshogi::BishopEffect::
evalOnePiece(int piece_number, const NumEffectState &state, const MultiWeights& weights) const
{
  MultiInt value;
  const Piece p = state.pieceOf(piece_number);
  if (p.isOnBoard())
  {
    const Square king =
      state.kingSquare(is_attack ? alt(p.owner()) : p.owner());
    value += evalOne(state, king, p, UL, weights);
    value += evalOne(state, king, p, UR, weights);
    value += evalOne(state, king, p, DL, weights);
    value += evalOne(state, king, p, DR, weights);
  }
  return value;
}

osl::MultiInt gpsshogi::BishopEffect::eval(const NumEffectState &state, const MultiWeights& weights,
					   CArray<MultiInt,2>& saved_state) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    saved_state[i-PtypeTraits<BISHOP>::indexMin] = evalOnePiece(i, state, weights);
  }
  return saved_state[0] + saved_state[1];
}

osl::MultiInt gpsshogi::BishopEffect::evalWithUpdate(
  const NumEffectState &state, Move moved, MultiInt last_value, const MultiWeights& weights,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_value;
  for (int i = PtypeTraits<BISHOP>::indexMin; i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    if (! state.changedPieces().test(i))
    {
      if (moved.ptype() != KING)
	continue;
      const Piece p = state.pieceOf(i);
      if ((is_attack ? alt(p.owner()) : p.owner()) != moved.player())
	continue;
    }
    saved_state[i-PtypeTraits<BISHOP>::indexMin] = evalOnePiece(i, state, weights);
  }
  return saved_state[0] + saved_state[1];
}

void gpsshogi::BishopEffect::featureOne(const NumEffectState &state,
					const Square king,
					const Piece bishop,
					const Direction dir,
					CArray<int, 612> &feature) const
{
  const Offset offset = Board_Table.getOffset(bishop.owner(), dir);
  for (Square pos = bishop.square() + offset;
       state.pieceAt(pos).isEmpty();
       pos += offset)
  {
    feature[(index(bishop.owner(), king, pos, (dir == UR || dir == DL),
		   bishop.isPromoted()))]
      += (bishop.owner() == BLACK ? 1 : -1);
  }
}

void gpsshogi::BishopEffect::featuresNonUniq(
  const NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  CArray<int, 612> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<BISHOP>::indexMin; i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
    {
      const Square king =
	state.kingSquare(is_attack ? alt(p.owner()) : p.owner());
      featureOne(state, king, p, UL, feature_count);
      featureOne(state, king, p, UR, feature_count);
      featureOne(state, king, p, DL, feature_count);
      featureOne(state, king, p, DR, feature_count);
    }
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

void gpsshogi::BishopEffect::showAll(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << "Bishop UR: " << std::endl;
    for (int y = 0; y < 17; ++y)
    {
      for (int x = 0; x < 9; ++x)
      {
	os << weights.value(y + x * 17)[s] << " ";
      }
      os << std::endl;
    }
    os << "UL: " << std::endl;
    for (int y = 0; y < 17; ++y)
    {
      for (int x = 0; x < 9; ++x)
      {
	os << weights.value(y + x * 17 + 153)[s] << " ";
      }
      os << std::endl;
    }
  }
}


template <osl::Ptype MajorPiece>
osl::PtypeO gpsshogi::
MajorEffectPieceFeatures<MajorPiece>::getPtypeO(const NumEffectState &state,
					const Square pos,
					const Player player,
					const Direction dir)
{
  const Offset offset = Board_Table.getOffset(player, dir);
  Square p = pos + offset;
  while (state.pieceAt(p).isEmpty())
  {
    p += offset;
  }
  if (!p.isOnBoard())
  {
    return PTYPEO_EDGE;
  }
  return state.pieceAt(p).ptypeO();
}

template <osl::Ptype MajorPiece>
void gpsshogi::MajorEffectPieceFeatures<MajorPiece>::addPtypeO(
  const Piece rook,
  const PtypeO ptypeO,
  IndexCacheI<MaxActiveWithDuplication> &features)
{
  if (ptypeO == PTYPEO_EDGE)
  {
    features.add(ptypeO - PTYPEO_MIN, rook.owner() == BLACK ? 1 : -1);
    return;
  }
  if (rook.owner() == BLACK)
  {
    features.add(ptypeO - PTYPEO_MIN, 1);
  }
  else
  {
    features.add(newPtypeO(alt(getOwner(ptypeO)),
			   getPtype(ptypeO)) - PTYPEO_MIN, -1);
  }
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorEffectPieceFeatures<MajorPiece>::featuresOneNonUniq(
  int piece_number,
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features)
{
  const Piece p = state.pieceOf(piece_number);
  if (p.isOnBoard())
  {
    if (MajorPiece == ROOK)
    {
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), U),
		features);
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), D),
		features);
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), L),
		features);
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), R),
		features);
    }
    else
    {
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), UR),
		features);
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), DR),
		features);
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), UL),
		features);
      addPtypeO(p,
		getPtypeO(state, p.square(), p.owner(), DL),
		features);
    }
  }
}

template <osl::Ptype MajorPiece>
const std::string gpsshogi::
MajorEffectPieceFeatures<MajorPiece>::describe(size_t local_index) const
{
  PtypeO ptypeO = static_cast<PtypeO>(local_index+PTYPEO_MIN);
  return csa::show(getOwner(ptypeO))+csa::show(getPtype(ptypeO));
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorEffectPieceFeatures<MajorPiece>::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<MajorPiece>::indexMin;
       i < PtypeTraits<MajorPiece>::indexLimit;
       ++i)
  {
    featuresOneNonUniq(i, state, features);
  }
}

template <osl::Ptype MajorPiece>
osl::MultiInt gpsshogi::
MajorEffectPieceStages<MajorPiece>::evalOne(
  const NumEffectState& state,
  int piece_number) const
{
  index_list_t values;
  MajorEffectPieceFeatures<MajorPiece>::featuresOneNonUniq(piece_number, state, values);
  return makeValue(values);
}

template <osl::Ptype MajorPiece>
osl::MultiInt gpsshogi::
MajorEffectPieceStages<MajorPiece>::evalMulti(
  const NumEffectState& state,
  CArray<MultiInt,2>& saved_state) const
{
  for (int i = PtypeTraits<MajorPiece>::indexMin;
       i < PtypeTraits<MajorPiece>::indexLimit;
       ++i)
  {
    saved_state[i - PtypeTraits<MajorPiece>::indexMin]
      = evalOne(state, i);
  }
  return saved_state[0] + saved_state[1];
}

template <osl::Ptype MajorPiece>
osl::MultiInt gpsshogi::
MajorEffectPieceStages<MajorPiece>::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  for (int i = PtypeTraits<MajorPiece>::indexMin;
       i < PtypeTraits<MajorPiece>::indexLimit;
       ++i)
  {
    if (state.changedPieces().test(i))
      saved_state[i - PtypeTraits<MajorPiece>::indexMin]
	= evalOne(state, i);
  }
  return saved_state[0] + saved_state[1];
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorEffectPieceFeatures<MajorPiece>::showAllOne(const Weights& w, int n, std::ostream &os) const
{
  os << name() << n;
  for (size_t i = 0; i < dimension(); ++i)
  {
    os << " " << w.value(i);
  }
  os << std::endl;
}


template <osl::Ptype MajorPiece>
int gpsshogi::
MajorEffectPieceKingRelative<MajorPiece>::index(
  Player p, Square pos, Square king,
  PtypeO ptypeO, bool attack, bool horizontal, bool promoted)
{
  const int y_diff = (p == BLACK ? king.y() - pos.y() : pos.y() - king.y());
  const int x_diff = std::abs(king.x() - pos.x());
  if (MajorPiece == BISHOP)
  {
    if (p == WHITE)
      horizontal = !horizontal;
    if (pos.x() > king.x() || (p == WHITE && pos.x() == king.x()))
      horizontal = !horizontal;
  }
  if (p == WHITE)
  {
    ptypeO = newPtypeO(alt(getOwner(ptypeO)), getPtype(ptypeO));
  }
  return y_diff + 8 + x_diff * 17 + (ptypeO - PTYPEO_MIN) * 17 * 9 +
    (horizontal ? 4896 : 0) + (promoted ? 9792 : 0) +
    (attack ? 0 : 19584);
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorEffectPieceKingRelative<MajorPiece>::addOne(
  const NumEffectState &state,
  const Square pos,
  const Player player,
  const Direction dir,
  const bool promoted,
  IndexCacheI<MaxActiveWithDuplication> &features)
{
  const Offset offset = Board_Table.getOffset(player, dir);
  const Square self_king = state.kingSquare(player);
  const Square opp_king = state.kingSquare(alt(player));
  const bool horizontal = (dir == L || dir == R || dir == UR || dir == DL);
  const int weight = (player == BLACK ? 1 : -1);
  Square p = pos + offset;
  while (state.pieceAt(p).isEmpty())
  {
    p += offset;
  }
  if (!p.isOnBoard())
  {
    features.add(0 + 0 + (PTYPEO_EDGE - PTYPEO_MIN) * 17 * 9 +
		 (horizontal ? 4896 : 0) + (promoted ? 9792 : 0) +
		 0, weight);
    features.add(0 + 0 + (PTYPEO_EDGE - PTYPEO_MIN) * 17 * 9 +
		 (horizontal ? 4896 : 0) + (promoted ? 9792 : 0) +
		 19584, weight);
  }
  else
  {
    features.add(index(player, p, opp_king, state.pieceAt(p).ptypeO(),
		       true, horizontal, promoted), weight);
    features.add(index(player, p, self_king, state.pieceAt(p).ptypeO(),
		       false, horizontal, promoted), weight);
  }
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorEffectPieceKingRelative<MajorPiece>::featuresOneNonUniq(
  int piece_number,
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features)
{
  const Piece p = state.pieceOf(piece_number);
  if (p.isOnBoard())
  {
    if (MajorPiece == ROOK)
    {
      addOne(state, p.square(), p.owner(), U,
	     p.isPromoted(),
	     features);
      addOne(state, p.square(), p.owner(), D,
	     p.isPromoted(),
	     features);
      addOne(state, p.square(), p.owner(), L,
	     p.isPromoted(),
	     features);
      addOne(state, p.square(), p.owner(), R,
	     p.isPromoted(),
	     features);
    }
    else
    {
      addOne(state, p.square(), p.owner(), UR,
	     p.isPromoted(),
	     features);
      addOne(state, p.square(), p.owner(), DR,
	     p.isPromoted(),
	     features);
      addOne(state, p.square(), p.owner(), UL,
	     p.isPromoted(),
	     features);
      addOne(state, p.square(), p.owner(), DL,
	     p.isPromoted(),
	     features);
    }
  }
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorEffectPieceKingRelative<MajorPiece>::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<MajorPiece>::indexMin;
       i < PtypeTraits<MajorPiece>::indexLimit;
       ++i)
  {
    featuresOneNonUniq(i, state, features);
  }
}

template <osl::Ptype MajorPiece>
void gpsshogi::
MajorEffectPieceKingRelative<MajorPiece>::showAllOne(const Weights &weights,
						     int n,
						     std::ostream &os) const
{
  // Warning.  This function prints only part of the values
  os << name() << " " << n << std::endl;
  for (int i = PTYPEO_MIN; i <= PTYPEO_MAX; ++i)
  {
    const PtypeO ptypeO = static_cast<PtypeO>(i);
    if (!isPiece(ptypeO))
      continue;
    os << ptypeO << std::endl;
    for (int y_diff = 0; y_diff < 17; ++y_diff)
    {
      os << "Y: " << y_diff - 8 << std::endl;
      for (int x_diff = 0; x_diff < 9; ++x_diff)
      {
	os << weights.value(y_diff + x_diff * 17 +
			    (ptypeO - PTYPEO_MIN) * 17 * 9 + n * dimension())
	   << " ";
      }
      os << std::endl;
    }
  }
}

osl::MultiInt gpsshogi::RookEffectPieceKingRelative::
evalMulti(const NumEffectState& state,
	  CArray<MultiInt,2>& saved_state) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    index_list_t features;
    MajorEffectPieceKingRelative<ROOK>::featuresOneNonUniq(i, state, features);
    saved_state[i - PtypeTraits<ROOK>::indexMin] = makeValue(features);
  }
  return saved_state[0] + saved_state[1];
}
osl::MultiInt gpsshogi::RookEffectPieceKingRelative::
evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_value,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == KING)
    return evalMulti(state, saved_state);
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    if (! state.changedPieces().test(i))
      continue;
    index_list_t features;
    MajorEffectPieceKingRelative<ROOK>::featuresOneNonUniq(i, state, features);
    saved_state[i - PtypeTraits<ROOK>::indexMin] = makeValue(features);
  }
  return saved_state[0] + saved_state[1];
}
osl::MultiInt gpsshogi::BishopEffectPieceKingRelative::
evalMulti(const NumEffectState& state,
	  CArray<MultiInt,2>& saved_state) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    index_list_t features;
    MajorEffectPieceKingRelative<BISHOP>::featuresOneNonUniq(i, state, features);
    saved_state[i - PtypeTraits<BISHOP>::indexMin] = makeValue(features);
  }
  return saved_state[0] + saved_state[1];
}
osl::MultiInt gpsshogi::BishopEffectPieceKingRelative::
evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_value,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == KING)
    return evalMulti(state, saved_state);
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    if (! state.changedPieces().test(i))
      continue;
    index_list_t features;
    MajorEffectPieceKingRelative<BISHOP>::featuresOneNonUniq(i, state, features);
    saved_state[i - PtypeTraits<BISHOP>::indexMin] = makeValue(features);
  }
  return saved_state[0] + saved_state[1];
}


void gpsshogi::BishopHead::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && !p.isPromoted())
    {
      const Square up = Board_Table.nextSquare(p.owner(), p.square(), U);
      if (up.isOnBoard() && !state.hasEffectAt(p.owner(), up))
      {
	const PtypeO ptypeo = state.pieceAt(up).ptypeO();
	features.add(index(p.owner(), ptypeo),
		     (p.owner() == BLACK ? 1 : -1));
      }
    }
  }
}

void gpsshogi::BishopHeadX::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && !p.isPromoted())
    {
      const Square up = Board_Table.nextSquare(p.owner(), p.square(), U);
      if (up.isOnBoard() && !state.hasEffectAt(p.owner(), up))
      {
	const PtypeO ptypeo = state.pieceAt(up).ptypeO();
	features.add(index(p.owner(), ptypeo, p.square().x()),
		     (p.owner() == BLACK ? 1 : -1));
      }
    }
  }
}

void gpsshogi::BishopHeadKingRelative::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && !p.isPromoted())
    {
      const Square up = Board_Table.nextSquare(p.owner(), p.square(), U);
      if (up.isOnBoard() && !state.hasEffectAt(p.owner(), up))
      {
	const Square king = state.kingSquare(p.owner());
	const PtypeO ptypeo = state.pieceAt(up).ptypeO();
	features.add(index(p.owner(), ptypeo,
			   std::abs(p.square().x() - king.x()),
			   p.square().y() - king.y()),
		     (p.owner() == BLACK ? 1 : -1));
      }
    }
  }
}



void gpsshogi::
RookPromoteDefense::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && !p.isPromoted() &&
	!p.square().canPromote(p.owner()))
    {
      for (Square pos = Board_Table.nextSquare(p.owner(),
						   p.square(), U);
	   pos.isOnBoard(); pos = Board_Table.nextSquare(p.owner(),
							   pos, U))
      {
	const Piece attacked = state.pieceAt(pos);
	if (!attacked.isEmpty())
	{
	  if (attacked.owner() != p.owner())
	  {
	    NumBitmapEffect effect = state.effectSetAt(pos);
	    if (effect.countEffect(attacked.owner()) == 1)
	    {
	      PieceMask mask = effect & state.piecesOnBoard(attacked.owner());
	      const Piece effect_piece = state.pieceOf(mask.takeOneBit());
	      features.add(attacked.ptype() * 16 + effect_piece.ptype(),
			   (p.owner() == BLACK ? 1 : -1));
	    }
	  }
	  break;
	}
      }
    }
  }
}

void gpsshogi::
RookPromoteDefenseRookH::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && !p.isPromoted() &&
	!p.square().canPromote(p.owner()))
    {
      // TODO: use mobilityOf.
      for (Square pos = Board_Table.nextSquare(p.owner(),
						   p.square(), U);
	   pos.isOnBoard(); pos = Board_Table.nextSquare(p.owner(),
							   pos, U))
      {
	const Piece attacked = state.pieceAt(pos);
	if (!attacked.isEmpty())
	{
	  if (attacked.owner() != p.owner())
	  {
	    NumBitmapEffect effect = state.effectSetAt(pos);
	    if (effect.countEffect(attacked.owner()) == 1)
	    {
	      PieceMask mask = effect & state.piecesOnBoard(attacked.owner());
	      const Piece effect_piece = state.pieceOf(mask.takeOneBit());
	      if (effect_piece.ptype() == ROOK &&
		  effect_piece.square().x() != p.square().x())
	      {
		features.add(
		  attacked.ptype() * 9 +
		  mobility::RookMobility::countHorizontalAll(p.owner(),
							     state, p),
		  (p.owner() == BLACK ? 1 : -1));
	      }
	    }
	  }
	  break;
	}
      }
    }
  }
}


void gpsshogi::
KingRookBishop::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<Square,2> kings = {{
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece rook = state.pieceOf(i);
    if (!rook.isOnBoard())
    {
      continue;
    }
    for (int j = PtypeTraits<BISHOP>::indexMin;
	 j < PtypeTraits<BISHOP>::indexLimit;
	 ++j)
    {
      const Piece bishop = state.pieceOf(j);
      if (!bishop.isOnBoard())
      {
	continue;
      }
      features.add(index<BLACK>(kings[BLACK], rook, bishop), 1);
      features.add(index<WHITE>(kings[WHITE], rook, bishop), -1);
    }
  }
}


template <int N>
void gpsshogi::
NumPiecesBetweenBishopAndKing<N>::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece bishop = state.pieceOf(i);
    if (!bishop.isOnBoard())
    {
      continue;
    }
    features.add(countBetween(state,
			      state.kingSquare(alt(bishop.owner())),
			      bishop),
		 bishop.owner() == BLACK ? 1 : -1);
  }
}

template <int N>
int gpsshogi::
NumPiecesBetweenBishopAndKing<N>::countBetween(
  const NumEffectState &state, Square king, Piece bishop)
{
  assert(bishop.isOnBoard());
  if ((king.x() + king.y() != bishop.square().x() + bishop.square().y()) &&
	(king.x() - king.y() != bishop.square().x() - bishop.square().y()))
  {
    return 8;
  }
  Direction dir;
  assert(king.x() != bishop.square().x());
  assert(king.y() != bishop.square().y());
  if (king.x() < bishop.square().x())
  {
    if (king.y() < bishop.square().y())
    {
	dir = UR;
    }
    else
    {
	dir = DR;
    }
  }
  else
  {
    if (king.y() < bishop.square().y())
    {
	dir = UL;
    }
    else
    {
	dir = DL;
    }
  }
  assert(0 <= N && N <= 2);
  const Player player = bishop.owner();
  const Direction move_dir = (player == BLACK ? dir : inverse(dir));
  int result = 0;
  for (Square pos = state.mobilityOf(dir, bishop.number());
	 pos != king; pos = Board_Table.nextSquare(player, pos, move_dir))
  {
    assert(pos.isOnBoard());
    const Piece piece = state.pieceAt(pos);
    if (!piece.isEmpty())
    {
	if ((N == 0 && piece.owner() == player) ||
	    (N == 1 && piece.owner() != player) ||
	    (N == 2))
	{
	  ++result;
	}
    }
  }
  return result;
}

template <int N>
void gpsshogi::
NumPiecesBetweenBishopAndKing<N>::showAllOne(const Weights &weights,
					     int n,
					     std::ostream &os) const
{
  os << name() << " " << n << std::endl;
  for (size_t i = 0; i < dimension(); ++i)
  {
    os << weights.value(i + n * dimension()) << " ";
  }
  os << std::endl;
}
void gpsshogi::
NumPiecesBetweenBishopAndKingCombination::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece bishop = state.pieceOf(i);
    if (!bishop.isOnBoard())
    {
      continue;
    }
    int self, opp;
    countBetween(state,
		 state.kingSquare(alt(bishop.owner())),
		 bishop, self, opp);
    features.add(self + 9 * opp,
		 bishop.owner() == BLACK ? 1 : -1);
  }
}

void gpsshogi::
NumPiecesBetweenBishopAndKingCombination::countBetween(
  const NumEffectState &state, Square king, Piece bishop,
  int &self, int &opp)
{
  self = opp = 0;
  assert(bishop.isOnBoard());
  if ((king.x() + king.y() != bishop.square().x() + bishop.square().y()) &&
	(king.x() - king.y() != bishop.square().x() - bishop.square().y()))
  {
    self = opp = 8;
    return;
  }
  Direction dir;
  assert(king.x() != bishop.square().x());
  assert(king.y() != bishop.square().y());
  if (king.x() < bishop.square().x())
  {
    if (king.y() < bishop.square().y())
    {
	dir = UR;
    }
    else
    {
	dir = DR;
    }
  }
  else
  {
    if (king.y() < bishop.square().y())
    {
	dir = UL;
    }
    else
    {
	dir = DL;
    }
  }
  const Player player = bishop.owner();
  const Direction move_dir = (player == BLACK ? dir : inverse(dir));
  for (Square pos = state.mobilityOf(dir, bishop.number());
	 pos != king; pos = Board_Table.nextSquare(player, pos, move_dir))
  {
    assert(pos.isOnBoard());
    const Piece piece = state.pieceAt(pos);
    if (!piece.isEmpty())
    {
      if (piece.owner() == player)
      {
	++self;
      }
      else
      {
	++opp;
      }
    }
  }
}


void gpsshogi::
BishopBishopPiece::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Piece bishop1 = state.pieceOf(PtypeTraits<BISHOP>::indexMin);
  const Piece bishop2 = state.pieceOf(PtypeTraits<BISHOP>::indexMin + 1);
  if (!bishop1.isOnBoard() || !bishop2.isOnBoard() ||
      bishop1.owner() == bishop2.owner())
    return;
  if (bishop1.square().x() + bishop1.square().y() !=
      bishop2.square().x() + bishop2.square().y() &&
      bishop1.square().x() - bishop1.square().y() !=
      bishop2.square().x() - bishop2.square().y())
    return;

  if (state.hasEffectByPtype<BISHOP>(bishop2.owner(), bishop1.square()))
    return;

  Direction dir;
  if (bishop1.square().x() < bishop2.square().x())
  {
    if (bishop1.square().y() < bishop2.square().y())
    {
	dir = UR;
    }
    else
    {
	dir = DR;
    }
  }
  else
  {
    if (bishop1.square().y() < bishop2.square().y())
    {
	dir = UL;
    }
    else
    {
	dir = DL;
    }
  }
  Square p1 = state.mobilityOf(inverse(dir), bishop1.number());
  Square p2 = state.mobilityOf(dir, bishop2.number());
  if (p1 == p2)
  {
    const Piece p = state.pieceAt(p1);
    const bool black_with_support =
      state.hasEffectAt<BLACK>(bishop1.owner() == BLACK ?
			       bishop1.square() : bishop2.square());
    const bool white_with_support =
      state.hasEffectAt<WHITE>(bishop1.owner() == WHITE ?
			       bishop1.square() : bishop2.square());
    features.add(index(p.ptypeO(), black_with_support, white_with_support),
		 p.owner() == BLACK ? 1 : -1);
  }
}

void gpsshogi::
RookRook::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  Piece rook1 = state.pieceOf(PtypeTraits<ROOK>::indexMin);
  Piece rook2 = state.pieceOf(PtypeTraits<ROOK>::indexMin + 1);

  // symmetric
  if (rook1.isPromoted() == rook2.isPromoted() &&
      rook1.owner() != rook2.owner() &&
      rook1.isOnBoard() == rook2.isOnBoard() &&
      (!rook1.isOnBoard() ||
       rook1.square().y() + rook2.square().y()  == 10))
  {
    return;
  }

  if (rook1.owner() != rook2.owner())
  {
    if (rook1.owner() != BLACK)
    {
      std::swap(rook1, rook2);
    }
    const int y1 = (rook1.isOnBoard() ? rook1.square().y() : 0);
    const int y2 = (rook2.isOnBoard() ? rook2.square().y() : 0);
    if (y1 + y2 > 10 || !rook1.isOnBoard() ||
	(y1 + y2 == 10 && rook1.isPromoted()))
    {
      features.add(index(rook1, rook2, y1, y2), 1);
    }
    else
    {
      features.add(index(rook2, rook1, (10 - y2) % 10, (10 - y1) % 10), -1);
    }
  }
  else
  {
    int y1 = (rook1.isOnBoard() ? rook1.square().y() : 0);
    int y2 = (rook2.isOnBoard() ? rook2.square().y() : 0);
    if (y1 > y2 || (y1 == y2 && !rook1.isPromoted() && rook2.isPromoted()))
    {
      std::swap(rook1, rook2);
      std::swap(y1, y2);
    }
    if (rook1.owner() == BLACK)
    {
      features.add(index(rook1, rook2, y1, y2),
		   rook1.owner() == BLACK ? 1 : -1);
    }
    else
    {
      if (y1 == 0 || y1 == y2)
      {
	features.add(index(rook1, rook2, (10 - y1) % 10, (10 - y2) % 10),
		     rook1.owner() == BLACK ? 1 : -1);
      }
      else
      {
	features.add(index(rook2, rook1, (10 - y2) % 10, (10 - y1) % 10),
		     rook1.owner() == BLACK ? 1 : -1);
      }
    }
  }
}

void gpsshogi::
RookRookPiece::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Piece rook1 = state.pieceOf(PtypeTraits<ROOK>::indexMin);
  const Piece rook2 = state.pieceOf(PtypeTraits<ROOK>::indexMin + 1);
  if (!rook1.isOnBoard() || !rook2.isOnBoard() ||
      rook1.owner() == rook2.owner())
    return;

  if (state.hasEffectByPtype<ROOK>(rook2.owner(), rook1.square()))
    return;

  Direction dir;
  bool vertical = false;
  if (rook1.square().x() == rook2.square().x())
  {
    vertical = true;
    if (rook1.square().y() < rook2.square().y())
    {
	dir = D;
    }
    else
    {
	dir = U;
    }
  }
  else if (rook1.square().y() == rook2.square().y())
  {
    if (rook1.square().x() < rook2.square().x())
    {
	dir = L;
    }
    else
    {
	dir = R;
    }
  }
  else
  {
    return;
  }
  Square p1 = state.mobilityOf(dir, rook1.number());
  Square p2 = state.mobilityOf(inverse(dir), rook2.number());
  assert(p1.isOnBoard() && p2.isOnBoard());
  if (p1 == p2)
  {
    const Piece p = state.pieceAt(p1);
    const bool black_with_support =
      state.hasEffectAt<BLACK>(rook1.owner() == BLACK ?
			       rook1.square() : rook2.square());
    const bool white_with_support =
      state.hasEffectAt<WHITE>(rook1.owner() == WHITE ?
			       rook1.square() : rook2.square());
    features.add(index(p.ptypeO(), black_with_support, white_with_support,
		       vertical),
		 p.owner() == BLACK ? 1 : -1);
  }
}


void gpsshogi::
BishopStandRank5::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  if (state.hasPieceOnStand<BISHOP>(BLACK))
  {
    features.add(ptypeOIndex(state.pieceAt(Square(5, 3)).ptypeO()), 1);
  }
  if (state.hasPieceOnStand<BISHOP>(WHITE))
  {
    PtypeO ptypeO = state.pieceAt(Square(5, 7)).ptypeO();
    if (isPiece(ptypeO))
    {
      ptypeO = NEW_PTYPEO(alt(getOwner(ptypeO)), getPtype(ptypeO));
    }
    features.add(ptypeOIndex(ptypeO), -1);
  }
}


void gpsshogi::
MajorCheckWithCapture::addOne(
  Player owner,
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Square king = state.kingSquare(owner);
  PieceMask pieces = state.effectedMask(alt(owner));
  pieces &= state.piecesOnBoard(owner);
  pieces &= ~state.effectedMask(owner);
  while (pieces.any()) {
    const Piece p = state.pieceOf(pieces.takeOneBit());
    const Square sq = p.square();
    if (state.hasLongEffectAt<ROOK>(alt(owner), sq)
	&& state.hasEffectIf(newPtypeO(BLACK,ROOK), sq, king))
      features.add(index(p.ptype(), true, sq.canPromote(alt(owner))),
		   sign(owner));
    if (state.hasLongEffectAt<BISHOP>(alt(owner), sq)
	&& state.hasEffectIf(newPtypeO(BLACK,BISHOP), sq, king))
      features.add(index(p.ptype(), false, sq.canPromote(alt(owner))),
		   sign(owner));
  }
}

void gpsshogi::
MajorCheckWithCapture::featuresOneNonUniq(const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  addOne(BLACK, state, features);
  addOne(WHITE, state, features);
}


void gpsshogi::
RookSilverKnight::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit;
       ++i)
  {
    const Piece rook = state.pieceOf(i);
    if (!rook.isOnBoard())
    {
      continue;
    }
    for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit;
       ++i)
    {
      const Piece silver = state.pieceOf(i);
      if (!silver.isOnBoard() || silver.isPromoted() ||
          silver.owner() != rook.owner())
      {
        continue;
      }
      for (int i = PtypeTraits<KNIGHT>::indexMin;
           i < PtypeTraits<KNIGHT>::indexLimit;
           ++i)
      {
        const Piece knight = state.pieceOf(i);
        if (!knight.isOnBoard() || knight.isPromoted() ||
            knight.owner() != rook.owner())
        {
          continue;
        }

        if (rook.owner() == BLACK)
        {
          if (rook.square().x() > 5)
          {
            features.add(index(9 - rook.square().x(), rook.square().y() - 1,
                               9 - silver.square().x(), silver.square().y() - 1,
                               9 - knight.square().x(), knight.square().y() - 1),
                         1);
          }
          else
          {
            features.add(index(rook.square().x() - 1, rook.square().y() - 1,
                               silver.square().x() - 1, silver.square().y() - 1,
                               knight.square().x() - 1, knight.square().y() - 1),
                         1);
          }
        }
        else
        {
          if (rook.square().x() >= 5)
          {
            features.add(index(9 - rook.square().x(), 9 - rook.square().y(),
                               9 - silver.square().x(), 9 - silver.square().y(),
                               9 - knight.square().x(), 9 - knight.square().y()),
                         -1);
          }
          else
          {
            features.add(index(rook.square().x() - 1, 9 - rook.square().y(),
                               silver.square().x() - 1, 9 - silver.square().y(),
                               knight.square().x() - 1, 9 - knight.square().y()),
                         -1);
          }
        }
      }
    }
  }
}

void gpsshogi::
BishopSilverKnight::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<BISHOP>::indexMin;
       i < PtypeTraits<BISHOP>::indexLimit;
       ++i)
  {
    const Piece bishop = state.pieceOf(i);
    if (!bishop.isOnBoard())
    {
      continue;
    }
    for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit;
       ++i)
    {
      const Piece silver = state.pieceOf(i);
      if (!silver.isOnBoard() || silver.isPromoted() ||
          silver.owner() != bishop.owner())
      {
        continue;
      }
      for (int i = PtypeTraits<KNIGHT>::indexMin;
           i < PtypeTraits<KNIGHT>::indexLimit;
           ++i)
      {
        const Piece knight = state.pieceOf(i);
        if (!knight.isOnBoard() || knight.isPromoted() ||
            knight.owner() != bishop.owner())
        {
          continue;
        }

        if (bishop.owner() == BLACK)
        {
          if (bishop.square().x() > 5)
          {
            features.add(index(9 - bishop.square().x(), bishop.square().y() - 1,
                               9 - silver.square().x(), silver.square().y() - 1,
                               9 - knight.square().x(), knight.square().y() - 1),
                         1);
          }
          else
          {
            features.add(index(bishop.square().x() - 1, bishop.square().y() - 1,
                               silver.square().x() - 1, silver.square().y() - 1,
                               knight.square().x() - 1, knight.square().y() - 1),
                         1);
          }
        }
        else
        {
          if (bishop.square().x() >= 5)
          {
            features.add(index(9 - bishop.square().x(), 9 - bishop.square().y(),
                               9 - silver.square().x(), 9 - silver.square().y(),
                               9 - knight.square().x(), 9 - knight.square().y()),
                         -1);
          }
          else
          {
            features.add(index(bishop.square().x() - 1, 9 - bishop.square().y(),
                               silver.square().x() - 1, 9 - silver.square().y(),
                               knight.square().x() - 1, 9 - knight.square().y()),
                         -1);
          }
        }
      }
    }
  }
}

void gpsshogi::
AttackMajorsInBase::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i=0; i<state.nthLimit<ROOK>(); ++i) {
    const Piece rook = state.nth<ROOK>(i);
    if (! rook.isOnBoard() || rook.isPromoted())
      continue;
    Player P = rook.owner();
    Square sq = rook.square();
    if (state.hasEffectAt(alt(P), sq)
	|| sq.squareForBlack(P).y() < 8)
      continue;
    typedef std::pair<Offset,Square> pair_t;
    const CArray<pair_t, 7> bishop_attack =
      {{
	  pair_t(Offset(P, U), sq.neighbor(P, UL)),
	  pair_t(Offset(P, U), sq.neighbor(P, UR)),
	  pair_t(Offset(P, L), sq.neighbor(P, UL)),
	  pair_t(Offset(P, R), sq.neighbor(P, UR)),
	  pair_t(Offset(P, D), sq.neighbor(P, UL)),
	  pair_t(Offset(P, D), sq.neighbor(P, UR)),
	  pair_t(Offset(P, U), sq.neighbor(P, U)),
	}};
    const bool has_gold = state.hasPieceOnStand(alt(P), GOLD);
    const bool rook_support = state.hasEffectAt(P, sq);
    for (pair_t pair: bishop_attack) {
      const Square attack_square = pair.second;
      if (! state[attack_square].isEmpty()
	  || state.countEffect(P, attack_square) > 1)
	continue;
      const Square bishop_square = attack_square + pair.first;
      Piece p = state[bishop_square];
      if (! p.isOnBoardByOwner(P) || p.ptype() != BISHOP
	  || state.hasEffectAt(alt(P), bishop_square))
	continue;
      int a = state.countEffect(alt(P), attack_square) + has_gold;
      if (a <= state.countEffect(P, attack_square))
	continue;
      features.add(index(state.findCheapAttack(P, attack_square).ptype(),
			 state.findCheapAttack(alt(P), attack_square).ptype(),
			 has_gold, rook_support,
			 state.hasEffectNotBy(P, rook, bishop_square)),
		   sign(P));
      features.add(0, sign(P));
    }
  }
}


namespace gpsshogi
{
  template class MajorPieceYBase<ROOK>;
  template class MajorPieceYBase<BISHOP>;
  template class MajorEffectPieceStages<ROOK>;
  template class MajorEffectPieceStages<BISHOP>;
  template class MajorEffectPieceKingRelative<ROOK>;
  template class MajorEffectPieceKingRelative<BISHOP>;
  template class NumPiecesBetweenBishopAndKing<0>;
  template class NumPiecesBetweenBishopAndKing<1>;
  template class NumPiecesBetweenBishopAndKing<2>;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
