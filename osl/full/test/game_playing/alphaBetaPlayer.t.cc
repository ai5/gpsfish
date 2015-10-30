#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/eval/progressEval.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::game_playing;


static void load()
{
  static bool loaded = osl::eval::ProgressEval::setUp();
  BOOST_CHECK(loaded);
}

const int seconds = 5;
BOOST_AUTO_TEST_CASE(AlphaBetaPlayerTestResign)
{
  load();
  const SimpleState state(CsaString(
			 "P1 *  *  *  * -OU *  *  *  * \n"
			 "P2 *  *  *  * +KI *  *  *  * \n"
			 "P3 *  *  *  * +FU *  *  *  * \n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  *  *  *  *  *  * \n"
			 "P7 *  *  *  *  *  *  *  *  * \n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9 *  *  *  * +OU *  *  *  * \n"
			 "P-00AL\n"
			 "-\n").initialState());
  const GameState gs(state);
  AlphaBeta2ProgressEvalPlayer player;
  const MoveWithComment best_move = player.selectBestMove(gs, 1500, seconds, 0);
  BOOST_CHECK_EQUAL(Move::INVALID(), best_move.move);
}

BOOST_AUTO_TEST_CASE(AlphaBetaPlayerTestWin)
{
  load();
  const SimpleState state(CsaString(
			 "P1 *  *  *  * -OU *  *  *  * \n"
			 "P2 *  *  *  *  *  *  *  *  * \n"
			 "P3 *  *  *  * +FU *  *  *  * \n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  *  *  *  *  *  * \n"
			 "P7 *  *  *  *  *  *  *  *  * \n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9 *  *  *  * +OU *  *  *  * \n"
			 "P+00KI\n"
			 "P-00AL\n"
			 "+\n").initialState());
  const GameState gs(state);
  AlphaBeta2ProgressEvalPlayer player;
  const MoveWithComment best_move = player.selectBestMove(gs, 1500, seconds, 0);
  const Move expected(Square(5,2), GOLD, BLACK);
  BOOST_CHECK_EQUAL(expected, best_move.move);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
