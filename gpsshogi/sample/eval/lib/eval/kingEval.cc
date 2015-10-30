/* kingEval.cc
 */
#include "eval/kingEval.h"
#include "eval/indexCache.h"
#include "osl/bits/centering5x3.h"
#include "osl/bits/king8Info.h"
#include <iostream>
#include <iomanip>


void gpsshogi::
PieceKingRelativeFeature::featuresOneNonUniq(const NumEffectState &state,
					     IndexCacheI<MaxActiveWithDuplication> &feature_count) const
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.ptype() == osl::KING || !piece.isOnBoard())
      continue;
    Player pl = piece.owner();
    const int index_attack = index(piece.owner(), kings[alt(pl)],
				   piece);
    const int index_defense = index(piece.owner(), kings[pl],
				    piece) + ONE_DIM;
    const int weight = (pl == BLACK ? 1 : -1);
    feature_count.add(index_attack, weight);
    feature_count.add(index_defense, weight);
  }
}

void gpsshogi::
PieceKingRelativeFeature::featuresOne(const NumEffectState &state,
                 features_one_t &out) const
{
  const CArray<Square,2> kings = {{
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  CArray<IndexCacheI<80>,2> feature_count;
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.ptype() == osl::KING || !piece.isOnBoard())
      continue;
    Player pl = piece.owner();
    const int index_attack = index(piece.owner(), kings[alt(pl)],
              piece);
    const int index_defense = index(piece.owner(), kings[pl],
               piece) + ONE_DIM;
    const int weight = (pl == BLACK ? 1 : -1);
    feature_count[0].add(index_attack, weight);
    feature_count[1].add(index_defense, weight);
  }
  feature_count[0].output(out);
  feature_count[1].output(out);
}

template <int N>
osl::CArray<int, N> gpsshogi::
PieceKingRelativeFeature::evalWithUpdateMulti(const NumEffectState& state,
					      Move moved,
					      const CArray<int, N> &last_values,
					      const Weights& w)
{
  assert(moved.ptype() != KING);
  CArray<int, N> value = last_values;
  if (moved.isDrop())
  {
    Player p = moved.player();
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   moved.ptype(), moved.to());
    const int index_defense = index(p, state.kingSquare(p),
				    moved.ptype(), moved.to()) + ONE_DIM;
    for (int i=0; i<N; ++i)
      value[i] +=
	(w.value(index_attack+i*ONE_DIM*2) + w.value(index_defense+i*ONE_DIM*2)) *
	(moved.player() == BLACK ? 1 : -1);
    return value;
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const Player p = alt(moved.player());
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   captured, moved.to());
    const int index_defense = index(p, state.kingSquare(p),
				    captured, moved.to()) + ONE_DIM;
    for (int i=0; i<N; ++i)
      value[i] +=
	(w.value(index_attack+i*ONE_DIM*2) + w.value(index_defense+i*ONE_DIM*2)) *
	(moved.player() == BLACK ? 1 : -1);
  }
  {
    const Player p = moved.player();
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   moved.oldPtype(), moved.from());
    const int index_defense = index(p, state.kingSquare(p),
				    moved.oldPtype(), moved.from()) + ONE_DIM;
    for (int i=0; i<N; ++i)
      value[i] -=
	(w.value(index_attack+i*ONE_DIM*2) + w.value(index_defense+i*ONE_DIM*2)) *
	(moved.player() == BLACK ? 1 : -1);
  }
  {
    const Player p = moved.player();
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   moved.ptype(), moved.to());
    const int index_defense = index(p, state.kingSquare(p),
				    moved.ptype(), moved.to()) + ONE_DIM;
    for (int i=0; i<N; ++i)
      value[i] +=
	(w.value(index_attack+i*ONE_DIM*2) + w.value(index_defense+i*ONE_DIM*2)) *
	(moved.player() == BLACK ? 1 : -1);
  }
  return value;
}
osl::MultiInt gpsshogi::
PieceKingRelativeFeature::evalWithUpdateStages(const NumEffectState& state,
					      Move moved,
					      const MultiInt &last_values,
					      const MultiWeights& w)
{
  assert(moved.ptype() != KING);
  MultiInt value = last_values;
  if (moved.isDrop())
  {
    Player p = moved.player();
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   moved.ptype(), moved.to());
    const int index_defense = index(p, state.kingSquare(p),
				    moved.ptype(), moved.to()) + ONE_DIM;
    if (moved.player() == BLACK)
      value += (w.value(index_attack) + w.value(index_defense));
    else
      value -= (w.value(index_attack) + w.value(index_defense));
    return value;
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const Player p = alt(moved.player());
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   captured, moved.to());
    const int index_defense = index(p, state.kingSquare(p),
				    captured, moved.to()) + ONE_DIM;
    if (moved.player() == BLACK)
      value += (w.value(index_attack) + w.value(index_defense));
    else
      value -= (w.value(index_attack) + w.value(index_defense));
  }
  {
    const Player p = moved.player();
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   moved.oldPtype(), moved.from());
    const int index_defense = index(p, state.kingSquare(p),
				    moved.oldPtype(), moved.from()) + ONE_DIM;
    if (moved.player() == BLACK)
      value -= (w.value(index_attack) + w.value(index_defense));
    else
      value += (w.value(index_attack) + w.value(index_defense));
  }
  {
    const Player p = moved.player();
    const int index_attack = index(p, state.kingSquare(alt(p)),
				   moved.ptype(), moved.to());
    const int index_defense = index(p, state.kingSquare(p),
				    moved.ptype(), moved.to()) + ONE_DIM;
    if (moved.player() == BLACK)
      value += (w.value(index_attack) + w.value(index_defense));
    else
      value -= (w.value(index_attack) + w.value(index_defense));
  }
  return value;
}

osl::MultiInt gpsshogi::
PieceKingRelativeStages::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  if (moved.ptype() == KING)
  {
    return evalMulti(state, saved_state);
  }
  return PieceKingRelativeFeature::evalWithUpdateStages(state, moved, last_values, weight);
}



#ifdef USE_OLD_FEATURE
template <int DIM, bool is_x>
int gpsshogi::PieceKingRelativeAbsBase<DIM, is_x>::eval(
  const NumEffectState &state) const
{
  int result = 0;
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.ptype() == osl::KING || !piece.isOnBoard())
      continue;
    result += this->value(index(state, piece)) *
      (piece.owner() == BLACK ? 1 : -1);
  }
  return result;
}

template <int DIM, bool is_x>
int gpsshogi::PieceKingRelativeAbsBase<DIM, is_x>::evalWithUpdate(
  const osl::NumEffectState &state,
  osl::Move moved, int last_value) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == osl::KING)
  {
    return eval(state);
  }
  if (moved.isDrop())
  {
    return last_value + value(index(state, moved.ptypeO(), moved.to())) *
      (moved.player() == BLACK ? 1 : -1);
  }
  int result = last_value;
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    result += value(index(state, moved.capturePtypeO(), moved.to())) *
      (moved.player() == BLACK ? 1 : -1);
  }
  result -= value(index(state, moved.oldPtypeO(), moved.from())) *
    (moved.player() == BLACK ? 1 : -1);
  result += value(index(state, moved.ptypeO(), moved.to())) *
    (moved.player() == BLACK ? 1 : -1);
  return result;
}

template <int DIM, bool is_x>
void gpsshogi::
PieceKingRelativeAbsBase<DIM, is_x>::features(
  const osl::NumEffectState &state, double& value,
  osl::stl::vector<std::pair<int, double> >&diffs,
  int offset) const
{
  value = 0.0;
  IndexCache<40> feature_count;
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() == KING || !piece.isOnBoard())
      continue;
    const int idx = index(state, piece);
    feature_count.add(idx, piece.owner() == BLACK ? 1.0 : -1.0);
    value += values[idx] * (piece.owner() == BLACK ? 1.0 : -1.0);
  }
  feature_count.uniqWrite(diffs, offset);
}

template <int DIM, bool is_x>
int gpsshogi::PieceKingRelativeAbsBase<DIM, is_x>::index(
  const NumEffectState &state,
  const PtypeO ptypeO,
  const Square position) const
{
  if (is_x)
  {
    // [PTYPE][KING_X (1-5)][X(1-9)][REL_Y(-8-+8)]
    const Player player = getOwner(ptypeO);
    const Square king =
      state.kingSquare(is_attack ? alt(player) : player);
    const int y = (player == osl::BLACK ? (king.y() - position.y()) : (position.y() - king.y()));
    int normalized_king_x, normalized_piece_x;
    if (king.x() < 5 || (king.x() == 5 && player == BLACK))
    {
      normalized_king_x = king.x();
      normalized_piece_x = position.x();
    }
    else
    {
      normalized_king_x = 10 - king.x();
      normalized_piece_x = 10 - position.x();
    }
    return (getPtype(ptypeO) - PTYPE_PIECE_MIN) * 5 * 9 * 17 +
      (normalized_king_x - 1) * 9 * 17 +
      (normalized_piece_x - 1) * 17 +
      y + 8;
  }
  else
  {
    // [PTYPE][KING_Y(1-9)][Y(1-9)][REL_X(-8-+8)]
    const Player player = getOwner(ptypeO);
    const Square king =
      state.kingSquare(is_attack ? alt(player) : player);
    const int x = (player == osl::BLACK ? (king.x() - position.x()) : (position.x() - king.x()));
    const int king_y = (player == osl::BLACK ? king.y() : 10 - king.y());
    const int piece_y =
      (player == osl::BLACK ? position.y() : 10 - position.y());

    return (getPtype(ptypeO) - PTYPE_PIECE_MIN) * 9 * 9 * 17 +
      (king_y - 1) * 9 * 17 +
      (piece_y - 1) * 17 +
      x + 8;
  }
}


int gpsshogi::PieceKingRelativeAbs::eval(
  const NumEffectState &state) const
{
  int result = 0;
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.ptype() == osl::KING || !piece.isOnBoard())
      continue;
    result += this->value(index(state, piece)) *
      (piece.owner() == BLACK ? 1 : -1);
  }
  return result;
}

int gpsshogi::PieceKingRelativeAbs::evalWithUpdate(
  const osl::NumEffectState &state,
  osl::Move moved, int last_value) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == osl::KING)
  {
    return eval(state);
  }
  if (moved.isDrop())
  {
    return last_value + value(index(state, moved.ptypeO(), moved.to())) *
      (moved.player() == BLACK ? 1 : -1);
  }
  int result = last_value;
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    result += value(index(state, moved.capturePtypeO(), moved.to())) *
      (moved.player() == BLACK ? 1 : -1);
  }
  result -= value(index(state, moved.oldPtypeO(), moved.from())) *
    (moved.player() == BLACK ? 1 : -1);
  result += value(index(state, moved.ptypeO(), moved.to())) *
    (moved.player() == BLACK ? 1 : -1);
  return result;
}

void gpsshogi::
PieceKingRelativeAbs::features(
  const osl::NumEffectState &state, double& value,
  osl::stl::vector<std::pair<int, double> >&diffs,
  int offset) const
{
  value = 0.0;
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() == KING || !piece.isOnBoard())
      continue;
    const int idx = index(state, piece);
    feature_count[idx] += (piece.owner() == BLACK ? 1 : -1);
  }
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.push_back(std::make_pair(offset + i, feature_count[i]));
      value += this->value(i) * feature_count[i];
    }
  }
}

int gpsshogi::PieceKingRelativeAbs::index(
  const NumEffectState &state,
  const PtypeO ptypeO,
  const Square position) const
{
  // [PTYPE][KING_X(1-9)][KING_Y(1-9)][X(1-9)][Y(1-9)]
  const Player player = getOwner(ptypeO);
  const Square king =
    state.kingSquare(is_attack ? alt(player) : player);
  const int king_x = (player == osl::BLACK ? king.x() : 10 - king.x());
  const int king_y = (player == osl::BLACK ? king.y() : 10 - king.y());
  const int piece_x =
    (player == osl::BLACK ? position.x() : 10 - position.x());
  const int piece_y =
    (player == osl::BLACK ? position.y() : 10 - position.y());

  return (getPtype(ptypeO) - PTYPE_PIECE_MIN) * 9 * 9 * 9 * 9 +
    (king_x - 1) * 9 * 9 * 9 +
    (king_y - 1) * 9 * 9 +
    (piece_x - 1) * 9 +
    (piece_y - 1);
}




int gpsshogi::SimpleAttackKing::distanceIndex(const osl::Square king,
					      const osl::Square pos) const
{
  return std::max(std::abs(king.x() - pos.x()),
		  std::abs(king.y() - pos.y())) - 1;
}

