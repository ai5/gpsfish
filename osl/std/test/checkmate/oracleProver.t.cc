/* oracleProverTest.cc
 */
#include "osl/checkmate/dualDfpn.h"
#include "osl/checkmate/dfpn.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
using namespace osl;
using namespace osl::checkmate;

const size_t limit = 400000;
static bool isCheckmate(const char *org, const char *similar)
{
  NumEffectState state(CsaString(org).initialState());

  DualDfpn checkmate_search(limit);
  checkmate_search.setVerbose(OslConfig::verbose());
  Move check_move;
  PathEncoding path(state.turn());
  const bool win = checkmate_search.isWinningState(limit, state, HashKey(state),
					  path, check_move);
  BOOST_CHECK(win);

  NumEffectState state2(CsaString(similar).initialState());
  Move best_move;
  const bool result = checkmate_search.isWinningState
    (0, state2, HashKey(state2), path, best_move);

  return result;
}

static bool isCheckmateRaw(const char *org, const char *similar)
{
  NumEffectState state(CsaString(org).initialState());

  DfpnTable table;
  Dfpn dfpn;
  dfpn.setTable(&table);
  Move check_move;
  PathEncoding path(state.turn());
  const ProofDisproof win = dfpn.hasCheckmateMove
    (state, HashKey(state), path, limit, check_move);
  BOOST_CHECK(win.isCheckmateSuccess());

  NumEffectState state2(CsaString(similar).initialState());
  Move best_move;
  Dfpn::ProofOracle oracle(HashKey(state), PieceStand(WHITE, state));
  const ProofDisproof result = dfpn.tryProof
    (state2, HashKey(state2), path, oracle, 1, best_move);

  return result.isCheckmateSuccess();
}

