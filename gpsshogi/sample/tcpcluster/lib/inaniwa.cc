/* inaniwa.cc
 */
#include "inaniwa.h"

namespace {
  int CountPieceOnEnemyCamp(const osl::NumEffectState& state, osl::Player Enemy) {
    int countPiece = 0;
    const int enemyCampMin = (Enemy==osl::BLACK) ? 8 : 1;
    const int enemyCampMax = enemyCampMin + 1;
    for (int i = enemyCampMin; i <= enemyCampMax; i++)
      for (int j=1; j<=9; j++){
	osl::Piece pieceOnEnemyCamp = state.pieceOnBoard(osl::Square(j,i));
	if (pieceOnEnemyCamp.isOnBoardByOwner(Enemy)) {
	  countPiece += 1 + 4 * isMajor(pieceOnEnemyCamp.ptype())
	    - 1 * (pieceOnEnemyCamp.ptype() == osl::PAWN);
	}
      }
    return countPiece;
  }
  int CountPawnOnEnemyFront(const osl::NumEffectState& state, osl::Player Enemy) {
    int countPiece = 0;
    const int enemyFront = (Enemy==osl::BLACK) ? 7 : 3;
    for (int i=2; i<=8; i++){
      osl::Piece pieceOnEnemyFront = state.pieceOnBoard(osl::Square(i,enemyFront));
      if (pieceOnEnemyFront.isOnBoardByOwner(Enemy) && pieceOnEnemyFront.ptype() == osl::PAWN)
	countPiece++;
    }
    return countPiece;
  }
}

bool osl::inaniwa::InaniwaDetection::IsInaniwa(const osl::NumEffectState& state, int my_total_sec, int op_total_sec, 
			     int my_remain_sec, int op_remain_sec) {
  // ���о��: 
  // �Ĥ���֤������֤�1/5 ̤�� (900 -> 180, 2500 -> 500)
  // ���λ��ѻ��֤�8 �ܤλ��֤����
  // ξü�����, Ũ�ؤ�3 ���ܤ��⤬6 ��ʾ夢��
  // Ũ��2 ���ܤˤ�����������18 ���ʾ�ʲ���ޤ��, ������, ����5����

  if (my_total_sec < 5 * my_remain_sec) return false;
  if (my_total_sec - my_remain_sec < 8 * (op_total_sec - op_remain_sec)) return false;
  const osl::Player Enemy = alt(state.turn());
  if (CountPawnOnEnemyFront(state, Enemy) < 5) return false;
  if (CountPieceOnEnemyCamp(state, Enemy) < 17) return false;
  return true;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