int gpsshogi::SimpleAttackKing::index(const osl::Ptype ptype,
				      int distanceIndex) const
{
  return (ptype - osl::PTYPE_PIECE_MIN) * 8  + distanceIndex;
}

int gpsshogi::SimpleAttackKing::index(const osl::Square king,
				      const osl::Square pos,
				      const osl::Ptype ptype) const
{
  return index(ptype, distanceIndex(king, pos));
}

int gpsshogi::SimpleAttackKing::pieceValue(
  const osl::NumEffectState &state,
  const osl::Piece piece) const
{
  const osl::Player p = is_attack ? osl::alt(piece.owner()) : piece.owner();
  osl::Square king = state.kingSquare(p);
  const int i = index(king, piece.square(), piece.ptype());
  const int value = this->value(i);
  if (piece.owner() == osl::BLACK)
    return value;
  else
    return -value;
}

int gpsshogi::SimpleAttackKing::eval(
  const osl::NumEffectState &state) const
{
  int result = 0;
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.ptype() == osl::KING || !piece.isOnBoard())
      continue;
    result += pieceValue(state, piece);
  }
  return result;
}

int gpsshogi::SimpleAttackKing::evalWithUpdate(
  const osl::NumEffectState &state, osl::Move moved,
  int last_value) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == osl::KING)
  {
    return eval(state);
  }
  const osl::Player p = is_attack ? osl::alt(moved.player()) : moved.player();
  const osl::Square king = state.kingSquare(p);

  if (moved.isDrop())
  {
    return last_value + value(index(king, moved.to(), moved.ptype())) * (moved.player() == osl::BLACK ? 1 : -1);
  }
  int result = last_value;
  if (osl::isPiece(moved.capturePtype()))
  {
    result += value(index(state.kingSquare(osl::alt(p)), moved.to(), moved.capturePtype())) * (moved.player() == osl::BLACK ? 1 : -1);
  }
  result -= value(index(king, moved.from(), moved.oldPtype())) * (moved.player() == osl::BLACK ? 1 : -1);
  result += value(index(king, moved.to(), moved.ptype())) * (moved.player() == osl::BLACK ? 1 : -1);

  return result;
}

void gpsshogi::SimpleAttackKing::features(
  const osl::NumEffectState &state, double& value,
  osl::stl::vector<std::pair<int, double> >&diffs,
  int offset) const
{
  value = 0.0;
  for (int i = osl::PTYPE_PIECE_MIN; i <= osl::PTYPE_MAX; ++i)
  {
    osl::CArray<int, 8> distance;
    distance.fill(0);
    osl::Ptype ptype = static_cast<osl::Ptype>(i);
    if (ptype == osl::KING)
      continue;
    for (int j = osl::Ptype_Table.getIndexMin(osl::unpromote(ptype));
	 j < Ptype_Table.getIndexLimit(osl::unpromote(ptype)); ++j)
    {
      const osl::Piece piece = state.pieceOf(j);
      if (piece.ptype() != ptype || !piece.isOnBoard())
	continue;
      const osl::Player p = is_attack ? osl::alt(piece.owner()) : piece.owner();
      distance[distanceIndex(state.kingSquare(p),
			     piece.square())] +=
	(piece.owner() == osl::BLACK) ? 1 : -1;
    }

    for (size_t j = 0; j < distance.size(); ++j)
    {
      if (distance[j] != 0)
      {
	const int k = index(ptype, j);
	diffs.push_back(std::make_pair(offset + k,
				       distance[j]));
	value += this->value(k) * distance[j];
      }
    }
  }
}

void gpsshogi::SimpleAttackKing::showSummary(std::ostream &os) const
{
  os << "SA done " << std::endl;
}

void gpsshogi::SimpleAttackKing::showAll(std::ostream &os) const
{
  for (int i = osl::PTYPE_PIECE_MIN; i <= osl::PTYPE_MAX; ++i) {
    osl::Ptype ptype = static_cast<osl::Ptype>(i);
    os << ptype << std::endl;
    for (int j = 0; j < 8; ++j) {
      os << std::setw(4) << value(index(ptype, j));
    }
    os << std::endl;
  }
}

const std::string gpsshogi::SimpleAttackKing::name() const
{
  return std::string("SimpleAttackKing") + ((is_attack) ? "1" : "0");
}



inline
int gpsshogi::PieceKingRelativeBase::
pieceValue(const osl::Square king,const osl::Piece piece) const
{
  const int i = index(piece.owner(), king, piece.square(), piece.ptype());
  const int value = this->value(i);
  if (piece.owner() == osl::BLACK)
    return value;
  else
    return -value;
}

inline
int gpsshogi::PieceKingRelativeBase::pieceValue(
  const osl::NumEffectState &state,
  const osl::Piece piece) const
{
  const osl::Player p = is_attack ? osl::alt(piece.owner()) : piece.owner();
  osl::Square king = state.kingSquare(p);
  return pieceValue(king, piece);
}

int gpsshogi::PieceKingRelativeBase::eval(
  const osl::NumEffectState &state) const
{
  int result = 0;
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const osl::Piece piece = state.pieceOf(i);
    if (piece.ptype() == osl::KING || !piece.isOnBoard())
      continue;
    Player pl = piece.owner();
    if (is_attack) 
      pl = alt(pl);
    result += pieceValue(kings[pl], piece);
  }
  return result;
}

int gpsshogi::PieceKingRelativeBase::evalWithUpdate(
  const osl::NumEffectState &state, osl::Move moved,
  int last_value) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == osl::KING)
  {
    return eval(state);
  }
  const osl::Player p = is_attack ? osl::alt(moved.player()) : moved.player();
  const osl::Square king = state.kingSquare(p);

  if (moved.isDrop())
  {
    return last_value + value(index(moved.player(), king, moved.to(), moved.ptype())) * (moved.player() == osl::BLACK ? 1 : -1);
  }
  int result = last_value;
  if (moved.capturePtype() != PTYPE_EMPTY)
  {
    result += value(index(osl::alt(moved.player()), state.kingSquare(osl::alt(p)), moved.to(), moved.capturePtype())) * (moved.player() == osl::BLACK ? 1 : -1);
  }
  result -= value(index(moved.player(), king, moved.from(), moved.oldPtype())) * (moved.player() == osl::BLACK ? 1 : -1);
  result += value(index(moved.player(), king, moved.to(), moved.ptype())) * (moved.player() == osl::BLACK ? 1 : -1);

  return result;
}

void gpsshogi::PieceKingRelativeBase::features(
  const osl::NumEffectState &state, double& value,
  osl::stl::vector<std::pair<int, double> >&diffs,
  int offset) const
{
  value = 0.0;
  IndexCache<40> feature_count;
  for (int i = 0; i < osl::Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.ptype() == KING || !piece.isOnBoard())
      continue;

    const osl::Player p = is_attack ? osl::alt(piece.owner()) : piece.owner();
    const int idx = index(piece.ptype(),
			  distanceIndex(piece.owner(),
					state.kingSquare(p),
					piece.square()));
    double v = (piece.owner()== BLACK) ? 1.0 : -1.0;
    feature_count.add(idx, v);
    value += this->value(idx) * v;
  }
  feature_count.uniqWrite(diffs, offset);
}

void gpsshogi::PieceKingRelativeBase::showSummary(std::ostream &os) const
{
  os << "PK (" << (is_attack ? "A" : "D") << ")done " << std::endl;
}

void gpsshogi::PieceKingRelativeBase::showAll(std::ostream &os) const
{
}



int gpsshogi::PieceKingRelative::index(const osl::Ptype ptype,
			     int distanceIndex) const
{
  return (ptype - osl::PTYPE_PIECE_MIN) * 17 * 9  + distanceIndex;
}

int gpsshogi::PieceKingRelative::distanceIndex(const osl::Player player,
				     const osl::Square king,
				     const osl::Square pos) const
{
  // x is absolute (0-8), y is relative (-8-8)
  const int x = std::abs(pos.x() - king.x());
  const int y = (king.y() - pos.y()) * (player == osl::BLACK ? 1 : -1) + 8;
  return (x * 17 + y);
}

void gpsshogi::PieceKingRelative::showAll(std::ostream &os) const
{
  for (int i = osl::PTYPE_PIECE_MIN; i <= osl::PTYPE_MAX; ++i) {
    osl::Ptype ptype = static_cast<osl::Ptype>(i);
    os << ptype << std::endl;
    for (int y = 0; y <= 16; ++y)
    {
      for (int x = 0; x <= 8; ++x)
      {
	const int distance = x * 17 + y;
	os << std::setw(5) << value(index(ptype, distance));
      }
      os << std::endl;
    }
  }
}
#endif



gpsshogi::King8EffectBase::EffectState
gpsshogi::King8EffectBase::effectState(
  const NumEffectState &state,
  const Player defense,
  const Direction dir) const
{
  const Square target =
    Board_Table.nextSquare(defense,
			     state.kingSquare(defense),
			     dir);
  if (!target.isOnBoard() ||
      !state.pieceAt(target).isEmpty())
    return NOT_EMPTY;

  const int attack_count = state.countEffect(alt(defense), target);
  const int defense_count = state.countEffect(defense, target);

  if (attack_count == 0)
    return NO_EFFECT;
  else if (defense_count == 1)
    return MORE_EFFECT_KING_ONLY;
  else if (attack_count >= defense_count)
    return MORE_EFFECT;
  else
    return LESS_EFFECT;
}

int gpsshogi::
King8EffectBase::eval(const osl::NumEffectState &state) const
{
  int result = 0;
  King8Info black_king = state.king8Info(BLACK);
  King8Info white_king = state.king8Info(WHITE);
  const Piece black_king_piece = state.kingPiece<BLACK>();
  const Piece white_king_piece = state.kingPiece<WHITE>();
  for (int i = UL; i <= 7; ++i)
  {
    const Direction dir = static_cast<Direction>(i);
    const EffectState black_effect_state = effectState(state, BLACK, dir);
    if (black_effect_state != NOT_EMPTY && isTarget(black_king))
    {
      result -= value(index(black_king_piece,
			    dir, black_effect_state));
    }
    const EffectState white_effect_state = effectState(state, WHITE, dir);
    if (white_effect_state != NOT_EMPTY && isTarget(white_king))
    {
      result += value(index(white_king_piece,
			    dir, white_effect_state));
    }
  }

  return result;
}

void gpsshogi::
King8EffectBase::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  King8Info black_king = state.king8Info(BLACK);
  King8Info white_king = state.king8Info(WHITE);
  const Piece black_king_piece = state.kingPiece<BLACK>();
  const Piece white_king_piece = state.kingPiece<WHITE>();
  const bool black_is_target = isTarget(black_king);
  const bool white_is_target = isTarget(white_king);
  if (!black_is_target && !white_is_target)
    return;
  for (int i = UL; i <= 7; ++i)
  {
    const Direction dir = static_cast<Direction>(i);
    const EffectState black_effect_state = effectState(state, BLACK, dir);
    const EffectState white_effect_state = effectState(state, WHITE, dir);
    const int black_index =
      (black_effect_state != NOT_EMPTY && black_is_target) ?
      index(black_king_piece, dir, black_effect_state) : -1;
    const int white_index =
      (white_effect_state != NOT_EMPTY && white_is_target) ?
      index(white_king_piece, dir, white_effect_state) : -1;
      
    if (black_index != white_index)
    {
      if (black_index != -1)
      {
	diffs.add(offset + black_index, -1);
      }
      if (white_index != -1)
      {
	diffs.add(offset + white_index, 1);
      }
    }
  }
}

int gpsshogi::
King8EffectEmptySquare::index(Piece,
				const Direction dir,
				EffectState state) const
{
  return dir * 4 + state;
}

void gpsshogi::King8EffectEmptySquare::showSummary(std::ostream &os) const
{
  os << name() << std::endl;
  for (int i = UL; i <= 7; ++i)
  {
    const Direction dir = static_cast<Direction>(i);
    os << dir << " ";
    for (int j = NO_EFFECT; j <= MORE_EFFECT_KING_ONLY; ++j)
    {
      os << value(index(Piece::EMPTY() /* dummy */,
			dir, static_cast<EffectState>(j))) << " ";
    }
    os << std::endl;
  }
}

int gpsshogi::
King8EffectEmptySquareY::index(Piece king,
				 const Direction dir,
				 EffectState state) const
{
  const int y = ((king.owner() == BLACK) ?
		 king.square().y() : 10 - king.square().y());
  return (dir * 4 + state) * 9 + y - 1;
}

