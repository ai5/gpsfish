/* rookMobility.t.cc
 */
#include "osl/mobility/rookMobility.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::mobility;

BOOST_AUTO_TEST_CASE(RookMobilityTestVertical)
{
  {
    NumEffectState state=
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
    // 83 飛車 
    Piece p=state.pieceAt(Square(8,3));
    BOOST_CHECK_EQUAL(6,RookMobility::countVerticalAll(BLACK,state,p));
    BOOST_CHECK_EQUAL(3,RookMobility::countVerticalSafe(BLACK,state,p));
    int countAll=0,countSafe=0;
    RookMobility::countVerticalBoth(BLACK,state,p,countAll,countSafe);
    BOOST_CHECK_EQUAL(6,countAll);
    BOOST_CHECK_EQUAL(3,countSafe);
  }
  {
    NumEffectState state=
      CsaString(
		"P1+NY+RY *  *  *  * -KE-OU-KY\n"
		"P2 *  *  *  *  * -GI-KI *  *\n"
		"P3 * +TO *  * +UM * -KI-FU-FU\n"
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
    // 81 飛車 
    Piece p=state.pieceAt(Square(8,1));
    BOOST_CHECK_EQUAL(1,RookMobility::countVerticalAll(BLACK,state,p));
    BOOST_CHECK_EQUAL(1,RookMobility::countVerticalSafe(BLACK,state,p));
    int countAll=0,countSafe=0;
    RookMobility::countVerticalBoth(BLACK,state,p,countAll,countSafe);
    BOOST_CHECK_EQUAL(1,countAll);
    BOOST_CHECK_EQUAL(1,countSafe);
  }
}

BOOST_AUTO_TEST_CASE(RookMobilityTestHorizontal)
{
  {
    NumEffectState state=
      CsaString(
		"P1+NY+TO *  *  *  * -OU-KE-KY\n"
		"P2 *  *  * -FU * -GI-KI *  *\n"
		"P3 * +RY *  * +UM * -KI-FU-FU\n"
		"P4 *  * +FU *  *  *  *  *  *\n"
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
    // 83 飛車 
    Piece p=state.pieceAt(Square(8,3));
    BOOST_CHECK_EQUAL(3,RookMobility::countHorizontalAll(BLACK,state,p));
    BOOST_CHECK_EQUAL(2,RookMobility::countHorizontalSafe(BLACK,state,p));
    int countAll=0,countSafe=0;
    RookMobility::countHorizontalBoth(BLACK,state,p,countAll,countSafe);
    BOOST_CHECK_EQUAL(3,countAll);
    BOOST_CHECK_EQUAL(2,countSafe);
  }
  {
    NumEffectState state=
      CsaString(
		"P1+NY *  *  *  *  * -KE-OU-KY\n"
		"P2+RY *  *  *  * -GI-KI *  *\n"
		"P3 * +TO *  * +UM * -KI-FU-FU\n"
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
    // 92 飛車 
    Piece p=state.pieceAt(Square(9,2));
    BOOST_CHECK_EQUAL(5,RookMobility::countHorizontalAll(BLACK,state,p));
    BOOST_CHECK_EQUAL(4,RookMobility::countHorizontalSafe(BLACK,state,p));
    int countAll=0,countSafe=0;
    RookMobility::countHorizontalBoth(BLACK,state,p,countAll,countSafe);
    BOOST_CHECK_EQUAL(5,countAll);
    BOOST_CHECK_EQUAL(4,countSafe);
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
