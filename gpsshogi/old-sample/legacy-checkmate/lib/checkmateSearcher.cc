/* checkmateSearcher.cc
 */

#include "checkmateSearcher.h"
#include "checkmateSearcher.tcc"
#include "dominanceTable.h"
#include "osl/state/numEffectState.h"

namespace osl
{
  namespace checkmate
  {
    template class CheckmateSearcher<DominanceTable>;
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
