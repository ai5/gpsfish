#include "stateInfo.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(StateInfoTestStateInfo)
{  
  std::vector<Move> history;
  {
    NumEffectState state;
    gpsshogi::StateInfo info(state, history, -1);
    BOOST_CHECK(info.pin_by_opposing_sliders.empty());    
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  * +RY *  * -GI-KE-OU\n"
			   "P2 *  *  *  *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  *  * -FU-FU\n"
			   "P4-FU * -FU *  * -FU *  *  * \n"
			   "P5 * -FU * -KA *  *  *  * +FU\n"
			   "P6+FU * +FU *  *  *  * +KY * \n"
			   "P7 * +FU *  *  * +FU+GI+FU * \n"
			   "P8 *  *  *  *  * +KI *  * +KY\n"
			   "P9 *  *  * -RY+FU * +KE+KE+OU\n"
			   "P+00KA00KI00KY\n"
			   "P-00KI00GI00KE00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    gpsshogi::StateInfo info(state, history, -1);
    BOOST_CHECK(info.pin_by_opposing_sliders.size() == 1);    
    BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(6,5)), info.pin_by_opposing_sliders[0]);
  }
}

BOOST_AUTO_TEST_CASE(StateInfoTestEquality)
{  
  {
    NumEffectState state(CsaString(
			   "P1 *  *  * +RY *  * -GI-KE-OU\n"
			   "P2 *  *  *  *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  *  * -FU-FU\n"
			   "P4-FU * -FU *  * -FU *  *  * \n"
			   "P5 * -FU * -KA *  *  *  * +FU\n"
			   "P6+FU * +FU *  *  *  * +KY * \n"
			   "P7 * +FU *  *  * +FU+GI+FU * \n"
			   "P8 *  *  *  *  * +KI *  * +KY\n"
			   "P9 *  *  * -RY+FU * +KE+KE+OU\n"
			   "P+00KA00KI00KY\n"
			   "P-00KI00GI00KE00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    std::vector<Move> history;
    const Move m33ky(Square(3,3),LANCE,BLACK);
    history.push_back(m33ky);
    gpsshogi::StateInfo info(state, history, -1);
    gpsshogi::StateInfo info2(state, history, -1);
    BOOST_CHECK(info == info2);
    state.makeMove(m33ky);
    info.update(m33ky);
    BOOST_CHECK(!(info == info2));
    gpsshogi::StateInfo info3(state, history, 0);
    BOOST_CHECK(info == info3);
  }
}

BOOST_AUTO_TEST_CASE(StateInfoTestFindCheckmateDefender)
{
  const std::vector<Move> moves;
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  * +GI-KE-KY\n"
			   "P2 *  *  *  *  *  * -HI * -OU\n"
			   "P3-FU-FU-FU-FU *  *  * -FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * +FU *  * +FU * \n"
			   "P6+FU+KA+FU-UM *  *  *  *  * \n"
			   "P7 * +FU+KI *  *  *  *  * +FU\n"
			   "P8+OU-GI-KI * -NY *  *  *  * \n"
			   "P9+KY+KE * -GI *  *  * +KE * \n"
			   "P+00HI00KI00KI00GI00FU\n"
			   "P-00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    BOOST_CHECK(std::make_pair(state.pieceAt(Square(3,2)), Square(2,2))
		== gpsshogi::StateInfo::findCheckmateDefender(state, WHITE));
    // todo: cases PAWN at 56 instead of 55 => +0035HI, bishop fork at 44 after exchanges
  }
  {
    NumEffectState state(CsaString(
			   "P1+RY+HI *  * -FU *  *  * -KY\n"
			   "P2 *  *  *  * -KI *  * -OU * \n"
			   "P3 * -FU+GI-GI *  * -KI-FU * \n"
			   "P4 *  * -FU-FU * -FU *  * -FU\n"
			   "P5-FU *  *  *  *  *  * +FU * \n"
			   "P6 * +GI-GI *  *  *  *  * +FU\n"
			   "P7+FU *  *  * +FU+FU *  *  * \n"
			   "P8+OU+UM *  *  *  *  *  *  * \n"
			   "P9+KY+KE-UM *  *  *  * +KE+KY\n"
			   "P+00KE00KE00KY\n"
			   "P-00KI00KI00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    BOOST_CHECK(std::make_pair(state.pieceAt(Square(8,8)), Square(8,7))
		== gpsshogi::StateInfo::findCheckmateDefender(state, BLACK));
  }
}

BOOST_AUTO_TEST_CASE(StateInfoTestThreatmateByCapture)
{
  std::vector<Move> moves;
  { 
    NumEffectState state(CsaString(
			   "P1+UM *  *  *  *  *  *  * -KY\n"
			   "P2 *  * -GI-KI * -GI *  *  * \n"
			   "P3-OU-FU *  *  * -FU *  *  * \n"
			   "P4-KE+FU * -FU *  * -FU * -FU\n"
			   "P5+OU * +HI-KE * +KE * -FU+FU\n"
			   "P6 *  * +FU *  * +GI+FU-KE * \n"
			   "P7 * +GI * +KI *  *  *  *  * \n"
			   "P8 *  * -TO+FU *  *  * -RY * \n"
			   "P9+KY-KI *  *  *  *  *  * +KY\n"
			   "P+00KY00FU00FU\n"
			   "P-00KA00KI00FU00FU00FU00FU\n"
			   "+\n").initialState());
    gpsshogi::StateInfo info(state, moves, -1);
    BOOST_CHECK_EQUAL(((1<<(KNIGHT-PTYPE_BASIC_MIN))
	       | (1<<(SILVER-PTYPE_BASIC_MIN))
	       | (1<<(GOLD-PTYPE_BASIC_MIN))
	       | (1<<(BISHOP-PTYPE_BASIC_MIN))
	       | (1<<(ROOK-PTYPE_BASIC_MIN))),
	      info.possible_threatmate_ptype);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
