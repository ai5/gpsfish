/* captureEstimation.h
 */
#ifndef _MOVE_ORDER_CAPTUREESTIMATION_H
#define _MOVE_ORDER_CAPTUREESTIMATION_H

#include "osl/move_order/promotion.h"
#include "osl/numEffectState.h"
#include "osl/eval/pieceEval.h"
namespace osl
{
  namespace move_order
  {
    struct CaptureEstimation
    {
      const NumEffectState& state;
      explicit CaptureEstimation(const NumEffectState& s) : state(s)
      {
      }
      bool operator()(Move l, Move r) const
      {
	const Ptype capture_ptype_l = l.capturePtype();
	const Ptype capture_ptype_r = r.capturePtype();

	const Ptype old_ptype_l = l.oldPtype();
	const Ptype old_ptype_r = r.oldPtype();

	const Player turn = l.player();
	
	int value_l = eval::Ptype_Eval_Table.value(capture_ptype_l);
	if (state.hasEffectAt(alt(turn), l.to()))
	  value_l -= eval::Ptype_Eval_Table.value(old_ptype_l);
	int value_r = eval::Ptype_Eval_Table.value(capture_ptype_r);
	if (state.hasEffectAt(alt(turn), r.to()))
	  value_r -= eval::Ptype_Eval_Table.value(old_ptype_r);
	
	// 実入の大きそうな手から指す
	if (value_l != value_r)
	  return value_l > value_r;

	// 成る手から読む
	return Promotion()(l, r);
      }
    };
  } // namespace move_order
} // namespace osl


#endif /* _MOVE_ORDER_CAPTUREESTIMATION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
