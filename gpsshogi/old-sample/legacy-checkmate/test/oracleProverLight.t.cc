/* oracleProverLightTest.cc
 */
#include "osl/checkmate/oracleProverLight.h"
#include "osl/checkmate/dualCheckmateSearcher.h"
#include "osl/checkmate/analyzer/checkTableAnalyzer.h"
#include "osl/checkmate/checkmateRecorder.h"
#include "osl/record/csaString.h"
#include "osl/state/hashEffectState.h"
#include "osl/apply_move/applyMove.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <fstream>
using namespace osl;
using namespace osl::checkmate;

class OracleProverLightTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(OracleProverLightTest);
  CPPUNIT_TEST(testCheckLong);
  CPPUNIT_TEST(testDistance);
  CPPUNIT_TEST(testNoCheck);
  CPPUNIT_TEST(testCheck);
  CPPUNIT_TEST(testCheckAfterAttack);
  CPPUNIT_TEST(testPawnCheckmate);
  CPPUNIT_TEST(testPawnCheckmateLose);
  CPPUNIT_TEST(testAdjustMove);
  CPPUNIT_TEST(testInvalidDefense);
  CPPUNIT_TEST(testInvalidDefenseLose);
  CPPUNIT_TEST_SUITE_END();
public:
  void testCheckLong();
  void testInvalidDefense();
  void testInvalidDefenseLose();
  void testDistance();
  void testAdjustMove();
  void testNoCheck();
  void testCheck();
  void testPawnCheckmate();
  void testPawnCheckmateLose();
  void testCheckAfterAttack();
};
CPPUNIT_TEST_SUITE_REGISTRATION(OracleProverLightTest);

extern bool isShortTest;

const size_t limit = 400000;
static bool isCheckmate(const char *org, const char *similar)
{
  SimpleState state=CsaString(org).getInitialState();
  HashEffectState e_state(state);

  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(! isShortTest);
  CheckHashTable& table = checker.getTable(state.turn());
  Move check_move;
  PathEncoding path(state.turn());
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState(limit, e_state, e_state.getHash(),
					  path, check_move, oracle_age);
  CPPUNIT_ASSERT(win);
  const CheckHashRecord *record = table.find(e_state.getHash());
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT(record->proofDisproof().isCheckmateSuccess());

  SimpleState state2=CsaString(similar).getInitialState();
  HashEffectState e_state2(state2);
  Move best_move;
  if (e_state2.turn() == BLACK)
  {
    OracleProverLight<BLACK> prover;
    ProofOracleAttack<BLACK> oracle(record);
    return prover.proofWin(e_state2, oracle, best_move);
  }
  else
  {
    OracleProverLight<WHITE> prover;
    ProofOracleAttack<WHITE> oracle(record);
    return prover.proofWin(e_state2, oracle, best_move);
  }
}

