/* ptypeEval.cc
 */
#include "eval/ptypeEval.h"

inline int gpsshogi::PtypeCountFeatures::
index(Ptype ptype, int count)
{
  assert(count > 0);
  return Ptype_Table.getIndexMin(unpromote(ptype)) +
    (isPromoted(ptype) ? 40 : 0) +
    count - 1;
}

void gpsshogi::PtypeCountFeatures::
featuresOneNonUniq(const NumEffectState &state,
		   IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  CArray2d<int, 2, PTYPE_SIZE> count;
  CArray2d<int, 2, PTYPE_SIZE> board_count;
  count.fill(0);
  board_count.fill(0);
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() == KING)
    {
      continue;
    }
    ++count[piece.owner()][piece.ptype()];
    if (piece.isOnBoard())
    {
      ++board_count[piece.owner()][piece.ptype()];
    }
  }
  for (int i = 0; i < 2; ++i)
  {
    for (int j = 0; j < PTYPE_SIZE; ++j)
    {
      if (count[i][j] != 0)
      {
	const Ptype ptype = static_cast<Ptype>(j);
	const int index = this->index(ptype, count[i][j]);
	diffs.add(index, i == playerToIndex(BLACK) ? 1 : -1);
      }
      if (board_count[i][j] != 0)
      {
	const Ptype ptype = static_cast<Ptype>(j);
	const int index = this->index(ptype, board_count[i][j]) + 80;
	diffs.add(index, i == playerToIndex(BLACK) ? 1 : -1);
      }
    }
  }
}

void gpsshogi::
PtypeCountFeatures::showAllOne(const Weights &weights,
		       int n,
		       std::ostream &os) const
{
  os << name() << " " << n << std::endl;
  for (int i = PTYPE_MIN; i <= PTYPE_MAX; ++i)
  {
    const Ptype ptype = static_cast<Ptype>(i);
    if (!isPiece(ptype))
      continue;
    os << ptype << " (all)";
    for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	 j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
    {
      os << " " << weights.value(j + n * dimension() +
				 (isPromoted(ptype) ? 40 : 0));
    }
    os << std::endl;
    os << ptype << " (on board)";
    for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	 j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
    {
      os << " " << weights.value(j + n * dimension() + 80 +
				 (isPromoted(ptype) ? 40 : 0));
    }
    os << std::endl;
  }
}

