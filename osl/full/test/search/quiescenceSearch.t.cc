/* quiescenceSearch.t.cc
 */
// #define QSEARCH_DEBUG
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/simpleHashTable.h"
#include "osl/game_playing/historyToTable.h"
#include "osl/game_playing/gameState.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/misc/milliSeconds.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::search;
using namespace osl::game_playing;
using namespace osl::eval;

const int Gold = PtypeEvalTraits<GOLD>::val;
const int Silver = PtypeEvalTraits<SILVER>::val;
const int Psilver = PtypeEvalTraits<PSILVER>::val;
const int Rook = PtypeEvalTraits<ROOK>::val;
const int Knight = PtypeEvalTraits<KNIGHT>::val;
const int Pawn = PtypeEvalTraits<PAWN>::val;
const int Pknight = PtypeEvalTraits<PKNIGHT>::val;
const int Bishop = PtypeEvalTraits<BISHOP>::val;
const int Pbishop = PtypeEvalTraits<PBISHOP>::val;

typedef NumEffectState state_t;
typedef QuiescenceSearch2<PieceEval> qsearch_t;

BOOST_AUTO_TEST_CASE(QuiescenceSearchTest050313)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
// 81RY 82FU PASS は68RY で詰ですよん
//P1-KY-KE *  *  *  * +RY * -KY
//P2 *  *  *  * +UM * +NK *  * 
//P3-FU-OU-GI-FU-FU-FU *  * -FU
//P4 *  * -FU *  *  *  *  *  * 
//P5 *  *  *  * +KA *  *  *  * 
//P6 *  *  *  *  *  *  *  *  * 
//P7+FU * +FU+FU+FU+FU+FU * +FU
//P8 *  * -NK * +OU *  *  *  * 
//P9+KY+KE * -HI * +KI+GI * +KY
//P+00KI00FU00FU
//P-00KI00KI00GI00GI00FU00FU00FU
//+

  // 王手を入れるとループがある問題
  {
    state_t state(CsaString(
		    "P1-KY+HI *  *  * -OU * -KE-KY\n"
		    "P2 *  *  *  *  *  * -KI *  * \n"
		    "P3 *  * -KE * -KI-FU * -FU * \n"
		    "P4-FU * +KI *  *  * -FU * -FU\n"
		    "P5 *  *  * -KE-GI+GI *  *  * \n"
		    "P6 *  *  *  *  *  * +FU * +FU\n"
		    "P7+FU+FU+KI * +KA+FU *  *  * \n"
		    "P8+KA *  *  *  *  *  * +HI * \n"
		    "P9+KY * +OU *  *  *  * +KE+KY\n"
		    "P+00FU00FU00FU00FU00FU00FU\n"
		    "P-00GI00GI00FU00FU\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(10000,-6);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int result = searcher.search(WHITE, ev, 
				       Move(Square(8,1),ROOK,BLACK));
    BOOST_CHECK(result < 1000000);
  }
  {
    state_t state(CsaString(
		    "P1 *  *  *  *  *  *  * -KE-KY\n"
		    "P2 * +RY *  *  *  * -KI-OU * \n"
		    "P3 *  *  *  *  * +FU-GI-KI * \n"
		    "P4-FU *  * -FU-FU *  *  * -FU\n"
		    "P5 * -UM *  *  * -FU-FU *  * \n"
		    "P6+FU+KI+FU+FU+FU *  *  * +FU\n"
		    "P7+OU *  *  * +GI *  *  *  * \n"
		    "P8 *  *  *  *  *  *  * -RY * \n"
		    "P9+KY+KE-GI *  *  *  *  *  * \n"
		    "P+00GI00FU00FU00FU00FU00FU\n"
		    "P-00KA00KI00KE00KE00KY00KY00FU\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(10000,-6);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int result = searcher.search(WHITE, ev, 
				       Move(Square(8,6),GOLD,BLACK));
    BOOST_CHECK(result < -5000000);
    BOOST_CHECK(searcher.nodeCount() < 5000); // 1手詰なんだけど...
  }
  {
    state_t state(CsaString(
		    "P1 *  *  *  *  *  *  * -KE-KY\n"
		    "P2 * +RY *  *  *  * -KI-OU * \n"
		    "P3 *  *  *  *  * +FU-GI-KI * \n"
		    "P4-FU *  * -FU-FU *  *  * -FU\n"
		    "P5 * -UM *  *  * -FU-FU *  * \n"
		    "P6+FU+KI+FU+FU+FU *  *  * +FU\n"
		    "P7+OU *  *  * +GI *  *  *  * \n"
		    "P8 * -GI *  *  *  *  * -RY * \n"
		    "P9+KY+KE+KE *  *  *  *  *  * \n"
		    "P+00GI00FU00FU00FU00FU00FU\n"
		    "P-00KA00KI00KE00KY00KY00FU\n"
		    "+\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(10000,-6);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int result = searcher.search(BLACK, ev, 
				       Move(Square(8,8),SILVER,WHITE));
    BOOST_CHECK(result < 0);
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestLose2)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  // -8889TO が先に生成されると，負けを認識できないバグが以前あった
  // -0097FU から作った局面とは駒番号が違う?
  state_t state(CsaString(
		  "P1 *  *  *  *  *  * -KI-KE-OU\n"
		  "P2 *  *  *  *  *  * -KI-GI-KY\n"
		  "P3 *  *  *  *  *  * -FU-FU-FU\n"
		  "P4 *  *  *  *  *  *  *  *  * \n"
		  "P5 *  *  *  *  *  *  *  *  * \n"
		  "P6 *  *  *  *  *  *  *  *  * \n"
		  "P7-FU-GI *  *  *  *  *  *  * \n"
		  "P8 * -FU *  *  *  *  *  *  * \n"
		  "P9+OU *  *  *  *  *  *  *  * \n"
		  "P+00AL\n"
		  "+\n").initialState());
  SearchState2Core sstate(state, checkmate_searcher);
  SimpleHashTable table(1000,-6);
  qsearch_t searcher(sstate, table);
  PieceEval ev(state);
  const int result = searcher.search<BLACK>(ev, Move(Square(9,7),PAWN,WHITE));
  BOOST_CHECK(result < -100000);
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestLose)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  state_t state(CsaString(	// 色々とれるが取ると詰，実は必死
			 "P1-KY-KE-GI * -OU-KI-GI-KE-KY\n"
			 "P2 * -HI *  *  *  *  * -KA * \n"
			 "P3-FU+FU-FU-FU+FU-FU-FU+FU-FU\n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  * -FU * \n"
			 "P6 * -FU *  *  * +FU *  *  * \n"
			 "P7+FU * +FU+FU-FU-KI+FU+HI+FU\n"
			 "P8 * +KA+KE * +KE *  * +KI * \n"
			 "P9+KY+KI+GI * +OU * +GI * +KY\n"
			 "+\n").initialState());
  SearchState2Core sstate(state, checkmate_searcher);
  SimpleHashTable table(0);
  qsearch_t searcher(sstate, table);
  PieceEval ev(state);
  const int val1 = searcher.search(BLACK, ev, Move(Square(5,7),PAWN,WHITE));
  // TODO: まだ必死は認識できない
  // BOOST_CHECK_EQUAL(FixedEval::winValue(WHITE), val1);
  // std::cerr << val1 << "\n";
  BOOST_CHECK(val1 < -1000);
}
BOOST_AUTO_TEST_CASE(QuiescenceSearchTestForcedEscape)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  state_t state(CsaString(	// 色々とれるが取ると詰，桂損確定
			 "P1-KY-KE-GI * -OU-KI-GI-KE-KY\n"
			 "P2 * -HI *  *  *  *  * -KA * \n"
			 "P3-FU+FU-FU-FU+FU-FU-FU+FU-FU\n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  * -FU * \n"
			 "P6 * -FU+KI *  * +FU *  *  * \n"
			 "P7+FU * +FU+FU-FU-KI+FU+HI+FU\n"
			 "P8 * +KA *  * +KE *  * +KI * \n"
			 "P9+KY+KE+GI * +OU * +GI * +KY\n"
			 "+\n").initialState());
  SearchState2Core sstate(state, checkmate_searcher);
  SimpleHashTable table(0);
  qsearch_t searcher(sstate, table);
  PieceEval ev(state);
  const int val1 = searcher.search(BLACK, ev, Move(Square(5,7),PAWN,WHITE));
  BOOST_CHECK(val1 <= -PtypeEvalTraits<KNIGHT>::val*2);
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestKnightAttack)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
#if 0
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI * -KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  * -KE *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  * +FU *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU * +FU+FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "+\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(BLACK, ev, Move(Square(3,4),KNIGHT,WHITE));
    BOOST_CHECK(val1 > 0);
  }
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU * -FU * \n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  * +KE * -FU\n"
		    "P7+FU * +FU+FU+FU+FU+FU+FU+FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI * +KY\n"
		    "P-00FU00FU\n"
		    "+\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(BLACK, ev, Move(Square(1,6),PAWN,WHITE));
    BOOST_CHECK(val1 < 0);
  }
