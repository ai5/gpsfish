#include "eval/minorPiece.h"
#include "osl/container.h"
#include "osl/additionalEffect.h"
#include "osl/csa.h"

bool gpsshogi::
SilverRetreat::canRetreat(const osl::NumEffectState &state,
			  const osl::Piece silver) const
{
  if (!silver.isOnBoard() || silver.isPromoted())
    return true;

  if ((silver.owner() == BLACK && silver.square().y() != 9) ||
      (silver.owner() == WHITE && silver.square().y() != 1))
  {
    Square dl = Board_Table.nextSquare(silver.owner(),
					   silver.square(), DL);
    Square dr = Board_Table.nextSquare(silver.owner(),
					   silver.square(), DR);
    if ((!dl.isOnBoard() ||
	 state.pieceAt(dl).isOnBoardByOwner(silver.owner()) ||
	 state.hasEffectAt(alt(silver.owner()), dl)) &&
	(!dr.isOnBoard() ||
	 state.pieceAt(dr).isOnBoardByOwner(silver.owner()) ||
	 state.hasEffectAt(alt(silver.owner()), dr)))
    {
      return false;
    }
  }
  return true;
}

osl::MultiInt gpsshogi::
SilverRetreat::eval(const osl::NumEffectState &state, const MultiWeights& weights,
		    CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit; ++i)
  {
    const Piece silver = state.pieceOf(i);
    if (!canRetreat(state, silver))
    {
      if (silver.owner() == BLACK)
	result += weights.value(index(silver.owner(), silver.square()));
      else
	result -= weights.value(index(silver.owner(), silver.square()));
    }
  }
  return result;
}

void gpsshogi::
SilverRetreat::featuresNonUniq(const osl::NumEffectState &state, 
			index_list_t&diffs,
			int offset) const
{
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit; ++i)
  {
    const Piece silver = state.pieceOf(i);
    if (!canRetreat(state, silver))
    {
      if (silver.owner() == BLACK)
	++feature_count[index(silver.owner(), silver.square())];
      else
	--feature_count[index(silver.owner(), silver.square())];
    }
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset + i, feature_count[i]);
    }
  }
}

void gpsshogi::
SilverRetreat::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {
    os << name() << " ";
    for (size_t i = 0; i < dimensionOne(); ++i)
    {
      os << weights.value(i)[s] << " ";
    }
    os << std::endl;
  }
}


bool gpsshogi::
GoldRetreat::canRetreat(const osl::NumEffectState &state,
			const osl::Piece gold) const
{
  if (!gold.isOnBoard())
    return true;

  if ((gold.owner() == BLACK && gold.square().y() != 9) ||
      (gold.owner() == WHITE && gold.square().y() != 1))
  {
    Square d = Board_Table.nextSquare(gold.owner(),
					  gold.square(), D);
    if ((state.pieceAt(d).isOnBoardByOwner(gold.owner()) ||
	 state.hasEffectAt(alt(gold.owner()), d)))
    {
      return false;
    }
  }
  return true;
}

osl::MultiInt gpsshogi::
GoldRetreat::eval(const osl::NumEffectState &state, const MultiWeights& weights,
		  CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    const Piece gold = state.pieceOf(i);
    if (!canRetreat(state, gold))
    {
      if (gold.owner() == BLACK)
	result += weights.value(index(gold.owner(), gold.square()));
      else
	result -= weights.value(index(gold.owner(), gold.square()));
    }
  }
  return result;
}

void gpsshogi::
GoldRetreat::featuresNonUniq(const osl::NumEffectState &state, 
		      index_list_t&diffs,
		      int offset) const
{
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    const Piece gold = state.pieceOf(i);
    if (!canRetreat(state, gold))
    {
      if (gold.owner() == BLACK)
	++feature_count[index(gold.owner(), gold.square())];
      else
	--feature_count[index(gold.owner(), gold.square())];
    }
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset + i, feature_count[i]);
    }
  }
}

void gpsshogi::
GoldRetreat::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {
    os << name() << " ";

    for (size_t i = 0; i < dimensionOne(); ++i)
    {
      os << weights.value(i)[s] << " ";
    }
    os << std::endl;
  }
}


bool gpsshogi::
GoldSideMove::canMove(const osl::NumEffectState &state,
		      const osl::Piece gold) const
{
  if (!gold.isOnBoard())
    return true;
  {
    Square r = Board_Table.nextSquare(gold.owner(),
					  gold.square(), R);
    Square l = Board_Table.nextSquare(gold.owner(),
					  gold.square(), L);
    // check effect is from lesser pieces?
    if ((!r.isOnBoard() ||
	 state.pieceAt(r).isOnBoardByOwner(gold.owner()) ||
	 state.hasEffectAt(alt(gold.owner()), r)) &&
	(!l.isOnBoard() ||
	 state.pieceAt(l).isOnBoardByOwner(gold.owner()) ||
	 state.hasEffectAt(alt(gold.owner()), l)))
    {
      return false;
    }
  }
  return true;
}

void gpsshogi::
GoldSideMove::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    const Piece gold = state.pieceOf(i);
    const int weight = (gold.owner() == BLACK ? 1 : -1);
    if (!canMove(state, gold))
    {
      features.add(indexX(gold.square()), weight);
      features.add(indexY(gold.owner(), gold.square()), weight);
    }
  }
}

inline
std::pair<int,int> gpsshogi::
SilverFork::matchRook(const NumEffectState& state, Piece rook,
		      const CArray<bool,2>& has_silver)
{
  const Square sq = rook.square();
  if (rook.isPromoted() || sq.isPieceStand())
    return std::make_pair(0,0);
  const Player owner = rook.owner();
  if (! has_silver[alt(owner)] || ! sq.canPromote(alt(owner)))
    return std::make_pair(0,0);
  const CArray<Offset,2> offset = {{
      Board_Table.getOffset(owner, UL), Board_Table.getOffset(owner, UR)
    }};
  for (size_t i=0; i<offset.size(); ++i) {
    const Square next = sq+offset[i], next2 = next+offset[i];
    if (! state.pieceAt(next).isEmpty() || state.hasEffectAt(owner, next))
      continue;
    const Piece p = state.pieceAt(next2);
    if (! p.isOnBoardByOwner(owner)) 
      continue;
    if (p.ptype() == ROOK)
      return std::make_pair(sign(owner), 0);
    if (p.ptype() == GOLD)
      return std::make_pair(sign(owner), state.hasEffectAt(owner, next2) ? 1 : 2);
  }
  return std::make_pair(0,0);
}
inline
std::pair<int,int> gpsshogi::
SilverFork::matchGold(const NumEffectState& state, Piece gold,
		      const CArray<bool,2>& has_silver)
{
  const Square sq = gold.square();
  if (sq.isPieceStand())
    return std::make_pair(0,0);
  const Player owner = gold.owner();
  if (! has_silver[alt(owner)] || ! sq.canPromote(alt(owner)))
    return std::make_pair(0,0);
  const CArray<Offset,2> offset = {{
      Board_Table.getOffset(BLACK, L), Board_Table.getOffset(BLACK, R)
    }};
  const bool guarded = state.hasEffectAt(owner, sq);
  for (size_t i=0; i<offset.size(); ++i) {
    const Square next = sq+offset[i], next2 = next+offset[i];
    const Piece np = state.pieceAt(next);
    if (np.isEdge())
      continue;
    const Square next_down = next + Board_Table.getOffset(owner, D);
    if (! state.pieceAt(next_down).isEmpty() || state.hasEffectAt(owner, next_down))
      continue;
    const Piece p = state.pieceAt(next2);
    if (! p.isOnBoardByOwner(owner))
      continue;
    if (p.ptype() == ROOK || p.ptype() == GOLD) {
      const bool recaputure = guarded
	|| (p.ptype() == GOLD && state.hasEffectAt(owner, next2))
	|| (np.canMoveOn(owner) && ! state.hasEffectAt(alt(owner), next));
      return std::make_pair(sign(owner), 3 + recaputure);
    }
  }
  return std::make_pair(0,0);
}
const std::string gpsshogi::
SilverFork::describe(size_t local_index) const
{
  std::string ret = (local_index % 2) ? "turn " : "";
  local_index /= 2;
  if (local_index == 0)
    return ret + "Rook Rook";
  if (local_index == 1)
    return ret + "Rook Gold+support";
  if (local_index == 2)
    return ret + "Rook Gold";
  if (local_index == 3)
    return ret + "Gold Gold";
  assert(local_index == 4);
  return ret + "Gold Gold+recapure";
}

