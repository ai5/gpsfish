/* dualDfpn.t.cc
 */
#include "osl/checkmate/dualDfpn.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::checkmate;
const size_t limit = 400000;

BOOST_AUTO_TEST_CASE(DualDfpnTestNoCheckmate)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -OU *  *  *  * \n"
			   "P2 *  *  *  *  * +GI *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6-HI-HI-KA-KA *  *  *  * +KY\n"
			   "P7-FU-GI * +GI *  *  *  * +KY\n"
			   "P8 * -FU *  *  *  *  *  * +KY\n"
			   "P9+OU * +FU+FU+FU+FU+FU * +KY\n"
			   "P+00KI00KI00GI00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "P-00KI00KI00KE00KE00KE00KE\n"
			   "-\n").initialState());
    // 98金の一手詰ではあるけど
    DualDfpn checker(limit);
    Move check_move = Move::INVALID();
    const PathEncoding path(WHITE, 3);
    const bool win = checker.isWinningState<WHITE>
      (10, state, HashKey(state), path, check_move);
    BOOST_CHECK_EQUAL(false, win);
  }
}

BOOST_AUTO_TEST_CASE(DualDfpnTestDistance)
{
  // bug 35
  NumEffectState state(CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  * +FU * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState());
  DualDfpn checker(limit);
  if (OslConfig::verbose())
    checker.setVerbose();
  Move check_move = Move::INVALID();
  const int distance = 3;
  const PathEncoding path(BLACK, distance);
  const bool win = checker.isWinningState<BLACK>
    (10, state, HashKey(state), path, check_move);
  BOOST_CHECK_EQUAL(true, win);
  BOOST_CHECK_EQUAL(Move(Square(2,2),GOLD,BLACK), check_move);

  BOOST_CHECK_EQUAL(distance, checker.distance(BLACK, HashKey(state)));
}

