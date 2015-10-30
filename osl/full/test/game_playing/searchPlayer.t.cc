#include "osl/game_playing/searchPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/search/timeControl.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::game_playing;

BOOST_AUTO_TEST_CASE(SearchPlayerTestSecondsForThisMove)
{
  const int moves_penalty = 1-240;
  {
    const SimpleState state(CsaString(
			      "P1-KY-KE-GI-KI * -KI-GI-KE-KY\n"
			      "P2 * -HI *  *  *  *  * -KA * \n"
			      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			      "P4 *  *  *  * -OU *  *  *  * \n"
			      "P5 *  *  *  *  *  *  *  *  * \n"
			      "P6 *  *  *  * +OU *  *  *  * \n"
			      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			      "P8 * +KA *  *  *  *  * +HI * \n"
			      "P9+KY+KE+GI+KI * +KI+GI+KE+KY\n"
			      "+\n").initialState());
    const GameState gs(state);
    const int e1400 = search::TimeControl::secondsForThisMove(1400);
    BOOST_CHECK_EQUAL(e1400, SearchPlayer::secondsForThisMove(gs, 1500-moves_penalty, 100, 0, 0));

    const int e300 = search::TimeControl::secondsForThisMove(300);
    BOOST_CHECK_EQUAL(e300, SearchPlayer::secondsForThisMove(gs, 1500-moves_penalty, 1200, 0, 0));

    BOOST_CHECK_EQUAL(1, SearchPlayer::secondsForThisMove(gs, 0, 0, 0, 0));
    BOOST_CHECK_EQUAL(-1, SearchPlayer::secondsForThisMove(gs, 0, -1, -1, 0));
  }
  {
    // 終盤で時間が豊富にあれば倍
    const SimpleState state(CsaString(
			      "P1 *  *  *  * -OU *  *  *  * \n"
			      "P2 *  *  *  *  *  *  *  *  * \n"
			      "P3 *  *  * +KI+GI+KI *  *  * \n"
			      "P4 *  *  *  *  *  *  *  *  * \n"
			      "P5 *  *  *  *  *  *  *  *  * \n"
			      "P6 *  *  *  *  *  *  *  *  * \n"
			      "P7 *  *  * -KI-GI-KI *  *  * \n"
			      "P8 *  *  *  *  *  *  *  *  * \n"
			      "P9 *  *  *  * +OU *  *  *  * \n"
			      "P-00AL\n"
			      "-\n").initialState());
    const GameState gs(state);
    const int e1400 = search::TimeControl::secondsForThisMove(1400);
    BOOST_CHECK_EQUAL(e1400*2,
			 SearchPlayer::secondsForThisMove(gs, 1500-moves_penalty, 100, 0, 0));

    const int e300 = search::TimeControl::secondsForThisMove(300);
    BOOST_CHECK_EQUAL(e300, SearchPlayer::secondsForThisMove(gs, 1500-moves_penalty, 1200, 0, 0));

    BOOST_CHECK_EQUAL(1, SearchPlayer::secondsForThisMove(gs, 0, 0, 0, 0));
    BOOST_CHECK_EQUAL(-1, SearchPlayer::secondsForThisMove(gs, 0, -1, -1, 0));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
