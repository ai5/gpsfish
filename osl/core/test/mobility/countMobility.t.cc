/* countMobility.t.cc
 */
#include "osl/mobility/countMobility.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::mobility;

BOOST_AUTO_TEST_CASE(CountMobilityTestCount)
{
  {
    SimpleState sState=
      CsaString(
		"P1+NY+TO *  *  *  * -OU-KE-KY\n"
		"P2 *  *  *  *  * -GI-FU *  *\n"
		"P3 * +RY *  * +UM * -KI-FU-FU\n"
		"P4 *  * +FU-KI *  *  *  *  *\n"
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
    // 83 飛車 
    {
      const Square pos(8,3);
      {
	const Offset o(0,-1);
	BOOST_CHECK_EQUAL(1,countMobilitySafe(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(1,countMobilityAll(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(1,countAll);      
	BOOST_CHECK_EQUAL(1,countSafe);      
      }
      {
	const Offset o(0,1);
	BOOST_CHECK_EQUAL(5,countMobilityAll(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(2,countMobilitySafe(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(5,countAll);      
	BOOST_CHECK_EQUAL(2,countSafe);      
      }
      {
	const Offset o(-1,0);
	BOOST_CHECK_EQUAL(2,countMobilityAll(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(1,countMobilitySafe(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(2,countAll);      
	BOOST_CHECK_EQUAL(1,countSafe);      
      }
      {
	const Offset o(1,0);
	BOOST_CHECK_EQUAL(1,countMobilityAll(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(1,countMobilitySafe(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(1,countAll);      
	BOOST_CHECK_EQUAL(1,countSafe);      
      }
    }
    // 53 馬
    {
      const Square pos(5,3);
      {
	const Offset o(-1,-1);
	BOOST_CHECK_EQUAL(1,countMobilityAll(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(0,countMobilitySafe(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(1,countAll);      
	BOOST_CHECK_EQUAL(0,countSafe);      
      }
      {
	const Offset o(-1,1);
	BOOST_CHECK_EQUAL(4,countMobilityAll(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(3,countMobilitySafe(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(4,countAll);      
	BOOST_CHECK_EQUAL(3,countSafe);      
      }
      {
	const Offset o(1,-1);
	BOOST_CHECK_EQUAL(2,countMobilityAll(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(2,countMobilitySafe(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(2,countAll);      
	BOOST_CHECK_EQUAL(2,countSafe);      
      }
      {
	const Offset o(1,1);
	BOOST_CHECK_EQUAL(1,countMobilityAll(BLACK,state,pos,o));
	BOOST_CHECK_EQUAL(1,countMobilitySafe(BLACK,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(BLACK,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(1,countAll);      
	BOOST_CHECK_EQUAL(1,countSafe);      
      }
    }
    // 77 馬
    {
      const Square pos(7,7);
      {
	const Offset o(-1,-1);
	BOOST_CHECK_EQUAL(1,countMobilityAll(WHITE,state,pos,o));
	BOOST_CHECK_EQUAL(1,countMobilitySafe(WHITE,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(WHITE,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(1,countAll);      
	BOOST_CHECK_EQUAL(1,countSafe);      
      }
      {
	const Offset o(-1,1);
	BOOST_CHECK_EQUAL(2,countMobilityAll(WHITE,state,pos,o));
	BOOST_CHECK_EQUAL(2,countMobilitySafe(WHITE,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(WHITE,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(2,countAll);      
	BOOST_CHECK_EQUAL(2,countSafe);      
      }
      {
	const Offset o(1,-1);
	BOOST_CHECK_EQUAL(2,countMobilityAll(WHITE,state,pos,o));
	BOOST_CHECK_EQUAL(1,countMobilitySafe(WHITE,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(WHITE,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(2,countAll);      
	BOOST_CHECK_EQUAL(1,countSafe);      
      }
      {
	const Offset o(1,1);
	BOOST_CHECK_EQUAL(2,countMobilityAll(WHITE,state,pos,o));
	BOOST_CHECK_EQUAL(0,countMobilitySafe(WHITE,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(WHITE,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(2,countAll);      
	BOOST_CHECK_EQUAL(0,countSafe);      
      }
    }
    // 11 香車
    {
      const Square pos(1,1);
      {
	const Offset o(0,1);
	BOOST_CHECK_EQUAL(1,countMobilityAll(WHITE,state,pos,o));
	BOOST_CHECK_EQUAL(1,countMobilitySafe(WHITE,state,pos,o));
	int countAll=0,countSafe=0;
	countMobilityBoth(WHITE,state,pos,o,countAll,countSafe);
	BOOST_CHECK_EQUAL(1,countAll);      
	BOOST_CHECK_EQUAL(1,countSafe);      
      }
    }
  }
}



/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
