#ifndef ENDGAME_KINGPIECEVALUES_H
#define ENDGAME_KINGPIECEVALUES_H
#include "osl/container/pieceValues.h"
#include "osl/simpleState.h"

namespace osl
{
  namespace eval
  {
    namespace endgame
    {
      /**
       * @param Eval AttackKing や DefenseKing
       */
      template <class Eval>
      struct KingPieceValues
      {
	/**
	 * 局面の駒の値を求める
	 */
	static void setValues(const SimpleState&, PieceValues&);
      };
    } // namespace endgame
  } // namespace eval
} // namespace osl

template <class Eval>
void osl::eval::endgame::
KingPieceValues<Eval>::setValues(const SimpleState& state, PieceValues& values)
{
  values.fill(0);
  // 速度は無視
  const Piece king_black = state.kingPiece(BLACK);
  const Piece king_white = state.kingPiece(WHITE);
  
  for (int i=0; i<Piece::SIZE; i++) {
    if(!state.usedMask().test(i)) continue;
    const Piece target = state.pieceOf(i);
    values[i] = (Eval::valueOf(king_black, target) 
		 + Eval::valueOf(king_white, target));
  }
}

#endif /* ENDGAME_KINGPIECEVALUES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
