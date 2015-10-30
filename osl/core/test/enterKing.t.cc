#include "osl/enterKing.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::enter_king;

BOOST_AUTO_TEST_CASE(EnterKingTestBeginning)
{
  const NumEffectState state((SimpleState(HIRATE)));
  EnterKing Enter;
  BOOST_CHECK( !Enter.canDeclareWin(state) );
}

BOOST_AUTO_TEST_CASE(EnterKingTestBlackWin)
{
  const NumEffectState state(CsaString(
				       "P1 *  * +RY *  * +KI * -KE * \n"
				       "P2+OU+TO+UM+UM+TO * -KI-OU * \n"
				       "P3-FU+FU+FU+FU+FU-GI-GI *  * \n"
				       "P4 *  *  *  * -FU-FU-FU-FU * \n"
				       "P5 *  * -RY-NY-GI *  *  *  * \n"
				       "P6 *  *  *  *  * +FU+FU+FU * \n"
				       "P7 *  *  *  *  * +KI+KE * -NY\n"
				       "P8 *  *  *  *  * +KI+GI *  * \n"
				       "P9 *  *  *  * -NY *  *  *  * \n"
				       "P+00FU00FU00FU00FU00KY00KE00KE\n"
				       "+\n").initialState());
  EnterKing Enter;
  BOOST_CHECK( Enter.canDeclareWin(state) );
}

void EnterKingTestWhiteWin()
{
  const NumEffectState state(CsaString(
					"P1 *  *  *  * +NY *  *  *  * \n"
					"P2 *  *  *  *  * -KI-GI *  * \n"
					"P3 *  *  *  *  * -KI-KE * +NY\n"
					"P4 *  *  *  *  * -FU-FU-FU * \n"
					"P5 *  * +RY+NY+GI *  *  *  * \n"
					"P6 *  *  *  * +FU+FU+FU+FU * \n"
					"P7+FU-FU-FU-FU-FU+GI+GI *  * \n"
					"P8-OU-TO-UM-UM-TO * +KI+OU * \n"
					"P9 *  * -RY *  * -KI * +KE * \n"
					"P-00FU00FU00FU00FU00KY00KE00KE\n"
					"-\n").initialState());
  EnterKing Enter;
  BOOST_CHECK( Enter.canDeclareWin(state) );
}

BOOST_AUTO_TEST_CASE(EnterKingTestBlackLessPiece)
{
  const NumEffectState state(CsaString(
				       "P1 *  * +RY *  * +KI * -KE * \n"
				       "P2+OU+TO+UM+UM+TO * -KI-OU * \n"
				       "P3-FU+FU+FU+FU * -GI-GI *  * \n"
				       "P4 *  *  *  * -FU-FU-FU-FU * \n"
				       "P5 *  * -RY-NY-GI *  *  *  * \n"
				       "P6 *  *  *  *  * +FU+FU+FU * \n"
				       "P7 *  *  *  *  * +KI+KE * -NY\n"
				       "P8 *  *  *  *  * +KI+GI *  * \n"
				       "P9 *  *  *  * -NY *  *  *  * \n"
				       "P+00FU00FU00FU00FU00KY00KE00KE\n"
				       "P-00FU\n"
				       "+\n").initialState());
  EnterKing Enter;
  BOOST_CHECK( !Enter.canDeclareWin(state) );
}
BOOST_AUTO_TEST_CASE(EnterKingTestWhiteLessPiece)
{
  const NumEffectState state(CsaString(
					"P1 *  *  *  * +NY *  *  *  * \n"
					"P2 *  *  *  *  * -KI-GI *  * \n"
					"P3 *  *  *  *  * -KI-KE * +NY\n"
					"P4 *  *  *  *  * -FU-FU-FU * \n"
					"P5 *  * +RY+NY+GI *  *  *  * \n"
					"P6 *  *  *  * +FU+FU+FU+FU * \n"
					"P7+FU-FU-FU-FU * +GI+GI *  * \n"
					"P8-OU-TO-UM-UM-TO * +KI+OU * \n"
					"P9 *  * -RY *  * -KI * +KE * \n"
					"P+00FU\n"
					"P-00FU00FU00FU00FU00KY00KE00KE\n"
					"-\n").initialState());
  EnterKing Enter;
  BOOST_CHECK( !Enter.canDeclareWin(state) );
}
BOOST_AUTO_TEST_CASE(EnterKingTestBlackLessPoint)
{
  const NumEffectState state(CsaString(
				       "P1 *  * +RY *  * +KI * -KE * \n"
				       "P2+OU+TO+UM+TO+TO * -KI-OU * \n"
				       "P3-FU+FU+FU+FU+FU-GI-GI *  * \n"
				       "P4 *  *  *  * -FU-FU-FU-FU * \n"
				       "P5 *  * -RY-NY-GI *  *  *  * \n"
				       "P6 *  *  *  *  * +FU+FU+FU * \n"
				       "P7+UM *  *  *  * +KI+KE * -NY\n"
				       "P8 *  *  *  *  * +KI+GI *  * \n"
				       "P9 *  *  *  * -NY *  *  *  * \n"
				       "P+00FU00FU00FU00KY00KE00KE\n"
				       "+\n").initialState());
  EnterKing Enter;
  BOOST_CHECK( !Enter.canDeclareWin(state) );
}
BOOST_AUTO_TEST_CASE(EnterKingTestWhiteLessPoint)
{
  const NumEffectState state(CsaString(
					"P1 *  *  *  * +NY *  *  *  * \n"
					"P2 *  *  *  *  * -KI-GI *  * \n"
					"P3-UM *  *  *  * -KI-KE * +NY\n"
					"P4 *  *  *  *  * -FU-FU-FU * \n"
					"P5 *  * +RY+NY+GI *  *  *  * \n"
					"P6 *  *  *  * +FU+FU+FU+FU * \n"
					"P7+FU-FU-FU-FU-FU+GI+GI *  * \n"
					"P8-OU-TO-UM-TO-TO * +KI+OU * \n"
					"P9 *  * -RY *  * -KI * +KE * \n"
					"P-00FU00FU00FU00KY00KE00KE\n"
					"-\n").initialState());
  EnterKing Enter;
  BOOST_CHECK( !Enter.canDeclareWin(state) );
}

