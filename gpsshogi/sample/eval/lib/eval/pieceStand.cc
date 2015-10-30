#include "eval/pieceStand.h"
#include "eval/indexCache.h"
#include "eval/minorPiece.h"
#include "osl/bits/king8Info.h"

osl::MultiInt gpsshogi::PieceStand::evalWithUpdate(
  const osl::NumEffectState &state, osl::Move moved,
  MultiInt last_value, const MultiWeights& weights,
  CArray<MultiInt,2>& /*saved_state*/) const
{
  if (moved.isPass())
    return last_value;
  osl::Ptype captured = moved.capturePtype();
  if (moved.isDrop())
  {
    int count = state.countPiecesOnStand(moved.player(), moved.ptype()) + 1;
    return (moved.player() == BLACK 
	    ? last_value - weights.value(Ptype_Table.getIndexMin(moved.ptype()) + count - 1)
	    : last_value + weights.value(Ptype_Table.getIndexMin(moved.ptype()) + count - 1)
      );
  }
  else if (captured != PTYPE_EMPTY)
  {
    Ptype ptype = unpromote(captured);
    int count = state.countPiecesOnStand(moved.player(), ptype);
    return (moved.player() == BLACK 
	    ? last_value + weights.value(Ptype_Table.getIndexMin(ptype) + count - 1)
	    : last_value - weights.value(Ptype_Table.getIndexMin(ptype) + count - 1)
      );
  }
  else
    return last_value;
}

osl::MultiInt gpsshogi::PieceStand::eval(
  const osl::NumEffectState &state, const MultiWeights& weights,
  CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    const int black_count =
      state.countPiecesOnStand(osl::BLACK, osl::PieceStand::order[i]);
    const int white_count =
      state.countPiecesOnStand(osl::WHITE, osl::PieceStand::order[i]);
    for (int j = 0; j < black_count; ++j)
    {
      result += weights.value(Ptype_Table.getIndexMin(ptype) + j);
    }
    for (int j = 0; j < white_count; ++j)
    {
      result -= weights.value(Ptype_Table.getIndexMin(ptype) + j);
    }
  }
  return result;
}

void gpsshogi::PieceStand::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  const CArray<Ptype,8> order = {{ PAWN, KNIGHT, SILVER, GOLD, KING, LANCE, BISHOP, ROOK }};

  for (size_t i = 0; i < order.size(); ++i)
  {
    const Ptype ptype = order[i];
    const int black_count =
      state.countPiecesOnStand(osl::BLACK, ptype);
    const int white_count =
      state.countPiecesOnStand(osl::WHITE, ptype);
    if (black_count > white_count)
    {
      for (int j = white_count; j < black_count; ++j)
      {
	const int index = Ptype_Table.getIndexMin(ptype) + j;
	diffs.add(offset + index, 1);
      }
    }
    else if (black_count < white_count)
    {
      for (int j = black_count; j < white_count; ++j)
      {
	const int index = Ptype_Table.getIndexMin(ptype) + j;
	diffs.add(offset + index, -1);
      }
    }
  }
}

void gpsshogi::PieceStand::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << "Stand ";
    for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
    {
      Ptype ptype = osl::PieceStand::order[i];
      os << ptype << " ";
      for (int j = Ptype_Table.getIndexMin(ptype);
	   j < Ptype_Table.getIndexLimit(ptype);
	   ++j)
      {
	os << weights.value(j)[s] << " ";
      }
      os << std::endl;
    }
  }
}


osl::MultiInt gpsshogi::NonPawnPieceStand::eval(
  const osl::NumEffectState &state, const MultiWeights& weights,
  CArray<MultiInt,2>& /*saved_state*/) const
{
  int black_total = 0;
  int white_total = 0;
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    if (ptype == PAWN)
      continue;
    black_total +=
      state.countPiecesOnStand(osl::BLACK, ptype);
    white_total +=
      state.countPiecesOnStand(osl::WHITE, ptype);
  }
  return weights.value(black_total) - weights.value(white_total);
}

void gpsshogi::NonPawnPieceStand::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  int black_total = 0;
  int white_total = 0;
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    if (ptype == PAWN)
      continue;
    black_total += 
      state.countPiecesOnStand(osl::BLACK, ptype);
    white_total += 
      state.countPiecesOnStand(osl::WHITE, ptype);
  }
  if (black_total != white_total)
  {
    if (black_total < white_total) 
    {
      diffs.add(offset + black_total, 1);
      diffs.add(offset + white_total, -1);
    }
    else 
    {
      diffs.add(offset + white_total, -1);
      diffs.add(offset + black_total, 1);
    }
  }
}

