/* oracleDisproverTest.cc
 */
#include "osl/checkmate/oracleDisprover.h"
#include "osl/checkmate/dualCheckmateSearcher.h"
#include "osl/checkmate/analyzer/checkTableAnalyzer.h"
#include "osl/checkmate/checkmateRecorder.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <fstream>

class OracleDisproverTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(OracleDisproverTest);
  CPPUNIT_TEST(testNoCheckLong);
  CPPUNIT_TEST(testAdjustAttack);
  CPPUNIT_TEST(testPawnCheckmate);
  CPPUNIT_TEST(testPawnCheckmateLose);
  CPPUNIT_TEST(testDistance);
  CPPUNIT_TEST(testNoCheck);
  CPPUNIT_TEST(testCheck);
  CPPUNIT_TEST(testAdjustMove);
  CPPUNIT_TEST_SUITE_END();
public:
  void testDistance();
  void testAdjustMove();
  void testAdjustAttack();
  void testNoCheck();
  void testNoCheckLong();
  void testCheck();
  void testPawnCheckmate();
  void testPawnCheckmateLose();
};
CPPUNIT_TEST_SUITE_REGISTRATION(OracleDisproverTest);

using namespace osl;
using namespace osl::checkmate;
extern bool isShortTest;
const size_t limit = 400000;

template <class Table, class State>
static void logNoCheckmate(Table& table, State& state2, 
			   const PathEncoding& path, 
			   const CheckHashRecord *record, 
			   const PathEncoding& path2)
{
  CheckmateRecorder::DepthTracer::maxVerboseLogDepth = 20;
  OracleDisprover<Table> disprover(table);
  disprover.proofNoCheckmate(state2, path, record, path2);
  CheckmateRecorder::DepthTracer::maxVerboseLogDepth = 1;
}

void OracleDisproverTest::testDistance()
{
  // bug 35
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
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state(sstate);
  const int distance = 3;
  const PathEncoding path(BLACK, distance);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  Move check_move = Move::INVALID();
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<BLACK>(100, state, state.getHash(),
						 path, check_move, oracle_age);
  CPPUNIT_ASSERT_EQUAL(false, win);

  SimpleState sstate_similar=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  * -FU+FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state_similar(sstate_similar);

  CheckHashTable& table = checker.getTable(BLACK);
  OracleDisprover<CheckHashTable> disprover(table);
  const CheckHashRecord *record = table.find(state.getHash());
  DisproofOracleAttack<BLACK> oracle(record);

  const int distance_similar = 30;
  const PathEncoding path_similar(BLACK, distance_similar);
  const bool result = 
    disprover.proofNoCheckmate(state_similar, state_similar.getHash(),
			       path_similar, oracle);
  CPPUNIT_ASSERT_EQUAL(true, result);

  const CheckHashRecord *record_similar = table.find(state_similar.getHash());
  CPPUNIT_ASSERT_EQUAL(distance, static_cast<int>(record->distance));
  CPPUNIT_ASSERT_EQUAL(distance_similar, static_cast<int>(record_similar->distance));
}

void OracleDisproverTest::testAdjustMove()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * +NK * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state(sstate);
  const PathEncoding path(WHITE);
  const Move last_move = Move(Square(3,4), Square(2,2),
				PKNIGHT, PTYPE_EMPTY, true, BLACK);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  AttackOracleAges oracle_age;
  const bool lose = checker.isLosingState<WHITE>
    (500, state, state.getHash(), path, last_move);
  CPPUNIT_ASSERT(! lose);
  CheckHashRecord *record = table.find(state.getHash());
  CPPUNIT_ASSERT(record);
  DisproofOracleDefense<BLACK> oracle(record);

  // 取る駒が違っても詰まない
  SimpleState sstate2=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * +KI * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state2(sstate2);

  OracleDisprover<CheckHashTable> disprover(table);
  Move escape_move;
  const bool result = 
    disprover.proofEscape(state2, state2.getHash(),
			  path, oracle, escape_move, last_move);
  CPPUNIT_ASSERT_EQUAL(true, result);
  CPPUNIT_ASSERT_EQUAL(Square(2,1), escape_move.from());
  CPPUNIT_ASSERT_EQUAL(Square(2,2), escape_move.to());
  CPPUNIT_ASSERT_EQUAL(GOLD, escape_move.capturePtype());
}

