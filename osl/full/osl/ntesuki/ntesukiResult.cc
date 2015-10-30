#include "osl/ntesuki/ntesukiResult.h"
#include <iostream>

std::ostream&
osl::ntesuki::operator<<(std::ostream& os, osl::ntesuki::NtesukiResult nr)
{

  if (osl::ntesuki::NTESUKI_YES == nr)
  {
    return os << "NTESUKI_YES";
  }
  else if (osl::ntesuki::NTESUKI_NO == nr)
  {
    return os << "NTESUKI_NO";
  }
  assert (osl::ntesuki::isUnknown(nr));

  return os << "NTESUKI_UNKNOWN("
	    << osl::ntesuki::getDepth(nr)
	    << ")";
}
  
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