osl::MultiInt gpsshogi::PtypeCount::
evalWithUpdateMulti(const NumEffectState& state, Move moved,
		    const MultiInt &last_values,
		    CArray<MultiInt,2>& /*save_state*/) const
{
  if (moved.isPass())
    return last_values;
  if (moved.capturePtype() == PTYPE_EMPTY && !moved.isPromotion()
      && !moved.isDrop())
    return last_values;

  MultiInt diff;
  const Ptype ptype = moved.ptype(), uptype = unpromote(ptype);
  const Player player = moved.player();
  CArray<int,2> ptype_board_count = {{0}};
  for (int j = Ptype_Table.getIndexMin(uptype);
       j < Ptype_Table.getIndexLimit(uptype); ++j)
  {
    const Piece p = state.pieceOf(j);
    if (p.isOnBoardByOwner(player))
      ++ptype_board_count[p.ptype() != uptype]; // 0 unpromote, 1 promote
  }
  if (moved.isDrop()) 
  {
    const int index = PtypeCountFeatures::index(ptype, ptype_board_count[0])+80;
    diff += weight.value(index);
    if (ptype_board_count[0] > 1) 
    {
      const int old_index = PtypeCountFeatures::index
	(ptype, ptype_board_count[0]-1) + 80;
      diff -= weight.value(old_index);
    }
    return last_values + ((player == BLACK) ? diff : -diff);
  }
  if (moved.isPromotion()) 
  {
    // promoted
    assert(ptype_board_count[1] > 0);
    const int pindex = PtypeCountFeatures::index(ptype, ptype_board_count[1]);
    diff += weight.value(pindex);
    diff += weight.value(pindex+80); // ptype_count==ptype_board_count (promoted)
    if (ptype_board_count[1] > 1) 
    {
      diff -= weight.value(pindex-1);
      diff -= weight.value(pindex+80-1);
    }
    // not promoted
    const int hand = state.countPiecesOnStand(player, uptype)
      - ((moved.capturePtype() != PTYPE_EMPTY && uptype == unpromote(moved.capturePtype())) ? 1 : 0);
    const int old_uindex 
      = PtypeCountFeatures::index(uptype, ptype_board_count[0]+hand+1);
    const int old_uindex_board
      = PtypeCountFeatures::index(uptype, ptype_board_count[0]+1)+80;
    diff -= weight.value(old_uindex);
    diff -= weight.value(old_uindex_board);
    if (hand + ptype_board_count[0] > 0) 
    {
      diff += weight.value(old_uindex-1);
      if (ptype_board_count[0] > 0) 
	diff += weight.value(old_uindex_board-1);
    }
  }
  if (moved.capturePtype() == PTYPE_EMPTY)
    return last_values + ((player == BLACK) ? diff : -diff);
  const Ptype cptype = moved.capturePtype(), ucptype = unpromote(cptype);
  CArray2d<int,2,2> ctype_board_count = {{{0}}};
  for (int j = Ptype_Table.getIndexMin(ucptype);
       j < Ptype_Table.getIndexLimit(ucptype); ++j)
  {
    const Piece p = state.pieceOf(j);
    if (p.isOnBoard())
      ++ctype_board_count[p.owner()][p.ptype() != ucptype]; // 0 unpromote, 1 promote
  }
  // player
  {
    const int chand = state.countPiecesOnStand(player, ucptype);
    const int index = PtypeCountFeatures::index(ucptype, ctype_board_count[player][0]+chand);
    diff += weight.value(index);
    if (ctype_board_count[player][0]+chand > 1) 
      diff -= weight.value(index-1);
  }
  // opponent
  {
    const int cur_board = ctype_board_count[alt(player)][cptype != ucptype];
    const int cur_hand = (cptype == ucptype) ? state.countPiecesOnStand(alt(player), ucptype) : 0;
    const int old_index = PtypeCountFeatures::index(cptype, cur_board+cur_hand+1);
    const int old_index_board = PtypeCountFeatures::index(cptype, cur_board+1) + 80;
    if (cur_board + cur_hand)
    {
      diff += -weight.value(old_index-1);
      if (cur_board)
	diff += -weight.value(old_index_board-1);
    }
    diff -= -weight.value(old_index);
    diff -= -weight.value(old_index_board);
  }

  return last_values + ((player == BLACK) ? diff : -diff);
}


template <bool Attack>
int gpsshogi::PtypeCountXYBase<Attack>::
indexX(int x, Ptype ptype, int count)
{
  assert(count > 0);
  return x - 1 + 5 *
	  (Ptype_Table.getIndexMin(unpromote(ptype)) + (isPromoted(ptype) ? 40 : 0) +
	   count - 1);
}
template <bool Attack>
int gpsshogi::PtypeCountXYBase<Attack>::
indexY(int y, Ptype ptype, int count)
{
  assert(count > 0);
  return y - 1 + 9 *
	  (Ptype_Table.getIndexMin(unpromote(ptype)) + (isPromoted(ptype) ? 40 : 0) +
	   count - 1) + 800;
}
template <bool Attack>
int gpsshogi::PtypeCountXYBase<Attack>::
indexXBoard(int x, Ptype ptype, int count)
{
  assert(count > 0);
  return x - 1 + 5 *
	  (Ptype_Table.getIndexMin(unpromote(ptype)) + (isPromoted(ptype) ? 40 : 0) +
	   count - 1) + 400;
}
template <bool Attack>
int gpsshogi::PtypeCountXYBase<Attack>::
indexYBoard(int y, Ptype ptype, int count)
{
  assert(count > 0);
  return y - 1 + 9 *
	  (Ptype_Table.getIndexMin(unpromote(ptype)) + (isPromoted(ptype) ? 40 : 0) +
	   count - 1) + 720 + 800;
}

