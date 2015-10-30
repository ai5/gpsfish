#include "osl/rating/feature/checkmate.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>
using namespace osl;
using namespace osl::rating;

static void testSquare(const NumEffectState& state)
{
  if (state.inCheck())
    return;
  if (ImmediateCheckmate::hasCheckmateMove(state.turn(), state))
    return;
  MoveVector moves;
  state.generateLegal(moves);
  for (size_t i=0; i<moves.size(); ++i) {
    NumEffectState next(state);
    next.makeMove(moves[i]);
    if (next.inCheck())
      continue;
    if (! ImmediateCheckmate::hasCheckmateMove(state.turn(), next))
      continue;
    if (! Threatmate::isCandidate(state, moves[i]))
      std::cerr << state << moves[i] << "\n";
    BOOST_CHECK(Threatmate::isCandidate(state, moves[i]));
  }
}

BOOST_AUTO_TEST_CASE(ThreatmateTestFile) 
{
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  *  * -KY\n"
			   "P2 *  *  *  * -GI *  *  *  * \n"
			   "P3-FU *  * -GI * -HI-KI * -FU\n"
			   "P4 *  *  *  *  * +UM-FU-FU-OU\n"
			   "P5 * -FU-FU *  *  *  *  * +KE\n"
			   "P6 *  * +FU+FU *  * +FU * +GI\n"
			   "P7+FU+FU+GI-FU *  *  *  * -RY\n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KI00KE00FU\n"
			   "P-00KA00KI00KE00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testSquare(state);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * +KA * +UM *  *  * -KY\n"
			   "P2 *  *  *  * -GI * -KI *  * \n"
			   "P3-FU *  * -GI * -HI-KE * -FU\n"
			   "P4 *  *  *  *  * -KI-FU-FU-OU\n"
			   "P5 * -FU-FU *  *  *  *  * +KE\n"
			   "P6 * -KE+FU+FU *  * +FU * +GI\n"
			   "P7+FU+FU *  *  * +KI *  * +FU\n"
			   "P8 * +OU+KI+GI-RY *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00FU00FU\n"
			   "P-00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testSquare(state);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  *  * -KY\n"
			   "P2 *  *  *  * -GI * -KI *  * \n"
			   "P3-FU *  * -GI * -HI-KE * -FU\n"
			   "P4 *  *  *  *  *  * -KI-FU-OU\n"
			   "P5 * -FU-FU *  *  *  *  * +KE\n"
			   "P6 *  * +FU+FU *  * +FU+UM+GI\n"
			   "P7+FU+FU+GI-FU * -RY *  * +FU\n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KI00KE00FU00FU\n"
			   "P-00KA00FU00FU00FU00FU\n"
			   "+\n").initialState());
    testSquare(state); // 62um
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-HI *  * -OU-GI *  * -KY\n"
			   "P2 *  * +KI * -FU * +NK *  * \n"
			   "P3 *  *  * -FU-GI *  *  *  * \n"
			   "P4-FU * -FU *  *  * +KI+FU-FU\n"
			   "P5 * -FU *  *  * +KE * -UM * \n"
			   "P6 *  * +FU+FU * -FU *  * +FU\n"
			   "P7+FU+FU *  *  *  *  *  *  * \n"
			   "P8 * +OU * -NK *  *  * +HI * \n"
			   "P9+KY+KE+GI *  *  *  *  * +KY\n"
			   "P+00GI00FU00FU\n"
			   "P-00KA00KI00KI00FU00FU00FU\n"
			   "+\n").initialState());
    testSquare(state); // 68gi
  }
  {
    NumEffectState state(CsaString(
			   "P1+KA *  *  *  *  * -KI-KE-OU\n"
			   "P2-KY *  *  *  * +TO * -GI-KY\n"
			   "P3 *  *  * -FU * +TO-FU-FU-FU\n"
			   "P4-FU *  *  * -FU+KE *  *  * \n"
			   "P5 *  *  * +FU *  *  *  *  * \n"
			   "P6+FU * +FU *  *  * +FU *  * \n"
			   "P7 *  *  *  * +FU * -KI+FU+FU\n"
			   "P8+KY+RY-RY *  *  *  * +GI+KY\n"
			   "P9-UM+KE *  *  * -TO * +KE+OU\n"
			   "P+00KI00GI00FU\n"
			   "P-00KI00GI00FU\n"
			   "-\n").initialState());
    testSquare(state);// 88um
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * +UM-KE * -KY\n"
			   "P2 *  *  *  * +FU *  *  *  * \n"
			   "P3-FU *  *  * +RY *  * -FU-FU\n"
			   "P4 *  *  * +FU * -KI-GI * -OU\n"
			   "P5 * +KE *  * -FU+UM-FU *  * \n"
			   "P6 * -FU *  *  * -FU+FU *  * \n"
			   "P7+FU *  *  *  * +GI+KE+OU+FU\n"
			   "P8+KY-TO *  *  * +FU+KI+FU * \n"
			   "P9 *  *  *  * -RY *  *  * +KY\n"
			   "P+00GI00FU\n"
			   "P-00KI00KI00GI00KE00FU00FU\n"
			   "+\n").initialState());
    testSquare(state);	// 44ry
  }
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for(int i=0;i<100 && (ifs >> filename) ; i++){
    const RecordMinimal record=CsaFileMinimal(OslConfig::testCsaFile(filename)).load();
    NumEffectState state(record.initial_state);
    for (Move move:record.moves) {
      testSquare(state);
      state.makeMove(move);
    }
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