void gpsshogi::
SilverFork::featuresOneNonUniq
(const NumEffectState &state, IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<bool,2> has_silver = {{ 
      state.hasPieceOnStand<SILVER>(BLACK), 
      state.hasPieceOnStand<SILVER>(WHITE),
    }};
  if (! has_silver[BLACK] && ! has_silver[WHITE])
    return;
  const Player turn = state.turn();
  for (int i = PtypeTraits<ROOK>::indexMin;
       i < PtypeTraits<ROOK>::indexLimit; ++i) 
  {
    const Piece rook = state.pieceOf(i);
    std::pair<int,int> match = matchRook(state, rook, has_silver);
    if (! match.first)
      continue;
    features.add(match.second*2 + (rook.owner() == turn), match.first);
  }
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i) 
  {
    const Piece gold = state.pieceOf(i);
    std::pair<int,int> match = matchGold(state, gold, has_silver);
    if (! match.first)
      continue;
    features.add(match.second*2 + (gold.owner() == turn), match.first);
  }
}


inline
bool gpsshogi::
BishopRookFork::findDropInLine(const NumEffectState& state, Player defense, 
			       const Square a, const Square b, Piece king)
{
  Offset offset = Board_Table.getShortOffset(Offset32(b,a));
  bool drop_position_found = false;
  Square sq=a+offset;
  for (Piece p=state.pieceAt(sq); p.isEmpty(); sq+=offset, p=state.pieceAt(sq))
  {
    drop_position_found |= ! state.hasEffectAt(defense, sq)
      || (state.hasEffectAt(alt(defense), sq)
	  && ! state.hasEffectNotBy(defense, king, sq));
  }
  return sq == b && drop_position_found;
}
inline
bool gpsshogi::
BishopRookFork::testCenter(const NumEffectState& state, Player defense, 
			   const Square a, const Square b, Piece king,
			   Square center)
{
  const Piece p = state.pieceAt(center);
  if (! p.isEmpty() 
      || (state.hasEffectAt(defense, center)
	  && (! state.hasEffectAt(alt(defense), center)
	      || state.hasEffectNotBy(defense, king, center))))
    return false;
  return state.isEmptyBetween(center, a, true)
    && state.isEmptyBetween(center, b, true);
}

bool gpsshogi::
BishopRookFork::isBishopForkSquare(const NumEffectState& state, Player defense, 
				 const Square a, const Square b)
{
  const int cx = b.x() - a.x(), cy = b.y() - a.y();
  if ((cx + cy) % 2)
    return false;
  const int p = (cx+cy)/2, q = (cx-cy)/2;
  const Piece king = state.kingPiece(defense);
  if (p == 0 || q == 0) 
    return findDropInLine(state, defense, a, b, king);

  const CArray<Square,2> centers = {{
      b + Offset(-p,-p), b + Offset(-q,q)
    }};
  
  for (size_t i=0; i<centers.size(); ++i) {
    if (! centers[i].isOnBoardRegion())
      continue;
    if (testCenter(state, defense, a, b, king, centers[i]))
      return true;
  }
  return false;
}

bool gpsshogi::
BishopRookFork::isRookForkSquare(const NumEffectState& state, Player defense, 
				   const Square a, const Square b)
{
  const Piece king = state.kingPiece(defense);
  const CArray<Square,2> centers = {{
      Square(a.x(), b.y()), Square(b.x(), a.y())
    }};
  if (centers[0] == a || centers[0] == b)
    return findDropInLine(state, defense, a, b, king);
  for (size_t i=0; i<centers.size(); ++i) 
  {
    assert(centers[i].isOnBoardRegion());
    if (testCenter(state, defense, a, b, king, centers[i])) 
      return true;
  }
  return false;
}

void gpsshogi::
BishopRookFork::featuresOneNonUniq
(const NumEffectState &state, IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<bool,2> has_bishop = {{ 
      state.hasPieceOnStand<BISHOP>(BLACK), 
      state.hasPieceOnStand<BISHOP>(WHITE),
    }};
  const CArray<bool,2> has_rook = {{ 
      state.hasPieceOnStand<ROOK>(BLACK), 
      state.hasPieceOnStand<ROOK>(WHITE),
    }};
  if (has_bishop[BLACK] + has_bishop[WHITE]
      + has_rook[BLACK] + has_rook[WHITE] == 0)
    return;
  const Player turn = state.turn();
  CArray<PieceVector,2> pieces;
  {
    PieceMask notcovered = ~state.effectedMask(alt(turn)); 
    notcovered &= ~state.effectedMask(turn);
    notcovered.clearBit<PAWN>();
    notcovered.setBit<KING>();
    for (int z=0; z<2; ++z) 
    {
      PieceMask target = notcovered & state.piecesOnBoard(indexToPlayer(z));
      while (target.any())
	pieces[z].push_back(state.pieceOf(target.takeOneBit()));
    }
  }
  for (int z=0; z<2; ++z) 	// owner of target pieces
  {
    const Player defense = indexToPlayer(z);
    if (! has_bishop[alt(defense)] && ! has_rook[alt(defense)])
      continue;
    for (size_t i=0; i<pieces[z].size(); ++i) 
    {
      const Piece pi = pieces[z][i];
      assert(pi.isOnBoardByOwner(defense));
      for (size_t j=i+1; j<pieces[z].size(); ++j) 
      {
	const Piece pj = pieces[z][j];
	assert(pj.isOnBoardByOwner(defense));
	if (has_bishop[alt(defense)] 
	    && isBishopForkSquare(state, defense, pi.square(), pj.square()))
	{
	  const int index = bishopIndex(pi.ptype(), pj.ptype())*2 + (defense == turn);
	  features.add(index, sign(defense));
	}
	if (has_rook[alt(defense)] 
	    && isRookForkSquare(state, defense, pi.square(), pj.square()))
	{
	  const int index = rookIndex(pi.ptype(), pj.ptype())*2 + (defense == turn);
	  features.add(index, sign(defense));
	}
      }
    }
  }
}

const std::string gpsshogi::
BishopRookFork::describe(size_t local_index) const
{
  std::string ret = (local_index % 2) ? "turn " : "";
  local_index /= 2;
  ret += (local_index >= DROP_DIM) ? "rook fork " : "bishop fork ";
  local_index %= DROP_DIM;
  const int a = (local_index / PTYPE_SIZE), b = (local_index % PTYPE_SIZE);
  return ret + Ptype_Table.getCsaName(static_cast<Ptype>(a))
    + Ptype_Table.getCsaName(static_cast<Ptype>(b));
}



bool gpsshogi::
KnightFork::isForkSquare(const NumEffectState& state, Player defense, 
			   int y, int x0, int x1)
{
  if (abs(x0 -x1) != 2)
    return false;
  const Square drop = Square((x0+x1)/2, y);
  return state.pieceAt(drop).isEmpty() && ! state.hasEffectAt(defense, drop);
}

