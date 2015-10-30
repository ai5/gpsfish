/* corruptCheckTable.h
 */
#ifndef _CORRUPTCHECKTABLE_H
#define _CORRUPTCHECKTABLE_H
#include <stdexcept>

namespace osl
{
  namespace checkmate
  {
    class CheckHashRecord;
    struct CorruptCheckTable : std::runtime_error
    {
      const CheckHashRecord *record;
#ifdef __GNUC__
__attribute__ ((noinline))
#endif
      CorruptCheckTable(const CheckHashRecord *r, 
			const char *msg="CorruptCheckTable") 
	: std::runtime_error(msg), record(r)
      {
      }
    };
  } // namespace checkmate
} // namespace osl


#endif /* _CORRUPTCHECKTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
