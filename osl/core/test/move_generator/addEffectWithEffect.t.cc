#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_generator/addEffectWithEffect.tcc"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include "osl/move_classifier/shouldPromoteCut.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::move_action;
using namespace osl::move_generator;

BOOST_AUTO_TEST_CASE(AddEffectTestOne) {
  {
    NumEffectState state(CsaString(
			   "P1 *  * +KA *  *  *  * -KE-KY\n"
			   "P2-KY *  *  *  *  * -KI *  * \n"
			   "P3 *  *  *  * +GI * -GI * -FU\n"
			   "P4-FU * -FU *  *  * +KE *  * \n"
			   "P5 * -FU *  *  *  * -OU+FU+FU\n"
			   "P6+FU * +FU * -FU *  *  *  * \n"
			   "P7 * +FU-KI-RY *  *  *  * +KY\n"
			   "P8+OU *  *  *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  * +KE * \n"
			   "P+00KI00GI\n"
			   "P-00HI00KA00KI00GI00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,5),store);
    }
    // open move
    BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(4,4),SILVER,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(5,3),Square(4,4),PSILVER,PTYPE_EMPTY,true,BLACK)) ||
		   (std::cerr << moves << std::endl,0)
      );
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  * +KA * -RY *  * -KE-KY\n"
			   "P2-KY *  *  *  *  * -KI *  * \n"
			   "P3 *  *  *  * +GI * -GI * -FU\n"
			   "P4-FU * -FU *  *  * +KE *  * \n"
			   "P5 * -FU *  *  *  * -OU+FU+FU\n"
			   "P6+FU * +FU-FU *  *  *  *  * \n"
			   "P7 * +FU-KI *  *  *  *  * +KY\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+KY+KE *  * +OU *  * +KE * \n"
			   "P+00KI00GI\n"
			   "P-00HI00KA00KI00GI00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,5),store);
    }
    // open move
    BOOST_CHECK(!moves.isMember(Move(Square(5,3),Square(4,4),SILVER,PTYPE_EMPTY,false,BLACK)));
  }
}
BOOST_AUTO_TEST_CASE(AddEffectTestKnight) {
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * -FU+GI-KI *  * -KY\n"
			   "P2 *  *  *  *  *  * -OU * +TO\n"
			   "P3 * +RY *  *  *  *  * -FU-FU\n"
			   "P4 *  * +FU+UM *  *  * -GI * \n"
			   "P5 *  *  *  * +FU+NK-KI+FU+FU\n"
			   "P6 *  *  * +FU * -FU+KE * +KE\n"
			   "P7 *  *  *  *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  * -NG\n"
			   "P+00HI00KA00KI00GI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,2),store);
    }
    // drop move
    BOOST_CHECK(moves.isMember(Move(Square(4,4),KNIGHT,BLACK)));
    // move
    BOOST_CHECK(moves.isMember(Move(Square(1,6),Square(2,4),KNIGHT,SILVER,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(2,4),KNIGHT,SILVER,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(4,4),KNIGHT,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(4,5),Square(4,4),PKNIGHT,PTYPE_EMPTY,false,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * -FU+GI-KI *  * -KY\n"
			   "P2 *  *  *  *  *  * -OU * +TO\n"
			   "P3 * +RY *  *  *  *  * -FU-FU\n"
			   "P4 *  * +FU+UM *  *  * -GI * \n"
			   "P5 *  *  *  * +FU+NK-KI+FU+FU\n"
			   "P6 *  *  * +FU+KE-FU+KE * +KE\n"
			   "P7 *  *  *  *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  * -NG\n"
			   "P+00HI00KA00KI00GI00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,2),store);
    }
    // move
    BOOST_CHECK(moves.isMember(Move(Square(1,6),Square(2,4),KNIGHT,SILVER,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(2,4),KNIGHT,SILVER,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(4,4),KNIGHT,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,6),Square(4,4),KNIGHT,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(4,5),Square(4,4),PKNIGHT,PTYPE_EMPTY,false,BLACK)));
  }
}
BOOST_AUTO_TEST_CASE(AddEffectTestDrop) {
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
			   "P2 *  *  *  *  *  * -KI *  * \n"
			   "P3-FU * -FU-FU-FU-FU *  * -FU\n"
			   "P4 *  *  *  *  *  * +HI *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 * -HI+FU *  *  *  *  *  * \n"
			   "P7+FU *  * +FU+FU+FU+FU * +FU\n"
			   "P8 * +GI+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
			   "P+00KA00FU00FU00FU\n"
			   "P-00KA00FU00FU\n"
			   "-\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(WHITE,state,Square(8,2),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(7,1),BISHOP,WHITE)));
  }
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * -FU+GI-KI * -FU-KY\n"
			   "P2 *  *  *  *  *  * -OU * +FU\n"
			   "P3 * +RY *  *  *  *  * -KE-FU\n"
			   "P4 *  * +FU+UM *  *  *  *  *\n"
			   "P5 *  * -KE * +KA * -KI+FU-GI\n"
			   "P6+KE *  * +FU+FU-FU *  * +TO\n"
			   "P7 *  *  *  *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  * -NG\n"
			   "P+00HI00KI00GI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,2),store);
    }
    // 22の利きをさえぎるので，打ち歩詰めでない
    BOOST_CHECK(moves.isMember(Move(Square(3,3),PAWN,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1+RY *  *  *  *  * -KA-KE-KY\n"
			   "P2 *  *  *  * +TO * -KI * -OU\n"
			   "P3 *  * -KE *  * -KI+NK-GI * \n"
			   "P4-FU *  *  * -FU-FU-FU * -FU\n"
			   "P5 * -FU+FU *  *  *  * +OU * \n"
			   "P6+FU+FU * +GI * +FU+FU+FU+FU\n"
			   "P7 *  *  *  *  * +KI * -RY * \n"
			   "P8+KY-TO *  *  *  *  *  *  * \n"
			   "P9 * +KE *  *  *  *  * -KI+KY\n"
			   "P+00KA00GI00KY00FU00FU\n"
			   "P-00GI00FU\n"
			   "-\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(WHITE,state,Square(2,5),store);
    }
    // drop rook
    BOOST_CHECK(!moves.isMember(Move(Square(2,4),PAWN,WHITE)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  * +RY * -KY\n"
			   "P2 * -OU *  * +UM * +NK *  * \n"
			   "P3-FU * -GI-FU-FU-FU *  * -FU\n"
			   "P4 *  * -FU *  *  *  *  *  * \n"
			   "P5 *  *  *  * +KA *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			   "P8 *  * -NK * +OU *  *  *  * \n"
			   "P9+KY+KE * -HI * +KI+GI * +KY\n"
			   "P+00KI00FU00FU00FU\n"
			   "P-00KI00KI00GI00GI00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(8,2),store);
    }
    // drop rook
    BOOST_CHECK(moves.isMember(Move(Square(8,3),PAWN,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
			   "P2 *  *  *  *  *  * -KI *  * \n"
			   "P3-FU *  * -FU-FU-FU *  * -FU\n"
			   "P4 *  * -FU *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			   "P8 * +GI+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
			   "P+00HI00KA00FU00FU\n"
			   "P-00HI00KA00FU00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(5,1),store);
    }
    // drop rook
    BOOST_CHECK(moves.isMember(Move(Square(5,2),ROOK,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * -FU+GI-KI * -KE-KY\n"
			   "P2 *  *  *  *  *  * -OU * +TO\n"
			   "P3 * +RY *  *  *  *  * -FU-FU\n"
			   "P4 *  * +FU+UM *  *  * -GI *\n"
			   "P5 *  * -KE * +FU * -KI+FU *\n"
			   "P6+KE *  * +FU * -FU *  * +FU\n"
			   "P7 *  *  *  *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  * -NG\n"
			   "P+00HI00KA00KI00GI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,2),store);
    }
    // 打ち歩詰めでない
    BOOST_CHECK(moves.isMember(Move(Square(3,3),PAWN,BLACK)));
    // 香車
    BOOST_CHECK(moves.isMember(Move(Square(3,3),LANCE,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),LANCE,BLACK)));
    // 桂馬
    BOOST_CHECK(moves.isMember(Move(Square(4,4),KNIGHT,BLACK)));
    // 銀
    BOOST_CHECK(moves.isMember(Move(Square(3,3),SILVER,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,3),SILVER,BLACK)));
    // 金
    BOOST_CHECK(moves.isMember(Move(Square(3,3),GOLD,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,3),GOLD,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,2),GOLD,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,2),GOLD,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,1),GOLD,BLACK)));
    // 角
    BOOST_CHECK(moves.isMember(Move(Square(4,3),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,4),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,5),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(7,6),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(8,7),BISHOP,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(9,8),BISHOP,BLACK)));
    // 飛車
    BOOST_CHECK(moves.isMember(Move(Square(2,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,1),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,3),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(7,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(8,2),ROOK,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(9,2),ROOK,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * -FU+GI-KI * -FU-KY\n"
			   "P2 *  *  *  *  *  * -OU * +TO\n"
			   "P3 * +RY *  *  *  *  * -KE-FU\n"
			   "P4 *  * +FU+UM *  *  * +KI *\n"
			   "P5 *  * -KE * +FU * -GI+FU *\n"
			   "P6+KE *  * +FU * -FU *  * +FU\n"
			   "P7 *  *  *  *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU * -GI *  *  *  * -NG\n"
			   "P+00HI00KA00KI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,2),store);
    }
    // 打ち歩詰めは生成しない
    BOOST_CHECK(!moves.isMember(Move(Square(3,3),PAWN,BLACK)));
    
    
  }
  {
    NumEffectState state(CsaString(
			   "P1+NY+TO * -FU+GI-KI * -KE-KY\n"
			   "P2 *  *  *  *  *  * -OU * +FU\n"
			   "P3 * +RY *  *  *  *  * -FU-FU\n"
			   "P4 *  * +FU+UM *  *  * -GI *\n"
			   "P5 *  * -KE * +KA * -KI+FU *\n"
			   "P6+KE *  * +FU+FU-FU *  * +TO\n"
			   "P7 *  *  *  *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  * -NG\n"
			   "P+00HI00KI00GI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(3,2),store);
    }
    // 取り返せるので，打ち歩詰めでない
    BOOST_CHECK(moves.isMember(Move(Square(3,3),PAWN,BLACK)));
  }
}


void AddEffectTestMove(){
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
			   "P2 *  *  *  *  *  * -KI *  * \n"
			   "P3-FU * -FU-FU-FU-FU *  * -FU\n"
			   "P4 *  * -HI *  *  * +HI *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			   "P8 * +GI+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
			   "P+00KA00FU00FU\n"
			   "P-00KA00FU00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(9,4),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,4),Square(7,4),ROOK,ROOK,false,BLACK)));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE * -KI *  * +GI-KE-KY\n"
      "P2 *  *  * +RY *  * +HI *  * \n"
      "P3-FU * -FU *  * +FU *  *  * \n"
      "P4 * -FU *  * -OU *  *  * +KA\n"
      "P5 *  *  *  * -GI *  *  * -FU \n"
      "P6 *  * +FU+FU *  *  *  *  * \n"
      "P7+FU+FU *  *  *  *  *  * +FU\n"
      "P8 *  * +KI *  *  * +GI+UM-GI\n"
      "P9+KY+KE * +OU+KI *  * +KE+KY\n"
      "P+00FU00FU\n"
      "P-00FU00FU00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P+00KI\n"
      "+\n").initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(5,4),store);
    }
    // prook
    BOOST_CHECK(moves.isMember(Move(Square(6,2),Square(5,1),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,2),Square(5,2),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,2),Square(5,3),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,2),Square(6,3),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,2),Square(6,4),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(6,2),Square(6,5),PROOK,PTYPE_EMPTY,false,BLACK)));
    // BISHOP
    BOOST_CHECK(moves.isMember(Move(Square(1,4),Square(3,6),BISHOP,PTYPE_EMPTY,false,BLACK)));
    // PBISHOP
    BOOST_CHECK(moves.isMember(Move(Square(2,8),Square(1,8),PBISHOP,SILVER,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,8),Square(2,7),PBISHOP,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(2,8),Square(5,5),PBISHOP,SILVER,false,BLACK)));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE * +RY *  *  *  *  * \n"
      "P2-OU * -GI *  *  *  *  *  * \n"
      "P3 *  * -UM-FU-FU *  * -FU * \n"
      "P4-FU-FU-FU *  *  *  *  * -FU\n"
      "P5 *  *  * -KY * -FU * +FU * \n"
      "P6+FU * +FU+FU *  *  *  * +FU\n"
      "P7 * +FU+GI-KE+UM+FU *  *  * \n"
      "P8+KY+GI+KI *  *  *  *  *  * \n"
      "P9+OU+KE *  *  *  *  * -RY * \n"
      "P+00KI00KI00KY\n"
      "P-00KI00GI00KE00FU00FU00FU\n"
      "+\n"
      ).initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(9,2),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(5,7),Square(7,4),PBISHOP,PAWN,false,BLACK)));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE * -KY+RY *  *  *  * \n"
      "P2 *  * -OU *  *  *  *  *  * \n"
      "P3 *  * -FU-KI * +TO * -FU * \n"
      "P4-FU *  *  * -FU *  *  * +FU\n"
      "P5 * +KE * -KA *  * -FU+FU * \n"
      "P6+FU * +KY * +FU *  *  *  * \n"
      "P7 * +OU * +FU+KA * +FU *  * \n"
      "P8 *  *  * +GI *  *  *  *  * \n"
      "P9+KY *  *  * +KI *  *  * -RY\n"
      "P+00KI00GI00FU00FU\n"
      "P-00KI00GI00GI00KE00KE00FU00FU00FU00FU\n"
      "+\n"
      ).initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(7,2),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(7,6),Square(7,3),PLANCE,PAWN,true,BLACK)));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
      "P2 * -HI *  *  *  * -KI-KA * \n"
      "P3-FU * -FU-FU-FU-FU *  * -FU\n"
      "P4 *  *  *  *  *  * -FU+HI * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 * -FU+FU *  *  *  *  *  * \n"
      "P7+FU+FU * +FU+FU+FU+FU * +FU\n"
      "P8 * +KA+KI *  *  *  *  *  * \n"
      "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
      "P+00FU\n"
      "P-00FU\n"
      "+\n"
      ).initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(5,1),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(2,4),Square(2,3),PROOK,PTYPE_EMPTY,true,BLACK)));
  }

  {
    SimpleState state1=CsaString(
      "P1-KY-KE *  *  * -OU * -KE-KY\n"
      "P2 *  *  *  *  *  * -KI-KA * \n"
      "P3-FU * -FU-FU+HI-GI * +RY * \n"
      "P4 * -FU *  *  *  * +FU-FU-FU\n"
      "P5 *  *  *  * -GI *  *  *  * \n"
      "P6 *  * +FU+FU * +KY *  *  * \n"
      "P7+FU+FU+GI *  * +FU+GI * +FU\n"
      "P8 *  * +KI * +KI *  *  *  * \n"
      "P9+KY+KE * +OU *  *  * +KE * \n"
      "P+00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P+00KI\n"
      "P-00KA\n"
      "+\n").initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(4,1),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(2,3),Square(4,3),PROOK,SILVER,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(4,3),PROOK,SILVER,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,6),Square(4,3),LANCE,SILVER,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(4,6),Square(4,3),PLANCE,SILVER,true,BLACK)));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE *  *  * -OU * -KE-KY\n"
      "P2 *  *  *  *  *  * -KI-KA * \n"
      "P3-FU * -FU-FU-HI-GI * +RY * \n"
      "P4 * -FU *  *  * -FU+FU * -FU\n"
      "P5 *  *  *  * -GI *  *  *  * \n"
      "P6 *  * +FU+FU *  *  *  *  * \n"
      "P7+FU+FU+GI *  * +FU+GI * +FU\n"
      "P8 *  * +KI * +KI *  *  *  * \n"
      "P9+KY+KE * +OU *  *  * +KE+KY\n"
      "P+00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P+00KI\n"
      "P-00KA\n"
      "+\n").initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    // 龍
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,eState,Square(3,1),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(2,3),Square(2,2),PROOK,BISHOP,false,BLACK)) ||
		(std::cerr << moves << std::endl,0));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE * -KI * -OU+GI-KE-KY\n"
      "P2 *  * +RY *  *  * +HI *  * \n"
      "P3-FU * -FU-FU+KI+FU *  *  * \n"
      "P4 * -FU *  * +KE-FU+FU * -FU\n"
      "P5 *  *  *  * -GI-GI *  * +KA\n"
      "P6 *  * +FU+FU *  *  *  *  * \n"
      "P7+FU+FU *  *  *  * +GI * +FU\n"
      "P8 *  * +KI *  *  *  *  *  * \n"
      "P9+KY+KE * +OU *  *  *  * +KY\n"
      "P+00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P+00KI\n"
      "P-00KA\n"
      "+\n").initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    // 龍
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(4,1),store);
    }
    // pawn
    BOOST_CHECK(moves.isMember(Move(Square(4,3),Square(4,2),PPAWN,PTYPE_EMPTY,true,BLACK)));
    // silver
    BOOST_CHECK(moves.isMember(Move(Square(3,1),Square(4,2),SILVER,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,1),Square(4,2),PSILVER,PTYPE_EMPTY,true,BLACK)));
    // KNIGHT
    BOOST_CHECK(moves.isMember(Move(Square(5,4),Square(4,2),PKNIGHT,PTYPE_EMPTY,true,BLACK)));
    // GOLD
    BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(4,2),GOLD,PTYPE_EMPTY,false,BLACK)));
    // ROOK
    BOOST_CHECK(moves.isMember(Move(Square(3,2),Square(4,2),PROOK,PTYPE_EMPTY,true,BLACK)));
    // PROOK
    BOOST_CHECK(moves.isMember(Move(Square(7,2),Square(4,2),PROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE * -KI *  * +GI-KE-KY\n"
      "P2 *  *  *  *  *  *  *  *  * \n"
      "P3-FU+RY-FU *  * +FU *  *  * \n"
      "P4 * +FU *  * -OU *  *  * +KA\n"
      "P5 *  *  *  *  *  *  *  * -FU \n"
      "P6 *  * +FU+FU *  *  *  *  * \n"
      "P7+FU-FU *  * -GI+RY *  * +FU\n"
      "P8 *  * +KI *  *  * +GI+UM-GI\n"
      "P9+KY+KE * +OU+KI *  * +KE+KY\n"
      "P+00FU00FU\n"
      "P-00FU00FU00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P+00KI\n"
      "+\n").initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(5,4),store);
    }
    // 47 PROOK
    BOOST_CHECK(moves.isMember(Move(Square(4,7),Square(4,4),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,7),Square(4,5),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,7),Square(5,6),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(4,7),Square(5,7),PROOK,SILVER,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(4,7),Square(5,8),PROOK,PTYPE_EMPTY,false,BLACK)));

    // 83 PROOK
    BOOST_CHECK(moves.isMember(Move(Square(8,3),Square(7,4),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(8,3),Square(9,4),PROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    SimpleState state1=CsaString(
      "P1-KY-KE * -KI *  * -GI+RY-KY\n"
      "P2 *  *  * -OU *  * -KI *  * \n"
      "P3-FU * -GI-FU-FU-FU *  * -FU\n"
      "P4 *  * -FU *  *  *  *  *  * \n"
      "P5 *  *  *  * +KA *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
      "P8 * +GI+KI *  *  *  *  *  * \n"
      "P9+KY+KE *  * +OU+KI+GI * +KY\n"
      "P+00FU\n"
      "P+00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P-00FU\n"
      "P+00KE\n"
      "P-00KE\n"
      "P-00KA\n"
      "P-00HI\n"
      "+\n").initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,eState,Square(4,1),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(2,1),Square(3,2),PROOK,GOLD,false,BLACK)) ||
		(std::cerr << moves << std::endl,0));
  }
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
  MoveVector moves;
  // ケイマ
  {
    Store store(moves);
    GenerateAddEffectWithEffect::
      generate<false>(BLACK,state,Square(9,2),store);
  }
  BOOST_CHECK(moves.isMember(Move(Square(9,6),Square(8,4),KNIGHT,PTYPE_EMPTY,false,BLACK)));

  // promoteして利きをつける
  moves.clear();
  {
    Store store(moves);
    GenerateAddEffectWithEffect::
      generate<false>(BLACK,state,Square(6,2),store);
  }
  BOOST_CHECK(moves.isMember(Move(Square(7,4),Square(7,3),PPAWN,PTYPE_EMPTY,true,BLACK)));

  // 銀を取って王手
  moves.clear();
  {
    Store store(moves);
    GenerateAddEffectWithEffect::
      generate<true>(BLACK,state,Square(3,1),store);
  }
  BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(4,2),PBISHOP,SILVER,false,BLACK)) ||
	      (std::cerr << moves << std::endl,0)
    );
}