void gpsshogi::
KnightFork::featuresOneNonUniq
(const NumEffectState &state, IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<bool,2> has_knight = {{ 
      state.hasPieceOnStand<KNIGHT>(BLACK), 
      state.hasPieceOnStand<KNIGHT>(WHITE),
    }};
  
  const CArray<bool,2> may_have_knight = {{ 
      has_knight[BLACK] 
      || (state.effectedMask(BLACK).selectBit<KNIGHT>() 
	  & ~state.effectedMask(WHITE).selectBit<KNIGHT>() 
	  & state.piecesOnBoard(WHITE).getMask(PtypeFuns<KNIGHT>::indexNum)).any(),
      has_knight[WHITE]
      || (state.effectedMask(WHITE).selectBit<KNIGHT>() 
	  & ~state.effectedMask(BLACK).selectBit<KNIGHT>() 
	  & state.piecesOnBoard(BLACK).getMask(PtypeFuns<KNIGHT>::indexNum)).any(),
    }};
  if (has_knight[BLACK] + has_knight[WHITE]
      + may_have_knight[BLACK] + may_have_knight[WHITE] == 0)
    return;
  const Player turn = state.turn();
  CArray<CArray<PieceVector,10>,2> pieces;
  {
    PieceMask not_target;
    not_target.setBit<PAWN>();
    not_target.setBit<LANCE>();
    not_target.setBit<KNIGHT>();
    for (int z=0; z<2; ++z) 
    {
      PieceMask target = state.piecesOnBoard(indexToPlayer(z));
      target &= ~not_target;
      while (target.any()) {
	const Piece p = state.pieceOf(target.takeOneBit());
	pieces[z][p.square().y()].push_back(p);
      }
    }
  }
  for (int z=0; z<2; ++z) 	// owner of target pieces
  {
    const Player defense = indexToPlayer(z);
    if (has_knight[alt(defense)] + may_have_knight[alt(defense)] == 0)
      continue;
    for (int y=3-z*2; y<=9-z*2; ++y) {
      for (size_t i=0; i<pieces[z][y].size(); ++i) 
      {
	const Piece pi = pieces[z][y][i];
	assert(pi.isOnBoardByOwner(defense));
	assert(pi.square().y() == y);
	for (size_t j=i+1; j<pieces[z][y].size(); ++j) 
	{
	  const Piece pj = pieces[z][y][j];
	  assert(pj.isOnBoardByOwner(defense));
	  assert(pj.square().y() == y);
	  if (! isForkSquare(state, defense, y - sign(defense)*2,
			       pi.square().x(), pj.square().x()))
	    continue;
	  int found = index(pi.ptype(), pj.ptype());
	  if (! has_knight[alt(defense)])
	    found += DROP_DIM;
	  found = found*2 + (defense == turn);
	  features.add(found, sign(defense));
	}
      }
    }
  }
}



bool gpsshogi::PawnAdvance::cantAdvance(const NumEffectState &state,
					const PtypeO ptypeO,
					const Square position) const
{
  assert(getPtype(ptypeO) == PAWN);
  return state.pieceAt(Board_Table.nextSquare(getOwner(ptypeO),
						   position,
						   U)).isOnBoardByOwner(getOwner(ptypeO));
}

osl::MultiInt gpsshogi::PawnAdvance::eval(const NumEffectState &state, const MultiWeights& weights,
					  CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted() && cantAdvance(state, pawn))
    {
      if (pawn.owner() == BLACK)
	result += weights.value(index(pawn.owner(), pawn.square()));
      else
	result -= weights.value(index(pawn.owner(), pawn.square()));
    }
  }
  return result;
}

osl::MultiInt gpsshogi::
PawnAdvance::evalWithUpdate(const osl::NumEffectState &state,
			    osl::Move moved, MultiInt last_value, const MultiWeights& weights,
			    CArray<MultiInt,2>& /*saved_state*/) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == PAWN)
  {
    if (cantAdvance(state, moved.ptypeO(), moved.to()))
    {
      return last_value 
	+ ((moved.player() == BLACK)
	   ? weights.value(index(moved.player(), moved.to()))
	   : -weights.value(index(moved.player(), moved.to())));
    }
  }
  MultiInt result = last_value;
  Ptype captured = moved.capturePtype();
  if (captured == PAWN)
  {
    if (cantAdvance(state, moved.capturePtypeO(), moved.to()))
      result += (moved.player() == BLACK)
	? weights.value(index(alt(moved.player()), moved.to()))
	: -weights.value(index(alt(moved.player()), moved.to()));
  }
  else if (captured != PTYPE_EMPTY)
  {
    const Piece piece = state.pieceAt(
      Board_Table.nextSquare(alt(moved.player()), moved.to(), D));
    if (piece.isOnBoardByOwner(alt(moved.player())) &&
	piece.ptype() == PAWN)
      result += (moved.player() == BLACK)
	? weights.value(index(piece.owner(), piece.square()))
	: -weights.value(index(piece.owner(), piece.square()))
	;
  }
  if (!moved.isDrop())
  {
    const Piece piece = state.pieceAt(
      Board_Table.nextSquare(moved.player(), moved.from(), D));
    if (piece.isOnBoardByOwner(moved.player()) &&
	piece.ptype() == PAWN)
      result -= (moved.player() == BLACK)
	? weights.value(index(piece.owner(), piece.square()))
	: -weights.value(index(piece.owner(), piece.square()));
  }
  {
    const Piece piece = state.pieceAt(
      Board_Table.nextSquare(moved.player(), moved.to(), D));
    if (piece.isOnBoardByOwner(moved.player()) &&
	piece.ptype() == PAWN)
      result += (moved.player() == BLACK)
	? weights.value(index(piece.owner(), piece.square()))
	: -weights.value(index(piece.owner(), piece.square()));
  }
  return result;
}

void gpsshogi::
PawnAdvance::featuresNonUniq(const osl::NumEffectState &state, 
		      index_list_t&diffs,
		      int offset) const
{
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted() && cantAdvance(state, pawn))
    {
      if (pawn.owner() == BLACK)
	++feature_count[index(pawn.owner(), pawn.square())];
      else
	--feature_count[index(pawn.owner(), pawn.square())];
    }
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset + i, feature_count[i]);
    }
  }
}

void gpsshogi::
PawnAdvance::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {  
  os << name() << " ";

  for (size_t i = 0; i < dimensionOne(); ++i)
  {
    os << weights.value(i)[s] << " ";
  }
  os << std::endl;
  }
}


bool gpsshogi::KnightAdvance::cantAdvance(const NumEffectState &state,
					  const PtypeO ptypeO,
					  const Square position) const
{
  assert(getPtype(ptypeO) == KNIGHT);
  const Square uul = Board_Table.nextSquare(getOwner(ptypeO),
						position,
						UUL);
  const Square uur = Board_Table.nextSquare(getOwner(ptypeO),
						position,
						UUR);
  return ((!uul.isOnBoard() ||
	   state.pieceAt(uul).isOnBoardByOwner(getOwner(ptypeO))) &&
	  (!uur.isOnBoard() ||
	   state.pieceAt(uur).isOnBoardByOwner(getOwner(ptypeO))));
}

osl::MultiInt gpsshogi::KnightAdvance::eval(const NumEffectState &state, const MultiWeights& weights,
					    CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = PtypeTraits<KNIGHT>::indexMin;
       i < PtypeTraits<KNIGHT>::indexLimit; ++i)
  {
    const Piece knight = state.pieceOf(i);
    if (knight.isOnBoard() && !knight.isPromoted() &&
	cantAdvance(state, knight))
    {
      if (knight.owner() == BLACK)
	result += weights.value(knight.square().y() - 1);
      else
	result -= weights.value(9 - knight.square().y());
    }
  }
  return result;
}