void OracleProverLightTest::testCheckLong()
{
  CPPUNIT_ASSERT(! isCheckmate(
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P+00KE\n"
		   "P-00AL\n"
		   "+\n",
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -FU-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P+00KE\n"
		   "P-00AL\n"
		   "+\n"));
  CPPUNIT_ASSERT(isCheckmate(
		   "P1-KY-KE+GI *  *  *  *  * -KY\n"
		   "P2-OU-KE-GI * -KI *  *  *  * \n"
		   "P3 * -FU *  *  *  * +TO * -FU\n"
		   "P4+FU * -FU-GI *  *  *  *  * \n"
		   "P5 *  *  *  *  *  * -KA *  * \n"
		   "P6 *  * +FU+FU-FU *  *  *  * \n"
		   "P7+KE+FU+KA+KI *  *  *  * +FU\n"
		   "P8 *  * +OU *  * +KI *  *  * \n"
		   "P9+KY *  *  * +FU *  * +KE+KY\n"
		   "P+00FU00FU00FU00FU00FU00GI00HI\n"
		   "P-00FU00FU00KI00HI\n"
		   "+\n",
		   "P1-KY-KE+GI *  *  *  *  * -KY\n"
		   "P2-OU-KE-GI * -KI *  *  *  * \n"
		   "P3 * -FU *  *  *  * +TO * -FU\n"
		   "P4+FU * -FU-GI *  *  *  *  * \n"
		   "P5 *  *  *  *  *  * -KA *  * \n"
		   "P6 *  * +FU+FU-FU *  *  *  * \n"
		   "P7+KE+FU+KA+KI *  *  *  * +FU\n"
		   "P8 *  * +OU * +FU+KI *  *  * \n"
		   "P9+KY *  *  *  *  *  * +KE+KY\n"
		   "P+00FU00FU00FU00FU00FU00GI00HI\n"
		   "P-00FU00FU00KI00HI\n"
		   "+\n"));
  CPPUNIT_ASSERT(isCheckmate(
		   "P1 *  *  *  * -KI *  *  * -KY\n"
		   "P2 * +NG+TO *  * -KI-OU *  * \n"
		   "P3 *  *  * -FU-FU-FU-KE+FU-FU\n"
		   "P4-FU *  *  * -KE * -KY-GI * \n"
		   "P5 *  *  * +UM *  *  *  *  * \n"
		   "P6+FU *  *  * +HI *  *  *  * \n"
		   "P7 * +FU * +FU+FU+FU+FU * +FU\n"
		   "P8 *  *  *  *  * +OU *  *  * \n"
		   "P9+KY+KE-TO *  * +KI * +GI+KY\n"
		   "P-00FU00FU00FU00KE00GI00KI00KA\n"
		   "P+00HI\n"
		   "-\n",
		   "P1 *  *  *  * -KI *  *  * -KY\n"
		   "P2 * +NG+TO *  * -KI-OU *  * \n"
		   "P3 *  *  * -FU-FU-FU-KE+FU-FU\n"
		   "P4-FU *  *  * -KE * -KY-GI * \n"
		   "P5 *  *  * +UM *  *  *  *  * \n"
		   "P6+FU *  *  * +HI *  *  *  * \n"
		   "P7 * +FU * +FU+FU+FU+FU * +FU\n"
		   "P8+KY *  *  *  * +OU *  *  * \n"
		   "P9 * +KE-TO *  * +KI * +GI+KY\n"
		   "P-00FU00FU00FU00KE00GI00KI00KA\n"
		   "P+00HI\n"
		   "-\n"));
  CPPUNIT_ASSERT(isCheckmate(
		   "P1 *  *  * -KI *  *  *  * -KY\n"
		   "P2 * +KI-GI *  *  *  *  *  * \n"
		   "P3+GI-FU-FU *  *  *  *  * -FU\n"
		   "P4-FU * -KE *  * -FU-FU-FU * \n"
		   "P5 *  * +KA *  *  *  *  *  * \n"
		   "P6 *  *  *  * -FU *  *  *  * \n"
		   "P7 * +FU+KE+FU * +KA-OU * +FU\n"
		   "P8 *  * +OU+GI-NG *  *  *  * \n"
		   "P9 *  * +KI *  * +KY *  * +KY\n"
		   "P-00FU00FU00FU00FU00FU00FU00KE00KI00KY00HI\n"
		   "P+00FU00KE00HI\n"
		   "+\n",
		   "P1 *  *  * -KI *  *  *  * -KY\n"
		   "P2 * +KI-GI *  *  *  *  *  * \n"
		   "P3+GI-FU-FU *  *  *  *  * -FU\n"
		   "P4-FU * -KE *  * -FU-FU-FU * \n"
		   "P5 *  * +KA *  *  *  *  *  * \n"
		   "P6 *  *  *  * -FU *  *  *  * \n"
		   "P7+FU * +KE+FU * +KA-OU * +FU\n"
		   "P8 *  * +OU+GI-NG *  *  *  * \n"
		   "P9 *  * +KI *  * +KY *  * +KY\n"
		   "P-00FU00FU00FU00FU00FU00FU00KE00KI00KY00HI\n"
		   "P+00FU00KE00HI\n"
		   "+\n"));
  CPPUNIT_ASSERT(isCheckmate(
		   "P1-KY *  * +TO-KI-OU *  *  * \n"
		   "P2 *  * +RY-GI *  * -KI *  * \n"
		   "P3-FU * -FU-FU *  *  * -GI-FU\n"
		   "P4 *  *  *  * -KE-FU *  *  * \n"
		   "P5 *  *  *  *  * -KE-FU *  * \n"
		   "P6 * +FU+FU *  *  *  *  * +FU\n"
		   "P7+FU+OU * +FU * +FU+GI *  * \n"
		   "P8 *  *  *  * -RY *  *  *  * \n"
		   "P9+KY+KE+KE *  *  * -UM * +KY\n"
		   "P+00FU00FU00FU00GI00KY00KA\n"
		   "P-00FU00FU00KI00KI\n"
		   "-\n",
		   "P1-KY *  * +TO-KI-OU *  *  * \n"
		   "P2 *  * +RY-GI * -KI *  *  * \n"
		   "P3-FU * -FU-FU *  *  * -GI-FU\n"
		   "P4 *  *  *  * -KE-FU *  *  * \n"
		   "P5 *  *  *  *  * -KE-FU *  * \n"
		   "P6 * +FU+FU *  *  *  *  * +FU\n"
		   "P7+FU+OU * +FU * +FU+GI *  * \n"
		   "P8 *  *  *  * -RY *  *  *  * \n"
		   "P9+KY+KE+KE *  *  * -UM * +KY\n"
		   "P+00FU00FU00FU00GI00KY00KA\n"
		   "P-00FU00FU00KI00KI\n"
		   "-\n"));
  CPPUNIT_ASSERT(isCheckmate(
		   "P1-KY * -OU-KI *  *  *  * -KY\n"
		   "P2 *  *  *  *  *  * -KI *  * \n"
		   "P3-FU-FU+NK * -FU-FU *  *  * \n"
		   "P4 *  *  *  *  * -HI *  * -FU\n"
		   "P5+FU+FU * -GI * -KE-KE *  * \n"
		   "P6+HI *  *  * +FU+KI * -FU+FU\n"
		   "P7 *  * -FU+FU+KA * +FU *  * \n"
		   "P8 *  * +KI *  *  *  * +FU * \n"
		   "P9+KY *  * +OU *  *  * +KE+KY\n"
		   "P-00FU00FU00FU00GI00GI\n"
		   "P+00FU00GI00KA\n"
		   "-\n",
		   "P1-KY * -OU-KI *  *  *  * -KY\n"
		   "P2 *  *  *  *  *  * -KI *  * \n"
		   "P3-FU-FU+NK * -FU-FU *  *  * \n"
		   "P4 *  *  *  *  * -HI *  * -FU\n"
		   "P5+FU+FU * -GI * -KE-KE *  * \n"
		   "P6+HI *  *  * +FU+KI * -FU+FU\n"
		   "P7 *  * -FU+FU+KA * +FU *  * \n"
		   "P8 *  * +KI *  *  *  * +FU * \n"
		   "P9+KY *  * +OU *  *  * +KE+KY\n"
		   "P-00FU00FU00FU00GI00GI\n"
		   "P+00FU00GI00KA\n"
		   "-\n"));

}

