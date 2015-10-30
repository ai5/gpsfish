#include "osl/move_generator/captureEffectToAroundKing8.h"
#include "osl/move_action/store.h"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"
#include "osl/move.h"
#include "osl/container/moveVector.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

using namespace osl;
using namespace osl::move_action;
using namespace osl::move_generator;

#include <iterator>

class CaptureEffectToAroundKing8Test : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(CaptureEffectToAroundKing8Test);
  CPPUNIT_TEST(testSimple0);
  CPPUNIT_TEST(testCentering0);
  CPPUNIT_TEST(testTest11);
  CPPUNIT_TEST_SUITE_END();
public:
  void testSimple0();
  void testCentering0();
  void testTest11();
};

void CaptureEffectToAroundKing8Test::
testSimple0()
{
  SimpleState sstate=CsaString(
			       "P1 *  *  *  *  *  * -KE-OU * \n"
			       "P2 *  *  *  *  *  *  *  * -KI\n"
			       "P3 *  *  * -HI *  *  * +FU * \n"
			       "P4 *  * +KI *  *  *  *  *  * \n"
			       "P5 *  *  *  *  *  *  *  *  * \n"
			       "P6 *  *  *  *  *  *  *  *  * \n"
			       "P7 *  *  *  *  *  *  *  *  * \n"
			       "P8 *  * +OU *  *  *  *  *  * \n"
			       "P9 *  *  * +KY *  *  *  *  * \n"
			       "P+00KI\n"
			       "P-00AL\n"
			       "-\n").initialState();
  NumEffectState state(sstate);
  {
    MoveVector moves;
    CaptureEffectToAroundKing8<BLACK>::generate(state, moves);
    BOOST_CHECK_EQUAL((size_t)3u, moves.size());
  }

  {
    MoveVector moves;
    CaptureEffectToAroundKing8<WHITE>::generate(state, moves);
    BOOST_CHECK_EQUAL((size_t)3u, moves.size());
  }
}

void CaptureEffectToAroundKing8Test::
testCentering0()
{
  SimpleState sstate=CsaString(
			       "P1-OU-KE-GI *  *  *  * +HI * \n"
			       "P2 *  *  *  *  *  *  * -KI * \n"
			       "P3 *  *  *  *  *  *  * +FU * \n"
			       "P4 *  * +KI *  *  *  * -HI * \n"
			       "P5 *  *  *  *  *  *  *  *  * \n"
			       "P6 *  *  *  *  *  *  *  *  * \n"
			       "P7 *  *  *  *  *  *  *  *  * \n"
			       "P8 *  * +OU *  *  *  *  *  * \n"
			       "P9 *  *  * +KY *  *  *  *  * \n"
			       "P+00KI\n"
			       "P-00AL\n"
			       "-\n").initialState();
  NumEffectState state(sstate);
  {
    MoveVector moves;
    CaptureEffectToAroundKing8<WHITE>::generate(state, moves);
    BOOST_CHECK_EQUAL((size_t)2u, moves.size());
  }
}

void CaptureEffectToAroundKing8Test::
testTest11()
{
  CsaFile file(OslConfig::testFile("grimbergen_testset/test11.csa"));
  SimpleState sstate = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  NumEffectState state(sstate);
  {
    MoveVector moves;
    CaptureEffectToAroundKing8<BLACK>::generate(state, moves);
  }

}

CPPUNIT_TEST_SUITE_REGISTRATION(CaptureEffectToAroundKing8Test);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
