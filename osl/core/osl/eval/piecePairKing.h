/* piecePairKing.h
 */
#ifndef OSL_EVAL_ML_PIECEPAIRKING_H
#define OSL_EVAL_ML_PIECEPAIRKING_H

#include "osl/numEffectState.h"
namespace osl
{
  namespace eval
  {
    namespace ml
    {
      struct Weights;
      class PiecePairKing
      {
      public:
	enum
	{
	  ONE_DIM = 1488375,
	  DIM = ONE_DIM
	};
	static void setUp(const Weights &weights);
	static CArray<int,2> eval(const NumEffectState&);
	template <Player P>
	static void evalWithUpdateBang(const NumEffectState& state, Move moved, CArray<int,2>& last_value);

	template <Player King>
	static int evalOne(const NumEffectState&);
      private:
	template <Player King>
	static int add(const NumEffectState& state, Square to, Ptype ptype);
	template <Player King>
	static int sub(const NumEffectState& state, Square from, Ptype ptype);
	template <Player King>
	static int addSub(const NumEffectState& state, Square to, Ptype ptype, Square from);
	static int composeIndex(int king, int i0, int i1)
	{
	  return king + i0*45*7 + i1;
	}
	static int indexWhite(Square p)
	{
	  return p.x()-1 + (p.y()-1)*9;
	}
	static int indexKing(Player owner, Square king, bool& flipx)
	{
	  if (owner == BLACK)
	    king = king.rotate180();
	  assert(king.y() <= 3);
	  if (king.x() > 5)
	  {
	    king = king.flipHorizontal();
	    flipx = true;
	  }
	  else
	    flipx = false;
	  return (king.x()-1 + (king.y()-1)*5)*45*7*45*7;
	}
	template <bool FlipX>
	static int indexPiece(Player owner, Square position, Ptype ptype)
	{
	  assert(! isPromoted(ptype));
	  if (owner == BLACK)
	    position = position.rotate180();
	  if (FlipX)
	    position = position.flipHorizontal();
	  assert(position.y() <= 5);
	  return indexWhite(position)*7 + ptype-PTYPE_BASIC_MIN-1;
	}
	static osl::CArray<int16_t, ONE_DIM> table;
      };
    }
  }
}


#endif /* OSL_EVAL_ML_PIECEPAIRKING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