void OracleProverLightTest::testInvalidDefense()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  *  * -KA-KY\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KE\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state(state);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  Move check_move;
  PathEncoding path(BLACK);
  AttackOracleAges oracle_age;
  const bool checkmate = checker.isWinningState<BLACK>
    (10, e_state, e_state.getHash(), path, check_move, oracle_age);
  CPPUNIT_ASSERT_EQUAL(true, checkmate);
  const Move m23KE = Move(Square(2,3),KNIGHT,BLACK);
  CPPUNIT_ASSERT_EQUAL(m23KE, check_move);

  SimpleState state2=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-KA-KY\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KE\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state2(state2);
  CheckHashTable& table = checker.getTable(BLACK);
  OracleProverLight<BLACK> prover;
  const CheckHashRecord *record = table.find(e_state.getHash());
  ProofOracleAttack<BLACK> oracle(record);
  
  const bool result2 = prover.proofWin(e_state2, oracle, check_move);
  CPPUNIT_ASSERT_EQUAL(true, result2);
  CPPUNIT_ASSERT_EQUAL(m23KE, check_move);
#if 0
  // ImmediateCheckmate では無理
  // さっきと違って -2223FU と取れるが無効. SafeMoveFilter が入れば解決?
  SimpleState state3=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-FU-KY\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  * +KA *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KE\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state3(state3);

  const bool result3 = prover.proofWin(e_state3, oracle, check_move);
  CPPUNIT_ASSERT_EQUAL(true, result3);
  CPPUNIT_ASSERT_EQUAL(m23KE, check_move);
#endif
  SimpleState state4=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-FU-KY\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  * +HI *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KE\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state4(state4);

  const bool result4 = prover.proofWin(e_state4, oracle, check_move);
  CPPUNIT_ASSERT_EQUAL(false, result4);
}

