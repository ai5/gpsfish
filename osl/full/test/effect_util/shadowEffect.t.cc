#include "osl/effect_util/shadowEffect.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::effect_util;

BOOST_AUTO_TEST_CASE(ShadowEffectTestHasEffect)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-FU-KA-FU-FU\n"
			   "P4 *  *  *  *  *  * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  *  *  *  *  * \n"
			   "P7+FU+FU+KA+FU+FU+FU+FU+FU+FU\n"
			   "P8 *  *  *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "+\n"
			   ).initialState());
    BOOST_CHECK_EQUAL(false, ShadowEffect::hasEffect
			 (state, Square(8,8), BLACK));
    BOOST_CHECK_EQUAL(true, ShadowEffect::hasEffect
			 (state, Square(8,8), WHITE));

    BOOST_CHECK_EQUAL(true, ShadowEffect::hasEffect
			 (state, Square(2,2), BLACK));
    BOOST_CHECK_EQUAL(false, ShadowEffect::hasEffect
			 (state, Square(2,2), WHITE));
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
