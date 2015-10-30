#include "osl/effect_util/virtualPin.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::effect_util;

BOOST_AUTO_TEST_CASE(VirtualPinTestFind) 
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  * -FU *  *  *  * \n"
			   "P2 * -OU * -KA-KI+NG+RY *  * \n"
			   "P3 * -FU-GI-RY *  *  *  *  * \n"
			   "P4 * -KY-FU-FU * -FU * +UM * \n"
			   "P5-FU *  * -KI * +FU *  * -FU\n"
			   "P6 *  * -KE+KE *  *  * -FU * \n"
			   "P7+FU+FU+GI+FU+FU *  *  * +FU\n"
			   "P8+KY * +KI *  * +GI *  *  * \n"
			   "P9+OU+KE+KI *  *  * +FU * +KY\n"
			   "P+00FU00FU\n"
			   "P-00FU\n"
			   "+\n").initialState());
    BOOST_CHECK(VirtualPin::find(state, WHITE, Square(5,1)));
    BOOST_CHECK(! VirtualPin::find(state, WHITE, Square(7,4)));
  }
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
