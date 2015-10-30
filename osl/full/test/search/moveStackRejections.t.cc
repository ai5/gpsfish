#include "osl/search/moveStackRejections.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/escape_.h"
#include "osl/csa.h"
#include "osl/repetitionCounter.h"
#include "osl/hashKey.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::search;
using namespace osl::move_generator;
using namespace osl::move_action;



static bool testString(const char *str,int alpha=0){
  CsaString csaString(str);
  NumEffectState state(csaString.initialState());
  std::vector<osl::Move> moves=csaString.load().moves;
  MoveStack history;
  RepetitionCounter repCounter(state);
  for(int i=0;i<(int)moves.size()-1;i++){
    Move move=moves[i];
    history.push(move);
    state.makeMove(move);
    repCounter.push(state);
  }
  Move lastMove=moves[moves.size()-1];
  int size=moves.size()-1;
  int cc=repCounter.checkCount(alt(state.turn()));
  if(state.turn()==BLACK)
    return MoveStackRejections::probe<BLACK>(state,history,size,lastMove,alpha,cc);
  else
    return MoveStackRejections::probe<WHITE>(state,history,size,lastMove,alpha,cc);
}


BOOST_AUTO_TEST_CASE(MoveStackRejectionsTestProbe)
{
  {
    // myPlus = 0, opPlus = 1, opMinus=1, myMinus=0, valid, gt
    BOOST_CHECK(testString(
"P1-KY-HI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  * -KI *  * \n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU+FU+FU\n"
"P6+FU * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  * +HI * \n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA\n"
"P-00KA\n"
"-\n"
"-0039KA\n"
"+2838HI\n"
"-3928UM\n"
			      ));
  }
  {
    // myPlus = 0, opPlus = 1, opMinus=1, myMinus=0, valid, eq, rejectSennichite
    BOOST_CHECK(testString(
"P1-KY-HI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  * -KI *  * \n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU+FU+FU\n"
"P6+FU * +FU+FU+FU+FU+FU * -UM\n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA\n"
"-\n"
"-1627UM\n"
"+1828HI\n"
"-2716UM\n"
			      ));
  }
  {
    // myPlus = 0, opPlus = 1, opMinus=1, myMinus=0, valid, eq, notrejectSennichite
    // alpha is greater than zero, so white prefers the draw game.
    BOOST_CHECK(!testString(
"P1-KY-HI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  * -KI *  * \n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU+FU+FU\n"
"P6+FU * +FU+FU+FU+FU+FU * -UM\n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA\n"
"-\n"
"-1627UM\n"
"+1828HI\n"
"-2716UM\n"
,100));
  }
  {
    // myPlus = 0, opPlus = 1, opMinus=1, myMinus=0, invalid, gt
    // the ROOK at square 81 can't capture the PAWN at square 86, 
    // because there is a ROOK at square 85
    BOOST_CHECK(!testString(
"P1-KY *  *  *  *  *  * -KE-KY\n"
"P2 *  *  *  *  *  * -KI-OU * \n"
"P3-FU *  * -GI * -FU-GI-FU-FU\n"
"P4 *  *  * -KI * -KA-FU *  * \n"
"P5 * -HI+FU-FU-FU *  *  * +FU\n"
"P6+FU+HI *  *  * +FU *  *  * \n"
"P7 * +FU * +FU+FU+GI+FU *  * \n"
"P8 * +KA+KI *  *  * +GI *  * \n"
"P9+KY *  *  *  * +KI+OU+KE+KY\n"
"P+00KE00FU\n"
"P-00KE00FU00FU\n"
"-\n"
"-8575HI\n"
"+0076FU\n"
"-0085FU\n"
"+7675FU\n"
"-8586FU\n"
"+0081HI\n"
"-0085HI\n"
			      ));
  }
  {
    // myPlus = 0, opPlus = 1, opMinus=2, myMinus=0, valid, eq, rejectSennichite
    BOOST_CHECK(!testString(
"P1-KY-HI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  * -KI *  * \n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6+FU * +FU+FU+FU+FU+FU * -UM\n"
"P7 * +FU+KE+KI * +GI+KE+FU * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA\n"
"-\n"
"-1627UM\n"
"+1828HI\n"
"-2716UM\n"
			      ));
  }
  {
    // myPlus = 0, opPlus = 1, opMinus=1, myMinus=1, valid, eq, rejectSennichite
    BOOST_CHECK(!testString(
"P1-KY-HI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  * -KI *  * \n"
"P3 *  * -KE-GI * -FU-KE * -FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU+FU+FU\n"
"P6+FU * +FU+FU+FU+FU+FU * -UM\n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  * -FU+HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA\n"
"-\n"
"-1627UM\n"
"+1828HI\n"
"-2716UM\n"
			      ));
  }
  {
    // myPlus=0, opPlus=0, opMinus=1, myMinus=0, !isPromoted, gt
    BOOST_CHECK(testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"+\n"
"+0024FU\n"
"-2122RY\n"
"+2423FU\n"
"-2223RY\n"
"+0024FU\n"
"-2324RY\n"
"+1816HI\n"
"-2421RY\n"
"+1618HI\n"
			      ));
  }
  {
    // myPlus=0, opPlus=0, opMinus=1, myMinus=0, !isPromoted, eq, rejectSennichite
    BOOST_CHECK(testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"+\n"
"+0024FU\n"
"-2122RY\n"
"+2423FU\n"
"-2223RY\n"
"+1816HI\n"
"-2321RY\n"
"+1618HI\n"
			      ));
  }
  {
    // myPlus=0, opPlus=0, opMinus=1, myMinus=0, !isPromoted, eq, rejectSennichite
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"+\n"
"+0024FU\n"
"-2122RY\n"
"+2423FU\n"
"-2223RY\n"
"+1816HI\n"
"-2321RY\n"
"+1618HI\n",
			       -100));
  }
  {
    // myPlus=0, opPlus=0, opMinus=1, myMinus=0, isPromoted, eq, rejectSennichite
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  * -UM * \n"
"P3 *  * -KE-GI * -FU-KE-TO-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"+\n"
"+0024FU\n"
"-2132RY\n"
"+2423FU\n"
"-3223RY\n"
"+1816HI\n"
"-2332RY\n"
"+1617HI\n"
"-3221RY\n"
"+1718HI\n"
			      ));
  }
  {
    // myPlus=0, opPlus=0, opMinus=0, myMinus=0 eq, rejectSennichite
    BOOST_CHECK(testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  * -UM * \n"
"P3 *  * -KE-GI * -FU-KE-TO-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"+\n"
"+1816HI\n"
"-2112RY\n"
"+1618HI\n"
"-1221RY\n"
			      ));
  }
  {
    // myPlus=0, opPlus=0, opMinus=0, myMinus=0 eq, notRejectSennichite
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  * -UM * \n"
"P3 *  * -KE-GI * -FU-KE-TO-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"+\n"
"+1816HI\n"
"-2112RY\n"
"+1618HI\n"
"-1221RY\n"
,100));
  }
  {
    // myPlus=0, opPlus=0, opMinus=2
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE * -FU\n"
"P4-FU * -FU-FU-FU-GI * +FU * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"+\n"
"+2423TO\n"
"-2123RY\n"
"+3725KE\n"
"-2325RY\n"
"+1816HI\n"
"-2522RY\n"
"+1618HI\n"
"-2221RY\n"
			      ));
  }
  {
    // myPlus=1, myMinus=1, opMinus=1, opPlus=0, valid, ge
    BOOST_CHECK(testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE+FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"-\n"
"-1223UM\n"
"+1816HI\n"
"-2312UM\n"
"+1618HI\n"
"-2123RY\n"
			      ));
  }
  {
    // myPlus=1, myMinus=1, opMinus=1, opPlus=0, invalid, ge
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  * -UM * \n"
"P3 *  * -KE-GI * -FU * +FU * \n"
"P4-FU * -FU-FU-FU-GI-KE * -FU\n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"-\n"
"-2223UM\n"
"+1816HI\n"
"-2322UM\n"
"+1617HI\n"
"-2132RY\n"
"+1718HI\n"
"-3223RY\n"
			      ));
  }
  {
    // myPlus=1, myMinus=1, opMinus=1, opPlus=1
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE+FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"-\n"
"-1223UM\n"
"+0025FU\n"
"-2312UM\n"
"+2524FU\n"
"-2123RY\n"
			      ));
  }
  {
    // myPlus=1, myMinus=1, opMinus=0, opPlus=0, valid, ge
    BOOST_CHECK(testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE * -FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU+FU+FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"-\n"
"-1223UM\n"
"+1816HI\n"
"-2312UM\n"
"+1618HI\n"
"-2123RY\n"
			      ));
  }
  {
    // myPlus=1, myMinus=1, opMinus=1, opPlus=0, invalid, ge
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  * -RY-KY\n"
"P2 *  * -OU-KI *  *  * -UM * \n"
"P3 *  * -KE-GI * -FU *  *  * \n"
"P4-FU * -FU-FU-FU-GI-KE * -FU\n"
"P5 * -FU *  *  *  * -FU+FU+FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"-\n"
"-2223UM\n"
"+1816HI\n"
"-2322UM\n"
"+1617HI\n"
"-2132RY\n"
"+1718HI\n"
"-3223RY\n"
			      ));
  }
  {
    // myPlus=1, myMinus=0, opPlus=1, opMinus=1, valid, eq, rejectSennichite
    BOOST_CHECK(testString(
"P1-KY-KI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE-RY-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"P-00FU\n"
"+\n"
"+0025FU\n"
"-2321RY\n"
"+2524FU\n"
"-0023FU\n"
"+2423TO\n"
			      ));
  }
  {
    // myPlus=1, myMinus=0, opPlus=1, opMinus=1, valid, eq, notRejectSennichite
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  *  *  * -UM\n"
"P3 *  * -KE-GI * -FU-KE-RY-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"P-00FU\n"
"+\n"
"+0025FU\n"
"-2321RY\n"
"+2524FU\n"
"-0023FU\n"
"+2423TO\n"
,-100));
  }
  {
    // myPlus=1, myMinus=0, opPlus=1, opMinus=0
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  *  * -RY-UM\n"
"P3 *  * -KE-GI * -FU-KE * -FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"P-00FU\n"
"+\n"
"+0026FU\n"
"-2221RY\n"
"+2625FU\n"
"-0024FU\n"
"+2524FU\n"
"-2122RY\n"
			      ));
  }
  {
    // myPlus=1, myMinus=0, opPlus=0, opMinus=0, not promoted
    BOOST_CHECK(testString(
"P1-KY-KI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  *  * -RY-UM\n"
"P3 *  * -KE-GI * -FU-KE * -FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"P-00FU\n"
"+\n"
"+0026FU\n"
"-2221RY\n"
"+2625FU\n"
"-2122RY\n"
"+2524FU\n"
			      ));
  }
  {
    // myPlus=1, myMinus=0, opPlus=0, opMinus=0, promoted
    BOOST_CHECK(!testString(
"P1-KY-KI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  *  * -RY-UM\n"
"P3 *  * -KE-GI * -FU-KE * -FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU * +FU\n"
"P6 *  * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  *  * +HI\n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA00FU00FU\n"
"P-00FU\n"
"+\n"
"+0025FU\n"
"-2221RY\n"
"+2524FU\n"
"-2122RY\n"
"+2423TO\n"
			      ));
  }
  {
    // 駒損ループの権利を相手に与えるように見えるが，実はその手が存在しない
    NumEffectState state(CsaString(
				   "P1-GI-KE *  * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU *  *  *  *  * -KY\n"
				   "P4 * -KY *  * +FU *  * -UM-FU\n"
				   "P5 *  *  * -KI * -HI-FU *  * \n"
				   "P6 *  * +KE+RY * +KY *  * +KE\n"
				   "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
				   "P8 * +KA+KI * +OU *  *  *  * \n"
				   "P9+KY * +GI *  * +KI+GI *  * \n"
				   "P-00FU\n"
				   "-\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(6,5),Square(7,5),GOLD,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,0,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,6),Square(5,5),PROOK,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,1,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,4),PAWN,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,2,m,1,0)));
      state.makeMove(m);
      history.push(m);
    }
    {
      Move m(Square(5,5),Square(6,4),PROOK,PAWN,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,3,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      // PROOKは6,6に戻れない
      Move m(Square(7,5),Square(6,5),GOLD,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,4,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
  }
  {
    // 駒損ループの権利を相手に与えるように見え，その時点ではできそうにないが，実はその手が存在する　
    NumEffectState state(CsaString(
				   "P1-GI-KE *  * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU *  *  *  *  * -KY\n"
				   "P4 * -KY *  * +FU *  * -UM-FU\n"
				   "P5 *  * -KI *  * -HI-FU *  * \n"
				   "P6 *  * +KE+RY * +KY *  * +KE\n"
				   "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
				   "P8 * +KA+KI * +OU *  *  *  * \n"
				   "P9+KY * +GI *  * +KI+GI *  * \n"
				   "P-00FU\n"
				   "-\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(7,5),Square(6,5),GOLD,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,0,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,6),Square(5,5),PROOK,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,1,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,4),PAWN,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,2,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(5,5),Square(6,4),PROOK,PAWN,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,3,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      // PROOKは6,6に戻れる
      Move m(Square(6,5),Square(7,5),GOLD,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((MoveStackRejections::probe<WHITE>(state,history,4,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
  }
  {
    // 駒損をして，ループする権利を相手に与える
    NumEffectState state(CsaString(
				   "P1-GI-KE-KI * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU-FU *  *  *  * -KY\n"
				   "P4 * -KY *  * +FU *  * -UM-FU\n"
				   "P5 *  *  *  *  * -HI-FU *  * \n"
				   "P6 *  * +KE *  * +KY *  * +KE\n"
				   "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
				   "P8 * +KA+KI * +OU *  * +HI * \n"
				   "P9+KY * +GI *  * +KI+GI *  * \n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(5,4),Square(5,3),PPAWN,PTYPE_EMPTY,true,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,0,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,2),Square(5,3),SILVER,PPAWN,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,1,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(5,4),PAWN,BLACK);
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,2,m,-1,0)));
    }
  }
  {
    // 相手のパス後に元からいける所に行く
    NumEffectState state(CsaString(
				   "P1-GI-KE-KI * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU-FU *  *  *  * -KY\n"
				   "P4 * -KY *  * +FU+GI * -UM-FU\n"
				   "P5 *  *  *  *  * -HI-FU *  * \n"
				   "P6 *  * +KE *  * +KY *  * +KE\n"
				   "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
				   "P8 * +KA+KI * +OU *  * +HI * \n"
				   "P9+KY *  *  *  * +KI+GI *  * \n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(4,4),Square(4,3),PSILVER,PTYPE_EMPTY,true,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,3,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m=Move::PASS(WHITE);
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(4,3),Square(5,3),PSILVER,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
    }
  }
  {
    // 敵味方往復中に，元からいけるところに一手はさむ
    NumEffectState state(CsaString(
				   "P1-GI-KE-KI * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU *  * -FU *  *  *  * -KY\n"
				   "P4 * -KY-FU * +FU+GI * -UM-FU\n"
				   "P5 *  *  *  *  * -HI-FU *  * \n"
				   "P6 *  * +KE *  * +KY *  * +KE\n"
				   "P7+FU+FU+FU+FU * +FU+FU+FU+FU\n"
				   "P8 * +KA+KI * +OU *  * +HI * \n"
				   "P9+KY *  *  *  * +KI+GI *  * \n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(4,4),Square(3,3),SILVER,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,0,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,2),Square(7,3),SILVER,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,1,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(5,4),Square(5,3),PPAWN,PTYPE_EMPTY,true,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,2,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(7,3),Square(6,2),SILVER,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,3,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(3,3),Square(4,4),SILVER,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
    }
  }
  {
    // 歩を交換して元からいけるところにいく
    NumEffectState state(CsaString(
				   "P1-GI-KE-KI * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU-FU *  *  *  * -KY\n"
				   "P4 * -KY *  * +FU *  * -UM-FU\n"
				   "P5 *  *  *  *  * -HI-FU *  * \n"
				   "P6 *  * +KE *  * +KY *  * +KE\n"
				   "P7+FU+FU+FU *  * +FU+FU+FU+FU\n"
				   "P8 * +KA+KI+HI+OU *  *  *  * \n"
				   "P9+KY * +GI *  * +KI+GI *  * \n"
				   "P+00FU\n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(6,4),PAWN,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,0,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,3),Square(6,4),PAWN,PAWN,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,1,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,8),Square(6,4),ROOK,PAWN,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,2,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,3),PAWN,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,3,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,4),Square(6,5),ROOK,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
    }
  }
  {
    // 元々一手でいけないところに行くのは可
    NumEffectState state(CsaString(
				   "P1-GI-KE-KI * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU-FU *  *  *  * -KY\n"
				   "P4 * -KY *  *  *  *  * -UM-FU\n"
				   "P5 *  *  * +FU * -HI-FU *  * \n"
				   "P6 *  * +KE+HI * +KY *  * +KE\n"
				   "P7+FU+FU+FU * +FU+FU+FU+FU+FU\n"
				   "P8 * +KA+KI * +OU *  *  *  * \n"
				   "P9+KY * +GI *  * +KI+GI *  * \n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    history.push(Move());
    {
      Move m(Square(6,6),Square(5,6),ROOK,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,0,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(2,4),Square(2,3),PBISHOP,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,1,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(5,6),Square(5,4),ROOK,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,2,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(2,3),Square(2,4),PBISHOP,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((MoveStackRejections::probe<WHITE>(state,history,3,m,-1,0)));
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,3,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(5,4),Square(6,4),ROOK,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
    }
  }
  {
    // 角交換で派手だが元々いけるところ
    NumEffectState state(CsaString(
				   "P1-GI-KE-KI * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU-FU *  *  *  * -KY\n"
				   "P4 * -KY *  *  *  *  *  * -FU\n"
				   "P5 *  *  * +FU * -HI-FU *  * \n"
				   "P6 *  * +FU *  * +KY *  * +KE\n"
				   "P7+FU+FU *  * +FU+FU+FU+FU+FU\n"
				   "P8 *  * +KI+HI+OU *  *  *  * \n"
				   "P9+KY+KE+GI *  * +KI+GI *  * \n"
				   "P+00KA\n"
				   "P-00KA\n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(8,8),BISHOP,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,0,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(3,3),BISHOP,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,1,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(8,8),Square(7,7),BISHOP,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,2,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(3,3),Square(7,7),PBISHOP,BISHOP,true,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,3,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(8,9),Square(7,7),KNIGHT,PBISHOP,false,BLACK);
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
    }
  }
  {
    // progress2/01.csa
    NumEffectState state(CsaString(
				   "P1-KY+HI * +KA *  *  * -KE-KY\n"
				   "P2 *  * -KI-FU *  *  *  *  * \n"
				   "P3-KE-KI-OU * +UM-GI-FU+FU-FU\n"
				   "P4 * -GI *  * -FU *  * -FU * \n"
				   "P5-FU *  *  *  * +GI *  *  * \n"
				   "P6 * +FU-FU *  *  *  *  *  * \n"
				   "P7+FU *  *  *  *  * +FU * +FU\n"
				   "P8+KY+KI+KI *  *  *  *  *  * \n"
				   "P9+OU+KE *  *  *  *  * +KE+KY\n"
				   "P+00FU\n"
				   "P-00HI00GI00FU00FU00FU00FU00FU\n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    {
      Move m(Square(7,4),PAWN,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,0,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(7,3),Square(7,4),KING,PAWN,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,1,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,1),Square(7,2),PBISHOP,GOLD,true,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,2,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,3),ROOK,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,3,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(5,3),Square(6,3),PBISHOP,ROOK,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,4,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(7,4),Square(7,5),KING,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,5,m,1,1)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,3),Square(5,3),PBISHOP,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,6,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(7,5),Square(7,4),KING,PTYPE_EMPTY,false,WHITE);
      BOOST_CHECK((MoveStackRejections::probe<WHITE>(state,history,7,m,1,2)));
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,2,m,1,2)));
      state.makeMove(m);    
      history.push(m);
    }
  }
  {
    // 一手パスもrejectする? -> しない
    NumEffectState state(CsaString(
				   "P1-GI-KE-KI * -KE-TO-TO-TO-OU\n"
				   "P2 *  *  * -GI *  *  * -TO-KI\n"
				   "P3-FU * -FU-FU *  *  *  * -KY\n"
				   "P4 * -KY *  * +FU *  * -UM-FU\n"
				   "P5 *  *  *  *  * -HI-FU *  * \n"
				   "P6 *  * +KE *  * +KY *  * +KE\n"
				   "P7+FU+FU+FU *  * +FU+FU+FU+FU\n"
				   "P8 * +KA+KI+HI+OU *  *  *  * \n"
				   "P9+KY * +GI *  * +KI+GI *  * \n"
				   "P+00FU\n"
				   "+\n").initialState());
    MoveStack history;
    history.push(Move());
    history.push(Move());
    {
      Move m(Square(6,4),PAWN,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,0,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,3),Square(6,4),PAWN,PAWN,false,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,1,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,8),Square(6,4),ROOK,PAWN,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,2,m,-1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,3),PAWN,WHITE);
      BOOST_CHECK((!MoveStackRejections::probe<WHITE>(state,history,3,m,1,0)));
      state.makeMove(m);    
      history.push(m);
    }
    {
      Move m(Square(6,4),Square(6,8),ROOK,PTYPE_EMPTY,false,BLACK);
      BOOST_CHECK((!MoveStackRejections::probe<BLACK>(state,history,4,m,1,0)));
      BOOST_CHECK((MoveStackRejections::probe<BLACK>(state,history,5,m,-1,0)));
    }
  }
}

BOOST_AUTO_TEST_CASE(MoveStackRejectionsTestBug20110212)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI * -OU-KI-GI-KE-KY\n"
			 "P2 *  *  *  * -HI *  * -KA * \n"
			 "P3-FU+FU-FU-FU-FU-FU-FU-FU-FU\n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 * +HI *  *  *  *  *  *  * \n"
			 "P7+FU * +FU+FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA *  *  *  *  *  *  * \n"
			 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			 "P+00KI00FU\n"
			 "+\n"
			 ).initialState());
  MoveStack history;
  CArray<Move,4> moves = {{
      Move(Square(8,3),Square(8,2),PPAWN,PTYPE_EMPTY,true,BLACK), // +8382TO

      Move(Square(7,1),Square(8,2),SILVER,PPAWN,false,WHITE), // -7182GI
      Move(Square(8,4),PAWN,BLACK),			      // +0084FU
      Move::PASS(WHITE),				      // pass
    }};
  for (Move move: moves) {
    history.push(move);
    state.makeMove(move);
  }
  Move m83to(Square(8,4),Square(8,3),PPAWN,PTYPE_EMPTY,true,BLACK);
  for (int i=-1024; i<=1024; ++i)
    BOOST_CHECK(! MoveStackRejections::probe<BLACK>(state,history,4,m83to,0,i));
  
}


static
bool isRejectMove(NumEffectState const& state,MoveStack const& history,Move move,
		  std::vector<HashKey> const& record,std::vector<std::vector<HashKey> > const& branches,
		  RepetitionCounter const& repCounter,
		  int alpha,bool showDebug=false)
{
  Player player=state.turn();
  assert(player == move.player());
  assert(!record.empty());
  HashKey h=record[record.size()-1].newMakeMove(move);
  HashKey h_r(h);
  h_r.changeTurn();
  assert(record[record.size()-1]==HashKey(state));
  PieceStand new_stand=h.blackStand();
  /**
   * 一手パス，および一手パス+駒損
   * recで自分の手番の局面に戻るもので，駒損か駒の損得なし
   * alphaの値は関係なし
   */
  for(int i=(int)record.size()-3; i>=0; i-=2){
    if(showDebug) std::cerr << "i=" << i << std::endl;
    const HashKey& old_h=record[i];
    PieceStand old_stand=old_h.blackStand();
    assert(old_h.turn()==player);
    if(old_h.isSameBoard(h_r) &&
       (old_stand==new_stand||
	old_stand.hasMoreThan(player,new_stand))){
      if(showDebug) std::cerr << "type 1" << std::endl;
      return true;
    }
  }
  /**
   * ループ(千日手)，およびループ+駒損
   * recで相手の手番の局面に戻るもの
   * 駒損か駒の損得なし 
   * ただし，alphaが負で手番で黒, alphaが正で手番が白の時は，千日手歓迎
   */
  for(int i=(int)record.size()-2; i>=0; i-=2){
    if(showDebug) std::cerr << "i=" << i << std::endl;
    const HashKey& old_h=record[i];
    PieceStand old_stand=old_h.blackStand();
    bool rejectSennichite=(player==BLACK ? alpha>=0 : alpha<=0);
    int j=(int)record.size()-1-i;
    // ? op(nocheck) my ? op(check) my ? op(check)  in record
    // myMove
    // の時は，repCounterは2
    // j=1,checkCount=1 - not reject
    // j=3,checkCount=2 - not reject
    // j=5,checkCount=2 - reject
    if(j<repCounter.checkCount(alt(player))*2){
      rejectSennichite=false;
    }
    assert(old_h.turn()==alt(player));
    if(old_h.isSameBoard(h)){
      if(old_stand==new_stand){
	if(rejectSennichite){
	  if(showDebug) std::cerr << "type 2.1" << std::endl;
	  return true;
	}
      }
      else if(old_stand.hasMoreThan(player,new_stand)){
	if(showDebug) std::cerr << "type 2.2" << std::endl;
	return true;
      }
    }
  }
  /**
   * もっと先祖から一手で行ける局面より駒損か駒の損得なし
   * 駒の損得なしで，その時のmoveが今からやろうとしているmoveと
   * 一致している時は，千日手歓迎モードではrejectせず．
   */
  for(int i=(int)record.size()-3; i>=0; i-=2){
    if(showDebug) std::cerr << "i=" << i << std::endl;
    for(int j=0;j<(int)branches[i].size();j++){
      const HashKey& old_h=branches[i][j];
      PieceStand old_stand=old_h.blackStand();
#if 0
      std::cerr << "i=" << i << ",j=" << j << ",old_h.turn()=" << old_h.turn() << ",player=" << player << std::endl;
      if(old_h.turn()!=alt(player)){
	for(int k=0;k<(int)record.size();k++){
	  std::cerr << "record[" << k << "].turn()=" << record[k].turn() << std::endl;
	  if(branches[k].size()>0)
	    std::cerr << "branches[" << k << "][0].turn()=" << branches[k][0].turn() << std::endl;
	}
      }
#endif
      assert(old_h.turn()==alt(player));
      if(old_h.isSameBoard(h)){
	if(old_stand==new_stand){
	  //	  int d=(int)record.size()-1-i;
	  //	  Move branchMove=history.lastMove(d);
	  //	  if(showDebug) std::cerr << "branchMove=" << branchMove << ",move=" << move << std::endl;
	  //	  assert(branchMove.player()==player);
	  if(old_h!=record[i+1]){
	    if(showDebug) std::cerr << "type 3.1" << std::endl;
	    return true;
	  }
	  else{
	    bool rejectSennichite=(player==BLACK ? alpha>=0 : alpha<=0);
	    int k=(int)record.size()-1-(i+1);
	    // ? op(nocheck) ? my op(check) ? my  op(check)  in record
	    // myMove
	    // の時は，repCounterは2
	    // j=1,checkCount=1 - not reject
	    // j=3,checkCount=2 - not reject
	    // j=5,checkCount=2 - reject
	    if(k<repCounter.checkCount(alt(player))*2){
	      rejectSennichite=false;
	    }
	    if(rejectSennichite){
	      if(showDebug) std::cerr << "type 3.2" << std::endl;
	      return true;
	    }
	  }
	}
	else if(old_stand.hasMoreThan(player,new_stand)){
	  if(showDebug) std::cerr << "type 3.3" << std::endl;
	  return true;
	}
      }
    }
  }
  /**
   * 相手から一手でループか，駒損局面にいける．
   */
  {
    NumEffectState state1(state);
    state1.makeMove(move);
    MoveVector allMoves;
    {
      if(state1.inCheck())
	GenerateEscapeKing::generate(state1,allMoves);
      else{
	Store store(allMoves);
	AllMoves<Store>::
	  generate(state1.turn(),state1,store);
      }
      for (int i=(int)allMoves.size()-1; i>=0; --i)
	if (allMoves[i].hasIgnoredUnpromote())
	  allMoves.push_back(allMoves[i].unpromote());
    }
    for (Move move: allMoves){
      const HashKey& new_h=h.newMakeMove(move);
      PieceStand new_stand=new_h.blackStand();
      for(int i=(int)record.size()-3; i>=0; i-=2){
	const HashKey& old_h=record[i];
	PieceStand old_stand=old_h.blackStand();
	assert(old_h.turn()==player);
	assert(new_h.turn()==player);
	if(old_h.isSameBoard(new_h)){
	  if(old_stand==new_stand){
	    bool rejectSennichite=(player==BLACK ? alpha>=0 : alpha<=0);
	    int j=(int)record.size()-1-(i+1);
	    // ? op(nocheck) ? my op(check) ? my  op(check)  in record
	    // myMove
	    // の時は，repCounterは2
	    // j=1,checkCount=1 - not reject
	    // j=3,checkCount=2 - not reject
	    // j=5,checkCount=2 - reject
	    if(j<repCounter.checkCount(alt(player))*2){
	      rejectSennichite=false;
	    }
	    if(rejectSennichite){
	      if(showDebug) std::cerr << "type 4.1" << std::endl;
	      return true;
	    }
	  }
	  else if(old_stand.hasMoreThan(player,new_stand)){
	    if(showDebug){
	      std::cerr << "old_stand=" << old_stand << ",new_stand=" << new_stand << std::endl;
	      std::cerr << "type 4.2,i=" << i << std::endl;
	    }
	    return true;
	  }
	}
      }
    }
  }
  return false;
}
template<Player P> static
void testState(NumEffectState const& state,MoveStack const& history,
	       std::vector<HashKey> const& rec,std::vector<std::vector<HashKey> > & branches,
	       RepetitionCounter const& repCounter,int alpha,bool update){
  HashKey hashKey=rec[rec.size()-1];
  std::vector<HashKey> this_branch;
  MoveVector allMoves;
  {
    if(state.inCheck())
      GenerateEscapeKing::generate(state,allMoves);
    else{
      Store store(allMoves);
      AllMoves<Store>::
	generate(state.turn(),state,store);
    }
    for (int i=(int)allMoves.size()-1; i>=0; --i)
      if (allMoves[i].hasIgnoredUnpromote())
	allMoves.push_back(allMoves[i].unpromote());
  }
  for (Move move: allMoves){
    this_branch.push_back(hashKey.newMakeMove(move));
    if(MoveStackRejections::probe<P>(state,history,std::max(1,(int)history.size()-3),move,alpha,repCounter.checkCount(alt(P)))){
      if(!isRejectMove(state,history,move,rec,branches,repCounter,alpha)){
	// 本来はassertで落とすべき
	std::cerr << "#### Error :\n";
	std::cerr << state << "\n";
	history.dump(std::cerr);
	std::cerr << "move=" << move << ",alpha=" << alpha << "\n";
	isRejectMove(state,history,move,rec,branches,repCounter,alpha,true);
	std::cerr << std::endl;
	BOOST_CHECK(0);
      }
    }
    else if(isRejectMove(state,history,move,rec,branches,repCounter,alpha,false)){
      std::cerr << "#### Warning :\n";
      std::cerr << state << "\n";
      history.dump(std::cerr);
      std::cerr << "move=" << move << ",alpha=" << alpha << "\n";
      isRejectMove(state,history,move,rec,branches,repCounter,alpha,true);
      std::cerr << std::endl;
    }
  }
  if(update)
    branches.push_back(this_branch);
}

static void testFile(const std::string& fileName)
{
  if(OslConfig::inUnitTestLong())
    std::cerr << "testFile(fileName=" << fileName << ")" << std::endl;
  NumEffectState state(CsaFileMinimal(fileName).load().initialState());
  const auto moves=CsaFileMinimal(fileName).load().moves;
  MoveStack history;
  std::vector<HashKey> record;
  std::vector<std::vector<HashKey> > branches;
  HashKey hash(state);
  RepetitionCounter repCounter(state);
  history.push(Move()); history.push(Move());
  for(unsigned int i=0;i<moves.size();i++){
    record.push_back(hash);
    { Player turn=state.turn();
      repCounter.push(hash,state);
      assert(turn==state.turn());
    }
    for(int alpha=-1;alpha<=1;alpha+=2){
      if(state.turn()==BLACK)
	testState<BLACK>(state,history,record,branches,repCounter,alpha,alpha==-1);
      else
	testState<WHITE>(state,history,record,branches,repCounter,alpha,alpha==-1);
    }
    Move move=moves[i];
    state.makeMove(move);
    hash=hash.newMakeMove(move);
    history.push(move);
  }
}

BOOST_AUTO_TEST_CASE(MoveStackRejectionsTestCsaFiles)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=3000;
  if (OslConfig::inUnitTestShort())
    count=10;
  std::string fileName;
  while ((ifs >> fileName) && (++i<count)) {
    if (fileName == "") 
      break;
    testFile(OslConfig::testCsaFile(fileName));
  }
}

BOOST_AUTO_TEST_CASE(MoveStackRejectionsTestBug20110215)
{
  {
    // public-domain/floodgate2010/wdoor+floodgate-900-0+gps_l+Bonanza+20100129073005.csa
    const char *str=
"P1-KY *  *  *  *  *  * -KE-KY\n"
"P2 *  *  *  *  *  * -KI-OU * \n"
"P3-FU *  * -GI * -FU-GI-FU-FU\n"
"P4 *  *  * -KI * -KA-FU *  * \n"
"P5 * -HI+FU-FU-FU *  *  * +FU\n"
"P6+FU+HI *  *  * +FU *  *  * \n"
"P7 * +FU * +FU+FU+GI+FU *  * \n"
"P8 * +KA+KI *  *  * +GI *  * \n"
"P9+KY *  *  *  * +KI+OU+KE+KY\n"
"P+00KE00FU\n"
"P-00KE00FU00FU\n"
"-\n"
"-8575HI\n"
"+0076FU\n"
"-0085FU\n"
"+7675FU\n"
"-8586FU\n"
"+0081HI\n"
"-0085HI\n"
      ;
    BOOST_CHECK(!testString(str));
  }
  {
    const char *str=
"P1 *  *  *  *  *  *  * -KE-KY\n"
"P2+RY-GI-KI *  *  *  *  *  * \n"
"P3-KY-FU-KE-GI * -KI *  * -FU\n"
"P4-KY-OU-FU * -FU *  * -FU * \n"
"P5 *  *  * +FU * -FU *  *  * \n"
"P6 * +FU+FU-KA+FU *  * +HI * \n"
"P7+FU+KI *  * -NK+FU+KE * +FU\n"
"P8+OU+GI *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  * +KY\n"
"P+00KA00GI00FU00FU00FU00FU00FU\n"
"P-00KI\n"
"-\n"
"-5767NK\n"
"+2629HI\n"
"-0078KI\n"
"+0099KA\n"
"-7888KI\n"
"+9988KA\n"
"-6688UM\n"
"+8788KI\n"
"-0066KA\n"
"+0087KI\n"
"-6688UM\n"
      ;
    BOOST_CHECK(testString(str));
  }
  //  testFile("/home/ktanaka/work/osl/public-domain/floodgate2010/wdoor+floodgate-900-0+Bonanza+gps_l+20100107000003.csa"); return;
  {
    const char *str=
"P1-KY-HI *  *  *  *  *  * -KY\n"
"P2 *  * -OU-KI *  * -KI *  * \n"
"P3 *  * -KE-GI * -FU-KE-FU-FU\n"
"P4-FU * -FU-FU-FU-GI *  *  * \n"
"P5 * -FU *  *  *  * -FU+FU+FU\n"
"P6+FU * +FU+FU+FU+FU+FU *  * \n"
"P7 * +FU+KE+KI * +GI+KE *  * \n"
"P8 * +OU+KI+GI *  *  * +HI * \n"
"P9+KY *  *  *  *  *  *  * +KY\n"
"P+00KA\n"
"P-00KA\n"
"+\n"
"+2829HI\n"
"-3536FU\n"
"+4736GI\n"
"-0058KA\n"
"+0047KA\n"
"-5847UM\n"
"+3647GI\n"
"-0035FU\n"
      ;
    BOOST_CHECK(testString(str));
  }
  {
    NumEffectState state(CsaString(
"P1-KY-KE * -OU *  *  *  * -KY\n"
"P2 *  *  * -HI *  * -KI-GI * \n"
"P3-FU *  * +UM-FU-FU-KE * -FU\n"
"P4 * +FU * -FU *  *  * -FU * \n"
"P5 *  *  *  *  *  * -FU *  * \n"
"P6 *  *  *  *  * +FU *  * +FU\n"
"P7+FU * -TO * +FU * +FU+FU * \n"
"P8 * -HI *  *  *  * +GI+OU * \n"
"P9+KY+KE * +KI * +KI * +KE+KY\n"
"P+00GI00FU\n"
"P-00KA00KI00GI00FU00FU\n"
"+\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,11> moves = {{
	Move(Square(6,3),Square(6,2),PBISHOP,ROOK,false,BLACK), //+6362UM
	Move(Square(6,1),Square(6,2),KING,PBISHOP,false,WHITE), //-6162OU
	Move(Square(8,2),ROOK,BLACK), //+0082HI
	Move(Square(7,2),SILVER,WHITE),//-0072GI
	Move(Square(8,3),SILVER,BLACK),//+0083GI
	Move(Square(6,3),BISHOP,WHITE),//-0063KA
	Move(Square(8,3),Square(7,2),PSILVER,SILVER,true,BLACK),//+8372NG
	Move(Square(6,3),Square(7,2),BISHOP,PSILVER,false,WHITE),//-6372KA
	Move(Square(8,3),SILVER,BLACK),//+0083GI
	Move(Square(7,1),SILVER,WHITE),//-0071GI
	Move(Square(8,3),Square(7,2),PSILVER,BISHOP,true,BLACK), // +8372NG
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m72gi(Square(7,1),Square(7,2),SILVER,PSILVER,false,WHITE);//-7172GI
    BOOST_CHECK(!MoveStackRejections::probe<WHITE>(state,history,11,m72gi,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1 *  *  *  *  *  *  *  *  * \n"
				   "P2 * +UM+GI * -OU-KI *  *  * \n"
				   "P3 * +NK * -KY *  *  * -FU * \n"
				   "P4 *  * -FU-GI-FU-FU-FU *  * \n"
				   "P5-GI-KY *  *  *  *  * +FU * \n"
				   "P6-KE-FU+FU+FU+FU+FU+FU+HI * \n"
				   "P7+GI *  * +KI+KA *  *  *  * \n"
				   "P8+OU * +KI *  * +KY *  * -RY\n"
				   "P9 *  *  * -KI *  *  * +KE * \n"
				   "P+00KE00KY00FU00FU00FU\n"
				   "P-00FU00FU00FU\n"
				   "+\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,5> moves = {{
	Move(Square(9,9),KNIGHT,BLACK), // +0099KE
	Move(Square(1,8),Square(1,5),PROOK,PTYPE_EMPTY,false,WHITE), // -1815RY
	Move(Square(2,6),Square(2,8),ROOK,PTYPE_EMPTY,false,BLACK), // +2628HI
	Move(Square(1,5),Square(1,9),PROOK,PTYPE_EMPTY,false,WHITE), // -1519RY
	Move(Square(2,8),Square(2,6),ROOK,PTYPE_EMPTY,false,BLACK), // +2826HI
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m18ry(Square(1,9),Square(1,8),PROOK,PTYPE_EMPTY,false,WHITE);
    BOOST_CHECK(MoveStackRejections::probe<WHITE>(state,history,5,m18ry,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1 *  *  *  *  *  *  *  *  * \n"
				   "P2 * +UM+GI * -OU-KI *  *  * \n"
				   "P3 * +NK * -KY *  *  * -FU * \n"
				   "P4 *  * -FU-GI-FU-FU-FU *  * \n"
				   "P5-GI-KY *  *  *  *  * +FU * \n"
				   "P6-KE-FU+FU+FU+FU+FU+FU+HI * \n"
				   "P7+GI *  * +KI+KA *  *  *  * \n"
				   "P8+OU * +KI *  * +KY *  * -RY\n"
				   "P9+KE *  * -KI *  *  * +KE * \n"
				   "P+00KY00FU00FU00FU\n"
				   "P-00FU00FU00FU\n"
				   "-\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,4> moves = {{
	Move(Square(1,8),Square(1,5),PROOK,PTYPE_EMPTY,false,WHITE), // -1815RY
	Move(Square(2,6),Square(2,8),ROOK,PTYPE_EMPTY,false,BLACK), // +2628HI
	Move(Square(1,5),Square(1,9),PROOK,PTYPE_EMPTY,false,WHITE), // -1519RY
	Move(Square(2,8),Square(2,6),ROOK,PTYPE_EMPTY,false,BLACK), // +2826HI
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m18ry(Square(1,9),Square(1,8),PROOK,PTYPE_EMPTY,false,WHITE);
    // root loop
    BOOST_CHECK(!MoveStackRejections::probe<WHITE>(state,history,4,m18ry,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1 * +GI *  *  *  *  *  *  * \n"
				   "P2 * +UM *  * -OU-KI *  *  * \n"
				   "P3 * +NK * -KY *  *  * -FU * \n"
				   "P4 *  * -FU-GI-FU-FU-FU *  * \n"
				   "P5-GI *  *  *  *  *  * +FU * \n"
				   "P6-KE-FU+FU+FU+FU+FU+FU+HI * \n"
				   "P7+GI *  * +KI+KA *  *  *  * \n"
				   "P8+OU * +KI *  * +KY *  * -RY\n"
				   "P9 *  *  * -KI *  *  * +KE * \n"
				   "P+00KE00KY00FU00FU00FU\n"
				   "P-00KY00FU00FU00FU\n"
				   "+\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,7> moves = {{
	Move(Square(8,1),Square(7,2),SILVER,PTYPE_EMPTY,false,BLACK), // +8172GI
	Move(Square(8,5),LANCE,WHITE), // -0085KY
	Move(Square(9,9),KNIGHT,BLACK), // +0099KE
	Move(Square(1,8),Square(1,5),PROOK,PTYPE_EMPTY,false,WHITE), // -1815RY
	Move(Square(2,6),Square(2,8),ROOK,PTYPE_EMPTY,false,BLACK), // +2628HI
	Move(Square(1,5),Square(1,9),PROOK,PTYPE_EMPTY,false,WHITE), // -1519RY
	Move(Square(2,8),Square(2,6),ROOK,PTYPE_EMPTY,false,BLACK), // +2826HI
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m18ry(Square(1,9),Square(1,8),PROOK,PTYPE_EMPTY,false,WHITE);
    BOOST_CHECK(MoveStackRejections::probe<WHITE>(state,history,7,m18ry,0,0));
  }

  {
    NumEffectState state(CsaString(
				   "P1 * -KE *  *  *  * -OU-KE * \n"
				   "P2 * -HI *  *  * -KA-KI *  * \n"
				   "P3 *  *  * +NY * -KI-GI-FU * \n"
				   "P4 *  * -FU * -FU-FU-FU *  * \n"
				   "P5-GI+KE *  *  *  *  * +FU+KY\n"
				   "P6 *  * +FU+FU+FU+FU+FU *  * \n"
				   "P7+KY+FU+GI+KI *  *  *  *  * \n"
				   "P8 * +OU+KI+KA *  *  * +HI * \n"
				   "P9 *  *  *  *  *  *  * +KE * \n"
				   "P+00KY00FU00FU00FU00FU00FU\n"
				   "P-00GI00FU\n"
				   "-\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,4> moves = {{
	Move(Square(9,6),PAWN,WHITE), // -0096FU
	Move(Square(9,8),PAWN,BLACK), // +0098FU
	Move(Square(9,6),Square(9,7),PPAWN,LANCE,true,WHITE), // -9697TO
	Move(Square(9,8),Square(9,7),PAWN,PPAWN,false,BLACK), // -9897FU
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m97um(Square(4,2),Square(9,7),PBISHOP,PAWN,true,WHITE);
    BOOST_CHECK(MoveStackRejections::probe<WHITE>(state,history,4,m97um,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1-KY-KE *  *  *  *  *  * -KY\n"
				   "P2 * +HI-KA-OU *  * -KI-GI * \n"
				   "P3-FU *  *  * -FU-FU-KE * -FU\n"
				   "P4 * +FU * -FU *  *  * -FU * \n"
				   "P5 *  *  *  *  *  * -FU *  * \n"
				   "P6 *  *  *  *  * +FU *  * +FU\n"
				   "P7+FU * -TO * +FU * +FU+FU * \n"
				   "P8 * -HI *  *  *  * +GI+OU * \n"
				   "P9+KY+KE * +KI * +KI * +KE+KY\n"
				   "P+00GI00FU\n"
				   "P-00KA00KI00GI00FU00FU\n"
				   "+\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,4> moves = {{
	Move(Square(8,3),SILVER,BLACK), // +0083GI
	Move(Square(7,1),SILVER,WHITE), // -0071GI
	Move(Square(8,3),Square(7,2),PSILVER,BISHOP,true,BLACK), // +8472NG
	Move(Square(7,1),Square(7,2),SILVER,PSILVER,false,WHITE), // +7172GI
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m72ry(Square(8,2),Square(7,2),PROOK,SILVER,true,BLACK);
    BOOST_CHECK(MoveStackRejections::probe<BLACK>(state,history,4,m72ry,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1-KY-KE-GI-KI-HI *  * -KE-KY\n"
				   "P2 *  * -OU *  *  * -KI *  * \n"
				   "P3 * -FU-FU-FU * -FU-FU-FU-FU\n"
				   "P4-FU-UM *  *  *  *  *  *  * \n"
				   "P5 *  *  * +KA *  * +HI *  * \n"
				   "P6 *  * +FU *  *  *  *  *  * \n"
				   "P7+FU+FU * +FU * +FU * +FU+FU\n"
				   "P8+KY+GI *  * +KI *  *  *  * \n"
				   "P9+OU+KE * +KI *  *  * +KE+KY\n"
				   "P+00FU00FU00GI\n"
				   "P-00GI00FU\n"
				   "+\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,2> moves = {{
	Move(Square(7,5),SILVER,BLACK), // +0075GI
	Move(Square(8,4),Square(9,3),PBISHOP,PTYPE_EMPTY,false,WHITE), // +8493UM
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m84gi(Square(7,5),Square(8,4),SILVER,PTYPE_EMPTY,false,BLACK);
    BOOST_CHECK(MoveStackRejections::probe<BLACK>(state,history,4,m84gi,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				   "P2 * -HI *  *  *  *  * -KA * \n"
				   "P3-FU-FU-FU-FU * -FU * -FU-FU\n"
				   "P4 *  *  *  *  *  * -FU *  * \n"
				   "P5 *  *  *  * -FU *  *  *  * \n"
				   "P6 *  * +FU *  *  *  *  *  * \n"
				   "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
				   "P8 * +KA * +OU+KI+GI * +HI * \n"
				   "P9+KY+KE+GI+KI *  *  * +KE+KY\n"
				   "-\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,2> moves = {{
	Move(Square(8,2),Square(5,2),ROOK,PTYPE_EMPTY,false,WHITE), // -8252HI
	Move(Square(6,8),Square(7,8),KING,PTYPE_EMPTY,false,BLACK), // +6878OU
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m82hi(Square(5,2),Square(8,2),ROOK,PTYPE_EMPTY,false,WHITE);
    BOOST_CHECK(MoveStackRejections::probe<WHITE>(state,history,4,m82hi,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1 *  *  *  *  *  * +KI+NY+NK\n"
				   "P2 * +TO * +UM *  *  *  *  * \n"
				   "P3 *  *  *  *  *  *  *  *  * \n"
				   "P4 *  * +GI *  *  *  *  * -FU\n"
				   "P5+FU+KI *  *  *  *  *  *  * \n"
				   "P6 *  *  *  *  * +FU *  *  * \n"
				   "P7 *  *  * +FU+FU+OU+FU * +FU\n"
				   "P8 * -UM+FU-FU+HI * +GI *  * \n"
				   "P9 * -OU-RY * +KE+GI *  * +KY\n"
				   "P+00KE00KY00KY00FU00FU00FU00FU00FU00FU00FU\n"
				   "P-00KI00KI00GI00KE00FU\n"
				   "+\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,4> moves = {{
	Move(Square(5,8),Square(4,8),ROOK,PTYPE_EMPTY,false,BLACK), // +5848HI
	Move(Square(7,9),Square(5,9),PROOK,KNIGHT,false,WHITE), // -7959RY
	Move(Square(4,8),Square(5,8),ROOK,PTYPE_EMPTY,false,BLACK), // +4858HI
	Move(Square(5,9),Square(7,9),PROOK,PTYPE_EMPTY,false,WHITE), // -5979RY
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m59ke(Square(5,9),KNIGHT,BLACK);
    BOOST_CHECK(MoveStackRejections::probe<BLACK>(state,history,4,m59ke,0,0));
  }
  {
    NumEffectState state(CsaString(
				   "P1-KY-HI *  *  *  *  * -KE-KY\n"
				   "P2 *  *  * -GI *  * -KI-OU * \n"
				   "P3 *  *  * -FU-KA-KI * -FU * \n"
				   "P4 * -FU-FU * -FU-FU *  * -FU\n"
				   "P5-FU-KE * +FU *  * +KA *  * \n"
				   "P6 * +GI+FU * +FU * -GI *  * \n"
				   "P7+FU+FU * +KI * +FU * +FU+KY\n"
				   "P8 * +OU+KI *  *  *  *  * +HI\n"
				   "P9+KY+KE *  *  *  *  * +KE * \n"
				   "P+00GI00FU\n"
				   "P-00FU00FU\n"
				   "+\n"
				   ).initialState());
    MoveStack history;
    CArray<Move,4> moves = {{
	Move(Square(1,7),Square(1,4),LANCE,PAWN,false,BLACK), // +1714KY
	Move(Square(1,1),Square(1,4),LANCE,LANCE,false,WHITE), // -1114KY
	Move(Square(1,8),Square(1,4),ROOK,LANCE,false,BLACK), // +1814HI
	Move(Square(1,1),LANCE,WHITE),			      // +0011KY
      }};
    for (Move move: moves) {
      history.push(move);
      state.makeMove(move);
    }
    Move m17ky(Square(1,7),LANCE,BLACK);
    BOOST_CHECK(! MoveStackRejections::probe<BLACK>(state,history,4,m17ky,0,0));
  }
}
BOOST_AUTO_TEST_CASE(MoveStackRejectionsTestBug20110217)
{
  //  testFile("/home/ktanaka/work/osl/public-domain/floodgate2010/wdoor+floodgate-10800-60+Bonanza_W3680_180_59+YssL980X_6c+20100811050008.csa"); return;
  {
    // wdoor+floodgate-10800-60+Bonanza_W3680_180_59+YssL980X_6c+20100811050008.csa
    // alpha値が-100なので千日手歓迎
    const char *str=
"P1 *  * +TO *  *  *  *  * +UM\n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3-FU * +TO+TO *  * +NY *  * \n"
"P4 *  *  *  * +KE *  *  * +UM\n"
"P5 *  *  * +RY *  *  *  *  * \n"
"P6 * +FU-FU+GI * -GI *  *  * \n"
"P7+FU *  * +GI-KI-NG *  *  * \n"
"P8 * +KY+OU+KI * -TO * -KI * \n"
"P9+KY+KE * +FU *  * -RY-OU * \n"
"P+00KI00KE00KE00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
"P-00KY\n"
"+\n"
"+0079KI\n"
"-5768KI\n"
"+7968KI\n"
"-0057KI\n"
"+0079KI\n"
      ;
    BOOST_CHECK(!testString(str,-100));
  }
  {
    // wdoor+floodgate-900-0+d01+gps_l+20110217190000
    const char *str=
"P1-KY-KE *  * -FU *  *  *  * \n"
"P2 * +GI *  *  * -OU * -KA+HI\n"
"P3-FU *  * -FU+FU-KI * +FU * \n"
"P4 * -FU+GI-KE *  * -FU * -FU\n"
"P5 *  * +OU *  * -GI *  *  * \n"
"P6 *  *  *  *  * -KE+FU *  * \n"
"P7+FU+FU-UM *  *  * -GI-FU+FU\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9+KY *  *  *  * +HI *  * +KY\n"
"P+00KI00KI00KI00KE00FU00FU00FU00FU00FU\n"
"P-00KY\n"
"-\n"
"-7776UM\n"
"+7584OU\n"
"-7694UM\n"
"+8475OU\n"
"-9476UM\n"
"+7584OU\n"
      ;
    BOOST_CHECK(!testString(str,-100));
  }
}
BOOST_AUTO_TEST_CASE(MoveStackRejectionsTestCheckLoop)
{
  {
    // wdoor+floodgate-900-0+d01+gps_l+20110217190000
    const char *str=
"P1-KY-KE *  * -FU *  *  *  * \n"
"P2 * +GI *  *  * -OU * -KA+HI\n"
"P3-FU *  * -FU+FU-KI * +FU * \n"
"P4 * -FU+GI-KE *  * -FU * -FU\n"
"P5 *  * +OU *  * -GI *  *  * \n"
"P6 *  *  *  *  * -KE+FU *  * \n"
"P7+FU+FU-UM *  *  * -GI-FU+FU\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9+KY *  *  *  * +HI *  * +KY\n"
"P+00KI00KI00KI00KE00FU00FU00FU00FU00FU\n"
"P-00KY\n"
"-\n"
"-7776UM\n"
"+7584OU\n"
"-7694UM\n"
"+8475OU\n"
"-9476UM\n"
"+7584OU\n"
      ;
    BOOST_CHECK(!testString(str,+100));
  }
  {
    // wdoor+floodgate-900-0+d01+gps_l+20110217190000
    // check side
    const char *str=
"P1-KY-KE *  * -FU *  *  *  * \n"
"P2 * +GI *  *  * -OU * -KA+HI\n"
"P3-FU *  * -FU+FU-KI * +FU * \n"
"P4 * -FU+GI-KE *  * -FU * -FU\n"
"P5 *  * +OU *  * -GI *  *  * \n"
"P6 *  *  *  *  * -KE+FU *  * \n"
"P7+FU+FU-UM *  *  * -GI-FU+FU\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9+KY *  *  *  * +HI *  * +KY\n"
"P+00KI00KI00KI00KE00FU00FU00FU00FU00FU\n"
"P-00KY\n"
"-\n"
"-7776UM\n"
"+7584OU\n"
"-7694UM\n"
"+8475OU\n"
"-9476UM\n"
      ;
    BOOST_CHECK(testString(str,-100));
  }
  {
    // wdoor+floodgate-900-0+d01+gps_l+20110217190000
    const char *str=
"P1-KY-KE *  * -FU *  *  *  * \n"
"P2 * +GI *  *  * -OU * -KA+HI\n"
"P3-FU *  * -FU+FU-KI * +FU * \n"
"P4 * -FU+GI-KE *  * -FU * -FU\n"
"P5 *  * +OU *  * -GI *  *  * \n"
"P6 *  *  *  *  * -KE+FU *  * \n"
"P7+FU+FU-UM *  *  * -GI-FU+FU\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9+KY *  *  *  * +HI *  * +KY\n"
"P+00KI00KI00KI00KE00FU00FU00FU00FU00FU\n"
"P-00KY\n"
"-\n"
"-7776UM\n"
"+7584OU\n"
"-7694UM\n"
"+8475OU\n"
"-9476UM\n"
      ;
    // 本来はalpha値にかかわらずtrueになってほしいが，できていない．
    //    BOOST_CHECK(testString(str,100));
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
