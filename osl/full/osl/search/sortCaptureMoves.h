/* sortCaptureMoves.h
 */
#ifndef SEARCH_SORTCAPTUREMOVES_H
#define SEARCH_SORTCAPTUREMOVES_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace search
  {
    /**
     * 安い駒の順にsort する.
     */
    struct SortCaptureMoves
    {
      /** 取り返しを考慮する．*/
      static void sortByTakeBack(const NumEffectState& state, 
				 MoveVector& moves);
      /** 取る駒は考えない．*/
      static void sortByMovingPiece(MoveVector& moves);
      /** 取る駒は考えない．from が一致する指手優先 */
      static void sortBySpecifiedPiece(MoveVector& moves, 
				       Square from);
    };
  }
}

#endif /* SEARCH_SORTCAPTUREMOVES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