void gpsshogi::King8EffectEmptySquareY::showAll(std::ostream &os) const
{
  os << name() << std::endl;
  for (int y = 1; y <= 9; ++y)
  {
    os << "Y: " << y << std::endl;
    for (int i = UL; i <= 7; ++i)
    {
      const Direction dir = static_cast<Direction>(i);
      os << dir << " ";
      for (int j = NO_EFFECT; j <= MORE_EFFECT_KING_ONLY; ++j)
      {
	os << value(index(Piece::makeKing(BLACK, Square(1, y)),
		      dir, static_cast<EffectState>(j))) << " ";
      }
      os << std::endl;
    }
  }
}

gpsshogi::King8EffectDefenseSquareY::EffectState
gpsshogi::King8EffectDefenseSquareY::effectState(
  const NumEffectState &state,
  const Player defense,
  const Direction dir) const
{
  const Square target =
    Board_Table.nextSquare(defense,
			     state.kingSquare(defense),
			     dir);
  if (!target.isOnBoard() ||
      !state.pieceAt(target).isOnBoardByOwner(defense))
    return NOT_EMPTY;

  const int attack_count = state.countEffect(alt(defense), target);
  const int defense_count = state.countEffect(defense, target);

  if (attack_count == 0)
    return NO_EFFECT;
  else if (defense_count == 1 && attack_count > defense_count)
    return MORE_EFFECT_KING_ONLY;
  else if (attack_count > defense_count)
    return MORE_EFFECT;
  else
    return LESS_EFFECT;
}

bool::gpsshogi::King8EffectEmptySquareDBlocked::isBlocked(
  const osl::checkmate::King8Info &info)
{
  return (info.liberty() & (DirectionTraits<DL>::mask |
			    DirectionTraits<D>::mask |
			    DirectionTraits<DR>::mask)) == 0;
}

bool::gpsshogi::King8EffectEmptySquareUBlocked::isBlocked(
  const osl::checkmate::King8Info &info)
{
  return (info.liberty() & (DirectionTraits<UL>::mask |
			    DirectionTraits<U>::mask |
			    DirectionTraits<UR>::mask)) == 0;
}

bool::gpsshogi::King8EffectEmptySquareLBlocked::isBlocked(
  const osl::checkmate::King8Info &info)
{
  return (info.liberty() & (DirectionTraits<UL>::mask |
			    DirectionTraits<L>::mask |
			    DirectionTraits<DL>::mask)) == 0;
}

bool::gpsshogi::King8EffectEmptySquareRBlocked::isBlocked(
  const osl::checkmate::King8Info &info)
{
  return (info.liberty() & (DirectionTraits<UR>::mask |
			    DirectionTraits<R>::mask |
			    DirectionTraits<DR>::mask)) == 0;
}

gpsshogi::King8EffectDefenseSquare::EffectState
gpsshogi::King8EffectDefenseSquare::effectState(
  const NumEffectState &state,
  const Player defense,
  const Direction dir) const
{
  const Square target =
    Board_Table.nextSquare(defense,
			     state.kingSquare(defense),
			     dir);
  if (!target.isOnBoard() ||
      !state.pieceAt(target).isOnBoardByOwner(defense))
    return NOT_EMPTY;

  const int attack_count = state.countEffect(alt(defense), target);
  const int defense_count = state.countEffect(defense, target);

  if (attack_count == 0)
    return NO_EFFECT;
  else if (defense_count == 1 && attack_count > defense_count)
    return MORE_EFFECT_KING_ONLY;
  else if (attack_count > defense_count)
    return MORE_EFFECT;
  else
    return LESS_EFFECT;
}


void gpsshogi::King25EffectCommon::countEffectAndPieces(
  bool is_attack,
  const osl::NumEffectState &state,
  const osl::Player player,
  int &effect,
  int &piece)
{
  const Square king = state.kingSquare(is_attack ? alt(player) : player);
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);

  PieceMask mask;
  int count = 0;
  for (int y = min_y; y <= max_y; ++y)
  {
    for (int x = min_x; x <= max_x; ++x)
    {
      const Square target(x, y);
      count += state.countEffect(player, target);
      mask = mask | state.effectSetAt(target);
    }
  }
  effect = std::min(127, count);
  mask = mask & state.piecesOnBoard(player);
  piece = std::min(16,  mask.countBit());
}

int gpsshogi::King25EffectCommon::index(int effect, int piece_count)
{
  return effect + 128 * piece_count;
}

void gpsshogi::King25EffectCommon::featuresNonUniq(
  bool is_attack,
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset)
{
  int black_effect, black_piece, white_effect, white_piece;
  countEffectAndPieces(is_attack, state, osl::BLACK, black_effect, black_piece);
  countEffectAndPieces(is_attack, state, osl::WHITE, white_effect, white_piece);
  if (black_effect != white_effect || black_piece != white_piece)
  {
    const int black_index = index(black_effect, black_piece);
    const int white_index = index(white_effect, white_piece);

    if (black_index < white_index) 
    {
      diffs.add(offset + black_index,1);
      diffs.add(offset + white_index,-1);
    }
    else
    {
      diffs.add(offset + white_index,-1);
      diffs.add(offset + black_index,1);
    }
  }
}

int gpsshogi::King25EffectAttack::eval(const osl::NumEffectState &state) const
{
  int black_effect, black_piece, white_effect, white_piece;
  countEffectAndPieces(true, state, osl::BLACK, black_effect, black_piece);
  countEffectAndPieces(true, state, osl::WHITE, white_effect, white_piece);
  return value(index(black_effect, black_piece)) - value(index(white_effect, white_piece));
}

void gpsshogi::King25EffectAttack::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  King25EffectCommon::featuresNonUniq(true, state, diffs, offset);
}

void gpsshogi::King25EffectAttack::showAll(std::ostream &os) const
{
  os << "King25Effect A" << std::endl;
  for (int piece_count = 0; piece_count <= 16; ++piece_count)
  {
    os << piece_count << ": ";
    for (int effect_count = 0; effect_count < 128; ++effect_count)
    {
      os << value(index(effect_count, piece_count)) << " ";
    }
    os << std::endl;
  }
}

osl::MultiInt gpsshogi::King25EffectDefense::eval(const osl::NumEffectState &state, const MultiWeights& weights,
						  CArray<MultiInt,2>& /*saved_state*/) const
{
  int black_effect, black_piece, white_effect, white_piece;
  countEffectAndPieces(false, state, osl::BLACK, black_effect, black_piece);
  countEffectAndPieces(false, state, osl::WHITE, white_effect, white_piece);
  return weights.value(index(black_effect, black_piece)) - weights.value(index(white_effect, white_piece));
}

void gpsshogi::King25EffectDefense::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  return King25EffectCommon::featuresNonUniq(false, state, diffs, offset);
}

void gpsshogi::King25EffectDefense::showAll(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << "King25Effect D" << std::endl;
    for (int piece_count = 0; piece_count <= 16; ++piece_count)
    {
      os << piece_count << ": ";
      for (int effect_count = 0; effect_count < 128; ++effect_count)
      {
	os << weights.value(index(effect_count, piece_count))[s] << " ";
      }
      os << std::endl;
    }
  }
}


void gpsshogi::King25EffectYCommon::countEffectAndPieces(
  bool is_attack,
  const osl::NumEffectState &state,
  const osl::Player player,
  int &effect,
  int &piece,
  int &king_y)
{
  const Player king_player = is_attack ? alt(player) : player;
  const Square king = state.kingSquare(king_player);
  if (king_player == BLACK)
    king_y = king.y();
  else
    king_y = 10 - king.y();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);

  PieceMask mask;
  int count = 0;
  for (int y = min_y; y <= max_y; ++y)
  {
    for (int x = min_x; x <= max_x; ++x)
    {
      const Square target(x, y);
      count += state.countEffect(player, target);
      mask = mask | state.effectSetAt(target);
    }
  }
  effect = std::min(255, count);
  mask = mask & state.piecesOnBoard(player);
  piece = std::min(16,  mask.countBit());
}

int gpsshogi::King25EffectYCommon::index(int king_y, int effect,
					 int piece_count)
{
  return effect + 128 * piece_count + 128 * 17 * (king_y - 1);
}

void gpsshogi::King25EffectYCommon::featuresNonUniq(
  bool is_attack,
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset)
{
  int black_effect, black_piece, white_effect, white_piece,
    king_y_to_black, king_y_to_white;
  countEffectAndPieces(is_attack, state, osl::BLACK, black_effect, black_piece,
		       king_y_to_black);
  countEffectAndPieces(is_attack, state, osl::WHITE, white_effect, white_piece,
		       king_y_to_white);
  if (black_effect != white_effect || black_piece != white_piece ||
      king_y_to_black != king_y_to_white)
  {
    const int black_index = index(king_y_to_black, black_effect, black_piece);
    const int white_index = index(king_y_to_white, white_effect, white_piece);
    if (black_index < white_index) 
    {  
      diffs.add(offset + black_index, 1);
      diffs.add(offset + white_index, -1);
    }
    else
    {
      diffs.add(offset + white_index, -1);
      diffs.add(offset + black_index, 1);
    }
  }
}

int gpsshogi::King25EffectYAttack::eval(const osl::NumEffectState &state) const
{
  int black_effect, black_piece, white_effect, white_piece;
  int king_y_to_black, king_y_to_white;
  countEffectAndPieces(true, state, osl::BLACK, black_effect, black_piece,
		       king_y_to_black);
  countEffectAndPieces(true, state, osl::WHITE, white_effect, white_piece,
		       king_y_to_white);
  return value(index(king_y_to_black, black_effect, black_piece)) -
    value(index(king_y_to_white, white_effect, white_piece));
}

void gpsshogi::King25EffectYAttack::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  return King25EffectYCommon::featuresNonUniq(true, state, diffs, offset);
}

void gpsshogi::King25EffectYAttack::showAll(std::ostream &os) const
{
  os << "King25EffectY A" << std::endl;
  for (int y = 1; y <= 9; y++)
  {
    os << "Y: " << y << std::endl;
    for (int piece_count = 0; piece_count <= 16; ++piece_count)
    {
      os << piece_count << ": ";
      for (int effect_count = 0; effect_count < 128; ++effect_count)
      {
	os << value(index(y, effect_count, piece_count)) << " ";
      }
      os << std::endl;
    }
  }
}

osl::MultiInt gpsshogi::King25EffectYDefense::eval(
  const osl::NumEffectState &state, const MultiWeights& weights,
  CArray<MultiInt,2>& /*saved_state*/) const
{
  int black_effect, black_piece, white_effect, white_piece;
  int king_y_to_black, king_y_to_white;
  countEffectAndPieces(false, state, osl::BLACK, black_effect, black_piece,
		       king_y_to_black);
  countEffectAndPieces(false, state, osl::WHITE, white_effect, white_piece,
		       king_y_to_white);
  return weights.value(index(king_y_to_black, black_effect, black_piece)) -
    weights.value(index(king_y_to_white, white_effect, white_piece));
}

void gpsshogi::King25EffectYDefense::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  return King25EffectYCommon::featuresNonUniq(false, state, diffs, offset);
}

void gpsshogi::King25EffectYDefense::showAll(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << "King25EffectY D" << std::endl;
    for (int y = 1; y <= 9; y++)
    {
      os << "Y: " << y << std::endl;
      for (int piece_count = 0; piece_count <= 16; ++piece_count)
      {
	os << piece_count << ": ";
	for (int effect_count = 0; effect_count < 128; ++effect_count)
	{
	  os << weights.value(index(y, effect_count, piece_count))[s] << " ";
	}
	os << std::endl;
      }
    }
  }
}



void gpsshogi::King25EffectXY::countEffectAndPieces(
  const osl::NumEffectState &state,
  const osl::Player player,
  int &effect,
  int &piece,
  int &king_x,
  int &king_y) const
{
  const Player king_player = is_attack ? alt(player) : player;
  const Square king = state.kingSquare(king_player);
  if (king_player == BLACK)
    king_y = king.y();
  else
    king_y = 10 - king.y();
  if (king.x() > 5)
    king_x = 10 - king.x();
  else
    king_x = king.x();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);

  PieceMask mask;
  int count = 0;
  for (int y = min_y; y <= max_y; ++y)
  {
    for (int x = min_x; x <= max_x; ++x)
    {
      const Square target(x, y);
      count += state.countEffect(player, target);
      mask = mask | state.effectSetAt(target);
    }
  }
  effect = std::min(255, count);
  mask = mask & state.piecesOnBoard(player);
  piece = std::min(16,  mask.countBit());
}

int gpsshogi::King25EffectXY::indexY(int king_y, int effect,
				     int piece_count) const
{
  return DIM_X + effect + 128 * piece_count + 128 * 17 * (king_y - 1);
}


int gpsshogi::King25EffectXY::indexX(int king_x, int effect,
				     int piece_count) const
{
  return effect + 128 * piece_count + 128 * 17 * (king_x - 1);
}

