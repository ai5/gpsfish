/* kingTable.h
 */

#ifndef EVAL_ML_KINGTABLE_H
#define EVAL_ML_KINGTABLE_H

#include "osl/eval/weights.h"
#include "osl/eval/midgame.h"
#include "osl/numEffectState.h"
#include <cstdlib>
namespace osl
{
  namespace eval
  {
    namespace ml
    {
      struct KingPieceRelative
      {
	typedef CArray2d<MultiInt, PTYPE_SIZE, 17 * 9> table_t;
	static table_t attack_table, defense_table;
	static int index(const NumEffectState &,
			 Player owner,
			 const Square position,
			 Square king)
	{
	  return std::abs(position.x() - king.x()) * 17 +
	    (owner == BLACK ? (king.y() - position.y()) :
	     (position.y() - king.y())) + 8;
	}
	static int index(const NumEffectState &state,
			 Player owner,
			 const Square position,
			 bool same_king)
	{
	  const Square king = state.kingSquare(
	    same_king ? owner : alt(owner));
	  return index(state, owner, position, king);
	}
	static int index(const NumEffectState &state,
			 PtypeO ptypeo,
			 const Square position,
			 bool same_king)
	{
	  return index(state, getOwner(ptypeo), position, same_king);
	}
	static int index(const NumEffectState &state,
			 const Piece piece,
			 bool same_king)
	{
	  return index(state, piece.owner(), piece.square(), same_king);
	}
	enum { DIM = ((osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) *
		      (17 * 9)) * 2};
	enum { TABLE_DIM = DIM / 2};
	static void setUp(const Weights &weights, int stage);
	static MultiInt eval(const NumEffectState &state);
	template<Player P>
	static MultiInt evalWithUpdate(const NumEffectState &state, Move moved, 
					const MultiInt& last_values);

      };

      class KingPieceRelativeNoSupport
      {
      public:
	enum { ONE_DIM = 4284, DIM = ONE_DIM * EvalStages };
	static MultiInt eval(const NumEffectState &state);
	static MultiInt evalWithUpdate(
	  const NumEffectState &state,
	  Move moved,
	  const CArray<PieceMask, 2> &effected_mask,
	  const MultiInt &last_values);
	static void setUp(const Weights &weights);
	template <int Sign>
	static void adjust(int attack, int defense, MultiInt& out);
      private:
	static int index(const Player player, const Square king,
			 const Ptype ptype, const Square pos)
	{
	  const int x = std::abs(pos.x() - king.x());
	  const int y = (king.y() - pos.y()) *
	    (player == osl::BLACK ? 1 : -1) + 8;
	  return (ptype - osl::PTYPE_PIECE_MIN) * 17 * 9 + (x * 17 + y);
	}
	static int index(const Player player, const Square king,
			 const Piece piece)
	{
	  return index(player, king, piece.ptype(), piece.square());
	}
	static CArray<MultiInt, ONE_DIM> table;
      };

      struct PtypeYY
      {
	enum { ONE_DIM = 2592, DIM = ONE_DIM * EvalStages};
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
	static MultiInt evalWithUpdate(
	  const NumEffectState& state,
	  Move moved,
	  const MultiInt &last_values);
	static CArray<MultiInt, 2592> table;
	template <int Sign>
	static void adjust(int black, int white, MultiInt &out);
      private:
	template <Player KingPlayer>
	static int index(const Piece p, const Square king)
	{
	  return index<KingPlayer>(p.ptypeO(), p.square(), king);
	}

	template <Player KingPlayer>
	static int index(const PtypeO ptypeO, const Square position,
			 const Square king)
	{
	  const int king_y = (KingPlayer == BLACK ? king.y() : 10 - king.y());
	  const int piece_y = (KingPlayer == BLACK ? position.y() :
			       10 - position.y());
	  return (king_y - 1) * 9 * 32 + (piece_y - 1) * 32 +
	    (KingPlayer == BLACK ? ptypeO : alt(ptypeO)) - PTYPEO_MIN;
	}
      };