BOOST_AUTO_TEST_CASE(OracleProverTestCheckLong)
{
  BOOST_CHECK(isCheckmateRaw(
		   "P1-KY-KE+GI *  *  *  *  * -KY\n"
		   "P2-OU-KE-GI * -KI *  *  *  * \n"
		   "P3 * -FU *  *  *  * +TO * -FU\n"
		   "P4+FU * -FU-GI *  *  *  *  * \n"
		   "P5 *  *  *  *  *  * -KA *  * \n"
		   "P6 *  * +FU+FU-FU *  *  *  * \n"
		   "P7+KE+FU+KA+KI *  *  *  * +FU\n"
		   "P8 *  * +OU *  * +KI *  *  * \n"
		   "P9+KY *  *  * +FU *  * +KE+KY\n"
		   "P+00FU00FU00FU00FU00FU00GI00HI\n"
		   "P-00FU00FU00KI00HI\n"
		   "+\n",
		   "P1-KY-KE+GI *  *  *  *  * -KY\n"
		   "P2-OU-KE-GI * -KI *  *  *  * \n"
		   "P3 * -FU *  *  *  * +TO * -FU\n"
		   "P4+FU * -FU-GI *  *  *  *  * \n"
		   "P5 *  *  *  *  *  * -KA *  * \n"
		   "P6 *  * +FU+FU-FU *  *  *  * \n"
		   "P7+KE+FU+KA+KI *  *  *  * +FU\n"
		   "P8 *  * +OU * +FU+KI *  *  * \n"
		   "P9+KY *  *  *  *  *  * +KE+KY\n"
		   "P+00FU00FU00FU00FU00FU00GI00HI\n"
		   "P-00FU00FU00KI00HI\n"
		   "+\n"));
  BOOST_CHECK(isCheckmate(
		   "P1 *  *  *  * -KI *  *  * -KY\n"
		   "P2 * +NG+TO *  * -KI-OU *  * \n"
		   "P3 *  *  * -FU-FU-FU-KE+FU-FU\n"
		   "P4-FU *  *  * -KE * -KY-GI * \n"
		   "P5 *  *  * +UM *  *  *  *  * \n"
		   "P6+FU *  *  * +HI *  *  *  * \n"
		   "P7 * +FU * +FU+FU+FU+FU * +FU\n"
		   "P8 *  *  *  *  * +OU *  *  * \n"
		   "P9+KY+KE-TO *  * +KI * +GI+KY\n"
		   "P-00FU00FU00FU00KE00GI00KI00KA\n"
		   "P+00HI\n"
		   "-\n",
		   "P1 *  *  *  * -KI *  *  * -KY\n"
		   "P2 * +NG+TO *  * -KI-OU *  * \n"
		   "P3 *  *  * -FU-FU-FU-KE+FU-FU\n"
		   "P4-FU *  *  * -KE * -KY-GI * \n"
		   "P5 *  *  * +UM *  *  *  *  * \n"
		   "P6+FU *  *  * +HI *  *  *  * \n"
		   "P7 * +FU * +FU+FU+FU+FU * +FU\n"
		   "P8+KY *  *  *  * +OU *  *  * \n"
		   "P9 * +KE-TO *  * +KI * +GI+KY\n"
		   "P-00FU00FU00FU00KE00GI00KI00KA\n"
		   "P+00HI\n"
		   "-\n"));
  BOOST_CHECK(isCheckmate(
		   "P1 *  *  * -KI *  *  *  * -KY\n"
		   "P2 * +KI-GI *  *  *  *  *  * \n"
		   "P3+GI-FU-FU *  *  *  *  * -FU\n"
		   "P4-FU * -KE *  * -FU-FU-FU * \n"
		   "P5 *  * +KA *  *  *  *  *  * \n"
		   "P6 *  *  *  * -FU *  *  *  * \n"
		   "P7 * +FU+KE+FU * +KA-OU * +FU\n"
		   "P8 *  * +OU+GI-NG *  *  *  * \n"
		   "P9 *  * +KI *  * +KY *  * +KY\n"
		   "P-00FU00FU00FU00FU00FU00FU00KE00KI00KY00HI\n"
		   "P+00FU00KE00HI\n"
		   "+\n",
		   "P1 *  *  * -KI *  *  *  * -KY\n"
		   "P2 * +KI-GI *  *  *  *  *  * \n"
		   "P3+GI-FU-FU *  *  *  *  * -FU\n"
		   "P4-FU * -KE *  * -FU-FU-FU * \n"
		   "P5 *  * +KA *  *  *  *  *  * \n"
		   "P6 *  *  *  * -FU *  *  *  * \n"
		   "P7+FU * +KE+FU * +KA-OU * +FU\n"
		   "P8 *  * +OU+GI-NG *  *  *  * \n"
		   "P9 *  * +KI *  * +KY *  * +KY\n"
		   "P-00FU00FU00FU00FU00FU00FU00KE00KI00KY00HI\n"
		   "P+00FU00KE00HI\n"
		   "+\n"));
  BOOST_CHECK(isCheckmate(
		   "P1-KY *  * +TO-KI-OU *  *  * \n"
		   "P2 *  * +RY-GI *  * -KI *  * \n"
		   "P3-FU * -FU-FU *  *  * -GI-FU\n"
		   "P4 *  *  *  * -KE-FU *  *  * \n"
		   "P5 *  *  *  *  * -KE-FU *  * \n"
		   "P6 * +FU+FU *  *  *  *  * +FU\n"
		   "P7+FU+OU * +FU * +FU+GI *  * \n"
		   "P8 *  *  *  * -RY *  *  *  * \n"
		   "P9+KY+KE+KE *  *  * -UM * +KY\n"
		   "P+00FU00FU00FU00GI00KY00KA\n"
		   "P-00FU00FU00KI00KI\n"
		   "-\n",
		   "P1-KY *  * +TO-KI-OU *  *  * \n"
		   "P2 *  * +RY-GI * -KI *  *  * \n"
		   "P3-FU * -FU-FU *  *  * -GI-FU\n"
		   "P4 *  *  *  * -KE-FU *  *  * \n"
		   "P5 *  *  *  *  * -KE-FU *  * \n"
		   "P6 * +FU+FU *  *  *  *  * +FU\n"
		   "P7+FU+OU * +FU * +FU+GI *  * \n"
		   "P8 *  *  *  * -RY *  *  *  * \n"
		   "P9+KY+KE+KE *  *  * -UM * +KY\n"
		   "P+00FU00FU00FU00GI00KY00KA\n"
		   "P-00FU00FU00KI00KI\n"
		   "-\n"));
  BOOST_CHECK(! isCheckmate(
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -KA-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P+00KE\n"
		   "P-00AL\n"
		   "+\n",
		   "P1 *  *  *  *  *  *  * -KE-OU\n"
		   "P2 *  *  *  *  *  *  * -FU-KY\n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 *  * +OU *  *  *  *  *  * \n"
		   "P+00KE\n"
		   "P-00AL\n"
		   "+\n"));
  BOOST_CHECK(isCheckmate(
		   "P1-KY * -OU-KI *  *  *  * -KY\n"
		   "P2 *  *  *  *  *  * -KI *  * \n"
		   "P3-FU-FU+NK * -FU-FU *  *  * \n"
		   "P4 *  *  *  *  * -HI *  * -FU\n"
		   "P5+FU+FU * -GI * -KE-KE *  * \n"
		   "P6+HI *  *  * +FU+KI * -FU+FU\n"
		   "P7 *  * -FU+FU+KA * +FU *  * \n"
		   "P8 *  * +KI *  *  *  * +FU * \n"
		   "P9+KY *  * +OU *  *  * +KE+KY\n"
		   "P-00FU00FU00FU00GI00GI\n"
		   "P+00FU00GI00KA\n"
		   "-\n",
		   "P1-KY * -OU-KI *  *  *  * -KY\n"
		   "P2 *  *  *  *  *  * -KI *  * \n"
		   "P3-FU-FU+NK * -FU-FU *  *  * \n"
		   "P4 *  *  *  *  * -HI *  * -FU\n"
		   "P5+FU+FU * -GI * -KE-KE *  * \n"
		   "P6+HI *  *  * +FU+KI * -FU+FU\n"
		   "P7 *  * -FU+FU+KA * +FU *  * \n"
		   "P8 *  * +KI *  *  *  * +FU * \n"
		   "P9+KY *  * +OU *  *  * +KE+KY\n"
		   "P-00FU00FU00FU00GI00GI\n"
		   "P+00FU00GI00KA\n"
		   "-\n"));
}

