/* dualCheckmateSearcher.t.cc
 */
#include "osl/checkmate/dualCheckmateSearcher.h"
#include "osl/checkmate/analyzer/checkTableAnalyzer.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"
#include "osl/apply_move/applyMove.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

class DualCheckmateSearcherTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(DualCheckmateSearcherTest);
  CPPUNIT_TEST(testCheck);
  CPPUNIT_TEST(testDistance);
  CPPUNIT_TEST(testInfty);
  CPPUNIT_TEST(testInfty2);
  CPPUNIT_TEST(testNoCheckmate);
  CPPUNIT_TEST(testPawnCheckmate);
  CPPUNIT_TEST(testPawnCheckmateDefense);
  CPPUNIT_TEST(testCopy);
  CPPUNIT_TEST_SUITE_END();
public:
  void testDistance();
  void testCheck();
  void testInfty();
  void testInfty2();
  void testNoCheckmate();
  void testPawnCheckmate();
  void testPawnCheckmateDefense();
  void testCopy();
};
CPPUNIT_TEST_SUITE_REGISTRATION(DualCheckmateSearcherTest);

using namespace osl;
using namespace osl::checkmate;
extern bool isShortTest;
const size_t limit = 400000;

void DualCheckmateSearcherTest::testNoCheckmate()
{
  {
    HashEffectState state(CsaString(
			   "P1 *  *  *  * -OU *  *  *  * \n"
			   "P2 *  *  *  *  * +GI *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6-HI-HI-KA-KA *  *  *  * +KY\n"
			   "P7-FU-GI * +GI *  *  *  * +KY\n"
			   "P8 * -FU *  *  *  *  *  * +KY\n"
			   "P9+OU * +FU+FU+FU+FU+FU * +KY\n"
			   "P+00KI00KI00GI00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "P-00KI00KI00KE00KE00KE00KE\n"
			   "-\n").getInitialState());
    // 98金の一手詰ではあるけど
    DualCheckmateSearcher<> checker(limit);
    Move check_move = Move::INVALID();
    const PathEncoding path(WHITE, 3);
    AttackOracleAges oracle_age;
    const bool win = checker.isWinningState<WHITE>
      (10, state, state.getHash(), path, check_move, oracle_age);
    CPPUNIT_ASSERT_EQUAL(false, win);
  }
}

void DualCheckmateSearcherTest::testDistance()
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
  HashEffectState hState(state);
  DualCheckmateSearcher<> checker(limit);
  if (! isShortTest)
    checker.setVerbose();
  Move check_move = Move::INVALID();
  const int distance = 3;
  const PathEncoding path(BLACK, distance);
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<BLACK>
    (10, hState, hState.getHash(), path, check_move, oracle_age);
  CPPUNIT_ASSERT_EQUAL(true, win);
  CPPUNIT_ASSERT_EQUAL(Move(Square(2,2),GOLD,BLACK), check_move);

  const CheckHashTable& table = checker.getTable(BLACK);
  const CheckHashRecord *record = table.find(hState.getHash());
  CPPUNIT_ASSERT_EQUAL(distance, static_cast<int>(record->distance));
}

void DualCheckmateSearcherTest::testPawnCheckmate()
{
  // 逃げられないので打歩詰 (bug 33)
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
  HashEffectState hStateNoEscape(stateNoEscape);
  const Move last_move = Move(Square(2,3), PAWN, BLACK);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  const PathEncoding path(WHITE);
  CPPUNIT_ASSERT_EQUAL(false, 
		       checker.isLosingState<WHITE>(500, hStateNoEscape, hStateNoEscape.getHash(),
						    path, last_move));
}

void DualCheckmateSearcherTest::testPawnCheckmateDefense()
{
  HashEffectState state(CsaString(
    "P1+KA *  * -KI *  *  *  *  * \n"
    "P2 *  *  * -KI *  *  *  *  * \n"
    "P3 * -FU+KY-OU-UM+RY-FU-FU * \n"
    "P4-FU *  * -FU-FU *  *  *  * \n"
    "P5 * +FU-FU *  * -FU * +FU * \n"
    "P6+FU+OU+FU+FU+FU *  *  *  * \n"
    "P7 *  *  *  * +GI+FU+KE *  * \n"
    "P8 *  * +GI+KI *  *  *  *  * \n"
    "P9+KY *  * +KI *  *  *  *  * \n"
    "P+00GI00GI00KY00FU\n"
    "P-00HI00KE00KE00KE00KY00FU00FU\n"
    "-\n").getInitialState());
  DualCheckmateSearcher<> checker(limit);
  const PathEncoding path(WHITE);
  Move check_move;
  checker.isWinningState(500, state, state.getHash(),
			 path, check_move);
// 動かすだけ 途中で攻め方を打ち歩詰めにする王手回避が出現
}

/**
 */
