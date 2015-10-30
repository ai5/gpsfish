#include "osl/search/quiescenceGenerator.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/effect_util/pin.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestPromote)
{
  {
    NumEffectState state(CsaString(
			   "P1+HI *  *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  * -FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7-FU-GI * +GI *  *  *  *  * \n"
			   "P8 * -FU *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::promote(state, PieceMask(), moves);
    // 98成銀をcheck は省くのでpromoteで生成していないと読まない
    BOOST_CHECK_EQUAL((size_t)3, moves.size());     
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  *  * -KY\n"
			   "P2 * -HI *  *  * -GI-KI-OU * \n"
			   "P3 *  * -KE * -KA-KI-KE-FU+FU\n"
			   "P4 * -GI-FU-FU-FU-FU *  *  * \n"
			   "P5-FU-FU *  *  *  * -FU+FU+HI\n"
			   "P6 *  * +FU+FU+FU+FU *  *  * \n"
			   "P7+FU+FU+GI+KI+GI * +KE *  * \n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00FU\n"
			   "P-00KA00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::promote(state, PieceMask(), moves);
    BOOST_CHECK_EQUAL((size_t)1, moves.size()); // 12TO
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestEscapeFromLastMove)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  * -OU * -KE-KY\n"
			   "P2 * -HI *  *  *  * -KI *  * \n"
			   "P3-FU *  * -FU * -KI-GI-FU-FU\n"
			   "P4 *  *  *  * -FU-FU-FU *  * \n"
			   "P5 *  *  *  *  *  *  * +FU * \n"
			   "P6 * -KA * +FU+FU+FU+FU *  * \n"
			   "P7+FU+FU * +KI+KA+GI+KE * +FU\n"
			   "P8 *  * +KI *  *  *  * +HI * \n"
			   "P9+KY+KE * +OU *  *  *  * +KY\n"
			   "P+00GI00FU\n"
			   "P-00GI00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    Move last_move(Square(8,7),PAWN,BLACK);
    QuiescenceGenerator<WHITE>::escapeFromLastMove<PieceEval>(state, last_move, moves);
    // 角逃げろ
    BOOST_CHECK_EQUAL((size_t)6, moves.size());     
  }
  {
    NumEffectState state(CsaString(
			   "P1+TO *  *  *  * -OU * -KE-KY\n"
			   "P2 *  *  *  *  *  * -KI-KA * \n"
			   "P3-FU * -KE-FU * -KI-GI-FU-FU\n"
			   "P4 *  * -FU-GI * -FU-FU *  * \n"
			   "P5 * -HI *  *  *  *  * +FU * \n"
			   "P6 * +KY+FU+FU * +FU+FU *  * \n"
			   "P7+FU * +GI+KI * +GI *  * +FU\n"
			   "P8 *  * +OU+KA *  *  * +HI * \n"
			   "P9+KY+KE *  *  *  *  * +KE+KY\n"
			   "P+00FU\n"
			   "P-00KI00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    Move last_move(Square(8,6),LANCE,BLACK);
    QuiescenceGenerator<WHITE>::escapeFromLastMove<PieceEval>(state, last_move, moves);
    // 飛車逃げろ
    BOOST_CHECK_EQUAL((size_t)2, moves.size()); // 95,55
  }
  {
    NumEffectState state(CsaString(
           "P1-KY-KE * -HI * -OU *  * -KY\n"
           "P2 *  *  *  * -KI-GI-KI *  * \n"
           "P3-FU * -FU *  * -FU-KE-FU-FU\n"
           "P4 * -FU *  * -HI *  *  *  * \n"
           "P5 *  *  *  * +KA *  *  *  * \n"
           "P6 *  * +FU+FU *  *  *  *  * \n"
           "P7+FU+FU+GI+KI * +FU * +FU+FU\n"
           "P8 *  * +KI *  * +GI *  *  * \n"
           "P9+KY+KE *  * +OU *  * +KE+KY\n"
           "P+00KA00GI00FU\n"
           "P-00FU00FU00FU00FU\n"
	   "+\n").initialState());
    MoveVector moves;
    const Move last_move(Square(5,4),ROOK,WHITE);
    QuiescenceGenerator<BLACK>::escapeFromLastMove<PieceEval>(state, last_move, moves);
    const Move m56fu(Square(5,6),PAWN,BLACK);
    BOOST_CHECK(moves.isMember(m56fu)); 
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  * -KI * -KE-KY\n"
			   "P2 *  * -HI * -KI * -GI+KA * \n"
			   "P3-FU *  * -FU * -FU *  *  * \n"
			   "P4 *  *  *  * -FU * -FU-OU-FU\n"
			   "P5 * -FU-GI+FU *  *  * -FU * \n"
			   "P6 *  *  *  *  * +FU+FU * +FU\n"
			   "P7+FU+FU+GI * +FU+KI+KE *  * \n"
			   "P8 *  *  * +HI *  * +GI *  * \n"
			   "P9+KY+KE *  *  * +KI+OU * +KY\n"
			   "P-00KA00FU00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    const Move last_move(Square(2,2),BISHOP,BLACK);
    QuiescenceGenerator<WHITE>::escapeFromLastMove<PieceEval>(state, last_move, moves);
    const Move m33ka(Square(3,3),BISHOP,WHITE);
    BOOST_CHECK(moves.isMember(m33ka)); 
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestEscapeKingInTakeBack)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  * -FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 * -GI *  *  *  *  *  *  * \n"
			   "P8-TO-FU *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    MoveVector moves;
    const bool has_safe_move 
      = QuiescenceGenerator<BLACK>::escapeKingInTakeBack(state, moves, false);
    BOOST_CHECK_EQUAL((size_t)0, moves.size());     
    BOOST_CHECK_EQUAL(false, has_safe_move);     
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestAdvanceBishop)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY *  * -KI *  *  *  *  * \n"
			   "P2 * -OU-GI *  * -KA-HI * -KY\n"
			   "P3 * -FU * -KI-KE-GI * -FU * \n"
			   "P4-FU * -FU-KE-FU * -FU * -FU\n"
			   "P5 *  *  * -FU * +GI * +FU * \n"
			   "P6+FU+FU+FU * +FU+FU+FU *  * \n"
			   "P7 * +GI+KI+FU+KA *  *  * +FU\n"
			   "P8 *  * +KI *  * +KE * +HI * \n"
			   "P9+KY+OU *  *  *  *  *  * +KY\n"
			   "P+00KE00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::advanceBishop(state, moves);
    BOOST_CHECK_EQUAL((size_t)2, moves.size());     // 15角が痛い
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestOpenCheck)
{
  {
    NumEffectState state(CsaString(
		    "P1-KY-KE-GI-KI * -KI-GI-KE-KY\n"
		    "P2 *  *  *  * -FU *  * -KA * \n"
		    "P3-FU-FU-FU-FU-OU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5 *  *  * +KY *  * -HI *  * \n"
		    "P6 *  *  * +GI+KI+KE *  *  * \n"
		    "P7+FU+FU+FU+FU+HI+FU+FU+FU+FU\n"
		    "P8 * +KA *  * +FU *  *  *  * \n"
		    "P9+KY+KE+GI+KI+OU *  *  *  * \n"
		    "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::check(state, PieceMask(), moves);
    // 一見ただでも玉が逃げている間に飛車がとれる
    BOOST_CHECK_EQUAL((size_t)1, moves.size()); // +5645KI
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestCheckPin)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY * -KI * -KE *  *  * +RY\n"
			   "P2 * -OU * -GI+GI *  *  *  * \n"
			   "P3 * -GI * -KI *  *  *  *  * \n"
			   "P4-FU+KE-FU-FU-FU * +KA * -FU\n"
			   "P5 * -FU *  *  * +FU *  *  * \n"
			   "P6+FU * +FU+FU+FU *  *  * +FU\n"
			   "P7 * +FU *  *  * +GI *  *  * \n"
			   "P8 * +OU+KI-RY *  * +FU *  * \n"
			   "P9+KY *  *  *  *  *  *  * +KY\n"
			   "P+00KA00KE00KY00FU00FU00FU\n"
			   "P-00KI00KE00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    const PieceMask pins = effect_util::Pin::make(state,Square(8,8),BLACK);
    QuiescenceGenerator<WHITE>::check(state, pins, moves);
    // △7七金で負け
    const Move m77ki(Square(7,7),GOLD,WHITE);
    BOOST_CHECK(moves.isMember(m77ki)); 
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestCheck)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY * -KI * -KE *  *  * +RY\n"
			   "P2 * -OU * -GI+GI *  *  *  * \n"
			   "P3 * -GI * -KI *  *  *  *  * \n"
			   "P4-FU+KE-FU-FU-FU * +KA * -FU\n"
			   "P5 * -FU *  *  * +FU *  *  * \n"
			   "P6+FU * +FU+FU+FU *  *  * +FU\n"
			   "P7 * +OU+KA *  * +GI *  *  * \n"
			   "P8 *  *  * -RY *  * +FU *  * \n"
			   "P9+KY *  *  *  *  *  *  * +KY\n"
			   "P+00KI00KE00KY00FU00FU00FU\n"
			   "P-00KI00KE00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::check(state, PieceMask(), moves);
    const Move m86ki(Square(8,6), GOLD, WHITE);
    BOOST_CHECK(moves.isMember(m86ki));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI *  *  *  * +RY\n"
			   "P2 * -OU *  * -GI *  * +NG-KY\n"
			   "P3 * -FU *  *  *  *  * -FU * \n"
			   "P4-FU *  *  * -FU * -FU * -FU\n"
			   "P5 * +FU *  *  *  *  * +FU * \n"
			   "P6+FU * -FU+KE+FU * +FU *  * \n"
			   "P7 *  *  * +KI *  *  * +KE+FU\n"
			   "P8 *  *  * +KI * -UM *  *  * \n"
			   "P9+KY+OU+KI *  *  *  *  * -RY\n"
			   "P+00GI00KY\n"
			   "P-00KA00GI00FU00FU00FU00FU00FU00KE\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::check(state, PieceMask(), moves);
    // 74桂は指してね
    BOOST_CHECK_EQUAL((size_t)1, moves.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  *  * -KY\n"
			   "P2 * -HI * -GI * -OU *  *  * \n"
			   "P3-FU * -FU-FU * -KI *  * -FU\n"
			   "P4 * -FU *  * -FU * -FU-FU * \n"
			   "P5 *  *  *  *  * +HI *  *  * \n"
			   "P6+FU * +FU *  *  * +FU+FU * \n"
			   "P7 * +FU * +FU+FU *  *  * +FU\n"
			   "P8 * +KA+KI+GI *  *  *  *  * \n"
			   "P9+KY+KE * +OU * +KI *  * -UM\n"
			   "P+00KI00KE\n"
			   "P-00GI00GI00KE00KY00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::check(state, PieceMask(), moves);
    // 43金に対するsendoffpositionがあるので，
    // 33金のような多少乱暴な王手も読む
    BOOST_CHECK_EQUAL((size_t)3, moves.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-OU *  *  *  * -HI * -KE-KY\n"
			   "P2-KY-KI-KI *  *  *  *  *  * \n"
			   "P3 * -GI *  * -FU *  *  * -FU\n"
			   "P4 * -FU-FU *  * -KA+FU-FU * \n"
			   "P5-FU+FU *  *  * -FU *  * +FU\n"
			   "P6 *  * +FU *  *  *  *  *  * \n"
			   "P7+FU *  * +KI+FU+FU+GI *  * \n"
			   "P8+KY+GI *  *  *  *  * +HI * \n"
			   "P9+OU+KE+KI *  *  *  *  * +KY\n"
			   "P+00KA00GI00KE00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::check(state, PieceMask(), moves); // 87桂
    BOOST_CHECK_EQUAL((size_t)1, moves.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  *  *  * \n"
			   "P2 * -OU-KI+RY *  *  *  *  * \n"
			   "P3-FU-FU-FU-GI *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  * +FU+FU+FU\n"
			   "P8 *  *  *  *  *  * +KI+GI+KY\n"
			   "P9 *  *  *  *  *  * +KI+KE+OU\n"
			   "P+00GI\n"
			   "P-00AL\n"
			   "-\n").initialState());
    MoveVector moves;
    const PieceMask pins = effect_util::Pin::make(state, Square(8,2), WHITE);
    QuiescenceGenerator<BLACK>::check(state, pins, moves, false); // 71GI
    BOOST_CHECK_EQUAL((size_t)1, moves.size());
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestEscape)
{
  {
    NumEffectState state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU * -FU-FU-FU-FU\n"
		    "P4 *  *  *  * -FU *  *  *  * \n"
		    "P5 *  *  *  * +KA *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
		    "P8 *  *  *  *  *  *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
		    "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::escapeAll(state, moves);
    BOOST_CHECK_EQUAL((size_t)2, moves.size()); // 46, 66
  }
  {
    NumEffectState state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE * \n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  * -KY *  *  *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  * +FU * +FU *  *  * \n"
		    "P7+FU+FU+FU * +KA * +FU+FU+FU\n"
		    "P8 *  *  *  * +KY *  * +HI * \n"
		    "P9+KY+KE+GI+KI+OU+KI+GI+KE * \n"
		    "P+00FU\n"
		    "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::escapeAll(state, moves);
    // 無駄な合駒をしなければ 48, 68
    BOOST_CHECK_EQUAL((size_t)2, moves.size());
  }
  {
    NumEffectState state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE * \n"
		    "P2 * -HI *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  * -KY *  *  *  * \n"
		    "P5 *  *  *  *  *  *  *  *  * \n"
		    "P6 *  *  * +FU * +FU *  *  * \n"
		    "P7+FU+FU+FU+GI+KA * +FU+FU+FU\n"
		    "P8 *  *  *  * +KY *  * +HI * \n"
		    "P9+KY+KE+KI * +OU+KI+GI+KE * \n"
		    "P+00FU\n"
		    "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::escapeAll(state, moves);
    // 無駄な合駒をしなければ 48, 68, +56FU
    BOOST_CHECK_EQUAL((size_t)3, moves.size());
  }
  {
    NumEffectState state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 *  *  *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5-HI *  *  *  *  *  *  *  * \n"
		    "P6 *  *  *  *  *  *  *  *  * \n"
		    "P7 * +FU+FU+FU+FU+FU+FU+FU+FU\n"
		    "P8 * +KY+KE *  *  *  * +HI * \n"
		    "P9+KA * +KI+GI+OU+KI+GI+KE+KY\n"
		    "P-00FU\n"
		    "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::escapeAll(state, moves);
    // 逃げられないが利きはつけられる
    BOOST_CHECK_EQUAL((size_t)1, moves.size()); // +7989KI
  }
  {
    NumEffectState state(CsaString(
           "P1-KY-KE * -HI * -OU *  * -KY\n"
           "P2 *  *  *  * -KI-GI-KI *  * \n"
           "P3-FU * -FU *  * -FU-KE-FU-FU\n"
           "P4 * -FU *  * -HI *  *  *  * \n"
           "P5 *  *  *  * +KA *  *  *  * \n"
           "P6 *  * +FU+FU *  *  *  *  * \n"
           "P7+FU+FU+GI+KI * +FU * +FU+FU\n"
           "P8 *  * +KI *  * +GI *  *  * \n"
           "P9+KY+KE *  * +OU *  * +KE+KY\n"
           "P+00KA00GI00FU\n"
           "P-00FU00FU00FU00FU\n"
	   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::escapeAll(state, moves);
    const Move m56fu(Square(5,6),PAWN,BLACK);
    BOOST_CHECK(moves.isMember(m56fu)); 
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestEscapeKing)
{
  {
    NumEffectState state(CsaString(
		    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
		    "P2 *  *  *  *  *  *  * -KA * \n"
		    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
		    "P4 *  *  *  *  *  *  *  *  * \n"
		    "P5 * -HI *  *  *  *  * +OU * \n"
		    "P6 *  *  *  *  * +FU *  *  * \n"
		    "P7+FU+FU+FU+FU+FU * +FU+FU+FU\n"
		    "P8 * +KA *  *  *  *  * +HI * \n"
		    "P9 * +KE+GI+KI * +KI+GI+KE+KY\n"
		    "P+00KY\n"
		    "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::escapeKing(state, moves);
      // 中合禁止: king 36,26,16, lance 35, 45
    BOOST_CHECK_EQUAL((size_t)5, moves.size()); 
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestAttackKing8)
{
  {
    NumEffectState state(CsaString(
			   "P1+UM *  *  *  *  * -OU * -KY\n"
			   "P2 *  * -HI-KI * -GI-KI *  * \n"
			   "P3 *  *  * -FU-GI-FU *  * -KE\n"
			   "P4-FU *  *  * -FU * -FU+HI-FU\n"
			   "P5 * +KE * +FU * +FU *  *  * \n"
			   "P6+FU *  * +KI+FU * +GI * +FU\n"
			   "P7+OU+FU-TO *  *  * +KE *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+KY *  *  *  *  *  *  * +KY\n"
			   "P+00KA00GI00KE00KY00FU00FU\n"
			   "P-00KI00FU00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::attackKing8(state, PieceMask(), moves);
    // 86歩と88金，95歩くらい?
    // 場合によっては76TO とか 76KI とかもしょうがない?
    BOOST_CHECK_EQUAL((size_t)3, moves.size()); 
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestAttackMajorPiece)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  * -OU * -KY\n"
			   "P2-HI *  *  *  *  * -KI *  * \n"
			   "P3-FU *  * -FU * -KI * -FU-FU\n"
			   "P4 * -FU *  *  *  * +FU *  * \n"
			   "P5 *  * +GI+FU+FU-KE+KA+FU-KA\n"
			   "P6 *  *  * +KI *  *  *  *  * \n"
			   "P7+FU+FU * +KI *  *  *  * +FU\n"
			   "P8 *  *  *  *  * +GI+HI *  * \n"
			   "P9+KY+KE * +OU *  *  * +KE+KY\n"
			   "P+00GI00FU00FU00FU00FU00FU\n"
			   "P-00GI00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::attackMajorPiece(state, PieceMask(), moves);
    BOOST_CHECK_EQUAL((size_t)6, moves.size());
// Drop(-,PAWN,Square(36)) これを作るかどうかは微妙
// Drop(-,PAWN,Square(37))
// Drop(-,SILVER,Square(27))
// Drop(-,SILVER,Square(49))
// Drop(-,SILVER,Square(44))
// Drop(-,SILVER,Square(26))
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE+KA *  *  *  *  *  * \n"
			   "P2-OU * -GI *  * +TO *  *  * \n"
			   "P3 * -FU-FU-KI *  * -KE * -FU\n"
			   "P4-FU *  *  *  *  *  *  *  * \n"
			   "P5 *  *  * -KI * -FU *  *  * \n"
			   "P6+FU * +FU-FU-FU *  *  * +FU\n"
			   "P7 * +FU+KI *  * +FU+GI-FU * \n"
			   "P8 *  * +OU *  *  *  *  * +HI\n"
			   "P9+KY+KE * +KI+RY *  *  * +KY\n"
			   "P+00KY00FU00FU\n"
			   "P-00KA00GI00GI00KE00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::attackMajorPiece(state, PieceMask(), moves);
    const Move m62gi(Square(6,2), SILVER, WHITE);
    BOOST_CHECK(moves.isMember(m62gi));
  }
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE *  *  *  *  *  * -KY\n"
			   "P2-KY-GI-GI * -KI *  *  * -HI\n"
			   "P3-FU * -FU * -FU *  * -FU * \n"
			   "P4 * +FU *  *  * +UM *  * -FU\n"
			   "P5 *  *  * -FU *  *  *  *  * \n"
			   "P6+FU * +FU *  *  *  * +FU * \n"
			   "P7 * +GI+GI+FU * -TO+FU * +FU\n"
			   "P8 * +OU+KI+KI *  *  *  *  * \n"
			   "P9+KY+KE+HI *  *  *  * +KE-UM\n"
			   "P+00KI00FU00FU00FU\n"
			   "P-00KE00KY00FU\n"
			   "+\n").initialState());
    // 上に逃げられる
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackMajorPiece(state, PieceMask(), moves);
    BOOST_CHECK(moves.empty());
  }
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE *  *  *  * +UM * -KY\n"
			   "P2-KY-GI-GI * -KI *  *  * -HI\n"
			   "P3-FU * -FU * -FU *  * -FU-FU\n"
			   "P4 * +FU *  *  *  *  *  *  * \n"
			   "P5 *  *  * -FU *  *  *  *  * \n"
			   "P6+FU * +FU *  *  *  * +FU * \n"
			   "P7 * +GI+GI+FU * -TO+FU * +FU\n"
			   "P8 * +OU+KI+KI *  *  *  *  * \n"
			   "P9+KY+KE+HI *  *  *  * +KE-UM\n"
			   "P+00KI00FU00FU00FU\n"
			   "P-00KE00KY00FU\n"
			   "+\n").initialState());
    // 仕留められる
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackMajorPiece(state, PieceMask(), moves);
    const Move m22ki(Square(2,2), GOLD, BLACK);
    BOOST_CHECK(moves.isMember(m22ki));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  * -KE-KY\n"
			   "P2 *  *  *  *  *  * -KI-OU * \n"
			   "P3 *  *  *  * -GI-KI-KA *  * \n"
			   "P4-FU-HI-FU-FU-FU * -GI *  * \n"
			   "P5 * -FU *  *  * -FU-FU-FU-FU\n"
			   "P6+FU * +HI+GI+FU *  *  *  * \n"
			   "P7 * +FU+KE * +KA+FU+FU+FU+FU\n"
			   "P8 *  *  *  *  *  * +KI+GI+KY\n"
			   "P9+KY *  *  *  *  * +KI+KE+OU\n"
			   "P-00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackMajorPiece(state, PieceMask(), moves);
    const Move m65gi(Square(6,6), Square(6,5), SILVER, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(m65gi));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  * -OU-KI-GI-KE-KY\n"
			   "P2 * -HI * -GI-KI *  * -KA * \n"
			   "P3-FU * -FU-FU * -FU * -FU-FU\n"
			   "P4 *  *  *  * -FU * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 * +KA+FU+FU *  *  *  *  * \n"
			   "P7+FU *  * +GI+FU+FU+FU+FU+FU\n"
			   "P8 * +HI *  *  * +OU *  *  * \n"
			   "P9+KY+KE * +KI * +KI+GI+KE+KY\n"
			   "P+00FU\n"
			   "P-00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackMajorPiece(state, PieceMask(), moves);
    const Move m42um(Square(8,6), Square(4,2), PBISHOP, PTYPE_EMPTY, true, BLACK);
    BOOST_CHECK(moves.isMember(m42um));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * +UM *  *  *  * -KE-KY\n"
			   "P2 *  *  *  *  *  *  * -OU * \n"
			   "P3 *  *  *  *  * -HI * -FU-FU\n"
			   "P4-FU * -FU * -FU-KI-FU-GI * \n"
			   "P5 *  *  *  *  *  *  *  * +FU\n"
			   "P6+FU+FU+FU+KI+FU * +FU *  * \n"
			   "P7 * +KI * +FU *  * -UM *  * \n"
			   "P8 * +OU * +GI *  * -NG *  * \n"
			   "P9+KY+KE *  *  *  *  *  * -RY\n"
			   "P+00KI00GI00KY00FU\n"
			   "P-00KE00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackMajorPiece(state, PieceMask(), moves);
    const Move m52gi(Square(5,2), SILVER, BLACK);
    const Move m82um(Square(7,1), Square(8,2), PBISHOP, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(! moves.isMember(m82um));
    BOOST_CHECK(moves.isMember(m52gi));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 *  *  *  *  *  *  * -KA * \n"
			   "P3-FU-HI-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 * +FU *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU * +FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +HI *  *  *  *  *  *  * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "P+00FU\n"
			   "P-00KA\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackMajorPiece(state, PieceMask(), moves);
    const Move m84fu(Square(8,5), Square(8,4), PAWN, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(m84fu));
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestAttackGoldWithPawn)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  * -OU *  * -KY\n"
			   "P2 * -HI * -GI * -KA-KI *  * \n"
			   "P3-FU * -FU-FU * -KI *  * -FU\n"
			   "P4 * -FU *  * -FU * -FU-FU * \n"
			   "P5 *  *  *  *  * -KE *  *  * \n"
			   "P6 *  * +FU *  *  * +FU+FU * \n"
			   "P7+FU+FU * +FU+FU *  *  * +FU\n"
			   "P8 * +KA+KI+GI * +HI *  *  * \n"
			   "P9+KY+KE * +OU * +KI *  * +KY\n"
			   "P+00GI00FU\n"
			   "P-00GI00KE00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackGoldWithPawn(state, PieceMask(), moves);
    BOOST_CHECK(moves.size() >= 1);
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -OU *  *  *  * \n"
			   "P2 *  *  *  * -KI *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  * +OU *  *  *  * \n"
			   "P+00KY00KE00GI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackGoldWithPawn(state, PieceMask(), moves);
    const Move m53ky(Square(5,3), LANCE, BLACK);
    const Move m53gi(Square(5,3), SILVER, BLACK);
    const Move m44ke(Square(4,4), KNIGHT, BLACK);
    BOOST_CHECK(moves.isMember(m53ky));
    BOOST_CHECK(moves.isMember(m53gi));
    BOOST_CHECK(moves.isMember(m44ke));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -OU *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  * -KE\n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  * -KA \n"
			   "P8 *  *  *  *  *  * +KI *  * \n"
			   "P9 *  *  *  * +OU * +KI *  * \n"
			   "P-00AL\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::attackGoldWithPawn(state, PieceMask(), moves);
    const Move m47ke(Square(4,7), KNIGHT, WHITE);
    const Move m27ke(Square(1,5), Square(2,7), KNIGHT, PTYPE_EMPTY, false, WHITE);
    BOOST_CHECK(moves.isMember(m47ke));
    BOOST_CHECK(moves.isMember(m27ke));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  * -KI *  * +TO *  * \n"
			   "P2 * -OU-GI * -KI *  *  *  * \n"
			   "P3 * -FU-KE * -FU *  *  * -FU\n"
			   "P4-FU * -FU *  *  * -FU *  * \n"
			   "P5 *  *  * -FU *  *  *  *  * \n"
			   "P6+FU+FU+FU * -KY * +KA *  * \n"
			   "P7 * +OU+GI *  *  * +FU * +FU\n"
			   "P8 *  * +GI+KI *  *  * -RY * \n"
			   "P9+KY+KE * +KI *  *  *  *  * \n"
			   "P+00HI00GI00KY00FU\n"
			   "P-00KA00KE00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackGoldWithPawn(state, PieceMask(), moves);
    const Move m64ky(Square(6,4), LANCE, BLACK);
    BOOST_CHECK(moves.isMember(m64ky));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI *  *  *  * +RY\n"
			   "P2 * -OU-GI * -KI * +TO *  * \n"
			   "P3 * -FU-FU-FU * -FU * -FU-FU\n"
			   "P4-FU *  * -KA-FU *  *  *  * \n"
			   "P5 *  *  *  *  *  *  * +GI * \n"
			   "P6+FU+FU * +FU+FU+GI *  *  * \n"
			   "P7+KY *  * +GI *  * +KI+FU+FU\n"
			   "P8 *  *  * +KA *  *  * +HI * \n"
			   "P9 * -NK *  *  * +OU *  * +KY\n"
			   "P+00KE00KY\n"
			   "P-00KI00KE00FU00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<WHITE>::attackGoldWithPawn(state, PieceMask(), moves);
    const Move m45ke(Square(4,5), KNIGHT, WHITE);
    
    BOOST_CHECK(moves.isMember(m45ke));
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestAttackSilverWithPawn)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -OU *  *  *  * \n"
			   "P2 *  *  *  * -GI *  * -GI * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  * +FU *  * +FU * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  * +OU *  *  *  * \n"
			   "P+00KY00KE\n"
			   "P-00AL\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackSilverWithPawn(state, PieceMask(), moves);
    const Move m53ky(Square(5,3), LANCE, BLACK);
    const Move m44ke(Square(4,4), KNIGHT, BLACK);
    BOOST_CHECK(moves.isMember(m53ky));
    BOOST_CHECK(moves.isMember(m44ke));

    const Move m23ky(Square(2,3), LANCE, BLACK);
    const Move m34ke(Square(3,4), KNIGHT, BLACK);
    BOOST_CHECK(! moves.isMember(m23ky));
    BOOST_CHECK(! moves.isMember(m34ke));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -HI-KI-KA * -KY\n"
			   "P2 *  * -GI *  *  *  * -OU * \n"
			   "P3+TO * -KE *  * -KI-KE *  * \n"
			   "P4 *  * -FU-FU-FU-FU-FU-GI-FU\n"
			   "P5+KA *  *  *  *  *  * -FU * \n"
			   "P6 *  * +FU+FU * +GI+FU * +FU\n"
			   "P7 * -TO *  *  * +FU+KE+FU * \n"
			   "P8 * -FU * +HI *  * +KI+KI+KY\n"
			   "P9 * +KE *  *  *  *  * +GI+OU\n"
			   "P+00KY00FU00FU\n"
			   "P-00KY\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::attackSilverWithPawn(state, PieceMask(), moves);
    const Move m82to(Square(9,3), Square(8,2), PPAWN, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(m82to));
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestAddLance)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -GI-OU-KY\n"
			   "P2 *  *  *  *  * -KA-KI-GI * \n"
			   "P3 *  *  *  *  *  * -KE-FU-FU\n"
			   "P4+RY * +KI * -FU-FU+FU *  * \n"
			   "P5 *  *  * -FU *  *  *  * +FU\n"
			   "P6 * +FU *  * +FU+FU *  *  * \n"
			   "P7+KE * -UM * +GI+GI *  *  * \n"
			   "P8 *  *  *  *  * +KI+OU *  * \n"
			   "P9+KY *  *  *  *  *  * +HI+KY\n"
			   "P+00KE00KE00KY00FU00FU00FU\n"
			   "P-00KI00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    QuiescenceGenerator<BLACK>::capture<KNIGHT,false>(state, moves,Piece::EMPTY());
    // 33to
    // 35,36,37kyo
    BOOST_CHECK_EQUAL((size_t)4, moves.size());
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestUtilizePromoted)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  * -HI *  * -GI-KY\n"
			   "P3-FU * +NK-FU * -FU-KA-FU-FU\n"
			   "P4 * -FU * -KI-FU * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6+FU * +HI+FU *  *  *  *  * \n"
			   "P7 * +FU * +GI+FU+FU+FU+FU+FU\n"
			   "P8 *  *  *  * +KI * +OU *  * \n"
			   "P9+KY *  *  *  * +KI+GI+KE+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00KA\n"
			   "+\n").initialState());
    MoveVector moves;
    const Piece target = state.pieceOnBoard(Square(7,3));
    QuiescenceGenerator<BLACK>::utilizePromoted(state, target, moves);
    BOOST_CHECK_EQUAL((size_t)5, moves.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  * -HI *  * -GI-KY\n"
			   "P3-FU * +NK-FU * -FU-KA-FU-FU\n"
			   "P4 * -FU * -KI-FU * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6+FU+HI * +FU *  *  *  *  * \n"
			   "P7 * +FU * +GI+FU+FU+FU+FU+FU\n"
			   "P8 *  *  *  * +KI * +OU *  * \n"
			   "P9+KY *  *  *  * +KI+GI+KE+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00KA\n"
			   "+\n").initialState());
    MoveVector moves;
    const Piece target = state.pieceOnBoard(Square(7,3));
    QuiescenceGenerator<BLACK>::utilizePromoted(state, target, moves);
    BOOST_CHECK_EQUAL((size_t)1, moves.size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  * -HI *  * -GI-KY\n"
			   "P3-FU-KE+NK-FU * -FU-KA-FU-FU\n"
			   "P4 * -FU * -KI-FU * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6+FU * +HI+FU *  *  *  *  * \n"
			   "P7 * +FU * +GI+FU+FU+FU+FU+FU\n"
			   "P8 *  *  *  * +KI * +OU *  * \n"
			   "P9+KY *  *  *  * +KI+GI+KE+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00KA\n"
			   "+\n").initialState());
    MoveVector moves;
    const Piece target = state.pieceOnBoard(Square(7,3));
    QuiescenceGenerator<BLACK>::utilizePromoted(state, target, moves);
    BOOST_CHECK_EQUAL((size_t)2, moves.size());
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestEscapeByMoveOnly)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  * -OU-KI * -KE-KA\n"
			   "P2 *  *  * -GI-KI * -GI *  * \n"
			   "P3-FU * -FU-FU * -FU * -FU-FU\n"
			   "P4 *  *  *  * -FU * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 * -HI+FU+FU *  *  *  *  * \n"
			   "P7+FU+KY * +KI+FU+FU+FU+FU+FU\n"
			   "P8 * +GI+KI *  *  *  * +HI * \n"
			   "P9+KY+KE * +OU *  * +GI+KE+KY\n"
			   "P+00FU\n"
			   "P-00KA00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    const Piece target = state.pieceOnBoard(Square(8,6));
    const bool has_safe_move
      = QuiescenceGenerator<WHITE>::escapeByMoveOnly(state, target, moves);
    // 後ろに下がっても取られる
    BOOST_CHECK(! has_safe_move);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  * -OU-KI * -KE-KA\n"
			   "P2 *  *  * -GI-KI * -GI *  * \n"
			   "P3-FU * -FU-FU * -FU * -FU-FU\n"
			   "P4 *  *  *  * -FU * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 * -HI+FU+FU *  *  *  *  * \n"
			   "P7+FU+FU * +KI+FU+FU+FU+FU+FU\n"
			   "P8 * +GI+KI *  *  *  * +HI * \n"
			   "P9+KY+KE * +OU *  * +GI+KE+KY\n"
			   "P+00KY\n"
			   "P-00KA00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    const Piece target = state.pieceOnBoard(Square(8,6));
    const bool has_safe_move
      = QuiescenceGenerator<WHITE>::escapeByMoveOnly(state, target, moves);
    // 後ろに下がれる
    BOOST_CHECK(has_safe_move);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI *  * -KI-KE-OU\n"
			   "P2 *  *  * -GI *  *  * -GI-KY\n"
			   "P3-FU-HI-FU-FU-FU-FU-UM-FU-FU\n"
			   "P4 * +FU *  *  *  * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU * +FU *  *  *  * \n"
			   "P7+FU * +KE+FU * +FU+FU+FU+FU\n"
			   "P8 * +HI * +GI+KI * +GI+OU * \n"
			   "P9+KY *  *  *  * +KI * +KE+KY\n"
			   "P+00FU\n"
			   "P-00KA\n"
			   "-\n").initialState());
    MoveVector moves;
    const Piece target = state.pieceOnBoard(Square(8,3));
    const bool has_safe_move
      = QuiescenceGenerator<WHITE>::escapeByMoveOnly(state, target, moves);
    // 後ろに下がっても追撃される
    BOOST_CHECK(! has_safe_move);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI *  * -KI-KE-OU\n"
			   "P2 *  *  * -GI *  *  * -GI-KY\n"
			   "P3-FU-HI *  * -FU-FU-UM-FU-FU\n"
			   "P4 * +FU-FU-FU *  * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU * +FU *  *  *  * \n"
			   "P7+FU * +KE+FU * +FU+FU+FU+FU\n"
			   "P8 * +HI * +GI+KI * +GI+OU * \n"
			   "P9+KY *  *  *  * +KI * +KE+KY\n"
			   "P+00FU\n"
			   "P-00KA\n"
			   "-\n").initialState());
    MoveVector moves;
    const Piece target = state.pieceOnBoard(Square(8,3));
    const bool has_safe_move
      = QuiescenceGenerator<WHITE>::escapeByMoveOnly(state, target, moves);
    // 横に二歩逃げられる
    BOOST_CHECK(has_safe_move);
  }
}

