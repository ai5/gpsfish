/* kfendPredictor.h
 */
#ifndef _KFENDPREDICTOR_H
#define _KFENDPREDICTOR_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace threatmate
  {
    class KfendPredictor
    {
    public:
      bool predict(const NumEffectState& state, const Move move);
    };

  } // namespace threatmate
} // namespace osl

#endif /* _KFENDPREDICTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
