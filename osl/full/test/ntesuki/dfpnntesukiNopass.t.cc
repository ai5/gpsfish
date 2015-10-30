/* dfpnntesuki.cc
 */
#include "osl/ntesuki/dfpnNtesukiNopass.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"
#include "osl/move_generator/legalMoves.h"
#include "osl/move_generator/escape.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

class dfpnNtesukiNopassTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(dfpnNtesukiNopassTest);

  CPPUNIT_TEST(test0tesuki0);
  CPPUNIT_TEST(test1tesuki0);
  CPPUNIT_TEST(test2tesuki0);
  CPPUNIT_TEST(test3tesuki0);
  CPPUNIT_TEST(test0tesuki1);
  CPPUNIT_TEST(test1tesuki1);
  CPPUNIT_TEST(test2tesuki1);
  CPPUNIT_TEST(test3tesuki1);
  //CPPUNIT_TEST(test2tesukiWithEscape);

  CPPUNIT_TEST_SUITE_END();
public:
  void test0tesuki0();
  void test0tesuki1();
  void test1tesuki0();
  void test1tesuki1();
  void test2tesuki0();
  void test2tesuki1();
  void test3tesuki0();
  void test3tesuki1();
  //void test2tesukiWithEscape();
};
CPPUNIT_TEST_SUITE_REGISTRATION(dfpnNtesukiNopassTest);

using namespace osl;
using namespace osl::ntesuki;
const size_t limit = 40000;

typedef  GetMultipleAttackMoves gam;
//typedef  GetAttackMoves gam;
//typedef  GetAllAttackMoves gam;

typedef  GetNoMoves gdm;

static void results()
{
#if 0
  if (OslConfig::verbose())
  {
    std::cerr << "Move reduction(all/used):\t" << gam::allMovesCount
	      << "\t" << gam::reducedMovesCount
	      << "\n";
  }
  gam::allMovesCount = 0;
  gam::reducedMovesCount = 0;
#endif
}

void dfpnNtesukiNopassTest::test0tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(0, ntesuki_num);
}

void dfpnNtesukiNopassTest::test0tesuki1()
{
  SimpleState state=CsaString(
    "P1-KY-KE-GI-KI-OU-KI+UM-KE-KY\n"
    "P2 * -HI *  * +KA *  *  *  * \n"
    "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 *  *  *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P+00GI\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(0, ntesuki_num);
}

void dfpnNtesukiNopassTest::test1tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void dfpnNtesukiNopassTest::test1tesuki1()
{
  SimpleState state=CsaString(
    "P1-KY-KE-GI-KI-OU-KI+UM-KE-KY\n"
    "P2 * -HI *  *  *  *  *  *  * \n"
    "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 *  *  *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P+00GI00KA\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(1, ntesuki_num);
}

void dfpnNtesukiNopassTest::test2tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  * +FU * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(2, ntesuki_num);
}

void dfpnNtesukiNopassTest::test2tesuki1()
{
  SimpleState state=CsaString(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * +UM * \n"
    "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 *  *  *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P+00KA\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose());

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(2, ntesuki_num);
}

void dfpnNtesukiNopassTest::test3tesuki0()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  * +FU * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose(), 4);

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(3, ntesuki_num);
}

void dfpnNtesukiNopassTest::test3tesuki1()
{
  SimpleState state=CsaString(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
    "P4 *  *  *  *  *  * -FU *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  * +FU *  *  *  *  *  * \n"
    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "+\n").initialState();
  HashEffectState hState(state);
  DfpnNtesukiNopassSearcher<HashEffectState, gam, gdm> searcher (hState, limit, OslConfig::verbose(), 4);

  const int ntesuki_num = searcher.searchSlow(hState.turn());
  results();
  BOOST_CHECK_EQUAL(3, ntesuki_num);
}
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
