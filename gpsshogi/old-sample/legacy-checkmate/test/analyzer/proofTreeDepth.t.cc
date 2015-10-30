#include "osl/checkmate/analyzer/proofTreeDepth.h"
#include "osl/checkmate/dualCheckmateSearcher.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl;
using namespace osl::checkmate;
using namespace osl::checkmate::analyzer;

class ProofTreeDepthTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(ProofTreeDepthTest);
  CPPUNIT_TEST(testConstruction);
  CPPUNIT_TEST(testZero);
  CPPUNIT_TEST(testOne);
  CPPUNIT_TEST(testThree);
  CPPUNIT_TEST_SUITE_END();
public:
  void testConstruction() 
  {
    ProofTreeDepth depth_analyzer;
  }
  void testZero();
  void testOne();
  void testThree();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProofTreeDepthTest);

const size_t limit = 400000;
void ProofTreeDepthTest::testZero()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * +KI * \n"
    "P3 *  *  *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state(sstate);
  DualCheckmateSearcher<> checker(limit);
  const PathEncoding path(WHITE);
  const bool lose = checker.isLosingState(10, state, state.getHash(), path);
  CPPUNIT_ASSERT_EQUAL(true, lose);

  const CheckHashTable& table = checker.getTable(BLACK);
  const CheckHashRecord *record = table.find(state.getHash());
  ProofTreeDepth depth_analyzer;
  CPPUNIT_ASSERT_EQUAL(0, depth_analyzer.depth(record, false));
}

void ProofTreeDepthTest::testOne()
{
  SimpleState sstate=CsaString(
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
    "+\n").getInitialState();
  HashEffectState state(sstate);
  DualCheckmateSearcher<> checker(100);
  const PathEncoding path(BLACK);
  Move check_move;
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState(10, state, state.getHash(), path, 
					  check_move, oracle_age);
  CPPUNIT_ASSERT_EQUAL(true, win);

  const CheckHashTable& table = checker.getTable(BLACK);
  const CheckHashRecord *record = table.find(state.getHash());
  ProofTreeDepth depth_analyzer;
  CPPUNIT_ASSERT_EQUAL(1, depth_analyzer.depth(record, true));
}

void ProofTreeDepthTest::testThree()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  *  *  *  * \n"
    "P2 *  *  *  *  *  *  * -OU * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  * +FU * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI00KI\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state(sstate);
  DualCheckmateSearcher<> checker(100);
  const PathEncoding path(BLACK);
  Move check_move;
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState(50, state, state.getHash(), path, 
					  check_move, oracle_age);
  CPPUNIT_ASSERT_EQUAL(true, win);

  const CheckHashTable& table = checker.getTable(BLACK);
  const CheckHashRecord *record = table.find(state.getHash());
  ProofTreeDepth depth_analyzer;
  CPPUNIT_ASSERT_EQUAL(3, depth_analyzer.depth(record, true));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
