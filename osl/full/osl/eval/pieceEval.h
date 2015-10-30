/* pieceEval.h
 */
#ifndef OSL_PIECEEVAL_H
#define OSL_PIECEEVAL_H

#include "osl/eval/ptypeEval.h"
#include "osl/progress.h"
#include "osl/numEffectState.h"
#include <cassert>

namespace osl
{
  namespace eval
  {
    /**
     * 駒の価値ベースの評価関数.
     * 必ず偶数
     * 先手有利 +, 後手有利 -
     * 歩 PtypeEvalTraits<PAWN>::val 点
     */
    class PieceEval
    {
      int val;
    public:
      explicit PieceEval(const NumEffectState& state);
      explicit PieceEval(int v) : val(v) {}
      PieceEval();
      static bool initialized() { return true; }
      void changeTurn() {}
      int value() const
      { 
	assert(isConsistentValueForNormalState<PieceEval>(val)); 
	return val; 
      }
      static int infty() { return 57984; }

      /** 
       * move による取り返し値の変化 (SOMA)
       *
       * - move 後のマスだけ考える
       * - 基本は価値の小さい順に調べる
       * - 順番に関して，PROMOTE の有無は考えていない
       * - 現在，飛車や香の利きはmove の後ろにあるものしか伸びない
       * - ?? 駒が味方の駒を飛び越える手も考える
       * - 数値の価値は Player にとって．
       * - 王手などは気にしない
       */
      template<Player P>
      static int computeDiffAfterMove(const NumEffectState& state,Move move);
      static int computeDiffAfterMove(const NumEffectState& state,Move move)
      {
	assert(state.turn() == move.player());
	if (state.turn() == BLACK)
	  return computeDiffAfterMove<BLACK>(state,move);
	else
	  return computeDiffAfterMove<WHITE>(state,move);
      }
      /**
       * 実現確率探索用取り返し値
       * 
       * 現在の局面の評価値と move 後の局面の差分(Pが得する場合が正とな
       * るよう符号を補正)を返す．
       */
      template<Player P>
      static int computeDiffAfterMoveForRP(const NumEffectState& state,Move move)
      {
	assert(move.player()==P);
	const int diff = computeDiffAfterMove<P>(state,move);
	return (P==BLACK) ? diff : -diff;
      }
      static int computeDiffAfterMoveForRP(const NumEffectState& state, Move move)
      {
	if (move.player()==BLACK)
	  return computeDiffAfterMoveForRP<BLACK>(state,move);
	else
	  return computeDiffAfterMoveForRP<WHITE>(state,move);
      }
    private:
      void addVal(int d) { val+=d; }
    public:
      const Move suggestMove(const NumEffectState&) const 
      {
	return Move();
      }
      /** state でmoveを指した後の評価値を予測 */
      int expect(const NumEffectState& /*state*/, Move move) const
      {
	if (move.isPass() || move.isDrop())
	  return value();
	const PtypeO ptypeO=move.ptypeO();
	const PtypeO captured=move.capturePtypeOSafe();
	int result = val 
	  + Ptype_Eval_Table.value(ptypeO)
	  - Ptype_Eval_Table.value(move.oldPtypeO());
	if (getPtype(captured) != PTYPE_EMPTY)
	  result += Ptype_Eval_Table.value(osl::captured(captured))
	    - Ptype_Eval_Table.value(captured);
	return result;
      }

      const Progress32 progress32() const { return Progress32(0); }
      const Progress16 progress16() const { return Progress16(0); }
      static int seeScale() { return 1; }
      /**
       * QuiescenceSearch の枝刈で使用
       */
      static int captureValue(PtypeO ptypeO)
      {
	return Ptype_Eval_Table.captureValue(ptypeO);
      }
      static int value(PtypeO ptypeO)
      {
	return Ptype_Eval_Table.value(ptypeO);
      }

      void update(const NumEffectState& /*new_state*/, Move last_move)
      {
	if (last_move.isPass() || last_move.isDrop())
	  return;

	addVal(Ptype_Eval_Table.value(last_move.ptypeO())
	       - Ptype_Eval_Table.value(last_move.oldPtypeO()));
	if (last_move.capturePtype() != PTYPE_EMPTY) {
	  const PtypeO capture_ptypeo = last_move.capturePtypeO();
	  addVal(Ptype_Eval_Table.value(captured(capture_ptypeo))
		 - Ptype_Eval_Table.value(capture_ptypeo));
	}
      }

      static PtypeEvalTable Piece_Value;
    };

  } // namespace eval
  using eval::PieceEval;
} // namespace osl

#endif /* OSL_PIECEEVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
