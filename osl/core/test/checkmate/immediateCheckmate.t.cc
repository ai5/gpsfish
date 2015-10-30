/* immediateCheckmate.t.cc
 */
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/immediateCheckmate.tcc"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <iostream>
#include <fstream>


using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(ImmediateCheckmateTestPawnDropFoul)
{
  {
    NumEffectState state
      (CsaString(
	"P1 *  *  *  *  *  *  * -KE-OU\n"
	"P2 *  *  *  *  *  *  * -KE * \n"
	"P3 *  *  *  *  *  *  *  *  * \n"
	"P4 *  *  *  *  *  *  * +KE * \n"
	"P5 *  *  *  *  *  *  *  *  * \n"
	"P6 *  *  *  *  *  *  *  *  * \n"
	"P7 *  *  *  *  *  *  *  *  * \n"
	"P8 *  *  *  *  *  *  *  *  * \n"
	"P9 * +OU *  *  *  *  *  *  * \n"
	"P+00FU\n"
	"P-00AL\n"
	"+\n").initialState());
    Move best_move;
    const bool checkmate 
      = ImmediateCheckmate::hasCheckmateMove<BLACK>(state,best_move);
    if (checkmate)
      std::cerr << best_move << "\n";
    BOOST_CHECK(! checkmate);
  }
  {
    NumEffectState state
      (CsaString(
	"P1 *  *  *  *  *  *  * -KE-OU\n"
	"P2 *  *  *  *  *  *  * -KE * \n"
	"P3 *  *  *  *  *  *  *  *  * \n"
	"P4 *  *  *  *  *  *  * +KE * \n"
	"P5 *  *  *  *  *  *  *  *  * \n"
	"P6 *  *  *  *  *  *  *  *  * \n"
	"P7 *  *  *  *  *  *  *  *  * \n"
	"P8 *  *  *  *  *  *  *  *  * \n"
	"P9 * +OU *  *  *  *  *  *  * \n"
	"P+00FU00KY\n"
	"P-00AL\n"
	"+\n").initialState());
    Move best_move;
    const bool checkmate 
      = ImmediateCheckmate::hasCheckmateMove<BLACK>(state,best_move);
    BOOST_CHECK(checkmate);
  }
}

/**
 * 足の短い利きのmoveでcheckmateをかける
 */
