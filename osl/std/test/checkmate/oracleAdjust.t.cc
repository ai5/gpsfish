/* oracleAdjust.t.cc
 */
#include "osl/checkmate/oracleAdjust.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(OracleAdjustTestPromote)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE+UM *  *  *  *  * -KY\n"
			   "P2 * -HI *  *  *  * -KI-OU * \n"
			   "P3 *  *  * -GI *  * -KE-FU * \n"
			   "P4 *  *  *  * -FU * +FU * -FU\n"
			   "P5-FU *  * +FU *  *  * +FU * \n"
			   "P6 *  *  * -RY+FU * -FU * +FU\n"
			   "P7+FU+GI+KI *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  * +KY\n"
			   "P9+KY * +OU *  *  *  * +KE * \n"
			   "P+00KA00KI00GI00GI00KE00FU00FU00FU00FU00FU00FU00FU\n"
			   "P-00KI\n"
			   "-\n").initialState());
    const Move oracle(Square(7,5),Square(7,7),PROOK,GOLD,true,WHITE);
    const Move adjusted = OracleAdjust::attack(state, oracle);
    const Move expected(Square(6,6),Square(7,7),PROOK,GOLD,false,WHITE);
    BOOST_CHECK_EQUAL(expected, adjusted);
  }
}

BOOST_AUTO_TEST_CASE(OracleAdjustTestSelection)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY * +UM * +RY *  * -KE * \n"
			   "P2 *  *  *  * -HI *  *  *  * \n"
			   "P3 * -GI-OU *  *  *  * -KI-KY\n"
			   "P4-FU-FU-FU-FU * +UM * -FU-FU\n"
			   "P5 *  *  * -KE * +FU *  *  * \n"
			   "P6+FU+FU+FU *  *  *  *  * +FU\n"
			   "P7 *  * +GI+KI *  * +KE *  * \n"
			   "P8+KY+GI+KI *  *  *  *  *  * \n"
			   "P9+OU+KE *  *  * -TO *  * +KY\n"
			   "P+00KI00FU00FU00FU00FU\n"
			   "P-00GI00FU00FU\n"
			   "+\n"
			   ).initialState());
    const Move oracle(Square(3,5),Square(6,2),PBISHOP,PTYPE_EMPTY,false,BLACK);
    const Move adjusted = OracleAdjust::attack(state, oracle);
    // 7,1 の方を動かすと無限ループ
    const Move expected(Square(4,4),Square(6,2),PBISHOP,PTYPE_EMPTY,false,BLACK);
    BOOST_CHECK_EQUAL(expected, adjusted);
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