template <bool Attack>
void gpsshogi::PtypeCountXYBase<Attack>::
featuresOneNonUniq(const NumEffectState &state,
		   IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  CArray2d<int, 2, PTYPE_SIZE> count;
  CArray2d<int, 2, PTYPE_SIZE> board_count;
  count.fill(0);
  board_count.fill(0);
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() == KING)
    {
      continue;
    }
    ++count[piece.owner()][piece.ptype()];
    if (piece.isOnBoard())
    {
      ++board_count[piece.owner()][piece.ptype()];
    }
  }
  CArray<int, 2> kings_x = {{ state.kingSquare<BLACK>().x(),
			      state.kingSquare<WHITE>().x() }};
  CArray<int, 2> kings_y = {{ state.kingSquare<BLACK>().y(),
			      10 - state.kingSquare<WHITE>().y() }};
  if (kings_x[0] > 5)
    kings_x[0] = 10 - kings_x[0];
  if (kings_x[1] > 5)
    kings_x[1] = 10 - kings_x[1];
  for (int i = 0; i < 2; ++i)
  {
    const int x = kings_x[Attack ? 1 - i : i];
    const int y = kings_y[Attack ? 1 - i : i];
    for (int j = 0; j < PTYPE_SIZE; ++j)
    {
      if (count[i][j] != 0)
      {
	const Ptype ptype = static_cast<Ptype>(j);
	const int index_x = indexX(x, ptype, count[i][j]);
	const int index_y = indexY(y, ptype, count[i][j]);
	diffs.add(index_x, i == playerToIndex(BLACK) ? 1 : -1);
	diffs.add(index_y, i == playerToIndex(BLACK) ? 1 : -1);
      }
      if (board_count[i][j] != 0)
      {
	const Ptype ptype = static_cast<Ptype>(j);
	const int index_x = indexXBoard(x, ptype, board_count[i][j]);
	const int index_y = indexYBoard(y, ptype, board_count[i][j]);
	diffs.add(index_x, i == playerToIndex(BLACK) ? 1 : -1);
	diffs.add(index_y, i == playerToIndex(BLACK) ? 1 : -1);
      }
    }
  }
}