BOOST_AUTO_TEST_CASE(EnterKingTestDropsBlackWin)
{
  const NumEffectState state(CsaString(
				       "P1 *  * +RY *  * +KI * -KE * \n"
				       "P2+OU+TO+UM+UM+TO * -KI-OU * \n"
				       "P3-FU+FU+FU+FU+FU-GI-GI *  * \n"
				       "P4 *  *  *  * -FU-FU-FU-FU * \n"
				       "P5 *  * -RY-NY-GI *  *  *  * \n"
				       "P6 *  *  *  *  * +FU+FU+FU * \n"
				       "P7 *  *  *  *  * +KI+KE * -NY\n"
				       "P8 *  *  *  *  * +KI+GI *  * \n"
				       "P9 *  *  *  * -NY *  *  *  * \n"
				       "P+00FU00FU00FU00FU00KY00KE00KE\n"
				       "+\n").initialState());
  EnterKing Enter;
  int drops;
  bool kachi = Enter.canDeclareWin(state, drops);
  BOOST_CHECK(kachi);
  BOOST_CHECK(drops == 0);
}

BOOST_AUTO_TEST_CASE(EnterKingTestDropsWhiteWin)
{
  const NumEffectState state(CsaString(
					"P1 *  *  *  * +NY *  *  *  * \n"
					"P2 *  *  *  *  * -KI-GI *  * \n"
					"P3 *  *  *  *  * -KI-KE * +NY\n"
					"P4 *  *  *  *  * -FU-FU-FU * \n"
					"P5 *  * +RY+NY+GI *  *  *  * \n"
					"P6 *  *  *  * +FU+FU+FU+FU * \n"
					"P7+FU-FU-FU-FU-FU+GI+GI *  * \n"
					"P8-OU-TO-UM-UM-TO * +KI+OU * \n"
					"P9 *  * -RY *  * -KI * +KE * \n"
					"P-00FU00FU00FU00FU00KY00KE00KE\n"
					"-\n").initialState());
  EnterKing Enter;
  int drops;
  bool kachi = Enter.canDeclareWin(state, drops);
  BOOST_CHECK(kachi);
  BOOST_CHECK(drops == 0);
}

