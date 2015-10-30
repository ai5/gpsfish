/* evalStagePair.h
 */
#ifndef OSL_EVALSTAGEPAIR_H
#define OSL_EVALSTAGEPAIR_H

#include "osl/eval/midgame.h"

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      typedef CArray<CArray<int,EvalStages>,2> EvalStagePair;
      typedef CArray<CArray<int,2>,EvalStages> PairEvalStage;
    }
  }
}

#endif /* OSL_EVALSTAGEPAIR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