template <bool Attack>
osl::MultiInt gpsshogi::PtypeCountXYBase<Attack>::
evalWithUpdateMulti(
    const NumEffectState& state,
    Move moved,
    const MultiInt &last_values, const MultiWeights& weight) const
{
  assert(moved.isNormal());
  assert(moved.ptype() != KING);
  if (moved.capturePtype() == PTYPE_EMPTY && !moved.isPromotion()
      && !moved.isDrop())
    return last_values;

  CArray<int, 2> kings_x = {{ state.kingSquare<BLACK>().x(),
			      state.kingSquare<WHITE>().x() }};
  CArray<int, 2> kings_y = {{ state.kingSquare<BLACK>().y(),
			      10 - state.kingSquare<WHITE>().y() }};
  if (kings_x[0] > 5)
    kings_x[0] = 10 - kings_x[0];
  if (kings_x[1] > 5)
    kings_x[1] = 10 - kings_x[1];

  MultiInt diff;
  const Ptype ptype = moved.ptype(), uptype = unpromote(ptype);
  const Player player = moved.player();
  CArray<int,2> ptype_board_count = {{0}};
  for (int j = Ptype_Table.getIndexMin(uptype);
       j < Ptype_Table.getIndexLimit(uptype); ++j)
  {
    const Piece p = state.pieceOf(j);
    if (p.isOnBoardByOwner(player))
      ++ptype_board_count[p.ptype() != uptype]; // 0 unpromote, 1 promote
  }

  const int x = kings_x[Attack ? alt(player) : player];
  const int y = kings_y[Attack ? alt(player) : player];

  if (moved.isDrop()) 
  {
    const int index_x = indexXBoard(x, ptype, ptype_board_count[0]);
    const int index_y = indexYBoard(y, ptype, ptype_board_count[0]);
    diff += weight.value(index_x);
    diff += weight.value(index_y);
    if (ptype_board_count[0] > 1) 
    {
      const int old_index_x = indexXBoard(x, ptype, ptype_board_count[0]-1);
      const int old_index_y = indexYBoard(y, ptype, ptype_board_count[0]-1);
      diff -= weight.value(old_index_x);
      diff -= weight.value(old_index_y);
    }
    return last_values + ((player == BLACK) ? diff : -diff);
  }

  if (moved.isPromotion()) 
  {
    // promoted
    assert(ptype_board_count[1] > 0);
    diff += weight.value(indexX(x, ptype, ptype_board_count[1]));
    diff += weight.value(indexY(y, ptype, ptype_board_count[1]));
    diff += weight.value(indexXBoard(x, ptype, ptype_board_count[1])); // ptype_count==ptype_board_count (promoted)
    diff += weight.value(indexYBoard(y, ptype, ptype_board_count[1]));
    if (ptype_board_count[1] > 1) 
    {
      diff -= weight.value(indexX(x, ptype, ptype_board_count[1]-1));
      diff -= weight.value(indexY(y, ptype, ptype_board_count[1]-1));
      diff -= weight.value(indexXBoard(x, ptype, ptype_board_count[1]-1));
      diff -= weight.value(indexYBoard(y, ptype, ptype_board_count[1]-1));
    }
    // not promoted
    const int hand = state.countPiecesOnStand(player, uptype)
      - ((moved.capturePtype() != PTYPE_EMPTY && uptype == unpromote(moved.capturePtype())) ? 1 : 0);
    diff -= weight.value(indexX(x, uptype, ptype_board_count[0]+hand+1));
    diff -= weight.value(indexY(y, uptype, ptype_board_count[0]+hand+1));
    diff -= weight.value(indexXBoard(x, uptype, ptype_board_count[0]+1));
    diff -= weight.value(indexYBoard(y, uptype, ptype_board_count[0]+1));
    if (hand + ptype_board_count[0] > 0) 
    {
      diff += weight.value(indexX(x, uptype, ptype_board_count[0]+hand));
      diff += weight.value(indexY(y, uptype, ptype_board_count[0]+hand));
      if (ptype_board_count[0] > 0) {
	diff += weight.value(indexXBoard(x, uptype, ptype_board_count[0]));
	diff += weight.value(indexYBoard(y, uptype, ptype_board_count[0]));
      }
    }
  }
  if (moved.capturePtype() == PTYPE_EMPTY)
    return last_values + ((player == BLACK) ? diff : -diff);

  const Ptype cptype = moved.capturePtype(), ucptype = unpromote(cptype);
  CArray2d<int,2,2> ctype_board_count = {{{0}}};
  for (int j = Ptype_Table.getIndexMin(ucptype);
       j < Ptype_Table.getIndexLimit(ucptype); ++j)
  {
    const Piece p = state.pieceOf(j);
    if (p.isOnBoard())
      ++ctype_board_count[p.owner()][p.ptype() != ucptype]; // 0 unpromote, 1 promote
  }
  // player
  {
    const int chand = state.countPiecesOnStand(player, ucptype);
    const int cboard = ctype_board_count[player][0];
    diff += weight.value(indexX(x, ucptype, cboard+chand));
    diff += weight.value(indexY(y, ucptype, cboard+chand));
    if (cboard+chand > 1) 
    {
      diff -= weight.value(indexX(x, ucptype, cboard+chand-1));
      diff -= weight.value(indexY(y, ucptype, cboard+chand-1));
    }
  }
  // opponent
  {
    const int op_x = kings_x[Attack ? player : alt(player)];
    const int op_y = kings_y[Attack ? player : alt(player)];
    const int cur_board = ctype_board_count[alt(player)][cptype != ucptype];
    const int cur_hand = (cptype == ucptype) ? state.countPiecesOnStand(alt(player), ucptype) : 0;
    if (cur_board + cur_hand)
    {
      diff += -weight.value(indexX(op_x, cptype, cur_board+cur_hand));
      diff += -weight.value(indexY(op_y, cptype, cur_board+cur_hand));
      if (cur_board)
      {
	diff += -weight.value(indexXBoard(op_x, cptype, cur_board));
	diff += -weight.value(indexYBoard(op_y, cptype, cur_board));
      }
    }
    diff -= -weight.value(indexX(op_x, cptype, cur_board+cur_hand+1));
    diff -= -weight.value(indexY(op_y, cptype, cur_board+cur_hand+1));
    diff -= -weight.value(indexXBoard(op_x, cptype, cur_board+1));
    diff -= -weight.value(indexYBoard(op_y, cptype, cur_board+1));
  }

  return last_values + ((player == BLACK) ? diff : -diff);
}