BOOST_AUTO_TEST_CASE(OracleProverTestAdjustMove)
{
  const Move m1 = 
    Move(Square(8,9),Square(7,9),KING,PTYPE_EMPTY,false,BLACK);
  Move m2 = m1.newCapture(PAWN);
  BOOST_CHECK(m1 != m2);
  BOOST_CHECK_EQUAL(m1.from(), m2.from());
  BOOST_CHECK_EQUAL(m1.to(), m2.to());
  BOOST_CHECK_EQUAL(m1.ptype(), m2.ptype());
  BOOST_CHECK_EQUAL(m1.promoteMask(), m2.promoteMask());
  BOOST_CHECK_EQUAL(m1.player(), m2.player());

  m2 = m2.newCapture(ROOK);
  BOOST_CHECK(m1 != m2);
  BOOST_CHECK_EQUAL(m1.from(), m2.from());
  BOOST_CHECK_EQUAL(m1.to(), m2.to());
  BOOST_CHECK_EQUAL(m1.ptype(), m2.ptype());
  BOOST_CHECK_EQUAL(m1.promoteMask(), m2.promoteMask());
  BOOST_CHECK_EQUAL(m1.player(), m2.player());

  m2 = m2.newCapture(PPAWN);
  BOOST_CHECK(m1 != m2);
  BOOST_CHECK_EQUAL(m1.from(), m2.from());
  BOOST_CHECK_EQUAL(m1.to(), m2.to());
  BOOST_CHECK_EQUAL(m1.ptype(), m2.ptype());
  BOOST_CHECK_EQUAL(m1.promoteMask(), m2.promoteMask());
  BOOST_CHECK_EQUAL(m1.player(), m2.player());

  m2 = m2.newCapture(PTYPE_EMPTY);
  BOOST_CHECK(m1 == m2);
  BOOST_CHECK_EQUAL(m1.from(), m2.from());
  BOOST_CHECK_EQUAL(m1.to(), m2.to());
  BOOST_CHECK_EQUAL(m1.ptype(), m2.ptype());
  BOOST_CHECK_EQUAL(m1.promoteMask(), m2.promoteMask());
  BOOST_CHECK_EQUAL(m1.player(), m2.player());

  // 取る駒が違っても詰む
  BOOST_CHECK(isCheckmateRaw(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE * +KE\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n",
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  * -KI * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  * +KE * +KE\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n"));
  
  // 初期位置が違っても詰む
  BOOST_CHECK(isCheckmate(
		   "P1 *  *  *  * +HI * -KI-OU * \n"
		   "P2 *  *  *  *  *  *  * -FU-FU\n"
		   "P3 *  *  *  *  *  * +FU *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 * +OU *  *  *  *  *  *  * \n"
		   "P-00AL\n"
		   "+\n",
		   "P1 *  *  * +HI *  * -KI-OU * \n"
		   "P2 *  *  *  *  *  *  * -FU-FU\n"
		   "P3 *  *  *  *  *  * +FU *  * \n"
		   "P4 *  *  *  *  *  *  *  *  * \n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 * +OU *  *  *  *  *  *  * \n"
		   "P-00AL\n"
		   "+\n"));
}

BOOST_AUTO_TEST_CASE(OracleProverTestNoCheck)
{
  // 玉が移動していると王手にならない
  BOOST_CHECK(! isCheckmate(
		   "P1 *  *  *  *  *  *  * -OU * \n"
		   "P2 *  *  *  *  *  *  *  *  * \n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  * +KE * +KE\n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 * +OU *  *  *  *  *  *  * \n"
		   "P-00AL\n"
		   "+\n",
		   "P1 *  * -OU *  *  *  *  *  * \n"
		   "P2 *  *  *  *  *  *  *  *  * \n"
		   "P3 *  *  *  *  *  *  *  *  * \n"
		   "P4 *  *  *  *  *  * +KE * +KE\n"
		   "P5 *  *  *  *  *  *  *  *  * \n"
		   "P6 *  *  *  *  *  *  *  *  * \n"
		   "P7 *  *  *  *  *  *  *  *  * \n"
		   "P8 *  *  *  *  *  *  *  *  * \n"
		   "P9 * +OU *  *  *  *  *  *  * \n"
		   "P-00AL\n"
		   "+\n"));
}

BOOST_AUTO_TEST_CASE(OracleProverTestCheck)
{
  BOOST_CHECK(isCheckmate(
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n",
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n"));
  // 類似局面も詰む
  BOOST_CHECK(isCheckmate(
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n",
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU+FU *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  * -FU *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU\n"
      "P-00KI00KY00FU\n"
      "P-00AL\n"
      "-\n"));

  // 駒が足りないと詰まない
  BOOST_CHECK(! isCheckmate(
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n",
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU+FU *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  * -FU *  *\n"
      "P8 *  *  *  *  *  * -KI *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU\n"
      "P-00KY00FU\n"
      "P-00AL\n"
      "-\n"));
  
  // 玉が動いても詰まない
  BOOST_CHECK(! isCheckmate(
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n",
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  * +OU *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n"));

  // 王手でも詰まない
  BOOST_CHECK(! isCheckmate(
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n",
      "P1+NY+TO *  *  * +KI-OU-KE-KY\n"
      "P2+FU *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6-KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM+GI *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU *  *  *  *  *  * -NG\n"
      "P+00HI00KE00KY00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "-\n"));
}

BOOST_AUTO_TEST_CASE(OracleProverTestPawnCheckmate)
{
  // 詰む局面は詰む 0088FU, 0098KY
  BOOST_CHECK(isCheckmate(
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * -KE-FU *  *  *  *  *\n"
    "P5 *  *  *  * +FU *  * +FU *\n"
    "P6 * -KE * +FU+GI-FU *  * +FU\n"
    "P7 * +FU-UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU+FU * -GI *  *  * -NG\n"
    "P+00KI00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
    "P-00KY00FU00FU\n"
    "P+00AL\n"
    "-\n",
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * -KE-FU *  *  *  *  *\n"
    "P5 *  *  *  * +FU *  * +FU *\n"
    "P6 * -KE * +FU+GI-FU *  * +FU\n"
    "P7 * +FU-UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU+FU * -GI *  *  * -NG\n"
    "P+00KI00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
    "P-00KY00FU00FU\n"
    "P+00AL\n"
    "-\n"));

  // 類似局面も詰む
  BOOST_CHECK(isCheckmate(
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * -KE-FU *  *  *  *  *\n"
    "P5 *  *  *  * +FU *  * +FU *\n"
    "P6 * -KE * +FU+GI-FU *  * +FU\n"
    "P7 * +FU-UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU+FU * -GI *  *  * -NG\n"
    "P+00KI00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
    "P-00KY00FU00FU\n"
    "P+00AL\n"
    "-\n",
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM+FU-KI-FU-FU\n"
    "P4 *  * -KE-FU *  *  *  *  *\n"
    "P5 *  *  *  * +FU *  * +FU *\n"
    "P6 * -KE * +FU+GI-FU *  * +FU\n"
    "P7 * +FU-UM *  *  * -FU *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU+FU * -GI *  *  * -NG\n"
    "P+00KI00HI00KI00KE00KY00FU00FU00FU00FU\n"
    "P-00KY00FU\n"
    "P+00AL\n"
    "-\n"));

  // 同じ手順では打歩詰になる
  BOOST_CHECK(! isCheckmate(
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * -KE-FU *  *  *  *  *\n"
    "P5 *  *  *  * +FU *  * +FU *\n"
    "P6 * -KE * +FU+GI-FU *  * +FU\n"
    "P7 * +FU-UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 * +OU+FU * -GI *  *  * -NG\n"
    "P+00KI00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
    "P-00KY00FU00FU\n"
    "P+00AL\n"
    "-\n",
    "P1+NY+TO *  *  *  * -OU-KE-KY\n"
    "P2 *  *  *  *  * -GI-KI *  *\n"
    "P3 * +RY *  * +UM * -KI-FU-FU\n"
    "P4 *  * -KE-FU *  *  *  *  *\n"
    "P5 *  *  *  * +FU *  * +FU *\n"
    "P6 * -KE * +FU+GI-FU *  * +FU\n"
    "P7 * +FU-UM *  *  *  *  *  *\n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9+FU+OU+FU * -GI *  *  * -NG\n"
    "P+00KI00HI00KI00KE00KY00FU00FU00FU00FU\n"
    "P-00KY00FU00FU\n"
    "P+00AL\n"
    "-\n"));
}

BOOST_AUTO_TEST_CASE(OracleProverTestPawnCheckmateLose)
{
  BOOST_CHECK(isCheckmate(
    "P1 *  *  *  *  *  *  * -KE * \n"
    "P2 *  *  *  *  *  *  * -OU * \n"
    "P3 *  *  *  *  *  *  *  * -FU\n"
    "P4 *  *  *  *  *  * +RY *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI00FU\n"
    "P-00AL\n"
    "+\n",
    "P1 *  *  *  *  *  *  * -KE * \n"
    "P2 *  *  *  *  *  *  * -OU * \n"
    "P3 *  *  *  *  *  *  *  * -FU\n"
    "P4 *  *  *  *  *  * +RY *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  * +OU *  *  *  *  * \n"
    "P+00KI00FU\n"
    "P-00AL\n"
    "+\n"));

  // 今度は逃げられないので打歩詰 (bug 33)
  BOOST_CHECK(! isCheckmate(
    "P1 *  *  *  *  *  *  * -KE * \n"
    "P2 *  *  *  *  *  *  * -OU * \n"
    "P3 *  *  *  *  *  *  *  * -FU\n"
    "P4 *  *  *  *  *  * +RY *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI00FU\n"
    "P-00AL\n"
    "+\n",
    "P1 *  *  *  *  *  *  * -KE-KY\n"
    "P2 *  *  *  *  *  *  * -OU-KE\n"
    "P3 *  *  *  *  *  *  *  * -FU\n"
    "P4 *  *  *  *  *  * +RY *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI00FU\n"
    "P-00AL\n"
    "+\n"));
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