BOOST_AUTO_TEST_CASE(DualDfpnTestPawnCheckmate)
{
  // 逃げられないので打歩詰 (bug 33)
  SimpleState stateNoEscape=CsaString(
    "P1 *  *  *  *  *  *  * -KE-KY\n"
    "P2 *  *  *  *  *  *  * -OU-KE\n"
    "P3 *  *  *  *  *  *  * +FU-FU\n"
    "P4 *  *  *  *  *  * +RY *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "-\n").initialState();
  NumEffectState hStateNoEscape(stateNoEscape);
  const Move last_move = Move(Square(2,3), PAWN, BLACK);
  DualDfpn checker(limit);
  checker.setVerbose(OslConfig::verbose());
  const PathEncoding path(WHITE);
  BOOST_CHECK_EQUAL(false, 
		       checker.isLosingState<WHITE>(500, hStateNoEscape, HashKey(hStateNoEscape),
						    path, last_move));
}

BOOST_AUTO_TEST_CASE(DualDfpnTestNoPromote)
{
  NumEffectState state(CsaString(
			 "P1 * -GI *  * -KI *  *  *  * \n"
			 "P2+FU * -OU * -FU *  *  *  * \n"
			 "P3-KE+FU-FU * -KE *  *  *  * \n"
			 "P4+KA *  * -GI *  *  *  *  * \n"
			 "P5 *  * +FU * +FU-GI *  *  * \n"
			 "P6 * -KI *  * +KA-HI *  *  * \n"
			 "P7+KI *  *  *  *  *  *  *  * \n"
			 "P8 *  * +KE *  *  *  *  *  * \n"
			 "P9 * +KY *  *  *  *  *  *  * \n"
			 "P+00FU00FU00FU\n"
			 "P-00HI00KI00GI00KE00KY00KY00KY00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
			 "+\n").initialState());
  // +8382FU
  DualDfpn checker(limit);
  Move checkmate_move;
  checker.setVerbose(OslConfig::verbose());
  const PathEncoding path(BLACK);
  BOOST_CHECK_EQUAL(true, 
		       checker.isWinningState(8000, state, HashKey(state),
					      path, checkmate_move));
  BOOST_CHECK_EQUAL(Move(Square(8,3),Square(8,2),PAWN,PTYPE_EMPTY,false,
			    BLACK),
		       checkmate_move);
}


BOOST_AUTO_TEST_CASE(DualDfpnTestPawnCheckmateDefense)
{
  NumEffectState state(CsaString(
    "P1+KA *  * -KI *  *  *  *  * \n"
    "P2 *  *  * -KI *  *  *  *  * \n"
    "P3 * -FU+KY-OU-UM+RY-FU-FU * \n"
    "P4-FU *  * -FU-FU *  *  *  * \n"
    "P5 * +FU-FU *  * -FU * +FU * \n"
    "P6+FU+OU+FU+FU+FU *  *  *  * \n"
    "P7 *  *  *  * +GI+FU+KE *  * \n"
    "P8 *  * +GI+KI *  *  *  *  * \n"
    "P9+KY *  * +KI *  *  *  *  * \n"
    "P+00GI00GI00KY00FU\n"
    "P-00HI00KE00KE00KE00KY00FU00FU\n"
    "-\n").initialState());
  DualDfpn checker(limit);
  const PathEncoding path(WHITE);
  Move check_move;
  checker.isWinningState(500, state, HashKey(state),
			 path, check_move);
// 動かすだけ 途中で攻め方を打ち歩詰めにする王手回避が出現
}

/**
 */
BOOST_AUTO_TEST_CASE(DualDfpnTestCheck)
{
  NumEffectState state(CsaString(
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * +FU-FU *  *  *  *  *\n"
    "P5 *  * -KE * +FU *  * +FU *\n"
    "P6-KE *  * +FU+GI-FU *  * +FU\n"
    "P7 *  * -UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU * -GI *  *  *  * -NG\n"
    "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
    "P-00KI00KY00FU00FU\n"
    "P-00AL\n"
    "+\n").initialState());

  state.makeMove(Move(Square(9,2),PAWN,BLACK));
  NumEffectState hState(state);

  {
    DualDfpn checker(limit);
    checker.setVerbose(OslConfig::verbose());
    Move checkmate_move = Move::INVALID();
    const PathEncoding path(WHITE);
    // 3 手詰なのですぐに詰めてほしいところ
    const bool win = checker.isWinningState<WHITE>
      (70, hState, HashKey(hState), path, checkmate_move);
    BOOST_CHECK(win);
    BOOST_CHECK(! checkmate_move.isPass());
  }
  
  {
    // 一手進めばすぐ詰む
    hState.makeMove(Move(Square(8,8),GOLD,WHITE));
    DualDfpn checker(limit);
    checker.setVerbose(OslConfig::verbose());
    const PathEncoding path(BLACK);
    const bool lose = checker.isLosingState<BLACK>(10, hState, HashKey(hState), path);
    BOOST_CHECK(lose);
  }
}

BOOST_AUTO_TEST_CASE(DualDfpnTestInfty)
{
  SimpleState state=CsaString(
"P1-KY * -KI-KE *  *  *  * +UM\n"
"P2 *  *  *  * -OU * -GI *  * \n"
"P3-FU * -KE-FU-FU-FU * -KA-FU\n"
"P4 * -HI-FU *  *  *  *  *  * \n"
"P5 *  * +KI *  * +FU *  *  * \n"
"P6 *  *  * +FU+KI *  *  *  * \n"
"P7+FU * +OU * +FU * +KI+GI+FU\n"
"P8-TO *  *  *  *  *  *  *  * \n"
"P9 *  *  *  *  *  *  *  * +KY\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P-00FU\n"
"P+00KE\n"
"P+00KE\n"
"P-00GI\n"
"P+00GI\n"
"P+00KY\n"
"P+00KY\n"
"P+00HI\n"
"-\n").initialState();
  NumEffectState hState(state);
  const PathEncoding path(WHITE);
  DualDfpn checker(limit);
  checker.setVerbose(OslConfig::verbose());
  Move checkmate_move = Move::INVALID();
  const bool win = checker.isWinningState<WHITE>
    (1000, hState, HashKey(hState), path, checkmate_move);
  BOOST_CHECK(win);
}

BOOST_AUTO_TEST_CASE(DualDfpnTestInfty2)
{
  SimpleState state=CsaString(
"P1-KY-KE *  *  * -OU * -KE+KA\n"
"P2 *  * +FU * -KI *  *  *  * \n"
"P3-FU *  * -FU-GI-KI * -FU-FU\n"
"P4 *  *  *  * -FU *  *  *  * \n"
"P5 * +GI * +FU * -FU *  *  * \n"
"P6 *  *  *  * +FU *  *  *  * \n"
"P7+FU+FU * +KI * +FU * +FU+FU\n"
"P8 * +OU-NY *  *  *  *  * -RY\n"
"P9+KY+KE * +KI *  * +FU *  * \n"
"P+00FU\n"
"P+00FU\n"
"P-00FU\n"
"P-00KE\n"
"P+00GI\n"
"P+00GI\n"
"P-00KY\n"
"P+00KA\n"
"P-00HI\n"
"+\n").initialState();
  
  NumEffectState hState((NumEffectState(state)));
  const PathEncoding path(BLACK);
  DualDfpn checker(limit);
  checker.setVerbose(OslConfig::verbose());
  const bool lose = checker.isLosingState<BLACK>(100, hState, HashKey(hState), path);
  BOOST_CHECK(! lose);
}

BOOST_AUTO_TEST_CASE(DualDfpnTestCopy)
{
  // should have different table on copy
  DualDfpn searcher(limit);
  DualDfpn searcher2(searcher);
  // BOOST_CHECK(&searcher.getTable(BLACK) != &searcher2.getTable(BLACK));
#if 0
  // currently not supported
  DualDfpn searcher3(limit);
  searcher3 = searcher;
#endif
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
