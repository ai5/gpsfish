/* mobilityTable.t.cc
 */
#include "osl/mobility/mobilityTable.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::mobility;

BOOST_AUTO_TEST_CASE(MobilityTableTestCount)
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
    MobilityTable mt(state);
    for(int num=32;num<=39;num++){
      if(state.pieceOf(num).square()==Square(1,2)){
	// 11 KY
	BOOST_CHECK_EQUAL(Square::STAND(),mt.get(U,num));
	BOOST_CHECK_EQUAL(Square::STAND(),mt.get(L,num));
	BOOST_CHECK_EQUAL(Square::STAND(),mt.get(R,num));
	BOOST_CHECK_EQUAL(Square(1,3),mt.get(D,num));
      }
      else if(state.pieceOf(num).square()==Square(9,1)){
	// 99 NY
	BOOST_CHECK_EQUAL(Square::STAND(),mt.get(U,num));
	BOOST_CHECK_EQUAL(Square::STAND(),mt.get(L,num));
	BOOST_CHECK_EQUAL(Square::STAND(),mt.get(R,num));
	BOOST_CHECK_EQUAL(Square::STAND(),mt.get(D,num));
      }
      else if(state.pieceOf(num).square()==Square(5,3)){
	// 53 馬
	BOOST_CHECK_EQUAL(Square(7,1),mt.get(UL,num));
	BOOST_CHECK_EQUAL(Square(4,2),mt.get(UR,num));
	BOOST_CHECK_EQUAL(Square(6,4),mt.get(DL,num));
	BOOST_CHECK_EQUAL(Square(1,7),mt.get(DR,num));
      }
      else if(state.pieceOf(num).square()==Square(7,7)){
	// 77 馬
	BOOST_CHECK_EQUAL(Square(9,5),mt.get(UL,num));
	BOOST_CHECK_EQUAL(Square(6,6),mt.get(UR,num));
	BOOST_CHECK_EQUAL(Square(9,9),mt.get(DL,num));
	BOOST_CHECK_EQUAL(Square(5,9),mt.get(DR,num));
      }
      else if(state.pieceOf(num).square()==Square(8,3)){
	// 83 竜
	BOOST_CHECK_EQUAL(Square(8,1),mt.get(U,num));
	BOOST_CHECK_EQUAL(Square(9,3),mt.get(L,num));
	BOOST_CHECK_EQUAL(Square(5,3),mt.get(R,num));
	BOOST_CHECK_EQUAL(Square(8,9),mt.get(D,num));
      }
    }
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
