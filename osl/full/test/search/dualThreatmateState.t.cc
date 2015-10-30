#include "osl/search/dualThreatmateState.h"
#include <boost/test/unit_test.hpp>
using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(DualThreatmateStateTestConstruction) {
  DualThreatmateState s;
  BOOST_CHECK(s.status(BLACK).isUnknown());
  BOOST_CHECK(s.status(WHITE).isUnknown());
}
BOOST_AUTO_TEST_CASE(DualThreatmateStateTestTransition) {
  DualThreatmateState t;
  const Move bmove(Square(5,2),GOLD,BLACK);
  const Move wmove(Square(5,8),GOLD,WHITE);

  t.setThreatmate(BLACK, bmove); // 先手番、先手玉に詰めろ
  BOOST_CHECK(t.isThreatmate(BLACK));
  BOOST_CHECK(! t.isThreatmate(WHITE));
  BOOST_CHECK_EQUAL(bmove, t.threatmateMove(BLACK));

  DualThreatmateState mc;
  mc.updateInLock(BLACK, &t, false);
  BOOST_CHECK(mc.mayHaveCheckmate(BLACK));
  BOOST_CHECK(mc.status(WHITE).isUnknown());

  mc.setThreatmate(WHITE, wmove); // さっきの先手の指しては詰めろだった
  BOOST_CHECK(mc.mayHaveCheckmate(BLACK));
  BOOST_CHECK(mc.isThreatmate(WHITE));
    
  DualThreatmateState ct;
  ct.updateInLock(WHITE, &mc, true);	// 後手が王手

  DualThreatmateState cte;
  cte.updateInLock(BLACK, &ct, false);	// 先手が逃げる

  BOOST_CHECK(cte.status(BLACK).isUnknown());
  BOOST_CHECK(cte.maybeThreatmate(WHITE));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
