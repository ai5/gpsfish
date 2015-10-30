/* mobility.h
 */

#ifndef EVAL_ML_MOBILITY_H
#define EVAL_ML_MOBILITY_H

#include "osl/eval/weights.h"
#include "osl/eval/midgame.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class RookMobilityAll
      {
	friend class RookMobility;
	friend class RookMobilityX;
	friend class RookMobilityY;
	friend class RookMobilitySum;
	friend class RookMobilitySumKingX;
	friend class RookMobilityXKingX;
      public:
	template<int Sign>
	static void adjust(const NumEffectState&, bool promoted,
			   int vertical, int horizontal,
			   Square pos, 
			   MultiInt& value);
	static void eval(const NumEffectState&, MultiInt& out);
      private:
	static int indexX(Square rook, bool promoted,
			  int count, bool vertical)
	{
	  const int x = (rook.x() > 5 ?
			 10 - rook.x() : rook.x());
	  return x - 1 + 5 * ((promoted ? 1 : 0) +
			      2 * ((vertical ? 1 : 0) + 2 * count));
	}
	template <int Sign>
	static int indexY(Square rook, bool promoted,
			  int count, bool vertical)
	{
	  const int y = (Sign > 0 ? rook.y() : 10 - rook.y());
	  return y - 1 + 9 * ((promoted ? 1 : 0) +
			      2 * ((vertical ? 1 : 0) + 2 * count));
	}
	template <int Sign>
	static int indexXKingX(Square rook, Square king, int count, bool vertical)
	{
	  const Square r = (Sign > 0) ? rook : rook.rotate180();
	  const Square k = (Sign > 0) ? king : king.rotate180();
	  const bool flip = r.x() > 5;
	  const int x = (flip ? 10 - r.x() : r.x());
	  const int king_x = (flip ? 10 - k.x() : k.x());
	  return king_x - 1 + 9 * (x - 1 + 5 * ((vertical ? 1 : 0) + 2 * count));
	}
	static CArray<MultiInt, 18> rook_vertical_table;
	static CArray<MultiInt, 18> rook_horizontal_table;
	static CArray<MultiInt, 34> sum_table;
	static CArray<MultiInt, 324> x_table;
	static CArray<MultiInt, 324> y_table;
	static CArray<MultiInt, 17 * 9> sumkingx_table;
	static CArray<MultiInt, 9 * 2 * 5 * 9> xkingx_table;
      };

      class RookMobility
      {
      public:
	enum { DIM = 36 };
	static void setUp(const Weights &weights,int stage);
      };

      class RookMobilitySum
      {
      public:
	enum { ONE_DIM = 34, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class RookMobilityX
      {
      public:
	enum { ONE_DIM = 180, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class RookMobilityY
      {
      public:
	enum { ONE_DIM = 324, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class RookMobilitySumKingX
      {
      public:
	enum { ONE_DIM = 17 * 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      class RookMobilityXKingX
      {
      public:
	enum { ONE_DIM = 9 * 2 * 5 * 9, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      struct BishopMobilityAll
      {
	friend class BishopMobility;
	friend class BishopMobilityEach;
      public:
	template<int Sign>
	static void adjust(bool promoted, int mobility1, int mobility2,
			   MultiInt& value);
	static void eval(const NumEffectState&, MultiInt& out);
      private:
	static CArray<MultiInt, 36> bishop_table;
	static CArray<MultiInt, 18> each_table;
      };
      class BishopMobility
      {
      public:
	enum { DIM = 36 };
	static void setUp(const Weights &weights,int stage);
      };
      class BishopMobilityEach
      {
      public:
	enum { ONE_DIM = 18, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };
      struct LanceMobilityAll
      {
	template<int Sign>
	static void adjust(int index, MultiInt& value);
	static void eval(const NumEffectState&, MultiInt& out);
      };
      class LanceMobility
      {
	static CArray<MultiInt, 9> lance_table;
	friend struct LanceMobilityAll;
      public:
	enum { DIM = 9 };
	LanceMobility() { };
	static void setUp(const Weights &weights,int stage);
      };
    }
  }
}
#endif // EVAL_ML_MOBILITY_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
