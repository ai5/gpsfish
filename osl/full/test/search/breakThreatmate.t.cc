/* breakThreatmate.t.cc
 */
#include "osl/search/breakThreatmate.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/container/moveLogProbVector.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(BreakThreatmateTestBlock)
{
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE *  *  *  *  *  * +TO\n"
			   "P2-KY * -KI+TO+KI * -FU *  * \n"
			   "P3 * -FU *  *  * -FU-KE *  * \n"
			   "P4-FU * +FU-UM * -RY *  *  * \n"
			   "P5 *  *  * -FU * -GI *  *  * \n"
			   "P6+FU * +KI * -KA-KY *  *  * \n"
			   "P7 * +FU *  *  *  *  * -FU+FU\n"
			   "P8+KY+GI-GI *  *  *  *  *  * \n"
			   "P9+OU+KE * +KY * +FU *  *  * \n"
			   "P+00HI00GI00KE00FU00FU00FU00FU00FU\n"
			   "P-00KI\n"
			   "+\n").initialState());
    MoveLogProbVector moves;
    BreakThreatmate::generate(500, state, Move(Square(7,8), Square(8,9), PSILVER, KNIGHT, true, WHITE), moves);
    BOOST_CHECK(! moves.empty());    
    BOOST_CHECK(moves.find(Move(Square(6,7), PAWN, BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-HI *  *  *  *  *  * +TO\n"
			   "P2 * -OU-FU *  * +RY *  *  * \n"
			   "P3 *  *  *  *  * -KE *  *  * \n"
			   "P4 *  * +KI * -GI * +KA-FU-FU\n"
			   "P5-FU * +GI * -FU *  *  *  * \n"
			   "P6 *  *  *  *  * -FU+FU-KY+FU\n"
			   "P7+FU *  *  *  *  *  *  *  * \n"
			   "P8+KY-FU *  *  * +KI+GI *  * \n"
			   "P9 * -NG-KA * -TO * +OU+KE+KY\n"
			   "P+00KE\n"
			   "P-00KI00KI00KE00FU00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    MoveLogProbVector moves;
    BreakThreatmate::generate(500, state, Move(Square(2,8), GOLD, WHITE), moves);
    BOOST_CHECK(! moves.empty());    
    BOOST_CHECK(moves.find(Move(Square(2,7), KNIGHT, BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE *  *  * +HI *  * -KY\n"
			   "P2-KY-GI+KI-FU *  * +FU *  * \n"
			   "P3 *  *  *  *  * -FU *  * -FU\n"
			   "P4-FU-FU * -KI-FU * -FU-FU * \n"
			   "P5 *  *  * -GI *  *  *  * +FU\n"
			   "P6+FU * -FU-KE+FU *  *  *  * \n"
			   "P7 * +FU+KI *  * +FU *  *  * \n"
			   "P8+KY+GI *  *  *  *  *  *  * \n"
			   "P9+OU+KE+KI *  * -HI *  * +KY\n"
			   "P+00FU00FU\n"
			   "P-00KA00KA00GI00KE00FU\n"
			   "-\n").initialState());
    MoveLogProbVector moves;
    const Move m81ki(Square(7,2), Square(8,1), GOLD, KNIGHT, false, BLACK);
    BreakThreatmate::generate(500, state, m81ki, moves);
    const Move m71gi(Square(7,1), SILVER, WHITE); // ·ËÇÏ¤ÏµÍ¤ß
    BOOST_CHECK(moves.isMember(MoveLogProb(m71gi, 300)));
  }
}

BOOST_AUTO_TEST_CASE(BreakThreatmateTestDropValidity)
{
  {
    NumEffectState state(CsaString(
			   "P1+OU *  *  *  *  * -KI-KE-OU\n"
			   "P2+FU-FU-GI *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  * -FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P+00KE00KY00FU\n"
			   "P-00AL\n"
			   "+\n").initialState());
    MoveLogProbVector moves;
    BreakThreatmate::generate(500, state, Move(Square(8,1), ROOK, WHITE), moves);
    BOOST_CHECK(! moves.empty());    
    BOOST_CHECK(moves.size() == 3); // 73KE, 93KE, 82OU
  }
}

BOOST_AUTO_TEST_CASE(BreakThreatmateTestGenerate)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  * -FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7-FU *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+OU *  *  *  *  *  *  *  * \n"
			   "P-00KI\n"
			   "P+00AL\n"
			   "+\n").initialState());
    MoveLogProbVector moves;
    BreakThreatmate::generate(500, state, Move(Square(9,8), GOLD, WHITE), 
				   moves);
    BOOST_CHECK(! moves.empty());    
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  * -FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU *  *  *  *  * \n"
			   "P8+OU * +KI *  * -HI *  *  * \n"
			   "P9+KY+KE-GI *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    MoveLogProbVector moves;
    const Move threatmate_move(Square(4,8), Square(7,8), PROOK, 
			       GOLD, false, WHITE);
    BreakThreatmate::generate(500, state, threatmate_move, moves);
    BOOST_CHECK(! moves.empty());    
    BOOST_CHECK(moves.isMember(MoveLogProb(Move(Square(5,8), GOLD, BLACK),
					      400))); 
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -KI-KE-OU\n"
			   "P2 *  *  *  *  *  * -KI-GI-KY\n"
			   "P3 *  *  *  *  *  * -FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 * -FU-KE-FU *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU *  *  *  *  *  * \n"
			   "P8+KY+KA *  *  *  *  *  *  * \n"
			   "P9+OU+KE *  *  *  *  *  *  * \n"
			   "P+00AL\n"
			   "+\n").initialState());
    MoveLogProbVector moves;
    const Move threatmate_move(Square(7,5), Square(8,7), KNIGHT, 
			       PAWN, false, WHITE);
    BreakThreatmate::generate(500, state, threatmate_move, moves);
    BOOST_CHECK(! moves.empty());    
    BOOST_CHECK(moves.isMember(MoveLogProb(Move(Square(8,8),Square(7,9),
						   BISHOP, PTYPE_EMPTY, false,
						   BLACK), 100))); 
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
    MoveLogProbVector moves;
    const Move m88ki(Square(7,8), Square(8,8), GOLD, PTYPE_EMPTY, false, WHITE);
    BreakThreatmate::generate(500, state, m88ki, moves);
    const Move m96fu(Square(9,7), Square(9,6), PAWN, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(moves.isMember(MoveLogProb(m96fu, 100)));
  }
}

BOOST_AUTO_TEST_CASE(BreakThreatmateTestDropCandidate)
{
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE *  *  *  *  *  * -KY\n"
			   "P2-KY * +KI-FU *  * +FU *  * \n"
			   "P3 *  *  * -KI * -FU *  * -FU\n"
			   "P4-FU-FU * -GI-FU * -FU-FU * \n"
			   "P5 *  *  * -GI *  *  *  * +FU\n"
			   "P6+FU * -FU-KE+FU+KA *  *  * \n"
			   "P7 * +FU+KI *  * +FU *  *  * \n"
			   "P8+KY+GI *  *  *  *  *  *  * \n"
			   "P9+OU+KE+KI *  * -HI *  * +KY\n"
			   "P+00HI00FU00FU\n"
			   "P-00KA00GI00KE00FU\n"
			   "-\n").initialState());
    MoveLogProbVector moves;
    const Move m64ka(Square(4,6), Square(6,4), BISHOP, SILVER, false, BLACK);
    BreakThreatmate::generate(500, state, m64ka, moves);
    const Move m71gi(Square(7,1), SILVER, WHITE);
    BOOST_CHECK(moves.isMember(MoveLogProb(m71gi, 100)));
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
