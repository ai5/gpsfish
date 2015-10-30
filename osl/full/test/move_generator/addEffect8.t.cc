#include "osl/move_generator/addEffect8.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/additionalEffect.h"
#include "osl/oslConfig.h"

#include <boost/progress.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>


using namespace osl;
using namespace osl::move_generator;
using namespace osl::move_action;

static bool isAddEffect8Move(const NumEffectState& state_org,Move move);
static bool mayNotGeneratedMove(const NumEffectState& savedState,Move move);
BOOST_AUTO_TEST_CASE(AddEffect8TestOne)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-OU-KI *  *  *  * +RY\n"
			   "P2 *  *  * -KI *  *  *  *  * \n"
			   "P3 * +GI-FU-FU * -FU * -FU * \n"
			   "P4-FU *  *  * -FU+FU *  * +FU\n"
			   "P5 * +KE+KE-KA *  * -FU+FU * \n"
			   "P6+FU * -KY * +FU *  *  *  * \n"
			   "P7 *  *  * +FU+KA * +FU *  * \n"
			   "P8 *  *  * +GI *  *  *  *  * \n"
			   "P9+KY+OU *  * +KI *  *  * -RY\n"
			   "P+00KI00GI00KY\n"
			   "P-00GI00KE00FU00FU00FU00FU\n"
			   "-\n"
			   ).initialState());
    BOOST_CHECK(mayNotGeneratedMove(state,Move(Square(7,6),Square(7,8),LANCE,PTYPE_EMPTY,false,WHITE)));
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(state.turn(),state,store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(7,6),Square(7,8),LANCE,PTYPE_EMPTY,false,WHITE)) ||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  * -KE-KY\n"
			   "P2-HI *  *  *  *  * -KI-OU * \n"
			   "P3 *  * +UM-FU *  * -KI-FU * \n"
			   "P4-FU * -FU *  * -GI-FU * -FU\n"
			   "P5 *  *  *  * -FU-FU * +FU * \n"
			   "P6+FU * +FU+FU *  * +FU * +FU\n"
			   "P7 * +FU+KE+GI+GI+FU * -UM * \n"
			   "P8 *  * +KI * +OU * -KI *  * \n"
			   "P9+KY+HI *  *  *  *  *  * +KY\n"
			   "P+00GI00KE00KE00FU00FU\n"
			   "-\n"
			   ).initialState());
    BOOST_CHECK(mayNotGeneratedMove(state,Move(Square(3,8),Square(3,9),GOLD,PTYPE_EMPTY,false,WHITE)));
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(state.turn(),state,store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,8),Square(3,9),GOLD,PTYPE_EMPTY,false,WHITE)) ||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI * -KA-FU * +RY\n"
			   "P2 * +GI-GI * -FU *  *  *  * \n"
			   "P3 * -OU * -FU * -FU *  * -FU\n"
			   "P4-FU-RY+KE-KY *  *  *  *  * \n"
			   "P5 *  * +KI * +UM *  *  *  * \n"
			   "P6+FU * +FU *  *  *  *  *  * \n"
			   "P7 *  *  *  * +OU+FU+FU * +FU\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+KY *  *  *  * +KI+GI+KE+KY\n"
			   "P+00KI00GI00FU00FU00FU00FU\n"
			   "P-00KE00FU00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(state.turn(),state,store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(6,4),Square(6,7),LANCE,PTYPE_EMPTY,false,WHITE)) ||
		   (std::cerr << moves << std::endl,0));
  }
}
BOOST_AUTO_TEST_CASE(AddEffect8TestDropShort)
{
  {
    SimpleState state=
      CsaString(
		"P1+NY+TO *  *  *  * -OU-KE-KY\n"
		"P2 *  *  *  *  * -GI-KI *  *\n"
		"P3 * +RY *  * +UM * -KI-FU-FU\n"
		"P4 *  * +FU-FU *  *  *  *  *\n"
		"P5 *  * -KE * +FU *  * +FU *\n"
		"P6+KE *  * +FU+GI-FU *  * +FU\n"
		"P7 *  *  *  *  *  *  *  *  *\n"
		"P8 *  *  *  *  *  *  *  *  * \n"
		"P9 * +OU * -GI *  *  *  * -NG\n"
		"P+00HI00KA00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
		"P-00KI00KY00FU00FU\n"
		"P-00AL\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(4,3),PAWN,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(1,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,4),KNIGHT,BLACK)));
    // 王手はproduceしない
    BOOST_CHECK(!moves.isMember(Move(Square(2,2),GOLD,BLACK)));
    // 二歩
    BOOST_CHECK(!moves.isMember(Move(Square(2,2),PAWN,BLACK)));
    // 玉以外から取られる飛車は生成しない
    BOOST_CHECK(!moves.isMember(Move(Square(2,2),ROOK,BLACK)));
    // 玉以外から取られない角生成する
    BOOST_CHECK(moves.isMember(Move(Square(4,1),BISHOP,BLACK)));
  }
  {
    SimpleState state=
      CsaString(
		"P1+NY+TO *  *  *  * -OU * -KY\n"
		"P2 *  *  *  *  * -KE-KI *  * \n"
		"P3 *  *  *  * +UM *  * -FU-FU\n"
		"P4 *  * +FU-FU *  *  *  * -KI\n"
		"P5 *  * -KE * +FU *  * +FU-GI\n"
		"P6+KE *  * +FU+GI-FU+HI * +FU\n"
		"P7 *  *  *  *  *  *  *  *  *\n"
		"P8 *  *  *  *  *  *  *  *  * \n"
		"P9 * +OU * -GI *  *  *  * -NG\n"
		"P+00HI00KA00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
		"P-00KI00KY00FU00FU\n"
		"P-00AL\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    // pinnedなら飛車角もdropする
    BOOST_CHECK(moves.isMember(Move(Square(2,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,1),BISHOP,BLACK)));
  }
}

BOOST_AUTO_TEST_CASE(AddEffect8TestDropLong)
{
  {
    SimpleState state=
      CsaString(
		"P1-KY *  *  *  * -OU * -KE-KY\n"
		"P2 * -HI *  *  *  * -KI *  * \n"
		"P3-FU *  *  * +KI-GI * -FU-FU\n"
		"P4 *  * -FU-FU *  * -FU *  * \n"
		"P5 *  *  * -KE *  *  * +FU * \n"
		"P6+GI-FU+FU *  *  * +FU *  * \n"
		"P7+FU *  * +FU-KI *  *  * +FU\n"
		"P8 * +OU+KI * -FU *  *  *  * \n"
		"P9+KY+KE *  * -RY *  *  * +KY\n"
		"P+00KA00GI00GI00KE00FU00FU00FU00FU\n"
		"P-00KA\n"
		"-\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(WHITE,eState,store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(4,6),BISHOP,WHITE)) ||
		   (std::cerr << moves << std::endl,0)
		   );
  }
  {
    SimpleState state=
      CsaString(
		"P1+NY+TO *  *  *  * -OU-KE *\n"
		"P2 *  *  *  *  * -GI-KI * -KY\n"
		"P3 * +RY *  *  *  *  * -FU-FU\n"
		"P4 *  * +FU-FU *  *  *  *  *\n"
		"P5 *  * -KE * +FU *  * +FU *\n"
		"P6+KE *  * +FU+GI-FU *  * +FU\n"
		"P7 *  * -UM *  *  *  *  *  *\n"
		"P8 *  *  *  *  *  *  *  *  * \n"
		"P9 * +OU * -GI *  *  *  * -NG\n"
		"P+00HI00KA00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
		"P-00KI00KI00KY00FU00FU\n"
		"P-00AL\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(2,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,2),LANCE,BLACK)));

    // 飛車のただ捨て
    BOOST_CHECK(!moves.isMember(Move(Square(4,3),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,3),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,4),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,4),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,5),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,5),LANCE,BLACK)));

    BOOST_CHECK(moves.isMember(Move(Square(5,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,2),ROOK,BLACK)));

    BOOST_CHECK(!moves.isMember(Move(Square(7,2),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(8,2),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(9,2),ROOK,BLACK)));

    BOOST_CHECK(moves.isMember(Move(Square(1,1),BISHOP,BLACK)));
    //
    BOOST_CHECK(!moves.isMember(Move(Square(3,3),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,4),BISHOP,BLACK)));

    BOOST_CHECK(!moves.isMember(Move(Square(4,3),BISHOP,BLACK)));

    BOOST_CHECK(moves.isMember(Move(Square(5,4),BISHOP,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(6,5),BISHOP,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(7,6),BISHOP,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(8,7),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(9,8),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,1),BISHOP,BLACK)));

    BOOST_CHECK(!moves.isMember(Move(Square(2,4),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(1,5),BISHOP,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(5,1),BISHOP,BLACK)));

    BOOST_CHECK(moves.isMember(Move(Square(5,2),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,3),BISHOP,BLACK)));

    // direct系
    BOOST_CHECK(moves.isMember(Move(Square(1,1),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(3,3),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,3),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,5),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,5),LANCE,BLACK)));
    // 2つ以上は生成しない
    BOOST_CHECK(!moves.isMember(Move(Square(3,6),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(3,7),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(3,8),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(3,9),ROOK,BLACK)));

    BOOST_CHECK(moves.isMember(Move(Square(3,6),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,7),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,8),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,9),LANCE,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(5,3),BISHOP,BLACK)));


    // 王手はproduceしない
    BOOST_CHECK(!moves.isMember(Move(Square(2,2),BISHOP,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(4,1),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(5,1),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(6,1),ROOK,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(7,1),ROOK,BLACK)));

    // 盤外への利き
    BOOST_CHECK(!moves.isMember(Move(Square(4,1),LANCE,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(6,2),BISHOP,BLACK)));
  }
}