BOOST_AUTO_TEST_CASE(ImmediateCheckmateTestMoveShort)
{
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE *  *  * +HI-FU-OU-KY\n"
				 "P2 *  *  *  *  *  *  *  *  * \n"
				 "P3-FU * -FU * -FU-GI+TO *  * \n"
				 "P4 *  *  *  *  *  *  * -FU * \n"
				 "P5 * -FU * +FU-KA+KI *  * -FU\n"
				 "P6 *  * +FU *  *  *  * +FU * \n"
				 "P7+FU+FU *  *  * +KI *  * +FU\n"
				 "P8 *  *  *  *  *  *  * +HI+KY\n"
				 "P9 * +KE *  *  *  * -UM+KE+OU\n"
				 "P-00KI00KI00GI00GI00GI00KE00KY00FU00FU00FU00FU\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    // +0027KE or +5528UM
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
    BOOST_CHECK(Move(Square(2,7),KNIGHT,WHITE) ==  bestMove ||
		   Move(Square(5,5),Square(2,8),PBISHOP,ROOK,true,WHITE) ==  bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  *  *  *  *  * -KE-KY\n"
				 "P2 *  *  *  * +RY *  *  *  * \n"
				 "P3 * +NK * +TO *  * -KI-FU * \n"
				 "P4 * +GI *  *  *  * -FU * -FU\n"
				 "P5-FU *  *  *  *  *  * -RY * \n"
				 "P6 *  *  * +OU-KA-KI *  * -OU\n"
				 "P7+FU-UM *  * +KI *  *  *  * \n"
				 "P8 *  *  *  * +GI *  *  * -KI\n"
				 "P9 *  *  *  *  *  *  *  *  * \n"
				 "P+00GI00GI00FU00FU\n"
				 "P-00KE00KE00KY00KY00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    // -0074KE
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));

    BOOST_CHECK_EQUAL(Move(Square(2,5),Square(6,5),PROOK,PTYPE_EMPTY,false,WHITE),bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1 * +RY * -KI-OU *  * -KE-KY\n"
				 "P2+KA *  *  *  *  * -KI *  * \n"
				 "P3 *  *  * -FU-FU-FU-GI-FU-FU\n"
				 "P4 *  * -FU+KE *  *  *  *  * \n"
				 "P5 * +GI+FU *  *  *  * +FU * \n"
				 "P6 *  *  *  * +FU+FU *  *  * \n"
				 "P7 *  * -RY+FU+OU+GI+FU * +FU\n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9+KY-GI *  *  * +KI * +KE+KY\n"
				 "P+00KA00KI00KY00FU00FU\n"
				 "P-00KE00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +0052KIは普通のpinned
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove == Move(Square(5,2),GOLD,BLACK));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE+RY-KI-OU-KI * -RY * \n"
				 "P2 *  *  *  *  *  *  *  *  * \n"
				 "P3-FU-FU-FU-FU *  *  *  * -FU\n"
				 "P4 *  *  *  * +KE-KI-FU *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  * +FU+FU *  * -GI *  * \n"
				 "P7+FU+FU *  * +GI * +KE * +FU\n"
				 "P8 * +OU+KI+GI * -TO *  *  * \n"
				 "P9+KY+KE *  *  *  *  *  * +KY\n"
				 "P+00KA00KA\n"
				 "P-00GI00KY00FU00FU00FU00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +7162RYはpinned attackになっていない
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE *  * -KI+HI-KI * -OU\n"
				 "P2 *  *  *  *  *  * +UM-GI-HI\n"
				 "P3-FU * -FU+TO+KE *  * -FU * \n"
				 "P4 * -FU *  *  *  *  *  * -KY\n"
				 "P5 *  *  *  * +FU+OU+FU+KI-FU\n"
				 "P6 *  * +FU *  *  *  *  *  * \n"
				 "P7+FU+FU *  *  *  *  * +FU * \n"
				 "P8 *  *  *  * -UM *  *  *  * \n"
				 "P9+KY+KE *  *  *  *  *  *  * \n"
				 "P+00KI00GI00GI00KY00FU00FU00FU00FU00FU\n"
				 "P-00GI00KE00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +0021KIはpinned attackになっていない
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1+NK *  *  * +UM *  * -KE-KY\n"
				 "P2 *  * -GI *  *  * -KI+GI * \n"
				 "P3 *  *  *  *  *  * -KY *  * \n"
				 "P4 *  * -FU+FU * +KI-FU-OU-GI\n"
				 "P5 *  *  *  * -FU *  * -FU-FU\n"
				 "P6 * +FU+FU * +FU * +FU+KE * \n"
				 "P7 *  * +OU *  *  *  *  *  * \n"
				 "P8 *  *  *  * -KA *  *  *  * \n"
				 "P9+KY *  *  *  * +HI *  * +KY\n"
				 "P+00HI00KI00KE00FU00FU00FU00FU00FU\n"
				 "P-00KI00GI00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +4434KI
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove == Move(Square(4,4),Square(3,4),GOLD,PAWN,false,BLACK));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  *  *  * -OU-GI * -KY\n"
				 "P2 *  *  * +NK *  *  *  *  * \n"
				 "P3 *  *  *  * -KI-FU+NK-FU * \n"
				 "P4-FU * -FU *  *  *  * -KI-FU\n"
				 "P5 *  *  * +KE-FU+FU-FU *  * \n"
				 "P6+FU-HI+FU *  *  *  * +FU+FU\n"
				 "P7 *  *  * -KA * +KI *  *  * \n"
				 "P8 *  *  *  *  *  * +GI+OU * \n"
				 "P9+KY *  *  *  * +KI *  * +KY\n"
				 "P+00KA00GI00FU\n"
				 "P-00HI00GI00KE00FU00FU00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +6553KE
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove == Move(Square(6,5),Square(5,3),KNIGHT,GOLD,false,BLACK));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  *  *  * -OU-GI * -KY\n"
				 "P2 *  *  * +NK *  *  *  *  * \n"
				 "P3 *  *  *  * -KI-KI+NK-FU * \n"
				 "P4-FU * -FU *  *  *  * -KI-FU\n"
				 "P5 *  *  * +KE-FU+FU-FU *  * \n"
				 "P6+FU-HI+FU *  *  *  * +FU+FU\n"
				 "P7 *  *  * -KA * -FU *  *  * \n"
				 "P8 *  *  *  *  *  * +GI+OU * \n"
				 "P9+KY *  *  *  * +KI *  * +KY\n"
				 "P+00KA00GI00FU\n"
				 "P-00HI00GI00KE00FU00FU00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +6553KE is captured by 4353KI
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  *  *  * -OU-GI * -KY\n"
				 "P2 *  *  * +NK *  *  *  *  * \n"
				 "P3 *  *  *  * -KI-KI+NK-FU * \n"
				 "P4-FU * -FU *  * +HI * -KI-FU\n"
				 "P5 *  *  * +KE-FU+FU-FU *  * \n"
				 "P6+FU-HI+FU *  *  *  * +FU+FU\n"
				 "P7 *  *  * -KA * -FU *  *  * \n"
				 "P8 *  *  *  *  *  * +GI+OU * \n"
				 "P9+KY *  *  *  * +KI *  * +KY\n"
				 "P+00KA00GI00FU\n"
				 "P-00GI00KE00FU00FU00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +6553KE is valid because 43KI is pinned
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove == Move(Square(6,5),Square(5,3),KNIGHT,GOLD,false,BLACK));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE * -FU *  *  *  * -KY\n"
				 "P2 *  *  *  *  *  * -GI *  * \n"
				 "P3-FU-FU-FU *  *  * -GI * -FU\n"
				 "P4 *  *  *  * -OU *  *  *  * \n"
				 "P5 *  * +KE * -KA * +KI *  * \n"
				 "P6 *  *  * +RY+KE-RY-FU *  * \n"
				 "P7+FU+FU+OU *  *  * +FU * +FU\n"
				 "P8 *  * +KI *  * -TO *  * +KA\n"
				 "P9+KY *  *  *  *  *  * +KE+KY\n"
				 "P+00KI00GI00FU00FU00FU00FU\n"
				 "P-00KI00GI00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +6663RYは自殺手
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  * -KE *  *  *  * +RY\n"
				 "P2 *  * -OU *  *  *  *  *  * \n"
				 "P3 *  *  * -GI-KI * -KE-FU * \n"
				 "P4-FU-FU-FU-FU *  *  *  * -FU\n"
				 "P5-KE *  *  *  * -FU *  *  * \n"
				 "P6+GI * +FU+KE+FU * +FU *  * \n"
				 "P7+FU+FU+OU * +KI *  *  * +FU\n"
				 "P8 *  *  *  *  *  *  *  * -RY\n"
				 "P9+KY * -KA *  *  *  *  * +KY\n"
				 "P+00KI00GI00GI00KY00FU00FU00FU00FU\n"
				 "P-00KA00KI00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // -7968UMは88の利きを消す
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE+KA+KA *  * -OU-KE-KY\n"
				 "P2 *  *  *  *  * -GI-KI *  * \n"
				 "P3-FU * -FU-FU *  *  *  *  * \n"
				 "P4 * -HI *  *  *  *  * -FU-FU\n"
				 "P5 * -FU+FU+FU+KI+GI-FU *  * \n"
				 "P6 *  *  *  *  *  *  *  * +FU\n"
				 "P7+FU+FU *  *  * +FU-RY+FU+OU\n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9+KY *  *  *  *  *  * -NK+KY\n"
				 "P+00KI00GI00FU00FU\n"
				 "P-00KI00GI00KE00FU00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // -3728RYは26の利きを消す
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE *  *  *  *  *  * -KY\n"
				 "P2 *  *  *  *  * +GI * -OU * \n"
				 "P3-FU *  * -FU * -GI-KE *  * \n"
				 "P4 *  *  *  *  *  * -FU *  * \n"
				 "P5 * -FU+FU-RY * -FU *  * -FU\n"
				 "P6 *  *  *  *  *  * +KI+FU * \n"
				 "P7+FU+FU *  *  *  * +KE+GI+FU\n"
				 "P8+KY *  *  *  * +KI *  * +KY\n"
				 "P9 * +KE-HI *  *  * -KI * +OU\n"
				 "P+00KA00KI\n"
				 "P-00KA00GI00FU00FU00FU00FU00FU00FU00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // 
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
    // open moveでも詰みだが，見つけられるのは寄る手
    BOOST_CHECK(bestMove == Move(Square(3,9),Square(2,9),GOLD,PTYPE_EMPTY,false,WHITE));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE * +RY *  *  *  * -KY\n"
				 "P2 *  *  *  *  *  * -GI *  * \n"
				 "P3-FU-FU-FU *  *  * -GI * -FU\n"
				 "P4 *  *  *  * -UM *  *  *  * \n"
				 "P5 *  *  *  * -OU * -FU *  * \n"
				 "P6 *  *  * -FU+KE-RY *  *  * \n"
				 "P7+FU+FU+OU *  *  * +FU * +FU\n"
				 "P8 *  * +KI+KI * -TO *  * +KA\n"
				 "P9+KY+KE *  *  *  *  * +KE+KY\n"
				 "P+00KI00GI00FU00FU00FU00FU\n"
				 "P-00KI00GI00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    // +7766OU などという王手をしてはいけない
    BOOST_CHECK(bestMove != Move(Square(7,7),Square(6,6),KING,PAWN,false,BLACK));
  }
  {
    SimpleState sstate=CsaString(
				 "P1+NK *  *  * +UM *  * -KE-KY\n"
				 "P2 *  * -GI *  *  * -KI+GI * \n"
				 "P3 *  *  *  *  *  * -KY * +HI\n"
				 "P4 *  * -FU+FU *  * -FU-OU-GI\n"
				 "P5 *  *  *  * -FU *  * -FU-FU\n"
				 "P6 * +FU+FU * +FU * +FU+KE * \n"
				 "P7 *  * +OU *  *  *  *  *  * \n"
				 "P8 *  *  *  * -KA *  *  *  * \n"
				 "P9+KY *  *  *  * +HI *  * +KY\n"
				 "P+00KI00KI00KE00FU00FU00FU00FU00FU\n"
				 "P-00KI00GI00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // +2614KEは王手になっていない
    Move bestMove;
    // 
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1+NK *  *  * +UM+RY * -KE-KY\n"
				 "P2 *  * -GI *  *  * -KI+GI-OU\n"
				 "P3 *  *  *  *  *  * -KY *  * \n"
				 "P4 *  * -FU+FU *  * -FU * -GI\n"
				 "P5 *  *  *  * -FU *  * -FU-FU\n"
				 "P6 * +FU+FU * +FU * +FU+KE * \n"
				 "P7 *  * +OU *  *  *  *  *  * \n"
				 "P8 *  *  *  * -KA *  *  *  * \n"
				 "P9+KY *  *  *  *  *  *  * +KY\n"
				 "P+00HI00KI00KI00KE00FU00FU00FU00FU00FU\n"
				 "P-00KI00GI00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // +4121RY は22に駒が無くて23に利きが通っていれば詰み
    Move bestMove;
    // 
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1+NK *  *  * +UM *  * -KE-KY\n"
				 "P2 *  * -GI *  *  * -KI *  * \n"
				 "P3 *  *  *  *  *  * -KY * +NG\n"
				 "P4 *  * -FU+FU *  * -FU-OU-GI\n"
				 "P5 *  *  *  * -FU *  * -FU-FU\n"
				 "P6 * +FU+FU * +FU * +FU+KE * \n"
				 "P7 *  * +OU *  *  *  *  *  * \n"
				 "P8 *  *  *  * -KA *  *  *  * \n"
				 "P9+KY *  *  *  * +HI *  * +KY\n"
				 "P+00HI00KI00KI00KE00FU00FU00FU00FU00FU\n"
				 "P-00KI00GI00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // +1314NG で元々14に利きがなかったので，移動後も利きがないものと思ってしまっている．
    Move bestMove;
    // 
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }

  {
    SimpleState sstate=CsaString(
				 "P1 *  *  * -KI *  *  *  * -KY\n"
				 "P2 * +KI-GI *  *  *  *  *  * \n"
				 "P3+GI-FU-FU *  *  *  *  * -FU\n"
				 "P4-FU * -KE *  * -FU-FU-FU * \n"
				 "P5 *  * +KA *  *  *  *  *  * \n"
				 "P6 *  *  *  * -FU *  * +HI * \n"
				 "P7 * +FU+KE+FU * +KA-OU * +FU\n"
				 "P8 *  * +OU+GI *  *  *  *  * \n"
				 "P9 *  * +KI * -NG+KY *  * +KY\n"
				 "P+00KE00FU\n"
				 "P-00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // 
    Move bestMove;
    // +7548KA で47の利きをBLOCKすることを忘れていると詰みだと思ってしまう．
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }

  {
    SimpleState sstate=CsaString(
				 "P1-KY *  *  *  *  *  *  * -KY\n"
				 "P2-OU+GI-GI * -KI *  *  *  * \n"
				 "P3-KE-FU *  *  *  * +TO * -FU\n"
				 "P4+FU * -FU-GI *  *  *  *  * \n"
				 "P5 *  *  *  *  *  * -KA *  * \n"
				 "P6 *  * +FU+FU-FU *  *  *  * \n"
				 "P7+KE+FU+KA+KI *  *  *  * +FU\n"
				 "P8 *  * +OU *  * +KI *  *  * \n"
				 "P9+KY *  *  * +FU *  * +KE+KY\n"
				 "P+00HI00GI00KE00FU00FU00FU00FU00FU\n"
				 "P-00HI00KI00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // 
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove == Move(Square(9,4),Square(9,3),PPAWN,KNIGHT,true,BLACK));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY+UM+RY+HI *  * -KI * -KY\n"
				 "P2 *  *  *  *  *  * -KI *  * \n"
				 "P3 *  *  * -FU * -KI-KE * -OU\n"
				 "P4-FU *  *  *  *  * +FU+KY-FU\n"
				 "P5 * -FU * +FU * -FU *  *  * \n"
				 "P6+FU *  *  * +FU-KI * +FU+FU\n"
				 "P7 * +FU *  *  * +FU * +OU * \n"
				 "P8 *  * +FU *  *  *  *  *  * \n"
				 "P9 *  *  *  *  * -NG-GI+KE-UM\n"
				 "P+00GI00GI00KE00KY00FU00FU00FU00FU\n"
				 "P-00KE\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // 
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
    BOOST_CHECK_EQUAL(Move(Square(1,9),Square(2,8),PBISHOP,PTYPE_EMPTY,false,WHITE),bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1 * +TO *  * -HI *  * -KE-KY\n"
				 "P2 *  *  *  *  *  * -KI *  * \n"
				 "P3 *  *  * -GI * -KI * -FU * \n"
				 "P4-OU+KY *  * -FU-GI *  * -FU\n"
				 "P5-KA * +FU *  * -FU-FU+KE * \n"
				 "P6 *  * -FU * +FU *  * +HI+FU\n"
				 "P7-TO *  * +FU+KA *  *  *  * \n"
				 "P8 * -GI+OU * +KI+GI *  *  * \n"
				 "P9 * -NY * +KI *  *  *  *  * \n"
				 "P+00FU00FU00FU00FU\n"
				 "P-00KE00KE00KY00FU00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // 
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
    BOOST_CHECK(bestMove == Move(Square(7,6),Square(7,7),PPAWN,PTYPE_EMPTY,true,WHITE) ||
		   bestMove == Move(Square(9,5),Square(7,7),PBISHOP,PTYPE_EMPTY,true,WHITE));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 *  *  *  * -KI *  *  * -KY\n"
				 "P2 * +NG+TO *  * -KI-OU *  * \n"
				 "P3 *  *  * -FU-FU-FU-KE+FU-FU\n"
				 "P4-FU *  *  * -KE * -KY-GI * \n"
				 "P5 *  *  * +UM *  *  *  *  * \n"
				 "P6+FU *  *  * +HI *  *  *  * \n"
				 "P7 * +FU * +FU+FU+FU+FU * +FU\n"
				 "P8 *  *  *  * +HI+OU * -KA * \n"
				 "P9+KY+KE-TO *  * +KI * +GI+KY\n"
				 "P-00KI00GI00KE00FU00FU00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    // 
    Move bestMove;
    // -2837UM で39の利きが消えることを忘れていると詰みだと思ってしまう．
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
  }
}
/**
 * 本当は一手詰みなのにまだ扱えないパターン
 */
BOOST_AUTO_TEST_CASE(ImmediateCheckmateTestChallenge)
{
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE * +RY *  *  *  * -KY\n"
				 "P2 *  *  *  *  *  * -GI *  * \n"
				 "P3-FU-FU-FU *  *  * -GI * -FU\n"
				 "P4 *  *  * +KI *  *  *  *  * \n"
				 "P5 *  *  *  * -OU * -FU *  * \n"
				 "P6 *  *  * -FU-UM-RY *  *  * \n"
				 "P7+FU+FU+OU *  *  * +FU * +FU\n"
				 "P8 *  * +KI+KI * -TO *  * +KA\n"
				 "P9+KY+KE *  *  *  *  * +KE+KY\n"
				 "P+00GI00KE00FU00FU00FU00FU\n"
				 "P-00KI00GI00FU00FU00FU\n"
				 "+\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +6454KI
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  *  *  *  * -KA * -KY\n"
				 "P2 *  * -KI *  * -KE+NG *  * \n"
				 "P3 *  *  * -FU-OU-FU-KE-FU * \n"
				 "P4-FU * -FU *  *  *  *  * -FU\n"
				 "P5 *  * +FU * -FU * -FU *  * \n"
				 "P6 *  *  * +HI * +KE *  *  * \n"
				 "P7+FU+FU * +FU+GI+FU+FU * +FU\n"
				 "P8 *  * -NG * +KI *  *  *  * \n"
				 "P9+KY *  *  * +OU * +KI-RY+KY\n"
				 "P+00FU\n"
				 "P-00KA00KI00GI00KE00FU00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // +2939RYは合駒なし
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE * +GI *  *  *  * -KY\n"
				 "P2 *  * +RY *  *  *  *  *  * \n"
				 "P3-OU-FU-FU * -KI *  *  *  * \n"
				 "P4-FU *  * -FU *  *  * -KA-FU\n"
				 "P5 * +FU *  * +FU+UM+KI-FU * \n"
				 "P6+FU * -RY * -GI+OU-FU *  * \n"
				 "P7 *  *  *  *  *  * -NG * +FU\n"
				 "P8 *  *  *  * +GI * +FU * +KY\n"
				 "P9+KY *  *  *  *  *  *  *  * \n"
				 "P+00KI00KE00FU\n"
				 "P-00KI00KE00KE00FU00FU00FU00FU00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // 05647NGはopen attack
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  * -KI+TO *  *  *  * \n"
				 "P2 *  *  *  *  * -FU *  *  * \n"
				 "P3 *  *  *  *  *  * +RY * -KY\n"
				 "P4 *  * -OU-FU * +FU *  * -FU\n"
				 "P5-FU * -FU *  *  *  *  *  * \n"
				 "P6 *  *  * +FU *  *  *  *  * \n"
				 "P7+FU-TO * +KI *  * +FU * +FU\n"
				 "P8 * +GI+KI *  *  *  *  *  * \n"
				 "P9+KY+OU+GI-GI *  *  * -RY+KY\n"
				 "P+00KI00KE00KE00FU\n"
				 "P-00KA00KA00GI00KE00KE00FU00FU00FU00FU00FU\n"
				 "-\n"
				 ).initialState(); 
    NumEffectState state(sstate);
    Move bestMove;
    // -6978GIはpinnedだが初期状態ではdetectできない
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
  }
}
static int state0Count;
static int state1Count;
static int stateCount;
static int immediateCount;
static int immediateDrop;
static int answerCount;
static int answerMoveCount;

