#ifndef _MOVE_ORDER_MOVESORTER_H
#define _MOVE_ORDER_MOVESORTER_H

#include "osl/container.h"
#include <algorithm>

namespace osl
{
  namespace move_order
  {
    struct MoveSorter
    {
      template <class Compare>
      static void sort(MoveVector& moves, const Compare& comp)
      {
	std::sort(moves.begin(), moves.end(), comp);
      }
    };
  } // namespace move_order
} // namespace osl

#endif /** _MOVE_ORDER_MOVESORTER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
