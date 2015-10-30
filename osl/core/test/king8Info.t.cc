/* king8Info.t.cc
 */
#include "osl/bits/king8Info.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(King8InfoTestDropCandidate)
{
  using misc::BitOp;
  {
    NumEffectState state(CsaString(
      "P1 *  *  *  * -OU *  *  *  * \n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3 *  *  *  * +FU *  *  *  * \n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  * -FU *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  *  *  * +OU *  *  *  * \n"
      "P+00KI\n"
      "P-00AL\n"
      "+\n").initialState());
    King8Info black=state.king8Info(BLACK);
    King8Info white=state.king8Info(WHITE);
    // count
    BOOST_CHECK_EQUAL(1, BitOp::countBit(black.dropCandidate()));
    BOOST_CHECK_EQUAL(1, BitOp::countBit(white.dropCandidate()));
    // direction
    BOOST_CHECK_EQUAL(1u<<U, black.dropCandidate());
    BOOST_CHECK_EQUAL(1u<<U, white.dropCandidate());
  }
  {
    NumEffectState state(CsaString(
      "P1 *  *  *  * -OU *  *  *  * \n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3 *  *  *  *  * +FU *  *  * \n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  *  *  * -FU *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  *  *  * +OU *  *  *  * \n"
      "P+00KI\n"
      "P-00AL\n"
      "+\n").initialState());
    King8Info black=state.king8Info(BLACK);
    King8Info white=state.king8Info(WHITE);
    // count
    BOOST_CHECK_EQUAL(1, BitOp::countBit(black.dropCandidate()));
    BOOST_CHECK_EQUAL(1, BitOp::countBit(white.dropCandidate()));
    // direction -- seems to be a view from king
    BOOST_CHECK_EQUAL(1u<<UR, black.dropCandidate());
    BOOST_CHECK_EQUAL(1u<<UL, white.dropCandidate());
  }
  {
    NumEffectState state(CsaString(
      "P1 *  *  *  * -OU *  *  *  * \n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3 *  *  * +FU *  *  *  *  * \n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  * -FU *  *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  *  *  * +OU *  *  *  * \n"
      "P+00KI\n"
      "P-00AL\n"
      "+\n").initialState());
    King8Info black=state.king8Info(BLACK);
    King8Info white=state.king8Info(WHITE);
    // direction
    BOOST_CHECK_EQUAL(1u<<UL, black.dropCandidate());
    BOOST_CHECK_EQUAL(1u<<UR, white.dropCandidate());
  }
  {
    NumEffectState state(CsaString(
      "P1 *  *  *  *  *  *  *  *  * \n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3 *  *  * +FU-OU *  *  *  * \n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7 *  *  * -FU+OU *  *  *  * \n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 *  *  *  *  *  *  *  *  * \n"
      "P+00KI\n"
      "P-00AL\n"
      "+\n").initialState());
    King8Info black=state.king8Info(BLACK);
    King8Info white=state.king8Info(WHITE);
    // direction
    BOOST_CHECK_EQUAL(1u<<DL, black.dropCandidate());
    BOOST_CHECK_EQUAL(1u<<DR, white.dropCandidate());
  }
}

