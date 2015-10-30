/* alphaBeta2.t.cc
 */
#include "problems.h"
#include "osl/search/alphaBeta2.h"
#include "osl/search/simpleHashTable.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>
#include <string>

using namespace osl;
using namespace osl::search;

typedef SearchState2::checkmate_t checkmate_t;

static void
testProblems(const Problem *problems, int numProblems, int depth)
{
  static bool loaded = eval::ProgressEval::setUp()
    && osl::eval::ml::OpenMidEndingEval::setUp()
    && osl::progress::ml::NewProgress::setUp();
  BOOST_CHECK(loaded);
 
  CountRecorder recorder;

  for (int i=0; i<numProblems; ++i)
  {
    NumEffectState state((CsaString(problems[i].state).initialState()));
    checkmate_t checkmate;
    SimpleHashTable table(80000, -1);
    AlphaBeta2OpenMidEndingEval searcher(state, checkmate, &table, recorder);

    const Move best_move = searcher.computeBestMoveIteratively(depth, 200);
    BOOST_CHECK(problems[i].acceptable(best_move));
  }
}

BOOST_AUTO_TEST_CASE(AlphaBeta2TestProblems1)
{
  testProblems(problems1, numProblems1, 600);
}
BOOST_AUTO_TEST_CASE(AlphaBeta2TestProblems2)
{
  testProblems(problems1, numProblems1, 800);
}
BOOST_AUTO_TEST_CASE(AlphaBeta2TestProblems3)
{
  testProblems(problems3, numProblems3, 1000);
}

BOOST_AUTO_TEST_CASE(AlphaBeta2TestRootIgnore)
{
  NumEffectState state(CsaString(
			 "P1-GI-KE-KI-GI-KE-TO-TO-TO-OU\n"
			 "P2 *  *  *  *  *  *  * -TO-KI\n"
			 "P3-FU * -FU-FU *  *  *  * -KY\n"
			 "P4 * -KY *  *  *  *  * -UM-FU\n"
			 "P5 *  *  *  *  * -HI-FU *  * \n"
			 "P6 *  * +KE *  * +KY *  * +KE\n"
			 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA+KI * +OU *  * +HI * \n"
			 "P9+KY * +GI *  * +KI+GI *  * \n"
			 "+\n").initialState());
  
  checkmate_t checkmate;
  SimpleHashTable table(1000000, -1);
  CountRecorder recorder;
  AlphaBeta2ProgressEval searcher(state, checkmate, &table, recorder);

  const Move best_move(Square(1,6),Square(2,4),KNIGHT,PBISHOP,false,BLACK);
  {
    const Move move = searcher.computeBestMoveIteratively(1000, 200);
    table.clear();
    BOOST_CHECK_EQUAL(best_move, move);
  }
  
  MoveVector ignores;
  searcher.setRootIgnoreMoves(&ignores, true);

  {
    const Move move = searcher.computeBestMoveIteratively(1000, 200);
    table.clear();
    BOOST_CHECK_EQUAL(best_move, move);
  }

  ignores.push_back(best_move);

  const Move best_move2(Square(4,6),Square(4,5),LANCE,ROOK,false,BLACK);
  {
    const Move move = searcher.computeBestMoveIteratively(1000, 200);
    table.clear();
    BOOST_CHECK_EQUAL(best_move2, move);
  }

  ignores.push_back(best_move2);

  const Move best_move3(Square(7,6),Square(8,4),KNIGHT,LANCE,false,BLACK);
  {
    const Move move = searcher.computeBestMoveIteratively(1000, 200);
    table.clear();
    BOOST_CHECK_EQUAL(best_move3, move);
  }
}

BOOST_AUTO_TEST_CASE(AlphaBeta2TestRootAllIgnore)
{
  NumEffectState state(CsaString(
			 "P1-GI-KE-KI-GI-KE-TO-TO-TO-OU\n"
			 "P2 *  *  *  *  *  *  * -TO-HI\n"
			 "P3-FU * -FU-FU *  *  *  * -KY\n"
			 "P4 * -KY *  *  *  *  * -KA-FU\n"
			 "P5 *  *  *  *  * -KI-FU *  * \n"
			 "P6 *  * +KE *  * +KY *  * +KE\n"
			 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA+KI * +OU *  * +HI * \n"
			 "P9+KY * +GI *  * +KI+GI *  * \n"
			 "+\n").initialState());
  
  checkmate_t checkmate;
  SimpleHashTable table(1000000, -1);
  CountRecorder recorder;
  AlphaBeta2ProgressEval searcher(state, checkmate, &table, recorder);

  MoveVector moves;
  state.generateLegal(moves);
  
  searcher.setRootIgnoreMoves(&moves, true);

  const Move move = searcher.computeBestMoveIteratively(1000, 200);
  std::cerr << move << "\n";
  BOOST_CHECK(move.isInvalid());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