BOOST_AUTO_TEST_CASE(QuiescenceGeneratorTestBreakThreatmate)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-HI *  *  *  * -GI-OU-KY\n"
			   "P2 *  *  *  * +FU *  *  *  * \n"
			   "P3 *  *  *  * -KI *  * +KI * \n"
			   "P4 * -FU-FU * -GI * +KE+UM * \n"
			   "P5-FU *  * +FU-FU-KE * -FU-FU\n"
			   "P6 *  * +FU *  *  *  *  *  * \n"
			   "P7+FU+FU+KI * -TO *  *  *  * \n"
			   "P8 * +OU+KI *  *  * +FU * +HI\n"
			   "P9+KY+KE *  * -UM *  *  *  * \n"
			   "P+00KE\n"
			   "P-00GI00GI00KY00FU00FU00FU00FU00FU\n"
			   "-\n").initialState());
    MoveVector moves;
    const Move m33ke(Square(3,3), KNIGHT, BLACK);
    QuiescenceGenerator<WHITE>::breakThreatmate(state, m33ke, PieceMask(),
						moves);
    BOOST_CHECK(! moves.empty());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  *  * -KY\n"
			   "P2 *  *  * -FU+FU *  *  *  * \n"
			   "P3-FU+RY *  *  *  * +TO+HI-FU\n"
			   "P4 *  *  * +FU * -GI+KA *  * \n"
			   "P5 *  * +FU * -GI-FU-FU *  * \n"
			   "P6 * +KI *  * -OU * -KE *  * \n"
			   "P7+FU+FU-KI *  * -TO *  * +FU\n"
			   "P8+OU * -KI *  *  *  *  *  * \n"
			   "P9+KY *  *  *  *  * +GI * +KY\n"
			   "P+00KA00KI00GI00KE00FU00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n").initialState());
    MoveVector moves;
    const Move m88ki(Square(7,8), Square(8,8), GOLD, PTYPE_EMPTY, false, WHITE);
    QuiescenceGenerator<BLACK>::breakThreatmate(state, m88ki, PieceMask(),
						moves);
    const Move m96fu(Square(9,7), Square(9,6), PAWN, PTYPE_EMPTY, false, BLACK);
    
    BOOST_CHECK(moves.isMember(m96fu));
  }
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