void OracleDisproverTest::testAdjustAttack()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  * +HI-KI-OU\n"
    "P2 *  *  *  *  *  *  * -KE-KI\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state(sstate);
  const PathEncoding path(BLACK);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  Move check_move;
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<BLACK>(500, state, state.getHash(), path, check_move,
						 oracle_age);
  CPPUNIT_ASSERT(! win);
  CheckHashRecord *record = table.find(state.getHash());
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT(record->proofDisproof().isCheckmateFail());
  DisproofOracleAttack<BLACK> oracle(record);
  OracleDisprover<CheckHashTable> disprover(table);

  // 遠くから王手 + 余計な駒
  SimpleState sstate2=CsaString(
    "P1+HI *  *  *  *  *  * -KI-OU\n"
    "P2 *  *  *  *  *  *  * -KE-KI\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  * -FU *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state2(sstate2);

  const bool result = 
    disprover.proofNoCheckmate<BLACK>(state2, state2.getHash(), path, oracle);
  CPPUNIT_ASSERT_EQUAL(true, result);

  // 龍でもよい，銀を取った後，金の変わりに32に打っても大丈夫
  SimpleState sstate3=CsaString(
    "P1 * +RY *  *  *  *  * -GI-OU\n"
    "P2 *  *  *  *  *  *  * -KE-KI\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  * -FU *  *  * \n"
    "P5 *  *  *  *  * -KY *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state3(sstate3);

  const bool result3 = 
    disprover.proofNoCheckmate<BLACK>(state3, state3.getHash(),
				      path, oracle);
  CPPUNIT_ASSERT_EQUAL(true, result3);

  // 角だと?
  SimpleState sstate4=CsaString(
    "P1 *  *  *  *  *  *  * -GI-OU\n"
    "P2 *  *  *  *  *  *  * -KE-KI\n"
    "P3 *  *  *  *  * +KA *  *  * \n"
    "P4 *  *  *  *  * -FU *  *  * \n"
    "P5 *  *  *  *  * -KY *  *  * \n"
    "P6 *  *  *  *  * -KE *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state4(sstate4);

  const bool result4 = 
    disprover.proofNoCheckmate<BLACK>(state4, state4.getHash(),
				      path, oracle);
  CPPUNIT_ASSERT_EQUAL(true, result4);
}

void OracleDisproverTest::testCheck()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * +NK * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state(sstate);
  const PathEncoding path(WHITE);
  const Move last_move = Move(Square(3,4), Square(2,2),
				PKNIGHT, PTYPE_EMPTY, true, BLACK);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  const bool lose = checker.isLosingState<WHITE>
    (500, state, state.getHash(), path, last_move);
  CPPUNIT_ASSERT(! lose);
  CheckHashRecord *record = table.find(state.getHash());
  CPPUNIT_ASSERT(record);
  DisproofOracleDefense<BLACK> oracle(record);

  // 取る駒が違っても詰まない
  SimpleState sstate2=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * +NK * \n"
    "P3 *  *  *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state2(sstate2);

  OracleDisprover<CheckHashTable> disprover(table);
  Move escape_move;
  const bool result = 
    disprover.proofEscape(state2, state2.getHash(),
			  path, oracle, escape_move, last_move);
  CPPUNIT_ASSERT_EQUAL(false, result);
}

void OracleDisproverTest::testNoCheck()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * +NK * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state(sstate);
  const PathEncoding path(WHITE);
  const Move last_move = Move(Square(3,4), Square(2,2),
				PKNIGHT, PTYPE_EMPTY, true, BLACK);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  const bool lose = checker.isLosingState<WHITE>
    (500, state, state.getHash(), path, last_move);
  CPPUNIT_ASSERT(! lose);
  CheckHashRecord *record = table.find(state.getHash());
  CPPUNIT_ASSERT(record);
  DisproofOracleDefense<BLACK> oracle(record);

  SimpleState sstate2=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * +KI * \n"
    "P3 *  *  *  *  * +FU *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state2(sstate2);

  OracleDisprover<CheckHashTable> disprover(table);
  Move escape_move;
  const bool result = 
    disprover.proofEscape(state2, state2.getHash(),
			  path, oracle, escape_move, last_move);
  CPPUNIT_ASSERT(result);
  CPPUNIT_ASSERT_EQUAL(escape_move.from(), Square(2,1));
  CPPUNIT_ASSERT_EQUAL(escape_move.to(), Square(2,2));
}

