/* richPredictor.h
 */
#ifndef _RICHPREDICTOR_H
#define _RICHPREDICTOR_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace threatmate
  {
    class RichPredictor
    {
    public:
      double predict(const NumEffectState& state, const Move move);
    };

  } // namespace threatmate
} // namespace osl

#endif /* _RICHPREDICTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
