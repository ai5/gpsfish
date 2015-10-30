/* bigramAttack.cc
 */
#include "osl/rating/feature/bigramAttack.h"
#include <sstream>

const std::string osl::rating::
BigramAttack::name(int x1, int y1, int x2, int y2, int king_index, bool same, bool focus_x)
{
  std::ostringstream os;
  os << "BA(" << x1 << "," << y1 << ")(" << x2 << "," << y2 << ")" << (same ? "=" : "!")
     << (focus_x ? "X" : "Y") << king_index;
  return os.str();

}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