void gpsshogi::
KnightAdvance::featuresNonUniq(const osl::NumEffectState &state, 
			index_list_t&diffs,
			int offset) const
{
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<KNIGHT>::indexMin;
       i < PtypeTraits<KNIGHT>::indexLimit; ++i)
  {
    const Piece knight = state.pieceOf(i);
    if (knight.isOnBoard() && !knight.isPromoted() &&
	cantAdvance(state, knight))
    {
      if (knight.owner() == BLACK)
	++feature_count[knight.square().y() - 1];
      else
	--feature_count[9 - knight.square().y()];
    }
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset + i, feature_count[i]);
    }
  }
}

void gpsshogi::
KnightAdvance::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) 
  {
    os << "Knight Advance ";

    for (size_t i = 0; i < dimensionOne(); ++i)
    {
      os << weights.value(i)[s] << " ";
    }
    os << std::endl;
  }
}


bool gpsshogi::KnightCheck::canCheck(const NumEffectState &state,
				     const Piece king)
{
  const Player defense = king.owner();
  const Player offense = alt(defense);
  const Square ul = Board_Table.nextSquare(defense, king.square(), UUL);
  const Square ur = Board_Table.nextSquare(defense, king.square(), UUR);
  if (ul.isOnBoard())
  {
    const Piece p = state.pieceAt(ul);
    if (!state.hasEffectAt(defense, ul) &&
	((p.isEmpty() && state.hasPieceOnStand<KNIGHT>(offense)) ||
	 (!p.isOnBoardByOwner(offense) &&
	  state.hasEffectByPtypeStrict<KNIGHT>(offense, ul))))
      return true;
  }
  if (ur.isOnBoard())
  {
    const Piece p = state.pieceAt(ur);
    if (!state.hasEffectAt(defense, ur) &&
	((p.isEmpty() && state.hasPieceOnStand<KNIGHT>(offense)) ||
	 (!p.isOnBoardByOwner(offense) &&
	  state.hasEffectByPtypeStrict<KNIGHT>(offense, ur))))
      return true;
  }
  return false;
}

osl::MultiInt gpsshogi::KnightCheck::eval(const NumEffectState &state, const MultiWeights& weights,
					  CArray<MultiInt,2>& /*saved_state*/) const
{
  int can_check = 0;
  if (canCheck(state, state.kingPiece<BLACK>()))
    ++can_check;
  if (canCheck(state, state.kingPiece<WHITE>()))
    --can_check;
  return EvalComponentStages::multiply(weights.value(0), can_check);
}

void gpsshogi::KnightCheck::featuresNonUniq(
  const NumEffectState &state,
  index_list_t &diffs,
  int offset) const
{
  int can_check = 0;
  if (canCheck(state, state.kingPiece<BLACK>()))
    ++can_check;
  if (canCheck(state, state.kingPiece<WHITE>()))
    --can_check;
  if (can_check != 0)
  {
    diffs.add(offset, can_check);
  }
}

void gpsshogi::KnightCheck::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name() << " " << weights.value(0)[s] << std::endl;
  }
}


void gpsshogi::
KnightCheckY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Piece black = state.kingPiece<BLACK>();
  if (KnightCheck::canCheck(state, black))
    features.add(black.square().y() -1, 1);
  const Piece white = state.kingPiece<WHITE>();
  if (KnightCheck::canCheck(state, white))
    features.add(9 - white.square().y(), -1);
}

void gpsshogi::KnightCheckY::showSummary(const Weights& w, std::ostream &os) const
{
  os << name();
  for (size_t i = 0; i < dimension(); ++i)
  {
    os << " " << w.value(i);
  }
  os << std::endl;
}



osl::MultiInt gpsshogi::PawnDrop::eval(const NumEffectState &state, const MultiWeights& weights,
				       CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  const Square black_king = state.kingSquare<BLACK>();
  const Square white_king = state.kingSquare<WHITE>();
  for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x))
    {
      if (is_defense)
	result += weights.value(std::abs(x - black_king.x()));
      else
	result += weights.value(std::abs(x - white_king.x()));
    }
    if (!state.isPawnMaskSet<WHITE>(x))
    {
      if (is_defense)
	result -= weights.value(std::abs(x - white_king.x()));
      else
	result -= weights.value(std::abs(x - black_king.x()));
    }
  }
  return result;
}

void gpsshogi::PawnDrop::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  CArray<int, 9> feature_count;
  feature_count.fill(0);
  const Square black_king = state.kingSquare<BLACK>();
  const Square white_king = state.kingSquare<WHITE>();
  for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x))
    {
      if (is_defense)
	++feature_count[std::abs(x - black_king.x())];
      else
	++feature_count[std::abs(x - white_king.x())];
    }
    if (!state.isPawnMaskSet<WHITE>(x))
    {
      if (is_defense)
	--feature_count[std::abs(x - white_king.x())];
      else
	--feature_count[std::abs(x - black_king.x())];
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

void gpsshogi::PawnDrop::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name() << " ";
    for (size_t i = 0; i < dimensionOne(); ++i)
    {
      os << weights.value(i)[s] << " ";
    }
    os << std::endl;
  }
}


void gpsshogi::PawnDropX::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x))
    {
      features.add(index(black_king, x, false), 1);
      features.add(index(white_king, x, true), 1);
    }
    if (!state.isPawnMaskSet<WHITE>(x))
    {
      features.add(index(white_king, x, false), -1);
      features.add(index(black_king, x, true), -1);
    }
  }
}



osl::MultiInt gpsshogi::PawnDropY::eval(const NumEffectState &state, const MultiWeights& weights,
					CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x))
    {
      if (is_defense)
	result += weights.value(index(black_king, x));
      else
	result += weights.value(index(white_king, x));
    }
    if (!state.isPawnMaskSet<WHITE>(x))
    {
      if (is_defense)
	result -= weights.value(index(white_king, x));
      else
	result -= weights.value(index(black_king, x));
    }
  }
  return result;
}

void gpsshogi::PawnDropY::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  CArray<int, 81> feature_count;
  feature_count.fill(0);
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x))
    {
      if (is_defense)
	++feature_count[index(black_king, x)];
      else
	++feature_count[index(white_king, x)];
    }
    if (!state.isPawnMaskSet<WHITE>(x))
    {
      if (is_defense)
	--feature_count[index(white_king, x)];
      else
	--feature_count[index(black_king, x)];
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

void gpsshogi::PawnDropY::showAll(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name() << std::endl;
    for (int y = 0; y < 9; ++y)
    {
      os << y << " ";
      for (int rel_x = 0; rel_x < 9; ++rel_x)
      {
	os << weights.value(rel_x * 9 + y)[s] << " ";
      }
      os << std::endl;
    }
  }
}


void gpsshogi::PawnDropNonDrop::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int x = 1; x <= 9; ++x)
  {
    const int index_x = (x > 5 ? 10 - x : x);
    const bool black_on_board = state.isPawnMaskSet<BLACK>(x);
    const bool white_on_board = state.isPawnMaskSet<WHITE>(x);
    if (black_on_board && !white_on_board)
    {
      features.add(index_x - 1, 1);
      features.add(index_x - 1 + 5, -1);
    }
    else if (!black_on_board && white_on_board)
    {
      features.add(index_x - 1 + 5, 1);
      features.add(index_x - 1, -1);
    }
  }
}

