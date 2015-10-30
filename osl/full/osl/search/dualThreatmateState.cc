/* dualThreatmateState.cc
 */
#include "osl/search/dualThreatmateState.h"
#include <iostream>

std::ostream& osl::search::operator<<(std::ostream& os, DualThreatmateState s)
{
  return os << "+ " << s.status(BLACK) << " - " << s.status(WHITE);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