static bool noEscape(const NumEffectState &state)
{
  Player pl=state.turn();
  // 自殺手
  if(state.hasEffectAt(pl,state.kingSquare(alt(pl)))) return false;
  // not 王手
  if(!state.hasEffectAt(alt(pl),state.kingSquare(pl))) return false;
  MoveVector moves;
  state.generateAllUnsafe(moves);

  for(Move move: moves){
    NumEffectState next_state = state;
    next_state.makeMove(move);
    Player pl=next_state.turn();
    // 王手をのがれた
    if(!next_state.hasEffectAt(pl,next_state.kingSquare(alt(pl)))) return false;
  }
  return true;
}

template<bool onlyDrop>
static bool isImmediateCheck(const NumEffectState& state_org,Move const& move)
{
  if(onlyDrop && move.from().isOnBoard()) return false;
  if(!move.from().isOnBoard() && move.ptype()==PAWN) return false;
  NumEffectState state = state_org;
  state.makeMove(move);
  if(noEscape(state))
    return true;
  return false;
}

template<bool onlyDrop>
static bool hasImmediateCheck(NumEffectState& state,Move& bestMove)
{
  MoveVector moves;
  state.generateAllUnsafe(moves);

  for(Move move: moves){
    if(isImmediateCheck<onlyDrop>(state,move)) {
      bestMove=move;
      return true;
    }
  }
  return false;
}
/**
 * 打った駒が足の短い利きでcheckmateをかける
 */