void gpsshogi::NonPawnPieceStand::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) { 
    os << name();
    for (size_t i = 0; i < dimensionOne(); ++i)
    {
      os << " " << weights.value(i)[s];
      if (i != 0 && i % 5 == 0)
	os << std::endl;
    }
    os << std::endl;
  }
}


void gpsshogi::
NonPawnPieceStandCombination::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
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
  if (black_index != white_index)
  {
    diffs.add(black_index, 1);
    diffs.add(white_index, -1);
  }
}

template <osl::Player P>
void gpsshogi::
NonPawnPieceStandCombinationEach::featuresPlayer(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  const int weight = (P == BLACK ? 1 : -1);
  const int rook_count = state.countPiecesOnStand<ROOK>(P);
  const int bishop_count = state.countPiecesOnStand<BISHOP>(P);
  const int gold_count = state.countPiecesOnStand<GOLD>(P);
  const int silver_count = state.countPiecesOnStand<SILVER>(P);
  const int knight_count = state.countPiecesOnStand<KNIGHT>(P);
  const int lance_count = state.countPiecesOnStand<LANCE>(P);
  if (rook_count + bishop_count + gold_count +
      silver_count + knight_count + lance_count == 0)
  {
    diffs.add(0, weight);
    return;
  }
  for (int rook = 0; rook <= rook_count; ++rook)
  {
    for (int bishop = 0; bishop <= bishop_count; ++bishop)
    {
      for (int gold = 0; gold <= gold_count; ++gold)
      {
	for (int silver = 0; silver <= silver_count; ++silver)
	{
	  for (int knight = 0; knight <= knight_count; ++knight)
	  {
	    for (int lance = 0; lance <= lance_count; ++lance)
	    {
	      if (rook + bishop + gold + silver + knight + lance == 0)
	      {
		continue;
	      }
	      diffs.add(index(rook, bishop, gold, silver, knight, lance),
			weight);
	    }
	  }
	}
      }
    }
  }
}

void gpsshogi::
NonPawnPieceStandCombinationEach::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  featuresPlayer<BLACK>(state, diffs);
  featuresPlayer<WHITE>(state, diffs);
}

void gpsshogi::
NonPawnPieceStandCombinationEach::showAllOne(const Weights& weights,
					     int n,
					     std::ostream &os) const
{
  os << name() << " " << n << std::endl;
  for (int rook = 0; rook <= 2; ++rook)
  {
    for (int bishop = 0; bishop <= 2; ++bishop)
    {
      for (int gold = 0; gold <= 4; ++gold)
      {
	for (int silver = 0; silver <= 4; ++silver)
	{
	  for (int knight = 0; knight <= 4; ++knight)
	  {
	    for (int lance = 0; lance <= 4; ++lance)
	    {
	      os << ROOK << " " << rook << " "
		 << BISHOP << " " << bishop << " "
		 << GOLD << " " << gold << " "
		 << SILVER << " " << silver << " "
		 << KNIGHT << " " << knight << " "
		 << LANCE << " " << lance << " "
		 << weights.value(index(rook, bishop, gold, silver,
					knight, lance) +
				  n * dimension()) << std::endl;
	    }
	  }
	}
      }
    }
  }
}

