/* piecePairWithStand.h
 */
#ifndef EVAL_PPAIR_PIECEPAIRWITHSTAND_H
#define EVAL_PPAIR_PIECEPAIRWITHSTAND_H

#include "osl/eval/ppair/piecePairEval.h"
#include "osl/eval/pieceEval.h"

namespace osl
{
  namespace eval
  {
    namespace ppair
    {
      /**
       * 持駒の点数は表以外で管理するフレームワーク.  
       * 
       * (持駒の点数を表に書き込むと，同じ種類の駒を複数もっていると問
       * 題が起こるため)
       * => その後盤上の駒も管理 (点数を変更可能にするため)
       */
      template <class Table>
      class PiecePairWithStand
	: public PiecePairEval<PiecePairWithStand<Table>,Table>
      {
      public:
	static int standBonus(PtypeO ptypeo)
	{
	  assert(isBasic(getPtype(ptypeo)));
	  if (isMajorBasic(getPtype(ptypeo)))
	    return Table::Piece_Value.value(newPtypeO(getOwner(ptypeo), PAWN));
	  return 0;
	}
	static int standBonus(const SimpleState& state);

	typedef PiecePairEval<PiecePairWithStand<Table>, Table> base_t;
	explicit PiecePairWithStand(const SimpleState& state);
      protected:
	~PiecePairWithStand() {}
      public:
	static int diffAfterDropMove(const SimpleState& state,Square to,PtypeO ptypeo)
	{
	  const int bonus = standBonus(ptypeo);
	  return base_t::diffAfterDropMove(state,to,ptypeo) - bonus;
	}
	static int diffAfterSimpleMove(const SimpleState& state,
				       Square from, Square to, 
				       int promote_mask)
	{
	  int diff = base_t::diffAfterSimpleMove(state, from, to, promote_mask);
	  if (promote_mask) {
	    const Piece old_piece=state.pieceAt(from);
	    const PtypeO newPtypeO = promoteWithMask(old_piece.ptypeO(), promote_mask);
	    diff += Table::Piece_Value.promoteValue(newPtypeO);
	  }
	  return diff;
	}
	static int diffAfterCaptureMove(const SimpleState& state,
					Square from, Square to, 
					PtypeO victim,int promote_mask)
	{
	  const PtypeO captured = osl::captured(victim);
	  int bonus = standBonus(captured);
	  if (promote_mask) {
	    const Piece old_piece=state.pieceAt(from);
	    const PtypeO newPtypeO = promoteWithMask(old_piece.ptypeO(), promote_mask);
	    bonus += Table::Piece_Value.promoteValue(newPtypeO);
	  }
	  return base_t::diffAfterCaptureMove(state,from,to,victim,promote_mask) 
	    + Table::Piece_Value.captureValue(victim) + bonus;
	}
	static int diffWithUpdate(const SimpleState& new_state, Move last_move)
	{
	  int diff = base_t::diffWithUpdate(new_state, last_move);
	  if (last_move.isDrop()) {
	    const int bonus = standBonus(last_move.ptypeO());
	    return diff - bonus;
	  }
	  if (last_move.isPromotion())
	    diff += Table::Piece_Value.promoteValue(last_move.ptypeO());
	  if (last_move.capturePtype() != PTYPE_EMPTY) {
	    const PtypeO captured = last_move.capturePtypeO();
	    const int bonus = standBonus(osl::captured(captured));
	    diff += Table::Piece_Value.captureValue(captured) + bonus;
	  }
	  return diff;
	}
	static void setValues(const SimpleState&, container::PieceValues&);
      };
      
    } // namespace ppair
  } // namespace eval
} // namespace osl


#endif /* EVAL_PPAIR_PIECEPAIRWITHSTAND_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