int gpsshogi::King25EffectXY::eval(const osl::NumEffectState &state) const
{
  int black_effect, black_piece, white_effect, white_piece;
  int king_x_to_black, king_x_to_white;
  int king_y_to_black, king_y_to_white;
  countEffectAndPieces(state, osl::BLACK, black_effect, black_piece,
		       king_x_to_black, king_y_to_black);
  countEffectAndPieces(state, osl::WHITE, white_effect, white_piece,
		       king_x_to_white, king_y_to_white);
  return value(indexY(king_y_to_black, black_effect, black_piece)) -
    value(indexY(king_y_to_white, white_effect, white_piece)) +
    value(indexX(king_x_to_black, black_effect, black_piece)) -
    value(indexX(king_x_to_white, white_effect, white_piece));
}

void gpsshogi::King25EffectXY::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  int black_effect, black_piece, white_effect, white_piece,
    king_x_to_black, king_x_to_white, king_y_to_black, king_y_to_white;
  countEffectAndPieces(state, osl::BLACK, black_effect, black_piece,
		       king_x_to_black, king_y_to_black);
  countEffectAndPieces(state, osl::WHITE, white_effect, white_piece,
		       king_x_to_white, king_y_to_white);
  if (black_effect != white_effect || black_piece != white_piece ||
      king_y_to_black != king_y_to_white)
  {
    const int black_index = indexY(king_y_to_black, black_effect, black_piece);
    diffs.add(offset + black_index, 1);

    const int white_index = indexY(king_y_to_white, white_effect, white_piece);
    diffs.add(offset + white_index, -1);
  }
  if (black_effect != white_effect || black_piece != white_piece ||
      king_x_to_black != king_x_to_white)
  {
    const int black_index = indexX(king_x_to_black, black_effect, black_piece);
    diffs.add(offset + black_index, 1);

    const int white_index = indexX(king_x_to_white, white_effect, white_piece);
    diffs.add(offset + white_index, -1);
  }
}

void gpsshogi::King25EffectXY::showAll(std::ostream &os) const
{
  os << name() << std::endl;
  for (int x = 1; x <= 5; x++)
  {
    os << "X: " << x << std::endl;
    for (int piece_count = 0; piece_count <= 16; ++piece_count)
    {
      os << piece_count << ": ";
      for (int effect_count = 0; effect_count < 128; ++effect_count)
      {
	os << value(indexX(x, effect_count, piece_count)) << " ";
      }
      os << std::endl;
    }
  }
  for (int y = 1; y <= 9; y++)
  {
    os << "Y: " << y << std::endl;
    for (int piece_count = 0; piece_count <= 16; ++piece_count)
    {
      os << piece_count << ": ";
      for (int effect_count = 0; effect_count < 128; ++effect_count)
      {
	os << value(indexY(y, effect_count, piece_count)) << " ";
      }
      os << std::endl;
    }
  }
}


void gpsshogi::King25Effect2::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  int black_effect, black_piece, white_effect, white_piece, dummy;
  countEffectAndPieces(state, osl::BLACK, black_effect, black_piece, dummy);
  countEffectAndPieces(state, osl::WHITE, white_effect, white_piece, dummy);
  const int black_stand_count =
    state.countPiecesOnStand<ROOK>(BLACK) +
    state.countPiecesOnStand<BISHOP>(BLACK) +
    state.countPiecesOnStand<GOLD>(BLACK) +
    state.countPiecesOnStand<SILVER>(BLACK);
  const int white_stand_count =
    state.countPiecesOnStand<ROOK>(WHITE) +
    state.countPiecesOnStand<BISHOP>(WHITE) +
    state.countPiecesOnStand<GOLD>(WHITE) +
    state.countPiecesOnStand<SILVER>(WHITE);
  features.add(index(black_effect, black_piece, black_stand_count), 1);
  features.add(index(white_effect, white_piece, white_stand_count), -1);
}

void gpsshogi::King25EffectY2::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  int black_effect, black_piece, white_effect, white_piece,
    black_king_y, white_king_y;
  King25Effect2::countEffectAndPieces(state, osl::BLACK, black_effect, black_piece, black_king_y);
  King25Effect2::countEffectAndPieces(state, osl::WHITE, white_effect, white_piece, white_king_y);
  const int black_stand_count =
    state.countPiecesOnStand<ROOK>(BLACK) +
    state.countPiecesOnStand<BISHOP>(BLACK) +
    state.countPiecesOnStand<GOLD>(BLACK) +
    state.countPiecesOnStand<SILVER>(BLACK);
  const int white_stand_count =
    state.countPiecesOnStand<ROOK>(WHITE) +
    state.countPiecesOnStand<BISHOP>(WHITE) +
    state.countPiecesOnStand<GOLD>(WHITE) +
    state.countPiecesOnStand<SILVER>(WHITE);
  features.add(index(black_king_y, black_effect, black_piece, black_stand_count), 1);
  features.add(index(white_king_y, white_effect, white_piece, white_stand_count), -1);
}


void gpsshogi::King25EffectSupported::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  int black_effect, black_supported, white_effect, white_supported, dummy;
  countEffectAndPieces(state, osl::BLACK, black_effect,
		       black_supported, dummy);
  countEffectAndPieces(state, osl::WHITE, white_effect,
		       white_supported, dummy);
  features.add(index(black_effect, black_supported), 1);
  features.add(index(white_effect, white_supported), -1);
}

void gpsshogi::King25EffectSupportedY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features) const
{
  int black_effect, black_supported, white_effect, white_supported, black_y, white_y;
  King25EffectSupported::countEffectAndPieces(state, osl::BLACK, black_effect,
					      black_supported, black_y);
  King25EffectSupported::countEffectAndPieces(state, osl::WHITE, white_effect,
					      white_supported, white_y);
  features.add(index(black_effect, black_supported, black_y), 1);
  features.add(index(white_effect, white_supported, white_y), -1);
}


namespace gpsshogi
{
#ifdef USE_OLD_FEATURE
  template class PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
					   9 * 9 * 17), true>;
  template class PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
					   9 * 9 * 17), false>;
  template class PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
					   5 * 9 * 17), true>;
  template class PieceKingRelativeAbsBase<((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
					   5 * 9 * 17), false>;
#endif
}



// x: [-2, 2], y: [-2, 2]
int gpsshogi::King25Empty::index(int rel_x, int rel_y) const
{
  return (rel_y + 2) * 3 + std::abs(rel_x);
}

template <osl::Player P>
void gpsshogi::King25Empty::stateOne(const NumEffectState &state,
				     CArray<int, DIM> &result) const
{
  const Square king = state.kingSquare<P>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (target.isOnBoard() && isTarget(state, target, P))
      {
	if (P == BLACK)
	  ++result[index(x - king.x(), y - king.y())];
	else
	  --result[index(king.x() - x, king.y() - y)];
      }
    }
  }
}

int gpsshogi::King25Empty::eval(const NumEffectState &state) const
{
  int result = 0;
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  stateOne<BLACK>(state, feature_count);
  stateOne<WHITE>(state, feature_count);
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    result += this->value(i) * feature_count[i];
  }
  return result;
}

void gpsshogi::King25Empty::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  stateOne<BLACK>(state, feature_count);
  stateOne<WHITE>(state, feature_count);
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

void gpsshogi::King25Empty::showSummary(std::ostream &os) const
{
  os << name() << std::endl;
  for (int y = -2; y <= 2; ++y)
  {
    for (int x = 0; x <= 2; ++x)
    {
      os << this->value(index(x, y)) << " ";
    }
    os << std::endl;
  }
}

bool gpsshogi::King25Empty::isTarget(const NumEffectState &state,
				     const Square pos,
				     const Player) const
{
  return state.pieceAt(pos).isEmpty();
}

bool gpsshogi::King25EmptyNoEffect::isTarget(const NumEffectState &state,
					     const Square pos,
					     const Player defense) const
{
  if (!state.pieceAt(pos).isEmpty())
    return false;

  int defense_effect = state.countEffect(defense, pos);
  const Square king = state.kingSquare(defense);
  if (std::abs(king.x() - pos.x()) <= 1 &&
      std::abs(king.y() - pos.y()) <= 1)
  {
    return defense_effect == 1;
  }
  else
    return defense_effect == 0;
}



#ifdef USE_OLD_FEATURE
int gpsshogi::King25EmptyAbs::index(Square king,
				    Square target,
				    Player player) const
{
  int x, target_x;
  if ((player == BLACK && king.x() >= 6) ||
      (player == WHITE && king.x() >= 5))
  {
    x = 10 - king.x();
    target_x = 10 - target.x();
  }
  else
  {
    x = king.x();
    target_x = target.x();
  }
  const int y = (player == BLACK ? king.y() : 10 - king.y());
  const int target_y = (player == BLACK ? target.y() : 10 - target.y());

  return target_y - y + 2 + (target_x - x + 2 ) * 5 + (y - 1) * 5 * 5
    + (x - 1) * 5 * 5 * 9;
}

template <osl::Player Defense>
int gpsshogi::King25EmptyAbs::evalOne(
  const osl::NumEffectState &state) const
{
  int result = 0;
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (target.isOnBoard() && state.pieceAt(target).isEmpty())
      {
	result += value(index(king, target, Defense));
      }
    }
  }
  if (Defense == BLACK)
    return result;
  else
    return -result;
}

int gpsshogi::King25EmptyAbs::eval(
  const osl::NumEffectState &state) const
{
  return evalOne<BLACK>(state) + evalOne<WHITE>(state);
}

int gpsshogi::King25EmptyAbs::evalWithUpdate(
  const osl::NumEffectState &state, osl::Move moved,
  int last_value) const
{
  if (moved.isPass())
    return last_value;
  if (moved.ptype() == osl::KING)
  {
    return eval(state);
  }
  const osl::Square self_king = state.kingSquare(moved.player());
  const osl::Square opp_king = state.kingSquare(alt(moved.player()));
  int result = last_value;

  if (!moved.isDrop())
  {
    const Square from = moved.from();
    if (std::abs(self_king.x() - moved.from().x()) <= 2 &&
	std::abs(self_king.y() - moved.from().y()) <= 2)
    {
      result += value(index(self_king, moved.from(), moved.player())) *
	(moved.player() == BLACK ? 1 : -1);
    }

    if (std::abs(opp_king.x() - moved.from().x()) <= 2 &&
	std::abs(opp_king.y() - moved.from().y()) <= 2)
    {
      result -= value(index(opp_king, moved.from(), alt(moved.player()))) *
	(moved.player() == BLACK ? 1 : -1);
    }
  }

  Ptype captured = moved.capturePtype();
  if (captured == PTYPE_EMPTY)
  {
    const Square to = moved.to();
    if (std::abs(self_king.x() - moved.to().x()) <= 2 &&
	std::abs(self_king.y() - moved.to().y()) <= 2)
    {
      result -= value(index(self_king, moved.to(), moved.player())) *
	(moved.player() == BLACK ? 1 : -1);
    }
    if (std::abs(opp_king.x() - moved.to().x()) <= 2 &&
	std::abs(opp_king.y() - moved.to().y()) <= 2)
    {
      result += value(index(opp_king, moved.to(), alt(moved.player()))) *
	(moved.player() == BLACK ? 1 : -1);
    }
  }
  return result;
}

template <osl::Player Defense>
void gpsshogi::King25EmptyAbs::oneFeature(
  const osl::NumEffectState &state,
  CArray<int, DIM> &features) const
{
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (target.isOnBoard() && state.pieceAt(target).isEmpty())
      {
	if (Defense == BLACK)
	  ++features[index(king, target, Defense)];
	else
	  --features[index(king, target, Defense)];
      }
    }
  }
}

void gpsshogi::King25EmptyAbs::features(
  const osl::NumEffectState &state, double& value,
  osl::stl::vector<std::pair<int, double> >&diffs,
  int offset) const
{
  value = 0.0;
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  oneFeature<BLACK>(state, feature_count);
  oneFeature<WHITE>(state, feature_count);
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.push_back(std::make_pair(offset+i, feature_count[i]));
      value += feature_count[i] * this->value(i);
    }
  }
}
#endif


gpsshogi::King25EffectEach::EffectState gpsshogi::
King25EffectEach::effectState(const NumEffectState &state,
			      Square target, Player defense)
{
  if (!state.hasEffectAt(alt(defense), target))
  {
    return static_cast<EffectState>(std::min(2, state.countEffect(defense, target)));
  }
  const int diff = state.countEffect(defense, target) -
    state.countEffect(alt(defense), target);
  return static_cast<EffectState>(std::max(-2, std::min(2, diff)) + ATTACK_DIFF_0);
}