static bool isNoCheckmate(const char *org, const char *similar)
{
  HashEffectState state(CsaString(org).getInitialState());

  const size_t limit = 100000;
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(state.turn());
  Move check_move;
  const PathEncoding path(state.turn());
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState(limit, state, state.getHash(), path, check_move,
					  oracle_age);
  CPPUNIT_ASSERT(! win);
  const CheckHashRecord *record = table.find(state.getHash());
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT(record->proofDisproof().isCheckmateFail()
		 || record->findLoop(path, table.getTwinTable()));
  CheckTableAnalyzer analyzer(table.getTwinTable());
  CPPUNIT_ASSERT(analyzer.disproofTreeSize(record, state.getHash(), path, false));

  HashEffectState state2(CsaString(similar).getInitialState());
  OracleDisprover<CheckHashTable> disprover(table);

  PathEncoding path2(alt(state2.turn()));
  path2.pushMove(Move(Square(5,5),PAWN,alt(state2.turn())));
  CPPUNIT_ASSERT(path != path2);
  
  const bool result = disprover.proofNoCheckmate(state2, path2, record, path);
  if (result)
  {
    const HashKey& key2 = state2.getHash();
    const CheckHashRecord *record2 = table.find(key2);
    const size_t tree_size = analyzer.disproofTreeSize(record2, key2, path2, false);
    CPPUNIT_ASSERT(tree_size);
  }
  if ((! result) && (! isShortTest))
  {
    CheckTableAnalyzer analyzer(table.getTwinTable());
    std::ofstream os("oracleDisproverTest.log");
    analyzer.showProofTree(record,state2.getHash(),path,false,os);
    logNoCheckmate(table, state2, path, record, path);
  }
  return result;
}