template <bool Attack>
void gpsshogi::
PtypeCountXYBase<Attack>::showAllOne(const Weights &weights,
				     int n,
				     std::ostream &os) const
{
  os << name() << " " << n << std::endl;
  for (int x = 0; x < 5; ++x)
  {
    os << "King X: " << x + 1<< std::endl;
    for (int i = PTYPE_MIN; i <= PTYPE_MAX; ++i)
    {
      const Ptype ptype = static_cast<Ptype>(i);
      if (!isPiece(ptype))
	continue;
      os << ptype << " (all)";
      for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	   j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
      {
	os << " " << weights.value(n * dimension() + x +
				   5 * (j + (isPromoted(ptype) ? 40 : 0)));
      }
      os << std::endl;
      os << ptype << " (on board)";
      for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	   j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
      {
	os << " " << weights.value(n * dimension() + x + 400 +
				   5 * (j + (isPromoted(ptype) ? 40 : 0)));
      }
      os << std::endl;
    }
  }
  for (int y = 0; y < 9; ++y)
  {
    os << "King Y: " << y + 1 << std::endl;
    for (int i = PTYPE_MIN; i <= PTYPE_MAX; ++i)
    {
      const Ptype ptype = static_cast<Ptype>(i);
      if (!isPiece(ptype))
	continue;
      os << ptype << " (all)";
      for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	   j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
      {
	os << " " << weights.value(n * dimension() + y + 800 +
				   9 * (j + (isPromoted(ptype) ? 40 : 0)));
      }
      os << std::endl;
      os << ptype << " (on board)";
      for (int j = Ptype_Table.getIndexMin(unpromote(ptype));
	   j < Ptype_Table.getIndexLimit(unpromote(ptype)); ++j)
      {
	os << " " << weights.value(n * dimension() + y + 720 + 800 +
				   9 * (j + (isPromoted(ptype) ? 40 : 0)));
      }
      os << std::endl;
    }
  }
}

void gpsshogi::PtypeYPawnYFeature::
featuresOneNonUniq(const NumEffectState &state,
		   IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  CArray2d<int, 2, 9> pawn_y;
  pawn_y.fill(0);
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit;
       ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (!pawn.isOnBoard() || pawn.isPromoted())
      continue;
    pawn_y[pawn.owner()][pawn.square().x() - 1] = pawn.square().y();
  }

  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    // only skip pawn, not ppawns
    if (piece.ptype() == PAWN)
      continue;
    if (!piece.isOnBoard())
      continue;
    diffs.add(index(piece.owner(), piece.ptype(), piece.square().y(),
		    pawn_y[piece.owner()][piece.square().x() - 1]),
	      piece.owner() == BLACK ? 1 : -1);
  }
}