int gpsshogi::King25EffectEach::index(
  const NumEffectState &state, Square target, Player defense)
{
  const Square king = state.kingSquare(defense);
  const Piece piece = state.pieceAt(target);
  // [0, 2]
  const int rel_x = std::abs(king.x() - target.x());
  // [-2, +2]
  const int rel_y = (target.y() - king.y()) * (defense == BLACK ? 1 : -1);
  int piece_owner;
  if (piece.isEmpty())
    piece_owner = 0;
  else if (piece.owner() == defense)
    piece_owner = 1;
  else
    piece_owner = 2;

  int val = rel_y + 2 +
    rel_x * 5 + effectState(state, target, defense) * 5 * 3 +
    piece_owner * 5 * 3 * 8;
  return val;
}

template <osl::Player Defense, typename Integer, class W>
Integer gpsshogi::King25EffectEach::evalOne(
  const osl::NumEffectState &state, Integer initial, const W& weights)
{
  Integer result = initial;
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      result += weights.value(index(state, target, Defense));
    }
  }
  if (Defense == BLACK)
    return result;
  else
    return -result;
}

osl::MultiInt gpsshogi::King25EffectEach::eval(const NumEffectState& state, const MultiWeights& w,
      CArray<MultiInt,2>& saved_state) const
{
  saved_state[BLACK] = evalOne<BLACK, MultiInt, MultiWeights>(state, MultiInt(), w);
  saved_state[WHITE] = evalOne<WHITE, MultiInt, MultiWeights>(state, MultiInt(), w);
  return saved_state[BLACK] + saved_state[WHITE];
}

osl::MultiInt gpsshogi::
King25EffectEach::evalWithUpdate(
  const NumEffectState& state, Move move, MultiInt last_value, const MultiWeights& weights,
      CArray<MultiInt,2>& saved_state) const
{
  if (move.isPass())
    return last_value;
  BoardMask mask = state.changedEffects();
  mask.set(move.from());
  mask.set(move.to());
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<BLACK>())))
    saved_state[BLACK] = evalOne<BLACK, MultiInt, MultiWeights>(state, MultiInt(), weights);
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<WHITE>())))
    saved_state[WHITE] = evalOne<WHITE, MultiInt, MultiWeights>(state, MultiInt(), weights);
  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Player Defense>
void gpsshogi::King25EffectEach::featureOne(
  const osl::NumEffectState &state,
  CArray<int, DIM> &feature_count) const
{
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (Defense == BLACK)
	++feature_count[index(state, target, Defense)];
      else
	--feature_count[index(state, target, Defense)];
    }
  }
}

void gpsshogi::King25EffectEach::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  featureOne<BLACK>(state, feature_count);
  featureOne<WHITE>(state, feature_count);
  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}


template <osl::Player Defense>
void gpsshogi::King25EffectEachYFeatures::featureOne(
  const osl::NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features)
{
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (Defense == BLACK)
	features.add(index(state, target, Defense), 1);
      else
	features.add(index(state, target, Defense), -1);
    }
  }
}

void gpsshogi::King25EffectEachYFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  featureOne<BLACK>(state, diffs);
  featureOne<WHITE>(state, diffs);
}

void gpsshogi::
King25EffectEachYFeatures::showAllOne(const Weights& weights,
				      int n,
				      std::ostream &os) const
{
  CArray<std::string, 3> owners = {{ "empty", "defense", "offense" }};
  os << name() << " " << n << std::endl;
  for (int y = 0; y < 9; ++y)
  {
    os << "King Y: " << y + 1 << std::endl;
    for (int y_diff = 0; y_diff < 5; ++y_diff)
    {
      os << "Y: " << y_diff - 2  << std::endl;
      for (int owner = 0; owner < 3; ++owner)
      {
	os << owners[owner] << std::endl;
	for (int state = King25EffectEach::NO_ATTACK_DEFENSE_0;
	     state < King25EffectEach::STATE_MAX; ++state)
	{
	  os << " " << state << ": ";
	  for (int x_diff = 0; x_diff < 3; ++x_diff)
	  {
	    os << " "
	       << weights.value((y_diff + x_diff * 5 +
				 state * 5 * 3 + owner * 5 * 3 * 8) * 9 +
				y + n * dimension());
	  }
	  os << std::endl;
	}
      }
    }
  }
}


osl::MultiInt gpsshogi::King25EffectEachYStages::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  BoardMask mask = state.changedEffects();
  mask.set(moved.from());
  mask.set(moved.to());
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<BLACK>())))
    saved_state[BLACK] = evalBlack(state);
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<WHITE>())))
    saved_state[WHITE] = evalWhite(state);

  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Player Defense>
void gpsshogi::King25EffectEachXFeatures::featureOne(
  const osl::NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features)
{
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (Defense == BLACK)
	features.add(index(state, target, Defense), 1);
      else
	features.add(index(state, target, Defense), -1);
    }
  }
}

void gpsshogi::King25EffectEachXFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  featureOne<BLACK>(state, diffs);
  featureOne<WHITE>(state, diffs);
}

osl::MultiInt gpsshogi::King25EffectEachXStages::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  BoardMask mask = state.changedEffects();
  mask.set(moved.from());
  mask.set(moved.to());
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<BLACK>())))
    saved_state[BLACK] = evalBlack(state);
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<WHITE>())))
    saved_state[WHITE] = evalWhite(state);
  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Player Defense>
void gpsshogi::King25EffectEachXYFeatures::featureOne(
  const osl::NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &features)
{
  const Square king = state.kingSquare<Defense>();
  const int min_x = std::max(1, king.x() - 2);
  const int max_x = std::min(9, king.x() + 2);
  const int min_y = std::max(1, king.y() - 2);
  const int max_y = std::min(9, king.y() + 2);
  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      Square target(x, y);
      if (Defense == BLACK)
	features.add(index(state, target, Defense), 1);
      else
	features.add(index(state, target, Defense), -1);
    }
  }
}

void gpsshogi::King25EffectEachXYFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  featureOne<BLACK>(state, diffs);
  featureOne<WHITE>(state, diffs);
}

void gpsshogi::King25EffectEachXYStages::
featureOneBlack(const NumEffectState &state, index_list_t& values) const
{
  King25EffectEachXYFeatures::featureOne<BLACK>(state, values);
}
void gpsshogi::King25EffectEachXYStages::
featureOneWhite(const NumEffectState &state, index_list_t& values) const
{
  King25EffectEachXYFeatures::featureOne<WHITE>(state, values);
}

osl::MultiInt gpsshogi::King25EffectEachXYStages::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  BoardMask mask = state.changedEffects();
  mask.set(moved.from());
  mask.set(moved.to());
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<BLACK>())))
    saved_state[BLACK] = evalBlack(state);
  if (mask.anyInRange(Board_Mask_Table5x5.mask(state.kingSquare<WHITE>())))
    saved_state[WHITE] = evalWhite(state);

  return saved_state[BLACK] + saved_state[WHITE];
}


int gpsshogi::KingXBlocked::index(const NumEffectState &/*state*/,
				  Piece king, int diff) const
{
  const int king_x = king.square().x();
  if (king.owner() == BLACK)
  {
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 6)
      x_diff = -x_diff;
    return target_x - 1 + (x_diff == 1 ? 0 : 5);
  }
  else
  {
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 5)
      x_diff = -x_diff;
    return target_x - 1 + (x_diff == 1 ? 0 : 5);
  }
}

template <osl::Player P>
bool gpsshogi::KingXBlocked::isBlocked(const NumEffectState &state,
				       int diff)
{
#if 1
  const King8Info info(state.Iking8Info(P));
  if ((diff == 1) ^ (P == BLACK))
    return (info.liberty() & (DirectionTraits<UR>::mask 
			      | DirectionTraits<R>::mask 
			      | DirectionTraits<DR>::mask)) == 0;
  assert((diff == 1 && P == BLACK) || (diff == -1 && P == WHITE));
  return (info.liberty() & (DirectionTraits<UL>::mask 
			    | DirectionTraits<L>::mask 
			    | DirectionTraits<DL>::mask)) == 0;
#else
  const Square pos = state.kingSquare<P>();
  const int target_x = pos.x() + diff;
  for (int y = pos.y() - 1; y <= pos.y() + 1; ++y)
  {
    Square p(target_x, y);
    if (p.isOnBoard() && !state.pieceAt(p).isOnBoardByOwner<P>() &&
	!state.hasEffectAt<alt(P)>(p))

    {
      return false;
    }
  }
  return true;
#endif
}

osl::MultiInt gpsshogi::KingXBlocked::eval(const NumEffectState &state, const MultiWeights& weights,
					   CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt val;
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  if (isBlocked<BLACK>(state, 1))
    val += weights.value(index(state, black_king, 1));
  if (isBlocked<BLACK>(state, -1))
    val += weights.value(index(state, black_king, -1));
  if (isBlocked<WHITE>(state, 1))
    val -= weights.value(index(state, white_king, 1));
  if (isBlocked<WHITE>(state, -1))
    val -= weights.value(index(state, white_king, -1));
  return val;
}

void gpsshogi::KingXBlocked::featuresNonUniq(
  const NumEffectState &state,
  index_list_t&diffs,
  int offset) const
{
  CArray<int, 90> feature_count;
  feature_count.fill(0);
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  if (isBlocked<BLACK>(state, 1))
    ++feature_count[index(state, black_king, 1)];
  if (isBlocked<BLACK>(state, -1))
    ++feature_count[index(state, black_king, -1)];
  if (isBlocked<WHITE>(state, 1))
    --feature_count[index(state, white_king, 1)];
  if (isBlocked<WHITE>(state, -1))
    --feature_count[index(state, white_king, -1)];

  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

int gpsshogi::KingXBlockedY::index(const NumEffectState &/*state*/,
				   Piece king, int diff) const
{
  const int king_x = king.square().x();
  if (king.owner() == BLACK)
  {
    const int king_y = king.square().y();
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 6)
      x_diff = -x_diff;
    return (target_x - 1 + (x_diff == 1 ? 0 : 5)) * 9 + king_y - 1;
  }
  else
  {
    const int king_y = 10 - king.square().y();
    const int target_x = (king_x > 5) ? 10 - king_x : king_x;
    int x_diff = diff;
    if (king_x >= 5)
      x_diff = -x_diff;
    return (target_x - 1 + (x_diff == 1 ? 0 : 5)) * 9 + king_y - 1;
  }
}