      class King25Effect
      {
      private:
	static void countEffectAndPieces(const NumEffectState &state,
					 const Player attack,
					 int &effect,
					 int &piece);
	static int index(int effect, int piece_count);
      public:
	enum { DIM = 17 * 128 };
	static CArray<int, DIM> table;
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state);
      };

      class King25EffectBoth
      {
      private:
	static int index(int effect, int piece_count)
	{
	  return effect + 128 * piece_count;
	}
      public:
	template <Player Attack>
	static void countEffectAndPiecesBoth(
	  const NumEffectState &state,
	  PieceMask& effect25,
	  PieceMask& effect_supported,
	  int &attack_effect,
	  int &attack_piece,
	  int &defense_effect,
	  int &defence_piece,
	  int &attack_piece_supported,
	  CArray<int, 5> &effect_vertical,
	  CArray<int, 5> &king_vertical);
	enum { DIM = 17 * 128 * 2};
	static CArray<int, DIM/2> attack_table;
	static CArray<int, DIM/2> defense_table;
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &,
			int black_attack_effect, int black_attack_piece,
			int white_attack_effect, int white_attack_piece,
			int black_defense_effect, int black_defense_piece,
			int white_defense_effect, int white_defense_piece)
	{
	  return attack_table[index(black_attack_effect, black_attack_piece)] -
	    attack_table[index(white_attack_effect, white_attack_piece)] +
	    defense_table[index(black_defense_effect, black_defense_piece)] -
	    defense_table[index(white_defense_effect, white_defense_piece)];
	}
      };

      class King25EffectAttack 
      {
      private:
	static int index(int effect, int piece_count)
	{
	  return effect + 128 * piece_count;
	}
      public:
	enum { DIM = 17 * 128};
	static CArray<int, 17 * 128> table;
	static void setUp(const Weights &weights)
	{
	  for (size_t i = 0; i < weights.dimension(); ++i)
	  {
	    table[i] = weights.value(i);
	  }
	}
	static int eval(const NumEffectState &,
			int black_effect, int black_piece,
			int white_effect, int white_piece)
	{
	  return table[index(black_effect, black_piece)] -
	    table[index(white_effect, white_piece)];
	}
      };

      class King25EffectDefense
      {
      private:
	static int index(int effect, int piece_count)
	{
	  return effect + 128 * piece_count;
	}
      public:
	enum { DIM = 17 * 128};
	static CArray<MultiInt, 17 * 128> table;
	static void setUp(const Weights &weights,int stage)
	{
	  for (size_t i = 0; i < weights.dimension(); ++i)
	  {
	    table[i][stage] = weights.value(i);
	  }
	}
	static MultiInt eval(const NumEffectState &,
			int black_effect, int black_piece,
			int white_effect, int white_piece)
	{
	  return table[index(black_effect, black_piece)] -
	    table[index(white_effect, white_piece)];
	}
      };


      class King25EffectYAttack
      {
      private:
	static int index(int king_y, int effect, int piece_count)
	{
	  return effect + 128 * piece_count + (king_y - 1) * 128 * 17;
	}
      public:
	enum { DIM = 17 * 128 * 9};
	static CArray<int, 17 * 128 * 9> table;
	static void setUp(const Weights &weights)
	{
	  for (size_t i = 0; i < weights.dimension(); ++i)
	  {
	    table[i] = weights.value(i);
	  }
	}
	static int eval(const NumEffectState &state,
			int black_effect, int black_piece,
			int white_effect, int white_piece)
	{
	  // ugly hack.  -1 is attack.  0 >= is defense
	    return table[index(10 - state.kingSquare<WHITE>().y(),
			       black_effect, black_piece)] -
	      table[index(state.kingSquare<BLACK>().y(),
			  white_effect, white_piece)];
	}
      };