void gpsshogi::PawnStateKingRelative::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int x = 1; x <= 9; ++x)
  {
    const CArray<Square, 2> kings = {{ state.kingSquare<BLACK>(),
					 state.kingSquare<WHITE>() }};
    const bool black_on_board = state.isPawnMaskSet<BLACK>(x);
    const bool white_on_board = state.isPawnMaskSet<WHITE>(x);
    if (black_on_board && white_on_board)
    {
      features.add(std::abs(kings[BLACK].x() - x) + BOTH_ON_BOARD * 9, 1);
      features.add(std::abs(kings[WHITE].x() - x) + BOTH_ON_BOARD * 9, -1);
    }
    else if (black_on_board && !white_on_board)
    {
      features.add(std::abs(kings[BLACK].x() - x) + SELF_ON_BOARD * 9, 1);
      features.add(std::abs(kings[WHITE].x() - x) + OPP_ON_BOARD * 9, -1);
    }
    else if (!black_on_board && white_on_board)
    {
      features.add(std::abs(kings[BLACK].x() - x) + OPP_ON_BOARD * 9, 1);
      features.add(std::abs(kings[WHITE].x() - x) + SELF_ON_BOARD * 9, -1);
    }
    else if (!black_on_board && !white_on_board)
    {
      features.add(std::abs(kings[BLACK].x() - x) + BOTH_ON_STAND * 9, 1);
      features.add(std::abs(kings[WHITE].x() - x) + BOTH_ON_STAND * 9, -1);
    }
    else
    {
      abort();
    }
  }
}


void gpsshogi::PawnDropPawnStand::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<bool, 2> has_pawn = {{ state.hasPieceOnStand<PAWN>(BLACK),
				      state.hasPieceOnStand<PAWN>(WHITE) }};
  if (!has_pawn[0] && !has_pawn[1])
    return;
  const CArray<Square, 2> kings = {{ state.kingSquare<BLACK>(),
				       state.kingSquare<WHITE>() }};

  for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x) && has_pawn[BLACK])
    {
      features.add(index<true>(kings[WHITE], x), 1);
      features.add(index<false>(kings[BLACK], x), 1);
    }
    if (!state.isPawnMaskSet<WHITE>(x) && has_pawn[WHITE])
    {
      features.add(index<true>(kings[BLACK], x), -1);
      features.add(index<false>(kings[WHITE], x), -1);
    }
  }
}

void gpsshogi::PawnDropPawnStandX::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<bool, 2> has_pawn = {{ state.hasPieceOnStand<PAWN>(BLACK),
				      state.hasPieceOnStand<PAWN>(WHITE) }};
  if (!has_pawn[0] && !has_pawn[1])
    return;
  const CArray<Piece, 2> kings = {{ state.kingPiece<BLACK>(),
				    state.kingPiece<WHITE>() }};

    for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x) && has_pawn[BLACK])
    {
      features.add(index<true>(kings[WHITE], x), 1);
      features.add(index<false>(kings[BLACK], x), 1);
    }
    if (!state.isPawnMaskSet<WHITE>(x) && has_pawn[WHITE])
    {
      features.add(index<true>(kings[BLACK], x), -1);
      features.add(index<false>(kings[WHITE], x), -1);
    }
  }
}

void gpsshogi::PawnDropPawnStandY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<bool, 2> has_pawn = {{ state.hasPieceOnStand<PAWN>(BLACK),
				      state.hasPieceOnStand<PAWN>(WHITE) }};
  if (!has_pawn[0] && !has_pawn[1])
    return;
  const CArray<Piece, 2> kings = {{ state.kingPiece<BLACK>(),
				    state.kingPiece<WHITE>() }};

    for (int x = 1; x <= 9; ++x)
  {
    if (!state.isPawnMaskSet<BLACK>(x) && has_pawn[BLACK])
    {
      features.add(index<true>(kings[WHITE], x), 1);
      features.add(index<false>(kings[BLACK], x), 1);
    }
    if (!state.isPawnMaskSet<WHITE>(x) && has_pawn[WHITE])
    {
      features.add(index<true>(kings[BLACK], x), -1);
      features.add(index<false>(kings[WHITE], x), -1);
    }
  }
}


gpsshogi::NoPawnOnStand::State
gpsshogi::NoPawnOnStand::getPawnState(const NumEffectState &state) const
{
  int black_pawn_count = 0;
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (pawn.owner() == osl::BLACK)
      ++black_pawn_count;
  }

  if (black_pawn_count > 9 && !state.hasPieceOnStand<PAWN>(osl::WHITE))
  {
    return WHITE;
  }
  else if (black_pawn_count < 9 && !state.hasPieceOnStand<PAWN>(osl::BLACK))
  {
    return BLACK;
  }
  return OTHER;
}


osl::MultiInt gpsshogi::NoPawnOnStand::eval(const NumEffectState &state, const MultiWeights& weights,
					    CArray<MultiInt,2>& /*saved_state*/) const
{
  const State pawn_state = getPawnState(state);
  if (pawn_state == OTHER)
    return MultiInt();
  return (pawn_state == BLACK) ? weights.value(0) : -weights.value(0);
}

void gpsshogi::NoPawnOnStand::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  State pawn_state = getPawnState(state);
  if (pawn_state != OTHER)
  {
    diffs.add(offset, pawn_state);
  }
}

;void gpsshogi::NoPawnOnStand::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name() << " " << weights.value(0)[s] << std::endl;
  }
}

int gpsshogi::PawnAndEmptyAndPieceBase::eval(const NumEffectState &state) const
{
  int result = 0;
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted() &&
	((pawn.owner() == BLACK && pawn.square().y() >= 3) ||
	 (pawn.owner() == WHITE && pawn.square().y() <= 7)))
    {
      if (pawn.owner() == BLACK &&
	  state.pieceAt(Square(pawn.square().x(),
				    pawn.square().y() - 1)).isEmpty())
      {
	result += value(index(state, pawn,
			      state.pieceAt(Square(pawn.square().x(),
							pawn.square().y() - 2))));
      }
      else if (pawn.owner() == WHITE &&
	       state.pieceAt(Square(pawn.square().x(),
					 pawn.square().y() + 1)).isEmpty())
      {
	result -= value(index(state, pawn,
			      state.pieceAt(Square(pawn.square().x(),
							pawn.square().y() + 2))));
      }
    }
  }
  return result;
}

int gpsshogi::PawnAndEmptyAndPieceBase::ptypeOIndex(
  const NumEffectState &,
  const Piece pawn,
  const Piece target)
{
  if (target.isEmpty() || pawn.owner() == BLACK)
    return osl::ptypeOIndex(target.ptypeO());
  return osl::ptypeOIndex(NEW_PTYPEO(alt(target.owner()), target.ptype()));
}

void gpsshogi::PawnAndEmptyAndPieceBase::featuresNonUniq(
  const NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  CArray<int, PTYPEO_SIZE * 9> feature_count;
  feature_count.fill(0);
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted() &&
	((pawn.owner() == BLACK && pawn.square().y() >= 3) ||
	 (pawn.owner() == WHITE && pawn.square().y() <= 7)))
    {
      if (pawn.owner() == BLACK &&
	  state.pieceAt(Square(pawn.square().x(),
				    pawn.square().y() - 1)).isEmpty())
      {
	++feature_count[index(state, pawn,
			    state.pieceAt(Square(pawn.square().x(),
						      pawn.square().y() - 2)))];
      }
      else if (pawn.owner() == WHITE &&
	       state.pieceAt(Square(pawn.square().x(),
					 pawn.square().y() + 1)).isEmpty())
      {
	--feature_count[index(state, pawn,
			      state.pieceAt(Square(pawn.square().x(),
							pawn.square().y() + 2)))];
      }
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

int gpsshogi::PawnAndEmptyAndPiece::index(const NumEffectState &state,
					  const Piece pawn,
					  const Piece target) const
{
  return ptypeOIndex(state, pawn, target);
}

void gpsshogi::PawnAndEmptyAndPiece::showSummary(std::ostream &os) const
{
  os << name();
  for (size_t i = 0; i < dimension(); ++i)
  {
    os << " " << value(i);
  }
  os << std::endl;
}

int gpsshogi::PawnAndEmptyAndPieceY::index(const NumEffectState &state,
					   const Piece pawn,
					   const Piece target) const
{
  const int ptypeo_index = ptypeOIndex(state, pawn, target);
  const int y = (pawn.owner() == BLACK ? pawn.square().y() : 10 - pawn.square().y());
  return ptypeo_index * 9 + y - 1;
}

int gpsshogi::PawnAndEmptyAndPieceX::index(const NumEffectState &state,
					   const Piece pawn,
					   const Piece target) const
{
  const int ptypeo_index = ptypeOIndex(state, pawn, target);
  const int x = (pawn.square().x() <= 5 ? pawn.square().x() :
		 10 - pawn.square().x());
  return ptypeo_index * 5 + x - 1;
}


void gpsshogi::
PawnPtypeOPtypeO::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted())
    {
      const Square up = Board_Table.nextSquare(pawn.owner(),
						   pawn.square(), U);
      const Square up_up = Board_Table.nextSquare(pawn.owner(),
						      up, U);
      PtypeO up_p =
	(up.isOnBoard() ? state.pieceAt(up).ptypeO() : PTYPEO_EDGE);
      PtypeO up_up_p =
	(up_up.isOnBoard() ? state.pieceAt(up_up).ptypeO() : PTYPEO_EDGE);
      if (pawn.owner() == BLACK)
      {
	features.add((up_p - PTYPEO_MIN) * 32 + (up_up_p - PTYPEO_MIN), 1);
      }
      else
      {
	if (isPiece(up_p))
	{
	  up_p = newPtypeO(alt(getOwner(up_p)), getPtype(up_p));
	}
	if (isPiece(up_up_p))
	{
	  up_up_p = newPtypeO(alt(getOwner(up_up_p)), getPtype(up_up_p));
	}
	features.add((up_p - PTYPEO_MIN) * 32 + (up_up_p - PTYPEO_MIN), -1);
      }
    }
  }  
}

