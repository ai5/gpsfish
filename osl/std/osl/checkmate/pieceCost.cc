/* pieceCost.cc
 */
#include "osl/checkmate/pieceCost.h"

namespace osl
{
  const CArray<signed char, PTYPE_SIZE> checkmate::
  PieceCost::attack_sacrifice_cost = 
  {{ 
    0, 0,			// empty, edge
    1, 2, 2, 2,		// ppawn - psilver
    4, 4,			// pbishop, prook
    0, 4, 			// king, gold
    1, 2, 2, 2,
    3, 3,
  }};
} // namespace osl

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
