/* checkmateIfCapture.t.cc
 */
#include "osl/checkmate/checkmateIfCapture.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(CheckmateIfCaptureTestAttack)
{
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -OU *  * \n"
			   "P2 * -HI *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  * +TO+TO *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  * +OU *  *  * \n"
			   "P+00GI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m83(Square(8,3), SILVER, BLACK);
    BOOST_CHECK(CheckmateIfCapture::effectiveAttack(state, m83, 0));

    const Move m73(Square(7,3), SILVER, BLACK);
    BOOST_CHECK(! CheckmateIfCapture::effectiveAttack(state, m73, 0));

    // 除きたい?
    const Move m84(Square(8,4), SILVER, BLACK);
    BOOST_CHECK(CheckmateIfCapture::effectiveAttack(state, m84, 0));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -OU *  * \n"
			   "P2 * -HI *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  * +TO+TO *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU *  *  *  *  *  *  * \n"
			   "P+00GI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m83(Square(8,3), SILVER, BLACK);
    // 取ると王手
    BOOST_CHECK(! CheckmateIfCapture::effectiveAttack(state, m83, 0));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  * -GI-OU-GI * \n"
			   "P2 * -HI *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  * +TO+TO *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  * +OU *  *  * \n"
			   "P+00GI00GI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m83(Square(8,3), SILVER, BLACK);
    BOOST_CHECK(! CheckmateIfCapture::effectiveAttack(state, m83, 0));

    // 3手詰
    BOOST_CHECK(CheckmateIfCapture::effectiveAttack(state, m83, 2));

    const Move m73(Square(7,3), SILVER, BLACK);
    BOOST_CHECK(! CheckmateIfCapture::effectiveAttack(state, m73, 0));

    // 除きたい?
    const Move m84(Square(8,4), SILVER, BLACK);
    BOOST_CHECK(CheckmateIfCapture::effectiveAttack(state, m84, 2));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  * -KE-KY\n"
			   "P2 *  *  *  *  *  *  * -OU * \n"
			   "P3 *  *  *  *  * +TO-FU-FU * \n"
			   "P4 *  *  *  *  *  *  *  * -FU\n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 *  *  *  *  * +OU *  *  * \n"
			   "P+00KA00KI\n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m31(Square(3,1), BISHOP, BLACK);
    BOOST_CHECK(CheckmateIfCapture::effectiveAttack(state, m31, 0));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  *  *  *  *  *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  * -OU *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  *  *  * +OU *  *  * \n"
			   "P9 *  *  *  *  *  *  *  *  * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    const Move m47(Square(4,8), Square(4,7), KING, PTYPE_EMPTY, false, BLACK);
    BOOST_CHECK(! CheckmateIfCapture::effectiveAttack(state, m47, 0));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  * +HI * -KI-OU * -KE-KY\n"
			   "P2 *  *  *  *  *  *  *  *  * \n"
			   "P3 *  *  * -FU-FU+FU-FU-FU-FU\n"
			   "P4 *  * +KA *  *  *  *  *  * \n"
			   "P5 *  *  *  * +KE *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU *  *  *  *  *  * \n"
			   "P8+KY+GI+KI *  *  *  *  *  * \n"
			   "P9+OU+KE+KI *  *  *  *  *  * \n"
			   "P+00KI\n"
			   "P-00HI00KA00GI00GI00GI00KE00KY00KY00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    const Move m52(Square(5,2), GOLD, BLACK);
    // 金ではとれない．玉で取ると詰
    BOOST_CHECK(CheckmateIfCapture::effectiveAttack(state, m52, 0));
  }
}

static void testCandidateSquare(NumEffectState& state)
{
  if (! state.inCheck() 
      && ImmediateCheckmate::hasCheckmateMove(state.turn(), state))
    return;
  MoveVector moves;
  state.generateLegal(moves);
  for (size_t i=0; i<moves.size(); ++i) {
    if (! state.hasEffectAt(alt(state.turn()), moves[i].to()))
      continue;			// specification of rating::CheckmateIfCapture
    const bool effective = CheckmateIfCapture::effectiveAttack(state, moves[i], 0);
    const bool predicted = CheckmateIfCapture::effectiveAttackCandidate0(state, moves[i]);
    if (effective && ! predicted)
      std::cerr << state << moves[i] << "\n";
    BOOST_CHECK(! effective || predicted);
  }
}

