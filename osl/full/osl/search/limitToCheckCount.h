/* limitToCheckCount.h
 */
#ifndef OSL_LIMITTOCHECKCOUNT_H
#define OSL_LIMITTOCHECKCOUNT_H
#include "osl/container.h"
#include <cstdlib>
namespace osl
{
  namespace checkmate
  {
    inline size_t limitToCheckCount(int limit)
    { 
      assert(limit >= 0);
      extern CArray<size_t,32> LimitToCheckCountTable;
      return LimitToCheckCountTable[limit>>7];
    }
  }
}

#endif /* _LIMITTOCHECKCOUNT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
