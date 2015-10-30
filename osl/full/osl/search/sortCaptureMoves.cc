/* sortCaptureMoves.cc
 */
#include "osl/search/sortCaptureMoves.h"
#include "osl/move_order/captureEstimation.h"
#include "osl/move_order/cheapPtype.h"
#include <algorithm>

void osl::search::SortCaptureMoves::sortByMovingPiece(MoveVector& moves)
{
  std::sort(moves.begin(), moves.end(), move_order::CheapPtype());
}

namespace osl
{
  namespace search
  {
    struct OrderSpecifiedPiece
    {
      Square from;
      explicit OrderSpecifiedPiece(Square f) : from(f)
      {
      }
      bool operator()(Move l, Move r) const
      {
	const Square from_l = l.from();
	if (from_l == from)
	  return true;
	const Square from_r = r.from();
	if (from_r == from)
	  return false;

	return move_order::CheapPtype()(l, r);
      }
    };
  } // anonymous namespace
} // namespace osl

void osl::search::SortCaptureMoves::
sortBySpecifiedPiece(MoveVector& moves, Square from)
{
  std::sort(moves.begin(), moves.end(), OrderSpecifiedPiece(from));
}

void osl::search::SortCaptureMoves::
sortByTakeBack(const NumEffectState& state, MoveVector& moves)
{
  std::sort(moves.begin(), moves.end(), 
	    move_order::CaptureEstimation(state));
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
