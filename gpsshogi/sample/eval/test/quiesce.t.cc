/* quiesce.t.cc
 */
#include "quiesce.h"
#include "eval/eval.h"
#include "osl/csa.h"
#include "osl/eval/pieceEval.h"

#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::eval;
extern bool isShortTest;

int Gold() { return gpsshogi::PieceEval().value(GOLD); }
int Bishop() { return gpsshogi::PieceEval().value(BISHOP); }
int Rook() { return gpsshogi::PieceEval().value(ROOK); }

BOOST_AUTO_TEST_CASE(QuiesceTestPv)
{
  NumEffectState state((CsaString(
			  "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			  "P2 * -HI *  *  *  *  * -KA *	\n"
			  "P3-FU-FU-FU-FU-FU * -FU-FU-FU\n"
			  "P4 *  *  *  *  * -FU *  *  *	\n"
			  "P5 *  *  *  *  *  *  *  *  *	\n"
			  "P6 *  * +FU *  *  *  *  *  *	\n"
			  "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI *	\n"
			  "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			  "+\n").initialState()));
  
  gpsshogi::PVVector pv;
  int value;
  gpsshogi::PieceEval eval;
  gpsshogi::Quiesce quiesce(&eval, 1, 4);
  quiesce.quiesce(state, value, pv, -300, +300);
  BOOST_REQUIRE(pv.size() > 0);
}

BOOST_AUTO_TEST_CASE(QuiesceTestConstruct)
{
  gpsshogi::PieceEval eval;
  {
    NumEffectState state((SimpleState(HIRATE)));
    gpsshogi::Quiesce quiesce(&eval);
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(0, value);
    BOOST_CHECK(pv.empty());
  }
}

BOOST_AUTO_TEST_CASE(QuiesceTestCapture)
{
  gpsshogi::PieceEval eval;
  gpsshogi::Quiesce quiesce(&eval);
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  * -KI * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "P+00FU\n"
			   "+\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(Gold()*2, value);
    const Move m25hi(Square(2,8),Square(2,5),ROOK,GOLD,false,BLACK);
    BOOST_CHECK_EQUAL((size_t)1, pv.size());
    BOOST_CHECK_EQUAL(m25hi, pv[0]);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  * -KI * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "P+00FU\n"
			   "+\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(0, value);
    BOOST_CHECK_EQUAL((size_t)0, pv.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI-GI-KE-KY\n"
			   "P2 *  *  *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * -HI *  *  *  *  * -KI\n"
			   "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "+\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(Rook()*2, value);
    BOOST_CHECK_EQUAL((size_t)1, pv.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI * -OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4+HI *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  * -KI\n"
			   "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  *  *  * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "+\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(0, value);
    BOOST_CHECK_EQUAL((size_t)0, pv.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 *  *  *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4+KI *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  * -HI\n"
			   "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU * +GI+KE+KY\n"
			   "+\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(Rook()*2 - Gold()*2, value);
    BOOST_CHECK_EQUAL((size_t)2, pv.size());
  }
}

BOOST_AUTO_TEST_CASE(QuiesceTestCheckmate)
{
  gpsshogi::PieceEval eval;
  gpsshogi::Quiesce quiesce(&eval);
  BOOST_REQUIRE(quiesce.infty(BLACK) > quiesce.infty(WHITE));
  BOOST_REQUIRE(quiesce.infty(BLACK) > Rook()*4);
  {
    NumEffectState state(CsaString(
			   "P1 *  *  * -OU *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  * +FU *  *  *  *  * \n"
			   "P4 *  *  * +KY *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  * +OU *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(quiesce.infty(BLACK), value);
    const Move m62to(Square(6,3),Square(6,2),PPAWN,PTYPE_EMPTY,true,BLACK);
    BOOST_CHECK_EQUAL((size_t)1, pv.size());
    BOOST_CHECK_EQUAL(m62to, pv[0]);
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  * -OU *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  * -KY *  *  *  *  * \n"
			   "P7 *  *  * -FU *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  * +OU *  *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK_EQUAL(quiesce.infty(WHITE), value);
    const Move m68to(Square(6,7),Square(6,8),PPAWN,PTYPE_EMPTY,true,WHITE);
    BOOST_CHECK_EQUAL((size_t)1, pv.size());
    BOOST_CHECK_EQUAL(m68to, pv[0]);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			   "P4 *  *  *  *  *  * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  *  *  *  *  * \n"
			   "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA * +GI *  *  * +HI * \n"
			   "P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
			   "-\n").initialState());
    gpsshogi::PVVector pv;
    int value;
    quiesce.quiesce(state, value, pv);
    BOOST_CHECK(value < -Bishop()*2);
    const Move m88um(Square(2,2),Square(8,8),PBISHOP,BISHOP,true,WHITE);
    BOOST_CHECK_EQUAL((size_t)1, pv.size());
    BOOST_CHECK_EQUAL(m88um, pv[0]);

    quiesce.clear();
    pv.clear();

    quiesce.quiesce(state, value, pv, -300, 300);
    BOOST_CHECK(value < -Bishop()*2);
    BOOST_CHECK_EQUAL((size_t)1, pv.size());
    BOOST_CHECK_EQUAL(m88um, pv[0]);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