osl::MultiInt gpsshogi::
PawnPtypeOPtypeOStages::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  if (moved.oldPtype() == PAWN || moved.capturePtype() == PAWN)
    return evalMulti(state, saved_state);
  const int tx = moved.to().x();
  if (state.isPawnMaskSet(BLACK, tx) || state.isPawnMaskSet(WHITE, tx))
    return evalMulti(state, saved_state);
  if (! moved.isDrop())
  {
    const int fx = moved.from().x();
    if (state.isPawnMaskSet(BLACK, fx) || state.isPawnMaskSet(WHITE, fx))
      return evalMulti(state, saved_state);
  }
  return last_values;
}

void gpsshogi::
PawnPtypeOPtypeOY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit; ++i)
  {
    Piece pawn = state.pieceOf(i);
    if (pawn.isOnBoard() && !pawn.isPromoted())
    {
      const Square up = Board_Table.nextSquare(pawn.owner(),
						   pawn.square(), U);
      const Square up_up = Board_Table.nextSquare(pawn.owner(),
						      up, U);
      PtypeO up_p =
	(up.isOnBoard() ? state.pieceAt(up).ptypeO() : PTYPEO_EDGE);
      PtypeO up_up_p =
	(up_up.isOnBoard() ? state.pieceAt(up_up).ptypeO() : PTYPEO_EDGE);
      if (pawn.owner() == BLACK)
      {
	features.add((up_p - PTYPEO_MIN) * 32 + (up_up_p - PTYPEO_MIN) +
		     1024 * (pawn.square().y() - 1), 1);
      }
      else
      {
	if (isPiece(up_p))
	{
	  up_p = newPtypeO(alt(getOwner(up_p)), getPtype(up_p));
	}
	if (isPiece(up_up_p))
	{
	  up_up_p = newPtypeO(alt(getOwner(up_up_p)), getPtype(up_up_p));
	}
	features.add((up_p - PTYPEO_MIN) * 32 + (up_up_p - PTYPEO_MIN) +
		     1024 * (9 - pawn.square().y()), -1);
      }
    }
  }  
}

void gpsshogi::PawnPtypeOPtypeOY::showAllOne(const Weights& weights,
					     int n,
					     std::ostream &os) const
{
  os << name() << " " << n << std::endl;
  for (int y = 0; y < 9; ++y)
  {
    os << "Y: " << y + 1 << std::endl;
    for (int ptypeO1 = PTYPEO_MIN; ptypeO1 <= PTYPEO_MAX; ++ptypeO1)
    {
      for (int ptypeO2 = PTYPEO_MIN; ptypeO2 <= PTYPEO_MAX; ++ptypeO2)
      {
	os << static_cast<PtypeO>(ptypeO1)
	   << " " << static_cast<PtypeO>(ptypeO2)
	   << " "
	   << weights.value((ptypeO1 - PTYPEO_MIN) * 32 +
			    (ptypeO2 - PTYPEO_MIN) +
			    1024 * y + n * dimension()) << std::endl;
      }
    }
  }
}

osl::MultiInt gpsshogi::
PawnPtypeOPtypeOYStages::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  if (moved.oldPtype() == PAWN || moved.capturePtype() == PAWN)
    return evalMulti(state, saved_state);
  const int tx = moved.to().x();
  if (state.isPawnMaskSet(BLACK, tx) || state.isPawnMaskSet(WHITE, tx))
    return evalMulti(state, saved_state);
  if (! moved.isDrop())
  {
    const int fx = moved.from().x();
    if (state.isPawnMaskSet(BLACK, fx) || state.isPawnMaskSet(WHITE, fx))
      return evalMulti(state, saved_state);
  }
  return last_values;
}


osl::MultiInt gpsshogi::AllGold::eval(const NumEffectState &state, const MultiWeights& weights,
				      CArray<MultiInt,2>& /*saved_state*/) const
{
  int black_gold_count = 0;
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.owner() == BLACK)
      ++black_gold_count;
  }
  if (black_gold_count == 0)
    return -weights.value(0);
  else if (black_gold_count == 4)
    return weights.value(0);

  return MultiInt();
}

void gpsshogi::AllGold::featuresNonUniq(
  const osl::NumEffectState &state,
  index_list_t&diffs,
  int offset) const
{
  int black_gold_count = 0;
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.owner() == BLACK)
      ++black_gold_count;
  }
  if (black_gold_count == 0)
  {
    diffs.add(offset, -1);
  }
  else if (black_gold_count == 4)
  {
    diffs.add(offset, 1);
  }
}

void gpsshogi::AllGold::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name() << " " << weights.value(0)[s] << std::endl;
  }
}

int gpsshogi::PtypeY::index(const Player player,
			    const Ptype ptype,
			    const Square pos) const
{
  const int y = (player == BLACK ? pos.y() : 10 - pos.y()) - 1;
  return ptype * 9 + y;
}
const std::string gpsshogi::PtypeY::
describe(size_t local_index) const
{
  return osl::csa::show(static_cast<Ptype>(local_index/9))
    + " y=" + std::to_string((local_index%9)+1);
}

osl::MultiInt gpsshogi::PtypeY::eval(const NumEffectState &state, const MultiWeights& weights,
				     CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    const MultiInt weight = weights.value(index(p));
    if (p.owner() == BLACK)
      result += weight;
    else
      result -= weight;
  }
  return result;
}


osl::MultiInt gpsshogi::PtypeY::evalWithUpdate(const osl::NumEffectState &,
					       osl::Move moved, MultiInt last_value, const MultiWeights& weights,
					       CArray<MultiInt,2>& /*saved_state*/) const
{
  if (moved.isPass())
    return last_value;
  MultiInt result = last_value;

  if (!moved.isDrop())
  {
    const MultiInt weight =
      weights.value(index(moved.player(), moved.oldPtype(), moved.from()));
    if (moved.player() == BLACK)
      result -= weight;
    else
      result += weight;
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const MultiInt weight =
      weights.value(index(alt(moved.player()), captured, moved.to()));
    if (moved.player() == BLACK)
      result += weight;
    else
      result -= weight;
  }
  {
    const MultiInt weight =
      weights.value(index(moved.player(), moved.ptype(), moved.to()));
    if (moved.player() == BLACK)
      result += weight;
    else
      result -= weight;
  }

  return result;
}

