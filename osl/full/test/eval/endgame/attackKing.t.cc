#include "osl/eval/endgame/attackKing.h"
#include "osl/eval/evalTraits.h"
#include "osl/eval/pieceEval.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <iomanip>
using namespace osl;
using namespace osl::eval;
using namespace osl::eval::endgame;

BOOST_AUTO_TEST_CASE(EndgameAttackKingTestSymmetry)
{
  for (int king_x=1; king_x <= 9; ++king_x) {
    for (int king_y=1; king_y <= 9; ++king_y) {
      const Square bkp(king_x, king_y);
      const Piece king_b = Piece::makeKing(BLACK, bkp);
      const Piece king_w = Piece::makeKing(WHITE, bkp.rotate180());

      for (int ptype=PTYPE_PIECE_MIN; ptype<=PTYPE_MAX; ++ptype) {
	const PtypeO ptypeo_b = newPtypeO(BLACK, static_cast<Ptype>(ptype));
	const PtypeO ptypeo_w = newPtypeO(WHITE, static_cast<Ptype>(ptype));
	for (int x=1; x <= 9; ++x) {
	  for (int y=1; y <= 9; ++y) {
	    const Square position_b(x,y);
	    const Square position_w = position_b.rotate180();

	    if (king_w.square() == position_b)
	      continue;

	    const int value_b=AttackKing::valueOf(king_w, ptypeo_b, position_b);
	    const int value_w=AttackKing::valueOf(king_b, ptypeo_w, position_w);
	    if (value_b != -value_w) {
	      std::cerr << king_w << " " << ptypeo_b << " " << position_b
			<< std::endl;
	      std::cerr << king_b << " " << ptypeo_w << " " << position_w
			<< std::endl;
	    }
	    BOOST_CHECK_EQUAL(value_b, -value_w);
	  }
	}
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(EndgameAttackKingTestPawnLance)
{
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const Square p58(5,8);
  const Square p52(5,2);
  // 1つしか動けない香は歩と同じ点数
  BOOST_CHECK_EQUAL(AttackKing::valueOf(black_king, newPtypeO(WHITE,PAWN), 
					   p58),
		       AttackKing::valueOf(black_king, newPtypeO(WHITE,LANCE), 
					   p58));
  BOOST_CHECK(AttackKing::valueOf(black_king, newPtypeO(WHITE,PAWN), p52)
		 > AttackKing::valueOf(black_king, newPtypeO(WHITE,LANCE), p52));
  BOOST_CHECK_EQUAL(AttackKing::valueOf(black_king, newPtypeO(BLACK,PAWN), 
					   p52),
		       AttackKing::valueOf(black_king, newPtypeO(BLACK,LANCE), 
					   p52));
  BOOST_CHECK(AttackKing::valueOf(black_king, newPtypeO(BLACK,PAWN), p58)
		<= AttackKing::valueOf(black_king, newPtypeO(BLACK,LANCE), p58));

  // 後手玉でも同じ
  BOOST_CHECK_EQUAL(AttackKing::valueOf(white_king, newPtypeO(WHITE,PAWN), 
					   p58),
		       AttackKing::valueOf(white_king, newPtypeO(WHITE,LANCE), 
					   p58));
  BOOST_CHECK(AttackKing::valueOf(white_king, newPtypeO(WHITE,PAWN), p52)
		 >= AttackKing::valueOf(white_king, newPtypeO(WHITE,LANCE), p52));
  BOOST_CHECK_EQUAL(AttackKing::valueOf(white_king, newPtypeO(BLACK,PAWN), 
					   p52),
		       AttackKing::valueOf(white_king, newPtypeO(BLACK,LANCE), 
					   p52));
  BOOST_CHECK(AttackKing::valueOf(white_king, newPtypeO(BLACK,PAWN), p58)
		 < AttackKing::valueOf(white_king, newPtypeO(BLACK,LANCE), p58));
}

BOOST_AUTO_TEST_CASE(EndgameAttackKingTestGold)
{
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const Square p51(5,1);
  const Square p52(5,2);
  const Square p58(5,8);
  const Square p59(5,9);
  // 1段目減点
  BOOST_CHECK(AttackKing::valueOf(black_king, newPtypeO(WHITE,GOLD), p58)
		 < AttackKing::valueOf(black_king, newPtypeO(WHITE,GOLD), p59));
  BOOST_CHECK(AttackKing::valueOf(white_king, newPtypeO(BLACK,GOLD), p52)
		 > AttackKing::valueOf(white_king, newPtypeO(BLACK,GOLD), p51));
}

static void testDirection(Ptype ptype)
{
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const Square up(4,4);
  const Square down(4,6);

  // 上から抑える方が点数が高い
  const int white_attack_front
    = AttackKing::valueOf(black_king, newPtypeO(WHITE,ptype), up);
  const int white_attack_back
    = AttackKing::valueOf(black_king, newPtypeO(WHITE,ptype), down);

  const int black_attack_front 
    = AttackKing::valueOf(white_king, newPtypeO(BLACK,ptype), down);
  const int black_attack_back
    = AttackKing::valueOf(white_king, newPtypeO(BLACK,ptype), up);

  BOOST_CHECK(betterThan(WHITE, white_attack_front, white_attack_back));
  BOOST_CHECK(betterThan(BLACK, black_attack_front, black_attack_back));

  // 対称性
  BOOST_CHECK_EQUAL(black_attack_front, -white_attack_front);
  BOOST_CHECK_EQUAL(black_attack_back,  -white_attack_back);
}

BOOST_AUTO_TEST_CASE(EndgameAttackKingTestStand) {
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const int black_attack
    = AttackKing::valueOf(white_king, newPtypeO(BLACK,GOLD), 
			  Square::STAND());
  BOOST_CHECK_EQUAL(static_cast<int>(Ptype_Eval_Table.value(GOLD) * 1.1),
		       black_attack);

  const int white_attack
    = AttackKing::valueOf(black_king, newPtypeO(WHITE,GOLD), 
			  Square::STAND());
  BOOST_CHECK_EQUAL(static_cast<int>(-Ptype_Eval_Table.value(GOLD) * 1.1),
		       white_attack);
}


BOOST_AUTO_TEST_CASE(EndgameAttackKingTestShow)
{
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);
  // debug 用
  for (int y=1; y<=9; ++y) 
  {
    for (int x=1; x<=9; ++x) 
    {
      std::cerr << std::setw(4) 
		<< AttackKing::valueOf(black_king, newPtypeO(WHITE,PAWN), 
				       Square(x,y));
    }
    std::cerr << "\n";
  }    
  std::cerr << "\n";
    
  for (int y=1; y<=9; ++y) 
  {
    for (int x=1; x<=9; ++x) 
    {
      std::cerr << std::setw(4) 
		<< AttackKing::valueOf(white_king, newPtypeO(BLACK,PAWN), 
				       Square(x,y));
    }
    std::cerr << "\n";
  }    
}

BOOST_AUTO_TEST_CASE(EndgameAttackKingTestSave)
{
  AttackKing::saveText("EndgameAttackKingTest.txt");
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
