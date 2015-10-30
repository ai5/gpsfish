/* progressEval.t.cc
 */
#include "osl/eval/endgame/attackDefense.h"
#include "osl/eval/progressEval.h"
#include "osl/csa.h"
#include "consistencyTest.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::eval;

BOOST_AUTO_TEST_CASE(ProgressEvalTestBeginning)
{
  const NumEffectState state(CsaString(
			       "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			       "P2 * -HI *  *  *  *  * -KA * \n"
			       "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			       "P4 *  *  *  *  *  *  *  *  * \n"
			       "P5 *  *  *  *  *  *  *  *  * \n"
			       "P6 *  *  *  *  *  *  *  *  * \n"
			       "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			       "P8 * +KA *  *  *  *  * +HI * \n"
			       "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			       "+\n").initialState());
  ProgressEval eval(state);
  BOOST_CHECK_EQUAL(0, eval.progress16().value());
  ProgressEval::opening_eval_t opening(state);
#ifdef EXPERIMENTAL_EVAL
  BOOST_CHECK(opening.value() == 0);
#else
  BOOST_CHECK(opening.value() != 0);
#endif
  BOOST_CHECK(abs(opening.value()*16 - eval.value())
		 < ProgressEval::ROUND_UP);
}

BOOST_AUTO_TEST_CASE(ProgressEvalTestInitial)
{
  const NumEffectState state(CsaString(
			       "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			       "P2 * -HI *  *  *  *  * -KA * \n"
			       "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			       "P4 *  *  *  *  *  *  *  *  * \n"
			       "P5 *  *  *  *  *  *  *  *  * \n"
			       "P6 *  *  *  *  *  *  *  *  * \n"
			       "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			       "P8 * +KA *  *  *  *  * +HI * \n"
			       "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			       "+\n").initialState());
  ProgressEval eval(state);
  BOOST_CHECK_EQUAL(0, eval.endgameValue());
}


BOOST_AUTO_TEST_CASE(ProgressEvalTestLoad)
{
  BOOST_CHECK(ProgressEval::setUp());
}

BOOST_AUTO_TEST_CASE(ProgressEvalTestConsistentUpdate)
{
  consistencyTestUpdate<ProgressEval>();
}

BOOST_AUTO_TEST_CASE(ProgressEvalTestConsistentExpect)
{
#ifndef EXPERIMENTAL_EVAL
  consistencyTestExpect<ProgressEval>(16000); // 48000
#endif
}

BOOST_AUTO_TEST_CASE(ProgressEvalTestPinBonus)
{
  {
    const NumEffectState state(
      CsaString("P1-KY-KE *  *  *  *  * -KE-KY\n"
		"P2 * -HI *  *  *  * -KI-OU * \n"
		"P3-FU-FU-FU-FU * -KI-GI-FU-FU\n"
		"P4 *  *  *  * -FU-KA-FU *  * \n"
		"P5 *  *  *  *  * -FU *  *  * \n"
		"P6 *  * +FU+FU+FU *  *  *  * \n"
		"P7+FU+FU * +KI * +FU+FU+FU+FU\n"
		"P8 * +OU+GI *  *  *  * -HI * \n"
		"P9+KY+KE+KA+KI *  *  * +KE+KY\n"
		"P+00GI\n"
		"P-00GI\n"
		"+\n").initialState());
    ProgressEval eval(state);
    endgame::AttackDefense endgame(state);
    const Piece black_king = state.kingPiece<BLACK>();
    const Piece white_king = state.kingPiece<WHITE>();
    const int bonus = eval.calculatePinBonus(state);
    BOOST_CHECK(bonus < 0);
    BOOST_CHECK_EQUAL(
      -(endgame.valueOf(black_king, white_king,
			state.pieceAt(Square(6, 6))) +
	endgame.valueOf(black_king, white_king,
			state.pieceAt(Square(7, 8)))) / 4 *
      eval.progress16().value() / 16,
      bonus);
  }

  {
    const NumEffectState state(
      CsaString("P1-KY-KE *  *  *  *  * -KE-KY\n"
		"P2 * +HI *  *  *  * -KI-OU * \n"
		"P3-FU-FU-FU-FU * -KI-GI-FU-FU\n"
		"P4 *  *  *  * -FU-KA-FU *  * \n"
		"P5 *  *  *  *  * -FU *  *  * \n"
		"P6 *  * +FU+FU+FU *  *  *  * \n"
		"P7+FU+FU+KE+KI * +FU+FU+FU+FU\n"
		"P8 * +OU+GI *  *  *  * +HI * \n"
		"P9+KY * +KA+KI *  *  * +KE+KY\n"
		"P+00GI\n"
		"P-00GI\n"
		"+\n").initialState());
    ProgressEval eval(state);
    endgame::AttackDefense endgame(state);
    const Piece black_king = state.kingPiece<BLACK>();
    const Piece white_king = state.kingPiece<WHITE>();
    const int bonus = eval.calculatePinBonus(state);
    BOOST_CHECK(bonus > 0);
    BOOST_CHECK_EQUAL(
      -(endgame.valueOf(black_king, white_king,
		       state.pieceAt(Square(3, 2))) / 4) *
      eval.progress16().value() / 16,
      bonus);
  }
}

BOOST_AUTO_TEST_CASE(ProgressEvalTestSeeScale)
{
  BOOST_CHECK_EQUAL(16, ProgressEval::seeScale());
}

BOOST_AUTO_TEST_CASE(ProgressEvalTestSymmetry)
{
  using namespace osl;

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;

    NumEffectState state(record.initialState());
    ProgressEval eval(state);
    for (unsigned int i=0; i<moves.size(); i++){
      const Move m = moves[i];
      state.makeMove(m);
      eval.update(state, m);

      NumEffectState state_r(state.rotate180());
      ProgressEval eval_r(state_r);
      ProgressDebugInfo debug = eval.debugInfo(state);
      ProgressDebugInfo debug_r = eval_r.debugInfo(state_r);
      // No opening value here because it's known to by assymetric.
      BOOST_CHECK_EQUAL(debug.endgame, -debug_r.endgame);
      BOOST_CHECK_EQUAL(debug.progress_bonus, -debug_r.progress_bonus);
      BOOST_CHECK_EQUAL(debug.progress_independent_bonus,
			   -debug_r.progress_independent_bonus);
      if (debug.minor_piece_bonus !=
	  -debug_r.minor_piece_bonus)
      {
	std::cout << state << moves[i]
		  << std::endl
		  << debug.minor_piece_bonus << " " << debug_r.minor_piece_bonus<< std::endl;
      }
      BOOST_CHECK_EQUAL(debug.progress_dependent_bonus,
			   -debug_r.progress_dependent_bonus);
      BOOST_CHECK_EQUAL(debug.minor_piece_bonus,
			   -debug_r.minor_piece_bonus);
    }
  }
}

BOOST_AUTO_TEST_CASE(ProgressEvalTestArray)
{
  ProgressEval evals[3];
  (void)evals[0];
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