void gpsshogi::PtypeY::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  CArray<int, 9 * PTYPE_SIZE> feature_count;
  feature_count.fill(0);
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    if (p.owner() == BLACK)
      ++feature_count[index(p)];
    else
      --feature_count[index(p)];
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

void gpsshogi::PtypeY::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name() << std::endl;
    for (int i = 0 ; i < PTYPE_SIZE; ++i)
    {
      Ptype ptype = static_cast<Ptype>(i);
      if (!isPiece(ptype))
	continue;

      os << ptype;
      for (int y = 0; y < 9; ++y)
      {
	os << " " << weights.value(ptype * 9 + y)[s];
      }
      os << std::endl;
    }
  }
}


int gpsshogi::PtypeX::index(const Player ,
			    const Ptype ptype,
			    const Square pos) const
{
  const int x = (pos.x() > 5 ? 10 - pos.x() : pos.x()) - 1;
  return ptype * 5 + x;
}

osl::MultiInt gpsshogi::PtypeX::eval(const NumEffectState &state, const MultiWeights& weights,
				     CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    const MultiInt weight = weights.value(index(p));
    if (p.owner() == BLACK)
      result += weight;
    else
      result -= weight;
  }
  return result;
}


osl::MultiInt gpsshogi::PtypeX::evalWithUpdate(
    const osl::NumEffectState &,
    osl::Move moved, MultiInt last_value, const MultiWeights& weights,
    CArray<MultiInt,2>& /*saved_state*/) const
{
  if (moved.isPass())
    return last_value;
  MultiInt result = last_value;

  if (!moved.isDrop())
  {
    const MultiInt weight =
      weights.value(index(moved.player(), moved.oldPtype(), moved.from()));
    if (moved.player() == BLACK)
      result -= weight;
    else
      result += weight;
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const MultiInt weight =
      weights.value(index(alt(moved.player()), captured, moved.to()));
    if (moved.player() == BLACK)
      result += weight;
    else
      result -= weight;
  }
  {
    const MultiInt weight =
      weights.value(index(moved.player(), moved.ptype(), moved.to()));
    if (moved.player() == BLACK)
      result += weight;
    else
      result -= weight;
  }

  return result;
}

void gpsshogi::PtypeX::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  CArray<int, 9 * PTYPE_SIZE> feature_count;
  feature_count.fill(0);
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    if (p.owner() == BLACK)
      ++feature_count[index(p)];
    else
      --feature_count[index(p)];
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

void gpsshogi::PtypeX::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name() << std::endl;
    for (int i = 0 ; i < PTYPE_SIZE; ++i)
    {
      Ptype ptype = static_cast<Ptype>(i);
      if (!isPiece(ptype))
	continue;

      os << ptype;
      for (int x = 0; x < 5; ++x)
      {
	os << " " << weights.value(ptype * 5 + x)[s];
      }
      os << std::endl;
    }
  }
}


template <osl::Player P>
void gpsshogi::
PromotedMinorPieces::featuresX(
  const NumEffectState &state,
  const PieceMask promoted,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask attack = promoted & state.piecesOnBoard(P);
  const Square king = state.kingSquare<alt(P)>();
  int min_left = -10;
  int min_right = 10;
  while (attack.any())
  {
    const Piece p = state.pieceOf(attack.takeOneBit());
    const int x_diff = (P == BLACK ? p.square().x() - king.x() :
			king.x() - p.square().x());
    const int weight = (P == BLACK ? 1 : -1);
    if (x_diff <= 0)
    {
      if (x_diff > min_left)
      {
	if (min_left != -10)
	  features.add(-min_left, weight);
	min_left = x_diff;
      }
      else
      {
	features.add(-x_diff, weight);
      }
    }
    if (x_diff >= 0)
    {
      if (x_diff < min_right)
      {
	if (min_right != 10)
	  features.add(min_right, weight);
	min_right = x_diff;
      }
      else if (x_diff != 0)
      {
	features.add(x_diff, weight);
      }
    }
  }
}

void gpsshogi::
PromotedMinorPieces::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask promoted_pieces = state.promotedPieces();
  promoted_pieces.clearBit<ROOK>();
  promoted_pieces.clearBit<BISHOP>();
  if (promoted_pieces.none())
    return;

  featuresX<BLACK>(state, promoted_pieces, features);
  featuresX<WHITE>(state, promoted_pieces, features);
}


template <osl::Player P>
void gpsshogi::
PromotedMinorPiecesY::featuresX(
  const NumEffectState &state,
  const PieceMask promoted,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask attack = promoted & state.piecesOnBoard(P);
  const Square king = state.kingSquare<alt(P)>();
  const Square self_king = state.kingSquare<P>();
  int min_left = -10;
  int min_right = 10;
  while (attack.any())
  {
    const Piece p = state.pieceOf(attack.takeOneBit());
    const int x_diff = (P == BLACK ? p.square().x() - king.x() :
			king.x() - p.square().x());
    const int weight = (P == BLACK ? 1 : -1);
    if (x_diff <= 0)
    {
      if (x_diff > min_left)
      {
	if (min_left != -10)
	{
	  features.add(index(true, king, P, -min_left), weight);
	  features.add(index(false, self_king, P, -min_left), weight);
	}
	min_left = x_diff;
      }
      else
      {
	features.add(index(true, king, P, -x_diff), weight);
	features.add(index(false, self_king, P, -x_diff), weight);
      }
    }
    if (x_diff >= 0)
    {
      if (x_diff < min_right)
      {
	if (min_right != 10)
	{
	  features.add(index(true, king, P, min_right), weight);
	  features.add(index(false, self_king, P, min_right), weight);
	}
	min_right = x_diff;
      }
      else if (x_diff != 0)
      {
	features.add(index(true, king, P, x_diff), weight);
	features.add(index(false, self_king, P, x_diff), weight);
      }
    }
  }
}

void gpsshogi::
PromotedMinorPiecesY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  PieceMask promoted_pieces = state.promotedPieces();
  promoted_pieces.clearBit<ROOK>();
  promoted_pieces.clearBit<BISHOP>();
  if (promoted_pieces.none())
    return;

  featuresX<BLACK>(state, promoted_pieces, features);
  featuresX<WHITE>(state, promoted_pieces, features);
}



void gpsshogi::
KnightHead::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<KNIGHT>::indexMin;
       i < PtypeTraits<KNIGHT>::indexLimit;
       ++i)
  {
    const Piece knight = state.pieceOf(i);
    if (knight.isOnBoard() && !knight.isPromoted())
    {
      const Square up = Board_Table.nextSquare(knight.owner(),
					       knight.square(), U);
      const Piece up_piece = state.pieceAt(up);
      if ((up_piece.isEmpty() && state.hasPieceOnStand<PAWN>(alt(knight.owner())) &&
	   !state.isPawnMaskSet(alt(knight.owner()), knight.square().x()) &&
	   state.countEffect(knight.owner(), up) <=
	   state.countEffect(alt(knight.owner()), up)) ||
	  (state.hasEffectByPtypeStrict<PAWN>(alt(knight.owner()), up) &&
	   (up_piece.isEmpty() || up_piece.owner() == knight.owner()) &&
	   state.countEffect(knight.owner(), up) <
	   state.countEffect(alt(knight.owner()), up)))
      {
	const int y = (knight.owner() == BLACK ? knight.square().y() :
		       10 - knight.square().y());
	features.add(y - 1, knight.owner() == BLACK ? 1 : -1);
      }
    }
  }
}