void gpsshogi::KingXBothBlocked::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  King8Info black = state.king8Info(BLACK);
  if ((black.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask |
			  DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    int x = state.kingSquare<BLACK>().x();
    if (x > 5)
    {
      x = 10 - x;
    }
    diffs.add(x - 1, 1);
  }

  King8Info white = state.king8Info(WHITE);
  if ((white.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask |
			  DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    int x = state.kingSquare<WHITE>().x();
    if (x > 5)
    {
      x = 10 - x;
    }
    diffs.add(x - 1, -1);
  }
}

void gpsshogi::KingXBothBlockedY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  King8Info black = state.king8Info(BLACK);
  if ((black.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask |
			  DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    const Square pos = state.kingSquare<BLACK>();
    int x = pos.x();
    const int y = pos.y();
    if (x > 5)
    {
      x = 10 - x;
    }
    diffs.add((y - 1) * 5 + x - 1, 1);
  }

  King8Info white = state.king8Info(WHITE);
  if ((white.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask |
			  DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    const Square pos = state.kingSquare<WHITE>();
    int x = pos.x();
    const int y = 10 - pos.y();
    if (x > 5)
    {
      x = 10 - x;
    }
    diffs.add((y - 1) * 5 + x - 1, -1);
  }
}


void gpsshogi::KingXBlocked3::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  King8Info black = state.king8Info(BLACK);
  if ((black.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask)) == 0)
  {
    diffs.add(
      index<BLACK>(state.kingSquare<BLACK>(),
		   true,
		   (black.liberty() & DirectionTraits<U>::mask) == 0,
		   (black.liberty() & DirectionTraits<UR>::mask) == 0,
		   (black.liberty() & DirectionTraits<R>::mask) == 0),
      1);
  }
  if ((black.liberty() & (DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    diffs.add(
      index<BLACK>(state.kingSquare<BLACK>(),
		   false,
		   (black.liberty() & DirectionTraits<U>::mask) == 0,
		   (black.liberty() & DirectionTraits<UL>::mask) == 0,
		   (black.liberty() & DirectionTraits<L>::mask) == 0),
      1);
  }
  King8Info white = state.king8Info(WHITE);
  if ((white.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask)) == 0)
  {
    diffs.add(
      index<WHITE>(state.kingSquare<WHITE>(),
		   true,
		   (white.liberty() & DirectionTraits<U>::mask) == 0,
		   (white.liberty() & DirectionTraits<UR>::mask) == 0,
		   (white.liberty() & DirectionTraits<R>::mask) == 0),
      -1);
  }
  if ((white.liberty() & (DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    diffs.add(
      index<WHITE>(state.kingSquare<WHITE>(),
		   false,
		   (white.liberty() & DirectionTraits<U>::mask) == 0,
		   (white.liberty() & DirectionTraits<UL>::mask) == 0,
		   (white.liberty() & DirectionTraits<L>::mask) == 0),
      -1);
  }
}

void gpsshogi::KingXBlocked3Y::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication>&diffs) const
{
  King8Info black = state.king8Info(BLACK);
  if ((black.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask)) == 0)
  {
    diffs.add(
      index<BLACK>(state.kingSquare<BLACK>(),
		   true,
		   (black.liberty() & DirectionTraits<U>::mask) == 0,
		   (black.liberty() & DirectionTraits<UR>::mask) == 0,
		   (black.liberty() & DirectionTraits<R>::mask) == 0),
      1);
  }
  if ((black.liberty() & (DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    diffs.add(
      index<BLACK>(state.kingSquare<BLACK>(),
		   false,
		   (black.liberty() & DirectionTraits<U>::mask) == 0,
		   (black.liberty() & DirectionTraits<UL>::mask) == 0,
		   (black.liberty() & DirectionTraits<L>::mask) == 0),
      1);
  }
  King8Info white = state.king8Info(WHITE);
  if ((white.liberty() & (DirectionTraits<UL>::mask |
			  DirectionTraits<L>::mask |
			  DirectionTraits<DL>::mask)) == 0)
  {
    diffs.add(
      index<WHITE>(state.kingSquare<WHITE>(),
		   true,
		   (white.liberty() & DirectionTraits<U>::mask) == 0,
		   (white.liberty() & DirectionTraits<UR>::mask) == 0,
		   (white.liberty() & DirectionTraits<R>::mask) == 0),
      -1);
  }
  if ((white.liberty() & (DirectionTraits<UR>::mask |
			  DirectionTraits<R>::mask |
			  DirectionTraits<DR>::mask)) == 0)
  {
    diffs.add(
      index<WHITE>(state.kingSquare<WHITE>(),
		   false,
		   (white.liberty() & DirectionTraits<U>::mask) == 0,
		   (white.liberty() & DirectionTraits<UL>::mask) == 0,
		   (white.liberty() & DirectionTraits<L>::mask) == 0),
      -1);
  }
}


int gpsshogi::AnagumaEmpty::index(const NumEffectState &/*state*/,
				  Square king, Square target)
{
  return std::abs(king.x() - target.x()) + std::abs(king.y() - target.y()) * 2;
}

osl::MultiInt gpsshogi::AnagumaEmpty::eval(const NumEffectState &state, const MultiWeights& weights,
					   CArray<MultiInt,2>& /*saved_state*/) const
{
  MultiInt result;
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  featureOne<BLACK>(state, feature_count);
  featureOne<WHITE>(state, feature_count);

  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      result += EvalComponentStages::multiply(weights.value(i), feature_count[i]);
    }
  }
  return result;
}

template <osl::Player Defense>
void gpsshogi::AnagumaEmpty::featureOne(const NumEffectState &state,
					CArray<int, DIM> &features) const
{
  const Square king = state.kingSquare<Defense>();
  if ((king.x() == 1 || king.x() == 9) &&
      ((Defense == BLACK && king.y() == 9) ||
       (Defense == WHITE && king.y() == 1)))
  {
    const int x = (king.x() == 1 ? 2 : 8);
    const int y = (Defense == BLACK ? 8 : 2);
    if (state.pieceAt(Square(king.x(), y)).isEmpty())
      features[index(state, king, Square(king.x(), y))] +=
	sign(Defense);
    if (state.pieceAt(Square(x, y)).isEmpty())
      features[index(state, king, Square(x, y))] +=
	sign(Defense);
    if (state.pieceAt(Square(x, king.y())).isEmpty())
      features[index(state, king, Square(x, king.y()))] +=
	sign(Defense);
  }
}

void gpsshogi::AnagumaEmpty::featuresNonUniq(
  const NumEffectState &state,
  index_list_t&diffs,
  int offset) const
{
  CArray<int, DIM> feature_count;
  feature_count.fill(0);
  featureOne<BLACK>(state, feature_count);
  featureOne<WHITE>(state, feature_count);

  for (size_t i = 0; i < feature_count.size(); ++i)
  {
    if (feature_count[i] != 0)
    {
      diffs.add(offset+i, feature_count[i]);
    }
  }
}

void gpsshogi::AnagumaEmpty::showSummary(std::ostream &os, const MultiWeights& weights) const
{
  for (size_t s=0; s<MultiInt::size(); ++s) {
    os << name();
    for (size_t i = 0; i < DIM; ++i)
    {
      os << " " << weights.value(i)[s];
    }
    os << std::endl;
  }
}


void gpsshogi::
PieceKingRelativeNoSupport::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};

  PieceMask black = (~state.effectedMask(BLACK)) & state.piecesOnBoard(BLACK);
  black.reset(state.kingPiece<BLACK>().number());
  while (black.any())
  {
    const osl::Piece piece = state.pieceOf(black.takeOneBit());
    const int index_attack = index(BLACK, kings[WHITE],
				   piece);
    const int index_defense = index(BLACK, kings[BLACK],
				    piece) + ONE_DIM;
    diffs.add(index_attack, 1);
    diffs.add(index_defense, 1);
  }

  PieceMask white = (~state.effectedMask(WHITE)) & state.piecesOnBoard(WHITE);
  white.reset(state.kingPiece<WHITE>().number());
  while (white.any())
  {
    const osl::Piece piece = state.pieceOf(white.takeOneBit());
    const int index_attack = index(WHITE, kings[BLACK],
				   piece);
    const int index_defense = index(WHITE, kings[WHITE],
				    piece) + ONE_DIM;
    diffs.add(index_attack, -1);
    diffs.add(index_defense, -1);
  }
}
void gpsshogi::
PieceKingRelativeNoSupport::featuresOne(
  const NumEffectState &state,
  features_one_t &out) const
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  CArray<IndexCacheI<160>,2> diffs;

  PieceMask black = (~state.effectedMask(BLACK)) & state.piecesOnBoard(BLACK);
  black.reset(state.kingPiece<BLACK>().number());
  while (black.any())
  {
    const osl::Piece piece = state.pieceOf(black.takeOneBit());
    const int index_attack = index(BLACK, kings[WHITE],
				   piece);
    const int index_defense = index(BLACK, kings[BLACK],
				    piece) + ONE_DIM;
    diffs[0].add(index_attack, 1);
    diffs[1].add(index_defense, 1);
  }

  PieceMask white = (~state.effectedMask(WHITE)) & state.piecesOnBoard(WHITE);
  white.reset(state.kingPiece<WHITE>().number());
  while (white.any())
  {
    const osl::Piece piece = state.pieceOf(white.takeOneBit());
    const int index_attack = index(WHITE, kings[BLACK],
				   piece);
    const int index_defense = index(WHITE, kings[WHITE],
				    piece) + ONE_DIM;
    diffs[0].add(index_attack, -1);
    diffs[1].add(index_defense, -1);
  }
  diffs[0].output(out);
  diffs[1].output(out);
}

void gpsshogi::PtypeYYFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  const CArray<Square,2> kings = {{ 
      state.kingSquare(BLACK),
      state.kingSquare(WHITE),
    }};
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece p = state.pieceOf(i);
    if (!p.isOnBoard())
      continue;
    diffs.add(index<BLACK>(p, kings[BLACK]), 1);
    diffs.add(index<WHITE>(p, kings[WHITE]), -1);
  }
}

template <int N>
osl::CArray<int, N> gpsshogi::PtypeYYFeatures::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const CArray<int, N> &last_values, const Weights& w)
{
  if (moved.isPass())
    return last_values;
  assert(moved.ptype() != KING);
  CArray<int, N> result = last_values;
  const size_t dim = DIM;
  if (!moved.isDrop())
  {
    const int index_black = index<BLACK>(moved.oldPtypeO(), moved.from(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(moved.oldPtypeO(), moved.from(),
					 state.kingSquare<WHITE>());
    for (int i=0; i<N; ++i)
      result[i] -= ((w.value(index_black+i*dim) - w.value(index_white+i*dim)) *
		    (moved.player() == BLACK ? 1 : 1));
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const PtypeO ptypeO = newPtypeO(alt(moved.player()), captured);
    const int index_black = index<BLACK>(ptypeO, moved.to(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(ptypeO, moved.to(),
					 state.kingSquare<WHITE>());
    for (int i=0; i<N; ++i)
      result[i] -= ((w.value(index_black+i*dim) - w.value(index_white+i*dim)) *
		    (moved.player() == BLACK ? 1 : 1));
  }

  {
    const int index_black = index<BLACK>(moved.ptypeO(), moved.to(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(moved.ptypeO(), moved.to(),
					 state.kingSquare<WHITE>());
    for (int i=0; i<N; ++i)
      result[i] += ((w.value(index_black+i*dim) - w.value(index_white+i*dim)) *
		    (moved.player() == BLACK ? 1 : 1));
  }
  return result;
}
osl::MultiInt gpsshogi::PtypeYYFeatures::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values, const MultiWeights& w)
{
  if (moved.isPass())
    return last_values;
  assert(moved.ptype() != KING);
  MultiInt result = last_values;
  if (!moved.isDrop())
  {
    const int index_black = index<BLACK>(moved.oldPtypeO(), moved.from(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(moved.oldPtypeO(), moved.from(),
					 state.kingSquare<WHITE>());
    result -= (w.value(index_black) - w.value(index_white));
  }
  Ptype captured = moved.capturePtype();
  if (captured != PTYPE_EMPTY)
  {
    const PtypeO ptypeO = newPtypeO(alt(moved.player()), captured);
    const int index_black = index<BLACK>(ptypeO, moved.to(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(ptypeO, moved.to(),
					 state.kingSquare<WHITE>());
    result -= (w.value(index_black) - w.value(index_white));
  }

  {
    const int index_black = index<BLACK>(moved.ptypeO(), moved.to(),
					 state.kingSquare<BLACK>());
    const int index_white = index<WHITE>(moved.ptypeO(), moved.to(),
					 state.kingSquare<WHITE>());
    result += (w.value(index_black) - w.value(index_white));
  }
  return result;
}

osl::MultiInt gpsshogi::PtypeYYStages::evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  if (moved.ptype() == KING)
  {
    return evalMulti(state, saved_state);
  }
  return PtypeYYFeatures::evalWithUpdateMulti(state, moved, last_values, weight);
}


void gpsshogi::King3Pieces::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

template <osl::Player King>
void gpsshogi::King3Pieces::featuresKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  const int weight = (King == BLACK ? 1 : -1);
  const Square king = state.kingSquare<King>();
  diffs.add(
    index(King,
	  state.pieceAt(Board_Table.nextSquare(King, king, U)).ptypeO(),
	  state.pieceAt(Board_Table.nextSquare(King, king, D)).ptypeO(),
	  VERTICAL), weight);
  diffs.add(
    index(King,
	  state.pieceAt(Board_Table.nextSquare(King, king, L)).ptypeO(),
	  state.pieceAt(Board_Table.nextSquare(King, king, R)).ptypeO(),
	  HORIZONTAL), weight);
  diffs.add(
    index(King,
	  state.pieceAt(Board_Table.nextSquare(King, king, UL)).ptypeO(),
	  state.pieceAt(Board_Table.nextSquare(King, king, DR)).ptypeO(),
	  DIAGONAL), weight);
  diffs.add(
    index(King,
	  state.pieceAt(Board_Table.nextSquare(King, king, UR)).ptypeO(),
	  state.pieceAt(Board_Table.nextSquare(King, king, DL)).ptypeO(),
	  DIAGONAL), weight);
}

void gpsshogi::King3PiecesXY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

template <osl::Player King>
void gpsshogi::King3PiecesXY::featuresKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  const int weight = (King == BLACK ? 1 : -1);
  const Square king = state.kingSquare<King>();
  diffs.add(
    indexX(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, U)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, D)).ptypeO(),
	   VERTICAL), weight);
  diffs.add(
    indexX(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, L)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, R)).ptypeO(),
	   HORIZONTAL), weight);
  diffs.add(
    indexX(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, UL)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, DR)).ptypeO(),
	   DIAGONAL), weight);
  diffs.add(
    indexX(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, UR)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, DL)).ptypeO(),
	   DIAGONAL), weight);
  diffs.add(
    indexY(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, U)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, D)).ptypeO(),
	   VERTICAL), weight);
  diffs.add(
    indexY(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, L)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, R)).ptypeO(),
	   HORIZONTAL), weight);
  diffs.add(
    indexY(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, UL)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, DR)).ptypeO(),
	   DIAGONAL), weight);
  diffs.add(
    indexY(King, king,
	   state.pieceAt(Board_Table.nextSquare(King, king, UR)).ptypeO(),
	   state.pieceAt(Board_Table.nextSquare(King, king, DL)).ptypeO(),
	   DIAGONAL), weight);
}

