/* oracleDisprover.cc
 */
#include "oracleDisprover.h"
#include "oracleDisprover.tcc"
#include "simpleCheckHashTable.h"
#include "dominanceTable.h"
#include "osl/state/numEffectState.h"

namespace osl
{
  namespace checkmate
  {
    // explicit template instantiation
#ifdef CHECKMATE_TABLE_ALL
    template class OracleDisprover<SimpleCheckHashTable>;
#endif
    template class OracleDisprover<DominanceTable>;
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
