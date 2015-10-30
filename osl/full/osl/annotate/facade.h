/* facade.h
 */
#ifndef OSL_ANNOTATE_FACADE_H
#define OSL_ANNOTATE_FACADE_H

#include "osl/annotate/analysesResult.h"
#include "osl/numEffectState.h"
#include <vector>

namespace osl
{
  namespace annotate
  {
    void analyze(const NumEffectState& src, const std::vector<Move>& moves,
		 int last_move,
		 AnalysesResult&);
  }
}

#endif /* OSL_ANNOTATE_FACADE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