template <osl::Player Defense, osl::Ptype PTYPE>
void gpsshogi::GoldAndSilverNearKing::countPiece(const NumEffectState &state,
						 CArray<int, 3> &out) const
{
  const Square king = state.kingSquare<Defense>();
  for (int i = PtypeTraits<PTYPE>::indexMin;
       i < PtypeTraits<PTYPE>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && piece.owner() == Defense)
    {
      const Square pos = piece.square();
      const int y_diff = std::abs(pos.y() - king.y());
      const int x_diff = std::abs(pos.x() - king.x());
      if (y_diff <= 2 && x_diff <= 3)
      {
	++out[std::max(x_diff, y_diff) - 1];
      }
    }
  }
}

template <osl::Player Defense>
void gpsshogi::GoldAndSilverNearKing::featuresKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  CArray<int, 3> count;
  count.fill(0);
  countPiece<Defense, GOLD>(state, count);
  countPiece<Defense, SILVER>(state, count);
  int total = 0;
  for (size_t i = 0; i < count.size(); ++i)
  {
    total += count[i];
    if (total != 0)
    {
      diffs.add(index<Defense>(state.kingSquare<Defense>(), i, total),
		Defense == BLACK ? 1 : -1);
    }
  }
}

void gpsshogi::GoldAndSilverNearKing::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

template <osl::Player P>
void gpsshogi::KingMobility::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  const Square king = state.kingSquare<P>();
  const int weight = (P == BLACK ? 1 : -1);
  if (P == BLACK)
  {
    diffs.add(index<BLACK, UL>(king, state.kingMobilityAbs(P, UL), 0), weight);
    diffs.add(index<BLACK, U>(king, state.kingMobilityAbs(P, U), 1), weight);
    diffs.add(index<BLACK, UR>(king, state.kingMobilityAbs(P, UR), 2), weight);
    diffs.add(index<BLACK, L>(king, state.kingMobilityAbs(P, L), 3), weight);
    diffs.add(index<BLACK, R>(king, state.kingMobilityAbs(P, R), 4), weight);
    diffs.add(index<BLACK, DL>(king, state.kingMobilityAbs(P, DL), 5), weight);
    diffs.add(index<BLACK, D>(king, state.kingMobilityAbs(P, D), 6), weight);
    diffs.add(index<BLACK, DR>(king, state.kingMobilityAbs(P, DR), 7), weight);
  }
  else
  {
    diffs.add(index<WHITE, UL>(king, state.kingMobilityAbs(P, UL), 7), weight);
    diffs.add(index<WHITE, U>(king, state.kingMobilityAbs(P, U), 6), weight);
    diffs.add(index<WHITE, UR>(king, state.kingMobilityAbs(P, UR), 5), weight);
    diffs.add(index<WHITE, L>(king, state.kingMobilityAbs(P, L), 4), weight);
    diffs.add(index<WHITE, R>(king, state.kingMobilityAbs(P, R), 3), weight);
    diffs.add(index<WHITE, DL>(king, state.kingMobilityAbs(P, DL), 2), weight);
    diffs.add(index<WHITE, D>(king, state.kingMobilityAbs(P, D), 1), weight);
    diffs.add(index<WHITE, DR>(king, state.kingMobilityAbs(P, DR), 0), weight);
  }
}

void gpsshogi::KingMobility::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

template <osl::Player P>
void gpsshogi::KingMobilitySum::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  const Square king = state.kingSquare<P>();
  const int weight = (P == BLACK ? 1 : -1);
  int sum = 0;
  for (int i = UL; i <= DR; ++i)
  {
    const Direction dir = static_cast<Direction>(i);
    const Square end = state.kingMobilityAbs(P, dir);
    sum += ((dir == L || dir == R) ? std::abs(king.x() - end.x()) :
	    std::abs(king.y() - end.y()));
  }
  diffs.add(index<P>(king, sum), weight);
}

void gpsshogi::KingMobilitySum::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

template <osl::Player P>
void gpsshogi::King25BothSideFeatures::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs)
{
  const int weight = (P == BLACK ? 1 : -1);
  CArray<int, 5> effects;
  for (int i = 0; i < effects.size(); ++i)
  {
    effects[i] = effectVertical<P>(state, i);
  }
  for (int i = 0; i <= 2; i++)
  {
    for (int j = 2; j <= 4; j++)
    {
      if (i == 2 && j ==2)
	continue;
      const int second = i == 2 ? j - 1 : j;
      diffs.add(index(effects[i], effects[j], i * 3 + second - 2), weight);
    }
  }
}

void gpsshogi::King25BothSideFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

osl::MultiInt gpsshogi::King25BothSide::
evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  const CArray<Square,2> kings = {{
      state.kingSquare<BLACK>(), 
      state.kingSquare<WHITE>()
    }};
  CArray<BoardMask,2> changes = {{ 
      state.changedEffects(BLACK), 
      state.changedEffects(WHITE)
    }};
  if (moved.ptype() == KING)
    changes[alt(moved.player())].set(moved.to());
  if (changes[BLACK].anyInRange(Board_Mask_Table5x5.mask(kings[WHITE])))
    saved_state[WHITE] = evalWhite(state);
  if (changes[WHITE].anyInRange(Board_Mask_Table5x5.mask(kings[BLACK])))
    saved_state[BLACK] = evalBlack(state);
  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Player P>
void gpsshogi::King25BothSideYFeatures::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs)
{
  const int weight = (P == BLACK ? 1 : -1);
  CArray<int, 5> effects;
  for (int i = 0; i < effects.size(); ++i)
  {
    effects[i] = effectVertical<P>(state, i);
  }
  for (int i = 0; i <= 2; i++)
  {
    for (int j = 2; j <= 4; j++)
    {
      if (i == 2 && j ==2)
	continue;
      const int second = i == 2 ? j - 1 : j;
      diffs.add(index<P>(state.kingSquare<P>(),
			 effects[i], effects[j], i * 3 + second - 2), weight);
    }
  }
}

void gpsshogi::King25BothSideYFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

osl::MultiInt gpsshogi::King25BothSideY::
evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  const CArray<Square,2> kings = {{
      state.kingSquare<BLACK>(), 
      state.kingSquare<WHITE>()
    }};
  CArray<BoardMask,2> changes = {{ 
      state.changedEffects(BLACK), 
      state.changedEffects(WHITE)
    }};
  if (moved.ptype() == KING)
    changes[alt(moved.player())].set(moved.to());
  if (changes[BLACK].anyInRange(Board_Mask_Table5x5.mask(kings[WHITE])))
    saved_state[WHITE] = evalWhite(state);
  if (changes[WHITE].anyInRange(Board_Mask_Table5x5.mask(kings[BLACK])))
    saved_state[BLACK] = evalBlack(state);
  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Player P>
void gpsshogi::King25BothSideXFeatures::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs)
{
  const int weight = (P == BLACK ? 1 : -1);
  CArray<int, 5> effects;
  for (int i = 0; i < effects.size(); ++i)
  {
    effects[i] = effectVertical<P>(state, i);
  }
  for (int i = 0; i <= 2; i++)
  {
    for (int j = 2; j <= 4; j++)
    {
      if (i == 2 && j ==2)
	continue;
      diffs.add(index<P>(state.kingSquare<P>(),
			 effects[i], effects[j], i, j), weight);
    }
  }
}

void gpsshogi::King25BothSideXFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

osl::MultiInt gpsshogi::King25BothSideX::
evalWithUpdateMulti(
      const NumEffectState& state,
      Move moved,
      const MultiInt &last_values,
      CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  const CArray<Square,2> kings = {{
      state.kingSquare<BLACK>(), 
      state.kingSquare<WHITE>()
    }};
  CArray<BoardMask,2> changes = {{ 
      state.changedEffects(BLACK), 
      state.changedEffects(WHITE)
    }};
  if (moved.ptype() == KING)
    changes[alt(moved.player())].set(moved.to());
  if (changes[BLACK].anyInRange(Board_Mask_Table5x5.mask(kings[WHITE])))
    saved_state[WHITE] = evalWhite(state);
  if (changes[WHITE].anyInRange(Board_Mask_Table5x5.mask(kings[BLACK])))
    saved_state[BLACK] = evalBlack(state);
  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Ptype MajorPiece>
template <osl::Player P>
void gpsshogi::KingMobilityWithMajor<MajorPiece>::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  assert(MajorPiece == ROOK || MajorPiece == BISHOP);
  const Square king = state.kingSquare<P>();
  const int weight = (P == BLACK ? 1 : -1);
  if (!state.hasPieceOnStand<MajorPiece>(alt(P)))
    return;
  if (P == BLACK)
  {
    diffs.add(index<BLACK, UL>(king, state.kingMobilityAbs(P, UL), 0), weight);
    diffs.add(index<BLACK, U>(king, state.kingMobilityAbs(P, U), 1), weight);
    diffs.add(index<BLACK, UR>(king, state.kingMobilityAbs(P, UR), 2), weight);
    diffs.add(index<BLACK, L>(king, state.kingMobilityAbs(P, L), 3), weight);
    diffs.add(index<BLACK, R>(king, state.kingMobilityAbs(P, R), 4), weight);
    diffs.add(index<BLACK, DL>(king, state.kingMobilityAbs(P, DL), 5), weight);
    diffs.add(index<BLACK, D>(king, state.kingMobilityAbs(P, D), 6), weight);
    diffs.add(index<BLACK, DR>(king, state.kingMobilityAbs(P, DR), 7), weight);
  }
  else
  {
    diffs.add(index<WHITE, UL>(king, state.kingMobilityAbs(P, UL), 7), weight);
    diffs.add(index<WHITE, U>(king, state.kingMobilityAbs(P, U), 6), weight);
    diffs.add(index<WHITE, UR>(king, state.kingMobilityAbs(P, UR), 5), weight);
    diffs.add(index<WHITE, L>(king, state.kingMobilityAbs(P, L), 4), weight);
    diffs.add(index<WHITE, R>(king, state.kingMobilityAbs(P, R), 3), weight);
    diffs.add(index<WHITE, DL>(king, state.kingMobilityAbs(P, DL), 2), weight);
    diffs.add(index<WHITE, D>(king, state.kingMobilityAbs(P, D), 1), weight);
    diffs.add(index<WHITE, DR>(king, state.kingMobilityAbs(P, DR), 0), weight);
  }
}

template <osl::Ptype MajorPiece>
void gpsshogi::KingMobilityWithMajor<MajorPiece>::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

template <osl::Player Defense, osl::Ptype PTYPE>
void gpsshogi::
GoldAndSilverNearKingCombination::countPiece(const NumEffectState &state,
					     CArray<int, 3> &out) const
{
  const Square king = state.kingSquare<Defense>();
  for (int i = PtypeTraits<PTYPE>::indexMin;
       i < PtypeTraits<PTYPE>::indexLimit;
       ++i)
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard() && piece.owner() == Defense)
    {
      const Square pos = piece.square();
      const int y_diff = std::abs(pos.y() - king.y());
      const int x_diff = std::abs(pos.x() - king.x());
      if (y_diff <= 2 && x_diff <= 3)
      {
	++out[std::max(x_diff, y_diff) - 1];
      }
    }
  }
}

template <osl::Player Defense>
void gpsshogi::GoldAndSilverNearKingCombination::featuresKing(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  CArray<int, 3> count;
  count.fill(0);
  countPiece<Defense, GOLD>(state, count);
  countPiece<Defense, SILVER>(state, count);
  diffs.add(index<Defense>(state.kingSquare<Defense>(),
			   std::min(5, count[0]),
			   std::min(5, count[1]),
			   std::min(5, count[2])),
	    Defense == BLACK ? 1 : -1);
}

void gpsshogi::GoldAndSilverNearKingCombination::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