void DualCheckmateSearcherTest::testCheck()
{
  SimpleState state=CsaString(
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * +FU-FU *  *  *  *  *\n"
    "P5 *  * -KE * +FU *  * +FU *\n"
    "P6-KE *  * +FU+GI-FU *  * +FU\n"
    "P7 *  * -UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU * -GI *  *  *  * -NG\n"
    "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
    "P-00KI00KY00FU00FU\n"
    "P-00AL\n"
    "+\n").getInitialState();

  ApplyMoveOfTurn::doMove(state, Move(Square(9,2),PAWN,BLACK));
  HashEffectState hState(state);

  size_t rootTreeSize = 0;
  AttackOracleAges oracle_age;
  {
    DualCheckmateSearcher<> checker(limit);
    checker.setVerbose(!isShortTest);
    Move checkmate_move = Move::INVALID();
    const PathEncoding path(WHITE);
    // 3 手詰なのですぐに詰めてほしいところ
    const bool win = checker.isWinningState<WHITE>
      (70, hState, hState.getHash(), path, checkmate_move, oracle_age);
    CPPUNIT_ASSERT(win);
    CPPUNIT_ASSERT(! checkmate_move.isPass());
    const CheckHashTable& table = checker.getTable(WHITE);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    const HashKey& key = hState.getHash();
    const CheckHashRecord *record = table.find(key);
    rootTreeSize = analyzer.treeSize(record);
    CPPUNIT_ASSERT_EQUAL(table.size(), rootTreeSize);
    CPPUNIT_ASSERT(analyzer.proofTreeSize(record,key,path,true)
		   <= rootTreeSize);
    if (! isShortTest) {
      analyzer.showProofTree(record,key,path,true,std::cerr);
      std::cerr << analyzer.proofTreeSize(record,key,path,true)
		<< " < " << rootTreeSize << "\n";
    }
  }
  
  {
    // 一手進めばすぐ詰む
    ApplyMoveOfTurn::doMove(hState, Move(Square(8,8),GOLD,WHITE));
    DualCheckmateSearcher<> checker(limit);
    checker.setVerbose(!isShortTest);
    const PathEncoding path(BLACK);
    const bool lose = checker.isLosingState<BLACK>(10, hState, hState.getHash(), path);
    CPPUNIT_ASSERT(lose);
    const CheckHashTable& table = checker.getTable(WHITE);
    CheckTableAnalyzer analyzer(table.getTwinTable());
    const HashKey& key = hState.getHash();
    const CheckHashRecord *record = table.find(key);
    CPPUNIT_ASSERT(analyzer.treeSize(record) < rootTreeSize);
    // analyzer.showTree(record, std::cerr, 10);
    if (! isShortTest) 
      analyzer.showProofTree(record,key,path,false,std::cerr);
    CPPUNIT_ASSERT(analyzer.proofTreeSize(record,key,path,false)
		   <= analyzer.treeSize(record));
  }
}

void DualCheckmateSearcherTest::testInfty()
{
  SimpleState state=CsaString(
"P1-KY * -KI-KE *  *  *  * +UM\n"
"P2 *  *  *  * -OU * -GI *  * \n"
"P3-FU * -KE-FU-FU-FU * -KA-FU\n"
"P4 * -HI-FU *  *  *  *  *  * \n"
"P5 *  * +KI *  * +FU *  *  * \n"
"P6 *  *  * +FU+KI *  *  *  * \n"
"P7+FU * +OU * +FU * +KI+GI+FU\n"
"P8-TO *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  * +KY\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P+00KE\n"
"P+00KE\n"
"P-00GI\n"
"P+00GI\n"
"P+00KY\n"
"P+00KY\n"
"P+00HI\n"
"-\n").getInitialState();
  HashEffectState hState(state);
  const PathEncoding path(WHITE);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  Move checkmate_move = Move::INVALID();
  AttackOracleAges oracle_age;
  const bool win = checker.isWinningState<WHITE>
    (400, hState, hState.getHash(), path, checkmate_move, oracle_age);
  CPPUNIT_ASSERT(win);
}

void DualCheckmateSearcherTest::testInfty2()
{
  SimpleState state=CsaString(
"P1-KY-KE *  *  * -OU * -KE+KA\n"
"P2 *  * +FU * -KI *  *  *  * \n"
"P3-FU *  * -FU-GI-KI * -FU-FU\n"
"P4 *  *  *  * -FU *  *  *  * \n"
"P5 * +GI * +FU * -FU *  *  * \n"
"P6 *  *  *  * +FU *  *  *  * \n"
"P7+FU+FU * +KI * +FU * +FU+FU\n"
"P8 * +OU-NY *  *  *  *  * -RY\n"
"P9+KY+KE * +KI *  * +FU *  * \n"
"P+00FU\n"
"P+00FU\n"
"P-00FU\n"
"P-00KE\n"
"P+00GI\n"
"P+00GI\n"
"P-00KY\n"
"P+00KA\n"
"P-00HI\n"
"+\n").getInitialState();
  
  HashEffectState hState((NumEffectState(state)));
  const PathEncoding path(BLACK);
  DualCheckmateSearcher<> checker(limit);
  checker.setVerbose(!isShortTest);
  const bool lose = checker.isLosingState<BLACK>(100, hState, hState.getHash(), path);
  CPPUNIT_ASSERT(! lose);
}

void DualCheckmateSearcherTest::testCopy()
{
  // should have different table on copy
  DualCheckmateSearcher<> searcher(limit);
  DualCheckmateSearcher<> searcher2(searcher);
  CPPUNIT_ASSERT(&searcher.getTable(BLACK) != &searcher2.getTable(BLACK));
#if 0
  // currently not supported
  DualCheckmateSearcher<> searcher3(limit);
  searcher3 = searcher;
#endif
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
