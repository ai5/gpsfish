/* threatmateState.cc
 */
#include "osl/search/threatmateState.h"
#include "osl/csa.h"
#include <iostream>

const osl::CArray<osl::search::ThreatmateState::Status,5*2> 
osl::search::ThreatmateState::transition = {{
  /*UNKNOWN*/ UNKNOWN, UNKNOWN,
  /*THREATMATE*/ MAY_HAVE_CHECKMATE, CHECK_AFTER_THREATMATE,
  /*MAYBE_THREATMATE*/ MAY_HAVE_CHECKMATE, CHECK_AFTER_THREATMATE,
  /*CHECK_AFTER_THREATMATE*/ MAYBE_THREATMATE, UNKNOWN,
  /*MAY_HAVE_CHECKMATE*/ UNKNOWN, UNKNOWN,
}};

#ifndef MINIMAL
std::ostream& osl::search::operator<<(std::ostream& os, ThreatmateState s)
{
  switch (s.status())
  {
  case ThreatmateState::THREATMATE:
    return os << "THREATMATE "; // << csa::show(s.threatmate_move);
  case ThreatmateState::MAYBE_THREATMATE:
    return os << "maybe threatmate";
  case ThreatmateState::MAY_HAVE_CHECKMATE:
    return os << "may have checkmate";
  case ThreatmateState::CHECK_AFTER_THREATMATE:
    return os << "check after threatmate";
  default:
    return os << "unkown";
  }
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
