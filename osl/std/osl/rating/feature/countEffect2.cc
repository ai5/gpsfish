/* countEffect2.cc
 */
#include "osl/rating/feature/countEffect2.h"
#include <sstream>

const int osl::rating::CountEffect2::Max;

std::string 
osl::rating::CountEffect2::name(int attack, int defense) 
{
  std::ostringstream ss;
  ss << attack << defense;
  return ss.str();
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
