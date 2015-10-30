/* pieceStand.h
 */

#ifndef EVAL_ML_PIECESTAND_H
#define EVAL_ML_PIECESTAND_H

#include "osl/eval/weights.h"
#include "osl/eval/minorPiece.h"
#include "osl/eval/evalStagePair.h"
#include "osl/numEffectState.h"
#include "osl/bits/king8Info.h"

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class PieceStand
      {
	static CArray<MultiInt, osl::Piece::SIZE> table;
      public:
	enum { DIM = osl::Piece::SIZE };
	PieceStand() { };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
	static MultiInt evalWithUpdate(const NumEffectState &state,
				  Move moved, MultiInt last_value)
	{
	  assert(moved.player()==P);
	  osl::Ptype captured = moved.capturePtype();
	  if (moved.isDrop())
	  {
	    const int count =
	      state.countPiecesOnStand(P, moved.ptype()) + 1;
	    const MultiInt value =
	      table[Ptype_Table.getIndexMin(moved.ptype()) + count - 1];
	    if(P==BLACK) 
	    	    return last_value - value;
	    else
	    	    return last_value + value;
	  }
	  else if (captured != PTYPE_EMPTY)
	  {
	    Ptype ptype = unpromote(captured);
	    const int count = state.countPiecesOnStand(P, ptype);
	    const MultiInt value = table[(Ptype_Table.getIndexMin(ptype) + count - 1)];
	    if(P==BLACK)
	      return last_value + value;
	    else
	      return last_value - value;
	  }
	  else
	    return last_value;
	}
      };

      class NonPawnPieceStand
      {
	static CArray<MultiInt, 21> table;
      public:
	enum { DIM = 21 };
	NonPawnPieceStand() { };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(int black_count, int white_count);
      };

      class NonPawnPieceStandCombination
      {
	friend class CanCheckNonPawnPieceStandCombination;
      public:
	enum { ONE_DIM = 5625, DIM = ONE_DIM * EvalStages};
	NonPawnPieceStandCombination() { };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state,
				   const CArray<bool, 2> &can_check);
	static MultiInt evalWithUpdate(
	  const NumEffectState &state,
	  Move moved, const MultiInt &last_value,
	  const CArray<bool, 2> &could_check,
	  const CArray<bool, 2> &can_check);
      private:
	static MultiInt sumUp(const CArray<int, 6> &indices,
			      const CArray<MultiInt, 5625> &values);
	static int index(int rook, int bishop, int gold, int silver,
			 int knight, int lance)
	{
	  return lance +
	    5 * (knight + 5 * (silver + 5 * (gold + 5 * (3 * bishop + rook))));
	}
	static CArray<MultiInt, 5625> table;
	static CArray<MultiInt, 5625> check_table;
      };

      class NonPawnPieceStandTurn
      {
      public:
	enum { ONE_DIM = 44, DIM = ONE_DIM * EvalStages };
	NonPawnPieceStandTurn() { };
	static void setUp(const Weights &weights);
	static void eval(const NumEffectState &state, MultiIntPair& out);
	template<Player P>
	static void evalWithUpdateBang(
	  const NumEffectState &state,
	  Move moved, MultiIntPair &last_value_and_out);
      private:
	static CArray<MultiInt, 44> table;
	static int index(Player player, Player turn, Ptype ptype, int count)
	{
	  return Ptype_Table.getIndexMin(ptype) - 18 + count +
	    (turn == player ? 22 : 0);
	}
      };
      class PieceStandY
      {
      private:
	static CArray<MultiInt, 360> y_attack_table;
	static CArray<MultiInt, 360> y_defense_table;
	static CArray<MultiInt, 9*7*19> y_attack_table_sum;
	static CArray<MultiInt, 9*7*19> y_defense_table_sum;
	static int index(Ptype ptype, Player player, Square king, int count)
	{
	  const int king_y = (player == BLACK ? king.y() : 10 - king.y());
	  return (king_y - 1) * 40 + Ptype_Table.getIndexMin(ptype) + count;
	}
	static int index(int i, Player player, Square king, int count)
	{
	  const int king_y = (player == BLACK ? king.y() : 10 - king.y());
	  return (king_y - 1) * 7*19 + i*19 + count;
	}
	static void updateResult(NumEffectState const& state, MultiInt &result,int i, Ptype ptype, CArray<Square,2> const&kings);
      public:
	enum { ONE_DIM = osl::Piece::SIZE * 9, DIM = ONE_DIM * 2*EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
	static MultiInt evalWithUpdate(
	  const NumEffectState &state, Move moved,
	  const MultiInt &last_value);
      };

      class CanCheckNonPawnPieceStandCombination
      {
      public:
	enum { ONE_DIM = 5625, DIM = ONE_DIM * EvalStages};
	static void setUp(const Weights &weights);
	template <Player Defense>
	static bool canCheck(const NumEffectState &state)
	{
	  const Player Attack=alt(Defense);
	  const King8Info king(state.Iking8Info(Defense));
	  return (king.dropCandidate() != 0 ||
		  king.hasMoveCandidate<Attack>(state) ||
		  KnightCheck::canCheck<Defense>(state));
	}
      };
      class PieceStandCombinationBoth
      {
      public:
	enum { ONE_DIM = 16384, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, 16384> table;
      };
    }
  }
}
#endif // EVAL_ML_PIECESTAND_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