void gpsshogi::
KnightHeadOppPiecePawnOnStand::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<KNIGHT>::indexMin;
       i < PtypeTraits<KNIGHT>::indexLimit;
       ++i)
  {
    const Piece knight = state.pieceOf(i);
    if (knight.isOnBoard() && !knight.isPromoted())
    {
      const Square up = Board_Table.nextSquare(knight.owner(),
						   knight.square(), U);
      const Piece up_piece = state.pieceAt(up);
      if (up_piece.isPiece() && up_piece.owner() != knight.owner() &&
	  state.hasPieceOnStand<PAWN>(up_piece.owner()))
      {
	const int y = (knight.owner() == BLACK ? knight.square().y() :
		       10 - knight.square().y());
	features.add(up_piece.ptype() * 9 + y - 1,
		     knight.owner() == BLACK ? 1 : -1);
      }
    }
  }
}

void gpsshogi::
LanceEffectPieceKingRelative::addOne(
  const NumEffectState &state,
  const Square pos,
  const Player player,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const Offset offset = Board_Table.getOffset(player, U);
  const Square self_king = state.kingSquare(player);
  const Square opp_king = state.kingSquare(alt(player));
  const int weight = (player == BLACK ? 1 : -1);
  Square p = pos + offset;
  while (state.pieceAt(p).isEmpty())
  {
    p += offset;
  }
  if (!p.isOnBoard())
  {
    features.add(0 + 0 + (PTYPEO_EDGE - PTYPEO_MIN) * 17 * 9, weight);
    features.add(0 + 0 + (PTYPEO_EDGE - PTYPEO_MIN) * 17 * 9 + 4896, weight);
  }
  else
  {
    features.add(index(player, p, opp_king, state.pieceAt(p).ptypeO(),
		       true), weight);
    features.add(index(player, p, self_king, state.pieceAt(p).ptypeO(),
		       false), weight);
  }
}

void gpsshogi::
LanceEffectPieceKingRelative::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<LANCE>::indexMin;
       i < PtypeTraits<LANCE>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && !p.isPromoted())
    {
      addOne(state, p.square(), p.owner(), features);
    }
  }
}

void gpsshogi::
SilverHeadPawnKingRelative::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<SILVER>::indexMin;
       i < PtypeTraits<SILVER>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard() && !p.isPromoted())
    {
      const Square up = Board_Table.nextSquare(p.owner(), p.square(), U);
      if (!up.isOnBoard())
	continue;
      const Piece up_piece = state.pieceAt(up);
      if (up_piece.isEmpty() &&
	  (state.hasEffectByPtypeStrict<PAWN>(alt(p.owner()), up)
	   || !state.isPawnMaskSet(alt(p.owner()), p.square().x())))
      {
	const Square king = state.kingSquare(p.owner());
	const int x_diff = std::abs(king.x() - p.square().x());
	const int y_diff = (p.owner() == BLACK ?
			    p.square().y() - king.y() :
			    king.y() - p.square().y());
	features.add(x_diff + 9 * (y_diff + 8), p.owner() == BLACK ? 1 : -1);
      }
    }
  }
}

void gpsshogi::
GoldKnightKingRelative::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit;
       ++i)
  {
    const Piece p = state.pieceOf(i);
    if (p.isOnBoard())
    {
      const Square uur =
	Board_Table.nextSquare(p.owner(), p.square(), UUR);
      const Square uul =
	Board_Table.nextSquare(p.owner(), p.square(), UUL);
      if ((uul.isOnBoard() && state.pieceAt(uul).isEmpty() &&
	   !state.hasEffectAt(p.owner(), uul)) ||
	  (uur.isOnBoard() && state.pieceAt(uur).isEmpty() &&
	   !state.hasEffectAt(p.owner(), uur)))
      {
	const Square king = state.kingSquare(p.owner());
	const int x_diff = std::abs(king.x() - p.square().x());
	const int y_diff = (p.owner() == BLACK ?
			    p.square().y() - king.y() :
			    king.y() - p.square().y());
	features.add(x_diff + 9 * (y_diff + 8), p.owner() == BLACK ? 1 : -1);
      }
    }
  }
}

void gpsshogi::
PawnAttackBase::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<Square, 2> kings = {{ state.kingSquare<BLACK>(),
				       state.kingSquare<WHITE>() }};
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit;
       ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (!pawn.isOnBoard() || pawn.isPromoted())
    {
      continue;
    }
    const Square up = Board_Table.nextSquare(pawn.owner(),
						 pawn.square(), U);
    const bool has_pawn_effect =
      state.hasEffectByPtypeStrict<PAWN>(alt(pawn.owner()), up);
    features.add(index(pawn.owner(),
		       kings[alt(pawn.owner())], pawn.square(),
		       has_pawn_effect,
		       state.effectSetAt(pawn.square()) &
		       state.piecesOnBoard(pawn.owner())),
		 pawn.owner() == BLACK ? 1 : -1);
  }
}



void gpsshogi::
SilverAdvance26::featuresOneNonUniq
(const NumEffectState &state, IndexCacheI<MaxActiveWithDuplication> &features) const
{
  const CArray<std::pair<Square,Ptype>,5> pattern = {{
      std::make_pair( Square(2,6), SILVER ),
      std::make_pair( Square(1,5), PAWN ),
      std::make_pair( Square(3,7), KNIGHT ),
      std::make_pair( Square(2,5), PAWN ),
      std::make_pair( Square(3,6), PAWN ),
    }};
  bool match = state.kingSquare(BLACK).x() >= 5;
  if (match) {
    for (size_t i=0; i<pattern.size(); ++i) {
      const Piece p = state.pieceAt(pattern[i].first);
      if (p.ptype() != pattern[i].second || p.owner() != BLACK) {
	match = false;
	break;
      }
    }
    if (match)
      features.add(0, 1);
  }
  match = state.kingSquare(WHITE).x() <= 5;
  if (match) {
    for (size_t i=0; i<pattern.size(); ++i) {
      const Piece p = state.pieceAt(pattern[i].first.rotate180());
      if (p.ptype() != pattern[i].second || p.owner() != WHITE) {
	match = false;
	break;
      }
    }
    if (match)
      features.add(0, -1);
  }
}



void gpsshogi::
Promotion37::featuresOneNonUniq
(const NumEffectState &state, IndexCacheI<MaxActiveWithDuplication> &features) const
{
  CArray<int,PTYPE_SIZE> count = {{ 0 }};
  // black
  for (int x=1; x<=9; ++x) {
    const Square target(x,3);
    if (! state[target].isEmpty())
      continue;
    int a = state.countEffect(BLACK, target);
    const int d = state.countEffect(WHITE, target);
    if (a > 0 && a == d)
      a += AdditionalEffect::hasEffect(state, target, BLACK);
    if (a <= d)
      continue;
    const Ptype ptype = state.findCheapAttack(BLACK, target).ptype();
    if (isPiece(ptype) && ! isPromoted(ptype))
      count[ptype]++;
  }
  for (int p=PTYPE_BASIC_MIN; p<=PTYPE_MAX; ++p) {
    if (count[p] > 0)
      features.add(p, 1);
    if (count[p] > 1)
      features.add(p-8, count[p]-1);
  }
  // white
  count.fill(0);
  for (int x=1; x<=9; ++x) {
    const Square target(x,7);
    if (! state[target].isEmpty())
      continue;
    int a = state.countEffect(WHITE, target);
    const int d = state.countEffect(BLACK, target);
    if (a > 0 && a == d)
      a += AdditionalEffect::hasEffect(state, target, WHITE);
    if (a <= d)
      continue;
    const Ptype ptype = state.findCheapAttack(WHITE, target).ptype();
    if (isPiece(ptype) && ! isPromoted(ptype))
      count[ptype]++;
  }
  for (int p=PTYPE_BASIC_MIN; p<=PTYPE_MAX; ++p) {
    if (count[p] > 0)
      features.add(p, -1);
    if (count[p] > 1)
      features.add(p-8, -(count[p]-1));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


