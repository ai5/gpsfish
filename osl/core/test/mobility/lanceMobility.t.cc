/* lanceMobility.t.cc
 */
#include "osl/mobility/lanceMobility.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::mobility;

BOOST_AUTO_TEST_CASE(LanceMobilityTestCount)
{
  {
    SimpleState sState=
      CsaString(
		"P1-KY+HI *  *  * -OU * -KE-KY\n"
		"P2 *  *  *  *  *  * -KI *  * \n"
		"P3 *  * -KE * -KI-FU * -FU * \n"
		"P4-FU * +KI *  *  * -FU * -FU\n"
		"P5 *  *  * -KE-GI+GI * -KE * \n"
		"P6 *  *  *  *  *  * +FU * +FU\n"
		"P7+FU+FU+KI * +KA+FU *  *  * \n"
		"P8+KA *  *  *  *  *  * +HI * \n"
		"P9+KY * +OU *  *  *  *  * +KY\n"
		"P+00FU00FU00FU00FU00FU00FU\n"
		"P-00GI00GI00FU00FU\n"
		"-\n").initialState();
    NumEffectState state(sState);
    {
      // 11 香車
      Piece p=state.pieceAt(Square(1,1));
      BOOST_CHECK_EQUAL(2,LanceMobility::countAll(WHITE,state,p));
      BOOST_CHECK_EQUAL(1,LanceMobility::countSafe(WHITE,state,p));
      int countAll=0,countSafe=0;
      LanceMobility::countBoth(WHITE,state,p,countAll,countSafe);
      BOOST_CHECK_EQUAL(2,countAll);
      BOOST_CHECK_EQUAL(1,countSafe);
    }
    {
      // 91 香車
      Piece p=state.pieceAt(Square(9,1));
      BOOST_CHECK_EQUAL(2,LanceMobility::countAll(WHITE,state,p));
      BOOST_CHECK_EQUAL(1,LanceMobility::countSafe(WHITE,state,p));
      int countAll=0,countSafe=0;
      LanceMobility::countBoth(WHITE,state,p,countAll,countSafe);
      BOOST_CHECK_EQUAL(2,countAll);
      BOOST_CHECK_EQUAL(1,countSafe);
    }
    {
      // 19 香車
      Piece p=state.pieceAt(Square(1,9));
      BOOST_CHECK_EQUAL(2,LanceMobility::countAll(BLACK,state,p));
      BOOST_CHECK_EQUAL(1,LanceMobility::countSafe(BLACK,state,p));
      int countAll=0,countSafe=0;
      LanceMobility::countBoth(BLACK,state,p,countAll,countSafe);
      BOOST_CHECK_EQUAL(2,countAll);
      BOOST_CHECK_EQUAL(1,countSafe);
    }
    {
      // 99 香車
      Piece p=state.pieceAt(Square(9,9));
      BOOST_CHECK_EQUAL(0,LanceMobility::countAll(BLACK,state,p));
      BOOST_CHECK_EQUAL(0,LanceMobility::countSafe(BLACK,state,p));
      int countAll=0,countSafe=0;
      LanceMobility::countBoth(BLACK,state,p,countAll,countSafe);
      BOOST_CHECK_EQUAL(0,countAll);
      BOOST_CHECK_EQUAL(0,countSafe);
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