BOOST_AUTO_TEST_CASE(EnterKingTestDropsBlackLessPiece)
{
  const NumEffectState state(CsaString(
				       "P1 *  * +RY *  * +KI * -KE * \n"
				       "P2+OU+TO+UM+UM+TO * -KI-OU * \n"
				       "P3-FU+FU+FU+FU * -GI-GI *  * \n"
				       "P4 *  *  *  * -FU-FU-FU-FU * \n"
				       "P5 *  * -RY-NY-GI *  *  *  * \n"
				       "P6 *  *  *  *  * +FU+FU+FU * \n"
				       "P7 *  *  *  *  * +KI+KE * -NY\n"
				       "P8 *  *  *  *  * +KI+GI *  * \n"
				       "P9 *  *  *  * -NY *  *  *  * \n"
				       "P+00FU00FU00FU00FU00KY00KE00KE\n"
				       "P-00FU\n"
				       "+\n").initialState());
  EnterKing Enter;
  int drops;
  bool kachi = Enter.canDeclareWin(state, drops);
  BOOST_CHECK(!kachi);
  BOOST_CHECK(drops == 1);
}
BOOST_AUTO_TEST_CASE(EnterKingTestDropsWhiteLessPiece)
{
  const NumEffectState state(CsaString(
					"P1 *  *  *  * +NY *  *  *  * \n"
					"P2 *  *  *  *  * -KI-GI *  * \n"
					"P3 *  *  *  *  * -KI-KE * +NY\n"
					"P4 *  *  *  *  * -FU-FU-FU * \n"
					"P5 *  * +RY+NY+GI *  *  *  * \n"
					"P6 *  *  *  * +FU+FU+FU+FU * \n"
					"P7+FU-FU-FU-FU * +GI+GI *  * \n"
					"P8-OU-TO-UM-UM-TO * +KI+OU * \n"
					"P9 *  * -RY *  * -KI * +KE * \n"
					"P+00FU\n"
					"P-00FU00FU00FU00FU00KY00KE00KE\n"
					"-\n").initialState());
  EnterKing Enter;
  int drops;
  bool kachi = Enter.canDeclareWin(state, drops);
  BOOST_CHECK(!kachi);
  BOOST_CHECK(drops == 1);
}

BOOST_AUTO_TEST_CASE(EnterKingTestDropsBlackLessPoint)
{
  const NumEffectState state(CsaString(
				       "P1 *  * +RY *  * +KI * -KE * \n"
				       "P2+OU+TO+UM+TO+TO * -KI-OU * \n"
				       "P3-FU+FU+FU+FU+FU-GI-GI *  * \n"
				       "P4 *  *  *  * -FU-FU-FU-FU * \n"
				       "P5 *  * -RY-NY-GI *  *  *  * \n"
				       "P6 *  *  *  *  * +FU+FU+FU * \n"
				       "P7+UM *  *  *  * +KI+KE * -NY\n"
				       "P8 *  *  *  *  * +KI+GI *  * \n"
				       "P9 *  *  *  * -NY *  *  *  * \n"
				       "P+00FU00FU00FU00KY00KE00KE\n"
				       "+\n").initialState());
  EnterKing Enter;
  int drops;
  bool kachi = Enter.canDeclareWin(state, drops);
  BOOST_CHECK(!kachi);
  BOOST_CHECK(drops == 41);
}
BOOST_AUTO_TEST_CASE(EnterKingTestDropsWhiteLessPoint)
{
  const NumEffectState state(CsaString(
					"P1 *  *  *  * +NY *  *  *  * \n"
					"P2 *  *  *  *  * -KI-GI *  * \n"
					"P3-UM *  *  *  * -KI-KE * +NY\n"
					"P4 *  *  *  *  * -FU-FU-FU * \n"
					"P5 *  * +RY+NY+GI *  *  *  * \n"
					"P6 *  *  *  * +FU+FU+FU+FU * \n"
					"P7+FU-FU-FU-FU-FU+GI+GI *  * \n"
					"P8-OU-TO-UM-TO-TO * +KI+OU * \n"
					"P9 *  * -RY *  * -KI * +KE * \n"
					"P-00FU00FU00FU00KY00KE00KE\n"
					"-\n").initialState());
  EnterKing Enter;
  int drops;
  bool kachi = Enter.canDeclareWin(state, drops);
  BOOST_CHECK(!kachi);
  BOOST_CHECK(drops == 41);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