BOOST_AUTO_TEST_CASE(King8InfoTestKing8Info)
{
  {
    NumEffectState state=CsaString(
      "P1-KY-KE+GI+KA *  * +RY * -KY\n"
      "P2 *  * -OU * -KI * +NK *  * \n"
      "P3-FU * -GI-FU-FU-FU *  * -FU\n"
      "P4 *  * -FU *  *  *  *  *  * \n"
      "P5 *  *  *  * +KA *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
      "P8 *  * -NK * +OU *  *  *  * \n"
      "P9+KY+KE * -HI * +KI+GI * +KY\n"
      "P+00FU00FU00FU\n"
      "P-00KI00KI00GI00FU00FU\n"
      "-\n"
      ).initialState();
    // +0027KE or +5528UM
    King8Info king8=King8Info::make(BLACK,state);
    BOOST_CHECK_EQUAL((1u<<D),king8.liberty());
    BOOST_CHECK_EQUAL(1u,king8.libertyCount());
  }
  {
    NumEffectState state=CsaString(
      "P1-KY * -GI-KI * +HI * -KE-KY\n"
      "P2 *  *  *  * -OU * +NK *  * \n"
      "P3-FU * -FU-FU-FU-FU-FU * -FU\n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 *  *  *  * -KA *  *  *  * \n"
      "P6 *  *  *  *  *  * +FU *  * \n"
      "P7+FU *  * +FU+FU+FU+GI * +FU\n"
      "P8 *  * -NK * +KI * +OU *  * \n"
      "P9+KY * -RY *  * -KA-GI+KE+KY\n"
      "P+00KI00KI00GI00FU00FU\n"
      "P-00FU00FU00FU\n"
      "+\n"
      ).initialState();
    // +0027KE or +5528UM
    King8Info king8=King8Info::make(WHITE,state);
    BOOST_CHECK_EQUAL((1u<<D),king8.liberty());
    BOOST_CHECK_EQUAL(1u,king8.libertyCount());
  }
  {
    NumEffectState state=CsaString(
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
    // +0027KE or +5528UM
    King8Info king8=King8Info::make(BLACK,state);
    BOOST_CHECK_EQUAL((1u<<DL),king8.liberty());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U)|(1u<<UR),
			  king8.dropCandidate());
    BOOST_CHECK_EQUAL(255u,king8.libertyCandidate());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U)|(1u<<UR),
			 king8.moveCandidate2());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U)|(1u<<UR),
			 king8.spaces());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U)|(1u<<UR),
			 king8.moves());
    BOOST_CHECK_EQUAL(1u,king8.libertyCount());
  }
  {
    NumEffectState state=CsaString(
      "P1+NY+TO *  *  *  * -OU-KE-KY\n"
      "P2 *  *  *  *  * -GI-KI *  *\n"
      "P3 * +RY *  * +UM * -KI-FU-FU\n"
      "P4 *  * +FU-FU *  *  *  *  *\n"
      "P5 *  * -KE * +FU *  * +FU *\n"
      "P6+KE *  * +FU+GI-FU *  * +FU\n"
      "P7 *  * -UM *  *  *  *  *  *\n"
      "P8 *  *  *  *  *  *  *  *  * \n"
      "P9 * +OU * -GI *  *  *  * -NG\n"
      "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
      "P-00KI00KY00FU00FU\n"
      "P-00AL\n"
      "+\n"
      ).initialState();
    King8Info king8=King8Info::make(BLACK,state);
    BOOST_CHECK_EQUAL((1u<<UL)|(1u<<R),
			 king8.liberty());
    BOOST_CHECK_EQUAL(0x00u,king8.dropCandidate());
    BOOST_CHECK_EQUAL((1u<<UL)|(1u<<R),
			 king8.libertyCandidate());
    BOOST_CHECK_EQUAL(0x00u,king8.moveCandidate2());
    BOOST_CHECK_EQUAL((1u<<UL)|(1u<<R),king8.spaces());
    BOOST_CHECK_EQUAL((1u<<UR),king8.moves());
    BOOST_CHECK_EQUAL((1u<<U),king8.libertyCount());
  }
  {
    NumEffectState state=CsaString(
      "P1 *  *  * -KI *  *  *  * -KY\n"
      "P2 * +KI-GI *  *  *  *  *  * \n"
      "P3+GI-FU-FU *  *  *  *  * -FU\n"
      "P4-FU * -KE *  * -FU-FU-FU * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  * -FU *  * +HI * \n"
      "P7 * +FU+KE+FU * +KA-OU * +FU\n"
      "P8 *  * +OU+GI * +KA *  *  * \n"
      "P9 *  * +KI * -NG+KY *  * +KY\n"
      "P+00KE00FU\n"
      "P-00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
      "-\n"
      ).initialState();
    // +0027KE or +5528UM
    King8Info king8=King8Info::make(BLACK,state);
    BOOST_CHECK_EQUAL((1u<<R),king8.liberty());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U),
			 king8.dropCandidate());
    BOOST_CHECK_EQUAL(255u,king8.libertyCandidate());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U),
			 king8.moveCandidate2());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U),
			 king8.spaces());
    BOOST_CHECK_EQUAL((1u<<DR)|(1u<<D)|(1u<<L)|(1u<<UL)|(1u<<U),
			 king8.moves());
    BOOST_CHECK_EQUAL((1u<<UL),king8.libertyCount());
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

