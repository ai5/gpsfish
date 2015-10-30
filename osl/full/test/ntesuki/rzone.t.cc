#include "osl/ntesuki/rzone.h"
#include "osl/record/csaString.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl::ntesuki;

namespace osl
{
  class RzoneTest : public CppUnit::TestFixture 
  {
    CPPUNIT_TEST_SUITE(RzoneTest);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testTest);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testSub);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST_SUITE_END();
  public:
    void testConstruct()
    {
      {
	SimpleState sstate(HIRATE);
	NumEffectState state(sstate);
	Rzone rzone(state, BLACK);
      }

      {
	SimpleState sstate = CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();;

	NumEffectState state(sstate);
	Rzone rzone(state, BLACK);
	BOOST_CHECK(!rzone.any());
      }

    }

    void testTest()
    {
      SimpleState sstate(HIRATE);
      NumEffectState state(sstate);

      Rzone rzone(state, BLACK);

      for (int x = 1; x <= 9; ++x)
      {
	for (int y = 1; y <= 9; ++y)
	{
	  Square pos(x, y);
	  if (x ==5 && y == 9)
	  {
	    BOOST_CHECK(rzone.test(pos));
	  }
	  else
	  {
	    BOOST_CHECK(!rzone.test(pos));
	  }
	}
      }
    }

    void testAdd()
    {
      Rzone rzone1(Square(5,9));
      Rzone rzone2(Square(5,8));
      Rzone rzone = rzone1 + rzone2;

      BOOST_CHECK(rzone.test(Square(5,8)));
      BOOST_CHECK(rzone.test(Square(5,9)));
      BOOST_CHECK(!rzone.test(Square(4,8)));
      BOOST_CHECK(!rzone.test(Square(4,9)));
      BOOST_CHECK(!rzone.test(Square(6,8)));
      BOOST_CHECK(!rzone.test(Square(6,9)));
      BOOST_CHECK(!rzone.test(Square(5,7)));
    }

    void testSub()
    {
      Rzone rzone1(Square(5,9));
      Rzone rzone2(Square(5,8));
      Rzone rzone = rzone1 + rzone2;

      bool exception_handled = false;
      try
      {
	Rzone rzone0 = rzone2 - rzone1;
      }
      catch (DfpnError& e)
      {
	exception_handled = true;
      }
      BOOST_CHECK(exception_handled);

      Rzone rzone0 = rzone - rzone1;
      BOOST_CHECK(rzone0 == rzone2);
    }

    void testUpdate()
    {
      Rzone rzone1(Square(5,9));
      Rzone rzone2(Square(5,8));
      Rzone rzone3(Square(5,7));

      rzone1 = rzone1 + rzone3;
      rzone2 = rzone2 + rzone3;
      Rzone rzone = rzone1.update(rzone2);

      BOOST_CHECK(!rzone.test(Square(5,7)));
      BOOST_CHECK(rzone.test(Square(5,8)));
      BOOST_CHECK(!rzone.test(Square(5,9)));
      BOOST_CHECK(rzone1.test(Square(5,7)));
      BOOST_CHECK(rzone1.test(Square(5,8)));
      BOOST_CHECK(rzone1.test(Square(5,9)));
    }
  };
} // namespace osl

CPPUNIT_TEST_SUITE_REGISTRATION(osl::RzoneTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
