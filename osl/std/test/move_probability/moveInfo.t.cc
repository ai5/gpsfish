#include "osl/move_probability/feature.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::move_probability;

BOOST_AUTO_TEST_CASE(MPMoveInfoTestAdhocAdjustSlider)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE * -KI-OU-KI-GI-KE-KY\n"
			 "P2 *  *  *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-HI-FU-FU-FU-FU\n"
			 "P4 *  *  *  * -GI *  *  *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  *  *  *  *  *  * \n"
			 "P7+FU+FU+FU+FU+HI+FU+FU+FU+FU\n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			 "P+00KA\n"
			 "P-00AL\n"
			 "+\n").initialState());
  MoveStack history;
  const Move m45ka(Square(4,5),BISHOP,BLACK);
  const StateInfo si(state,Progress16(0),history);
  const MoveInfo mi(si, m45ka);
  BOOST_CHECK(mi.adhocAdjustSlider(si));
}

BOOST_AUTO_TEST_CASE(MPMoveInfoTestAdhocAdjustBishopFork)
{
  MoveStack history;
  {
    NumEffectState state(CsaString(
			   "P1 * +HI * +RY *  *  * -KE-KY\n"
			   "P2 *  * +TO *  *  * -KI-OU * \n"
			   "P3-FU *  *  *  * -KI * -FU-FU\n"
			   "P4 *  * -UM * -FU-FU-FU-GI * \n"
			   "P5 *  *  *  *  *  *  * +KE+FU\n"
			   "P6 *  * +KI *  *  * +FU+FU * \n"
			   "P7+FU-NY *  * +GI+FU+KE *  * \n"
			   "P8 *  *  * +OU * +GI *  *  * \n"
			   "P9+KY *  *  *  *  *  *  *  * \n"
			   "P+00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KA00KI00GI00FU\n"
			   "-\n").initialState());
    const Move m45ka(Square(5,1),GOLD,WHITE);
    const StateInfo si(state,Progress16(0),history);
    const MoveInfo mi(si, m45ka);
    BOOST_CHECK(! mi.adhocAdjustSlider(si));
    BOOST_CHECK(mi.adhocAdjustBishopFork(si));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -KI-OU * -KE-KY\n"
			   "P2 *  * +RY *  *  * -KI *  * \n"
			   "P3-KY *  *  *  * -GI * -FU * \n"
			   "P4 *  *  *  *  * -RY *  * -FU\n"
			   "P5-FU *  *  * -FU *  *  *  * \n"
			   "P6 *  * +FU+GI+FU *  *  * +FU\n"
			   "P7+FU+FU * +FU *  *  *  *  * \n"
			   "P8 *  * +OU *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KA00KA00KI00GI00GI00KE00KE00FU00FU00FU\n"
			   "P-00KI00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    const Move m45ke(Square(4,5),KNIGHT,BLACK);
    const StateInfo si(state,Progress16(0),history);
    const MoveInfo mi(si, m45ke);
    BOOST_CHECK(mi.adhocAdjustBishopFork(si));
  }
}

BOOST_AUTO_TEST_CASE(MPMoveInfoTestAdhocAdjustBreakThreatmate)
{
  MoveStack history;
  {
    NumEffectState state(CsaString(
			   "P1 * -KE *  *  *  * -KI *  * \n"
			   "P2 *  *  *  * -GI-OU * -GI * \n"
			   "P3 *  * +NY *  *  *  * -FU * \n"
			   "P4 *  *  *  * -FU-FU-FU * +RY\n"
			   "P5+KA *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  * +GI *  *  * \n"
			   "P7 *  *  * +FU+FU-GI *  * +FU\n"
			   "P8 * -RY *  *  *  * +KI *  * \n"
			   "P9+KY+KE-TO *  *  * +OU+KE+KY\n"
			   "P+00KI00KE00KY00FU00FU00FU00FU\n"
			   "P-00KA00KI00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    const Move m38ry(Square(4,7),Square(3,8),PSILVER,GOLD,true,WHITE);
    const Move m86ka(Square(9,5),Square(8,6),BISHOP,PTYPE_EMPTY,false,BLACK);
    StateInfo si(state,Progress16(0),history,m38ry);
    BOOST_CHECK(si.threatmate_move.isNormal());
    const MoveInfo mi(si, m86ka);
    BOOST_CHECK(mi.adhocAdjustBreakThreatmate(si));
  }
}

BOOST_AUTO_TEST_CASE(MPMoveInfoTestAdhocAdjustAttackCheckmateDefender)
{
  MoveStack history;
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
    const Move m35hi(Square(3,5),ROOK,BLACK);
    const StateInfo si(state,Progress16(0),history);
    const MoveInfo mi(si, m35hi);
    BOOST_CHECK(mi.adhocAdjustAttackCheckmateDefender(si));
    // todo: cases PAWN at 56 instead of 55 => bishop fork at 44 after exchanges
  }
}

BOOST_AUTO_TEST_CASE(MPMoveInfoTestAdhocAdjustKeepCheckmateDefender)
{
  MoveStack history;
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
    const Move m79um(Square(8,8),Square(7,9),PBISHOP,PBISHOP,false,BLACK);
    const StateInfo si(state,Progress16(0),history);
    const MoveInfo mi(si, m79um);
    BOOST_CHECK(mi.adhocAdjustKeepCheckmateDefender(si));
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
			   "P8+OU+UM-UM *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  * +KE+KY\n"
			   "P+00KE00KE00KY\n"
			   "P-00KI00KI00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    const Move m78um(Square(8,8),Square(7,8),PBISHOP,PBISHOP,false,BLACK);
    const StateInfo si(state,Progress16(0),history);
    const MoveInfo mi(si, m78um);
    BOOST_CHECK(! mi.adhocAdjustKeepCheckmateDefender(si));
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
