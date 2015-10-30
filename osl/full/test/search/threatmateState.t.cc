#include "osl/search/threatmateState.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(ThreatmateStateTestConstruction) {
  ThreatmateState s;
  BOOST_CHECK_EQUAL(ThreatmateState::UNKNOWN, s.status());
}
BOOST_AUTO_TEST_CASE(ThreatmateStateTestTransition) {
  ThreatmateState t;
  t.setThreatmate(ThreatmateState::THREATMATE);
  BOOST_CHECK_EQUAL(ThreatmateState::THREATMATE, t.status());

  ThreatmateState mc(t.newStatus(false));
  BOOST_CHECK_EQUAL(ThreatmateState::MAY_HAVE_CHECKMATE, mc.status());
    
  ThreatmateState ct(t.newStatus(true));
  BOOST_CHECK_EQUAL(ThreatmateState::CHECK_AFTER_THREATMATE, ct.status());

  ThreatmateState u(ct.newStatus(true));
  BOOST_CHECK_EQUAL(ThreatmateState::UNKNOWN, u.status());

  ThreatmateState mc2(ct.newStatus(false));
  BOOST_CHECK_EQUAL(ThreatmateState::MAYBE_THREATMATE, mc2.status());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