osl::MultiInt gpsshogi::PtypeYPawnY::
evalWithUpdateMulti(const NumEffectState& state,
		    Move moved,
		    const MultiInt &last_values,
		    CArray<MultiInt,2>& /*save_state*/) const
{
  if (moved.isPass())
    return last_values;

  const Player player = moved.player();
  MultiInt diff;
  if (moved.isDrop()) 
  {
    if (moved.ptype() != PAWN)
    {
      int pawn_y=9;
      for (; pawn_y >= 1; --pawn_y)
      {
	const Piece p = state.pieceAt(Square(moved.to().x(), pawn_y));
	if (p.isOnBoardByOwner(player) && p.ptype() == PAWN)
	  break;
      }
      const int index
	= PtypeYPawnYFeature::index(player, moved.ptype(), moved.to().y(),
				    pawn_y);
      diff += weight.value(index);
      return last_values + ((player == BLACK) ? diff : -diff);
    }
    for (int y=1; y<=9; ++y)
    {
      if (y == moved.to().y())
	continue;
      const Piece p = state.pieceAt(Square(moved.to().x(), y));
      if (! p.isOnBoardByOwner(player))
	continue;
      const int index
	= PtypeYPawnYFeature::index(player, p.ptype(), y,
				    moved.to().y());
      diff += weight.value(index);
      const int old_index
	= PtypeYPawnYFeature::index(player, p.ptype(), y, 0);
      diff -= weight.value(old_index);
    }
    return last_values + ((player == BLACK) ? diff : -diff);
  }

  if (moved.capturePtype() == PAWN)
  {
    const int x = moved.to().x(), old_y = moved.to().y();
    const int new_y = 0;
    for (int y=1; y<=9; ++y)
    {
      const Piece p = state.pieceAt(Square(x, y));
      if (! p.isOnBoardByOwner(alt(player)))
	continue;
      const int index
	= PtypeYPawnYFeature::index(alt(player), p.ptype(), y, new_y);
      diff += -weight.value(index);
      const int old_index
	= PtypeYPawnYFeature::index(alt(player), p.ptype(), y, old_y);
      diff -= -weight.value(old_index);
    }    
  }

  if (moved.oldPtype() == PAWN)
  {
    const int x = moved.from().x(), old_y = moved.from().y();
    const int new_y = moved.isPromotion() ? 0 : moved.to().y();    
    Piece op_pawn;
    for (int y=1; y<=9; ++y)
    {
      if (y == new_y)
	continue;
      const Piece p = state.pieceAt(Square(x, y));
      if (! p.isPiece())
	continue;
      if (p.owner() != player) 
      {
	if (p.ptype() == PAWN)
	  op_pawn = p;
	continue;
      }
      const int index
	= PtypeYPawnYFeature::index(player, p.ptype(), y, new_y);
      diff += weight.value(index);
      if (y != moved.to().y()) 
      {
	const int old_index
	  = PtypeYPawnYFeature::index(player, p.ptype(), y, old_y);
	diff -= weight.value(old_index);
      }
    }
    if (moved.capturePtype() != PTYPE_EMPTY
	&& moved.capturePtype() != PAWN) 
    {
      const int old_index
	= PtypeYPawnYFeature::index(alt(player), moved.capturePtype(), moved.to().y(), 
				    op_pawn.isPiece() ? op_pawn.square().y() : 0);
      diff -= -weight.value(old_index);
    }
    return last_values + ((player == BLACK) ? diff : -diff);
  }

  CArray2d<int, 2, 9> pawn_y = {{{ 0 }}};
  for (int i = PtypeTraits<PAWN>::indexMin;
       i < PtypeTraits<PAWN>::indexLimit;
       ++i)
  {
    const Piece pawn = state.pieceOf(i);
    if (!pawn.isOnBoard() || pawn.isPromoted())
      continue;
    pawn_y[pawn.owner()][pawn.square().x() - 1] = pawn.square().y();
  }
  assert(! moved.from().isPieceStand());
  {
    const int index
      = PtypeYPawnYFeature::index(player, moved.oldPtype(), moved.from().y(),
			   pawn_y[player][moved.from().x() - 1]);
    diff -= weight.value(index);
  }
  if (moved.capturePtype() != PTYPE_EMPTY && moved.capturePtype() != PAWN)
  {
    const int index
      = PtypeYPawnYFeature::index(alt(player), moved.capturePtype(), moved.to().y(),
			   pawn_y[alt(player)][moved.to().x() - 1]);
    diff -= -weight.value(index);
  }
  const int index
    = PtypeYPawnYFeature::index(player, moved.ptype(), moved.to().y(),
			 pawn_y[player][moved.to().x() - 1]);
  diff += weight.value(index);
  return last_values + ((player == BLACK) ? diff : -diff);
}

