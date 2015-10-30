/* testEval.h
 */

#ifndef EVAL_ML_MINORPIECE_H
#define EVAL_ML_MINORPIECE_H

#include "osl/eval/midgame.h"
#include "osl/eval/weights.h"
#include "osl/eval/evalStagePair.h"
#include "osl/numEffectState.h"
#include <cstdlib>
namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class PawnDrop
      {
      public:
	enum { ONE_DIM = 9, DIM = ONE_DIM * 2};
	static void setUp(const Weights &weights,int stage);
      };

      class PawnDropY
      {
      public:
	enum { ONE_DIM = 81, DIM = ONE_DIM * 2};
	static void setUp(const Weights &weights,int stage);
      };

      class PawnDropBoth
      {
	friend class PawnDrop;
	friend class PawnDropY;
	friend class PawnDropX;
	friend class PawnDropPawnStand;
	friend class PawnDropPawnStandX;
	friend class PawnDropPawnStandY;
	friend class PawnDropNonDrop;
	friend class PawnStateKingRelative;
      private:
	enum { BOTH_ON_BOARD, SELF_ON_BOARD, OPP_ON_BOARD, BOTH_ON_STAND };
	static CArray<MultiInt, 9> attack_table, defense_table;
	static CArray<MultiInt, 81> attack_y_table, defense_y_table;
	static CArray<MultiInt, 90> x_table;
	static CArray<MultiInt, 18> stand_table;
	static CArray<MultiInt, 90> x_stand_table;
	static CArray<MultiInt, 162> y_stand_table;
	static CArray<MultiInt, 10> drop_non_drop_table;
	static CArray<MultiInt, 36> state_king_relative_table;
	template <Player Owner>
	static int indexY(const Piece king, int x)
	{
	  assert(Owner == king.owner());
	  const int king_y = (Owner == BLACK ?
			      king.square().y() : 10 - king.square().y());
	  return std::abs(x - king.square().x()) * 9 + king_y - 1;
	}
	static int index(const Square king, int x)
	{
	  return std::abs(x - king.x());
	}
	template <bool Attack>
	static int indexX(const Piece king, int x)
	{
	  const int king_x = king.square().x();
	  const int target_x = (king_x > 5 ? 10 - king_x : king_x);
	  if (king_x >= 6 || (king.owner() == WHITE && king_x == 5))
	    x = 10 - x;
	  return (x - 1) * 5 + target_x - 1 + (Attack ? 0 : 45);
	}
      public:
	static MultiInt value(
			 int attack_index, int defense_index, 
			 int attack_index_y, int defense_index_y,
			 int attack_index_x, int defense_index_x)
	{
	  return (attack_table[attack_index] +
		  defense_table[defense_index] +
		  attack_y_table[attack_index_y] +
		  defense_y_table[defense_index_y] +
		  x_table[attack_index_x] +
		  x_table[defense_index_x]);
	}
	static MultiInt standValue(
			      int attack_index, int defense_index, 
			      int attack_index_y, int defense_index_y,
			      int attack_index_x, int defense_index_x)
	{
	  return (stand_table[attack_index] +
		  stand_table[defense_index + 9] +
		  y_stand_table[attack_index_y] +
		  y_stand_table[defense_index_y + 81] +
		  x_stand_table[attack_index_x] +
		  x_stand_table[defense_index_x]);
	}
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
 	static MultiInt evalWithUpdate(const NumEffectState &state,
 					     Move moved,
 					     MultiInt &last_value)
	{
	  const Player altP=alt(P);
	  Ptype captured = moved.capturePtype();
	  if (moved.ptype() == KING ||
	      (moved.isDrop() && moved.ptype() == PAWN &&
	       !state.hasPieceOnStand<PAWN>(P)) ||
	      (captured != PTYPE_EMPTY &&
	       unpromote(captured) == PAWN &&
	       state.countPiecesOnStand<PAWN>(P) == 1))
	  {
	    return eval(state);
	  }

	  MultiInt result(last_value);
	  const CArray<Square, 2> king_bw = {{ state.kingSquare<BLACK>(), state.kingSquare<WHITE>() }};
	  const CArray<Square, 2> kings = {{ king_bw[playerToIndex(P)], king_bw[playerToIndex(alt(P))] }};
	  const CArray<Piece, 2> king_piece = {{ state.kingPiece(P),
						 state.kingPiece(alt(P)) }};
	  if (moved.oldPtype() == PAWN) 
	  {
	    if (moved.isDrop())
	    {
	      const int attack_index = index(kings[1], moved.to().x());
	      const int defense_index = index(kings[0], moved.to().x());
	      const int attack_index_x =
		indexX<true>(king_piece[1], moved.to().x());
	      const int defense_index_x =
		indexX<false>(king_piece[0], moved.to().x());

	      const int attack_index_y = indexY<altP>(king_piece[1], moved.to().x());
	      const int defense_index_y = indexY<P>(king_piece[0], moved.to().x());
	      const int index_x = (moved.to().x() > 5 ? 10 -
				   moved.to().x() : moved.to().x());
	      if (state.isPawnMaskSet<altP>(moved.to().x()))
	      {
		if (P == BLACK)
		{
		  result -= drop_non_drop_table[index_x - 1 + 5];
		  result += drop_non_drop_table[index_x - 1];
		  result -=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		  result +=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		}
		else
		{
		  result -= drop_non_drop_table[index_x - 1];
		  result += drop_non_drop_table[index_x - 1 + 5];
		  result -=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		  result +=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		}
		result +=
		  state_king_relative_table[std::abs(king_bw[BLACK].x() -
						     moved.to().x()) +
					    BOTH_ON_BOARD * 9];
		result -=
		  state_king_relative_table[std::abs(king_bw[WHITE].x() -
						     moved.to().x()) +
					    BOTH_ON_BOARD * 9];
	      }
	      else
	      {
		result -=
		  state_king_relative_table[std::abs(king_bw[BLACK].x() -
						     moved.to().x()) +
					    BOTH_ON_STAND * 9];
		result +=
		  state_king_relative_table[std::abs(king_bw[WHITE].x() -
						     moved.to().x()) +
					    BOTH_ON_STAND * 9];
		if (P == BLACK)
		{
		  result += drop_non_drop_table[index_x - 1];
		  result -= drop_non_drop_table[index_x - 1 + 5];
		  result +=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		  result -=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		}
		else
		{
		  result += drop_non_drop_table[index_x - 1 + 5];
		  result -= drop_non_drop_table[index_x - 1];
		  result +=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		  result -=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		}
	      }
	      if (P == BLACK)
	      {
		result -= value(attack_index, defense_index, attack_index_y, 
				defense_index_y, attack_index_x, defense_index_x);
		if (state.hasPieceOnStand<PAWN>(P))
		{
		  result -= standValue(attack_index, defense_index, attack_index_y, 
				       defense_index_y, attack_index_x, defense_index_x);
		}
	      }
	      else
	      {
		result += value(attack_index, defense_index, attack_index_y, 
				defense_index_y, attack_index_x, defense_index_x);
		if (state.hasPieceOnStand<PAWN>(P))
		{
		  result += standValue(attack_index, defense_index, attack_index_y, 
				       defense_index_y, attack_index_x, defense_index_x);
		}
	      }
	    }
	    if (moved.isPromotion())
	    {
	      const int attack_index = index(kings[1], moved.to().x());
	      const int defense_index = index(kings[0], moved.to().x());
	      const int attack_index_x =
		indexX<true>(king_piece[1], moved.to().x());
	      const int defense_index_x =
		indexX<false>(king_piece[0], moved.to().x());
	      const int attack_index_y = indexY<altP>(king_piece[1], moved.to().x());
	      const int defense_index_y = indexY<P>(king_piece[0], moved.to().x());
	      if (P == BLACK)
	      {
		result += value(attack_index, defense_index, attack_index_y, 
				defense_index_y, attack_index_x, defense_index_x);
		if (state.hasPieceOnStand<PAWN>(P))
		{
		  result += standValue(attack_index, defense_index, attack_index_y, 
				       defense_index_y, attack_index_x, defense_index_x);
		}
	      }
	      else
	      {
		result -= value(attack_index, defense_index, attack_index_y, 
				defense_index_y, attack_index_x, defense_index_x);
		if (state.hasPieceOnStand<PAWN>(P))
		{
		  result -= standValue(attack_index, defense_index, attack_index_y, 
				       defense_index_y, attack_index_x, defense_index_x);
		}
	      }
	      const int index_x = (moved.to().x() > 5 ? 10 -
				   moved.to().x() : moved.to().x());
	      if (state.isPawnMaskSet<altP>(moved.to().x()))
	      {
		result -=
		  state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					    BOTH_ON_BOARD * 9];
		result +=
		  state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					    BOTH_ON_BOARD * 9];
		if (P == BLACK)
		{
		  result += drop_non_drop_table[index_x - 1 + 5];
		  result -= drop_non_drop_table[index_x - 1];
		  result +=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		  result -=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		}
		else
		{
		  result += drop_non_drop_table[index_x - 1];
		  result -= drop_non_drop_table[index_x - 1 + 5];
		  result +=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		  result -=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		}
	      }
	      else
	      {
		result +=
		  state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					    BOTH_ON_STAND * 9];
		result -=
		  state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					    BOTH_ON_STAND * 9];
		if (captured == PAWN)
		{
		  result -=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      BOTH_ON_BOARD * 9];
		  result +=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      BOTH_ON_BOARD * 9];
		}
		else
		{
		  if (P == BLACK)
		  {
		    result -= drop_non_drop_table[index_x - 1];
		    result += drop_non_drop_table[index_x - 1 + 5];
		    result -=
		      state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
						SELF_ON_BOARD * 9];
		    result +=
		      state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
						OPP_ON_BOARD * 9];
		  }
		  else
		  {
		    result -= drop_non_drop_table[index_x - 1 + 5];
		    result += drop_non_drop_table[index_x - 1];
		    result -=
		      state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
						OPP_ON_BOARD * 9];
		    result +=
		      state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
						SELF_ON_BOARD * 9];
		  }
		}
	      }
	    }
	  }

	  if (captured == PAWN)
	  {
	    const int attack_index = index(kings[0], moved.to().x());
	    const int defense_index = index(kings[1], moved.to().x());
	    const int attack_index_x =
	      indexX<true>(king_piece[0], moved.to().x());
	    const int defense_index_x =
	      indexX<false>(king_piece[1], moved.to().x());
	    const int attack_index_y = indexY<P>(king_piece[0], moved.to().x());
	    const int defense_index_y = indexY<altP>(king_piece[1], moved.to().x());
	    if (P == BLACK)
	    {
	      result -= value(attack_index, defense_index, attack_index_y, 
			      defense_index_y, attack_index_x, defense_index_x);
	      if (state.hasPieceOnStand<PAWN>(alt(P)))
	      {
		result -= standValue(attack_index, defense_index, attack_index_y, 
				     defense_index_y, attack_index_x, defense_index_x);
	      }
	    }
	    else
	    {
	      result += value(attack_index, defense_index, attack_index_y, 
			      defense_index_y, attack_index_x, defense_index_x);
	      if (state.hasPieceOnStand<PAWN>(alt(moved.player())))
	      {
		result += standValue(attack_index, defense_index, attack_index_y, 
				     defense_index_y, attack_index_x, defense_index_x);
	      }
	    }
	    if (!(moved.ptype() == PPAWN && moved.isPromotion())) // promote is already handled above
	    {
	      const int index_x =
		(moved.to().x() > 5 ? 10 - moved.to().x() : moved.to().x());
	      if (state.isPawnMaskSet<P>(moved.to().x()))
	      {
		result -=
		  state_king_relative_table[std::abs(king_bw[BLACK].x() -
						     moved.to().x()) +
					    BOTH_ON_BOARD * 9];
		result +=
		  state_king_relative_table[std::abs(king_bw[WHITE].x() -
						     moved.to().x()) +
					    BOTH_ON_BOARD * 9];
		if (P == BLACK)
		{
		  result += drop_non_drop_table[index_x - 1];
		  result -= drop_non_drop_table[index_x - 1 + 5];
		  result +=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() -
						       moved.to().x()) +
					      SELF_ON_BOARD * 9];
		  result -=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() -
						       moved.to().x()) +
					      OPP_ON_BOARD * 9];
		}
		else
		{
		  result += drop_non_drop_table[index_x - 1 + 5];
		  result -= drop_non_drop_table[index_x - 1];
		  result +=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() -
						       moved.to().x()) +
					      OPP_ON_BOARD * 9];
		  result -=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() -
						       moved.to().x()) +
					      SELF_ON_BOARD * 9];
		}
	      }
	      else
	      {
		if (P == BLACK)
		{
		  result -= drop_non_drop_table[index_x - 1 + 5];
		  result += drop_non_drop_table[index_x - 1];
		  result -=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		  result +=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		}
		else
		{
		  result -= drop_non_drop_table[index_x - 1];
		  result += drop_non_drop_table[index_x - 1 + 5];
		  result -=
		    state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					      SELF_ON_BOARD * 9];
		  result +=
		    state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					      OPP_ON_BOARD * 9];
		}
		result +=
		  state_king_relative_table[std::abs(king_bw[BLACK].x() - moved.to().x()) +
					    BOTH_ON_STAND * 9];
		result -=
		  state_king_relative_table[std::abs(king_bw[WHITE].x() - moved.to().x()) +
					    BOTH_ON_STAND * 9];
	      }
	    }
	  }
	  return result;
	}
      };

      class PawnDropX
      {
      public:
	enum { ONE_DIM = 90, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class PawnDropPawnStand
      {
      public:
	enum { ONE_DIM = 18, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class PawnDropPawnStandX
      {
      public:
	enum { ONE_DIM = 90, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class PawnDropPawnStandY
      {
      public:
	enum { ONE_DIM = 162, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class PawnDropNonDrop
      {
      public:
	enum { ONE_DIM = 10, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class PawnStateKingRelative
      {
      public:
	enum { ONE_DIM = 36, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class NoPawnOnStand
      {
      public:
	enum { DIM = 1 };
      private:
	static MultiInt weight;
      public:
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state, int black_pawn_count)
	{
	  if (black_pawn_count > 9 && !state.hasPieceOnStand<PAWN>(WHITE))
	    return -weight;
	  else if (black_pawn_count < 9 && !state.hasPieceOnStand<PAWN>(BLACK))
	    return weight;

	  return MultiInt();
	}
      };

      struct PawnAdvanceUtil
      {
	static int index(Player P, Square pos)
	{
	  return (P == BLACK ? (pos.y() - 1) : (9 - pos.y()));
	}
	static bool cantAdvance(const NumEffectState &state, const Piece pawn)
	{
	  return cantAdvance(state, pawn.ptypeO(), pawn.square());
	}
	static bool cantAdvance(const NumEffectState &state,
				const PtypeO ptypeO, const Square position)
	{
	  assert(getPtype(ptypeO) == PAWN);
	  return state.pieceAt(Board_Table.nextSquare(getOwner(ptypeO),
							   position,
							   U)).isOnBoardByOwner(getOwner(ptypeO));
	}
      };
      struct PawnAdvanceAll : public PawnAdvanceUtil
      {
	template <osl::Player P> 
	static void adjust(int index, MultiInt& values);
	template<Player P>
	static void evalWithUpdateBang(const NumEffectState &state, Move moved,
				       MultiInt& last_value);
      };

      class PawnAdvance : public PawnAdvanceUtil
      {
      public:
	enum { DIM = 9 };
      private:
	static CArray<MultiInt, 9> table;
	friend struct PawnAdvanceAll;
      public:
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state);
      };

      class SilverFeatures
      {
      public:
	static MultiInt eval(const NumEffectState &state);
      protected:
	template <Player P>
	static int indexRetreat(Square pos)
	{
	  return (P == BLACK ? (pos.y() - 1) : (9 - pos.y()));
	}
	template<Player P>
	static bool canRetreat(const NumEffectState &state,
			       const Piece silver);
	template <Player P>
	static MultiInt evalOne(const NumEffectState &state,
				const Piece silver,
				const CArray<Square, 2> &kings)
	{
	  MultiInt result;
	  if (!canRetreat<P>(state,silver))
	  {
	    result += retreat_table[indexRetreat<P>(silver.square())];
	  }
	  const Square up =
	    silver.square() + DirectionPlayerTraits<U, P>::offset();
	  if (up.isOnBoard())
	  {
	    const Piece up_piece = state.pieceAt(up);
	    if (up_piece.isEmpty() &&
		(state.hasEffectByPtypeStrict<PAWN>(alt(P), up) ||
		 !state.isPawnMaskSet<alt(P)>(silver.square().x())))
	    {
	      const int x_diff =
		std::abs(kings[P].x() - silver.square().x());
	      const int y_diff = (P == BLACK ?
				  silver.square().y() - kings[P].y() :
				  kings[P].y() - silver.square().y());
	      result += head_table[x_diff + 9 * (y_diff + 8)];
	    }
	  }
	  return result;
	}
	static CArray<MultiInt, 153> head_table;
	static CArray<MultiInt, 9> retreat_table;
      };

      class SilverHeadPawnKingRelative : public SilverFeatures
      {
      public:
	enum { ONE_DIM = 153, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class SilverRetreat : public SilverFeatures
      {
      public:
	enum { DIM = 9 };
	static void setUp(const Weights &weights,int stage);
      };

      class GoldFeatures
      {
      public:
	static MultiInt eval(const NumEffectState &state);
      protected:
	template <Player P>
	static int indexRetreat(Square pos)
	{
	  return (P == BLACK ? (pos.y() - 1) : (9 - pos.y()));
	}
	static int indexSideX(Square pos)
	{
	  return (pos.x() > 5 ? 9 - pos.x() : pos.x() - 1);
	}
	template <Player P>
	static int indexSideY(Square pos)
	{
	  return (P == BLACK ? (pos.y() - 1) : (9 - pos.y())) + 5;
	}
	template<Player P>
	static bool canRetreat(const NumEffectState &state,
			       const Piece gold);
	template<Player P>
	static bool canMoveToSide(const NumEffectState &state,
				  const Piece gold)
	{
	  Square r = gold.square() + DirectionPlayerTraits<R, P>::offset();
	  Square l = gold.square() + DirectionPlayerTraits<L, P>::offset();
	  // check effect is from lesser pieces?
	  if ((!r.isOnBoard() ||
	       state.pieceAt(r).isOnBoardByOwner(gold.owner()) ||
	       state.hasEffectAt(alt(gold.owner()), r)) &&
	      (!l.isOnBoard() ||
	       state.pieceAt(l).isOnBoardByOwner(gold.owner()) ||
	       state.hasEffectAt(alt(gold.owner()), l)))
	  {
	    return false;
	  }
	  return true;
	}
	template <Player P>
	static MultiInt evalOne(const NumEffectState &state,
				const Piece gold,
				const CArray<Square, 2> &kings)
	{
	  assert(P==gold.owner());
	  MultiInt result;
	  if (!canRetreat<P>(state, gold))
	  {
	    result += retreat_table[indexRetreat<P>(gold.square())];
	  }
	  if (!canMoveToSide<P>(state, gold))
	  {
	    result += side_table[indexSideX(gold.square())] +
	      side_table[indexSideY<P>(gold.square())];
	  }
	  const Square uur = gold.square().neighbor<P,UUR>();
	  const Square uul = gold.square().neighbor<P,UUL>();
	  if ((state.pieceAt(uul).isEmpty() && !state.hasEffectAt(P, uul))
	      || (state.pieceAt(uur).isEmpty() && !state.hasEffectAt(P, uur)))
	  {
	    assert(state.kingSquare(gold.owner()) == kings[gold.owner()]);
	    const Square king = kings[P];
	    const int x_diff = std::abs(king.x() - gold.square().x());
	    const int y_diff = (P == BLACK ?
				gold.square().y() - king.y() :
				king.y() - gold.square().y());
	    result += knight_table[x_diff + 9 * (y_diff + 8)];
	  }
	  return result;
	}
	static CArray<MultiInt, 153> knight_table;
	static CArray<MultiInt, 9> retreat_table;
	static CArray<MultiInt, 14> side_table;
      };

      class GoldKnightKingRelative : public GoldFeatures
      {
      public:
	enum { ONE_DIM = 153, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class GoldRetreat : public GoldFeatures
      {
      public:
	enum { DIM = 9 };
	static void setUp(const Weights &weights,int stage);
      };

      class GoldSideMove : public GoldFeatures
      {
      public:
	enum { ONE_DIM = 14, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class KnightAdvance
      {
      public:
	enum { DIM = 9 };
      private:
	static CArray<MultiInt, 9> table;
	static int index(Player P, Square pos)
	{
	  return (P == BLACK ? (pos.y() - 1) : (9 - pos.y()));
	}
	template<Player P>
	static bool cantAdvance(const NumEffectState &state,
				const Piece knight);
      public:
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state);
      };

      class AllGold
      {
      public:
	enum { DIM = 1 };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(int black_major_count)
	{
	  if (black_major_count == 4)
	    return weight;
	  else if (black_major_count == 0)
	    return -weight;

	  return MultiInt();
	}
      private:
	static MultiInt weight;
      };

      class PtypeY
      {
      public:
	enum { DIM = PTYPE_SIZE * 9 };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
	static MultiInt evalWithUpdate(const NumEffectState &, Move moved,
				  MultiInt const& last_value);
      private:
	static CArray<MultiInt, 144> table;
	static int index(const Piece piece)
	{
	  return index(piece.owner(), piece.ptype(), piece.square());
	}
	static int index(const Player player, const Ptype ptype, const Square pos)
	{
	  const int y = (player == BLACK ? pos.y() : 10 - pos.y()) - 1;
	  return ptype * 9 + y;
	}
      };

      class PtypeX
      {
      public:
	enum { DIM = PTYPE_SIZE * 5 };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
	static MultiInt evalWithUpdate(const NumEffectState &, Move moved,
				  MultiInt const& last_value);
      private:
	static CArray<MultiInt, 80> table;
	static int index(const Piece piece)
	{
	  return index(piece.owner(), piece.ptype(), piece.square());
	}
	static int index(const Player, const Ptype ptype, const Square pos)
	{
	  const int x = (pos.x() > 5 ? 10 - pos.x() : pos.x()) - 1;
	  return ptype * 5 + x;
	}
      };

      class KnightCheck
      {
	friend class KnightCheckY;
      public:
	enum { DIM = 1 };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state);
	template <Player Defense>
	static bool canCheck(const NumEffectState &state)
	{
	  const Square king = state.kingSquare<Defense>();
	  const Player offense = alt(Defense);
	  const Square ul =
	    king + DirectionPlayerTraits<UUL, Defense>::offset();
	  const Square ur =
	    king + DirectionPlayerTraits<UUR, Defense>::offset();
	  if (ul.isOnBoard())
	  {
	    const Piece p = state.pieceAt(ul);
	    if (!state.hasEffectAt<Defense>(ul) &&
		((p.isEmpty() && state.hasPieceOnStand<KNIGHT>(offense)) ||
		 (!p.isOnBoardByOwner<offense>() &&
		  state.hasEffectByPtypeStrict<KNIGHT>(offense, ul))))
	      return true;
	  }
	  if (ur.isOnBoard())
	  {
	    const Piece p = state.pieceAt(ur);
	    if (!state.hasEffectAt<Defense>(ur) &&
		((p.isEmpty() && state.hasPieceOnStand<KNIGHT>(offense)) ||
		 (!p.isOnBoardByOwner<offense>() &&
		  state.hasEffectByPtypeStrict<KNIGHT>(offense, ur))))
	      return true;
	  }
	  return false;
	}
	static MultiInt value(int index_y) { return weight + y_table[index_y]; }
      private:
	static MultiInt weight;
	template <Player King>
	static int indexY(int y) 
	{
	  return (King == BLACK ? y - 1 : 9 - y) ;
	}
	static CArray<MultiInt, 9> y_table;
      };

      class KnightCheckY
      {
      public:
	enum { ONE_DIM = 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class KnightHead
      {
	friend class KnightHeadOppPiecePawnOnStand;
      public:
	enum { ONE_DIM = 9, DIM = ONE_DIM * EvalStages};
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, 9> table;
	static CArray<MultiInt, 144> opp_table;
      };

      class KnightHeadOppPiecePawnOnStand
      {
      public:
	enum { ONE_DIM = 9 * 16, DIM = ONE_DIM * EvalStages};
	static void setUp(const Weights &weights);
      private:
      };

      class PawnPtypeOPtypeO
      {
	friend class PawnPtypeOPtypeOY;
      public:
	enum { ONE_DIM = 1024, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
	static MultiInt evalWithUpdate(const NumEffectState &state,
					Move moved,
					const CArray2d<int, 2, 9> &pawns,
					const MultiInt &last_value);
      private:
	static int index(Player P, PtypeO up, PtypeO up_up)
	{
	  if (P == WHITE)
	  {
	    up = altIfPiece(up);
	    up_up = altIfPiece(up_up);
	  }
	  return (up - PTYPEO_MIN) * 32 + (up_up - PTYPEO_MIN);
	}
	static int indexY(Player P, PtypeO up, PtypeO up_up, int y)
	{
	  const int y_index = (P == BLACK ? y - 1 : 9 - y);
	  return index(P, up, up_up) + 1024 * y_index;
	}
	static CArray<MultiInt, 1024> table;
	static CArray<MultiInt, 9216> y_table;
      };

      class PromotedMinorPieces
      {
	friend class PromotedMinorPiecesY;
      public:
	enum { ONE_DIM = 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
	static MultiInt evalWithUpdate(
	  const NumEffectState &state,
	  Move moved,
	  const MultiInt &last_values);
	template <int Sign>
	static void adjust(int index, int index_attack, int index_defense,
			   MultiInt &result);
      private:
	template <Player P>
	static void evalOne(const NumEffectState &state,
			    const PieceMask promoted,
			    MultiInt &result);
	template <bool attack, Player owner>
	static int indexY(const Square king, int x_diff)
	{
	  const int y = (owner == BLACK ? king.y() : 10 - king.y());
	  return x_diff + (y - 1) * 9 + (attack ? 0 : 81);
	}
	static CArray<MultiInt, 9> table;
	static CArray<MultiInt, 162> y_table;
      };
      class NonPawnAttacked
      {
	friend class NonPawnAttackedKingRelative;
      public:
	enum { ONE_DIM = 64, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static void eval(const NumEffectState &state, MultiIntPair& out);
	template<Player P>
	static void evalWithUpdateBang(
	  const NumEffectState &state,
	  Move moved,
	  const CArray<PieceMask, 2> &effected_mask,
	  MultiIntPair &last_value_and_out);
	template <int Sign>
	static void adjust(int black_turn_king_attack,
			   int black_turn_king_defense,
			   int white_turn_king_attack,
			   int white_turn_king_defense,
			   MultiIntPair &result);
      private:
	static int index(bool same_turn, bool has_support, Ptype ptype)
	{
	  return ptype + (same_turn ? 0 : PTYPE_SIZE) +
	    (has_support ? 0 : PTYPE_SIZE * 2);
	}
	template <bool Attack>
	static int indexK(Square king, bool same_turn, bool has_support,
			  Square position, Player owner, Ptype ptype)
	{
	  const int x_diff = std::abs(position.x() - king.x());
	  const int y_diff = (owner == BLACK ?
			      position.y() - king.y() :
			      king.y() - position.y());
	  return ((ptype + (same_turn ? 0 : PTYPE_SIZE) +
		   (has_support ? 0 : PTYPE_SIZE * 2)) * 9 + x_diff) * 17 +
	    y_diff + 8 + (Attack ? 0 : 9792);
	}
	template <bool Attack>
	static int indexK(Square king, bool same_turn, bool has_support,
			  Piece piece)
	{
	  return indexK<Attack>(king, same_turn, has_support,
				piece.square(), piece.owner(),
				piece.ptype());
	}

	template <Player Attacked>
	static void updateEffectChanged(
	  const NumEffectState &state,
	  const CArray<PieceMask, 2> &effected_mask,
	  const CArray<PieceMask, 2> &new_mask,
	  int moved_piece_number,
	  MultiIntPair &result)
	{
	  CArray<Square, 2> kings = {{ state.kingSquare<BLACK>(),
					 state.kingSquare<WHITE>() }};
	  // old without, new with
	  PieceMask black_old = (~effected_mask[alt(Attacked)]) & new_mask[alt(Attacked)] & state.piecesOnBoard(Attacked);
	  black_old.reset(moved_piece_number);
	  while (black_old.any())
	  {
	    const Piece piece = state.pieceOf(black_old.takeOneBit());
	    const bool has_support =
	      new_mask[Attacked].test(piece.number());
	    const int index_king_black_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == BLACK, has_support, piece);
	    const int index_king_white_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == WHITE, has_support, piece);
	    const int index_king_black_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == BLACK, has_support, piece);
	    const int index_king_white_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == WHITE, has_support, piece);
	    adjust<Attacked == BLACK ? 1 : -1>(
	      index_king_black_turn_attack, index_king_black_turn_defense,
	      index_king_white_turn_attack, index_king_white_turn_defense,
	      result);
	  }

	  // old with, new without
	  PieceMask black_new = effected_mask[alt(Attacked)] & (~new_mask[alt(Attacked)]) & state.piecesOnBoard(Attacked);
	  black_new.reset(moved_piece_number);
	  while (black_new.any())
	  {
	    const Piece piece = state.pieceOf(black_new.takeOneBit());
	    const bool has_support =
	      effected_mask[Attacked].test(piece.number());
	    const int index_king_black_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == BLACK, has_support, piece);
	    const int index_king_white_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == WHITE, has_support, piece);
	    const int index_king_black_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == BLACK, has_support, piece);
	    const int index_king_white_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == WHITE, has_support, piece);
	    adjust<Attacked == BLACK ? -1 : 1>(
	      index_king_black_turn_attack, index_king_black_turn_defense,
	      index_king_white_turn_attack, index_king_white_turn_defense,
	      result);
	  }
	  // old with, new with, self with, self without
	  PieceMask black_self_old = effected_mask[alt(Attacked)] & new_mask[alt(Attacked)] &
	    effected_mask[Attacked] & (~new_mask[Attacked]) & state.piecesOnBoard(Attacked);
	  black_self_old.reset(moved_piece_number);
	  while (black_self_old.any())
	  {
	    const Piece piece = state.pieceOf(black_self_old.takeOneBit());
	    const int index_king_black_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == BLACK, false, piece);
	    const int index_king_white_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == WHITE, false, piece);
	    const int index_king_black_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == BLACK, false, piece);
	    const int index_king_white_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == WHITE, false, piece);
	    const int index_king_black_turn_attack_old =
	      indexK<true>(kings[alt(Attacked)], Attacked == BLACK, true, piece);
	    const int index_king_white_turn_attack_old =
	      indexK<true>(kings[alt(Attacked)], Attacked == WHITE, true, piece);
	    const int index_king_black_turn_defense_old =
	      indexK<false>(kings[Attacked], Attacked == BLACK, true, piece);
	    const int index_king_white_turn_defense_old =
	      indexK<false>(kings[Attacked], Attacked == WHITE, true, piece);
	    adjust<Attacked == BLACK ? -1 : 1>(
	      index_king_black_turn_attack_old, index_king_black_turn_defense_old,
	      index_king_white_turn_attack_old, index_king_white_turn_defense_old,
	      result);
	    adjust<Attacked == BLACK ? 1 : -1>(
	      index_king_black_turn_attack, index_king_black_turn_defense,
	      index_king_white_turn_attack, index_king_white_turn_defense,
	      result);
	  }
	  // old with, new with, self without, self with
	  PieceMask black_self_new = effected_mask[alt(Attacked)] & new_mask[alt(Attacked)] &
	    (~effected_mask[Attacked]) & new_mask[Attacked] & state.piecesOnBoard(Attacked);
	  black_self_new.reset(moved_piece_number);
	  while (black_self_new.any())
	  {
	    const Piece piece = state.pieceOf(black_self_new.takeOneBit());
	    const int index_king_black_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == BLACK, true, piece);
	    const int index_king_white_turn_attack =
	      indexK<true>(kings[alt(Attacked)], Attacked == WHITE, true, piece);
	    const int index_king_black_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == BLACK, true, piece);
	    const int index_king_white_turn_defense =
	      indexK<false>(kings[Attacked], Attacked == WHITE, true, piece);
	    const int index_king_black_turn_attack_old =
	      indexK<true>(kings[alt(Attacked)], Attacked == BLACK, false, piece);
	    const int index_king_white_turn_attack_old =
	      indexK<true>(kings[alt(Attacked)], Attacked == WHITE, false, piece);
	    const int index_king_black_turn_defense_old =
	      indexK<false>(kings[Attacked], Attacked == BLACK, false, piece);
	    const int index_king_white_turn_defense_old =
	      indexK<false>(kings[Attacked], Attacked == WHITE, false, piece);

	    adjust<Attacked == BLACK ? -1 : 1>(
	      index_king_black_turn_attack_old, index_king_black_turn_defense_old,
	      index_king_white_turn_attack_old, index_king_white_turn_defense_old,
	      result);
	    adjust<Attacked == BLACK ? 1 : -1>(
	      index_king_black_turn_attack, index_king_black_turn_defense,
	      index_king_white_turn_attack, index_king_white_turn_defense,
	      result);
	  }
	}
	static CArray<MultiInt, 64> table;
	static CArray<MultiInt, 19584> king_table;
      };

      class NonPawnAttackedKingRelative
      {
      public:
	enum { ONE_DIM = 19584, DIM = ONE_DIM * EvalStages};
	static void setUp(const Weights &weights);
      };

      class PromotedMinorPiecesY
      {
      public:
	enum { ONE_DIM = 162, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class PawnPtypeOPtypeOY
      {
      public:
	enum { ONE_DIM = 9216, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class NonPawnAttackedPtype
      {
      public:
	enum { ONE_DIM = 1024, DIM = ONE_DIM * EvalStages};
	static void setUp(const Weights &weights);
	static void eval(const NumEffectState &state,
			 CArray<PieceMask, 40> &attacked_mask,
			 MultiIntPair& out);
	template<Player P>
	static void evalWithUpdateBang(
	  const NumEffectState &state,
	  Move moved,
	  const CArray<PieceMask, 2> &effected_mask,
	  CArray<PieceMask, 40> &attacked_mask,
	  MultiIntPair &last_value_and_out);
      private:
	static int index(bool same_turn, bool has_support, Ptype ptype,
			 Ptype attack_ptype)
	{
	  return (ptype + (same_turn ? 0 : PTYPE_SIZE) +
		  (has_support ? 0 : PTYPE_SIZE * 2)) * 16 + attack_ptype;
	}
	template <int Sign>
	static void adjust(int black, int white, MultiIntPair &result)
	{
	  if(Sign>0){
	    result[BLACK] += table[black];
	    result[WHITE] += table[white];
	  }
	  else{
	    result[BLACK] -= table[black];
	    result[WHITE] -= table[white];
	  }
	}
	template <bool Plus>
	static void evalOnePiece(const Player player,
				 const Ptype ptype,
				 const Ptype attack_ptype,
				 bool with_support,
				 MultiIntPair &result)
	{
	  const int index_black_turn = index(BLACK == player, with_support,
					     ptype, attack_ptype);
	  const int index_white_turn = index(WHITE == player, with_support,
					     ptype, attack_ptype);
	  if (Plus)
	    adjust<1>(index_black_turn, index_white_turn, result);
	  else
	    adjust<-1>(index_black_turn, index_white_turn, result);
	}
	template <Player P>
	static void updateChanged(const NumEffectState &state,
				  const Piece p,
				  Move moved,
				  int captured_number,
				  const CArray<PieceMask, 2> &effected_mask,
				  const CArray<PieceMask, 2> &new_mask,
				  CArray<PieceMask, 40> &attacked_mask,
				  MultiIntPair &result)
	{
	  // old without, new with
	  PieceMask old = (~effected_mask[alt(P)]) & new_mask[alt(P)] & state.piecesOnBoard(P);
	  old.reset(p.number());
	  while (old.any())
	  {
	    const Piece piece = state.pieceOf(old.takeOneBit());
	    const bool has_support =
	      new_mask[P].test(piece.number());
	    PieceMask attacking =
	      state.effectSetAt(piece.square()) &
	      state.piecesOnBoard(alt(P));
	    attacked_mask[piece.number()] = attacking;
	    while (attacking.any())
	    {
	      const Piece attack = state.pieceOf(attacking.takeOneBit());
	      evalOnePiece<P == BLACK>(P, piece.ptype(), attack.ptype(),
				       has_support, result);
	    }
	  }
	  // old with, new without
	  PieceMask new_without = effected_mask[alt(P)] & (~new_mask[alt(P)]) & state.piecesOnBoard(P);
	  new_without.reset(p.number());
	  while (new_without.any())
	  {
	    const Piece piece = state.pieceOf(new_without.takeOneBit());
	    const bool has_support =
	      effected_mask[P].test(piece.number());
	    PieceMask attacking = attacked_mask[piece.number()];
	    if (moved.isPromotion() && attacking.test(p.number()))
	    {
	      evalOnePiece<P != BLACK>(P, piece.ptype(), moved.oldPtype(),
				       has_support, result);
	      attacking.reset(p.number());
	    }
	    if (captured_number != -1 && attacking.test(captured_number))
	    {
	      evalOnePiece<P != BLACK>(P, piece.ptype(), moved.capturePtype(),
				       has_support, result);
	      attacking.reset(captured_number);
	    }
	    while (attacking.any())
	    {
	      const Piece attack = state.pieceOf(attacking.takeOneBit());
	      evalOnePiece<P != BLACK>(P, piece.ptype(), attack.ptype(),
				       has_support, result);
	    }
	  }
	  // old with, new with, self with, self without
	  PieceMask self_old = effected_mask[alt(P)] &
	    new_mask[alt(P)] &
	    effected_mask[P] & (~new_mask[P]) & state.piecesOnBoard(P);
	  self_old.reset(p.number());
	  while (self_old.any())
	  {
	    const Piece piece = state.pieceOf(self_old.takeOneBit());
	    PieceMask old_attacking = attacked_mask[piece.number()];
	    if (moved.isPromotion() && old_attacking.test(p.number()))
	    {
	      evalOnePiece<P != BLACK>(P, piece.ptype(), moved.oldPtype(),
				       true, result);
	      old_attacking.reset(p.number());
	    }
	    if (captured_number != -1 && old_attacking.test(captured_number))
	    {
	      evalOnePiece<P != BLACK>(P, piece.ptype(), moved.capturePtype(),
				       true, result);
	      old_attacking.reset(captured_number);
	    }
	    while (old_attacking.any())
	    {
	      const Piece attack = state.pieceOf(old_attacking.takeOneBit());
	      evalOnePiece<P != BLACK>(P, piece.ptype(), attack.ptype(),
				       true, result);
	    }
	    PieceMask new_attacking = state.effectSetAt(piece.square())
	      & state.piecesOnBoard(alt(P));
	    attacked_mask[piece.number()] = new_attacking;
	    while (new_attacking.any())
	    {
	      const Piece attack = state.pieceOf(new_attacking.takeOneBit());
	      evalOnePiece<P == BLACK>(P, piece.ptype(), attack.ptype(),
				       false, result);
	    }
	  }
	  // old with, new with, self without, self with
	  PieceMask self_new_with = effected_mask[alt(P)] &
	    new_mask[alt(P)] &
	    (~effected_mask[P]) & new_mask[P] & state.piecesOnBoard(P);
	  self_new_with.reset(p.number());
	  while (self_new_with.any())
	  {
	    const Piece piece = state.pieceOf(self_new_with.takeOneBit());
	    PieceMask old_attacking = attacked_mask[piece.number()];
	    if (moved.isPromotion() && old_attacking.test(p.number()))
	    {
	      evalOnePiece<P != BLACK>(P, piece.ptype(), moved.oldPtype(),
				       false, result);
	      old_attacking.reset(p.number());
	    }
	    if (captured_number != -1 && old_attacking.test(captured_number))
	    {
	      evalOnePiece<P != BLACK>(P, piece.ptype(), moved.capturePtype(),
				       false, result);
	      old_attacking.reset(captured_number);
	    }
	    while (old_attacking.any())
	    {
	      const Piece attack = state.pieceOf(old_attacking.takeOneBit());
	      evalOnePiece<P != BLACK>(P, piece.ptype(), attack.ptype(),
				       false, result);
	    }
	    PieceMask new_attacking = state.effectSetAt(piece.square())
	      & state.piecesOnBoard(alt(P));
	    attacked_mask[piece.number()] = new_attacking;
	    while (new_attacking.any())
	    {
	      const Piece attack = state.pieceOf(new_attacking.takeOneBit());
	      evalOnePiece<P == BLACK>(P, piece.ptype(), attack.ptype(),
				       true, result);
	    }
	  }
	  // old with, new with, support unchanged, attack changed
	  PieceMask effected = effected_mask[P];
	  effected ^= new_mask[P];
	  effected = ~effected;
	  PieceMask attack_changed = effected_mask[alt(P)] &
	    new_mask[alt(P)] &
	    effected & state.piecesOnBoard(P) &
	    state.effectedChanged(alt(P));
	  attack_changed.reset(p.number());
	  while (attack_changed.any())
	  {
	    const Piece attacked = state.pieceOf(attack_changed.takeOneBit());
	    PieceMask attack_old_mask = attacked_mask[attacked.number()];
	    PieceMask attack_new_mask = state.effectSetAt(attacked.square()) & state.piecesOnBoard(alt(P));
	    if (captured_number != -1 &&
		attack_old_mask.test(captured_number))
	    {
	      evalOnePiece<P != BLACK>(P, attacked.ptype(),
				       moved.capturePtype(),
				       new_mask[P].test(attacked.number()),
				       result);
	      attack_old_mask.reset(captured_number);
	    }
	    if (moved.isPromotion() &&
		attack_old_mask.test(p.number()))
	    {
	      evalOnePiece<P != BLACK>(P, attacked.ptype(),
				       moved.oldPtype(),
			       new_mask[P].test(attacked.number()),
				       result);
	      attack_old_mask.reset(p.number());
	    }
	    if (moved.isPromotion() &&
		attack_new_mask.test(p.number()))
	    {
	      evalOnePiece<P == BLACK>(P, attacked.ptype(),
				       moved.ptype(),
				       new_mask[P].test(attacked.number()),
				       result);
	      attack_new_mask.reset(p.number());
	    }
	    PieceMask gone = attack_old_mask & (~attack_new_mask);
	    while (gone.any())
	    {
	      const Piece attacking = state.pieceOf(gone.takeOneBit());
	      evalOnePiece<P != BLACK>(P, attacked.ptype(),
				       attacking.ptype(),
				       effected_mask[P].test(attacked.number()),
				       result);
	    }
	    PieceMask added = (~attack_old_mask) & attack_new_mask;
	    while (added.any())
	    {
	      const Piece attacking = state.pieceOf(added.takeOneBit());
	      evalOnePiece<P == BLACK>(P, attacked.ptype(),
				       attacking.ptype(),
				       new_mask[P].test(attacked.number()),
				       result);
	    }

	    attacked_mask[attacked.number()] = state.effectSetAt(attacked.square()) & state.piecesOnBoard(alt(P));
	  }
	}

	static CArray<MultiInt, 1024> table;
      };

      class NonPawnAttackedPtypePair
      {
      public:
	enum {
	  ONE_DIM = (PTYPE_SIZE * 2 * PTYPE_SIZE)*(PTYPE_SIZE * 2 * PTYPE_SIZE),
	  DIM = ONE_DIM * EvalStages,     
	};
	static void setUp(const Weights &weights);
	template <Player Owner>
	static MultiInt evalOne(const NumEffectState &state);	
	static MultiInt eval(const NumEffectState &state);	
	static int index1(const NumEffectState &state, Piece piece)
	{
	  const Ptype attack_ptype
	    = state.findCheapAttack(alt(piece.owner()), piece.square()).ptype();
	  const bool has_support = state.hasEffectAt(piece.owner(),
						     piece.square());
	  return (piece.ptype() + 
		  (has_support ? 0 : PTYPE_SIZE)) * PTYPE_SIZE + attack_ptype;
	}
	static int index2(int i0, int i1) 
	{
	  return i0 * PTYPE_SIZE * 2 * PTYPE_SIZE + i1;
	}	  
	static CArray<MultiInt, ONE_DIM> table;
      };      

      class PtypeCount
      {
	friend class PtypeCountXY;
	friend class PtypeCountXYAttack;
      public:
	enum { ONE_DIM = 160, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	template<osl::Player P,osl::Ptype T>
	static MultiInt evalPlayerPtype(const CArray2d<int, 2, PTYPE_SIZE> &ptype_count,
					const CArray2d<int, 2, PTYPE_SIZE> &ptype_board_count,
					const osl::CArray<int,2> &kings_x,
					const osl::CArray<int,2> &kings_y);
	static void eval(const NumEffectState &state,
			 const CArray2d<int, 2, PTYPE_SIZE> &ptype_count,
			 const CArray2d<int, 2, PTYPE_SIZE> &ptype_board_count,
			 MultiInt &out);
	template<Player P>
	static void evalWithUpdateBang(
	  const NumEffectState &state,
	  Move last_move,
	  CArray2d<int, 2, PTYPE_SIZE> &ptype_count,
	  CArray2d<int, 2, PTYPE_SIZE> &ptype_board_count,
	  MultiInt &last_value_and_out,unsigned int &ptypeo_mask);
      private:
	static int indexCount(Ptype ptype, int count)
	{
	  return Ptype_Table.getIndexMin(unpromote(ptype)) +
	    (isPromoted(ptype) ? 40 : 0) +
	    count - 1;
	}
	static int indexBoardCount(Ptype ptype, int count)
	{
	  return Ptype_Table.getIndexMin(unpromote(ptype)) +
	    (isPromoted(ptype) ? 40 : 0) + 80 +
	    count - 1;
	}
	static int indexCountX(Ptype ptype, int count, int x)
	{
	  return x - 1 + 5 *
	    (Ptype_Table.getIndexMin(unpromote(ptype)) +
	     (isPromoted(ptype) ? 40 : 0) +
	     count - 1);
	}
	static int indexCountY(Ptype ptype, int count, int y)
	{
	  return y - 1 + 9 *
	    (Ptype_Table.getIndexMin(unpromote(ptype)) +
	     (isPromoted(ptype) ? 40 : 0) +
	     count - 1) + 800;
	}
	static int indexBoardCountX(Ptype ptype, int count, int x)
	{
	  return x - 1 + 5 *
	    (Ptype_Table.getIndexMin(unpromote(ptype)) +
	     (isPromoted(ptype) ? 40 : 0) +
	     count - 1) + 400;
	}
	static int indexBoardCountY(Ptype ptype, int count, int y)
	{
	  return y - 1 + 9 *
	    (Ptype_Table.getIndexMin(unpromote(ptype)) +
	     (isPromoted(ptype) ? 40 : 0) +
	     count - 1) + 720 + 800;
	}
	template<Ptype T>
	static int indexCount(int count)
	{
	  return  PtypeTraits<T>::indexMin+ (isPromoted(T) ? 40 : 0)  +
	    count - 1;
	}
	template<Ptype T>
	static int indexBoardCount(int count)
	{
	  return PtypeTraits<T>::indexMin+(isPromoted(T) ? 40 : 0)+ 80 +
	    count - 1;
	}
	template<Ptype T>
	static int indexCountX(int count, int x)
	{
	  return x - 1 + 5 *
	    (PtypeTraits<T>::indexMin+(isPromoted(T) ? 40 : 0) +
	     count - 1);
	}
	template<Ptype T>
	static int indexCountY(int count, int y)
	{
	  return y - 1 + 9 *
	    (PtypeTraits<T>::indexMin+(isPromoted(T) ? 40 : 0) +
	     count - 1) + 800;
	}
	template<Ptype T>
	static int indexBoardCountX(int count, int x)
	{
	  return x - 1 + 5 *
	    (PtypeTraits<T>::indexMin+(isPromoted(T) ? 40 : 0) +
	   count - 1) + 400;
	}
	template<Ptype T>
	static int indexBoardCountY(int count, int y)
	{
	  return y - 1 + 9 *
	    (PtypeTraits<T>::indexMin+(isPromoted(T) ? 40 : 0) +
	     count - 1) + 720 + 800;
	}
	static MultiInt valueAll(Ptype ptype, int count, int my_king_x, int my_king_y, int op_king_x, int op_king_y)
	{
	  assert(count>0);
	  return 
	    xy_table_diff[indexCountX(ptype, count, my_king_x)]+
	    xy_table_diff[indexCountY(ptype, count, my_king_y)]+
	    xy_attack_table_diff[indexCountX(ptype,count, op_king_x)]+
	    xy_attack_table_diff[indexCountY(ptype, count, op_king_y)];
	}
	static MultiInt valueBoardAll(Ptype ptype, int count, int my_king_x, int my_king_y, int op_king_x, int op_king_y)
	{
	  assert(count>0);
	  return 
	    xy_table_diff[indexBoardCountX(ptype, count, my_king_x)]+
	    xy_table_diff[indexBoardCountY(ptype, count, my_king_y)]+
	    xy_attack_table_diff[indexBoardCountX(ptype,count, op_king_x)]+
	    xy_attack_table_diff[indexBoardCountY(ptype, count, op_king_y)];
	}
	static CArray<MultiInt, 160> table;
	static CArray<MultiInt, 2240> xy_table;
	static CArray<MultiInt, 2240> xy_attack_table;
	static CArray<MultiInt, 2240> xy_table_diff;
	static CArray<MultiInt, 2240> xy_attack_table_diff;
      };

      class PtypeCountXY
      {
      public:
	enum { ONE_DIM = 2240, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class PtypeCountXYAttack
      {
      public:
	enum { ONE_DIM = 2240, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class LanceEffectPieceKingRelative
      {
      public:
	enum { ONE_DIM = 9792, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static int index(Player p, Square pos, Square king,
			 PtypeO ptypeO, bool attack)
	{
	  const int y_diff = (p == BLACK ? king.y() - pos.y() : pos.y() - king.y());
	  const int x_diff = std::abs(king.x() - pos.x());
	  if (p == WHITE)
	  {
	    ptypeO = alt(ptypeO);
	  }
	  return y_diff + 8 + x_diff * 17 + (ptypeO - PTYPEO_MIN) * 17 * 9 +
	    (attack ? 0 : 4896);
	}
	static CArray<MultiInt, 9792> table;
      };

      class PtypeYPawnY
      {
      public:
	enum { ONE_DIM = 1440, DIM = ONE_DIM * EvalStages };
	static MultiInt eval(const NumEffectState &state,
			     const CArray2d<int, 2, 9> &pawns);
	template<Player P>
	static void evalWithUpdateBang(const NumEffectState &state,
				       Move moved,
				       const CArray2d<int, 2, 9> &pawns,
				       MultiInt& last_value);
	static void setUp(const Weights &weights);
      private:
	static int index(Player player, Ptype ptype, int y, int pawn_y)
	{
	  if (player == WHITE)
	  {
	    y = 10 - y;
	    pawn_y = (10 - pawn_y) % 10;
	  }
	  return pawn_y + 10 * (y - 1 + 9 * ptype);
	}
	static CArray<MultiInt, 1440> table;
      };

      class GoldAndSilverNearKing
      {
	friend class GoldAndSilverNearKingCombination;
      public:
	enum { ONE_DIM = 1215, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state,
			      const CArray2d<int, 2, 3> &gs_count);
      private:
	template <Player Defense>
	static int index(const Square king, int distance0, int count)
	{
	  int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
	  int king_y = (Defense == WHITE ? 10 - king.y() : king.y());
	  return king_x - 1 + 5 * (king_y - 1+ 9 * (distance0 + 3 * count));
	}
	template <Player P>
	static MultiInt evalOne(const NumEffectState &state,
				 const CArray2d<int, 2, 3> &gs_count);
	template <Player Defense>
	static int indexCombination(const Square king, int count0,
			     int count1, int count2)
	{
	  int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
	  int king_y = (Defense == WHITE ? 10 - king.y() : king.y());
	  return king_x + 5 * (king_y + 9 * (std::min(5,count0) + 6 *
					     (std::min(5,count1) + 6 * std::min(5,count2))));
	}
	static CArray<MultiInt, 1215> table;
	static CArray<MultiInt, 9720> combination_table;
      };

      class GoldAndSilverNearKingCombination
      {
      public:
	enum { ONE_DIM = 9720, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      private:
      };

      class PtypeCombination
      {
      public:
	enum { ONE_DIM = 8192, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(unsigned int ptypeo_mask);
      private:
	template <Player P>
	static MultiInt evalOne(unsigned int ptypeo_mask)
	{
	  int index = 0;
	  if (P==BLACK) index=((ptypeo_mask>>19)&0x1fc0)|((ptypeo_mask>>18)&0x3f);
	  else index=((ptypeo_mask>>3)&0x1fc0)|((ptypeo_mask>>2)&0x3f);
	  if (P == BLACK)
	    return table[index];
	  else
	    return -table[index];
	}
	static CArray<MultiInt, 8192> table;
      };
      class SilverFork
      {
	static std::pair<int,int> matchRook(const NumEffectState& state, Piece rook,
					    const CArray<bool,2>& has_silver,
					    Square& silver_drop);
	static std::pair<int,int> matchGold(const NumEffectState& state, Piece gold, 
					    const CArray<bool,2>& has_silver,
					    Square& silver_drop);
      public:
	enum { ONE_DIM = 5*2, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiIntPair eval(const NumEffectState& state,
				 CArray<std::pair<Square,int>,2>& silver_drop);
	static CArray<MultiInt, ONE_DIM> table;
      };
      class BishopRookFork
      {
      public:
	enum { 
	  DROP_DIM = PTYPE_SIZE*PTYPE_SIZE, 
	  ONE_DIM = 2*DROP_DIM*2, DIM = ONE_DIM * EvalStages 
	};
	static const Square isBishopForkSquare(const NumEffectState& state, Player defense, const Square a, const Square b, bool maybe_empty=false);
	static const Square isRookForkSquare(const NumEffectState& state, Player defense, const Square a, const Square b);
	static int bishopIndex(Ptype a, Ptype b) { return a * PTYPE_SIZE + b; }
	static int rookIndex(Ptype a, Ptype b) { return bishopIndex(a,b) + DROP_DIM; }
	static void setUp(const Weights &weights);
	template <Player Defense>
	static MultiIntPair evalOne(const NumEffectState& state, const PieceVector& target,
				    std::pair<Square,int>& bishop_drop,
				    std::pair<Square,int>& rook_drop);
	static MultiIntPair eval(const NumEffectState& state,
				 CArray<std::pair<Square,int>,2>& bishop_drop,
				 CArray<std::pair<Square,int>,2>& rook_drop);
	static CArray<MultiInt, ONE_DIM> table;
      private:
	static const Square findDropInLine
	(const NumEffectState& state, Player defense, 
	 const Square a, const Square b, Piece king);
	static bool testCenter(const NumEffectState& state, Player defense, 
			       const Square a, const Square b, Piece king,
			       Square center, bool maybe_empty=false);
      };

      class KnightFork
      {
      public:
	enum { 
	  DROP_DIM = PTYPE_SIZE*PTYPE_SIZE, ONE_DIM = DROP_DIM*2*2, 
	  DIM = ONE_DIM * EvalStages
	};
	static void setUp(const Weights &weights);
	template <Player Defense>
	static MultiIntPair evalOne(const NumEffectState& state,
				    bool has_knight,
				    BoardMask& knight_fork_squares, 
				    std::pair<Square,int>& knight_drop);
	static MultiIntPair eval(const NumEffectState& state, 
				 CArray<BoardMask,2>& knight_fork_squares,
				 CArray<std::pair<Square,int>,2>& knight_drop);
	template <Player P>
	static MultiIntPair evalWithUpdate(const NumEffectState& state, 
					   Move moved,
					   CArray<BoardMask,2>& knight_fork_squares,
					   CArray<std::pair<Square,int>,2>& knight_drop);
	static CArray<MultiInt, ONE_DIM> table;

	static bool isForkSquare(const NumEffectState& state, Player defense, 
				   int y, int x0, int x1);
	static int index(Ptype a, Ptype b)
	{
	  return a * PTYPE_SIZE + b;
	}
	static bool isTarget(Ptype ptype) 
	{
	  ptype = unpromote(ptype);
	  return ptype != PAWN && ptype != LANCE && ptype != KNIGHT;
	}
      private:
	template <Player P, Player Defense>
	static void updateSquares
	(const NumEffectState& state, Move moved,
	 BoardMask& knight_fork_squares);
	template <osl::Player Defense>
	static MultiIntPair accumulate
	(const NumEffectState& state,
	 bool has_knight, const BoardMask& knight_fork_squares,
	 std::pair<Square,int>& knight_drop);
      };

      class SilverAdvance26
      {
      public:
	enum { ONE_DIM = 1, DIM = ONE_DIM*EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, ONE_DIM> table;
      };

      class Promotion37
      {
      public:
	enum { ONE_DIM = PTYPE_SIZE, DIM = ONE_DIM*EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
	static MultiInt evalOne(const NumEffectState &state, int rank);
	template<Player P>
	static MultiInt evalWithUpdate(const NumEffectState &, Move moved,
				       MultiInt const& last_value);

	static CArray<MultiInt, ONE_DIM> table;
      };
    }
  }
}

#endif // EVAL_ML_MINORPIECE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
