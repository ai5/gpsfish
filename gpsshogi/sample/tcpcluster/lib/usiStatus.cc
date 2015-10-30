/* usiStatus.cc
 */
#include "usiStatus.h"
#include <iostream>

std::ostream& gpsshogi::operator<<(std::ostream& os,
				   UsiStatus status)
{
  static const char *table[Sentinel] = {
    "WaitConnection", "Initializing", "Idle",
    "Go", "SearchStop", "GoMate", "CheckmateStop",
    "Quitting", "Disconnected", 
  };
  return os << table[status];
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