#endif
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestNoCapture)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  {
    state_t state(CsaString(
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
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(BLACK, ev, Move(Square(8,2),ROOK,WHITE));
    BOOST_CHECK_EQUAL(0, val1);
  }
  {
    state_t state(CsaString(
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  *  *  *  * -KA * \n"
      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 * -HI *  * +GI *  *  *  * \n"
      "P6 *  *  *  * +FU *  *  *  * \n"
      "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
      "P8 *  * +KI *  * +KA * +HI * \n"
      "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
      "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(WHITE, ev, Move(Square(5,6),PAWN,BLACK));
    BOOST_CHECK_EQUAL(0, val1);
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestCaptureBlack)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  SimpleHashTable table(0);
  {
    state_t state1(CsaString(
			    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			    "P2 *  *  *  *  *  *  * -KA * \n"
			    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			    "P4 *  *  *  *  *  *  *  *  * \n"
			    "P5 *  *  *  *  *  *  *  *  * \n"
			    "P6 * -HI *  *  *  *  *  *  * \n"
			    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			    "P8 * +KA *  *  *  *  * +HI * \n"
			    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			    "+\n").initialState());
    SearchState2Core sstate(state1, checkmate_searcher);
    qsearch_t searcher1(sstate, table);
    PieceEval ev(state1);
    const int val1 = searcher1.search(BLACK, ev, Move(Square(8,6),ROOK,WHITE));
    BOOST_CHECK_EQUAL(PtypeEvalTraits<ROOK>::val*2, val1);
  }
  {    
    // 飛車より角を取るのが得
    state_t state2(CsaString(
			    "P1-KY-KE * -KI-OU-KI-GI-KE-KY\n"
			    "P2 * -GI *  *  *  *  *  *  * \n"
			    "P3-FU * -FU-FU-FU-FU-FU-FU-FU\n"
			    "P4 * +FU *  *  *  *  *  *  * \n"
			    "P5 * -FU *  *  *  *  *  *  * \n"
			    "P6 * -HI *  *  *  *  * -KA * \n"
			    "P7+FU+HI+FU+FU+FU+FU+GI+FU+FU\n"
			    "P8 * +KA+GI *  *  * +FU *  * \n"
			    "P9+KY+KE * +KI+OU * +KI+KE+KY\n"
			    "+\n").initialState());
    SearchState2Core sstate(state2, checkmate_searcher);
    qsearch_t searcher2(sstate, table);
    PieceEval ev2(state2);
    const int val2 = searcher2.search(BLACK, ev2, 
				      Move(Square(8,6),ROOK,WHITE));
    BOOST_CHECK_EQUAL(PtypeEvalTraits<BISHOP>::val*2, val2); //  + 脅威
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestCaptureWhite)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  SimpleHashTable table(0);
  {
    state_t state1(CsaString(
			    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			    "P2 *  *  *  *  *  *  * -KA * \n"
			    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			    "P4 *  *  *  *  *  *  *  *  * \n"
			    "P5 *  *  *  *  *  *  *  *  * \n"
			    "P6 * -HI *  *  *  *  *  *  * \n"
			    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			    "P8 * +KA *  *  *  * +HI+GI+KI\n"
			    "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
			    "-\n").initialState());
    SearchState2Core sstate(state1, checkmate_searcher);
    qsearch_t searcher1(sstate, table);
    PieceEval ev(state1);
    const int val1 = searcher1.search(WHITE, ev, Move(Square(8,7),PAWN,BLACK));
    BOOST_CHECK_EQUAL(-PtypeEvalTraits<PROOK>::val
			 -PtypeEvalTraits<PAWN>::val*2
			 +PtypeEvalTraits<ROOK>::val,
			 val1);
  }
  {    
    // 飛車を先にとれば駒損なし
    state_t state2(CsaString(
			    "P1-KY-KE * -KI-OU-KI-GI-KE-KY\n"
			    "P2 * -GI *  *  *  *  *  *  * \n"
			    "P3-FU * -FU-FU-FU-FU-FU-FU-FU\n"
			    "P4 * +FU *  *  *  *  *  *  * \n"
			    "P5 * -FU *  *  *  *  *  * -KA\n"
			    "P6 * -HI+FU *  *  *  *  *  * \n"
			    "P7+FU+HI+KA+FU+FU+FU+GI+FU+FU\n"
			    "P8 *  * +GI * +OU * +FU *  * \n"
			    "P9+KY+KE * +KI * +KI * +KE+KY\n"
			    "-\n").initialState());
    SearchState2Core sstate(state2, checkmate_searcher);
    qsearch_t searcher2(sstate, table);
    PieceEval ev2(state2);
    const int val2 = searcher2.search(WHITE, ev2, Move(Square(7,8),SILVER,BLACK));
    BOOST_CHECK_EQUAL(0, val2); // 0 + 脅威
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestCaptureCheck)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  SimpleHashTable table(0);
  {
    // 銀を取ると龍が素抜きなので歩を取る
    state_t state0(CsaString(
		     "P1 * -OU *  * -RY-GI * +RY * \n"
		     "P2 *  *  *  * -KA * +GI *  * \n"
		     "P3-FU-FU-FU-FU * -FU-FU-FU-FU\n"
		     "P4 *  *  *  * -FU * -GI-KI * \n"
		     "P5 *  *  *  * +FU *  *  *  * \n"
		     "P6 *  *  *  *  *  *  *  *  * \n"
		     "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
		     "P8 * +KA *  *  *  * +KI *  * \n"
		     "P9+KY+KE+GI+KI+OU *  * +KE+KY\n"
		     "P-00AL\n"
		     "-\n").initialState());
    SearchState2Core sstate(state0, checkmate_searcher);
    qsearch_t searcher0(sstate, table);
    PieceEval ev0(state0);
    const int val0 = searcher0.search(WHITE, ev0, Move(Square(3,2),SILVER,BLACK));
    BOOST_CHECK_EQUAL(-Pawn*2+Psilver-Silver, val0);
  }
  {
    // 32金を取ると王が素抜き
    state_t state1(CsaString(
		     "P1-KY-OU *  *  * -KI * +RY * \n"
		     "P2-KY *  *  * -GI * +KI *  * \n"
		     "P3-FU-FU-FU-KA * -FU-FU-FU-FU\n"
		     "P4 *  *  *  * -FU *  * -RY * \n"
		     "P5 *  *  * -FU+FU *  *  *  * \n"
		     "P6 *  *  *  *  *  *  *  *  * \n"
		     "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
		     "P8 * +KA *  *  *  * +GI *  * \n"
		     "P9+KY+KE+GI * +OU+KI * +KE+KY\n"
		     "P-00AL\n"
		     "-\n").initialState());
    SearchState2Core sstate(state1, checkmate_searcher);
    BOOST_CHECK_EQUAL(0, PieceEval(state1).value());
    qsearch_t searcher1(sstate, table);
    PieceEval ev(state1);
    const int val1 = searcher1.search(WHITE, ev, Move(Square(3,2),GOLD,BLACK));
    BOOST_CHECK_EQUAL(-PtypeEvalTraits<PAWN>::val*2, val1);
  }
  {
    state_t state2(CsaString(
		     "P1-KY-KE-KE-KI *  *  * +RY-KY\n"
		     "P2-KA-GI-OU-FU *  *  *  *  * \n"
		     "P3-FU-FU * -KI *  *  *  *  * \n"
		     "P4 *  *  *  * +FU *  * -FU-FU\n"
		     "P5 *  *  * +FU *  *  *  *  * \n"
		     "P6 *  * +FU *  *  * +GI+FU+FU\n"
		     "P7+FU *  *  * -GI * +KI *  * \n"
		     "P8 *  *  *  *  * -RY+FU+OU * \n"
		     "P9+KY+KE *  *  *  * -GI+KE+KY\n"
		     "P+00FU00FU00KI00KA\n"
		     "P-00FU00FU00FU00FU\n"
		     "+\n"
		     ).initialState());
    // +2818OU -6354KI +2111RY で損なので後手は2手目でパス
    SearchState2Core sstate(state2, checkmate_searcher);
    qsearch_t searcher2(sstate, table);
    PieceEval ev(state2);
    if (OslConfig::verbose())
      std::cerr << ev.value() << "\n";
    const Move last_move = Move(Square(3,9),SILVER,WHITE);
    const int val2 = searcher2.search(BLACK, ev, last_move);
    const int val2n = searcher2.search<BLACK>(-899, -895, ev, last_move);
    BOOST_CHECK_EQUAL(val2, val2n);
  }
  {
    state_t state3(CsaString(
		     "P1-KY-KE * -KI *  *  * +RY-KY\n"
		     "P2 * -GI-OU-FU *  *  *  *  * \n"
		     "P3-FU-FU * -KI *  *  *  *  * \n"
		     "P4 *  *  *  * +FU *  *  * -FU\n"
		     "P5 *  *  * +FU *  *  *  *  * \n"
		     "P6 *  * +FU *  *  * +GI * +FU\n"
		     "P7+FU *  *  *  *  * +KI+FU * \n"
		     "P8 *  *  *  *  * -RY+FU+OU * \n"
		     "P9+KY+KE *  *  * -GI-KA+KE+KY\n"
		     "P+00FU00FU00KI00KA\n"
		     "P-00FU00FU00FU00FU00FU00KE00GI\n"
		     "+\n"
		     ).initialState());
    SearchState2Core sstate(state3, checkmate_searcher);
    qsearch_t searcher3(sstate, table);
    PieceEval ev(state3);
    if (OslConfig::verbose())
      std::cerr << ev.value() << "\n";
    // +2818OU -4938NG PASS
    const Move last_move = Move(Square(3,9),BISHOP,WHITE);
    const int val3  = searcher3.search(BLACK, ev, last_move);
    const int val3n = searcher3.search<BLACK>(val3-1, val3+1, 
					      ev, last_move);
    // window を狭めても結果が同じ
    BOOST_CHECK_EQUAL(val3, val3n);
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestCaptureCheckMate)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  SimpleHashTable table(0);
  {
    // 詰
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-KA-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -OU * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  * +FU * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "+\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(BLACK, ev, Move(Square(2,3),PAWN,WHITE));
    BOOST_CHECK_EQUAL(FixedEval::winByCheckmate(BLACK), val1);
  }
  {
    // 王手じゃなければ飛車がとれるが，逃げている間に歩をとられる．
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI * -KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU * -FU-FU-FU-OU-FU-FU\n"
		    "P4 *  *  *  *  *  * -FU *  * \n"
		    "P5 *  * -FU *  *  * +HI *  * \n"
		    "P6 *  * +FU *  *  *  *  *  * \n"
		    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
		    "P8 * +KA *  *  *  *  *  *  * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(WHITE, ev, Move(Square(7,6),PAWN,BLACK));
    BOOST_CHECK_EQUAL(Pawn*2, val1);
  }
  {
    // 王で飛車を取って王手回避
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI * -KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-OU-FU-FU\n"
		    "P4 *  *  *  *  *  * -FU+HI * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  * +FU *  *  *  *  *  * \n"
		    "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
		    "P8 * +KA *  *  *  *  *  *  * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(WHITE, ev, Move(Square(7,6),PAWN,BLACK));
    BOOST_CHECK_EQUAL(-PtypeEvalTraits<ROOK>::val*2, val1);
  }
  {
    // 送り金
    state_t state(CsaString(
		    "P1 *  *  *  *  *  *  *  *  * \n"
		    "P2 * +KI-OU-KI+RY *  *  *  * \n"
		    "P3-FU-FU-FU-FU-FU * -FU-FU-FU\n"
		    "P4 *  * -RY *  *  * -KI *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
		    "P8 * +KA * +GI *  *  *  *  * \n"
		    "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
		    "P-00AL\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(WHITE, ev, Move(Square(8,2),GOLD,BLACK));
    BOOST_CHECK_EQUAL(Pawn*2, val1);
  }
  {
    // バグの可能性があったが QuiescenceSearch のせいではないようだ
    state_t state(CsaString(
		    "P1-KY *  *  *  * +NK-OU * -KY\n"
		    "P2 *  *  *  * +TO-GI-KI *  * \n"
		    "P3-FU * -HI *  * -FU * -FU * \n"
		    "P4 *  *  *  * -GI * -FU-KY-FU\n"
		    "P5 * -FU *  *  * -KE * +FU * \n"
		    "P6+FU *  * -FU+FU *  *  *  * \n"
		    "P7+KA+FU+KE+GI * +FU+FU * +FU\n"
		    "P8-GI+KI+KI *  *  *  * +HI * \n"
		    "P9-UM * +OU *  *  *  * +KE+KY\n"
		    "P+00FU\n"
		    "P+00FU\n"
		    "P-00FU\n"
		    "P-00KI\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int val1 = searcher.search(WHITE, ev, 
				     Move(Square(5,1),Square(4,1),
					     PKNIGHT,PTYPE_EMPTY,false,BLACK));
    BOOST_CHECK(eval::isConsistentValueForNormalState<PieceEval>(val1));
    BOOST_CHECK(! FixedEval::isWinValue(BLACK, val1));
  }
}

static int error_count = 0;
static void testIsEvalValueConsistent(std::string const& filename)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  // QuiescenceLog::init("qsearch.log");
  
  auto record=CsaFileMinimal(filename).load();
  const NumEffectState src(record.initialState());
  const std::vector<osl::Move> moves=record.moves;

  typedef QuiescenceSearch2<PieceEval> qsearch_t;

  PieceEval ev(src);
  GameState state(src);
  SimpleHashTable table(400000,-8,OslConfig::verbose());
  SimpleHashTable null_table(400000,100,false);
  const int black_win = search::FixedEval::winByLoop(BLACK);
  const int white_win = search::FixedEval::winByLoop(WHITE);
  time_point started = clock::now();
  size_t node_count = 0;
 for (unsigned int i=0;i<moves.size();i++)
  {
    if (i > 0)
    {
#if 0
      if ((i % 64) == 0)
	table.clear();
#endif
      NumEffectState state0(state.state());
      NumEffectState state1(state0);
      HistoryToTable::adjustTable(state, table, black_win, 0, white_win);
      HistoryToTable::adjustTable(state, null_table, black_win, 0, white_win);

      SearchState2Core sstate0(state0, checkmate_searcher);
      SearchState2Core sstate1(state1, checkmate_searcher);
      qsearch_t qs_null(sstate0, null_table);
      qsearch_t qs(sstate1, table);
      const Move last_move = moves[i-1];
      const int val_null = qs_null.search(state0.turn(), ev, last_move, 6);
      const int val = qs.searchIteratively(state1.turn(), ev, last_move, 6);
      node_count += qs.nodeCount() + qs_null.nodeCount();
      if (abs(val_null - val) > 10000)
      {
	if (OslConfig::verbose())
	  std::cerr << filename << " " << i << "\n" << state0
		    << "\n" << ev.value() << " " << val_null << " " << val << "\n";
	++error_count;
      }
    
      BOOST_CHECK(isConsistentValue(val));
      
      if (OslConfig::inUnitTestShort())		// 最近はかなり異なる
	BOOST_CHECK(error_count < 50);
      // BOOST_CHECK_EQUAL(val_null, val);
      BOOST_CHECK(state0.isConsistent(true));
      BOOST_CHECK(state1.isConsistent(true));
    }

    const Move move = moves[i];
    state.pushMove(move);
    ev.update(state.state(), move);
  }
  if (OslConfig::verbose())
  {
    std::cerr << node_count << " " << toSeconds(clock::now() - started) << "\n";
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestEquality)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  const int count = (OslConfig::inUnitTestShort() ? 10: 100);
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong() && !OslConfig::verbose())
    progress.reset(new boost::progress_display(count, std::cerr));
  std::string file_name;
  for (int i=0; i<count && (ifs >> file_name); i++){
    if (progress)
      ++(*progress);
    if (file_name == "") 
      break;
    testIsEvalValueConsistent(OslConfig::testCsaFile(file_name));
  }
  BOOST_CHECK(error_count < 50);
  // BOOST_CHECK_EQUAL(0, error_count);
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestTakeBack)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  * +FU * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.takeBackValue<WHITE>
      (1001, -1001, ev, Move(Square(2,6),PAWN,BLACK));
    BOOST_CHECK_EQUAL(0, value);
  }
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  * +FU * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.takeBackValue<WHITE>
      (1001, -1001, ev, Move(Square(2,4),PAWN,BLACK));
    BOOST_CHECK_EQUAL(0, value);
  }
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  * +FU * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  *  * +HI\n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.takeBackValue<WHITE>
      (1001, -1001, ev, Move(Square(2,4),PAWN,BLACK));
    BOOST_CHECK_EQUAL(-Pawn*2, value);
  }
  {
    state_t state(CsaString(	// 取れない
		    "P1-KY-KE-GI-KI-OU-KI-GI * -KY\n"
		    "P2 * -HI *  *  *  *  *  *  * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5 *  *  *  *  * +FU+FU * -KA\n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU-KE * +FU+FU\n"
		    "P8 * +KA *  *  * +GI * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI * +KE+KY\n"
		    "+\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.takeBackValue<BLACK>
      (-1001, 1001, ev, Move(Square(4,7),KNIGHT,WHITE));
    BOOST_CHECK_EQUAL(0, value);
  }
  {				// 詰
    state_t state(CsaString(
		    "P1-KY-KE * -KI *  *  * +RY-KY\n"
		    "P2 * -GI-OU-FU *  *  *  *  * \n"
		    "P3-FU-FU * -KI *  *  *  *  * \n"
		    "P4 *  *  *  * +FU *  *  * -FU\n"
		    "P5 *  *  * +FU *  *  *  *  * \n"
		    "P6 *  * +FU *  *  * +GI * +FU\n"
		    "P7+FU *  *  *  *  * -NG+FU * \n"
		    "P8 *  *  *  *  * -RY *  * +OU\n"
		    "P9+KY+KE *  *  *  * -KA+KE+KY\n"
		    "P+00FU00FU00KI00KA\n"
		    "P-00FU00FU00FU00FU00FU00KE00GI00FU00KI\n"
		    "+\n"
		    ).initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.takeBackValue<BLACK>
      (-10001,+10001,ev,Move(Square(3,8),Square(3,7),PSILVER,GOLD,false,WHITE));
    BOOST_CHECK_EQUAL(FixedEval::winByCheckmate(WHITE), value);
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestThreat)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  * +FU * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.staticValueWithThreat<WHITE>(ev);
    BOOST_CHECK_EQUAL(ev.value(), value);
  }
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  * +FU * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.staticValueWithThreat<WHITE>(ev);
    BOOST_CHECK(ev.value() < value); // 後手がパスすると危険
  }
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  * +FU * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  *  * +HI\n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.staticValueWithThreat<WHITE>(ev);
    BOOST_CHECK(ev.value() < value);
    BOOST_CHECK(state.isConsistent());
  }
  {
    state_t state(CsaString(
		    "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
		    "P2 * -HI *  *  *  * -KI-KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  * +FU * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
		    "P8 * +KA *  *  *  *  *  * +HI\n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.staticValueWithThreat<WHITE>(ev);
    BOOST_CHECK_EQUAL(ev.value(), value);
    BOOST_CHECK(state.isConsistent());
  }
  {
    state_t state(CsaString(	// 取れない
		    "P1-KY-KE-GI-KI-OU-KE-GI * -KY\n"
		    "P2 * -HI *  *  *  *  *  *  * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5 *  *  *  *  * +FU+FU * -KA\n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU-KI * +FU+FU\n"
		    "P8 * +KA * +GI * +KI * +HI * \n"
		    "P9+KY+KE * +KI+OU * +GI+KE+KY\n"
		    "+\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.staticValueWithThreat<BLACK>(ev);
    BOOST_CHECK_EQUAL(ev.value(), value);
    BOOST_CHECK(state.isConsistent());
  }
  {
    state_t state(CsaString(
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  *  *  *  * -KA * \n"
      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 * -HI *  * +GI *  *  *  * \n"
      "P6 *  *  *  * +FU *  *  *  * \n"
      "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
      "P8 *  * +KI *  * +KA * +HI * \n"
      "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
      "+\n").initialState());
    SimpleHashTable table(0);
    SearchState2Core sstate(state, checkmate_searcher);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.staticValueWithThreat<BLACK>(ev);
    BOOST_CHECK_EQUAL(0, value);
  }
  {
    state_t state(CsaString(
		    "P1-KY-KE * -KI *  *  * +RY-KY\n"
		    "P2 * -GI-OU-FU *  *  *  *  * \n"
		    "P3-FU-FU * -KI *  *  *  *  * \n"
		    "P4 *  *  *  * +FU *  *  * -FU\n"
		    "P5 *  *  * +FU *  *  *  *  * \n"
		    "P6 *  * +FU *  *  * +GI * +FU\n"
		    "P7+FU *  *  *  *  * +KI+FU * \n"
		    "P8 *  *  *  *  * -RY-NG * +OU\n"
		    "P9+KY+KE *  *  *  * -KA+KE+KY\n"
		    "P+00FU00FU00KI00KA\n"
		    "P-00FU00FU00FU00FU00FU00KE00GI00FU\n"
		    "+\n"
		    ).initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(0);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const int value = searcher.staticValueWithThreat<BLACK>(ev);
    BOOST_CHECK(value < -8000);
  }
}

static bool betterThan(const char *l, const char *r)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
  state_t state_l(CsaString(l).initialState());
  state_t state_r(CsaString(r).initialState());
  assert(state_l.turn() == state_r.turn());
  SimpleHashTable table(1000);
  int value_l, value_r;
  {
    // root ではない状態にするため、手番を変更しておいてからパス
    SearchState2Core sstate(state_l, checkmate_searcher);
    sstate.pushPass();
    qsearch_t searcher(sstate, table);
    sstate.pushPass();

    PieceEval ev(state_l);
    value_l = searcher.staticValueWithThreat(ev);
  }
  table.clear();
  {
    // root ではない状態にする
    SearchState2Core sstate(state_r, checkmate_searcher);
    sstate.pushPass();
    qsearch_t searcher(sstate, table);
    sstate.pushPass();

    PieceEval ev(state_r);
    value_r = searcher.staticValueWithThreat(ev);
  }
  return eval::betterThan(state_l.turn(), value_l, value_r);
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestCompare)
{
  if (QSearchTraits::FirstThreat >= 20)
  {
    const char *l = 
      "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
      "P2 * -HI *  *  *  * -KI-KA * \n"
      "P3-FU * -FU-FU-FU-FU-FU-FU-FU\n"
      "P4 * -FU *  *  *  *  * +HI * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
      "P8 * +KA *  *  *  *  *  *  * \n"
      "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
      "P+00FU\n"
      "+\n";
    const char *r = 
      "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
      "P2 * -HI *  *  *  * -KI-KA * \n"
      "P3-FU * -FU-FU-FU-FU-FU * -FU\n"
      "P4 * -FU *  *  *  *  * -FU * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU+FU+FU+FU+FU+FU+FU * +FU\n"
      "P8 * +KA *  *  *  *  *  * +HI\n"
      "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
      "P-00FU\n"
      "+\n";
    BOOST_CHECK(betterThan(l, r)); // 交換した方が良い
  }
  {
    // 二つの脅威
    // http://www31.ocn.ne.jp/~kfend/inside_kfend/quiescence.html#c5
    const char *l = 
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  *  *  *  * -KA * \n"
      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 * -HI *  * +GI *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
      "P8 *  * +KI *  * +KA * +HI * \n"
      "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
      "+\n";
    const char *r = 
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  *  *  *  * -KA * \n"
      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 * -HI *  * +GI *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
      "P8 *  *  *  *  * +KA * +HI * \n"
      "P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
      "+\n";
    BOOST_CHECK(betterThan(l, r));
  }
  {
    // 二つの脅威 同じptype
    const char *l = 
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  * -FU *  * -KA * \n"
      "P3-FU-FU-FU-FU * -FU-FU-FU-FU\n"
      "P4 * -HI *  * +GI *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 * +GI *  *  *  *  *  *  * \n"
      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
      "P8 *  * +KI *  * +KA * +HI * \n"
      "P9+KY+KE *  * +OU+KI * +KE+KY\n"
      "+\n";
    const char *r = 
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  * -FU *  * -KA * \n"
      "P3-FU-FU-FU-FU * -FU-FU-FU-FU\n"
      "P4 * -HI *  * +GI *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 * +GI *  *  *  *  *  *  * \n"
      "P7+FU * +FU+FU+FU+FU+FU+FU+FU\n"
      "P8 * +FU+KI *  * +KA * +HI * \n"
      "P9+KY+KE *  * +OU+KI * +KE+KY\n"
      "+\n";
    BOOST_CHECK(betterThan(l, r));
  }
  {
    // 成る脅威
    const char *l = 
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  *  *  *  * -KA * \n"
      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
      "P4 * -HI *  *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
      "P8 *  * +GI *  * +KA * +HI * \n"
      "P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
      "+\n";
    const char *r = 
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 *  *  *  *  *  *  * -KA * \n"
      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
      "P4 * -HI *  *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU * +FU+FU+FU+FU+FU+FU+FU\n"
      "P8 *  * +GI *  * +KA * +HI * \n"
      "P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
      "P+00FU\n"
      "+\n";
    BOOST_CHECK(betterThan(l, r));
  }
  {
    // 逃げられない脅威はr4103で廃止
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceSearchTestFocalPawn)
{
  osl::search::SearchState2::checkmate_t checkmate_searcher;
#if 0
  {
    // -2324FU+3524GI-4334GI で後手歩得で有利 とか思っていると
    // +0033FUで破滅
    state_t state(CsaString(
		    "P1-KY-KE * -KI *  *  * -KE * \n"
		    "P2 * -OU-GI * -KI * -HI-KA-KY\n"
		    "P3 * -FU-FU *  * -GI * -FU-FU\n"
		    "P4-FU *  * -FU-FU-FU+FU+FU * \n"
		    "P5 *  *  *  *  *  * +GI *  * \n"
		    "P6+FU * +FU * +FU *  *  * +FU\n"
		    "P7 * +FU * +FU * +FU *  *  * \n"
		    "P8 * +KA+OU * +KI * +HI *  * \n"
		    "P9+KY+KE+GI+KI *  *  * +KE+KY\n"
		    "P-00FU\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(10000,-6);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const Move m24fu(Square(2,5),Square(2,4),PAWN,PTYPE_EMPTY,false,BLACK);
    const int val = searcher.search(WHITE, ev, m24fu);
    BOOST_CHECK(val > 0);	// 先手有利!
  }
  {
    // 角が引っ込んでいても同じ
    state_t state(CsaString(
		    "P1-KY-KE * -KI *  *  * -KE-KA\n"
		    "P2 * -OU-GI * -KI * -HI * -KY\n"
		    "P3 * -FU-FU * -FU-GI * -FU-FU\n"
		    "P4-FU *  * -FU * -FU+FU+FU * \n"
		    "P5 *  *  *  *  *  * +GI *  * \n"
		    "P6+FU * +FU * +FU *  *  * +FU\n"
		    "P7 * +FU * +FU * +FU *  *  * \n"
		    "P8 * +KA+OU * +KI * +HI *  * \n"
		    "P9+KY+KE+GI+KI *  *  * +KE+KY\n"
		    "P-00FU\n"
		    "-\n").initialState());
    SearchState2Core sstate(state, checkmate_searcher);
    SimpleHashTable table(10000,-6);
    qsearch_t searcher(sstate, table);
    PieceEval ev(state);
    const Move m24fu(Square(2,5),Square(2,4),PAWN,PTYPE_EMPTY,false,BLACK);
    const int val = searcher.search(WHITE, ev, m24fu);
    BOOST_CHECK(val > 0);	// 先手有利!
  }
#endif
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