      class King25EffectYDefense
      {
      private:
	static int index(int king_y, int effect, int piece_count)
	{
	  return effect + 128 * piece_count + (king_y - 1) * 128 * 17;
	}
      public:
	enum { DIM = 17 * 128 * 9};
	static CArray<MultiInt, 17 * 128 * 9> table;
	static void setUp(const Weights &weights,int stage)
	{
	  for (size_t i = 0; i < weights.dimension(); ++i)
	  {
	    table[i][stage] = weights.value(i);
	  }
	}
	static MultiInt eval(const NumEffectState &state,
			int black_effect, int black_piece,
			int white_effect, int white_piece)
	{
	  return table[index(state.kingSquare<BLACK>().y(),
			     black_effect, black_piece)] -
	    table[index(10 - state.kingSquare<WHITE>().y(),
			white_effect, white_piece)];
	}
      };

      class King25EffectY
      {
      public:
	enum { DIM = 17 * 128 * 2 * 9};
      private:
	static int index(int king_y, int effect, int piece_count)
	{
	  return effect + 128 * piece_count + (king_y - 1) * 128 * 17;
	}
	static CArray<int, DIM/2> attack_table;
	static CArray<int, DIM/2> defense_table;
      public:
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state,
			int black_attack_effect, int black_attack_piece,
			int white_attack_effect, int white_attack_piece,
			int black_defense_effect, int black_defense_piece,
			int white_defense_effect, int white_defense_piece)
	{
	  return attack_table[index(10 - state.kingSquare<WHITE>().y(),
				    black_attack_effect, black_attack_piece)] -
	    attack_table[index(state.kingSquare<BLACK>().y(),
			       white_attack_effect, white_attack_piece)] +
	    defense_table[index(state.kingSquare<BLACK>().y(),
				black_defense_effect, black_defense_piece)] -
	    defense_table[index(10 - state.kingSquare<WHITE>().y(),
				white_defense_effect, white_defense_piece)];
	}
      };

      class King25Effect2
      {
      public:
	enum { ONE_DIM = 17 * 13 * 64, DIM = ONE_DIM * EvalStages };
      private:
	static int index(int effect, int piece_count,
			 int stand_count)
	{
	  return (effect + 64 * piece_count) * 13 + stand_count;
	}
	static CArray<MultiInt, ONE_DIM> table;
      public:
	static void setUp(const Weights &weights);
	static MultiInt eval(
	  const NumEffectState &,
	  int black_attack_effect, int black_attack_piece,
	  int white_attack_effect, int white_attack_piece,
	  int black_stand_count, int white_stand_count)
	{
	  const int black_index = index(black_attack_effect,
					black_attack_piece,
					black_stand_count);
	  const int white_index = index(white_attack_effect,
					white_attack_piece,
					white_stand_count);
	  return table[black_index] - table[white_index];
	}
      };
      class King25EffectY2
      {
      public:
	enum { ONE_DIM = 17 * 13 * 64 * 9, DIM = ONE_DIM * EvalStages };
      private:
	static int index(int king_y, int effect, int piece_count,
			 int stand_count)
	{
	  return ((effect + 64 * piece_count) * 13 + stand_count) * 9 +
	    (king_y - 1);
	}
	static CArray<MultiInt, ONE_DIM> table;
      public:
	static void setUp(const Weights &weights);
	static MultiInt eval(
	  const NumEffectState &state,
	  int black_attack_effect, int black_attack_piece,
	  int white_attack_effect, int white_attack_piece,
	  int black_stand_count, int white_stand_count)
	{
	  const int black_index = index(10 - state.kingSquare<WHITE>().y(),
					black_attack_effect, black_attack_piece,
					black_stand_count);
	  const int white_index = index(state.kingSquare<BLACK>().y(),
					white_attack_effect, white_attack_piece,
					white_stand_count);
	  return table[black_index] - table[white_index];
	}
      };
      class King25EffectSupported
      {
      public:
	enum { ONE_DIM = 17 * 17, DIM = ONE_DIM * EvalStages };
      private:
	static int index(int piece_count, int supported)
	{
	  return supported * 17 + piece_count;
	}
	static CArray<MultiInt, ONE_DIM> table;
      public:
	static void setUp(const Weights &weights);
	static MultiInt eval(
	  int black_attack_piece,
	  int white_attack_piece,
	  int black_attack_supported_piece, int white_attack_supported_piece)
	{
	  const int black_index = index(black_attack_piece,
					black_attack_supported_piece);
	  const int white_index = index(white_attack_piece,
					white_attack_supported_piece);
	  return table[black_index] - table[white_index];
	}
      };
      class King25EffectSupportedY
      {
      public:
	enum { ONE_DIM = 17 * 17 * 9, DIM = ONE_DIM * EvalStages };
      private:
	static int index(int piece_count, int supported, int y)
	{
	  return (supported * 17 + piece_count) * 9 + y - 1;
	}
	static CArray<MultiInt, ONE_DIM> table;
      public:
	static void setUp(const Weights &weights);
	static MultiInt eval(
	  int black_attack_piece,
	  int white_attack_piece,
	  int black_attack_supported_piece, int white_attack_supported_piece,
	  int black_king_y, int white_king_y)
	{
	  const int black_index = index(black_attack_piece,
					black_attack_supported_piece,
					10 - white_king_y);
	  const int white_index = index(white_attack_piece,
					white_attack_supported_piece,
					black_king_y);
	  return table[black_index] - table[white_index];
	}
      };
      struct King25EmptySquareNoEffect
      {
	enum { DIM = 3 * 5 };
	template <Player defense>
	static int evalOne(const NumEffectState &state, const CArray<int, 15>& table);
	template <Player defense>
	static std::pair<int,int> evalOne(const NumEffectState &state, const CArray<int, 15>& opening, const CArray<int, 15>& ending);
	static std::pair<CArray<int,2>, CArray<int,2> >
	eval(const NumEffectState &state, const CArray<int, 15>& opening, const CArray<int, 15>& ending);
	static std::pair<CArray<int,2>, CArray<int,2> >
	evalWithUpdate(const NumEffectState &state, Move last_move,
		       const CArray<int, 15>& opening, const CArray<int, 15>& ending,
		       const CArray<int,2>& last_opening_value, const CArray<int,2>& last_ending_value);
	static int index(int rel_x, int rel_y)
	{
	  return (rel_y + 2) * 3 + std::abs(rel_x);
	}
	static void setUpBase(const Weights &weigths, CArray<int, 15>& table);
      };

      class King25EmptySquareNoEffectOpening
	: public King25EmptySquareNoEffect
      {
	static CArray<int, 15> table;
      public:
	static void setUp(const Weights &weigths) { setUpBase(weigths, table); }
	static const CArray<int,2> eval(const NumEffectState &state);
	static const CArray<int, 15>& weights() { return table; }
      };

      class King25EmptySquareNoEffectEnding
	: public King25EmptySquareNoEffect
      {
	static CArray<int, 15> table;
      public:
	static void setUp(const Weights &weigths) { setUpBase(weigths, table); }
	static const CArray<int,2> eval(const NumEffectState &state);
	static const CArray<int, 15>& weights() { return table; }
      };

      template <bool Opening>
      class King25EmptyAbs
      {
      public:
	enum { DIM = 5 * 5 * 5 * 9 };
      private:
	static CArray<int, 1125> table;
	template <Player player>
	static int index(Square king,
			 Square target);
	static int index(Square king,
			 Square target, Player player) {
	  if (player == BLACK)
	    return index<BLACK>(king, target);
	  else
	    return index<WHITE>(king, target);
	}
	template <Player Defense>
	static int evalOne(const NumEffectState &state);
      public:
	static int evalWithUpdate(
	  const NumEffectState &state, osl::Move moved,
	  int last_value);
	static void setUp(const Weights &weigths);
	static int eval(const NumEffectState &state);
      };

      class King25EmptyAbsOpening : public King25EmptyAbs<true>
      {
      };
      class King25EmptyAbsEnding : public King25EmptyAbs<false>
      {
      };

      enum EffectState
      {
	NO_ATTACK_DEFENSE_0,
	NO_ATTACK_DEFENSE_1,
	NO_ATTACK_DEFENSE_2,
	ATTACK_DIFF_N2,
	ATTACK_DIFF_N1,
	ATTACK_DIFF_0,
	ATTACK_DIFF_1,
	ATTACK_DIFF_2,
	STATE_MAX, // 8
      };
      template <int Stage>
      class King25EffectEach
      {
      public:
	enum { DIM = 5 * 3 * STATE_MAX * 3 };
      private:
	static CArray<int, 5 * 3 * 8 * 3> table;
	template <Player Defense>
	static EffectState effectState(const NumEffectState &state,
				       Square target);
	template <Player Defense>
	static int index(const NumEffectState &state, Square king,
			 Square target);
	template <osl::Player Defense>
	static int evalOne(const NumEffectState &state);
      public:
	static void setUp(const Weights &weigths);
	static int eval(const NumEffectState &state);
      };

      class King25EffectEachOpening : public King25EffectEach<0>
      {
      };
      class King25EffectEachMidgame : public King25EffectEach<1>
      {
      };
      class King25EffectEachEnding : public King25EffectEach<2>
      {
      };

      class King25EffectEachBothOpening
      {
      public:
	enum { DIM = 5 * 3 * STATE_MAX * 3 };
	static void setUp(const Weights &weigths);
      };
      class King25EffectEachBothMidgame
      {
      public:
	enum { DIM = 5 * 3 * STATE_MAX * 3 };
	static void setUp(const Weights &weigths);
      };
      class King25EffectEachBothMidgame2
      {
      public:
	enum { DIM = 5 * 3 * STATE_MAX * 3 };
	static void setUp(const Weights &weigths);
      };
      class King25EffectEachBothEnding
      {
      public:
	enum { DIM = 5 * 3 * STATE_MAX * 3 };
	static void setUp(const Weights &weigths);
      };

      class King25EffectEachBoth
      {
	enum EffectState
	{
	  NO_ATTACK_DEFENSE_0,
	  NO_ATTACK_DEFENSE_1,
	  NO_ATTACK_DEFENSE_2,
	  ATTACK_DIFF_N2,
	  ATTACK_DIFF_N1,
	  ATTACK_DIFF_0,
	  ATTACK_DIFF_1,
	  ATTACK_DIFF_2,
	  STATE_MAX, // 8
	};
	friend class King25EffectEachBothOpening;
	friend class King25EffectEachBothMidgame;
	friend class King25EffectEachBothMidgame2;
	friend class King25EffectEachBothEnding;
	friend class King25EffectEachXY;
	friend class King25EffectEachKXY;
      private:
	static CArray<MultiInt, 5 * 3 * 8 * 3> table;
	static CArray<MultiInt, 3000> x_table;
	static CArray<MultiInt, 3240> y_table;
	static CArray<MultiInt, 27000> xy_table;
	static CArray<int, 256> effect_state_table;
	template <Player Defense>
	static int effectStateIndex3(const NumEffectState &state,
				     Square target);
	template <Player Defense>
	static void index(const NumEffectState &state, 
			  Square target,
			  int &index_xy,
			  int rel_y, int king_x, int king_y, int x_diff
	  );
	template <osl::Player Defense>
	static void evalOne(const NumEffectState &state,
			    MultiInt& out);
      public:
	static void eval(const NumEffectState &state,
			 MultiIntPair &out);
	static void
	evalWithUpdate(const NumEffectState &state, Move last_move,
		       MultiIntPair & values);
      };

      class King25EffectEachXY
      {
      public:
	enum { X_DIM = 3000, Y_DIM = 3240, DIM = (X_DIM + Y_DIM) * EvalStages};
	static void setUp(const Weights &weigths);
      };

      class King25EffectEachKXY
      {
      public:
	enum { ONE_DIM = 27000, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weigths);
      };

      class King3Pieces
      {
	friend class King3PiecesXY;
      public:
	enum { ONE_DIM = 3072, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
	static MultiInt evalWithUpdate(const NumEffectState &state,
					Move last_move,
					MultiInt &last_value);
      private:
	enum Direction
	{
	  HORIZONTAL = 0,
	  VERTICAL,
	  DIAGONAL,
	};
	template <Player King, Direction Dir>
	static int index(PtypeO p1, PtypeO p2)
	{
	  if (King == WHITE)
	  {
	    p1 = altIfPiece(p1);
	    p2 = altIfPiece(p2);
	  }
	  return ptypeOIndex(p1) * 32 + ptypeOIndex(p2) + 1024 * Dir;
	}
	template <Player King, Direction Dir>
	static int indexY(const Square king_position,
			  PtypeO p1, PtypeO p2)
	{
	  if (King == WHITE)
	  {
	    p1 = altIfPiece(p1);
	    p2 = altIfPiece(p2);
	  }
	  const int king_y = (King == BLACK ? king_position.y() :
			      10 - king_position.y());
	  return ptypeOIndex(p1) * 32 + ptypeOIndex(p2) + 1024 * Dir
	    + (king_y - 1) * 32 * 32 * 3;
	}
	template <Player King, Direction Dir>
	static int indexX(const Square king_position,
			  PtypeO p1, PtypeO p2)
	{
	  if (King == WHITE)
	  {
	    p1 = altIfPiece(p1);
	    p2 = altIfPiece(p2);
	  }
	  const int king_x = (king_position.x() > 5 ? 10 - king_position.x() :
			      king_position.x());
	  if (Dir == HORIZONTAL &&
	      ((King == BLACK && king_position.x() >= 6) ||
	       (King == WHITE && king_position.x() <= 4)))
	  {
	    PtypeO tmp = p1;
	    p1 = p2; p2 = tmp;
	  }
	  return ptypeOIndex(p1) * 32 + ptypeOIndex(p2) + 1024 * Dir
	    + (king_x - 1) * 32 * 32 * 3;
	}
	static MultiInt value(int vertical_index, int horizontal_index,
			       int diagonal_index1, int diagonal_index2,
			       int vertical_index_x,  int horizontal_index_x,
			       int diagonal_index1_x, int diagonal_index2_x,
			       int vertical_index_y , int horizontal_index_y,
			       int diagonal_index1_y, int diagonal_index2_y) 
	{
	  return table[vertical_index] + table[horizontal_index] +
	    table[diagonal_index1] + table[diagonal_index2] +
	    x_table[vertical_index_x] + x_table[horizontal_index_x] +
	    x_table[diagonal_index1_x] + x_table[diagonal_index2_x] +
	    y_table[vertical_index_y] + y_table[horizontal_index_y] +
	    y_table[diagonal_index1_y] + y_table[diagonal_index2_y];
	}
	
	template <Player King>
	static void evalOne(const NumEffectState &state,
			    MultiInt &result);
	static CArray<MultiInt, 3072> table;
	static CArray<MultiInt, 15360> x_table;
	static CArray<MultiInt, 27648> y_table;
      };

      class King3PiecesXY
      {
      public:
	enum
	{
	  X_DIM = 32 * 32 * 3 * 5,
	  Y_DIM = 32 * 32 * 3 * 9,
	  ONE_DIM = X_DIM + Y_DIM,
	  DIM = ONE_DIM * EvalStages,
	};
	static void setUp(const Weights &weights);
      };

      class KingMobility
      {
	friend class KingMobilityWithRook;
	friend class KingMobilityWithBishop;
      public:
	enum { ONE_DIM = 3240, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	template <Player P>
	static MultiInt evalOne(const NumEffectState &state);
	template<Direction Dir>
	static int mobilityDir(Square king,Square target)
	{
	  if(Dir==L) return king.x()-target.x()-1;
	  else if(Dir==R) return target.x()-king.x()-1;
	  else if(Dir==UL || Dir==U || Dir==UR) return target.y()-king.y()-1;
	  else return king.y()-target.y()-1;
	}
	static CArray<MultiInt, 3240> table;
	static CArray<MultiInt, 3240> rook_table;
	static CArray<MultiInt, 3240> bishop_table;
	static CArray<MultiInt, 3240> rook_bishop_table;
      };

      class KingMobilityWithRook
      {
      public:
	enum { ONE_DIM = 3240, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class KingMobilityWithBishop
      {
      public:
	enum { ONE_DIM = 3240, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class KingMobilitySum
      {
      public:
	enum { ONE_DIM = 2925, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
      private:
	template <Player P>
	static MultiInt evalOne(const NumEffectState &state);
	static CArray<MultiInt, 45*33> table;
      };

      class King25BothSide
      {
	friend class King25BothSideX;
	friend class King25BothSideY;
      public:
	enum { ONE_DIM = 8192, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	template<Player P>
	static MultiInt evalOne(const NumEffectState &state,
				const CArray<int, 5> &effects);
	static MultiInt eval(const NumEffectState &state,
			     const CArray<int, 5> &black,
			     const CArray<int, 5> &white);
      private:
	static int index(int effect1, int effect2, int i)
	{
	  assert(0 <= effect1 && effect1 < 32);
	  assert(0 <= effect2 && effect2 < 32);
	  return effect1 + 32 * (effect2 + 32 * i);
	}
	template <Player P>
	static int indexX(Square king, int effect1, int effect2,
			  int i, int j)
	{
	  const int king_x = (king.x() >= 6 ? 10 - king.x() : king.x());
	  if ((P == BLACK && king.x() > 5) ||
	      (P == WHITE && king.x() < 5))
	  {
	    const int tmp = effect1;
	    effect1 = effect2;
	    effect2 = tmp;
	    const int tmp2 = i;
	    i = 4 - j;
	    j = 4 - tmp2;
	  }
	  if (i == 2)
	    --j;
	  const int combination = (i * 3 + j - 2);
	  assert(0 <= effect1 && effect1 < 32);
	  assert(0 <= effect2 && effect2 < 32);
	  return king_x - 1 + 5 * (effect1 + 32 *
				   (effect2 + 32 * combination));
	}
	static int indexX(int king_x,int effect1,int effect2, int i){
	  return king_x - 1 + 5 * (effect1 + 32 *
				   (effect2 + 32 * i));
	}
	template <Player P>
	static int indexY(Square king, int effect1, int effect2, int i)
	{
	  const int king_y = (P == BLACK ? king.y() : 10 - king.y());
	  assert(0 <= effect1 && effect1 < 32);
	  assert(0 <= effect2 && effect2 < 32);
	  return king_y - 1 + 9 *(effect1 + 32 * (effect2 + 32 * i));
	}
	static int indexY(int king_y,int effect1,int effect2, int i){
	  return king_y - 1 + 9 *(effect1 + 32 * (effect2 + 32 * i));
	}
	static CArray<MultiInt, 8192> table;
	static CArray<MultiInt, 40960> x_table;
	static CArray<MultiInt, 73728> y_table;
      };
      class King25BothSideX
      {
      public:
	enum { ONE_DIM = 40960, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class King25BothSideY
      {
      public:
	enum { ONE_DIM = 73728, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class King25Mobility
      {
	friend class King25MobilityX;
	friend class King25MobilityY;
      public:
	enum { ONE_DIM = 4096, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state,
			     const CArray<int, 5> &black,
			     const CArray<int, 5> &white);
      private:
	static int index(int effect1, int effect2, int i)
	{
	  assert(0 <= effect1 && effect1 < 32);
	  assert(0 <= effect2 && effect2 < 32);
	  return effect1 + 32 * (effect2 + 32 * i);
	}
	template <Player Defense>
	static int indexX(Square king, int effect1, int effect2, int i)
	{
	  const int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
	  if ((Defense == BLACK && king.x() > 5) ||
	      (Defense == WHITE && king.x() < 5))
	  {
	    const int tmp = effect1;
	    effect1 = effect2;
	    effect2 = tmp;
	    i = 3 - i;
	  }
	  assert(0 <= effect1 && effect1 < 32);
	  assert(0 <= effect2 && effect2 < 32);
	  return king_x - 1 + 5 * (effect1 + 32 * (effect2 + 32 * i));
	}
	template <Player Defense>
	static int indexY(Square king, int effect1, int effect2, int i)
	{
	  const int king_y = (Defense == BLACK ? king.y() : 10 - king.y());
	  assert(0 <= effect1 && effect1 < 32);
	  assert(0 <= effect2 && effect2 < 32);
	  return king_y - 1 + 9 * (effect1 + 32 * (effect2 + 32 * i));
	}
	static CArray<MultiInt, 4096> table;
	static CArray<MultiInt, 20480> x_table;
	static CArray<MultiInt, 36864> y_table;
      };
      class King25MobilityX
      {
      public:
	enum { ONE_DIM = 20480, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class King25MobilityY
      {
      public:
	enum { ONE_DIM = 36864, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class King25Effect3
      {
	friend class King25Effect3Y;
      public:
	enum { ONE_DIM = 2400, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state,
			     const CArray<PieceMask, 2> &king25_mask);
      private:
	static int index(int piece_count, bool with_knight,
			 int stand_count, bool with_knight_on_stand,
			 int attacked_count)
	{
	  assert(piece_count >= 0 && piece_count <= 9);
	  assert(stand_count >= 0 && stand_count <= 9);
	  assert(attacked_count >= 0 && attacked_count <= 5);
	  return (piece_count + 10 *
		  ((with_knight ? 1 : 0) + 2 *
		   (stand_count + 10 * ((with_knight_on_stand ? 1 : 0) +
					2 * attacked_count))));
	}
	static int indexY(int piece_count, bool with_knight,
			  int stand_count, bool with_knight_on_stand,
			  int attacked_count, int king_y)
	{
	  assert(piece_count >= 0 && piece_count <= 9);
	  assert(stand_count >= 0 && stand_count <= 9);
	  assert(attacked_count >= 0 && attacked_count <= 5);
	  return ((piece_count + 10 *
		   ((with_knight ? 1 : 0) + 2 *
		    (stand_count + 10 * ((with_knight_on_stand ? 1 : 0) +
					 2 * attacked_count))))) * 9 +
	    king_y - 1;
	}
	template <osl::Player Attack>
	static MultiInt evalOne(const NumEffectState &state,
				PieceMask king25);
	static CArray<MultiInt, 2400> table;
	static CArray<MultiInt, 21600> y_table;
      };
      class King25Effect3Y
      {
      public:
	enum { ONE_DIM = 21600, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class King25EffectCountCombination
      {
	friend class King25EffectCountCombinationY;
      public:
	enum { ONE_DIM = 100, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state,
			     const CArray<PieceMask, 2> &king25);
      private:
	template <osl::Player Attack>
	static MultiInt evalOne(const NumEffectState &state,
				PieceMask king25);
	static CArray<MultiInt, 100> table;
	static CArray<MultiInt, 900> y_table;
      };
      class King25EffectCountCombinationY
      {
      public:
	enum { ONE_DIM = 900, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class BishopExchangeSilverKing
      {
	static int indexKing(Square king) 
	{
	  const int y = king.y();
	  if (y >= 3)
	    return -1;
	  return (y-1)*9 + king.x()-1;
	}
	static int indexRook(Square rook) 
	{
	  assert(rook.isOnBoard());
	  const int y = rook.y();
	  if (y >= 6)
	    return -1;
	  return (y-1)*9 + rook.x()-1;
	}
	static int indexSilver(Square silver) 
	{
	  return (silver.y()-1)*9 + silver.x()-1;
	}
      public:
	enum { BISHOP_ONE_DIM = 18 * 81 * (45*2), DIM = BISHOP_ONE_DIM*3 };
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState& state);
      private:
	template <Player KingOwner>
	static int evalOne(const NumEffectState &state, int offset);
	static CArray<int, DIM> table;
      };

      class EnterKingDefense
      {
      public:
	enum { DIM = (8+8+8+8)*3 };
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state);
      private:
	template <Player KingOwner>
	static int evalOne(const NumEffectState &state);
	static CArray<int, DIM> table;
      };
    }
  }
}
#endif // EVAL_ML_KINGTABLE_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
