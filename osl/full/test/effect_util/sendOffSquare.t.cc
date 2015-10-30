#include "osl/effect_util/sendOffSquare.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
using namespace osl;
using namespace osl::effect_util;

typedef osl::effect_util::SendOffSquare::SendOff8 SendOff8;

BOOST_AUTO_TEST_CASE(SendOffSquareTestSimple)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * +HI *  * \n"
			   "P2 *  * -OU-KI *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    Square8 sendoffs, sendoffs2;
    Square king(7,2);
    SendOff8 data = SendOffSquare::find<BLACK>(state, king, sendoffs);
    BOOST_CHECK(sendoffs.empty());
    SendOffSquare::unpack(data, king, sendoffs2);
    BOOST_CHECK_EQUAL(sendoffs, sendoffs2);
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  * -OU-KI *  * +HI *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    Square8 sendoffs, sendoffs2;
    Square king(7,2);
    SendOff8 data = SendOffSquare::find<BLACK>(state, king, sendoffs);
    BOOST_CHECK_EQUAL((size_t)3, sendoffs.size());
    SendOffSquare::unpack(data, king, sendoffs2);
    BOOST_CHECK_EQUAL(sendoffs, sendoffs2);
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  * -KI *  * +HI *  * \n"
			   "P3 *  * -OU *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    Square8 sendoffs, sendoffs2;
    Square king(7,3);
    SendOff8 data = SendOffSquare::find<BLACK>(state, king, sendoffs);
    BOOST_CHECK_EQUAL((size_t)5, sendoffs.size());
    SendOffSquare::unpack(data, king, sendoffs2);
    BOOST_CHECK_EQUAL(sendoffs, sendoffs2);
  }
}
BOOST_AUTO_TEST_CASE(SendOffSquareTestDefense)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  * -KI *  * +HI *  * \n"
			   "P3 *  * -OU *  *  *  *  *  * \n"
			   "P4 *  * -KI *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    Square8 sendoffs, sendoffs2;
    Square king(7,3);
    SendOff8 data = SendOffSquare::find<BLACK>(state, king, sendoffs);
#if 0
    for (Square8::const_iterator p=sendoffs.begin(); p!=sendoffs.end(); ++p)
    {
      std::cerr << *p << "\n";
    }
#endif
    BOOST_CHECK_EQUAL((size_t)3, sendoffs.size()); // 82, 83, 72
    SendOffSquare::unpack(data, king, sendoffs2);
    BOOST_CHECK_EQUAL(sendoffs, sendoffs2);
  }
}

BOOST_AUTO_TEST_CASE(SendOffSquareTestEdge)
{
  {
    NumEffectState state(CsaString(
			   "P1 * -KI * +HI *  *  *  *  * \n"
			   "P2-OU *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    Square8 sendoffs, sendoffs2;
    Square king(9,2);
    SendOff8 data = SendOffSquare::find<BLACK>(state, king, sendoffs);
    BOOST_CHECK_EQUAL((size_t)2, sendoffs.size()); // 93, 83
    SendOffSquare::unpack(data, king, sendoffs2);
    BOOST_CHECK_EQUAL(sendoffs, sendoffs2);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