void OracleProverLightTest::testInvalidDefenseLose()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  *  * -KA-KY\n"
    "P3 *  *  *  *  *  *  * +KE * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState e_state(state);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  PathEncoding path(WHITE);
  const Move last_move = Move(Square(2,3),KNIGHT,BLACK);
  const bool checkmate = checker.isLosingState<WHITE>
    (10, e_state, e_state.getHash(), path, last_move);
  CPPUNIT_ASSERT_EQUAL(true, checkmate);

  SimpleState state2=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-KA-KY\n"
    "P3 *  *  *  *  *  *  * +KE * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState e_state2(state2);
  CheckHashTable& table = checker.getTable(BLACK);
  OracleProverLight<BLACK> prover;
  const CheckHashRecord *record = table.find(e_state.getHash());
  ProofOracleDefense<BLACK> oracle(record);
  
  const bool result2 = prover.proofLose(e_state2, oracle, last_move);
  CPPUNIT_ASSERT_EQUAL(true, result2);

  // さっきと違って -2223FU と取れるが無効. SafeMoveFilter が入れば解決?
  SimpleState state3=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-FU-KY\n"
    "P3 *  *  *  *  *  *  * +KE * \n"
    "P4 *  *  *  *  * +KA *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState e_state3(state3);

  const bool result3 = prover.proofLose(e_state3, oracle, last_move);
  CPPUNIT_ASSERT_EQUAL(true, result3);

  SimpleState state4=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-FU-KY\n"
    "P3 *  *  *  *  *  *  * +KE * \n"
    "P4 *  *  *  *  * +HI *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState e_state4(state4);

  const bool result4 = prover.proofLose(e_state4, oracle, last_move);
  CPPUNIT_ASSERT_EQUAL(false, result4);
}

void OracleProverLightTest::testDistance()
{
  // bug 35
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
    "+\n").getInitialState();
  HashEffectState e_state(state);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  Move check_move = Move::INVALID();
  const int distance = 3;
  const PathEncoding path(BLACK, distance);
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<BLACK>(10, e_state, e_state.getHash(), path, check_move,
						 oracle_age);
  CPPUNIT_ASSERT_EQUAL(true, win);
  CPPUNIT_ASSERT_EQUAL(Move(Square(2,2),GOLD,BLACK), check_move);

  SimpleState stateSimilar=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  * -FU+FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_stateSimilar(stateSimilar);

  CheckHashTable& table = checker.getTable(BLACK);
  OracleProverLight<BLACK> prover;
  const CheckHashRecord *record = table.find(e_state.getHash());
  ProofOracleAttack<BLACK> oracle(record);

  Move checkmate_move2 = Move::INVALID();
  const int distanceSimilar = 30;
  const PathEncoding pathSimilar(BLACK, distanceSimilar);
  const bool result = prover.proofWin(e_stateSimilar, oracle, checkmate_move2);
  CPPUNIT_ASSERT_EQUAL(check_move, checkmate_move2);
  CPPUNIT_ASSERT_EQUAL(true, result);
}

