/* oracleProver.cc
 */
#include "oracleProver.h"
#include "oracleProver.tcc"
#include "simpleCheckHashTable.h"
#include "dominanceTable.h"
#include "osl/state/numEffectState.h"

namespace osl
{
  namespace checkmate
  {
    // explicit template instantiation
#ifdef CHECKMATE_TABLE_ALL
    template class OracleProver<SimpleCheckHashTable>;
#endif
    template class OracleProver<DominanceTable>;
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