BOOST_AUTO_TEST_CASE(ImmediateCheckmateTestDropShort)
{
  {
    NumEffectState state(CsaString(
      "P1 *  *  * -KI *  *  *  * -KY\n"
      "P2 * +KI-GI *  *  *  *  *  * \n"
      "P3+GI-FU-FU *  *  *  *  * -FU\n"
      "P4-FU * -KE *  * -FU-FU-FU * \n"
      "P5 *  * +KA *  *  *  * +HI * \n"
      "P6 *  *  *  * -FU-KY *  *  * \n"
      "P7 * +FU+KE+FU * +KA-OU * +FU\n"
      "P8 *  * +OU+GI-NG *  *  *  * \n"
      "P9 *  * +KI *  * +KY *  * +KY\n"
      "P+00KE00FU\n"
      "P-00HI00KI00KE00FU00FU00FU00FU00FU00FU\n"
      "+\n"
      ).initialState());
    // 
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove==Move(Square(2,9),KNIGHT,BLACK));
  }
  {
    NumEffectState state(CsaString(
				 "P1-KY-KE * +GI *  *  *  * -KY\n"
				 "P2 *  * +RY *  *  *  *  *  * \n"
				 "P3-OU * -FU *  *  *  *  *  * \n"
				 "P4-FU-KE * -FU *  *  * -KA-FU\n"
				 "P5 *  *  *  * +FU * +KI-FU * \n"
				 "P6+FU * +KE * -GI+OU *  *  * \n"
				 "P7 * +HI * +GI *  * -TO * +FU\n"
				 "P8 *  *  *  *  * -GI+FU * +KY\n"
				 "P9+KY *  *  *  *  *  *  *  * \n"
				 "P+00KA00KE00FU00FU00FU\n"
				 "P-00KI00KI00KI00FU00FU00FU00FU00FU\n"
				 "+\n"
			 ).initialState());
    // 
    Move bestMove;
    BOOST_CHECK(isImmediateCheck<true>(state,Move(Square(8,5),KNIGHT,BLACK)));
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove==Move(Square(8,5),KNIGHT,BLACK));
  }
  {
    NumEffectState state(CsaString(
				 "P1-KY-KE * +GI *  *  *  * -KY\n"
				 "P2 *  * +RY *  *  *  *  *  * \n"
				 "P3-OU * -FU * -KI *  *  *  * \n"
				 "P4-FU-FU * -FU *  *  * -KA-FU\n"
				 "P5 *  * +KA * +FU * +KI-FU * \n"
				 "P6+FU+FU *  * -GI+OU *  *  * \n"
				 "P7 *  * -RY+GI *  * -TO * +FU\n"
				 "P8 *  *  *  *  * -GI+FU * +KY\n"
				 "P9+KY *  *  *  *  *  *  *  * \n"
				 "P+00KE00FU\n"
				 "P-00KI00KI00KE00KE00FU00FU00FU00FU00FU\n"
				 "+\n"
			 ).initialState());
    // 
    Move bestMove;
    BOOST_CHECK(isImmediateCheck<true>(state,Move(Square(8,5),KNIGHT,BLACK)));
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK(bestMove==Move(Square(8,5),KNIGHT,BLACK));
  }
  {
    NumEffectState state(CsaString(
				 "P1-KY-KE * +GI *  *  *  * -KY\n"
				 "P2 *  * +RY *  *  *  *  *  * \n"
				 "P3-OU-FU-FU * -KI *  *  *  * \n"
				 "P4-FU *  * -FU *  * +UM-KA-FU\n"
				 "P5 * +FU *  * +FU * +KI-FU * \n"
				 "P6+FU * -RY * -GI+OU *  *  * \n"
				 "P7 *  *  * +GI *  * -TO * +FU\n"
				 "P8 *  *  *  *  * -GI+FU * +KY\n"
				 "P9+KY *  *  *  *  *  *  *  * \n"
				 "P+00KI00KE00FU\n"
				 "P-00KI00KE00KE00FU00FU00FU00FU00FU\n"
				 "-\n"
			 ).initialState());
    // 
    Move bestMove;
    BOOST_CHECK(isImmediateCheck<true>(state,Move(Square(3,6),GOLD,WHITE)));
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 * +TO *  * -HI *  * -KE-KY\n"
				 "P2 *  *  *  *  *  * -KI *  * \n"
				 "P3 *  *  * -GI * -KI * -FU * \n"
				 "P4-OU+KY *  * -FU-GI *  * -FU\n"
				 "P5-KA * -FU *  * -FU-FU+KE * \n"
				 "P6 *  * +FU+KA+FU *  * +HI+FU\n"
				 "P7-TO *  * +FU *  *  *  *  * \n"
				 "P8 * -GI+OU * +KI+GI *  *  * \n"
				 "P9 * -NY * +KI *  *  *  *  * \n"
				 "P+00FU00FU00FU00FU\n"
				 "P-00KE00KE00KY00FU00FU\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 
    Move bestMove;
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove) ||
		   (std::cerr << bestMove << std::endl,0)
		   );
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY *  *  *  *  *  *  * -KY\n"
				 "P2 * -HI *  * +KI * -OU *  * \n"
				 "P3-FU * -FU-FU *  *  * -FU * \n"
				 "P4 *  *  *  *  * +UM-FU *  * \n"
				 "P5 *  *  *  *  *  *  *  * -FU\n"
				 "P6+FU * +FU *  * +KE+FU *  * \n"
				 "P7 * +FU *  *  * +FU+KE+FU * \n"
				 "P8 *  * +KI *  *  * +GI+OU * \n"
				 "P9+KY+KE *  *  * +KI *  * +KY\n"
				 "P+00KI00FU00FU\n"
				 "P-00HI00KA00GI00GI00GI00KE00FU00FU00FU00FU\n"
				 "+\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 22KI
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK_EQUAL(Move(Square(2,2),GOLD,BLACK),bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-KE *  *  *  *  *  * -KY\n"
				 "P2 *  * -KI *  *  *  *  *  * \n"
				 "P3 * -FU-GI * -FU-FU-OU *  * \n"
				 "P4-FU *  * -FU *  * -FU-FU * \n"
				 "P5 *  *  *  *  * +GI *  * -FU\n"
				 "P6+FU * +KI+FU+KA *  * -UM * \n"
				 "P7 *  * +KE * +FU+FU+FU * +FU\n"
				 "P8 * -RY+GI *  * -HI *  *  * \n"
				 "P9+KY *  * +KI+OU *  *  * +KY\n"
				 "P-00KI00GI00KE00KE00FU00FU00FU00FU\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    // -0049KI!!
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
    BOOST_CHECK_EQUAL(Move(Square(4,9),GOLD,WHITE),bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY-GI * +TO *  *  *  *  * \n"
				 "P2-FU-OU-KI * -GI *  *  *  * \n"
				 "P3 *  *  *  *  *  *  *  * -FU\n"
				 "P4+FU-FU * +GI *  *  *  *  * \n"
				 "P5 *  * -FU+KA *  * +FU *  * \n"
				 "P6+OU *  *  *  * +FU *  *  * \n"
				 "P7 * +FU *  *  * -RY-FU *  * \n"
				 "P8 * +UM *  * -TO *  *  *  * \n"
				 "P9+KY *  * +KI+KY-RY *  *  * \n"
				 "P+00KI00KI00GI00KE00KY00FU00FU00FU00FU00FU\n"
				 "P-00KE00KE00KE00FU00FU\n"
				 "+\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 74KEは角をふさぐ
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY+RY *  *  *  *  * -KE-KY\n"
				 "P2 *  * +TO *  * -KI-OU-KE * \n"
				 "P3 *  *  *  *  * -KI-GI-FU * \n"
				 "P4 *  * +GI * -FU-FU *  * -FU\n"
				 "P5-FU *  * +FU *  *  *  *  * \n"
				 "P6 *  *  *  * +FU+FU *  * +FU\n"
				 "P7+FU-TO *  *  * +KI+GI+FU * \n"
				 "P8 *  *  * -UM-TO+KI+OU *  * \n"
				 "P9+KY-UM *  *  *  *  * +KE+KY\n"
				 "P+00HI00KE00FU00FU00FU\n"
				 "P-00GI00FU\n"
				 "+\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 31HI
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK_EQUAL(Move(Square(3,1),ROOK,BLACK),bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY+HI *  *  * -KI * -KE * \n"
				 "P2 *  *  *  *  * -GI-OU *  * \n"
				 "P3-FU *  * +TO-FU-GI * -FU * \n"
				 "P4 *  *  *  *  * -FU-FU *  * \n"
				 "P5 *  * -TO * +KI *  *  *  * \n"
				 "P6 *  *  *  *  *  * -RY *  * \n"
				 "P7 * +FU * -KY * +FU+FU-NK * \n"
				 "P8 *  *  *  * +FU+GI-KI *  * \n"
				 "P9 *  *  * +KA+OU * +KI *  * \n"
				 "P+00GI00KY00FU00FU00FU00FU00FU\n"
				 "P-00KA00KE00KE00KY00FU00FU\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 68KA
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state,bestMove));
    BOOST_CHECK_EQUAL(Move(Square(6,8),BISHOP,WHITE),bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY+NG *  *  *  *  *  * +UM\n"
				 "P2 *  *  *  *  *  *  *  * -KY\n"
				 "P3 * -OU-FU *  *  *  *  *  * \n"
				 "P4-FU * -GI * -KI *  * -FU-FU\n"
				 "P5 * +KE * -FU *  *  * +KE * \n"
				 "P6+FU+OU *  * +FU+FU *  * +FU\n"
				 "P7 *  *  *  * +GI *  *  *  * \n"
				 "P8 *  *  *  *  *  *  * +HI * \n"
				 "P9+KY *  * -UM *  *  *  * +KY\n"
				 "P+00HI00KI00KI00GI00KE00FU00FU00FU00FU00FU00FU00FU\n"
				 "P-00KI00KE00FU00FU\n"
				 "+\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 82HI
    Move bestMove;
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state,bestMove));
    BOOST_CHECK_EQUAL(Move(Square(8,2),ROOK,BLACK),bestMove);
  }
  {
    SimpleState sstate=CsaString(
				 "P1-KY+UM+RY+HI *  * -KI * -KY\n"
				 "P2 *  *  *  *  *  * -KI *  * \n"
				 "P3 *  *  * -FU * -KI-KE * -OU\n"
				 "P4-FU *  *  *  *  * +FU+KY-FU\n"
				 "P5 * -FU * +FU * -FU *  *  * \n"
				 "P6+FU *  *  * +FU *  * +FU+FU\n"
				 "P7 * +FU *  *  * +FU+OU *  * \n"
				 "P8 *  * +FU *  *  *  *  * -UM\n"
				 "P9 *  *  *  *  * -NG-GI+KE * \n"
				 "P+00GI00GI00KE00KY00FU00FU00FU00FU\n"
				 "P-00KI00KE\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 27金は46への馬の利きをふさぐ
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state));
  }

  {
    SimpleState sstate=CsaString(
				 "P1-KY+HI *  *  * -KI * -KE * \n"
				 "P2 *  * +TO *  * -GI-OU *  * \n"
				 "P3-FU *  *  * -FU-GI * -FU * \n"
				 "P4 *  *  *  *  * -FU-FU *  * \n"
				 "P5 *  * -TO * +KI *  *  *  * \n"
				 "P6 *  *  * -RY *  *  *  *  * \n"
				 "P7 * +FU *  *  * +FU+FU-NK * \n"
				 "P8 *  *  *  * +FU+GI-KI *  * \n"
				 "P9 *  *  *  * +OU * +KI *  * \n"
				 "P+00KA00GI00KY00FU00FU00FU00FU00FU\n"
				 "P-00KA00KE00KE00KY00KY00FU00FU\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 67桂は68,69への龍の利きをふさぐ
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<WHITE>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 * +NY *  *  *  *  * -KE-KY\n"
				 "P2 *  *  *  * -FU-KI-OU *  * \n"
				 "P3 * +HI *  *  *  *  * -FU-FU\n"
				 "P4 *  *  * -FU+FU-FU-FU *  * \n"
				 "P5+HI+FU+FU * -KE *  *  *  * \n"
				 "P6 *  * -KY+GI *  *  *  *  * \n"
				 "P7-GI+OU *  *  * +FU+FU+FU+FU\n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 *  *  *  *  *  *  * +KE+KY\n"
				 "P+00KA00KA00KI00KI00GI00GI00KE00FU00FU00FU00FU00FU\n"
				 "P-00KI\n"
				 "-\n"
				 ).initialState();
    NumEffectState state(sstate);
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<WHITE>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 *  *  *  *  *  *  * -OU * \n"
				 "P2 *  *  *  *  *  *  *  *  * \n"
				 "P3 *  *  *  *  *  *  * +TO * \n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7 *  *  *  *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 *  * +OU *  *  *  *  *  * \n"
				 "P+00KI\n"
				 "P-00AL\n"
				 "+\n").initialState();
    NumEffectState state(sstate);
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 *  *  *  *  *  * -FU-OU-FU\n"
				 "P2 *  *  *  *  *  *  *  *  * \n"
				 "P3 *  *  *  *  *  *  * +TO-KE\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7 *  *  *  *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 *  * +OU *  *  *  *  *  * \n"
				 "P+00KE\n"
				 "P-00AL\n"
				 "+\n").initialState();
    NumEffectState state(sstate);
    BOOST_CHECK(ImmediateCheckmate::hasCheckmateMove<BLACK>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 *  *  *  *  *  *  *  *  * \n"
				 "P2 *  *  *  *  *  *  * -OU * \n"
				 "P3 *  *  *  *  *  *  *  *  * \n"
				 "P4 *  *  *  *  *  *  * +TO * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7 *  *  *  *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 *  * +OU *  *  *  *  *  * \n"
				 "P+00KI\n"
				 "P-00AL\n"
				 "+\n").initialState();
    NumEffectState state(sstate);
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 *  *  *  *  *  *  * -OU-FU\n"
				 "P2 *  *  *  *  *  *  *  *  * \n"
				 "P3 *  *  *  *  *  *  * +TO-KE\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7 *  *  *  *  *  *  *  *  * \n"
				 "P8 *  *  *  *  *  *  *  *  * \n"
				 "P9 *  * +OU *  *  *  *  *  * \n"
				 "P+00KE\n"
				 "P-00AL\n"
				 "+\n").initialState();
    NumEffectState state(sstate);
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1 *  *  * +UM *  *  *  *  * \n"
				 "P2+GI *  * -KE-KY *  *  * -UM\n"
				 "P3-FU+TO * -OU-FU+GI-KE *  * \n"
				 "P4 *  * -FU *  *  *  * -FU * \n"
				 "P5 *  *  *  * +KY *  * -RY * \n"
				 "P6 * -HI * -FU *  *  *  *  * \n"
				 "P7+FU * +FU-KI * +FU *  * -NK\n"
				 "P8 *  *  *  *  * +GI *  *  * \n"
				 "P9+KY+KE+KI+OU+KI *  *  *  * \n"
				 "P+00KI00GI00KY00FU00FU00FU00FU00FU00FU\n"
				 "P-00FU00FU00FU\n"
				 "+\n"
				 ).initialState();
    NumEffectState state(sstate);
    // 65KYは離れたところからのdropなのでまだ打てない．
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state));
  }
  {
    SimpleState sstate=CsaString(
				 "P1+RY-KY *  * -OU * -FU+UM-KY\n"
				 "P2 * -KI *  * -KI * -KI *  * \n"
				 "P3 * +KE-FU-KA-FU-FU *  *  * \n"
				 "P4+OU *  *  * +KE *  * -FU-FU\n"
				 "P5-FU * +FU-GI *  *  *  *  * \n"
				 "P6 * +GI * -FU * +GI * +KI+FU\n"
				 "P7+FU *  * +FU+FU+FU *  *  * \n"
				 "P8 *  * -NK *  *  *  *  *  * \n"
				 "P9+KY+KE *  *  *  * +FU * +KY\n"
				 "P+00HI00GI\n"
				 "P-00FU00FU00FU\n"
				 "+\n"
				 ).initialState();
    NumEffectState state(sstate);
    // +0071HIは離れたところからのdropなので打てない．
    BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove<BLACK>(state));
  }
}