// 開き王手
void AddEffectTestOpen(){
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-OU+KA *  * +RY * -KY\n"
			   "P2 *  *  *  * -KI * +NK *  * \n"
			   "P3-FU * -GI-FU-FU-FU *  * -FU\n"
			   "P4 *  * -FU *  *  *  *  *  * \n"
			   "P5 *  *  *  * +KA *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			   "P8 *  * -NK * +OU *  *  *  * \n"
			   "P9+KY+KE * -HI * +KI+GI * +KY\n"
			   "P+00FU00FU00FU\n"
			   "P-00KI00KI00GI00GI00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(7,1),store);
    }
    // open move
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(5,2),PBISHOP,GOLD,true,BLACK)) ||
		   (std::cerr << moves << std::endl,0)
      );
  }
  {
    SimpleState state1=CsaString(
"P1-KY-KE-GI-KI * -KI-GI-KE-KY\n"
"P2 * -HI *  *  *  *  * -KA * \n"
"P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
"P4 *  *  *  * -OU *  *  *  * \n"
"P5 *  *  *  * +KE *  *  *  * \n"
"P6 *  * +FU *  *  *  *  *  * \n"
"P7+FU+FU * +FU+KY+FU+FU+FU+FU\n"
"P8 * +KA *  * +FU *  * +HI * \n"
"P9+KY+KE+GI+KI+OU+KI+GI *  * \n"
"+\n"
).initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(5,4),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(4,3),KNIGHT,PAWN,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(4,3),PKNIGHT,PAWN,true,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(6,3),KNIGHT,PAWN,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(6,3),PKNIGHT,PAWN,true,BLACK)));
  }
  {
    SimpleState state1=CsaString(
"P1-KY-KE-GI-KI * -KI-GI-KE-KY\n"
"P2 * -HI *  * -FU *  * -KA * \n"
"P3-FU-FU-FU-FU-OU-FU-FU-FU-FU\n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  * +KE *  *  *  * \n"
"P6 *  * +FU *  *  *  *  *  * \n"
"P7+FU+FU * +FU+KY+FU+FU+FU+FU\n"
"P8 * +KA *  * +FU *  * +HI * \n"
"P9+KY+KE+GI+KI+OU+KI+GI *  * \n"
"+\n"
).initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,eState,Square(5,3),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(4,3),KNIGHT,PAWN,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(5,5),Square(6,3),KNIGHT,PAWN,false,BLACK)));
  }
  {
    SimpleState state1=CsaString(
"P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
"P2 * -HI *  *  *  *  * -KA * \n"
"P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
"P4 *  *  *  *  *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  * +FU *  *  *  *  *  * \n"
"P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
"P8 * +KA *  *  *  *  * +HI * \n"
"P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
"-\n"
).initialState();
    NumEffectState eState(state1);
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(WHITE,eState,Square(5,5),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE))||
		   (std::cerr << moves << std::endl,0));
  }
}


