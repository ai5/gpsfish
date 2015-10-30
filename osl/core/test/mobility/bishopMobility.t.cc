/* bishopMobility.t.cc
 */
#include "osl/mobility/bishopMobility.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::mobility;

BOOST_AUTO_TEST_CASE(BishopMobilityTestCount)
{
  {
    SimpleState sState=
      CsaString(
		"P1+NY+TO *  *  *  * -OU-KE-KY\n"
		"P2 *  *  *  *  * -GI-KI *  *\n"
		"P3 * +RY *  * +UM * -KI-FU-FU\n"
		"P4 *  * +FU-FU *  *  *  *  *\n"
		"P5 *  * -KE * +FU *  * +FU *\n"
		"P6+KE *  * +FU+GI-FU *  * +FU\n"
		"P7 *  * -UM *  *  *  *  *  *\n"
		"P8 *  *  *  *  *  *  *  *  * \n"
		"P9 * +OU * -GI *  *  *  * -NG\n"
		"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
		"P-00KI00KY00FU00FU\n"
		"P-00AL\n"
		"+\n"
		).initialState();
    NumEffectState state(sState);
    {
      // 53 馬
      Piece p=state.pieceAt(Square(5,3));
      BOOST_CHECK_EQUAL(8,BishopMobility::countAll(BLACK,state,p));
      BOOST_CHECK_EQUAL(6,BishopMobility::countSafe(BLACK,state,p));
      int countAll=0,countSafe=0;
      BishopMobility::countBoth(BLACK,state,p,countAll,countSafe);
      BOOST_CHECK_EQUAL(8,countAll);
      BOOST_CHECK_EQUAL(6,countSafe);
    }
    {
      // 77 馬
      Piece p=state.pieceAt(Square(7,7));
      BOOST_CHECK_EQUAL(7,BishopMobility::countAll(WHITE,state,p));
      BOOST_CHECK_EQUAL(4,BishopMobility::countSafe(WHITE,state,p));
      int countAll=0,countSafe=0;
      BishopMobility::countBoth(WHITE,state,p,countAll,countSafe);
      BOOST_CHECK_EQUAL(7,countAll);
      BOOST_CHECK_EQUAL(4,countSafe);
    }
  }
}

BOOST_AUTO_TEST_CASE(BishopMobilityTestCountDir)
{
  {
    SimpleState sState=
      CsaString(
		"P1+NY+TO *  *  *  * -OU-KE-KY\n"
		"P2 *  *  *  *  * -GI-KI *  *\n"
		"P3 * +RY *  * +UM * -KI-FU-FU\n"
		"P4 *  * +FU-FU *  *  *  *  *\n"
		"P5 *  * -KE * +FU *  * +FU *\n"
		"P6+KE *  * +FU+GI-FU *  * +FU\n"
		"P7 *  * -UM *  *  *  *  *  *\n"
		"P8 *  *  *  *  *  *  *  *  * \n"
		"P9 * +OU * -GI *  *  *  * -NG\n"
		"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
		"P-00KI00KY00FU00FU\n"
		"P-00AL\n"
		"+\n"
		).initialState();
    NumEffectState state(sState);
    {
      // 53 馬
      Piece p=state.pieceAt(Square(5,3));
      BOOST_CHECK_EQUAL(2, (BishopMobility::countAllDir<BLACK,UL>(state,p)));
      BOOST_CHECK_EQUAL(1, (BishopMobility::countAllDir<BLACK,UR>(state,p)));
      BOOST_CHECK_EQUAL(1, (BishopMobility::countAllDir<BLACK,DL>(state,p)));
      BOOST_CHECK_EQUAL(4, (BishopMobility::countAllDir<BLACK,DR>(state,p)));
    }
    {
      // 77 馬
      Piece p=state.pieceAt(Square(7,7));
      BOOST_CHECK_EQUAL(2, (BishopMobility::countAllDir<WHITE,UL>(state,p)));
      BOOST_CHECK_EQUAL(2, (BishopMobility::countAllDir<WHITE,UR>(state,p)));
      BOOST_CHECK_EQUAL(1, (BishopMobility::countAllDir<WHITE,DL>(state,p)));
      BOOST_CHECK_EQUAL(2, (BishopMobility::countAllDir<WHITE,DR>(state,p)));
    }
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