void OracleProverLightTest::testAdjustMove()
{
  const Move m1 = 
    Move(Square(8,9),Square(7,9),KING,PTYPE_EMPTY,false,BLACK);
  Move m2 = m1;
  m2.setCapturePtype(PAWN);
  CPPUNIT_ASSERT(m1 != m2);
  CPPUNIT_ASSERT_EQUAL(m1.from(), m2.from());
  CPPUNIT_ASSERT_EQUAL(m1.to(), m2.to());
  CPPUNIT_ASSERT_EQUAL(m1.ptype(), m2.ptype());
  CPPUNIT_ASSERT_EQUAL(m1.promoteMask(), m2.promoteMask());
  CPPUNIT_ASSERT_EQUAL(m1.player(), m2.player());

  m2.setCapturePtype(ROOK);
  CPPUNIT_ASSERT(m1 != m2);
  CPPUNIT_ASSERT_EQUAL(m1.from(), m2.from());
  CPPUNIT_ASSERT_EQUAL(m1.to(), m2.to());
  CPPUNIT_ASSERT_EQUAL(m1.ptype(), m2.ptype());
  CPPUNIT_ASSERT_EQUAL(m1.promoteMask(), m2.promoteMask());
  CPPUNIT_ASSERT_EQUAL(m1.player(), m2.player());

  m2.setCapturePtype(PPAWN);
  CPPUNIT_ASSERT(m1 != m2);
  CPPUNIT_ASSERT_EQUAL(m1.from(), m2.from());
  CPPUNIT_ASSERT_EQUAL(m1.to(), m2.to());
  CPPUNIT_ASSERT_EQUAL(m1.ptype(), m2.ptype());
  CPPUNIT_ASSERT_EQUAL(m1.promoteMask(), m2.promoteMask());
  CPPUNIT_ASSERT_EQUAL(m1.player(), m2.player());

  m2.setCapturePtype(PTYPE_EMPTY);
  CPPUNIT_ASSERT(m1 == m2);
  CPPUNIT_ASSERT_EQUAL(m1.from(), m2.from());
  CPPUNIT_ASSERT_EQUAL(m1.to(), m2.to());
  CPPUNIT_ASSERT_EQUAL(m1.ptype(), m2.ptype());
  CPPUNIT_ASSERT_EQUAL(m1.promoteMask(), m2.promoteMask());
  CPPUNIT_ASSERT_EQUAL(m1.player(), m2.player());

  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE * +KE\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state(state);

  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  Move checkmate_move = Move::INVALID();
  PathEncoding path(BLACK);
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<BLACK>
    (500, e_state, e_state.getHash(), path, checkmate_move, oracle_age);
  CPPUNIT_ASSERT(win);
  CPPUNIT_ASSERT(checkmate_move != Move::INVALID());
  const CheckHashRecord *record = table.find(e_state.getHash());
  CPPUNIT_ASSERT(record);

  // 取る駒が違っても詰む
  SimpleState state2=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * -KI * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE * +KE\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state2(state2);

  Move checkmate_move2 = Move::INVALID();
  OracleProverLight<BLACK> prover;
  ProofOracleAttack<BLACK> oracle(record);
  const bool result = prover.proofWin(e_state2, oracle, checkmate_move2);
  CPPUNIT_ASSERT_EQUAL(true, result);
  CPPUNIT_ASSERT_EQUAL(checkmate_move.from(), checkmate_move2.from());
  CPPUNIT_ASSERT_EQUAL(checkmate_move.to(), checkmate_move2.to());
  CPPUNIT_ASSERT_EQUAL(checkmate_move.ptype(), checkmate_move2.ptype());

  {
    // 初期位置が違っても詰む
    HashEffectState state(CsaString(
			   "P1 *  *  *  * +HI *  * -KI-OU\n"
			   "P2 *  *  *  *  *  *  * -FU-FU\n"
			   "P3 *  *  *  *  * +KI+FU *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").getInitialState());

    DualCheckmateSearcher<> checker(limit);
    checker.setVerbose(!isShortTest);
    CheckHashTable& table = checker.getTable(BLACK);
    Move checkmate_move;
    PathEncoding path(BLACK);
    AttackOracleAges oracle_age;
    const bool win = checker.isWinningState<BLACK>
      (500, state, state.getHash(), path, checkmate_move, oracle_age);
    CPPUNIT_ASSERT(win);
    CPPUNIT_ASSERT(checkmate_move.isNormal());
    const CheckHashRecord *oracle = table.find(state.getHash());
    CPPUNIT_ASSERT(oracle);

    // 飛車移動
    HashEffectState state2(CsaString(
			   "P1 *  *  * +HI *  *  * -KI-OU\n"
			   "P2 *  *  *  *  *  *  * -FU-FU\n"
			   "P3 *  *  *  *  * +KI+FU *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").getInitialState());

    Move checkmate_move2;
    OracleProverLight<BLACK> prover;
    const bool result = prover.proofWin(state2, ProofOracleAttack<BLACK>(oracle), checkmate_move2);
    CPPUNIT_ASSERT_EQUAL(true, result);
    CPPUNIT_ASSERT_EQUAL(checkmate_move.to(), checkmate_move2.to());
    CPPUNIT_ASSERT_EQUAL(checkmate_move.ptype(), checkmate_move2.ptype());
    CPPUNIT_ASSERT_EQUAL(checkmate_move.capturePtype(), checkmate_move2.capturePtype());
    CPPUNIT_ASSERT(checkmate_move.from() != checkmate_move2.from());
    CPPUNIT_ASSERT_EQUAL(Square(6,1), checkmate_move2.from());
  }
}

