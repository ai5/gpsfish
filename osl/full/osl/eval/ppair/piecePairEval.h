/* piecePairEval.h
 */
#ifndef _PIECE_PAIR_EVAL_H
#define _PIECE_PAIR_EVAL_H

#include "osl/eval/ppair/piecePairIndex.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/evalTraits.h"
#include <iosfwd>

namespace osl
{
  namespace container
  {
    class PieceValues;
  } // container
  
  namespace eval
  {
    namespace ppair
    {
      /**
       * PiecePairEval の，template parameterに依存しない部分の
       * 共通の実装.
       */
      class PiecePairEvalBase
      {
      protected:
	int val;
	PiecePairEvalBase() : val(0)
	{
	}
	~PiecePairEvalBase()
	{
	}
      public:
	/** roundup は 2^n であること */
	static const int ROUND_UP = 2;
	static int roundUp(int v)
	{
	  return v & (~(ROUND_UP-1)); 
	}
	int value() const	{ return roundUp(val); }
	int rawValue() const { return val; }
	static int infty()
	{
	  // ProgressEval で足してもoverflowしない値にする
	  // PieceEval::infty() + 100*40*39/2
	  return 150000;
	}

	static int captureValue(PtypeO ptypeo)
	{
	  return PieceEval::captureValue(ptypeo); 
	}
      };

      template <class Table>
      class PiecePairEvalTableBase : public PiecePairEvalBase
      {
      protected:
	// 必ず継承して使う
	explicit PiecePairEvalTableBase(const SimpleState& state);
	~PiecePairEvalTableBase() {}
      public:
	/**
	 * 駒が old_index から new_index に動いたときの値の差分
	 * @param state 動く前の局面
	 * @param old_index 駒+移動元
	 * @param new_index 駒+移動先
	 */
	static int adjustPairs(const SimpleState& state,
			       unsigned int new_index);    
	static int adjustPairs(const SimpleState& state,
			       unsigned int old_index, unsigned int new_index);    
	static int adjustPairs(const SimpleState& state, 
			       unsigned int old_index, unsigned int old_index2, 
			       unsigned int new_index);
	static int diffAfterSimpleMove(const SimpleState& state,
				       Square from, Square to, 
				       int promote_mask)
	{
	  const Piece old_piece=state.pieceAt(from);
	  const PtypeO newPtypeO = promoteWithMask(old_piece.ptypeO(), promote_mask);
	  const unsigned int old_index = PiecePairIndex::indexOf(old_piece);
	  const unsigned int new_index = PiecePairIndex::indexOf(to, newPtypeO);
	  return adjustPairs(state, old_index, new_index);
	}
	static int diffAfterDropMove(const SimpleState& state,Square to,PtypeO ptypeo)
	{
	  const unsigned int new_index = PiecePairIndex::indexOf(to, ptypeo);
	  return adjustPairs(state, new_index);
	}
	static int diffAfterCaptureMove(const SimpleState& state,
					Square from, Square to, 
					PtypeO victim,int promote_mask)
	{
	  const Piece old_piece=state.pieceAt(from);
	  const PtypeO newPtypeO = promoteWithMask(old_piece.ptypeO(), promote_mask);
	  const unsigned int old_index = PiecePairIndex::indexOf(old_piece);
	  const unsigned int new_index = PiecePairIndex::indexOf(to, newPtypeO);

	  const unsigned int indexVictim = PiecePairIndex::indexOf(to, victim);
	  return adjustPairs(state, old_index, indexVictim, 
			     new_index);
	}
	/** この時 state は move した後
	 */
	static int adjustPairsAfterMove(const SimpleState& state,
					unsigned int new_index);    
	static int adjustPairsAfterMove(const SimpleState& state,
					unsigned int old_index, unsigned int new_index);    
	static int adjustPairsAfterMove(const SimpleState& state, 
					unsigned int old_index, unsigned int old_index2, 
					unsigned int new_index);
	static int diffWithUpdate(const SimpleState& new_state, Move last_move)
	{
	  const unsigned int new_index = PiecePairIndex::indexOf(last_move.to(), last_move.ptypeO());
	  if (last_move.isDrop())
	    return adjustPairsAfterMove(new_state, new_index);
	  
	  const unsigned int old_index = PiecePairIndex::indexOf(last_move.from(), last_move.oldPtypeO());
	  if (last_move.capturePtype() == PTYPE_EMPTY)
	    return adjustPairsAfterMove(new_state, old_index, new_index);

	  const unsigned int index_victim = PiecePairIndex::indexOf(last_move.to(), last_move.capturePtypeO());
	  return adjustPairsAfterMove(new_state, old_index, index_victim, new_index);
	}
	/**
	 * 関係の値をPiece 毎の価値に変換する.
	 * - 駒Aの価値は r(A,A) + \sum_B(r(A+B)/2)
	 * - 但し，A,Bのどちらか(のみ)玉の時には，玉でない方に関係の点数を集める
	 */
	static void setValues(const SimpleState&, container::PieceValues&);

      private:
	static bool& initializationFlag();
      public:
	static bool initialized() { return initializationFlag(); } // read only
	static bool setUp(const char *filename);
	static bool setUp();
      };

      /**
       * 駒のペアの統計情報を元にした評価関数の共通部分.
       * - 必ず偶数
       * - 先手有利 +, 後手有利 -
       * @param Table PiecePairTable のどれかのinstatiationを想定
       */
      template <class Eval,class Table>
      class PiecePairEval : public PiecePairEvalTableBase<Table>
      {
      protected:
	// 必ず継承して使う
	explicit PiecePairEval(const SimpleState& state);
      public:
	typedef PiecePairEvalTableBase<Table> base_t;
	void changeTurn() {}
	/** この時 state は move する前
	 */
	int expect(const SimpleState& state, Move m) const;
	/** この時 state は move した後
	 */
	void update(const SimpleState& new_state, Move last_move)
	{
	  base_t::val += Eval::diffWithUpdate(new_state, last_move);
	}
	  
	static int diffWithMove(const SimpleState& state, Move move)
	{
	  // TRICK: Eval::diffXXX とすることでsubclassによるoverrideを可能にする
	  const Square from=move.from();
	  const Square to=move.to();
	  if (from.isPieceStand())
	    return Eval::diffAfterDropMove(state, to, move.ptypeO());

	  const Ptype ptypeCaptured=move.capturePtype();
	  if (ptypeCaptured != PTYPE_EMPTY)
	    return Eval::diffAfterCaptureMove(state, from, to,
					      newPtypeO(alt(move.player()),
							ptypeCaptured),
					      move.promoteMask());

	  return Eval::diffAfterSimpleMove(state, from,to,move.promoteMask());
	}
	
      };

    } // namespace ppair
  } // namespace eval
} // namespace osl

#endif /* _PIECE_PAIR_EVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