template <osl::Player P>
void gpsshogi::
CanCheckNonPawnPieceStandCombinationEach::featuresPlayer(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  const Player defense = alt(P);
  const King8Info king = state.king8Info(defense);
  if (!KnightCheck::canCheck(state, state.kingPiece<defense>()) &&
      king.dropCandidate() == 0 && king.hasMoveCandidate<P>(state) == 0)
    return;

  const int weight = (P == BLACK ? 1 : -1);
  const int rook_count = state.countPiecesOnStand<ROOK>(P);
  const int bishop_count = state.countPiecesOnStand<BISHOP>(P);
  const int gold_count = state.countPiecesOnStand<GOLD>(P);
  const int silver_count = state.countPiecesOnStand<SILVER>(P);
  const int knight_count = state.countPiecesOnStand<KNIGHT>(P);
  const int lance_count = state.countPiecesOnStand<LANCE>(P);
  if (rook_count + bishop_count + gold_count +
      silver_count + knight_count + lance_count == 0)
  {
    diffs.add(0, weight);
    return;
  }
  for (int rook = 0; rook <= rook_count; ++rook)
  {
    for (int bishop = 0; bishop <= bishop_count; ++bishop)
    {
      for (int gold = 0; gold <= gold_count; ++gold)
      {
	for (int silver = 0; silver <= silver_count; ++silver)
	{
	  for (int knight = 0; knight <= knight_count; ++knight)
	  {
	    for (int lance = 0; lance <= lance_count; ++lance)
	    {
	      if (rook + bishop + gold + silver + knight + lance == 0)
	      {
		continue;
	      }
	      diffs.add(index(rook, bishop, gold, silver, knight, lance),
			weight);
	    }
	  }
	}
      }
    }
  }
}

void gpsshogi::
CanCheckNonPawnPieceStandCombinationEach::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  featuresPlayer<BLACK>(state, diffs);
  featuresPlayer<WHITE>(state, diffs);
}

void gpsshogi::NonPawnPieceStandCombination::showAllOne(const Weights& weights,
							int n,
							std::ostream &os) const
{
  os << name() << " " << n << std::endl;
  for (int rook = 0; rook <= 2; ++rook)
  {
    for (int bishop = 0; bishop <= 2; ++bishop)
    {
      for (int gold = 0; gold <= 4; ++gold)
      {
	for (int silver = 0; silver <= 4; ++silver)
	{
	  for (int knight = 0; knight <= 4; ++knight)
	  {
	    for (int lance = 0; lance <= 4; ++lance)
	    {
	      os << ROOK << " " << rook << " "
		 << BISHOP << " " << bishop << " "
		 << GOLD << " " << gold << " "
		 << SILVER << " " << silver << " "
		 << KNIGHT << " " << knight << " "
		 << LANCE << " " << lance << " "
		 << weights.value(index(rook, bishop, gold, silver,
					knight, lance) +
				  n * dimension()) << std::endl;
	    }
	  }
	}
      }
    }
  }
  
}

void gpsshogi::
CanCheckNonPawnPieceStandCombination::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  int black_index = -1;

  const King8Info white_king = state.king8Info(WHITE);
  if (KnightCheck::canCheck(state, state.kingPiece<WHITE>()) ||
      white_king.dropCandidate() != 0 || white_king.hasMoveCandidate<BLACK>(state))
  {
    black_index = index(state.countPiecesOnStand<ROOK>(BLACK),
			state.countPiecesOnStand<BISHOP>(BLACK),
			state.countPiecesOnStand<GOLD>(BLACK),
			state.countPiecesOnStand<SILVER>(BLACK),
			state.countPiecesOnStand<KNIGHT>(BLACK),
			state.countPiecesOnStand<LANCE>(BLACK));
  }
  int white_index = -1;
  const King8Info black_king = state.king8Info(BLACK);
  if (KnightCheck::canCheck(state, state.kingPiece<BLACK>()) ||
      black_king.dropCandidate() != 0 || black_king.hasMoveCandidate<WHITE>(state))
  {
    white_index = index(state.countPiecesOnStand<ROOK>(WHITE),
			state.countPiecesOnStand<BISHOP>(WHITE),
			state.countPiecesOnStand<GOLD>(WHITE),
			state.countPiecesOnStand<SILVER>(WHITE),
			state.countPiecesOnStand<KNIGHT>(WHITE),
			state.countPiecesOnStand<LANCE>(WHITE));
  }
  if (black_index != white_index)
  {
    if (black_index != -1)
      diffs.add(black_index, 1);
    if (white_index != -1)
      diffs.add(white_index, -1);
  }
}

void gpsshogi::
NonPawnPieceStandTurn::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>& feature_count) const
{
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    if (ptype == PAWN)
      continue;
    const int black_count =
      state.countPiecesOnStand(osl::BLACK, osl::PieceStand::order[i]);
    const int white_count =
      state.countPiecesOnStand(osl::WHITE, osl::PieceStand::order[i]);
    for (int j = 0; j < black_count; ++j)
    {
      feature_count.add(index(BLACK, state.turn(), ptype, j), 1);
    }
    for (int j = 0; j < white_count; ++j)
    {
      feature_count.add(index(WHITE, state.turn(), ptype, j), -1);
    }
  }
}

