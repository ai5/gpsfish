#include "osl/move_generator/attackToPinned.h"
#include "osl/move_generator/allMoves.h"
// #include "osl/move_generator/move_action.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::move_generator;
using namespace osl::move_action;

BOOST_AUTO_TEST_CASE(AttackToPinnedTestSimple)
{
  {
    SimpleState state=
      CsaString(
		"P1+NY+RY *  * -KI * -OU-KE *\n"
		"P2 *  *  *  *  * -GI-KI-FU *\n"
		"P3 * +TO *  *  *  *  *  * +KA\n"
		"P4 *  * +FU+UM *  *  *  *  *\n"
		"P5 *  * -KE-FU-FU *  *  * -FU\n"
		"P6+KE *  * +FU+GI-FU *  * +FU\n"
		"P7 *  *  *  *  *  *  *  *  *\n"
		"P8 *  *  *  *  *  *  *  *  * \n"
		"P9 * +OU * -GI *  * +KY * -NG\n"
		"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU00FU\n"
		"P-00KI00KY00FU00FU\n"
		"P-00AL\n"
		"+\n"
		).initialState();
    NumEffectState eState(state);
    MoveVector moves;
    {
      move_action::Store store(moves);
      GenerateAttackToPinned::generate(BLACK,eState,store);
    }
    // 歩を攻める手も生成する
    BOOST_CHECK(moves.isMember(Move(Square(1,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,3),PAWN,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(1,2),GOLD,BLACK)));
    // 金を攻める
    BOOST_CHECK(moves.isMember(Move(Square(2,4),KNIGHT,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,3),GOLD,BLACK)));
    // 利きをふさいでいる
    BOOST_CHECK(moves.isMember(Move(Square(3,3),PAWN,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,3),GOLD,BLACK)));
    // 銀を攻める
    BOOST_CHECK(moves.isMember(Move(Square(4,3),PAWN,BLACK)));
    // 金を攻める
    BOOST_CHECK(moves.isMember(Move(Square(5,2),PAWN,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,4),Square(7,3),PBISHOP,PTYPE_EMPTY,false,BLACK)));
  }
}