void AddEffectTestAdditional(){
  {
    NumEffectState state(CsaString(
			  "P1-KY+RY * +UM *  *  *  * -KY\n"
			  "P2 * -FU *  *  *  * +NK *  * \n"
			  "P3-FU * -GI-FU-FU-FU *  * -FU\n"
			  "P4 * -OU-FU *  *  *  *  *  * \n"
			  "P5 *  *  *  * +KA *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			  "P8 *  * -NK * +OU *  *  *  * \n"
			  "P9+KY+KE * -HI * +KI+GI * +KY\n"
			  "P+00KI00KE00FU00FU\n"
			  "P-00KI00KI00GI00GI00FU00FU\n"
			  "+\n"
			  ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(2,1),store);
    }
    // double count +6143UM
    BOOST_CHECK(moves.isMember(Move(Square(6,1),Square(4,3),PBISHOP,PAWN,false,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			  "P1-KY *  *  *  * -KI * -KE-KY\n"
			  "P2 *  *  *  * -GI *  *  * -OU\n"
			  "P3-FU *  *  * -KI-FU-FU-FU-FU\n"
			  "P4 *  * -FU * -FU *  *  *  * \n"
			  "P5 * -KE * -FU+FU *  * +KA * \n"
			  "P6+FU * +FU *  * +FU+HI * +FU\n"
			  "P7 * +FU * +FU-GI+UM * +FU * \n"
			  "P8 *  * +OU *  *  *  *  *  * \n"
			  "P9+KY+KE * -KI *  *  *  * +KY\n"
			  "P+00HI00KI00KE\n"
			  "P-00GI00GI00FU00FU\n"
			  "+\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(6,9),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(3,3),PROOK,PAWN,true,BLACK))||
		   (std::cerr << moves << std::endl,0));
  }
}

// 追加利き
void AddEffectTestShadow(){
  {
    NumEffectState state(CsaString(
"P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
"P2 *  *  *  *  *  * -KI *  * \n"
"P3-FU * -FU-FU-FU-FU *  * -FU\n"
"P4 *  *  *  *  *  * +HI *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 * -HI+FU *  *  *  *  *  * \n"
"P7+FU *  * +FU+FU+FU+FU * +FU\n"
"P8 * +GI+KI *  *  *  *  *  * \n"
"P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
"P+00KA00FU00FU00FU\n"
"P-00KA00FU00FU\n"
"-\n"
).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(WHITE,state,Square(9,7),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(7,9),BISHOP,WHITE))||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
"P1-KY-KE *  *  * -OU-GI-KE-KY\n"
"P2 *  * -HI * -KI * -KI *  * \n"
"P3-FU * -FU-FU *  *  * -FU * \n"
"P4 *  *  *  * +FU+FU-FU * -FU\n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6 *  * +FU * -KE+GI+FU * +FU\n"
"P7+FU+FU * +FU *  *  *  *  * \n"
"P8 * +GI+KI *  * +KI-UM *  * \n"
"P9+KY+KE * +OU+HI *  *  * +KY\n"
"P+00GI00FU00FU00FU\n"
"P-00KA00FU\n"
"-\n"
			  ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(WHITE,state,Square(5,7),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,8),Square(3,9),PBISHOP,PTYPE_EMPTY,false,WHITE)) ||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
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
			  ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(4,2),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(2,1),Square(1,2),PROOK,PTYPE_EMPTY,false,BLACK)) ||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
			  "P1-KY-KE * -KI-OU * -GI-KE-KY\n"
			  "P2 * -GI *  *  *  * -KI *  * \n"
			  "P3-FU *  * -FU-FU-FU *  * -FU\n"
			  "P4 *  * -FU *  *  *  *  *  * \n"
			  "P5 *  *  *  * +KA *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			  "P8 * +GI+KI *  *  *  *  *  * \n"
			  "P9+KY+KE *  * +OU+KI+GI+KE+KY\n"
			  "P+00HI00FU\n"
			  "P-00HI00KA00FU00FU00FU00FU\n"
			  "-\n"
			  ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(WHITE,state,Square(1,1),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(6,6),BISHOP,WHITE))||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
			  "P1-KY *  *  *  * -KI * -KE-KY\n"
			  "P2 *  *  *  * -GI *  *  * -OU\n"
			  "P3-FU *  *  * -KI-FU-FU-FU-FU\n"
			  "P4 *  * -FU * -FU *  *  *  * \n"
			  "P5 * -KE * -FU+FU *  * +KA * \n"
			  "P6+FU * +FU *  * +FU+HI * +FU\n"
			  "P7 * +FU * +FU-GI-UM * +FU * \n"
			  "P8 *  * +OU *  *  *  *  *  * \n"
			  "P9+KY+KE * +KI *  *  *  * +KY\n"
			  "P+00HI00KI00KE\n"
			  "P-00GI00GI00FU00FU\n"
			  "+\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(6,9),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(3,3),PROOK,PAWN,true,BLACK))||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
			  "P1-OU-KE-KI * +KA *  * +RY-KY\n"
			  "P2-KY-GI+GI *  *  *  *  *  * \n"
			  "P3 * -FU *  *  *  *  *  *  * \n"
			  "P4-FU * -FU+KE * +UM *  *  * \n"
			  "P5 * +FU * -FU * -FU * -FU-FU\n"
			  "P6+FU * +FU * +FU-KY+FU *  * \n"
			  "P7 *  * +KE+FU *  * +KE+FU+FU\n"
			  "P8 *  * -HI *  * -GI+GI+OU * \n"
			  "P9 *  *  *  *  *  * +KI * +KY\n"
			  "P+00KI00FU\n"
			  "P-00KI00FU00FU\n"
			  "+\n"
			  ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(3,9),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(5,1),Square(8,4),PBISHOP,PTYPE_EMPTY,true,BLACK))||
		   (std::cerr << moves << std::endl,0));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
			   "P2 *  *  *  *  *  * -KI-KA * \n"
			   "P3-FU * -FU-FU-FU-FU *  * -FU\n"
			   "P4 *  *  *  *  *  * +HI *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 * -HI+FU *  *  *  *  *  * \n"
			   "P7+FU *  * +FU+FU+FU+FU * +FU\n"
			   "P8 * +KA+KI *  *  *  *  *  * \n"
			   "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
			   "P+00FU00FU00FU\n"
			   "P-00FU00FU\n"
			   "-\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(WHITE,state,Square(2,4),store);
    }
    // shadow attackも生成する．
    BOOST_CHECK(moves.isMember(Move(Square(8,6),Square(8,4),ROOK,PTYPE_EMPTY,false,WHITE)));
  }
}

