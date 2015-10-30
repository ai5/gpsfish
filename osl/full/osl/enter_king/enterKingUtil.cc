#include "osl/enter_king/enterKingUtil.h"

// 範囲内(x0,y0) - (x1,y1) の利きの数を調べる
int osl::enter_king::countEffectInRange(const NumEffectState& state,
					osl::Player Turn,
					int x0, int x1, int y0, int y1) {
  assert(x0 >= 1 && x0 <= 9);
  assert(x1 >= 1 && x1 <= 9);
  assert(y0 >= 1 && y0 <= 9);
  assert(y1 >= 1 && y1 <= 9);
  assert(x0 <= x1);
  assert(y0 <= y1);
  int effect_count = 0;
  for (int sy = y0; sy <= y1; sy++)
    for (int sx = x0; sx <= x1; sx++)
      effect_count += state.countEffect(Turn, osl::Square(sx,sy));
  return effect_count;
}

// target の前方（target を中心とした幅3マス) の利きの数を調べる
int osl::enter_king::countEffectInFrontOf(const NumEffectState& state, 
					  osl::Player attack, osl::Square target, 
					  osl::Player defense) {
  const int ky = target.y();
  // 一番前面にいるので前方が存在しない
  if ((defense == osl::BLACK && ky == 1) ||
      (defense == osl::WHITE && ky == 9)) return 0;

  const int kx = (target.x() == 1) ? 2 : ((target.x() == 9)? 8 : target.x());
  if (defense == osl::BLACK)
    return osl::enter_king::countEffectInRange(state, attack, kx - 1, kx + 1, 1, ky - 1);
  else
    return osl::enter_king::countEffectInRange(state, attack, kx - 1, kx + 1, ky + 1, 9);
}

// 駒台の駒の点数を数える
int osl::enter_king::countPiecePointsOnStand(const NumEffectState& state, osl::Player Turn) {
  return (5 * state.countPiecesOnStand<osl::ROOK>(Turn)
	  + 5 * state.countPiecesOnStand<osl::BISHOP>(Turn)
	  + state.countPiecesOnStand<osl::GOLD>(Turn)
	  + state.countPiecesOnStand<osl::SILVER>(Turn)
	  + state.countPiecesOnStand<osl::KNIGHT>(Turn)
	  + state.countPiecesOnStand<osl::LANCE>(Turn)
	  + state.countPiecesOnStand<osl::PAWN>(Turn));
}

// 範囲内(x0,y0) - (x1,y1) の駒の点数を返す
// num_pieces には点数ではなく、枚数を足し込む
template <osl::Player Turn>
int osl::enter_king::countPiecePointsInRange(const NumEffectState& state, 
					     int& num_pieces, 
					     int x0, int x1, int y0, int y1) {
  assert(x0 >= 1 && x0 <= 9);
  assert(x1 >= 1 && x1 <= 9);
  assert(y0 >= 1 && y0 <= 9);
  assert(y1 >= 1 && y1 <= 9);
  assert(x0 <= x1);
  assert(y0 <= y1);
  int count = 0;
  for (int sy = y0; sy <= y1; sy++) {
    for (int sx = x0; sx <= x1; sx++) {
      osl::Piece pieceOnSquare = state.pieceOnBoard(osl::Square(sx,sy));
      if (! pieceOnSquare.isOnBoardByOwner<Turn>() ||
	  pieceOnSquare.ptype() == osl::KING)
	continue;
      count += (1 + 4 * isMajor(pieceOnSquare.ptype()));
      num_pieces++;
    }
  }
  return count;
}
int osl::enter_king::countPiecePointsInRange(const NumEffectState& state, osl::Player Turn, 
					     int& num_pieces, 
					     int x0, int x1, int y0, int y1) {
  if (Turn == osl::BLACK)
    return osl::enter_king::countPiecePointsInRange<BLACK>(state, num_pieces, x0, x1, y0, y1);
  else
    return osl::enter_king::countPiecePointsInRange<WHITE>(state, num_pieces, x0, x1, y0, y1);
}
// row 行目の駒の点数と枚数を調べる
template <osl::Player Turn>
int osl::enter_king::countPiecePointsOnRow(const NumEffectState& state,
					   int& num_pieces, int row) {
  return osl::enter_king::countPiecePointsInRange<Turn>(state, num_pieces, 1, 9, row, row);
}
int osl::enter_king::countPiecePointsOnRow(const NumEffectState& state, osl::Player Turn,
					   int& num_pieces, int row) {
  if (Turn == osl::BLACK)
    return osl::enter_king::countPiecePointsOnRow<BLACK>(state, num_pieces, row);
  else
    return osl::enter_king::countPiecePointsOnRow<WHITE>(state, num_pieces, row);
}

namespace osl {
  namespace enter_king {
    template 
    int countPiecePointsInRange<BLACK>
    (const NumEffectState& state, int& num_pieces, 
     int x0, int x1, int y0, int y1);
    template 
    int countPiecePointsInRange<WHITE>
    (const NumEffectState& state, int& num_pieces, 
     int x0, int x1, int y0, int y1);
    template 
    int countPiecePointsOnRow<BLACK>
    (const NumEffectState& state, int& num_pieces, int row);
    template 
    int countPiecePointsOnRow<WHITE>
    (const NumEffectState& state, int& num_pieces, int row);
  }
}
