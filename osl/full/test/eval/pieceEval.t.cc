/* pieceEval.t.cc
 */
#include "osl/eval/pieceEval.h"
#include "osl/eval/pieceEval.tcc"
#include "osl/numEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "consistencyTest.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::eval;

const int Gold = PtypeEvalTraits<GOLD>::val;
const int Rook = PtypeEvalTraits<ROOK>::val;
const int Knight = PtypeEvalTraits<KNIGHT>::val;
const int Pawn = PtypeEvalTraits<PAWN>::val;
const int Pknight = PtypeEvalTraits<PKNIGHT>::val;
const int Bishop = PtypeEvalTraits<BISHOP>::val;
const int Pbishop = PtypeEvalTraits<PBISHOP>::val;

BOOST_AUTO_TEST_CASE(PieceEvalTestSoma)
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
  const PieceEval ev(state);
  {
    int ret=ev.computeDiffAfterMove
      (state, Move(Square(1,5),Square(1,4),PAWN,PAWN,false,BLACK));
    /** 相手が取り替えさないのが最善 */
    BOOST_CHECK_EQUAL(+Pawn*2, ret);
  }
  {
    int ret=ev.computeDiffAfterMove
      (state, Move(Square(6,5),Square(9,2),PBISHOP,PTYPE_EMPTY,true,BLACK));
    /** 香車取り返し only */
    BOOST_CHECK_EQUAL(-Bishop*2, ret);
  }
  {
    int ret=ev.computeDiffAfterMove
      (state, Move(Square(6,5),Square(8,3),PBISHOP,PTYPE_EMPTY,true,BLACK));
    /** 成り得 */
    BOOST_CHECK_EQUAL(+Pbishop-Bishop, ret);
  }
  {
    int ret=ev.computeDiffAfterMove
      (state, Move(Square(6,5),Square(2,1),PBISHOP,KNIGHT,true,BLACK));
    BOOST_CHECK_EQUAL(+Knight*2-Bishop*2, ret);
  }
  {
    int ret=ev.computeDiffAfterMove
      (state, Move(Square(8,4),Square(7,2),PKNIGHT,PTYPE_EMPTY,true,BLACK));
    BOOST_CHECK_EQUAL(-Knight*2, ret);
  }
  {
    int ret1=ev.computeDiffAfterMove
      (state, Move(Square(6,5),Square(5,4),BISHOP,PTYPE_EMPTY,false,BLACK));
    BOOST_CHECK_EQUAL(-Bishop*2, ret1);
  }
}

BOOST_AUTO_TEST_CASE(PieceEvalTestBug)
{
  const NumEffectState state=CsaString(
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
    "+\n").initialState();
  const PieceEval ev(state);
  int ret=ev.computeDiffAfterMove<BLACK>
    (state, Move(Square(5,6),Square(5,5),PAWN,PTYPE_EMPTY,false,BLACK));
  BOOST_CHECK_EQUAL(0, ret);
}

BOOST_AUTO_TEST_CASE(PieceEvalTestCannotTake)
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
    const int ret
      = PieceEval::computeDiffAfterMoveForRP(state, m);
    // 相手は取れない
    BOOST_CHECK_EQUAL(0, ret);
  }
}

BOOST_AUTO_TEST_CASE(PieceEvalTestComputeDiffAfterMoveForRP)
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
    const int ret
      = PieceEval::computeDiffAfterMoveForRP(state, m);
    // 相手が取り替えさないのが最善
    BOOST_CHECK_EQUAL(Pawn*2, ret);
  }
  {
    Move m = Move(Square(8,4),Square(7,2),PKNIGHT,PTYPE_EMPTY,true,BLACK);
    const int ret
      = PieceEval::computeDiffAfterMoveForRP(state, m);
    // ただ捨て
    BOOST_CHECK_EQUAL(-Knight*2, ret);
  }
  state.changeTurn();
  {
    Move m = Move(Square(8,5),Square(8,6),PAWN,PTYPE_EMPTY,false,WHITE);
    const int ret
      = PieceEval::computeDiffAfterMoveForRP(state, m);
    // ただ捨て
    BOOST_CHECK_EQUAL(-Pawn*2, ret);
  }
  {
    Move m = Move(Square(8,2),Square(8,4),ROOK,KNIGHT,false,WHITE);
    const int ret
      = PieceEval::computeDiffAfterMoveForRP(state, m);
    // ただ取り
    BOOST_CHECK_EQUAL(Knight*2, ret);
  }
}

BOOST_AUTO_TEST_CASE(PieceEvalTestComputeDiffAfterMoveForRPBug030910)
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
    // こちらは通る
    const int ret = PieceEval::computeDiffAfterMoveForRP(state, m);
    BOOST_CHECK_EQUAL(Pawn*2, ret);
    const int rets = 
      PieceEval::computeDiffAfterMoveForRP(state, m);
    BOOST_CHECK_EQUAL(Pawn*2, rets);

  }
  {
    const Square p36 = Square(3,6);
    Move m = Move(Square(3,5),p36,PAWN,PAWN,false,WHITE);
    // EffectUtil::showEffect(state, p36, std::cerr);
    BOOST_CHECK_EQUAL(-Pawn*2, 
			 Ptype_Eval_Table.diffWithMove(state, m));

    const int ret = PieceEval::computeDiffAfterMoveForRP(state, m);
    // ただ取りなので得するはず
    BOOST_CHECK_EQUAL(Pawn*2, ret);
    const int rets = 
      PieceEval::computeDiffAfterMoveForRP(state, m);
    BOOST_CHECK_EQUAL(Pawn*2, rets);
  }
}

BOOST_AUTO_TEST_CASE(PieceEvalTestConsistentUpdate)
{
  consistencyTestUpdate<PieceEval>();
}

BOOST_AUTO_TEST_CASE(PieceEvalTestConsistentExpect)
{
  consistencyTestExpect<PieceEval>();
}

BOOST_AUTO_TEST_CASE(PieceEvalTestArray)
{
  PieceEval evals[3];
  (void)evals[0];
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