void gpsshogi::NonPawnPieceStandTurn::showSummary(const Weights& w, std::ostream &os) const
{
  os << "Stand (not your turn)";
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    Ptype ptype = osl::PieceStand::order[i];
    if (ptype == PAWN)
      continue;
    os << ptype << " ";
    for (int j = Ptype_Table.getIndexMin(ptype);
	 j < Ptype_Table.getIndexLimit(ptype);
	 ++j)
    {
      os << w.value(j - 18) << " ";
    }
    os << std::endl;
  }
  os << "Stand (your turn)";
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    Ptype ptype = osl::PieceStand::order[i];
    if (ptype == PAWN)
      continue;
    os << ptype << " ";
    for (int j = Ptype_Table.getIndexMin(ptype);
	 j < Ptype_Table.getIndexLimit(ptype);
	 ++j)
    {
      os << w.value(j - 18 + 22) << " ";
    }
    os << std::endl;
  }
}

void gpsshogi::PieceStandY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  for (size_t i = 0; i < osl::PieceStand::order.size(); ++i)
  {
    const Ptype ptype = osl::PieceStand::order[i];
    const int black_count =
      state.countPiecesOnStand(osl::BLACK, osl::PieceStand::order[i]);
    const int white_count =
      state.countPiecesOnStand(osl::WHITE, osl::PieceStand::order[i]);
    for (int j = 0; j < black_count; ++j)
    {
      features.add(index(ptype, BLACK, kings[WHITE], j, true), 1);
      features.add(index(ptype, BLACK, kings[BLACK], j, false), 1);
    }
    for (int j = 0; j < white_count; ++j)
    {
      features.add(index(ptype, WHITE, kings[BLACK], j, true), -1);
      features.add(index(ptype, WHITE, kings[WHITE], j, false), -1);
    }
  }
}

void gpsshogi::PieceStandY::showAllOne(const Weights &weights,
				       int n, std::ostream &os) const
{
  os << name() << " " << n << std::endl
     << "Attack" << std::endl;
  for (int y = 0; y < 9; ++y)
  {
    os << "Y: " << y + 1 << std::endl;
    for (int i = PTYPE_MIN; i <= PTYPE_MAX; ++i)
    {
      const Ptype ptype = static_cast<Ptype>(i);
      if (!isPiece(ptype) || isPromoted(ptype))
	continue;
      os << ptype << " ";
      for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	   j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
      {
	os << " " << weights.value(y * 40 + j);
      }
      os << std::endl;
    }
  }
  os << "Defense" << std::endl;
  for (int y = 0; y < 9; ++y)
  {
    os << "Y: " << y + 1 << std::endl;
    for (int i = PTYPE_MIN; i <= PTYPE_MAX; ++i)
    {
      const Ptype ptype = static_cast<Ptype>(i);
      if (!isPiece(ptype) || isPromoted(ptype))
	continue;
      os << ptype << " ";
      for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	   j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
      {
	os << " " << weights.value(y * 40 + j + 360);
      }
      os << std::endl;
    }
  }
}

void gpsshogi::PieceStandCombinationBoth::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
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
  if (black_index > white_index)
  {
    features.add((black_index << 7) | white_index, 1);
  }
  else if (black_index < white_index)
  {
    features.add((white_index << 7) | black_index, -1);
  }
}

const std::string gpsshogi::
PieceStandCombinationBoth::describe(size_t local_index) const
{
  int b = local_index >> 7, w = local_index & ((1<<7)-1);
  std::string ret, pieces="PLKSGBR";
  for (int i=0; i<7; ++i)
    if (b & (1<<i))
      ret += pieces[i];
  ret += "-";
  for (int i=0; i<7; ++i)
    if (w & (1<<i))
      ret += pieces[i];  
  return ret;
}


void gpsshogi::PieceStandOnBoard::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (!piece.isOnBoard())
    {
      continue;
    }
    for (size_t j = 0; j < osl::PieceStand::order.size(); ++ j)
    {
      const Ptype ptype = osl::PieceStand::order[j];
      if (state.hasPieceOnStand(BLACK, ptype))
      {
	features.add(index<BLACK>(j, piece), 1);
      }
      if (state.hasPieceOnStand(WHITE, ptype))
      {
	features.add(index<WHITE>(j, piece), -1);
      }
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
