/* promotion.h
 */
#ifndef _MOVE_ORDER_PROMOTION_H
#define _MOVE_ORDER_PROMOTION_H
#include "osl/basic_type.h"

namespace osl
{
  namespace move_order
  {
    /**
     * 成る手を優先
     */
    struct Promotion
    {
      bool operator()(Move l, Move r) const
      {
	const int promotion_l = l.promoteMask();
	const int promotion_r = r.promoteMask();

	return promotion_l > promotion_r;
      }
    };
  } // namespace move_order
} // namespace osl

#endif /* _MOVE_ORDER_PROMOTION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
