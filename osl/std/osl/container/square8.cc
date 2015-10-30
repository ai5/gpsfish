/* position8.cc
 */
#include "osl/container/square8.h"
#include <iostream>

#ifndef MINIMAL
std::ostream& osl::container::operator<<(std::ostream& os, const Square8& v)
{
  os << "Square8(";
  for (size_t i=0; i<v.size(); ++i)
    os << " " << v[i].x() << v[i].y();
  return os << ")";
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
