/* treePredictor.h
 */
#ifndef _TREEPREDICTOR_H
#define _TREEPREDICTOR_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace threatmate
  {
    class TreePredictor
    {
    public:
      bool predict(const NumEffectState& state, const Move move);
      double probability(const NumEffectState& state, const Move move);
    };
  } // namespace threatmate
} // namespace osl

#endif /* _TREEPREDICTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