void OracleDisproverTest::testNoCheckLong()
{
  // この辺のテストがfail したら。
  // - -DCHECKMATE_DEBUG つきでコンパイル
  // - CheckmateRecorder::maxVerboseLogDepth あたりを増やす
  // - 該当テスト以外を #if 0 で囲み、oracleDiprover.t を実行
  // - simplationのログが test/checkmate.log に、
  //   simulationのお手本が、同/oracleDisproverTest.log に出来ているので比較
  // - emacs で開くと、outline-mode になっている/自分でする必要がある。
  //   ファイルが大きい場合は、font-lock-mode をoffに。
  // - C-c d 閉じる C-c i 1レベル開く
  // - 局面が違うせいで思わぬ王手が現れていたら問題が悪い:(
  CPPUNIT_ASSERT(! isNoCheckmate(
		   "P1-OU-KE *  *  *  *  * +UM * \n"
		   "P2 * -KI+NK *  *  *  *  *  * \n"
		   "P3 *  *  * +TO *  *  * -FU-FU\n"
		   "P4-KY * -FU * -FU * -HI *  * \n"
		   "P5-FU-FU *  *  *  * -GI+FU * \n"
		   "P6 *  * +KI * -NK-FU-FU * +FU\n"
		   "P7 * +FU *  *  *  *  *  *  * \n"
		   "P8 * +GI * -FU *  *  *  *  * \n"
		   "P9+KY+OU+GI *  *  * -UM * +KY\n"
		   "P+00FU00GI\n"
		   "P-00FU00FU00FU00FU00KE00KI00KI00KY00HI\n"
		   "+\n",
		   // これは一瞬で詰むダミー
		   "P1-OU-KE *  *  *  *  * +UM * \n"
		   "P2 *  * +NK *  *  *  *  *  * \n"
		   "P3+FU *  * +TO *  *  * -FU-FU\n"
		   "P4-KY * -FU * -FU * -HI *  * \n"
		   "P5-FU-FU *  *  *  * -GI+FU * \n"
		   "P6 *  * +KI * -NK-FU-FU * +FU\n"
		   "P7 * +FU *  *  *  *  *  *  * \n"
		   "P8 * +GI+GI-FU *  *  *  *  * \n"
		   "P9+KY+OU *  *  *  * -UM * +KY\n"
		   "P+00FU00GI\n"
		   "P-00FU00FU00FU00KE00KI00KI00KY00HI00KI\n"
		   "+\n"));
  CPPUNIT_ASSERT(isNoCheckmate(
		   "P1-OU-KE *  *  *  *  * +UM * \n"
		   "P2 * -KI+NK *  *  *  *  *  * \n"
		   "P3 *  *  * +TO *  *  * -FU-FU\n"
		   "P4-KY * -FU * -FU * -HI *  * \n"
		   "P5-FU-FU *  *  *  * -GI+FU * \n"
		   "P6 *  * +KI * -NK-FU-FU * +FU\n"
		   "P7 * +FU *  *  *  *  *  *  * \n"
		   "P8 * +GI * -FU *  *  *  *  * \n"
		   "P9+KY+OU+GI *  *  * -UM * +KY\n"
		   "P+00FU00GI\n"
		   "P-00FU00FU00FU00FU00KE00KI00KI00KY00HI\n"
		   "+\n",
		   "P1-OU-KE *  *  *  *  * +UM * \n"
		   "P2 * -KI+NK *  *  *  *  *  * \n"
		   "P3 *  *  * +TO *  *  * -FU-FU\n"
		   "P4-KY * -FU * -FU * -HI *  * \n"
		   "P5-FU-FU *  *  *  * -GI+FU * \n"
		   "P6 *  * +KI * -NK-FU-FU * +FU\n"
		   "P7 * +FU *  *  *  *  *  *  * \n"
		   "P8 * +GI * -FU *  *  *  *  * \n"
		   "P9+KY+OU+GI * -TO * -UM * +KY\n"
		   "P+00FU00GI\n"
		   "P-00FU00FU00FU00KE00KI00KI00KY00HI\n"
		   "+\n"));
  CPPUNIT_ASSERT(isNoCheckmate(
		   "P1-KY-KE * +KA * -KI-GI-KE-KY\n"
		   "P2 * +HI * -FU *  * -OU *  * \n"
		   "P3-FU *  *  * +TO-FU * -FU-FU\n"
		   "P4 *  *  *  *  *  * -FU *  * \n"
		   "P5 * -UM * -RY-FU *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7+FU * -TO *  * +FU+FU+FU+FU\n"
		   "P8 *  *  *  * +KI *  *  * +KY\n"
		   "P9+KY *  *  *  * +KI+GI+KE+OU\n"
		   "P-00FU00FU00FU00KE00GI\n"
		   "P+00FU00GI00KI\n"
		   "+\n",
		   "P1-KY-KE * +KA * -KI-GI-KE-KY\n"
		   "P2 * +HI * -FU *  * -OU *  * \n"
		   "P3-FU *  *  * +TO-FU * -FU-FU\n"
		   "P4 *  *  *  *  *  * -FU *  * \n"
		   "P5 * -UM * -RY-FU *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7+FU * -TO *  * +FU+FU+FU+FU\n"
		   "P8 *  *  *  * +KI *  * +OU+KY\n"
		   "P9+KY *  *  *  * +KI+GI+KE * \n"
		   "P-00FU00FU00FU00KE00GI\n"
		   "P+00FU00GI00KI\n"
		   "+\n"));
  CPPUNIT_ASSERT(isNoCheckmate(
		   "P1-KY-KE *  *  * +KA * -KE-KY\n"
		   "P2 * -HI *  *  *  * -KI-OU * \n"
		   "P3 *  *  * -FU * -KI-GI-FU-FU\n"
		   "P4-FU *  *  * -FU-FU-FU *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6+FU * +FU+FU+FU+GI+FU+FU+FU\n"
		   "P7 * +FU *  *  * +FU+KE *  * \n"
		   "P8 * +KI * +KI-UM *  *  * +KY\n"
		   "P9+KY+KE+OU+HI-GI *  *  *  * \n"
		   "P+00FU00GI\n"
		   "P-00FU\n"
		   "+\n",
		   "P1-KY-KE *  *  * +KA * -KE-KY\n"
		   "P2 * -HI *  *  *  * -KI-OU * \n"
		   "P3 *  *  * -FU * -KI-GI-FU-FU\n"
		   "P4-FU *  *  * -FU-FU-FU *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6+FU * +FU+FU+FU+GI+FU+FU+FU\n"
		   "P7 * +FU *  *  * +FU+KE *  * \n"
		   "P8 * +KI+OU+KI-UM *  *  * +KY\n"
		   "P9+KY+KE * +HI-GI *  *  *  * \n"
		   "P+00FU00GI\n"
		   "P-00FU\n"
		   "+\n"));
  CPPUNIT_ASSERT(isNoCheckmate(
		   "P1+TO *  *  *  * -OU * -KE-KY\n"
		   "P2 * -FU-KE *  *  * -KI-GI * \n"
		   "P3 *  * -GI-FU-FU-FU * -FU-FU\n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  * +FU+FU\n"
		   "P6+KY+OU * +FU+KY+KE-HI *  * \n"
		   "P7 * +FU *  * +FU+FU+KE *  * \n"
		   "P8+KI *  *  *  *  * +GI *  * \n"
		   "P9 *  *  *  *  * +KI * +HI+KY\n"
		   "P-00FU00FU00KA\n"
		   "P+00FU00FU00FU00GI00KI00KA\n"
		   "+\n",
		   "P1+TO *  *  *  * -OU * -KE-KY\n"
		   "P2 * -FU-KE *  *  * -KI-GI * \n"
		   "P3 *  * -GI-FU-FU-FU * -FU-FU\n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  * +FU+FU\n"
		   "P6+KY+OU * +FU+KY+KE-HI *  * \n"
		   "P7 * +FU *  * +FU+FU+KE *  * \n"
		   "P8+KI *  *  *  *  * +GI *  * \n"
		   "P9-TO *  *  *  * +KI * +HI+KY\n"
		   "P-00FU00FU00KA\n"
		   "P+00FU00FU00GI00KI00KA\n"
		   "+\n"));
  CPPUNIT_ASSERT(isNoCheckmate(
		   "P1+NY+RY *  *  *  * -KI-KE-OU\n"
		   "P2+RY-FU *  * +NG * -KI-GI-KY\n"
		   "P3 * -KE *  * -FU *  * -FU-FU\n"
		   "P4-FU-KA * +FU *  * -FU *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6+FU+FU *  *  * -FU *  *  * \n"
		   "P7 *  *  *  *  *  * +FU+FU+FU\n"
		   "P8 *  *  *  *  *  * +GI+OU * \n"
		   "P9 *  *  * -NK *  * +KI+KE+KY\n"
		   "P+00FU00FU00FU00KA\n"
		   "P-00FU00FU00GI00KI00KY\n"
		   "-\n",
		   "P1 * +RY *  *  *  * -KI-KE-OU\n"
		   "P2+RY-FU *  * +NG * -KI-GI-KY\n"
		   "P3 * -KE *  * -FU *  * -FU-FU\n"
		   "P4-FU-KA * +FU *  * -FU *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6+FU+FU *  *  * -FU *  *  * \n"
		   "P7 *  *  *  *  *  * +FU+FU+FU\n"
		   "P8 *  *  *  *  *  * +GI+OU * \n"
		   "P9 *  *  * -NK *  * +KI+KE+KY\n"
		   "P+00FU00FU00FU00KY00KA\n"
		   "P-00FU00FU00GI00KI00KY\n"
		   "-\n"));
  CPPUNIT_ASSERT(isNoCheckmate(
		   "P1-KY-KE *  *  * -OU *  * -KY\n"
		   "P2 *  *  *  * -KI * -KI-GI * \n"
		   "P3 *  *  * -FU-GI-FU-KE *  * \n"
		   "P4-FU *  *  *  *  *  *  * -FU\n"
		   "P5 * +FU-HI *  *  *  *  *  * \n"
		   "P6+FU+KA *  * +HI *  *  * +FU\n"
		   "P7 *  *  * +FU+FU+FU+FU *  * \n"
		   "P8 *  * +KI * +OU+GI+KI *  * \n"
		   "P9+KY * +GI *  *  *  * +KE+KY\n"
		   "P-00FU00FU00FU00FU00KE00KA\n"
		   "P+00FU00FU00FU\n"
		   "-\n",
		   "P1-KY-KE *  *  *  * -OU * -KY\n"
		   "P2 *  *  *  * -KI * -KI-GI * \n"
		   "P3 *  *  * -FU-GI-FU-KE *  * \n"
		   "P4-FU *  *  *  *  *  *  * -FU\n"
		   "P5 * +FU-HI *  *  *  *  *  * \n"
		   "P6+FU+KA *  * +HI *  *  * +FU\n"
		   "P7 *  *  * +FU+FU+FU+FU *  * \n"
		   "P8 *  * +KI * +OU+GI+KI *  * \n"
		   "P9+KY * +GI *  *  *  * +KE+KY\n"
		   "P-00FU00FU00FU00FU00KE00KA\n"
		   "P+00FU00FU00FU\n"
		   "-\n"));
}

