/* checkAssert.h
 */
#ifndef _CHECKASSERT_H
#define _CHECKASSERT_H
#include <cassert>

namespace osl
{
  namespace checkmate
  {
    void checkAbort(const char *func, const char *file, int line, 
		    const char *exp);
  } // namespace checkmate
} // namespace osl

#define check_assert(x) assert((x) || (checkAbort(__func__, __FILE__, __LINE__, #x),0))

#endif /* _CHECKASSERT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