BOOST_AUTO_TEST_CASE(CheckmateIfCaptureTestCandidate)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY * +KA * +UM *  *  * -KY\n"
			   "P2 *  *  *  * -GI * -KI *  * \n"
			   "P3-FU *  * -GI * -HI-KE * -FU\n"
			   "P4 *  *  *  *  * -KI-FU-FU-OU\n"
			   "P5 * -FU-FU *  *  *  *  * +KE\n"
			   "P6 *  * +FU+FU *  * +FU * +GI\n"
			   "P7+FU+FU+GI-FU * -RY *  * +FU\n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00FU\n"
			   "P-00KI00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testCandidateSquare(state);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * +KA *  *  *  *  * -KY\n"
			   "P2 *  *  *  * -GI * -KI *  * \n"
			   "P3-FU *  * -GI * -HI+UM * -FU\n"
			   "P4 *  *  *  *  *  * -FU-FU-OU\n"
			   "P5 * -FU-FU *  *  * -KI * +KE\n"
			   "P6 *  * +FU+FU *  * +FU * +GI\n"
			   "P7+FU+FU+GI-FU * -RY *  * +FU\n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KE00FU\n"
			   "P-00KI00KE00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testCandidateSquare(state);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * +KA *  *  *  *  * +UM\n"
			   "P2 *  *  *  * -GI * -KI *  * \n"
			   "P3-FU *  * -GI * -HI *  * -FU\n"
			   "P4 *  *  *  *  * -KI-FU-FU-OU\n"
			   "P5 * -FU *  *  *  *  *  * +KE\n"
			   "P6 *  * -FU+FU *  * +FU * +GI\n"
			   "P7+FU+FU+GI-FU *  * -RY * +FU\n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KE00KY00FU\n"
			   "P-00KI00KE00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testCandidateSquare(state);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * +KA *  *  *  *  * -KY\n"
			   "P2 *  *  *  * -GI *  *  *  * \n"
			   "P3-FU *  * -GI *  *  * -HI-FU\n"
			   "P4 *  *  *  *  * -KI-FU-FU-OU\n"
			   "P5 * -FU-FU *  *  *  *  * +KE\n"
			   "P6 *  * +FU+FU *  * +FU-KE+GI\n"
			   "P7+FU+FU+GI-FU * -RY *  * +FU\n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KI00KI00KE00FU\n"
			   "P-00KA00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testCandidateSquare(state);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  *  * -KY\n"
			   "P2 *  *  *  * -GI * +UM *  * \n"
			   "P3-FU *  * -GI * -HI * -KE-FU\n"
			   "P4 *  *  *  *  * +UM-FU-FU-OU\n"
			   "P5 * -FU-FU *  *  *  * -KI+KE\n"
			   "P6 *  * +FU+FU *  * -RY * +GI\n"
			   "P7+FU+FU+GI-FU *  *  *  * +FU\n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KI00KI00KE00FU\n"
			   "P-00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testCandidateSquare(state); // 43um
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  * -HI * +UM-KE-KY\n"
			   "P2 *  *  *  *  *  * +KI * -OU\n"
			   "P3 *  * -GI *  * -KI * -KI-FU\n"
			   "P4-FU * -FU * -FU * -FU-GI * \n"
			   "P5 * +FU *  *  * -FU *  *  * \n"
			   "P6 *  * +FU *  *  *  *  *  * \n"
			   "P7+FU *  *  * -UM+FU+FU+FU+FU\n"
			   "P8 *  *  * -RY * -KY+GI+OU * \n"
			   "P9 *  *  *  *  *  * +KI+KE+KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00KE00FU00FU00FU\n"
			   "+\n").initialState());
    testCandidateSquare(state); // 52gi
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * -KI-KI *  * \n"
			   "P2 * -HI *  *  *  * -OU+GI * \n"
			   "P3 *  *  *  * -GI * +KA-FU+NK\n"
			   "P4-FU * -FU * -FU-FU-GI * -FU\n"
			   "P5 * -FU * -KE *  * -FU *  * \n"
			   "P6+FU * +FU-UM+FU+FU * +FU * \n"
			   "P7 * +FU *  * -TO * +FU+GI * \n"
			   "P8 *  * -RY *  * +KY+KI+OU * \n"
			   "P9+KY *  *  *  *  *  *  *  * \n"
			   "P+00KI\n"
			   "P-00KE00KE00KY00FU00FU\n"
			   "+\n").initialState());
    testCandidateSquare(state); // 15ka
  }
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for(int i=0;i<100 && (ifs >> filename) ; i++){
    const RecordMinimal record=CsaFileMinimal(OslConfig::testCsaFile(filename)).load();
    NumEffectState state(record.initial_state);
    if (i % 32 == 0) 
      std::cerr << '.';
    for (auto move:record.moves) {
      testCandidateSquare(state);
      state.makeMove(move);
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
