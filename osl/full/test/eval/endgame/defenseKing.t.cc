#include "osl/eval/endgame/defenseKing.h"
#include "osl/eval/evalTraits.h"
#include "osl/eval/pieceEval.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::endgame;

BOOST_AUTO_TEST_CASE(EndgameDefenseKingTestSymmetry)
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

	    if (king_b.square() == position_b)
	      continue;

	    const int value_b=DefenseKing::valueOf(king_b, ptypeo_b, position_b);
	    const int value_w=DefenseKing::valueOf(king_w, ptypeo_w, position_w);
	    if (value_b != -value_w) {
	      std::cerr << king_b << " " << ptypeo_b << " " << position_b
			<< std::endl;
	      std::cerr << king_w << " " << ptypeo_w << " " << position_w
			<< std::endl;
	    }
	    BOOST_CHECK_EQUAL(value_b, -value_w);
	  }
	}
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(EndgameDefenseKingTestPawnLance)
{
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const Square p58(5,8);
  const Square p52(5,2);
  // 1つしか動けない香は歩と同じ点数
  BOOST_CHECK_EQUAL(DefenseKing::valueOf(black_king, newPtypeO(WHITE,PAWN), 
					   p58),
		       DefenseKing::valueOf(black_king, newPtypeO(WHITE,LANCE), 
					   p58));
  BOOST_CHECK(DefenseKing::valueOf(black_king, newPtypeO(WHITE,PAWN), p52)
		 >= DefenseKing::valueOf(black_king, newPtypeO(WHITE,LANCE), p52));
  BOOST_CHECK_EQUAL(DefenseKing::valueOf(black_king, newPtypeO(BLACK,PAWN), 
					   p52),
		       DefenseKing::valueOf(black_king, newPtypeO(BLACK,LANCE), 
					   p52));
  BOOST_CHECK(DefenseKing::valueOf(black_king, newPtypeO(BLACK,PAWN), p58)
		< DefenseKing::valueOf(black_king, newPtypeO(BLACK,LANCE), p58));

  // 後手玉でも同じ
  BOOST_CHECK_EQUAL(DefenseKing::valueOf(white_king, newPtypeO(WHITE,PAWN), 
					   p58),
		       DefenseKing::valueOf(white_king, newPtypeO(WHITE,LANCE), 
					   p58));
  BOOST_CHECK(DefenseKing::valueOf(white_king, newPtypeO(WHITE,PAWN), p52)
		 > DefenseKing::valueOf(white_king, newPtypeO(WHITE,LANCE), p52));
  BOOST_CHECK_EQUAL(DefenseKing::valueOf(white_king, newPtypeO(BLACK,PAWN), 
					   p52),
		       DefenseKing::valueOf(white_king, newPtypeO(BLACK,LANCE), 
					   p52));
  BOOST_CHECK(DefenseKing::valueOf(white_king, newPtypeO(BLACK,PAWN), p58)
		 <= DefenseKing::valueOf(white_king, newPtypeO(BLACK,LANCE), p58));
}

BOOST_AUTO_TEST_CASE(EndgameDefenseKingTestKing)
{
  const Square p11(1,1);
  const Square p19(1,9);
  const Piece black_king11 = Piece::makeKing(BLACK, p11);
  const Piece black_king19 = Piece::makeKing(BLACK, p19);
  BOOST_CHECK(EvalTraits<BLACK>::betterThan
		 (DefenseKing::valueOf(black_king11, black_king11),
		  DefenseKing::valueOf(black_king19, black_king19)));

  const Piece white_king11 = Piece::makeKing(WHITE, p11);
  const Piece white_king19 = Piece::makeKing(WHITE, p19);
  BOOST_CHECK(EvalTraits<WHITE>::betterThan
		 (DefenseKing::valueOf(white_king19, white_king19),
		  DefenseKing::valueOf(white_king11, white_king11)));
}

static void directionTest(Ptype ptype, bool up_is_better)
{
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const Square up(4,4);
  const Square down(4,6);

  const int black_defense_front 
    = DefenseKing::valueOf(black_king, newPtypeO(BLACK,ptype), up);
  const int black_defense_back
    = DefenseKing::valueOf(black_king, newPtypeO(BLACK,ptype), down);

  const int white_defense_front
    = DefenseKing::valueOf(white_king, newPtypeO(WHITE,ptype), down);
  const int white_defense_back
    = DefenseKing::valueOf(white_king, newPtypeO(WHITE,ptype), up);

  if (up_is_better)
  {
    BOOST_CHECK(betterThan(WHITE, white_defense_front, white_defense_back));
    BOOST_CHECK(betterThan(BLACK, black_defense_front, black_defense_back));
  }
  else
  {
    BOOST_CHECK(! betterThan(WHITE, white_defense_front, white_defense_back));
    BOOST_CHECK(! betterThan(BLACK, black_defense_front, black_defense_back));
  }

  // 対称性
  BOOST_CHECK_EQUAL(black_defense_front, -white_defense_front);
  BOOST_CHECK_EQUAL(black_defense_back,  -white_defense_back);
}
  

BOOST_AUTO_TEST_CASE(EndgameDefenseKingTestStand) {
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const int black_defense
    = DefenseKing::valueOf(black_king, newPtypeO(BLACK,GOLD), 
			   Square::STAND());
  BOOST_CHECK_EQUAL(Ptype_Eval_Table.value(GOLD), black_defense);

  const int white_defense
    = DefenseKing::valueOf(white_king, newPtypeO(WHITE,GOLD), 
			   Square::STAND());
  BOOST_CHECK_EQUAL(-Ptype_Eval_Table.value(GOLD), white_defense);
}

BOOST_AUTO_TEST_CASE(EndgameDefenseKingTestSave)
{
  DefenseKing::saveText("EndgameDefenseKingTest.txt");
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
