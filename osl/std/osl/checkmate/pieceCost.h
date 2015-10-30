/* pieceCost.h
 */
#ifndef _PIECECOST_H
#define _PIECECOST_H

#include "osl/container.h"
namespace osl
{
  namespace checkmate
  {

    /**
     * 駒の損得に基づいたcostの推定
     */
    struct PieceCost
    {
      static const CArray<signed char, PTYPE_SIZE> attack_sacrifice_cost;      
    };
  } // namespace checkmate
} // namespace osl

#endif /* _PIECECOST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