template <osl::Player P>
void gpsshogi::GoldNearKing::countGold(const NumEffectState &state,
				       CArray<int, 4> &golds) const
{
  const Square king = state.kingSquare<P>();
  for (int i = PtypeTraits<GOLD>::indexMin;
       i < PtypeTraits<GOLD>::indexLimit; ++i)
  {
    const Piece gold = state.pieceOf(i);
    if (gold.owner() != P || !gold.isOnBoard())
      continue;
    const int distance = (((P == BLACK && gold.square().y() < king.y()) ||
			   (P == WHITE && gold.square().y() > king.y())) ?
			  std::abs(gold.square().x() - king.x()) +
			  std::abs(gold.square().y() - king.y()) :
			  std::max(std::abs(gold.square().x() - king.x()),
				   std::abs(gold.square().y() - king.y())));
    if (distance <= 4)
    {
      ++golds[distance - 1];
    }
  }
}

void gpsshogi::GoldNearKing::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  CArray<int, 4> golds;
  golds.fill(0);
  countGold<BLACK>(state, golds);
  diffs.add(index(golds), 1);
  golds.fill(0);
  countGold<WHITE>(state, golds);
  diffs.add(index(golds), -1);
}

namespace gpsshogi
{
  template class KingMobilityWithMajor<ROOK>;
  template class KingMobilityWithMajor<BISHOP>;
}

template <osl::Player P>
void gpsshogi::King25KingMobilityFeatures::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs)
{
  const int weight = (P == BLACK ? 1 : -1);
  CArray<int, 5> effects;
  for (int i = 0; i < effects.size(); ++i)
  {
    effects[i] = effectVertical<P>(state, i);
  }
  for (size_t i = 1; i < effects.size(); ++i)
  {
    diffs.add(index(effects[i - 1], effects[i], i - 1), weight);
  }
}

void gpsshogi::King25KingMobilityFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

osl::MultiInt gpsshogi::King25KingMobility::
evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  
  const Square kb = state.kingSquare<BLACK>(), kw = state.kingSquare<WHITE>();
  BoardMask mb = state.changedEffects(BLACK), mw = state.changedEffects(WHITE);
  mb.set(Square(moved.to())); mb.set(Square(moved.from()));
  mw.set(Square(moved.to())); mw.set(Square(moved.from()));
  if (mb.anyInRange(Board_Mask_Table5x5.mask(kw)))
    saved_state[WHITE] = evalWhite(state);
  if (mw.anyInRange(Board_Mask_Table5x5.mask(kb)))
    saved_state[BLACK] = evalBlack(state);
  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Player P>
void gpsshogi::King25KingMobilityYFeatures::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs)
{
  const int weight = (P == BLACK ? 1 : -1);
  const Square king = state.kingSquare<P>();
  CArray<int, 5> effects;
  for (int i = 0; i < effects.size(); ++i)
  {
    effects[i] = effectVertical<P>(state, i);
  }
  for (size_t i = 1; i < effects.size(); ++i)
  {
    diffs.add(index<P>(king, effects[i - 1], effects[i], i - 1), weight);
  }
}

void gpsshogi::King25KingMobilityYFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

osl::MultiInt gpsshogi::King25KingMobilityY::
evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  
  const Square kb = state.kingSquare<BLACK>(), kw = state.kingSquare<WHITE>();
  BoardMask mb = state.changedEffects(BLACK), mw = state.changedEffects(WHITE);
  mb.set(Square(moved.to())); mb.set(Square(moved.from()));
  mw.set(Square(moved.to())); mw.set(Square(moved.from()));
  if (mb.anyInRange(Board_Mask_Table5x5.mask(kw)))
    saved_state[WHITE] = evalWhite(state);
  if (mw.anyInRange(Board_Mask_Table5x5.mask(kb)))
    saved_state[BLACK] = evalBlack(state);
  return saved_state[BLACK] + saved_state[WHITE];
}

template <osl::Player P>
void gpsshogi::King25KingMobilityXFeatures::
featuresKing(const NumEffectState &state,
	    IndexCacheI<MaxActiveWithDuplication> &diffs)
{
  const int weight = (P == BLACK ? 1 : -1);
  const Square king = state.kingSquare<P>();
  CArray<int, 5> effects;
  for (int i = 0; i < effects.size(); ++i)
  {
    effects[i] = effectVertical<P>(state, i);
  }
  for (size_t i = 1; i < effects.size(); ++i)
  {
    diffs.add(index<P>(king, effects[i - 1], effects[i], i - 1), weight);
  }
}

void gpsshogi::King25KingMobilityXFeatures::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  featuresKing<BLACK>(state, diffs);
  featuresKing<WHITE>(state, diffs);
}

osl::MultiInt gpsshogi::King25KingMobilityX::
evalWithUpdateMulti(
  const NumEffectState& state,
  Move moved,
  const MultiInt &last_values,
  CArray<MultiInt,2>& saved_state) const
{
  if (moved.isPass())
    return last_values;
  
  const Square kb = state.kingSquare<BLACK>(), kw = state.kingSquare<WHITE>();
  BoardMask mb = state.changedEffects(BLACK), mw = state.changedEffects(WHITE);
  mb.set(Square(moved.to())); mb.set(Square(moved.from()));
  mw.set(Square(moved.to())); mw.set(Square(moved.from()));
  if (mb.anyInRange(Board_Mask_Table5x5.mask(kw)))
    saved_state[WHITE] = evalWhite(state);
  if (mw.anyInRange(Board_Mask_Table5x5.mask(kb)))
    saved_state[BLACK] = evalBlack(state);
  return saved_state[BLACK] + saved_state[WHITE];
}



void gpsshogi::King25Effect3::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  {
    int piece_count, stand_count, attacked_count;
    bool with_knight, stand_with_knight;
    countPieces<BLACK>(state, piece_count, with_knight,
		       stand_count, stand_with_knight, attacked_count);
    diffs.add(index(piece_count, with_knight,
		    stand_count, stand_with_knight, attacked_count),
	      1);
  }
  {
    int piece_count, stand_count, attacked_count;
    bool with_knight, stand_with_knight;
    countPieces<WHITE>(state, piece_count, with_knight,
		       stand_count, stand_with_knight, attacked_count);
    diffs.add(index(piece_count, with_knight,
		    stand_count, stand_with_knight, attacked_count),
	      -1);
  }
}

void gpsshogi::King25Effect3Y::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  {
    int piece_count, stand_count, attacked_count;
    bool with_knight, stand_with_knight;
    countPieces<BLACK>(state, piece_count, with_knight,
		       stand_count, stand_with_knight, attacked_count);
    diffs.add(index(piece_count, with_knight,
		    stand_count, stand_with_knight, attacked_count,
		    state.kingSquare<WHITE>().y()),
	      1);
  }
  {
    int piece_count, stand_count, attacked_count;
    bool with_knight, stand_with_knight;
    countPieces<WHITE>(state, piece_count, with_knight,
		       stand_count, stand_with_knight, attacked_count);
    diffs.add(index(piece_count, with_knight,
		    stand_count, stand_with_knight, attacked_count,
		    10 - state.kingSquare<BLACK>().y()),
	      -1);
  }
}

void gpsshogi::King25EffectCountCombination::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  {
    int attack_count, defense_count;
    countPieces<BLACK>(state, attack_count, defense_count);
    diffs.add(index(attack_count, defense_count), 1);
  }
  {
    int attack_count, defense_count;
    countPieces<WHITE>(state, attack_count, defense_count);
    diffs.add(index(attack_count, defense_count), -1);
  }
}

const std::string gpsshogi::
King25EffectCountCombination::describe(size_t local_index) const
{
  return "atk="+std::string(1,(char)('0'+local_index%10))
    +",def="+std::string(1,(char)('0'+local_index/10));
}

void gpsshogi::King25EffectCountCombinationY::featuresOneNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs) const
{
  {
    int attack_count, defense_count;
    King25EffectCountCombination::countPieces<BLACK>(
      state, attack_count, defense_count);
    diffs.add(index(attack_count, defense_count,
		    state.kingSquare<WHITE>().y()), 1);
  }
  {
    int attack_count, defense_count;
    King25EffectCountCombination::countPieces<WHITE>(
      state, attack_count, defense_count);
    diffs.add(index(attack_count, defense_count,
		    10 - state.kingSquare<BLACK>().y()), -1);
  }
}



void gpsshogi::BishopExchangeSilverKing::addOne(
  Player king_owner,
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs, int offset) const
{
  Square king = state.kingSquare(king_owner);
  int king_index = indexKing(king.squareForBlack(alt(king_owner))); // rotate if king is black
  if (king_index < 0)
    return;
  CArray<Piece,2> rook = {{
      state.nth<ROOK>(0), state.nth<ROOK>(1),
    }};
  if (rook[0].owner() == rook[1].owner())
    return;
  if (rook[0].owner() == king_owner)
    std::swap(rook[0], rook[1]);
  int rook_index0 = indexKing(rook[0].square().squareForBlack(king_owner)); // rotate if attaking rook is black
  if (rook_index0 < 0)
    return;
  int rook_index1 = indexKing(rook[1].square().squareForBlack(alt(king_owner))); // rotate if defending rook is black
  if (rook_index1 < 0)
    return;  
  FixedCapacityVector<Square, 4> silver;
  for (int i=0; i<state.nthLimit<SILVER>(); ++i) {
    Piece p = state.nth<SILVER>(i);
    assert(p.isOnBoard());
    if (p.owner() == king_owner)
      continue;
    silver.push_back(p.square().squareForBlack(king_owner)); // rotate if silver is black
  }
  if (silver.size() != 2 || silver[0].x() == silver[1].x())
    return;
  int silver_index
    = indexSilver((silver[0].x() > silver[1].x()) ? silver[0] : silver[1]);
  int index = offset + (king_index*81+silver_index)*90;
  diffs.add(index + rook_index0, sign(king_owner));
  diffs.add(index + 45 + rook_index1, sign(king_owner));
}

void gpsshogi::BishopExchangeSilverKing::featuresNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs, int offset) const
{
  if (state.promotedPieces().any())
    return;
  PieceMask stand_all = state.standMask(BLACK) | state.standMask(WHITE);
  stand_all.clearBit<BISHOP>();
  stand_all.clearBit<PAWN>();
  if (stand_all.any())
    return;
  if (state.nth<BISHOP>(0).owner() == state.nth<BISHOP>(1).owner())
    return;
  if (state.nth<BISHOP>(0).isOnBoard() != state.nth<BISHOP>(1).isOnBoard())
    return;
  if (state.nth<BISHOP>(0).isOnBoard()) {
    offset += BISHOP_ONE_DIM;
    if (state.hasEffectByPiece(state.nth<BISHOP>(0),
			       state.nth<BISHOP>(1).square()))
      offset += BISHOP_ONE_DIM;
  }
  addOne(BLACK, state, diffs, offset);
  addOne(WHITE, state, diffs, offset);
}

int gpsshogi::BishopExchangeSilverKing::
eval(const NumEffectState& state) const
{
  index_list_t features;
  featuresNonUniq(state, features, 0);
  int ret=0;
  for (size_t i=0; i<features.size(); ++i)
    ret += value(features[i].first) * features[i].second;
  return ret;
}



void gpsshogi::EnterKingDefense::addOne(
  Player king_owner,
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs, int offset) const
{
  Square king = state.kingSquare(king_owner);
  if (king.y() < 4 || king.y() > 6) // target: [4,6]
    return;
  CArray2d<int, 2, 2> count = {{{ 0 }}};
  for (int x=std::max(1, king.x()-2); x<=std::min(9, king.x()+2); ++x) {
    for (int y=king.y()-2*sign(king_owner); 1; y-=sign(king_owner)) {
      const Piece p = state.pieceAt(Square(x, y));
      if (p.isEdge())
	break;
      if (p.isEmpty())
	continue;
      count[p.owner() == king_owner][p.ptype() == PAWN]++;
    }
  }
  int c = king.squareForBlack(king_owner).y() - 4;
  diffs.add(offset + c*(   std::min(7, count[0][0])), sign(king_owner));
  diffs.add(offset + c*(8 +std::min(7, count[0][1])), sign(king_owner));
  diffs.add(offset + c*(16+std::min(7, count[1][0])), sign(king_owner));
  diffs.add(offset + c*(24+std::min(7, count[1][1])), sign(king_owner));
}

void gpsshogi::EnterKingDefense::featuresNonUniq(
  const NumEffectState &state,
  IndexCacheI<MaxActiveWithDuplication> &diffs, int offset) const
{
  addOne(BLACK, state, diffs, offset);
  addOne(WHITE, state, diffs, offset);
}

int gpsshogi::EnterKingDefense::
eval(const NumEffectState& state) const
{
  index_list_t features;
  featuresNonUniq(state, features, 0);
  int ret=0;
  for (size_t i=0; i<features.size(); ++i)
    ret += value(features[i].first) * features[i].second;
  return ret;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