static bool hasShadowEffect(NumEffectState const& state, PtypeO ptypeO,Square from, Square to)
{
  if(!from.isOnBoard()) return false;
  EffectContent ec=Ptype_Table.getEffect(ptypeO,from,to);
  if(ec.hasBlockableEffect()){
    Offset offset=ec.offset();
    Piece p;
    for(from+=offset;(p=state.pieceAt(from)).isEmpty();from+=offset)
      ;
    if(!p.isEdge() && state.hasEffectByPiece(p,to)) return true;
  }
  return false;
}
// pinされている駒を使う
BOOST_AUTO_TEST_CASE(AddEffectTestPinned) { 
  {
    // actually, 98KY is not a pinned piece
    NumEffectState state(CsaString(
			   "P1 *  *  * -KI-OU *  *  * -KY\n"
			   "P2 *  * -GI * -KI-HI *  *  * \n"
			   "P3 * -FU *  * -FU-GI-KE * -FU\n"
			   "P4 *  * -FU-FU * -KA-FU+HI * \n"
			   "P5 * +KE *  *  * -FU *  *  * \n"
			   "P6 *  * +FU * +FU * +FU *  * \n"
			   "P7-KY+FU * +FU * +FU *  * +FU\n"
			   "P8+KY+GI *  * +KI+GI *  *  * \n"
			   "P9+OU *  * +KI *  *  * +KE+KY\n"
			   "P+00KA00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(9,1),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(9,8),Square(9,7),LANCE,LANCE,false,BLACK)));
  }
  {
    // 88KY is a pinned piece
    NumEffectState state(CsaString(
			   "P1-KY *  *  * -KI-OU *  *  * \n"
			   "P2 *  *  * -GI * -KI-HI *  * \n"
			   "P3-FU * -FU *  * -FU-GI-KE * \n"
			   "P4 *  *  * -FU-FU * -KA-FU+HI\n"
			   "P5 *  * +KE *  *  * -FU *  * \n"
			   "P6 *  * +FU+FU * +FU * +FU * \n"
			   "P7+FU-KY-KA * +FU * +FU *  * \n"
			   "P8 * +KY+GI *  * +KI+GI *  * \n"
			   "P9+OU+KY *  * +KI *  *  * +KE\n"
			   "P+00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(8,1),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(8,8),Square(8,7),LANCE,LANCE,false,BLACK)));
  }
  {
    // 88KY is a pinned piece
    NumEffectState state(CsaString(
			   "P1-KY *  *  * -KI-OU *  *  * \n"
			   "P2 *  *  * -GI * -KI *  *  * \n"
			   "P3-FU * -FU *  * -FU-GI-KE * \n"
			   "P4 *  *  * -FU-FU * -KA-FU+HI\n"
			   "P5 *  * +KE *  *  * -FU *  * \n"
			   "P6 *  * +FU+FU * +FU * +FU * \n"
			   "P7+FU-KY-KA * +FU * +FU *  * \n"
			   "P8+OU+KY-HI *  * +KI+GI *  * \n"
			   "P9 * +KY+GI * +KI *  *  * +KE\n"
			   "P+00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(8,1),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(8,8),Square(8,7),LANCE,LANCE,false,BLACK)));
  }
  {
    // actually, 98KY is not a pinned piece
    NumEffectState state(CsaString(
			   "P1-OU *  * -KI *  *  *  * -KY\n"
			   "P2 *  * -GI * -KI-HI *  *  * \n"
			   "P3 * -FU *  * -FU-GI-KE * -FU\n"
			   "P4 *  * -FU-FU * -KA-FU+HI * \n"
			   "P5 * +KE *  *  * -FU *  *  * \n"
			   "P6 *  * +FU * +FU * +FU *  * \n"
			   "P7-KY+FU * +FU * +FU *  * +FU\n"
			   "P8+KY+GI *  * +KI+GI *  *  * \n"
			   "P9+OU *  * +KI *  *  * +KE+KY\n"
			   "P+00KA00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(9,1),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(9,8),Square(9,7),LANCE,LANCE,false,BLACK)));
  }
  {
    // 88KY is a pinned piece
    NumEffectState state(CsaString(
			   "P1-KY-OU *  * -KI *  *  *  * \n"
			   "P2 *  *  * -GI * -KI-HI *  * \n"
			   "P3-FU * -FU *  * -FU-GI-KE * \n"
			   "P4 *  *  * -FU-FU * -KA-FU+HI\n"
			   "P5 *  * +KE *  *  * -FU *  * \n"
			   "P6 *  * +FU+FU * +FU * +FU * \n"
			   "P7+FU-KY-KA * +FU * +FU *  * \n"
			   "P8 * +KY+GI *  * +KI+GI *  * \n"
			   "P9+OU+KY *  * +KI *  *  * +KE\n"
			   "P+00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(8,1),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(8,8),Square(8,7),LANCE,LANCE,false,BLACK)));
  }
  {
    // 88KY is a pinned piece
    NumEffectState state(CsaString(
			   "P1-KY-OU *  * -KI *  *  *  * \n"
			   "P2 *  *  * -GI * -KI *  *  * \n"
			   "P3-FU * -FU *  * -FU-GI-KE * \n"
			   "P4 *  *  * -FU-FU * -KA-FU+HI\n"
			   "P5 *  * +KE *  *  * -FU *  * \n"
			   "P6 *  * +FU+FU * +FU * +FU * \n"
			   "P7+FU-KY-KA * +FU * +FU *  * \n"
			   "P8+OU+KY-HI *  * +KI+GI *  * \n"
			   "P9 * +KY+GI * +KI *  *  * +KE\n"
			   "P+00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(8,1),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(8,8),Square(8,7),LANCE,LANCE,false,BLACK)));
  }
  // ROOK (attack king)
  {
    // 
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  * -OU * -KY *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  * +HI *  * \n"
			   "P6 *  *  *  *  *  * +OU *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,5),Square(5,5),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  * -OU *  *  *  * \n"
			   "P4 *  *  *  *  *  *  * -KA * \n"
			   "P5 *  *  *  *  *  * +HI *  * \n"
			   "P6 *  *  *  *  * +OU *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,5),Square(5,5),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  * -OU *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  * -HI *  * +HI+OU * \n"
			   "P6 *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,5),Square(5,5),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  * -OU *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * -HI * +HI+OU * \n"
			   "P6 *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,5),Square(5,5),ROOK,ROOK,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * -OU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * -HI *  *  * +HI+OU *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(5,4),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,6),Square(3,4),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  * -HI *  * \n"
			   "P4 *  *  *  * -OU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  * +HI *  *  * \n"
			   "P7 *  *  *  *  *  * +OU *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(5,4),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(3,4),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  // ROOK (not attack king)
  {
    // 
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  * -KY *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  * +HI *  * \n"
			   "P6 *  *  *  *  *  * +OU *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,5),Square(5,5),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  * -KA * \n"
			   "P5 *  *  *  *  *  * +HI *  * \n"
			   "P6 *  *  *  *  * +OU *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,5),Square(5,5),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  * -HI *  * +HI+OU * \n"
			   "P6 *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,5),Square(5,5),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * -HI * +HI+OU * \n"
			   "P6 *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(5,3),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,5),Square(5,5),ROOK,ROOK,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * -HI *  *  * +HI+OU *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(5,4),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,6),Square(3,4),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    // 
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  * -HI *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  * +HI *  *  * \n"
			   "P7 *  *  *  *  *  * +OU *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(5,4),store);
    }
    BOOST_CHECK(moves.isMember(Move(Square(3,6),Square(3,4),ROOK,PTYPE_EMPTY,false,BLACK)));
  }
  // ROOK (attack king)
  {
    // 
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  * -KY *  * \n"
			   "P3 *  *  * -OU *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +RY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  * +OU *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(BLACK,state,Square(6,3),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,4),Square(4,3),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),Square(3,3),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(3,4),Square(2,3),PROOK,PTYPE_EMPTY,false,BLACK)));
  }
  // ROOK (not attack king)
  {
    // 
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  * -KY *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * +RY *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  * +OU *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00AL\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(BLACK,state,Square(6,3),store);
    }
    BOOST_CHECK(!moves.isMember(Move(Square(3,4),Square(4,3),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(moves.isMember(Move(Square(3,4),Square(3,3),PROOK,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(!moves.isMember(Move(Square(3,4),Square(2,3),PROOK,PTYPE_EMPTY,false,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  * -OU * -KI-KE-KY\n"
			   "P2 *  * -GI * -KI-HI *  *  * \n"
			   "P3 * -FU * -FU-FU-GI+UM-FU-FU\n"
			   "P4 *  * -FU *  *  * -FU *  * \n"
			   "P5-FU *  *  *  * -FU * +FU * \n"
			   "P6 *  * +FU * +FU * +FU *  * \n"
			   "P7+KE+FU * +FU * +FU *  * +FU\n"
			   "P8+KY *  *  * +KI+GI * +HI * \n"
			   "P9+OU * +GI+KI *  *  * +KE+KY\n"
			   "P+00KA00KE\n"
			   "P-00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    Store store(moves);
    GenerateAddEffectWithEffect::
      generate<false>(state.turn(),state,Square(3,1),store);
    BOOST_CHECK(!moves.isMember(Move(Square(4,2),Square(4,1),ROOK,PTYPE_EMPTY,false,WHITE)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  * -KI-OU *  * -KE-KY\n"
			   "P2 *  * -GI * -KI-HI *  *  * \n"
			   "P3 * -FU * -FU-FU-GI+UM-FU-FU\n"
			   "P4 *  * -FU *  *  * -FU *  * \n"
			   "P5-FU *  *  *  * -FU * +FU * \n"
			   "P6 *  * +FU * +FU * +FU *  * \n"
			   "P7+KE+FU * +FU * +FU *  * +FU\n"
			   "P8+KY *  *  * +KI+GI * +HI * \n"
			   "P9+OU * +GI+KI *  *  * +KE+KY\n"
			   "P+00KA00KE\n"
			   "P-00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(state.turn(),state,Square(6,1),store);
    }
    for (Move move: moves) {
      BOOST_CHECK(state.isSafeMove(move));
    }
  }
}

static bool isAddEffectWithEffectMove(const NumEffectState& state_org,Square pos,Move move){
  // 自分で動いて元のpositionに利きをつける手は含まない
  if(move.from()==pos) return false;
  bool shadowFlag=hasShadowEffect(state_org,move.ptypeO(),move.from(),pos);
  NumEffectState state = state_org;
  state.makeMove(move);
  bool flag=state.hasEffectAt(move.player(),pos);
  // 打ち歩詰めはfalseを返せ
  // 本当は相手が取りかえせない場所
  if(flag && state.pieceAt(pos).ptype()==KING &&
     state.pieceAt(pos).owner() != move.player() &&
     move.isDrop() && move.ptype()==PAWN &&
     !state.hasMultipleEffectAt(alt(move.player()),move.to()) &&
     King8Info(state.Iking8Info(alt(move.player()))).liberty()==0)
    return false;
  if(!flag && state.pieceAt(pos).ptype()!=KING){ // additional or shadow
    // check direct
    if(!shadowFlag && hasShadowEffect(state,move.ptypeO(),move.to(),pos))
      flag=true;
    // check open
    Square pos1=move.from();
    if(pos1.isOnBoard()){
      Offset offset=Board_Table.getShortOffset(Offset32(pos1,pos));
      if(!offset.zero()){
	Piece p;
	Square pos2;
	for(pos2=pos+offset;(p=state.pieceAt(pos2)).isEmpty();pos2+=offset)
	  ;
	if(!p.isEdge() && state.hasEffectByPiece(p,pos)){
	  for(pos2+=offset;pos2!=pos1 && (p=state.pieceAt(pos2)).isEmpty();pos2+=offset)
	    ;
	  if(pos2==pos1){
	    for(pos2+=offset;(p=state.pieceAt(pos2)).isEmpty();pos2+=offset)
	      ;
	    if(pos2!=move.to() && p.isOnBoardByOwner(move.player()) &&
	       Ptype_Table.getEffect(p.ptypeO(),pos2,pos).hasEffect())
	      flag=true;
	  }
	}
      }
    }
  }
  return flag
    && (! state.inCheck(alt(state.turn()))); // 自殺
}

template<bool KingOnly>
static void testMoveFile(const std::string& filename){
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initial_state);
  for (auto record_move:record.moves) {
    if (! state.inCheck()) { // 王手以外の状況でテスト
      MoveVector all;
      state.generateAllUnsafe(all);
      for(int y=1;y<=9;y++)
	for(int x=9;x>0;x--){
	  Square pos(x,y);
	  MoveVector moves;
	  if(pos==state.kingSquare(alt(state.turn()))){
	    {
	      Store store(moves);
	      GenerateAddEffectWithEffect::
		generate<true>(state.turn(),state,pos,store);
	    }
	    for (auto m:moves) {
	      BOOST_CHECK(state.isSafeMove(m));
	      BOOST_CHECK(!m.ignoreUnpromote());
	      BOOST_CHECK(std::count(moves.begin(),moves.end(),m) == 1);
	    }
	  }
	  else{
	    if(KingOnly) continue;
	    {
	      Store store(moves);
	      GenerateAddEffectWithEffect::
		generate<false>(state.turn(),state,pos,store);
	    }
	  }
	  moves.unique();
	
	  size_t count1=0;
	  for (Move m:moves) {
	    BOOST_CHECK(state.isSafeMove(m));
	    BOOST_CHECK(!m.ignoreUnpromote());
	    BOOST_CHECK((state.isValidMove(m,true)  
			 && isAddEffectWithEffectMove(state,pos,m)));
	    if(!ShouldPromoteCut::canIgnoreAndNotDrop(m)) count1++;
	  }
	  if(state.hasEffectAt(state.turn(),pos)) continue;
	  // 
	  size_t count=0;
	  MoveVector tmp;
	  for (Move m:all) {
	    if (isAddEffectWithEffectMove(state,pos,m)) {
	      tmp.push_back(m);
	      if (state.isPawnDropCheckmate(m))
		continue;
	      count++;
	      BOOST_CHECK(moves.isMember(m));
	    }
	  }
	  BOOST_CHECK(count == count1);
	}
    }
    state.makeMove(record_move);
  }
}

BOOST_AUTO_TEST_CASE(AddEffectTestMoveMember){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=10000;
  if (OslConfig::inUnitTestShort()) 
    count=2;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename != "" && ++i<count){
    if (progress)
      ++(*progress);
    testMoveFile<false>(OslConfig::testCsaFile(filename));
  }
}

BOOST_AUTO_TEST_CASE(AddEffectTestMoveMemberCheck){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=10000;
  if (OslConfig::inUnitTestShort()) 
    count=100;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  std::string filename;
  while((ifs >> filename) && filename != "" && ++i<count){
    if (progress)
      ++(*progress);
    testMoveFile<true>(OslConfig::testCsaFile(filename));
  }
}


BOOST_AUTO_TEST_CASE(AddEffectTestBug20110116)
{
  NumEffectState state(CsaString(
			 "P1-KY *  * -KI-OU *  * -KE-KY\n"
			 "P2 *  * -GI * -KI-HI *  *  * \n"
			 "P3 * -FU * -FU-FU-GI+UM-FU-FU\n"
			 "P4 *  * -FU *  *  * -FU *  * \n"
			 "P5-FU *  *  *  * -FU * +FU * \n"
			 "P6 *  * +FU * +FU * +FU *  * \n"
			 "P7+KE+FU * +FU * +FU *  * +FU\n"
			 "P8+KY *  *  * +KI+GI * +HI * \n"
			 "P9+OU * +GI+KI *  *  * +KE+KY\n"
			 "P+00KA00KE\n"
			 "P-00FU\n"
			 "-\n").initialState());
  MoveVector moves;
  {
    Store store(moves);
    GenerateAddEffectWithEffect::
      generate<false>(state.turn(),state,Square(6,1),store);
  }
  for (Move move: moves) {
    BOOST_CHECK(state.isSafeMove(move));
  }
}
BOOST_AUTO_TEST_CASE(AddEffectTestBug20110121)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  * +KA-KY *  * \n"
			   "P2 * -HI *  * +FU * -KE *  * \n"
			   "P3 *  * -FU-GI-FU *  * -OU-KI\n"
			   "P4-FU *  * -FU * -FU-GI *  * \n"
			   "P5 * -FU+FU *  *  *  *  * -KY\n"
			   "P6 *  *  *  *  *  * -FU *  * \n"
			   "P7+FU+FU+KE+FU * +FU+FU+FU+HI\n"
			   "P8 * +GI *  * +KI *  *  *  * \n"
			   "P9+KY *  *  *  *  *  *  * +OU\n"
			   "P+00KA00KI00KE\n"
			   "P-00KI00GI00FU00FU00FU\n"
			   "-\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<true>(WHITE,state,Square(1,9),store);
    }
    // open move
    BOOST_CHECK(moves.isMember(Move(Square(1,5),Square(1,7),LANCE,ROOK,false,WHITE)));
    BOOST_CHECK(!moves.isMember(Move(Square(1,5),Square(1,7),PLANCE,ROOK,true,WHITE)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  * +KA-KY *  * \n"
			   "P2 * -HI *  * +FU * -KE *  * \n"
			   "P3 *  * -FU-GI-FU *  * -OU-KI\n"
			   "P4-FU *  * -FU * -FU-GI *  * \n"
			   "P5 * -FU+FU *  *  *  *  * -KY\n"
			   "P6 *  *  *  *  *  * -FU *  * \n"
			   "P7+FU+FU+KE+FU * +FU+FU+FU+HI\n"
			   "P8 * +GI *  * +KI *  *  *  * \n"
			   "P9+KY *  *  *  *  * +OU *  * \n"
			   "P+00KA00KI00KE\n"
			   "P-00KI00GI00FU00FU00FU\n"
			   "-\n"
			   ).initialState());
    MoveVector moves;
    {
      Store store(moves);
      GenerateAddEffectWithEffect::
	generate<false>(WHITE,state,Square(1,9),store);
    }
    // open move
    BOOST_CHECK(moves.isMember(Move(Square(1,5),Square(1,7),LANCE,ROOK,false,WHITE)));
    BOOST_CHECK(!moves.isMember(Move(Square(1,5),Square(1,7),PLANCE,ROOK,true,WHITE)));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
