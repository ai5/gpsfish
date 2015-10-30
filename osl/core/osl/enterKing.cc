/* enterKing.cc
 */
#include "osl/enterKing.h"

bool osl::enter_king::EnterKing::canDeclareWin(const NumEffectState& state)
{
  if (state.turn() == BLACK)
    return canDeclareWin<BLACK>(state);
  else
    return canDeclareWin<WHITE>(state);
}

template <osl::Player Turn>
bool
osl::enter_king::EnterKing::canDeclareWin(const NumEffectState& state)
{
  //手番, 持時間は省略
  assert(Turn == state.turn());
  const Square myKingSquare
    = state.kingSquare(Turn);

  //王手がかかっていないか
  if ( state.hasEffectAt(alt(Turn), myKingSquare) )
    return false;

  //自玉が敵陣にいるか
  //先手なら1~3
  //後手なら7~9
  const int y = myKingSquare.y();
  const int enemyCampMin = (Turn==BLACK) ? 1 : 7;
  const int enemyCampMax = enemyCampMin + 2;

  if( (y < enemyCampMin) || (y > enemyCampMax) )
    return false;

  // 敵陣に自分の駒が10枚以上 (自玉を除いて) あるか
  // 駒の点数を勘定する.  (対象: 敵陣の駒 + 持駒)
  // 大駒を5点として, 先手は28点, 後手なら27点必要
  int countPiece = 0;
  int onEnemyCamp = -1; // 自玉の分を予め引いておく

  for (int i = enemyCampMin; i <= enemyCampMax; i++)
    for (int j=1; j<=9; j++){
      Piece pieceOnEnemyCamp = state.pieceOnBoard(Square(j,i));
      if (pieceOnEnemyCamp.isOnBoardByOwner<Turn>()) {
	++countPiece;
	onEnemyCamp += 1 + 4 * isMajor(pieceOnEnemyCamp.ptype());
      }
    }

  if (countPiece < 11)
    return false;

  int onStand =
    5 * state.countPiecesOnStand<ROOK>(Turn)
    + 5 * state.countPiecesOnStand<BISHOP>(Turn)
    + state.countPiecesOnStand<GOLD>(Turn)
    + state.countPiecesOnStand<SILVER>(Turn)
    + state.countPiecesOnStand<KNIGHT>(Turn)
    + state.countPiecesOnStand<LANCE>(Turn)
    + state.countPiecesOnStand<PAWN>(Turn);

  if ( onEnemyCamp + onStand < 27 + (Turn==BLACK) )
    return false;

  return true;
}

bool osl::enter_king::EnterKing::canDeclareWin(const NumEffectState& state, int &distance)
{
  if (state.turn() == BLACK)
    return canDeclareWin<BLACK>(state, distance);
  else
    return canDeclareWin<WHITE>(state, distance);
}

template <osl::Player Turn>
bool
osl::enter_king::EnterKing::canDeclareWin(const NumEffectState& state, int &drops)
{
  // 宣言勝ちの条件を
  //「敵陣に（自玉を除いて）10枚以上いる」
  // 以外満たしている時に
  // 敵陣にあと何枚駒を打てば良いかをdrops で返す
  // 他の条件を満たしていない場合何枚打っても勝てないので41を返す
  
  //手番, 持時間は省略
  assert(Turn == state.turn());
  const Square myKingSquare
    = state.kingSquare(Turn);
  drops = 41;

  //王手がかかっていないか
  if ( state.hasEffectAt(alt(Turn), myKingSquare) )
    return false;

  //自玉が敵陣にいるか
  //先手なら1~3
  //後手なら7~9
  const int y = myKingSquare.y();
  const int enemyCampMin = (Turn==BLACK) ? 1 : 7;
  const int enemyCampMax = enemyCampMin + 2;

  if( (y < enemyCampMin) || (y > enemyCampMax) )
    return false;

  // 敵陣に自分の駒が10枚以上 (自玉を除いて) あるか
  // 駒の点数を勘定する.  (対象: 敵陣の駒 + 持駒)
  // 大駒を5点として, 先手は28点, 後手なら27点必要
  int countPiece = 0;
  int onEnemyCamp = -1; // 自玉の分を予め引いておく

  for (int i = enemyCampMin; i <= enemyCampMax; i++)
    for (int j=1; j<=9; j++){
      Piece pieceOnEnemyCamp = state.pieceOnBoard(Square(j,i));
      if (pieceOnEnemyCamp.isOnBoardByOwner<Turn>()) {
	++countPiece;
	onEnemyCamp += 1 + 4 * isMajor(pieceOnEnemyCamp.ptype());
      }
    }

  int onStand =
    5 * state.countPiecesOnStand<ROOK>(Turn)
    + 5 * state.countPiecesOnStand<BISHOP>(Turn)
    + state.countPiecesOnStand<GOLD>(Turn)
    + state.countPiecesOnStand<SILVER>(Turn)
    + state.countPiecesOnStand<KNIGHT>(Turn)
    + state.countPiecesOnStand<LANCE>(Turn)
    + state.countPiecesOnStand<PAWN>(Turn);

  if ( onEnemyCamp + onStand < 27 + (Turn==BLACK) )
    return false;

  if (countPiece < 11) {
    drops = 11 - countPiece;
    return false;
  } else {
    drops = 0;
  }

  return true;
}

namespace osl
{
  namespace enter_king
  {
    template bool osl::enter_king::EnterKing::canDeclareWin<BLACK>(const NumEffectState&);
    template bool osl::enter_king::EnterKing::canDeclareWin<WHITE>(const NumEffectState&);
    template bool osl::enter_king::EnterKing::canDeclareWin<BLACK>(const NumEffectState&, int &drops);
    template bool osl::enter_king::EnterKing::canDeclareWin<WHITE>(const NumEffectState&, int &drops);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