void OracleProverLightTest::testNoCheck()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE * +KE\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state(state);

  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  Move checkmate_move = Move::INVALID();
  PathEncoding path(BLACK);
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<BLACK>
    (500, e_state, e_state.getHash(), path, checkmate_move, oracle_age);
  CPPUNIT_ASSERT(win);
  CPPUNIT_ASSERT(checkmate_move != Move::INVALID());
  const CheckHashRecord *record = table.find(e_state.getHash());
  CPPUNIT_ASSERT(record);

  // 玉が移動していると王手にならない
  SimpleState state2=CsaString(
    "P1 *  * -OU *  *  *  *  *  * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE * +KE\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state2(state2);

  Move checkmate_move2 = Move::INVALID();
  OracleProverLight<BLACK> prover;
  ProofOracleAttack<BLACK> oracle(record);
  const bool result = prover.proofWin(e_state2, oracle, checkmate_move2);
  CPPUNIT_ASSERT_EQUAL(false, result);
}



void OracleProverLightTest::testCheck()
{
  HashEffectState e_state
    (CsaString(
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  *  * -KE+FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n").getInitialState());
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(WHITE);
  Move checkmate_move = Move::INVALID();
  const PathEncoding path(e_state.turn());
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<WHITE>
    (500, e_state, e_state.getHash(), path, checkmate_move, oracle_age);
  CPPUNIT_ASSERT(win);
  const CheckHashRecord *record = table.find(e_state.getHash());
  CPPUNIT_ASSERT(record);

  // 詰む局面は詰む
  Move best_move = Move::INVALID();
  OracleProverLight<WHITE> prover;
  ProofOracleAttack<WHITE> oracle(record);
  const bool result = prover.proofWin(e_state, oracle, best_move);
  CPPUNIT_ASSERT_EQUAL(true, result);
  CPPUNIT_ASSERT_EQUAL(record->bestMove->move, best_move);
  // 類似局面も詰む
  best_move = Move::INVALID();
  ApplyMoveOfTurn::doMove(e_state, Move(Square(3,7),PAWN,WHITE));
  ApplyMoveOfTurn::doMove(e_state, Move(Square(8,2),PAWN,BLACK));
  CPPUNIT_ASSERT_EQUAL(true, prover.proofWin(e_state, oracle, best_move));
  CPPUNIT_ASSERT_EQUAL(best_move, record->bestMove->move);

  // 駒が足りないと詰まない
  {
    HashEffectState h2 = e_state;
    ApplyMoveOfTurn::doMove(h2, Move(Square(3,8),GOLD,WHITE));
    ApplyMoveOfTurn::doMove(h2, Move(Square(4,3),PAWN,BLACK));
    CPPUNIT_ASSERT_EQUAL(false, prover.proofWin(h2, oracle, best_move));
  }
  
  // 玉が動いても詰まない
  {
    HashEffectState h2 = e_state;
    ApplyMoveOfTurn::doMove(h2, Move(Square(9,3),PAWN,WHITE));
    ApplyMoveOfTurn::doMove(h2, Move(Square(8,9),Square(7,9),KING,PTYPE_EMPTY,false,BLACK));
    const bool result = prover.proofWin(h2, oracle, best_move);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  // 王手でも詰まない
  {
    HashEffectState h2 = e_state;
    ApplyMoveOfTurn::doMove(h2, Move(Square(9,3),PAWN,WHITE));
    ApplyMoveOfTurn::doMove(h2, Move(Square(4,1),GOLD,BLACK));
    const bool result = prover.proofWin(h2, oracle, best_move);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }
}

void OracleProverLightTest::testPawnCheckmate()
{
  SimpleState state=CsaString(
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * -KE-FU *  *  *  *  *\n"
    "P5 *  *  *  * +FU *  * +FU *\n"
    "P6 * -KE * +FU+GI-FU *  * +FU\n"
    "P7 * +FU-UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU+FU * -GI *  *  * -NG\n"
    "P+00KI00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
    "P-00KY00FU00FU\n"
    "P+00AL\n"
    "-\n").getInitialState();
  HashEffectState e_state((HashEffectState(state)));

  const PathEncoding path(WHITE);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(WHITE);
  Move checkmate_move = Move::INVALID();
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<WHITE>
    (500, e_state, e_state.getHash(), path, checkmate_move, oracle_age);
  CPPUNIT_ASSERT(win);
  const CheckHashRecord *record = table.find(e_state.getHash());

  // 詰む局面は詰む 0088FU, 0098KY
  Move best_move = Move::INVALID();
  OracleProverLight<WHITE> prover;
  ProofOracleAttack<WHITE> oracle(record);
  CPPUNIT_ASSERT_EQUAL(true, prover.proofWin(e_state, oracle, best_move));
  CPPUNIT_ASSERT_EQUAL(best_move, record->bestMove->move);

  // 類似局面も詰む
  best_move = Move::INVALID();
  ApplyMoveOfTurn::doMove(e_state, Move(Square(3,7),PAWN,WHITE));
  ApplyMoveOfTurn::doMove(e_state, Move(Square(4,3),PAWN,BLACK));
  CPPUNIT_ASSERT_EQUAL(true, prover.proofWin(e_state, oracle, best_move));
  CPPUNIT_ASSERT_EQUAL(best_move, record->bestMove->move);

  // 同じ手順では打歩詰になる
  best_move = Move::INVALID();
  ApplyMoveOfTurn::doMove(e_state, Move(Square(7,3),LANCE,WHITE));
  ApplyMoveOfTurn::doMove(e_state, Move(Square(9,9),PAWN,BLACK));
  CPPUNIT_ASSERT_EQUAL(false, prover.proofWin(e_state, oracle, best_move));
}

void OracleProverLightTest::testPawnCheckmateLose()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -KE * \n"
    "P2 *  *  *  *  *  *  * -OU * \n"
    "P3 *  *  *  *  *  *  * +FU-FU\n"
    "P4 *  *  *  *  *  * +RY *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState e_state(state);
  const PathEncoding path(WHITE);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  const Move last_move = Move(Square(2,3), PAWN, BLACK);
  const bool win = checker.isLosingState<WHITE>(500, e_state, e_state.getHash(), path);
  CPPUNIT_ASSERT(win);

  const CheckHashRecord *record = table.find(e_state.getHash());
  OracleProverLight<BLACK> prover;
  ProofOracleDefense<BLACK> oracle(record);
  CPPUNIT_ASSERT_EQUAL(true, prover.proofLose(e_state, oracle, last_move));

  // 今度は逃げられないので打歩詰 (bug 33)
  SimpleState stateNoEscape=CsaString(
    "P1 *  *  *  *  *  *  * -KE-KY\n"
    "P2 *  *  *  *  *  *  * -OU-KE\n"
    "P3 *  *  *  *  *  *  * +FU-FU\n"
    "P4 *  *  *  *  *  * +RY *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState e_stateNoEscape(stateNoEscape);
  CPPUNIT_ASSERT_EQUAL(false, prover.proofLose(e_stateNoEscape, oracle, last_move));
}

void OracleProverLightTest::testCheckAfterAttack()
{
  SimpleState state=CsaString(
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2+FU *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * +FU-FU *  *  *  *  *\n"
    "P5 *  * -KE * +FU *  * +FU *\n"
    "P6-KE *  * +FU+GI-FU *  * +FU\n"
    "P7 *  * -UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9-KI+OU * -GI *  *  *  * -NG\n"
    "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
    "P-00KY00FU00FU\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState e_state((HashEffectState(state)));
  const PathEncoding path(BLACK);

  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(WHITE);
  const bool lose = checker.isLosingState<BLACK>(500, e_state, e_state.getHash(), path);
  CPPUNIT_ASSERT(lose);
  const CheckHashRecord *record = table.find(e_state.getHash());

  // 詰む局面は詰む
  OracleProverLight<WHITE> prover;
  ProofOracleDefense<WHITE> oracle(record);
  CPPUNIT_ASSERT_EQUAL(true, prover.proofLose(e_state, oracle));

  // 類似局面も詰む
  ApplyMoveOfTurn::doMove(e_state, Move(Square(4,3),PAWN,BLACK));
  ApplyMoveOfTurn::doMove(e_state, Move(Square(3,7),PAWN,WHITE));
  CPPUNIT_ASSERT_EQUAL(true, prover.proofLose(e_state, oracle));
  CPPUNIT_ASSERT_EQUAL(true, checker.isLosingState<BLACK>(1, e_state, e_state.getHash(), path));

  // 玉が動くと同じ手順では詰まない
  {
    HashEffectState h2 = e_state;
    ApplyMoveOfTurn::doMove(h2, Move(Square(8,9),Square(7,9),KING,PTYPE_EMPTY,false,BLACK));
    ApplyMoveOfTurn::doMove(h2, Move(Square(9,9),Square(8,9),GOLD,PTYPE_EMPTY,false,WHITE));
    CPPUNIT_ASSERT_EQUAL(false, prover.proofLose(h2, oracle));
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
