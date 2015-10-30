/* mlPredictor.h
 */
#ifndef _MLPREDICTOR_H
#define _MLPREDICTOR_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace threatmate
  {
    class MlPredictor
    {
    public:
      double predict(const NumEffectState& state, const Move move, size_t index=0);
      double probability(const NumEffectState& state, const Move move, size_t index=0);
    };
  } // namespace threatmate
} // namespace osl

#endif /* _MLPREDICTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
