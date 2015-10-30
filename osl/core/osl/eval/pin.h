/* pin.h
 */

#ifndef EVAL_ML_PIN_H
#define EVAL_ML_PIN_H

#include "osl/numEffectState.h"
#include "osl/eval/weights.h"
#include "osl/eval/midgame.h"
#include <cstdlib>
namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class SimplePin
      {
	static CArray<int, PTYPE_SIZE> table;
      public:
	SimplePin() { };
	static void setUp(const Weights &weights);
	int eval(const NumEffectState &state,
		 PieceMask black_mask, PieceMask white_mask) const;
      };

      class Pin
      {
	static int index(const Square king,
			 const Piece piece)
	{
	  return std::abs(piece.square().x() - king.x()) * 17 +
	    (piece.owner() == BLACK ? (king.y() - piece.square().y()) :
	     (piece.square().y() - king.y())) + 8;
	}
	static CArray2d<MultiInt, PTYPE_SIZE, 17 * 9> table;
      public:
	enum { DIM = (osl::PTYPE_MAX - osl::PTYPE_PIECE_MIN + 1) * 17 * 9};
	Pin() { };
	static void setUp(const Weights &weights,int stage);
	static MultiInt eval(const NumEffectState &state,
			      PieceMask black_mask, PieceMask white_mask);
      };

      class PinPtypeAll
      {
      public:
	static MultiInt eval(const NumEffectState &state);
      private:
	template <Player Defense>
	static MultiInt evalOne(const NumEffectState &state);
	template <Player Defense>
	static bool pawnAttack(const NumEffectState &state, Piece piece)
	{
	  const Square up =
	    piece.square() + DirectionPlayerTraits<U, Defense>::offset();
	  return (up.isOnBoard() &&
		  (state.hasEffectByPtypeStrict<PAWN>(alt(Defense), up) 
		   || (!state.isPawnMaskSet(alt(Defense),
					    piece.square().x()) 
		       && state.pieceAt(up).isEmpty())));
	}
      protected:
	static CArray<MultiInt, 80> table;
	static CArray<MultiInt, 48> pawn_table;
	static CArray<MultiInt, 560> distance_table;
      };

      class PinPtype : public PinPtypeAll
      {
      public:
	enum { ONE_DIM = 80, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class PinPtypeDistance : public PinPtypeAll
      {
      public:
	enum { ONE_DIM = 560, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class PinPtypePawnAttack : public PinPtypeAll
      {
      public:
	enum { ONE_DIM = 48, DIM = ONE_DIM * EvalStages };
	static void setUp(const Weights &weights);
      };

      class CheckShadowPtype 
      {
      public:
	enum { 
	  // rook v, rook h, bishop u, bishop d, lance
	  ONE_DIM = PTYPE_SIZE * 5, 
	  DIM = ONE_DIM * EvalStages 
	};
	static void setUp(const Weights &weights);
	static MultiInt eval(const NumEffectState &state);
	template <Player King>
	static MultiInt evalOne(const NumEffectState &state);
	static CArray<MultiInt, ONE_DIM> table;
      };
    }
  }
}
#endif // EVAL_ML_PIN_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
