#include "osl/move_generator/addEffect8Defense.h"
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

class AddEffect8DefenseTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(AddEffect8DefenseTest);
  CPPUNIT_TEST(testSimple0);
  CPPUNIT_TEST(testSuicide0);
  CPPUNIT_TEST(testTest11);
  CPPUNIT_TEST_SUITE_END();
public:
  void testSimple0();
  void testSuicide0();
  void testTest11();
};

void AddEffect8DefenseTest::
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
			       "P-00KI\n"
			       "P+00AL\n"
			       "-\n").initialState();
  NumEffectState state(sstate);
  {
    MoveVector moves;
    AddEffect8Defense<WHITE>::generate(state, moves);
    BOOST_CHECK_EQUAL((size_t)
			 (7 + 4	/*Drop GOLD*/
			 + 1	/*KING*/
			 + 3	/*GOLD*/
			 + 4	/*ROOK*/),
			 moves.size());
  }
}

void AddEffect8DefenseTest::
testSuicide0()
{
  SimpleState sstate=CsaString(
			       "P1 *  *  *  *  *  *  * -OU * \n"
			       "P2 *  *  *  *  *  *  *  *  * \n"
			       "P3 *  *  *  *  * -KI *  *  * \n"
			       "P4 *  *  *  *  *  *  *  *  * \n"
			       "P5 *  *  * +KA *  *  *  *  * \n"
			       "P6 *  *  *  *  *  *  *  *  * \n"
			       "P7 *  *  *  *  *  *  *  *  * \n"
			       "P8 *  * +OU *  *  *  *  *  * \n"
			       "P9 *  *  * +KY *  *  *  *  * \n"
			       "P+00AL\n"
			       "-\n").initialState();
  NumEffectState state(sstate);
  {
    MoveVector moves;
    AddEffect8Defense<WHITE>::generate(state, moves);
    BOOST_CHECK_EQUAL((size_t)3u, moves.size());
  }
}

void AddEffect8DefenseTest::
testTest11()
{
  CsaFile file(OslConfig::testFile("grimbergen_testset/test11.csa"));
  SimpleState sstate = file.initialState ();
  const osl::vector<Move> moves=file.load().moves();
  NumEffectState state(sstate);
  {
    MoveVector moves;
    AddEffect8Defense<BLACK>::generate(state, moves);
  }

}

CPPUNIT_TEST_SUITE_REGISTRATION(AddEffect8DefenseTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