void ImmediateCheckmateTestCheckmate()
{
  {
    // no checkmate
    NumEffectState state
      (CsaString("P1-KI *  *  * +TO+TO+TO+TO+NY\n"
		 "P2 * -FU+NK *  *  *  *  *  * \n"
		 "P3+RY-TO * -OU-FU-FU * -FU-KY\n"
		 "P4 *  *  * -GI-KI-TO *  * +RY\n"
		 "P5-UM-NK+KY+TO * -TO *  *  * \n"
		 "P6 * +FU *  *  * -GI-GI+FU * \n"
		 "P7 *  * +FU * +TO-GI *  * +KE\n"
		 "P8 *  *  *  * +TO * +FU * +KY\n"
		 "P9+UM *  *  *  *  * +KI * -KI\n"
		 "P-00KE\n"
		 "+\n"
	).initialState());
    Move check_move;
    const bool is_checkmate
      = ImmediateCheckmate::hasCheckmateMove<BLACK>(state, check_move);
    // +6574TOは -5372OUで後1000手くらい詰まない
    BOOST_CHECK(check_move != Move(Square(6,5),Square(7,4),PPAWN,PTYPE_EMPTY,false,BLACK));
    BOOST_CHECK(! is_checkmate);
  }
}

static void level1(const NumEffectState &state_org)
{
  state0Count++;
  MoveVector moves;
  state_org.generateAllUnsafe(moves);
  for(size_t i=0;i<moves.size();i++){
    NumEffectState state = state_org;
    state.makeMove(moves[i]);
    Player pl=state.turn();
    state1Count++;
    if(!state.hasEffectAt(pl,state.kingSquare(alt(pl))) &&
       !state.hasEffectAt(alt(pl),state.kingSquare(pl))
       ){
      stateCount++;
      Move bestMove1;
      if(hasImmediateCheck<false>(state,bestMove1)){
	if((stateCount%100)==0){
	  FixedDepthSearcher solver(state);
	  Move best_move;
	  ProofDisproof pdp = solver.hasCheckmateMoveOfTurn(2, best_move);
	  BOOST_CHECK(pdp.isCheckmateSuccess());
	}
	immediateCount++;
	if(hasImmediateCheck<true>(state,bestMove1)){
	  immediateDrop++;
	  Move bestMove;
	  if(!ImmediateCheckmate::hasCheckmateMove(state.turn(),state,bestMove)){
	    if(OslConfig::verbose())
	      std::cerr << std::endl << bestMove1 << std::endl << 
		state << std::endl;
	  }
	  else{
	    BOOST_CHECK(isImmediateCheck<false>(state,bestMove) ||
			   (std::cerr << state << std::endl << bestMove << std::endl,0)
			   );
	    answerCount++;
	  }
	}
	else{
	  Move bestMove;
	  if(!ImmediateCheckmate::hasCheckmateMove(state.turn(),state,bestMove)){
	    if(OslConfig::verbose())
	      std::cerr << std::endl << bestMove1 << std::endl << 
		state << std::endl;
	  }
	  else{
	    BOOST_CHECK(isImmediateCheck<false>(state,bestMove) ||
			   (std::cerr << std::endl << bestMove << std::endl << state << std::endl,0)
			   );
	    answerMoveCount++;
	  }
	}
      }
      else{
	Move bestMove;
	BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove(state.turn(),state,bestMove) ||
		       (std::cerr << std::endl << bestMove << std::endl << state << std::endl,0)
		       );
	BOOST_CHECK(!ImmediateCheckmate::hasCheckmateMove(state.turn(),state,bestMove) ||
		       (std::cerr << std::endl << bestMove << std::endl << state << std::endl,0)
		       );
      }
    }
  }
}
static void testFile(std::string filename)
{
  filename = OslConfig::testPublicFile(filename);  
  NumEffectState state(CsaFileMinimal(filename).load().initial_state);
  // 2手進めたところで，チェックする
  MoveVector moves;
  state.generateAllUnsafe(moves);
  for(size_t i=0;i<moves.size();i++){
    NumEffectState new_state = state;
    new_state.makeMove(moves[i]);
    Player pl=new_state.turn();
    if(!new_state.hasEffectAt(pl,new_state.kingSquare(alt(pl)))){
      level1(new_state);
    }
  }
}
BOOST_AUTO_TEST_CASE(ImmediateCheckmateTestFiles)
{
  std::ifstream ifs(OslConfig::testPublicFile("short-checkmate-problems/FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=10;
  if (OslConfig::inUnitTestShort())
    count=2;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && (++i<count)) {
    if (progress)
      ++(*progress);
    testFile("short-checkmate-problems/" + filename);
  }
  if(OslConfig::verbose())
    std::cerr << std::endl << "stateCount=" << stateCount <<
      ",state0Count=" << state0Count <<
      ",state1Count=" << state1Count <<
      ",immediateCount=" << immediateCount <<
      ",immediateDrop=" << immediateDrop << ",answerCount=" << answerCount <<
      ",answerMoveCount=" << answerMoveCount <<
      std::endl;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

