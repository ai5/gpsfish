/* recordSet.h
 */
#ifndef _CHECK_RECORDSET_H
#define _CHECK_RECORDSET_H

#include "osl/container/pointerSet.h"

namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;
  }
}

namespace osl
{
  namespace checkmate
  {
    namespace analyzer
    {
      struct RecordSet : public container::PointerSet<CheckHashRecord>
      {
	RecordSet();
	~RecordSet();
      };
    } // namespace analyzer
  } // namespace checkmate
} // namespace osl

#endif /* _CHECK_RECORDSET_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