void OracleDisproverTest::testPawnCheckmate()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  * -KY-OU-KY\n"
    "P2 *  *  *  *  *  * -KE * -KE \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state(sstate);
  const PathEncoding path(BLACK);

  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  Move check_move = Move::INVALID();
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<BLACK>(500, state, state.getHash(), path, check_move,
						 oracle_age);
  CPPUNIT_ASSERT(! win);
  const CheckHashRecord *record = table.find(state.getHash());
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT(record->proofDisproof().isCheckmateFail()
		 || record->findLoop(path, table.getTwinTable()));
  CPPUNIT_ASSERT((ProofDisproof::PawnCheckmate() == record->proofDisproof())
		 || record->filter.isTarget(MoveFlags::NoPromote));
  DisproofOracleAttack<BLACK> oracle(record,path);

  SimpleState sstate2=CsaString(
    "P1 *  *  *  *  *  * -KY-OU-KY\n"
    "P2 *  *  *  *  *  * -KE * -KE \n"
    "P3 *  *  *  *  * +FU *  *  * \n"
    "P4 *  *  *  *  *  * +KE *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state2(sstate2);

  OracleDisprover<CheckHashTable> disprover(table);
  const bool result = 
    disprover.proofNoCheckmate(state2, state2.getHash(),
			       path, oracle);
  if (! result)
  {
    logNoCheckmate(table, state2, path, record, path);
    record->dump();
  }
  CPPUNIT_ASSERT(result);

  // 同じ手順では打歩詰にならない
  SimpleState sstate3=CsaString(
    "P1 *  *  *  *  *  *  * -OU-KY\n"
    "P2 *  *  *  *  *  * -KE * -KE \n"
    "P3 *  *  *  *  * +FU *  *  * \n"
    "P4 *  *  *  *  *  * +KE *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState state3(sstate3);

  const bool result3 = 
    disprover.proofNoCheckmate(state3, state3.getHash(),
			       path, oracle);
  CPPUNIT_ASSERT(! result3);
}

void OracleDisproverTest::testPawnCheckmateLose()
{
  SimpleState sstate=CsaString(
    "P1 *  *  *  *  *  * -KY * -KY\n"
    "P2 *  *  *  *  *  * -KE-OU-KE \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE+KI * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state(sstate);
  const PathEncoding path(WHITE);
  const Move last_move = Move(Square(3,4), PKNIGHT, BLACK);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  CheckHashTable& table = checker.getTable(BLACK);
  const bool lose = checker.isLosingState<WHITE>
    (500, state, state.getHash(), path, last_move);
  CPPUNIT_ASSERT(! lose);
  CheckHashRecord *record = table.find(state.getHash());
  CPPUNIT_ASSERT(record);
  DisproofOracleDefense<BLACK> oracle(record);

  SimpleState sstate2=CsaString(
    "P1 *  *  *  *  *  * -KY * -KY\n"
    "P2 *  *  *  *  *  * -KE-OU-KE \n"
    "P3 *  *  *  *  * +FU *  *  * \n"
    "P4 *  *  *  *  *  * +KE+KI * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state2(sstate2);

  OracleDisprover<CheckHashTable> disprover(table);
  Move escape_move;
  const bool result2 = 
    disprover.proofEscape(state2, state2.getHash(),
			  path, oracle, escape_move, last_move);
  CPPUNIT_ASSERT(result2);

  // 同じ手順では打歩詰にならない
  SimpleState sstate3=CsaString(
    "P1 *  *  *  *  *  *  *  * -KY\n"
    "P2 *  *  *  *  *  * -KE-OU-KE \n"
    "P3 *  *  *  *  * +FU *  *  * \n"
    "P4 *  *  *  *  *  * +KE+KI * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState state3(sstate3);

  const bool result3 = 
    disprover.proofEscape(state3, state3.getHash(),
			  path, oracle, escape_move, last_move);
  CPPUNIT_ASSERT(! result3);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