void gpsshogi::PtypeCombination::
featuresOneNonUniq(const NumEffectState &state,
		   IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  CArray<int, 2> ptypes;
  ptypes.fill(0);
  updatePtype<PAWN, 12>(state, ptypes);
  updatePtype<LANCE, 11>(state, ptypes);
  updatePtype<KNIGHT, 10>(state, ptypes);
  updatePtype<SILVER, 9>(state, ptypes);
  updatePtype<BISHOP, 8>(state, ptypes);
  updatePtype<ROOK, 7>(state, ptypes);
  updatePtype<GOLD, 0>(state, ptypes);
  if (ptypes[0] == ptypes[1])
    return;

  diffs.add(ptypes[0], 1);
  diffs.add(ptypes[1], -1);
}

const std::string gpsshogi::PtypeCombination::
describe(size_t local_index) const
{
  static const CArray<const char *, 13> pieces = {{
      "G", "PR", "PB", "PS", "PN", "PL", "PP",
      "R", "B", "S", "K", "L", "P"
    }};
  std::string ret;
  for (size_t i=0; i<pieces.size(); ++i)
    if ((1<<i)&local_index)
      ret += pieces[i];
  return ret;
}


void gpsshogi::PtypeCombinationY::
featuresOneNonUniq(const NumEffectState &state,
		   IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  CArray<int, 2> ptypes;
  ptypes.fill(0);
  updatePtype<PAWN, 12>(state, ptypes);
  updatePtype<LANCE, 11>(state, ptypes);
  updatePtype<KNIGHT, 10>(state, ptypes);
  updatePtype<SILVER, 9>(state, ptypes);
  updatePtype<BISHOP, 8>(state, ptypes);
  updatePtype<ROOK, 7>(state, ptypes);
  updatePtype<GOLD, 0>(state, ptypes);

  diffs.add((state.kingSquare<BLACK>().y() - 1) * (1 << 13) + ptypes[0],
	    1);
  diffs.add((9 - state.kingSquare<WHITE>().y()) * (1 << 13) + ptypes[1],
	    -1);
}

void gpsshogi::PtypeCombinationOnBoard::
featuresOneNonUniq(const NumEffectState &state,
		   IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  CArray<int, 2> ptypes;
  ptypes.fill(0);
  updatePtype<PAWN, 12>(state, ptypes);
  updatePtype<LANCE, 11>(state, ptypes);
  updatePtype<KNIGHT, 10>(state, ptypes);
  updatePtype<SILVER, 9>(state, ptypes);
  updatePtype<BISHOP, 8>(state, ptypes);
  updatePtype<ROOK, 7>(state, ptypes);
  updatePtype<GOLD, 0>(state, ptypes);
  if (ptypes[0] == ptypes[1])
    return;

  diffs.add(ptypes[0], 1);
  diffs.add(ptypes[1], -1);
}


void gpsshogi::PtypeCombinationOnBoardY::
featuresOneNonUniq(const NumEffectState &state,
		   IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  CArray<int, 2> ptypes;
  ptypes.fill(0);
  updatePtype<PAWN, 12>(state, ptypes);
  updatePtype<LANCE, 11>(state, ptypes);
  updatePtype<KNIGHT, 10>(state, ptypes);
  updatePtype<SILVER, 9>(state, ptypes);
  updatePtype<BISHOP, 8>(state, ptypes);
  updatePtype<ROOK, 7>(state, ptypes);
  updatePtype<GOLD, 0>(state, ptypes);

  diffs.add((state.kingSquare<BLACK>().y() - 1) * (1 << 13) + ptypes[0],
	    1);
  diffs.add((9 - state.kingSquare<WHITE>().y()) * (1 << 13) + ptypes[1],
	    -1);
}

namespace gpsshogi
{
  template class PtypeCountXYBase<true>;
  template class PtypeCountXYBase<false>;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