BOOST_AUTO_TEST_CASE(AddEffect8TestShort)
{
  {
    SimpleState state=
      CsaString(
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
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(4,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(1,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,3),PAWN,BLACK)));
    // 王手はproduceしない
    BOOST_CHECK(!moves.isMember(Move(Square(4,3),KNIGHT,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(2,2),GOLD,BLACK)));
  }
  {
    SimpleState state=
      CsaString(
		"P1+NY *  *  *  *  * -OU-KE-KY\n"
		"P2 * +HI *  * -FU-GI-KI-FU *\n"
		"P3 * +RY *  * +UM-FU-KI * -FU\n"
		"P4 *  * +FU-FU *  *  * +FU *\n"
		"P5 *  * -KE * +FU+KY *  *  *\n"
		"P6+KE * +KA+FU+GI *  *  * +FU\n"
		"P7 *  *  *  *  *  *  *  *  *\n"
		"P8 *  *  *  *  *  *  *  *  * \n"
		"P9 * +OU * -GI *  *  *  * -NG\n"
		"P+00KI00KE00FU00FU00FU00FU00FU00FU\n"
		"P-00KI00KY00FU\n"
		"P-00AL\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(2,4),Square(2,3),PAWN,PTYPE_EMPTY,false,BLACK)));
  }
}
BOOST_AUTO_TEST_CASE(AddEffect8TestLong)
{
  {
    SimpleState state=
      CsaString(
"P1 *  *  *  * +RY *  *  * -KY\n"
"P2 *  *  * -HI *  *  *  *  * \n"
"P3-KY *  * -OU-KA * +KI *  * \n"
"P4 * -FU-FU-GI-KI *  *  *  * \n"
"P5 *  *  * -KE-FU-FU-FU *  * \n"
"P6-KY * +FU+GI *  *  *  *  * \n"
"P7+KE+FU * +KI *  *  *  * +FU\n"
"P8+FU+OU+KI+KA-GI *  *  *  * \n"
"P9 * +KE *  *  *  *  *  * +KY\n"
"P+00GI00KE00FU00FU00FU00FU00FU00FU00FU\n"
"P-00FU00FU\n"
"-\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(WHITE,eState,store);
    }
    // 2重の追加利き
    BOOST_CHECK(moves.isMember(Move(Square(6,2),Square(9,2),ROOK,PTYPE_EMPTY,false,WHITE)));
  }
  {
    SimpleState state=
      CsaString(
"P1 *  *  *  *  *  *  * +TO+RY\n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3+OU+NK * +TO *  *  *  *  * \n"
"P4+GI+KI *  *  *  *  *  *  * \n"
"P5-KI *  *  *  * +GI *  *  * \n"
"P6 *  * -UM * +FU * +TO-KI * \n"
"P7 *  *  *  *  *  * -NK-NY * \n"
"P8 *  *  *  *  *  * -NY-OU-KI\n"
"P9-RY *  *  *  *  *  *  *  * \n"
"P+00KA00KY00KY00FU00FU00FU00FU\n"
"P-00GI00GI00KE00KE00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
"-\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(WHITE,eState,store);
    }
    // open，直接利きを減らしているが，生成する．
    BOOST_CHECK(moves.isMember(Move(Square(9,5),Square(8,6),GOLD,PTYPE_EMPTY,false,WHITE)));
  }
  {
    SimpleState state=
      CsaString(
"P1 *  *  *  *  *  *  * +TO+RY\n"
"P2 *  *  *  *  *  *  *  *  * \n"
"P3+OU+NK * +TO *  *  *  *  * \n"
"P4+GI+KI *  *  *  *  *  *  * \n"
"P5-GI *  *  *  * +GI *  *  * \n"
"P6 *  * -UM * +FU * +TO-KI * \n"
"P7 *  *  *  *  *  * -NK-NY * \n"
"P8 *  *  *  *  *  * -NY-OU-KI\n"
"P9-RY *  *  *  *  *  *  *  * \n"
"P+00KA00KY00KY00FU00FU00FU00FU\n"
"P-00KI00GI00KE00KE00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
"-\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(WHITE,eState,store);
    }
    // open
    BOOST_CHECK(moves.isMember(Move(Square(9,5),Square(8,6),SILVER,PTYPE_EMPTY,false,WHITE)));
  }
  {
    SimpleState state=
      CsaString(
"P1-KY-KE *  *  *  *  *  * -KY\n"
"P2 *  *  *  *  *  * -KI-OU * \n"
"P3 *  * -FU *  * -KI *  *  * \n"
"P4-FU-HI *  * -FU * -FU+GI-FU\n"
"P5 * -FU+FU * +GI-FU * -FU+KY\n"
"P6+FU *  *  *  *  * +KE *  * \n"
"P7 * +FU * -TO * +FU+FU+FU * \n"
"P8+KY *  *  *  * +KI+KI+GI * \n"
"P9 * +KE-UM * +KA *  * +KE+OU\n"
"P+00FU\n"
"P-00HI00GI00FU00FU\n"
"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    // 飛車角の追加利きは取られるところには移動しても良い
    BOOST_CHECK(moves.isMember(Move(Square(5,9),Square(6,8),BISHOP,PTYPE_EMPTY,false,BLACK)));
  }
  {
    SimpleState state=
      CsaString(
"P1 * +RY *  * -FU-OU * -KE-KY\n"
"P2 *  *  *  * +FU-GI-KI *  * \n"
"P3-FU * -FU-FU * -KI *  *  * \n"
"P4 *  *  *  *  * -FU+FU * -FU\n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  * +FU+FU *  * +KY *  * \n"
"P7+FU+FU *  *  * +FU-UM * +FU\n"
"P8 * +KI * +GI+OU *  *  *  * \n"
"P9+KY+KE *  * +KE *  * -RY+KY\n"
"P+00KA00GI00FU\n"
"P-00KI00GI00KE00FU00FU00FU\n"
"-\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(WHITE,eState,store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(2,9),Square(2,7),PROOK,PTYPE_EMPTY,false,WHITE)));
  }
  {
    SimpleState state=
      CsaString(
		"P1-KY-KE * -KI *  * -GI+RY-KY\n"
		"P2 *  *  * -OU *  * -KI *  * \n"
		"P3-FU * -GI-FU-FU-FU *  * -FU\n"
		"P4 *  * -FU *  *  *  *  *  * \n"
		"P5 *  *  *  * +KA *  *  *  * \n"
		"P6 *  *  *  *  *  *  *  *  * \n"
		"P7+FU * +FU+FU+FU+FU+FU * +FU\n"
		"P8 * +GI+KI *  *  *  *  *  * \n"
		"P9+KY+KE *  * +OU+KI+GI * +KY\n"
		"P+00KE00FU00FU\n"
		"P-00HI00KA00KE00FU00FU00FU\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(6,4),BISHOP,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(4,4),BISHOP,PTYPE_EMPTY,false,BLACK)));
    // 飛車角は利きのあるところには移動しても良い
  }

  {
    SimpleState state=
      CsaString(
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
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(5,2),PBISHOP,PTYPE_EMPTY,false,BLACK))||
		   (std::cerr << moves << std::endl,0)
		   );
    BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(4,3),PBISHOP,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(8,3),Square(8,2),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(8,3),Square(7,2),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(8,3),Square(9,2),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(2,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,2),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,5),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,4),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,3),LANCE,BLACK)));
    // 王手はproduceしない
    BOOST_CHECK(!moves.isMember(Move(Square(5,3),Square(4,2),PBISHOP,PTYPE_EMPTY,false,BLACK)));
    // 盤外に利きを付ける手は生成しない
    BOOST_CHECK(!moves.isMember(Move(Square(5,3),Square(6,2),PBISHOP,PTYPE_EMPTY,false,BLACK)));
    // 利きがblockされていることはチェックする
    BOOST_CHECK(!moves.isMember(Move(Square(5,3),Square(4,4),PBISHOP,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(3,4),LANCE,BLACK)));
  }
}
BOOST_AUTO_TEST_CASE(AddEffect8TestAdditional)
{
  {
    SimpleState state=
      CsaString(
		"P1-KY-KE * -KI *  *  *  * +RY\n"
		"P2 * -OU-GI-KI *  *  *  *  * \n"
		"P3 *  * -FU-FU * -FU * -FU * \n"
		"P4-FU+FU *  * -FU-GI *  * +FU\n"
		"P5 *  *  *  *  *  * -FU+FU * \n"
		"P6+FU *  *  * +FU+FU *  *  * \n"
		"P7 * +GI+KE+FU+KA * +FU *  * \n"
		"P8 *  * +OU+GI *  *  *  *  * \n"
		"P9+KY *  *  * +KI *  *  * -RY\n"
		"P+00KE00KY\n"
		"P-00KA00KI00KE00KY00FU00FU00FU\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(BLACK,eState,store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(8,5),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(8,6),LANCE,BLACK)));
  }
  {
    SimpleState state=
      CsaString(
		"P1-KY-KE * -KI *  *  *  * +RY\n"
		"P2 * -OU-GI * -KI *  *  *  * \n"
		"P3 * -FU-FU-FU-FU-FU * -FU * \n"
		"P4-FU *  *  * +KE-GI *  * +FU\n"
		"P5 * +FU *  *  *  * -FU+FU * \n"
		"P6+FU * -KY * +FU+FU *  *  * \n"
		"P7 * +GI+KA+FU *  * +FU *  * \n"
		"P8 * +OU+KI+GI *  *  *  *  * \n"
		"P9+KY+KE *  * +KI *  *  * -RY\n"
		"P+00KA\n"
		"P-00KE00KY00FU00FU\n"
		"-\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(WHITE,eState,store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(7,4),LANCE,WHITE)));
  }
  {
    SimpleState state=
      CsaString(
		"P1-KY-KE * -KI *  *  *  * +RY\n"
		"P2 * -OU-GI * -KI *  *  *  * \n"
		"P3 * -FU * -FU-FU-FU * -FU * \n"
		"P4 *  *  *  * +KE-GI *  * +FU\n"
		"P5 * +FU-FU *  *  * -FU+FU * \n"
		"P6-FU *  *  * +FU+FU *  *  * \n"
		"P7+FU+GI+KA+FU *  * +FU *  * \n"
		"P8 * +OU+KI+GI *  *  *  *  * \n"
		"P9+KY+KE *  * +KI *  *  * -RY\n"
		"P+00KA\n"
		"P-00KE00KY00KY00FU00FU\n"
		"-\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAddEffect8::generate(WHITE,eState,store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(7,4),LANCE,WHITE)));
    BOOST_CHECK(moves.isMember(Move(Square(7,6),LANCE,WHITE)));
    BOOST_CHECK(moves.isMember(Move(Square(9,4),LANCE,WHITE)));
  }
}
static bool isCheckMove(NumEffectState& state,Move move){
  // 自分で動いて元のpositionに利きをつける手は含まない
  Player pl=state.turn();
  Square kingSquare=state.kingSquare(alt(pl));
  NumEffectState next_state(state);
  next_state.makeMove(move);
  // なるべく王手は生成したくないが，生成してしまうこともある．
  return next_state.hasEffectAt(pl,kingSquare);
}

/**
 * 作られない可能性のあるmove
 * 作られる可能性もある．
 */
static bool mayNotGeneratedMove(const NumEffectState& savedState,Move move){
  NumEffectState state = savedState;
  Player pl=state.turn();
  Square kingSquare=state.kingSquare(alt(pl));
  Ptype ptype=move.ptype();
  // 
  if(move.isPromotion()) return true;
  if(move.capturePtype()!=PTYPE_EMPTY) return true;
  // 王が動いた時は生成しないことがある．
  if(ptype==KING) return true;
  // 飛車角のdropは2つしか生成しないので
  if((ptype==ROOK || ptype==BISHOP) &&
     !move.from().isOnBoard()) return true;
  // 飛車角歩がpromote可能なのにpromoteしない手は生成しない
  if(move.from().isOnBoard() &&
     (ptype==ROOK || ptype==BISHOP || ptype==PAWN) &&
     (move.from().canPromote(pl) || move.to().canPromote(pl))) return true;
  // 二段目の香車は必ず成る
  if(move.from().isOnBoard() &&
     ptype==LANCE &&
     ((pl==BLACK && move.to().y()==2) ||
      (pl==WHITE && move.to().y()==8))) return true;
  state.makeMove(move);
  // なるべく王手は生成したくないが，生成してしまうこともある．
  if(state.hasEffectAt(pl,kingSquare)) return true;
  for(int i=0;i<8;i++){
    Direction d=static_cast<Direction>(i);
    Square pos=kingSquare+Board_Table.getOffsetForBlack(d);
    if(!pos.isOnBoard()) continue;
    if(savedState.countEffect(pl,pos)>state.countEffect(pl,pos)) continue;
    if(effect_util::AdditionalEffect::count2(savedState,pos,pl)>
       effect_util::AdditionalEffect::count2(state,pos,pl)){
      // 元々の駒が利きのあったところに動いても利きを持つが，追加が変わる例
      // 元々の駒が直接利きを持っていたのを間接利きに変わる例も
      // これは未対応
      if(move.from().isOnBoard() &&
	 savedState.hasEffectByPiece(savedState.pieceAt(move.from()),pos)){
	return true;
      }
      continue;
    }
    else if(effect_util::AdditionalEffect::count2(savedState,pos,pl)==
	    effect_util::AdditionalEffect::count2(state,pos,pl)) continue;
    else return false;
  }
  return true;
}
/**
 * 相手の8近傍に利きをつける手かどうかのチェック.
 * 欲しい仕様は8近傍のどこかにこれまで利きのなかった駒の利きが追加されること．
 * 全体としてそのマスへの利きが減っても可．
 * 王手は除外する．
 * 王によって8近傍に利きをつける手も除外する．
 * 自殺手は生成してしまうことはあってもよい．
 * @param state - move前の局面
 * @param move - チェックの対象の手
 */
static bool isAddEffect8Move(const NumEffectState& state_org,Move move){
  // 自分で動いて元のpositionに利きをつける手は含まない
  NumEffectState state = state_org;
  Player pl=state.turn();
  Square kingSquare=state.kingSquare(alt(pl));
  NumEffectState savedState(state);
  Ptype ptype=move.ptype();
  if(move.isPromotion()) return false;
  if(move.capturePtype()!=PTYPE_EMPTY) return false;
  if(isCheckMove(state,move)) return false;
  // 飛車角は相手の利きのあるところにはdropしない
  if(!move.from().isOnBoard() && (ptype==ROOK || ptype==BISHOP) &&
     state.hasEffectByNotPinnedAndKing(alt(pl),move.to())) return false;
  state.makeMove(move);
  for(int i=0;i<8;i++){
    Direction d=static_cast<Direction>(i);
    Square pos=kingSquare+Board_Table.getOffsetForBlack(d);
    if(!pos.isOnBoard()){
      PtypeO ptypeO=newPtypeO(pl,ptype);
      // 盤外であっても短い利きが増えたらOK
      Square from=move.from();
      Square to=move.to();
      if(move.from().isOnBoard() &&
	 (abs(from.x()-pos.x())>1 ||
	  abs(from.y()-pos.y())>1 ||
	 !Ptype_Table.getEffect(ptypeO,Offset32(pos,move.from())).hasUnblockableEffect()) &&
	 abs(to.x()-pos.x())<=1 &&
	 abs(to.y()-pos.y())<=1 &&
	 Ptype_Table.getEffect(ptypeO,Offset32(pos,move.to())).hasUnblockableEffect() &&
	 (!(ptype==ROOK || ptype==BISHOP || ptype==PAWN) ||
	  !(move.from().canPromote(pl) || move.to().canPromote(pl)))){
	return true;
      }
      continue;
    }
    if(state.hasEffectAt(pl,pos) &&
       (effect_util::AdditionalEffect::count2(savedState,pos,pl)<
	effect_util::AdditionalEffect::count2(state,pos,pl))){
      return true;
    }
    if(state.hasEffectAt(pl,pos) &&
       (effect_util::AdditionalEffect::count(savedState,pl,pos)<
	effect_util::AdditionalEffect::count(state,pl,pos))){
      return true;
    }
    for(int num=0;num<40;num++){
      Piece p=state.pieceOf(num);
      if(p.ptype()==KING) continue;
      if(p.isOnBoardByOwner(pl) &&
	 state.hasEffectByPiece(p,pos)){
	if(!savedState.hasEffectByPiece(savedState.pieceOf(num),pos) &&
	   ((p.square()!=move.to() && move.to() != pos)||
	    !move.from().isOnBoard() ||
	    !(ptype==ROOK || ptype==BISHOP || ptype==PAWN) ||
	    !(move.from().canPromote(pl) || move.to().canPromote(pl)))){
	  // ただ捨ての飛車，角のdropは生成しない
	  return true;
	}
	// PROOKが斜めに動いて斜めに利きをつける場合は
	// 元々長い利きがある場合もあるが，常にtrueにする．
	if(p.square()==move.to() && ptype==PROOK && 
	   abs(move.from().x()-move.to().x())==1 &&
	   abs(move.from().y()-move.to().y())==1 &&
	   abs(move.to().x()-pos.x())==1 &&
	   abs(move.to().y()-pos.y())==1){
	  return true;
	}
      }
    }
  }
  return false;
}

BOOST_AUTO_TEST_CASE(AddEffect8TestIsAddEffect8)
{
  {
    SimpleState state=
      CsaString(
"P1-KY-KE-OU-KY+RY *  *  *  * \n"
"P2 *  * -KI-KI *  *  *  *  * \n"
"P3 * +GI-FU *  * +TO * -FU * \n"
"P4-FU *  *  * -FU *  *  * +FU\n"
"P5 * +KE * -KA *  * -FU+FU * \n"
"P6+FU * +KY * +FU *  *  *  * \n"
"P7 * +OU * +FU+KA * +FU *  * \n"
"P8 *  *  * +GI *  *  *  *  * \n"
"P9+KY *  *  * +KI *  *  * -RY\n"
"P+00GI00KE00FU00FU\n"
"P-00KI00GI00KE00FU00FU00FU00FU\n"
"+\n"
		).initialState();
    NumEffectState eState(state);
    BOOST_CHECK(isAddEffect8Move(eState,Move(Square(5,1),Square(4,2),PROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    SimpleState state=
      CsaString(
		"P1 *  * +RY-KE *  *  * -KE-KY\n"
		"P2 *  *  * +GI * -OU-KI *  * \n"
		"P3-FU * -FU-FU * -KI *  *  * \n"
		"P4 *  *  * -UM * -FU+FU * -FU\n"
		"P5+OU *  *  *  *  *  *  *  * \n"
		"P6 *  * +FU-KI *  * +KY *  * \n"
		"P7+FU+FU *  * +FU+FU *  * +FU\n"
		"P8 * +KI *  *  *  *  *  *  * \n"
		"P9+KY+KE *  * -RY *  *  * +KY\n"
		"P+00GI00KE00FU\n"
		"P-00KA00GI00GI00FU00FU00FU00FU00FU\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    //    BOOST_CHECK(!isAddEffect8Move(eState,Move(Square(3,4),Square(3,3),PAWN,PTYPE_EMPTY,false,BLACK)));
  }
}


static bool isAddEffect8Drop(const NumEffectState& state_org,Move move){
  // 自分で動いて元のpositionに利きをつける手は含まない
  if(!move.isDrop()) return false;
  NumEffectState state = state_org;
  Player pl=state.turn();
  Square kingSquare=state.kingSquare(alt(pl));
  NumEffectState savedState(state);
  state.makeMove(move);
  if(state.hasEffectAt(pl,kingSquare)) return false;
  // ただ捨ての飛車角
  if(!move.from().isOnBoard() && 
     (move.ptype()==ROOK || move.ptype()==BISHOP) &&
     state.hasEffectByNotPinnedAndKing(alt(pl),move.to())) return false;
  for(int i=0;i<8;i++){
    Direction d=static_cast<Direction>(i);
    Square pos=kingSquare+Board_Table.getOffsetForBlack(d);
    if(!pos.isOnBoard()) continue;
    if(effect_util::AdditionalEffect::count2(savedState,pos,pl)<
       effect_util::AdditionalEffect::count2(state,pos,pl)) return true;
    for(int num=0;num<40;num++){
      Piece p=state.pieceOf(num);
      if(p.ptype()==KING) continue;
      if(p.isOnBoardByOwner(pl) &&
	 state.hasEffectByPiece(p,pos) &&
	 !savedState.hasEffectByPiece(p,pos))
	return true;
    }
  }
  return false;
}


static void testDropFile(const std::string& fileName){
  auto record=CsaFileMinimal(fileName).load();
  NumEffectState state(record.initialState());
  const auto& moves=record.moves;
  for(unsigned int i=0;i<moves.size();i++){
    if (! state.inCheck())
    {				// 王手以外の状況でテスト
      MoveVector allMoves;
      {
	Store store(allMoves);
	AllMoves<Store>::
	  generate(state.turn(),state,store);
      }
	  // 次の1行はaddEffect.hのバグの範囲を突き止めるためのテスト
	  //	if(state.hasEffectAt(state.turn(),pos)) continue;
      MoveVector effectMoves;
      {
	Store storeEffect(effectMoves);
	GenerateAddEffect8::
	  generate(state.turn(),state,storeEffect);
      }
      
      size_t count1=0;
      for(size_t j=0;j<effectMoves.size();j++){
	if(effectMoves[j].isDrop()){
	  count1++;
	  BOOST_CHECK((state.isValidMove(effectMoves[j],true)  
			 && isAddEffect8Drop(state,effectMoves[j])) ||
			 (std::cerr << std::endl << state  << std::endl << effectMoves[j],0)
			 );
	}
      }
      // 
      size_t count=0;
      MoveVector tmpMoves;
      for(size_t j=0;j<allMoves.size();j++){
	if(isAddEffect8Drop(state,allMoves[j])){
	  tmpMoves.push_back(allMoves[j]);
	  count++;
	  BOOST_CHECK(effectMoves.isMember(allMoves[j]) ||
			 ((allMoves[j].ptype()==ROOK ||allMoves[j].ptype()==BISHOP) &&
			  !allMoves[j].from().isOnBoard()) ||
			 (std::cerr << allMoves[j] << std::endl <<
			  state << std::endl,0)
			 );
	}
      }
      //      BOOST_CHECK(count == count1);
    }
    Move move=moves[i];
    state.makeMove(move);
  }
}

BOOST_AUTO_TEST_CASE(AddEffect8TestDropMember){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=100;
  if (OslConfig::inUnitTestShort()) 
    count=2;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && ++i<count){
    if (progress)
      ++(*progress);
    testDropFile(OslConfig::testCsaFile(filename));
  }
}

static void testMoveFile(const std::string& filename){
  auto record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  const auto& moves=record.moves;
  for(unsigned int i=0;i<moves.size();i++){
    if (! state.inCheck())
    {				// 王手以外の状況でテスト
      MoveVector allMoves;
      {
	Store store(allMoves);
	AllMoves<Store>::
	  generate(state.turn(),state,store);
      }
	  // 次の1行はaddEffect.hのバグの範囲を突き止めるためのテスト
	  //	if(state.hasEffectAt(state.turn(),pos)) continue;
      MoveVector effectMoves;
      {
	Store storeEffect(effectMoves);
	GenerateAddEffect8::
	  generate(state.turn(),state,storeEffect);
      }
      
      for(size_t j=0;j<effectMoves.size();j++){
	BOOST_CHECK(state.isValidMove(effectMoves[j],true) ||
		       (std::cerr << std::endl << state << std::endl << effectMoves[j] << std::endl,0));
	BOOST_CHECK(state.isSafeMove(effectMoves[j]));
	if(!isAddEffect8Move(state,effectMoves[j]) &&
	   !isCheckMove(state,effectMoves[j])){
	  std::cerr << std::endl << state << std::endl << effectMoves[j] << std::endl;
	}
      }
      // 
      for(size_t j=0;j<allMoves.size();j++){
	if(isAddEffect8Move(state,allMoves[j]))
	  BOOST_CHECK(effectMoves.isMember(allMoves[j]) ||
			 mayNotGeneratedMove(state,allMoves[j]) ||
			 (std::cerr << std::endl << state << std::endl << allMoves[j] << std::endl,0)
			 );
      }
    }
    Move move=moves[i];
    state.makeMove(move);
  }
}

BOOST_AUTO_TEST_CASE(AddEffect8TestMoveMember){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=200;
  if (OslConfig::inUnitTestShort()) 
    count=2;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  std::string filename;
  while((ifs >> filename) && filename != "" && ++i<count){
    if (progress)
      ++(*progress);
    testMoveFile(OslConfig::testCsaFile(filename));
  }
}

BOOST_AUTO_TEST_CASE(AddEffect8TestBug_r3527)
{
  NumEffectState state(CsaString(
			 "P1 *  *  * +RY *  *  * -KE-KY\n"
			 "P2 *  *  *  *  * +GI *  *  * \n"
			 "P3-FU *  * +TO * -FU * -FU-OU\n"
			 "P4 *  *  *  * -FU-KI-FU * -FU\n"
			 "P5 * -FU *  *  *  *  *  *  * \n"
			 "P6 *  *  *  * +FU+FU * +FU+FU\n"
			 "P7+FU+FU * +KI *  * +FU * +KE\n"
			 "P8+KY *  * +FU+GI+KA *  * +OU\n"
			 "P9 * +KE-RY *  *  *  *  * -KA\n"
			 "P+00KI00KI00GI00KE00KY00KY\n"
			 "P-00GI00FU00FU\n"
			 "-\n").initialState());
  MoveVector moves;
  {
    Store store(moves);
    move_generator::AddEffect8<WHITE>::generate(state, store); // assersion failure here with r3527
  }
  BOOST_CHECK(! moves.empty());
}

BOOST_AUTO_TEST_CASE(AddEffect8Test20100117)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * -KI *  * -KY\n"
			   "P2 *  *  *  * -KI *  *  *  * \n"
			   "P3-FU *  *  *  * -FU * -FU * \n"
			   "P4 *  *  *  *  *  * -FU * -FU\n"
			   "P5 *  *  * +FU * +KE *  *  * \n"
			   "P6 * -OU *  * +GI+FU *  * +FU\n"
			   "P7+FU *  *  * +KA * +FU+FU * \n"
			   "P8+GI-UM *  * +KI * +GI * +OU\n"
			   "P9+KY * +FU *  *  *  * +KE+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00HI00HI00KI00KE00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      move_generator::AddEffect8<BLACK>::generate(state, store);
    }
    Move m89gi(Square(9,8), Square(8,9), SILVER, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(m89gi)
		   || mayNotGeneratedMove(state,m89gi)
		   || ! isAddEffect8Move(state,m89gi));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * -KI *  * -KY\n"
			   "P2 *  *  *  * -KI *  *  *  * \n"
			   "P3-FU *  *  *  * -FU * -FU * \n"
			   "P4 *  *  *  *  *  * -FU * -FU\n"
			   "P5+FU *  * +FU * +KE *  *  * \n"
			   "P6+KE-OU *  * +GI+FU *  * +FU\n"
			   "P7 *  *  *  * +GI * +FU+FU * \n"
			   "P8 * -UM *  * +KI * +GI * +OU\n"
			   "P9+KY * +FU *  *  *  * +KA+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00HI00HI00KI00KE00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      move_generator::AddEffect8<BLACK>::generate(state, store);
    }
    Move m84ke(Square(9,6), Square(8,4), KNIGHT, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(m84ke)
		   || mayNotGeneratedMove(state, m84ke)
		   ||! isAddEffect8Move(state, m84ke));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * -KI *  * -KY\n"
			   "P2 *  *  *  * -KI *  *  *  * \n"
			   "P3-FU *  *  *  * -FU * -FU * \n"
			   "P4 *  *  *  *  *  * -FU * -FU\n"
			   "P5+FU *  * +FU * +KE *  *  * \n"
			   "P6+KA-OU *  * +GI+FU *  * +FU\n"
			   "P7 *  *  *  * +GI * +FU+FU * \n"
			   "P8 * -UM *  * +KI * +GI * +OU\n"
			   "P9+KY * +FU *  *  *  * +KE+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00HI00HI00KI00KE00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      move_generator::AddEffect8<BLACK>::generate(state, store);
    }
    Move m69ka(Square(9,6), Square(6,9), BISHOP, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(m69ka)
		   || mayNotGeneratedMove(state, m69ka)
		   ||! isAddEffect8Move(state, m69ka));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * -KI *  * -KY\n"
			   "P2 *  *  *  * -KI *  *  *  * \n"
			   "P3-FU *  *  *  * -FU * -FU * \n"
			   "P4 *  *  *  *  *  * -FU * -FU\n"
			   "P5 *  *  * +FU * +KE *  *  * \n"
			   "P6 * -OU *  * +GI+FU *  * +FU\n"
			   "P7+FU *  *  * +GI * +FU+FU * \n"
			   "P8+KA-UM *  * +KI * +GI * +OU\n"
			   "P9+KY * +FU *  *  *  * +KE+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00HI00HI00KI00KE00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      move_generator::AddEffect8<BLACK>::generate(state, store);
    }
    Move m89ka(Square(9,8), Square(8,9), BISHOP, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(m89ka)
		   || mayNotGeneratedMove(state,m89ka)
		   || ! isAddEffect8Move(state,m89ka));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * -OU * +GI *  *  * -KY\n"
			   "P2 *  *  * -HI *  *  * +TO * \n"
			   "P3-KE *  * -KI-FU+KA *  * -FU\n"
			   "P4-FU+OU * -FU *  *  * +HI * \n"
			   "P5 *  *  * -KE *  *  *  *  * \n"
			   "P6+FU * +FU * +FU+FU *  *  * \n"
			   "P7 *  * +KA+FU *  *  *  * +FU\n"
			   "P8-GI *  *  * +KI *  *  *  * \n"
			   "P9 * -TO * +KI *  *  * +KE+KY\n"
			   "P+00KI00GI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			   "P-00GI\n"
			   "-\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      move_generator::AddEffect8<WHITE>::generate(state, store);
    }
    Move m85ke(Square(9,3), Square(8,5), KNIGHT, PTYPE_EMPTY, false, WHITE);
    BOOST_CHECK(moves.isMember(m85ke)
		   || mayNotGeneratedMove(state, m85ke)
		   ||! isAddEffect8Move(state, m85ke));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
