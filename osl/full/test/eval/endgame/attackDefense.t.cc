#include "osl/eval/endgame/attackDefense.h"
#include "osl/eval/evalTraits.h"
#include "osl/eval/pieceEval.h"
#include "osl/csa.h"
#include "../consistencyTest.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::endgame;

BOOST_AUTO_TEST_CASE(EndgameAttackDeffenseTestInitial)
{
  const NumEffectState state(CsaString(
			       "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			       "P2 * -HI *  *  *  *  * -KA * \n"
			       "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			       "P4 *  *  *  *  *  *  *  *  * \n"
			       "P5 *  *  *  *  *  *  *  *  * \n"
			       "P6 *  *  *  *  *  *  *  *  * \n"
			       "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			       "P8 * +KA *  *  *  *  * +HI * \n"
			       "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			       "+\n").initialState());
  AttackDefense eval(state);
  BOOST_CHECK_EQUAL(0, eval.value());
}

BOOST_AUTO_TEST_CASE(EndgameAttackDeffenseTestMax) 
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0; i<(OslConfig::inUnitTestShort() ? 10 : 100) && (ifs >> file_name) ; i++)
  {
    if ((i % 20) == 0)
      std::cerr << '.';
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const auto record=CsaFileMinimal(file_name).load();
    const auto& moves=record.moves;

    NumEffectState state(record.initialState());
    for (unsigned int i=0; i<moves.size(); i++)
    {
      const Move m = moves[i];
      state.makeMove(m);

      for (int j=0; j<Piece::SIZE; ++j)
      {
	const Piece target = state.pieceOf(j);
	const Piece black_king = state.kingPiece<BLACK>();
	const Piece white_king = state.kingPiece<WHITE>();

	const int attack_black = AttackKing::valueOf(black_king, target);
	const int attack_white = AttackKing::valueOf(white_king, target);
	BOOST_CHECK((attack_black == 0) || (attack_white == 0));

	const int defense_black = DefenseKing::valueOf(black_king, target);
	const int defense_white = DefenseKing::valueOf(white_king, target);
	BOOST_CHECK((defense_black == 0) || (defense_white == 0));
	
	const int attack_defense 
	  = AttackDefense::valueOf(black_king, white_king, target);
	BOOST_CHECK_EQUAL(max(target.owner(), 
				 attack_black  + attack_white,
				 defense_black + defense_white),
			     attack_defense);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(EndgameAttackDeffenseTestStand) 
{
  const Square king_position(5,5);
  const Piece black_king = Piece::makeKing(BLACK, king_position);
  const Piece white_king = Piece::makeKing(WHITE, king_position);

  const int black_value
    = AttackDefense::valueOf(black_king, white_king, newPtypeO(BLACK,GOLD), 
			     Square::STAND());
  BOOST_CHECK_EQUAL(static_cast<int>(Ptype_Eval_Table.value(GOLD) * 1.1),
		       black_value);

  const int white_value
    = AttackDefense::valueOf(black_king, white_king, newPtypeO(WHITE,GOLD), 
			     Square::STAND());
  BOOST_CHECK_EQUAL(static_cast<int>(-Ptype_Eval_Table.value(GOLD) * 1.1),
		       white_value);

  const int black_rook
    = AttackDefense::valueOf(black_king, white_king, newPtypeO(BLACK,ROOK), 
			     Square::STAND());
  BOOST_CHECK_EQUAL(Ptype_Eval_Table.value(ROOK) +
		       Ptype_Eval_Table.value(PAWN), black_rook);

  const int white_rook
    = AttackDefense::valueOf(black_king, white_king, newPtypeO(WHITE,ROOK), 
			     Square::STAND());
  BOOST_CHECK_EQUAL(-Ptype_Eval_Table.value(ROOK) -
		       Ptype_Eval_Table.value(PAWN), white_rook);
}

BOOST_AUTO_TEST_CASE(EndgameAttackDeffenseTestConsistentUpdate)
{
  consistencyTestUpdate<AttackDefense>();
}

BOOST_AUTO_TEST_CASE(EndgameAttackDeffenseTestConsistentExpect)
{
  consistencyTestExpect<AttackDefense>();
}

BOOST_AUTO_TEST_CASE(EndgameAttackDeffenseTestYagura)
{
  {
    NumEffectState in_castle(CsaString(
			       "P1-KY-KE *  *  *  *  * -KE-KY\n"
			       "P2 * -HI *  *  *  * -KI-OU * \n"
			       "P3 *  * -GI-FU-KA-KI-GI-FU * \n"
			       "P4-FU * -FU * -FU-FU-FU * -FU\n"
			       "P5 * -FU *  *  *  *  * +FU * \n"
			       "P6+FU * +FU+FU+FU * +FU * +FU\n"
			       "P7 * +FU+GI+KI+KA+FU+GI *  * \n"
			       "P8 * +OU+KI *  *  *  * +HI * \n"
			       "P9+KY+KE *  *  *  *  * +KE+KY\n"
			       "+\n").initialState());
    NumEffectState out_castle1(CsaString(
				 "P1-KY-KE *  *  *  *  * -KE-KY\n"
				 "P2 * -HI *  *  *  * -KI-OU * \n"
				 "P3 *  * -GI-FU-KA-KI-GI-FU * \n"
				 "P4-FU * -FU * -FU-FU-FU * -FU\n"
				 "P5 * -FU *  *  *  *  * +FU * \n"
				 "P6+FU * +FU+FU+FU * +FU * +FU\n"
				 "P7 * +FU+GI+KI+KA+FU+GI *  * \n"
				 "P8 *  * +KI *  *  *  * +HI * \n"
				 "P9+KY+KE+OU *  *  *  * +KE+KY\n"
				 "+\n").initialState());
    NumEffectState out_castle2(CsaString(
				 "P1-KY-KE *  *  *  *  * -KE-KY\n"
				 "P2 * -HI *  *  *  * -KI-OU * \n"
				 "P3 *  * -GI-FU-KA-KI-GI-FU * \n"
				 "P4-FU * -FU * -FU-FU-FU * -FU\n"
				 "P5 * -FU *  *  *  *  * +FU * \n"
				 "P6+FU * +FU+FU+FU * +FU * +FU\n"
				 "P7 * +FU+GI+KI+KA+FU+GI *  * \n"
				 "P8 *  * +KI *  *  *  * +HI * \n"
				 "P9+KY+KE * +OU *  *  * +KE+KY\n"
				 "+\n").initialState());
    NumEffectState out_castle3(CsaString(
				 "P1-KY-KE *  *  *  *  * -KE-KY\n"
				 "P2 * -HI *  *  *  * -KI-OU * \n"
				 "P3 *  * -GI-FU-KA-KI-GI-FU * \n"
				 "P4-FU * -FU * -FU-FU-FU * -FU\n"
				 "P5 * -FU *  *  *  *  * +FU * \n"
				 "P6+FU * +FU+FU+FU * +FU * +FU\n"
				 "P7 * +FU+GI+KI+KA+FU+GI *  * \n"
				 "P8 *  * +KI *  *  *  * +HI * \n"
				 "P9+KY+KE * +OU *  *  * +KE+KY\n"
				 "+\n").initialState());
    AttackDefense in(in_castle), out1(out_castle1), out2(out_castle2),
      out3(out_castle3);
    BOOST_CHECK(in.value(BLACK) > out1.value(BLACK));
    BOOST_CHECK(out1.value(BLACK) > out2.value(BLACK));
    BOOST_CHECK(out1.value(BLACK) > out3.value(BLACK));
    BOOST_CHECK(in.value() - out1.value() > -100); // 本当は左辺大にしたいけど難しい
    BOOST_CHECK(out1.value() > out2.value());
    BOOST_CHECK(out1.value() > out3.value());
  }
}

BOOST_AUTO_TEST_CASE(EndgameAttackDeffenseTestBlockingSilver)
{
  {
    NumEffectState castle1(CsaString(
			     "P1-KY-KE *  *  *  *  * -KE-KY\n"
			     "P2 * -HI *  *  *  * -KI-OU * \n"
			     "P3 *  * -GI-FU-KA-KI-GI-FU * \n"
			     "P4-FU * -FU * -FU-FU-FU * -FU\n"
			     "P5 * -FU *  *  *  *  * +FU * \n"
			     "P6+FU * +FU+FU+FU * +FU * +FU\n"
			     "P7 * +FU+GI+KI+KA+FU+GI *  * \n"
			     "P8 *  * +KI *  *  *  * +HI * \n"
			     "P9+KY+KE+OU *  *  *  * +KE+KY\n"
			     "+\n").initialState());
    NumEffectState bad_castle(CsaString(
			     "P1-KY-KE *  *  *  *  * -KE-KY\n"
			     "P2 * -HI *  *  *  * -KI-OU * \n"
			     "P3 *  * -GI-FU-KA-KI-GI-FU * \n"
			     "P4-FU * -FU * -FU-FU-FU * -FU\n"
			     "P5 * -FU *  *  *  *  * +FU * \n"
			     "P6+FU * +FU+FU+FU * +FU * +FU\n"
			     "P7 * +FU * +KI+KA+FU+GI *  * \n"
			     "P8 * +GI+KI *  *  *  * +HI * \n"
			     "P9+KY+KE+OU *  *  *  * +KE+KY\n"
			     "+\n").initialState());
    AttackDefense normal(castle1), bad(bad_castle);
    BOOST_CHECK(normal.value(BLACK) > bad.value(BLACK));
    BOOST_CHECK(normal.value() > bad.value());
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
