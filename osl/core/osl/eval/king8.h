/* king8.h
 */

#ifndef EVAL_ML_KING8_H
#define EVAL_ML_KING8_H

#include "osl/eval/weights.h"
#include "osl/eval/evalStagePair.h"
#include "osl/numEffectState.h"
#include "osl/bits/king8Info.h"

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class King8Effect
      {
      public:
	enum { DIM = 32 + 32 + 288 + 288 };
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state);
      private:
	enum EffectState
	{
	  NOT_EMPTY = -1,
	  NO_EFFECT = 0,
	  LESS_EFFECT,
	  MORE_EFFECT,
	  MORE_EFFECT_KING_ONLY
	};
	static CArray<int, 32> empty_table;
	static CArray<int, 32> defense_table;
	static CArray<int, 288> empty_y_table;
	static CArray<int, 288> defense_y_table;
	static int index(const Direction dir,
			 EffectState state);
	static int indexY(Piece king,
			  const Direction dir,
			  EffectState state);
	static void effectState(const NumEffectState &state,
				const Player defense,
				const Direction dir,
				EffectState &empty,
				EffectState &/*defense*/);
      };


      class King8EffectBase
      {
      public:
	enum { DIM = 32 };
	typedef CArray<int, 32> table_t;
	enum EffectState
	{
	  NOT_EMPTY = -1,
	  NO_EFFECT = 0,
	  LESS_EFFECT,
	  MORE_EFFECT,
	  MORE_EFFECT_KING_ONLY
	};
	template <class MakeEffectState>
	static const CArray<int,2> evalCommon(const NumEffectState &state, const table_t&);
	template <class MakeEffectState>
	static const CArray<int,2> evalWithUpdateCommon(const NumEffectState &new_state, Move last_move, 
							const CArray<int,2>& last_value, const table_t&);

	template <class MakeEffectState>
	static std::pair<CArray<int,2>, CArray<int,2> >
	evalWithUpdateCommon(const NumEffectState &new_state, Move last_move, 
			     const CArray<int,2>& last_value_opening, const CArray<int,2>& last_value_ending, 
			     const table_t&, const table_t&);

	struct MakeEffectStateSimple;
	struct MakeEffectStateDefense;
      protected:
	static int index(const Direction dir, EffectState state)
	{
	  return dir * 4 + state;
	}
      };
      class King8EffectEmptySquareBoth;
      template <bool Opening>
      class King8EffectEmptySquare : public King8EffectBase
      {
	friend class King8EffectEmptySquareBoth;
	static table_t table;
      public:
	static void setUp(const Weights &weights);
	static const CArray<int,2> eval(const NumEffectState &state);
	static const CArray<int,2> evalWithUpdate(const NumEffectState &new_state, Move last_move, 
						  const CArray<int,2>& last_value);
      };

      class King8EffectEmptySquareBoth : public King8EffectBase
      {
      public:
	static std::pair<CArray<int,2>, CArray<int,2> >
	evalWithUpdate(const NumEffectState &new_state, Move last_move, 
		       const CArray<int,2>& last_value_opening,
		       const CArray<int,2>& last_value_ending);
      };
      struct King8EffectEmptySquareOpening
	: public King8EffectEmptySquare<true>
      {
      };
      struct King8EffectEmptySquareEnding
	: public King8EffectEmptySquare<false>
      {
      };

      class King8EffectDefenseSquareBoth;
      template <bool Opening>
      class King8EffectDefenseSquare
	: public King8EffectBase
      {
	friend class King8EffectDefenseSquareBoth;
	static CArray<int, 32> table;
      public:
	static void setUp(const Weights &weights);
	static const CArray<int,2> eval(const NumEffectState &state);
	static const CArray<int,2> evalWithUpdate(const NumEffectState &new_state, Move last_move, 
						  const CArray<int,2>& last_value);
      };
      class King8EffectDefenseSquareBoth : public King8EffectBase
      {
      public:
	static std::pair<CArray<int,2>, CArray<int,2> >
	evalWithUpdate(const NumEffectState &new_state, Move last_move, 
		       const CArray<int,2>& last_value_opening,
		       const CArray<int,2>& last_value_ending);
      };

      struct King8EffectDefenseSquareOpening
	: public King8EffectDefenseSquare<true>
      {
      };
      struct King8EffectDefenseSquareEnding
	: public King8EffectDefenseSquare<false>
      {
      };

      class King8EffectAll
      {
      public:
	enum { ONE_DIM = 32, DIM = 32 * 5 * 2};
      private:
	static CArray<int, ONE_DIM> base_table;
	static CArray<int, ONE_DIM> u_table;
	static CArray<int, ONE_DIM> d_table;
	static CArray<int, ONE_DIM> l_table;
	static CArray<int, ONE_DIM> r_table;
	static CArray<int, ONE_DIM> base_defense_piece_table;
	static CArray<int, ONE_DIM> u_defense_piece_table;
	static CArray<int, ONE_DIM> d_defense_piece_table;
	static CArray<int, ONE_DIM> l_defense_piece_table;
	static CArray<int, ONE_DIM> r_defense_piece_table;
      public:
	enum EffectState
	{
	  NOT_EMPTY = -1,
	  NO_EFFECT = 0,
	  LESS_EFFECT,
	  MORE_EFFECT,
	  MORE_EFFECT_KING_ONLY
	};
	static void setUp(const Weights &weights);
	King8EffectAll() { }
	static int eval(const NumEffectState &state,
			PieceMask black_mask, PieceMask white_mask);
	static void effectState(const NumEffectState &state,
				const Player defense,
				const Direction dir,
				EffectState &empty,
				EffectState &/*defense*/);
	static int index(const Direction dir, EffectState state);
      };

      struct KingXBlockedBase
      {
	enum { DIM = 10 };
	typedef CArray<MultiInt, 10> table_t;
	static const MultiIntPair eval(const NumEffectState &state,
					const table_t& table);
	template <osl::Player P>
	static int index(Square king, int diff);
	template <osl::Player P>
	static bool isBlocked(const NumEffectState &state,
			      int diff);
#if 0
	static std::pair<CArray<int,2>,CArray<int,2> > 
	evalWithUpdate(const NumEffectState &new_state, Move last_move,
		       const CArray<int,2>& last_value_o,
		       const CArray<int,2>& last_value_e,
		       const table_t& table_o, const table_t& table_e);
#endif
      };
      class KingXBlockedBoth;

      class KingXBlocked : public KingXBlockedBase
      {
	friend class KingXBlockedBoth;
	friend class KingXBlockedYBase;
      public:
	static void setUp(const Weights &weights,int stage);
	static MultiIntPair eval(const NumEffectState &state)
	{
	  return KingXBlockedBase::eval(state, table);
	}
      private:
	static table_t table;
      };

      class KingXBlockedYBase// : public KingXBlockedBase
      {
      public:
	enum { DIM = 90 };
	typedef CArray<MultiInt, 90> table_t;
	static const MultiIntPair eval(const NumEffectState &state,
					const table_t& table);
	static void
	evalWithUpdateBang(const NumEffectState &state, Move laste_move,
			   MultiIntPair &last_values_and_out);
	template <osl::Player P>
	static int index(Square king, int diff);
      private:
	template <int Sign>
	static void adjust(int index, int index_y, MultiInt &out);
      };

      class KingXBlockedY : public KingXBlockedYBase
      {
	friend class KingXBlockedBoth;
	friend class KingXBlockedYBase;
      public:
	static void setUp(const Weights &weights,int stage);
	static const MultiIntPair eval(const NumEffectState &state)
	{
	  return KingXBlockedYBase::eval(state, table);
	}
      private:
	static table_t table;
      };

      class KingXBlockedBoth : public KingXBlockedBase
      {
      public:
	static void
	evalWithUpdateBang(const NumEffectState &new_state, Move last_move,
			   MultiIntPair& last_values_and_out);
      };

      class KingXBothBlocked
      {
	friend class KingXBlockedYBase;
      public:
	enum { ONE_DIM = 5, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
	static MultiIntPair eval(const NumEffectState &state);
      private:
	static CArray<MultiInt, ONE_DIM> table;
	static int index(const Square king)
	{
	  const int x = king.x();
	  return (x > 5 ? 9 - x : x - 1);;
	}
	template <Player P>
	static int indexY(const Square king)
	{
	  const int x = king.x();
	  const int y = (P == BLACK ? king.y() : 10 - king.y());
	  return (y - 1) * 5 + (x > 5 ? 9 - x : x - 1);
	}
	template <int Sign>
	static void adjust(int index, int index_y, MultiInt &out);
      };

      class KingXBothBlockedY
      {
	friend class KingXBlockedYBase;
	friend class KingXBothBlocked;
      public:
	enum { ONE_DIM = 5 * 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      private:
	static CArray<MultiInt, ONE_DIM> table;
      };

      class KingXBlocked3
      {
	friend class KingXBlocked3Y;
      public:
	enum { ONE_DIM = 80, DIM = ONE_DIM * EvalStages };
	static MultiInt eval(const NumEffectState &state);
	static void setUp(const Weights &weights);
      private:
	template <int Sign>
	static void adjust(int index_y, MultiInt &result)
	{
	  if(Sign>0)
	    result += y_table[index_y];
	  else
	    result -= y_table[index_y];
	}
	template <Player P>
	static int index(const Square king, bool is_l,
		  bool u_blocked, bool opp_u_blocked, bool opp_blocked)
	{
	  int x = king.x();
	  if (x >= 6)
	  {
	    x = 10 - x;
	    if (P == BLACK)
	    {
	      is_l = !is_l;
	    }
	  }
	  else if (P == WHITE && x <= 4)
	  {
	    is_l = !is_l;
	  }
	  return x - 1 + 5 * ((is_l ? 1 : 0) + 2 * ((u_blocked ? 1 : 0) + 2 * ((opp_u_blocked ? 1  : 0) +2 * (opp_blocked ? 1 : 0))));
	}
	template <Player P>
	static int indexY(const Square king, bool is_l,
			  bool u_blocked, bool opp_u_blocked,
			  bool opp_blocked)
	{
	  int x = king.x();
	  const int y = (P == BLACK ? king.y() : 10 - king.y());
	  if (x >= 6)
	  {
	    x = 10 - x;
	    if (P == BLACK)
	    {
	      is_l = !is_l;
	    }
	  }
	  else if (P == WHITE && x <= 4)
	  {
	    is_l = !is_l;
	  }
	  return x - 1 + 5 * (y - 1 + 9 * ((is_l ? 1 : 0) + 2 * ((u_blocked ? 1 : 0) + 2 * ((opp_u_blocked ? 1 : 0) + 2 * (opp_blocked ? 1 : 0)))));
	}
	static CArray<MultiInt, 80> table;
	static CArray<MultiInt, 720> y_table;
      };

      class KingXBlocked3Y
      {
      public:
	enum { ONE_DIM = 720, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class AnagumaEmpty
      {
      public:
	enum { DIM = 4 };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state);
	static int index(Square king, Square target);
	template <osl::Player Defense>
	static MultiInt evalOne(const NumEffectState &state);
      private:
	static CArray<MultiInt, 4> table;
      };
    }
  }
}

#endif // EVAL_ML_KING8_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
