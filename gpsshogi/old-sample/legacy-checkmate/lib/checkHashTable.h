/* checkHashTable.h
 */
#ifndef _CHECKHASHTABLE_H
#define _CHECKHASHTABLE_H

// TODO: checkHashTableFwd.h を作って依存関係を減らす
// #define USE_SIMPLE_CHECK_HASH_TABLE
#ifndef USE_SIMPLE_CHECK_HASH_TABLE
#  include "dominanceTable.h"
namespace osl
{
  namespace checkmate
  {
    typedef DominanceTable CheckHashTable;
  } // namespace checkmate
} // namespace osl
#else
#  include "osl/checkmate/simpleCheckHashTable.h"
namespace osl
{
  namespace checkmate
  {
    typedef SimpleCheckHashTable CheckHashTable;
  } // namespace checkmate
} // namespace osl
#endif

namespace osl
{
  using checkmate::CheckHashTable;
}

#endif /* _CHECKHASHTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
