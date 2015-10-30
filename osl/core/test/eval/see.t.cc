/* see.t.cc
 */
#include "osl/eval/see.h"
#include "osl/eval/ptypeEval.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::eval;

const int Gold = PtypeEvalTraits<GOLD>::val;
const int Silver = PtypeEvalTraits<SILVER>::val;
const int Rook = PtypeEvalTraits<ROOK>::val;
const int Knight = PtypeEvalTraits<KNIGHT>::val;
const int Pawn = PtypeEvalTraits<PAWN>::val;
const int Pknight = PtypeEvalTraits<PKNIGHT>::val;
const int Bishop = PtypeEvalTraits<BISHOP>::val;
const int Pbishop = PtypeEvalTraits<PBISHOP>::val;

BOOST_AUTO_TEST_CASE(SeeTestSee)
{
  const NumEffectState state(CsaString(
			       "P1-KY-KE-GI-KI *  * -HI-KE * \n"
			       "P2 * -HI *  * -KA-OU *  *  * \n"
			       "P3-FU * -FU-FU-FU * -GI-FU-KY\n"
			       "P4 * +KE *  *  * -FU *  * -FU\n"
			       "P5 * -FU+KE+KA *  *  *  * +FU\n"
			       "P6 *  * +FU *  *  * +FU *  * \n"
			       "P7+FU+FU * +FU+FU+FU *  *  * \n"
			       "P8 * +GI+KI *  *  * +GI *  * \n"
			       "P9+KY *  *  * +OU+KI *  * +KY\n"
			       "P+00FU\n"
			       "P+00FU\n"
			       "P+00KI\n"
			       "+\n").initialState());
  {
    int ret=See::see
      (state, Move(Square(1,5),Square(1,4),PAWN,PAWN,false,BLACK));
    /** 相手が取り替えさないのが最善 */
    BOOST_CHECK_EQUAL(+Pawn*2, ret);
  }
  {
    int ret=See::see
      (state, Move(Square(6,5),Square(9,2),PBISHOP,PTYPE_EMPTY,true,BLACK));
    /** 香車取り返し only */
    BOOST_CHECK_EQUAL(-Bishop*2, ret);
  }
  {
    int ret=See::see
      (state, Move(Square(6,5),Square(8,3),PBISHOP,PTYPE_EMPTY,true,BLACK));
    /** 成り得 */
    BOOST_CHECK_EQUAL(+Pbishop-Bishop, ret);
  }
  {
    int ret=See::see
      (state, Move(Square(6,5),Square(2,1),PBISHOP,KNIGHT,true,BLACK));
    BOOST_CHECK_EQUAL(+Knight*2-Bishop*2, ret);
  }
  {
    int ret=See::see
      (state, Move(Square(8,4),Square(7,2),PKNIGHT,PTYPE_EMPTY,true,BLACK));
    BOOST_CHECK_EQUAL(-Knight*2, ret);
  }
  {
    int ret1=See::see
      (state, Move(Square(6,5),Square(5,4),BISHOP,PTYPE_EMPTY,false,BLACK));
    BOOST_CHECK_EQUAL(-Bishop*2, ret1);
  }
  
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE * -KI *  *  * -KE-KY\n"
				 "P2 * -OU-GI * -KI * -HI *  * \n"
				 "P3 * -FU *  * -FU-GI-KA * -FU\n"
				 "P4 *  * -FU-FU * -FU-FU-FU * \n"
				 "P5-FU *  *  *  *  *  *  *  * \n"
				 "P6 *  * +FU * +FU+GI+FU * +FU\n"
				 "P7 * +FU * +FU * +FU *  *  * \n"
				 "P8 * +KA+OU * +KI+GI * +HI * \n"
				 "P9+KY+KE * +KI *  *  * +KE+KY\n"
				 "P-00FU00FU\n"
				 "+\n").initialState());
    int ret=See::see(state, Move(Square(5,6),Square(5,5),PAWN,PTYPE_EMPTY,false,BLACK));
    BOOST_CHECK_EQUAL(0, ret);
  }
  {
    BOOST_CHECK_EQUAL(0, Ptype_Eval_Table.value(PTYPE_EMPTY));
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  * +RY-KI-OU *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00KI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    {
      Move m = Move(Square(6,3),GOLD,BLACK);
      const int ret = See::see(state, m, PieceMask(), state.pin(WHITE));
      // 相手は取れない
      BOOST_CHECK_EQUAL(0, ret);
    }
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI *  * -HI-KE * \n"
			   "P2 * -HI *  * -KA-OU *  *  * \n"
			   "P3-FU * -FU-FU-FU * -GI-FU-KY\n"
			   "P4 * +KE *  *  * -FU *  * -FU\n"
			   "P5 * -FU+KE+KA *  *  *  * +FU\n"
			   "P6 *  * +FU *  *  * +FU *  * \n"
			   "P7+FU+FU * +FU+FU+FU *  *  * \n"
			   "P8 * +GI+KI *  *  * +GI *  * \n"
			   "P9+KY *  *  * +OU+KI *  * +KY\n"
			   "P+00FU\n"
			   "P+00FU\n"
			   "P+00KI\n"
			   "+\n").initialState());
    {
      Move m = Move(Square(1,5),Square(1,4),PAWN,PAWN,false,BLACK);
      const int ret = See::see(state, m);
      // 相手が取り替えさないのが最善
      BOOST_CHECK_EQUAL(Pawn*2, ret);
    }
    {
      Move m = Move(Square(8,4),Square(7,2),PKNIGHT,PTYPE_EMPTY,true,BLACK);
      const int ret = See::see(state, m);
      // ただ捨て
      BOOST_CHECK_EQUAL(-Knight*2, ret);
    }
    state.changeTurn();
    {
      Move m = Move(Square(8,5),Square(8,6),PAWN,PTYPE_EMPTY,false,WHITE);
      const int ret = See::see(state, m);
      // ただ捨て
      BOOST_CHECK_EQUAL(-Pawn*2, ret);
    }
    {
      Move m = Move(Square(8,2),Square(8,4),ROOK,KNIGHT,false,WHITE);
      const int ret = See::see(state, m);
      // ただ取り
      BOOST_CHECK_EQUAL(Knight*2, ret);
    }
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -HI-KE-KY\n"
			   "P2 *  *  * +RY+UM-GI-KI-OU * \n"
			   "P3+TO *  *  * -FU * -GI *  * \n"
			   "P4-FU *  * -FU * -GI+KY-FU * \n"
			   "P5 * -FU+FU-KI * -FU-FU *  * \n"
			   "P6+FU+FU *  * +KY * +FU * -FU\n"
			   "P7 *  *  *  * +FU+FU * +FU * \n"
			   "P8 *  *  *  * +KI * +GI+OU+FU\n"
			   "P9 *  *  *  *  * +KI * +KE+KY\n"
			   "P-00FU\n"
			   "P-00KE\n"
			   "P-00KE\n"
			   "P+00KA\n"
			   "-\n").initialState());
    {
      const Square p86 = Square(8,6);
      Move m = Move(Square(8,5),p86,PAWN,PAWN,false,WHITE);
      const int ret = See::see(state, m);
      BOOST_CHECK_EQUAL(Pawn*2, ret);

    }
    {
      const Square p36 = Square(3,6);
      Move m = Move(Square(3,5),p36,PAWN,PAWN,false,WHITE);
      const int ret = See::see(state, m);
      // ただ取りなので得するはず
      BOOST_CHECK_EQUAL(Pawn*2, ret);
    }
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  * -KI-OU-KE-KY\n"
			   "P2 *  *  *  * -KI * -GI *  * \n"
			   "P3 *  *  *  * -FU+FU * -FU-FU\n"
			   "P4 *  *  *  *  *  * +GI *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  * +HI *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    // 追加効き
    Move m = Move(Square(3,3),SILVER,BLACK);
    const int ret = See::see(state, m);
    BOOST_CHECK(ret > -Silver*2);
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  * -OU\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  * +FU\n"
			   "P7 *  *  *  *  * +FU+FU+FU * \n"
			   "P8 *  *  *  *  * -GI+GI+OU * \n"
			   "P9 *  * -RY *  * +KI * +KE+KY\n"
			   "P-00AL\n"
			   "-\n").initialState());
    // 影効き
    Move m = Move(Square(3,9),BISHOP,WHITE);
    const int ret = See::see(state, m);
    BOOST_CHECK(ret > -Bishop*2);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  * -KE-KY\n"
			   "P2 * -HI *  *  * -GI-KI-OU * \n"
			   "P3 *  *  * -FU * -KI-GI-FU-FU\n"
			   "P4 *  * -FU * -FU-FU-FU *  * \n"
			   "P5-FU *  * -KE *  *  *  * +FU\n"
			   "P6 * +FU+FU+GI+FU * +FU+FU * \n"
			   "P7+FU *  * +KI * +FU *  * +KY\n"
			   "P8 * +OU+KI *  * +GI *  * +HI\n"
			   "P9+KY+KE *  *  *  *  * +KE * \n"
			   "P+00KA00FU\n"
			   "P-00KA00FU\n"
			   "-\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(8,2),Square(8,5),ROOK,PTYPE_EMPTY,false,WHITE);
    const int ret = See::see(state, m, state.pinOrOpen(WHITE),state.pinOrOpen(BLACK));
    BOOST_CHECK(ret < 0);
  }
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-KI *  *  *  * -KE-KY\n"
			   "P2-KY-GI * -KI *  *  *  *  * \n"
			   "P3-FU-FU * -FU *  *  * -FU-FU\n"
			   "P4 *  * -FU-GI-FU * -FU *  * \n"
			   "P5 *  *  *  *  *  *  * +FU * \n"
			   "P6 *  * +FU *  * +GI+FU * +FU\n"
			   "P7+FU+FU-UM+FU *  *  *  *  * \n"
			   "P8+KY+GI *  *  * +KI * +HI * \n"
			   "P9+OU+KE+KI *  *  *  * +KE+KY\n"
			   "P+00HI00FU00FU\n"
			   "P-00KA00FU\n"
			   "-\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(7,7),Square(8,7),PBISHOP,PAWN,false,WHITE);
    const int ret = See::see(state, m, state.pinOrOpen(WHITE),state.pinOrOpen(BLACK));
    BOOST_CHECK(ret < 0);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  * -KI-OU *  * -KY\n"
			   "P2 *  *  * -GI *  * -KI+TO * \n"
			   "P3 *  * -FU-FU-FU-FU-KE * -FU\n"
			   "P4-FU *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6+FU * +FU *  *  *  * +HI * \n"
			   "P7 *  * +KE+FU+FU+FU+FU * +FU\n"
			   "P8 * -RY * +GI+OU+GI+KI *  * \n"
			   "P9+KY-TO *  *  *  *  * +KE+KY\n"
			   "P+00KA00KA00FU00FU\n"
			   "P-00KI00GI00FU\n"
			   "-\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(8,8),Square(7,7),PROOK,KNIGHT,false,WHITE);
    const int ret = See::see(state, m, state.pinOrOpen(WHITE),state.pinOrOpen(BLACK));
    BOOST_CHECK(ret < 0);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  * -KI-OU-GI * +RY\n"
			   "P2 *  *  * -GI *  * -KI *  * \n"
			   "P3 *  * -FU-FU-FU-FU-KE * -FU\n"
			   "P4-FU *  *  *  *  *  * +HI * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6+FU * +FU *  *  *  *  *  * \n"
			   "P7 *  *  * +FU+FU+FU+FU * +FU\n"
			   "P8 *  *  *  * +OU+GI+KI *  * \n"
			   "P9+KY-UM *  *  *  *  * +KE+KY\n"
			   "P+00KA00KY00FU00FU00FU\n"
			   "P-00KI00GI00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(2,4),Square(2,2),PROOK,PTYPE_EMPTY,true,BLACK);
    const int ret = See::see(state, m, state.pinOrOpen(BLACK),state.pinOrOpen(WHITE));
    BOOST_CHECK(ret < -1000);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI *  * -KI * -KE * \n"
			   "P2 *  *  *  *  * -OU-GI *  * \n"
			   "P3-FU * -FU-FU * -FU * -FU-FU\n"
			   "P4 *  *  * -KI+KY * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  *  *  *  * +FU\n"
			   "P7+FU * -UM+FU * +FU+FU+FU * \n"
			   "P8 * -RY *  * +HI *  * +OU * \n"
			   "P9+KY+KE * +KI * +KI+GI+KE+KY\n"
			   "P+00FU00FU\n"
			   "P-00KA00GI00FU00FU\n"
			   "+\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(5,4),Square(5,3),PLANCE,PTYPE_EMPTY,true,BLACK);
    const int ret = See::see(state, m, state.pinOrOpen(BLACK),state.pinOrOpen(WHITE));
    BOOST_CHECK(ret > 0);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI *  * -KI * -KE * \n"
			   "P2 * -HI *  *  * -OU-GI *  * \n"
			   "P3-FU * -FU-FU * -FU * -FU-FU\n"
			   "P4 *  *  * -KI+KY * -FU *  * \n"
			   "P5 *  *  *  * -FU *  *  *  * \n"
			   "P6 * +UM+FU *  *  *  *  * +FU\n"
			   "P7+FU *  * +FU * +FU+FU+FU * \n"
			   "P8 * +GI *  * +HI+OU *  *  * \n"
			   "P9+KY+KE * +KI * +KI+GI+KE+KY\n"
			   "P+00FU00FU\n"
			   "P-00KA00FU\n"
			   "+\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(5,8),Square(5,5),ROOK,PAWN,false,BLACK);
    const int ret = See::see(state, m, state.pinOrOpen(BLACK),state.pinOrOpen(WHITE));
    BOOST_CHECK(ret > 0);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  * -GI * -KY\n"
			   "P2 * -HI * -GI *  * -KI-OU * \n"
			   "P3-FU *  * -FU-FU * -KE-FU-FU\n"
			   "P4 *  *  *  * -KY+KA-FU *  * \n"
			   "P5 *  * +HI *  * -FU *  *  * \n"
			   "P6+FU+KA * +FU *  *  *  *  * \n"
			   "P7 * +FU *  * +FU+FU+FU+FU+FU\n"
			   "P8 *  *  *  * +KI+OU+GI *  * \n"
			   "P9+KY+KE *  *  * +KI * +KE-NG\n"
			   "P+00FU00FU00FU\n"
			   "P-00KI\n"
			   "+\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(7,5),Square(4,5),ROOK,PAWN,false,BLACK);
    const int ret = See::see(state, m, state.pinOrOpen(BLACK),state.pinOrOpen(WHITE));
    BOOST_CHECK(ret > 0);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  * -KI *  *  *  * -KY\n"
			   "P2 * -GI-OU * +UM *  * -GI * \n"
			   "P3-FU-FU-KE * -FU-FU-KE-FU-FU\n"
			   "P4 *  * +HI *  *  *  *  *  * \n"
			   "P5 *  * +OU * -HI *  *  *  * \n"
			   "P6 *  *  *  * +KA+FU *  *  * \n"
			   "P7+FU+FU * +FU+FU-KI * +FU+FU\n"
			   "P8 * +GI *  *  *  *  *  *  * \n"
			   "P9+KY *  *  *  *  *  * -GI+KY\n"
			   "P+00KE00FU00FU00FU\n"
			   "P-00KI00KI00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    // not pin
    Move m = Move(Square(5,6),Square(6,5),BISHOP,PTYPE_EMPTY,false,BLACK);
    const int ret = See::see(state, m, state.pinOrOpen(BLACK),state.pinOrOpen(WHITE));
    BOOST_CHECK(ret < -1000);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
