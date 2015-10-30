/* cheapPtype.h
 */
#ifndef _CHEAPPTYPE_H
#define _CHEAPPTYPE_H

#include "osl/move_order/promotion.h"
#include "osl/eval/pieceEval.h"

namespace osl
{
  namespace move_order
  {
    /**
     * 安い駒から使う
     */
    struct CheapPtype
    {
      bool operator()(Move l, Move r) const
      {
	const Ptype old_ptype_l = l.oldPtype();
	const Ptype old_ptype_r = r.oldPtype();

	if (old_ptype_l != old_ptype_r)
	  return (eval::Ptype_Eval_Table.value(old_ptype_l)
		  < eval::Ptype_Eval_Table.value(old_ptype_r));
	return Promotion()(l, r);
      }
    };
  } // namespace move_order
} // namespace osl

#endif /* _CHEAPPTYPE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
