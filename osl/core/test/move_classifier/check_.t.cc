#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::move_action;
using namespace osl::move_classifier;

BOOST_AUTO_TEST_CASE(CheckTestIsCheck){
  const char *stateString = 
    "P1-KY *  *  *  *  *  * +NY * \n"
    "P2 * -OU-KI-KI *  *  *  * +RY\n"
    "P3 * -GI-KE+KI *  *  *  * +HI\n"
    "P4 *  * -FU-KY-FU *  * -FU * \n"
    "P5-FU-FU * -KE * -FU *  *  * \n"
    "P6 *  * +FU-FU+FU * -FU *  * \n"
    "P7+FU+FU *  *  *  *  *  *  * \n"
    "P8+KY+GI+GI-UM *  *  *  *  * \n"
    "P9+OU+KE *  *  *  *  * +KE * \n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00FU\n"
    "P-00GI\n"
    "P+00KI\n"
    "P-00KA\n"
    "+\n";
  NumEffectState state((CsaString(stateString).initialState()));
  const Move m = Move(Square(7,1),GOLD,BLACK);
  using namespace move_classifier;
  BOOST_CHECK_EQUAL(false, PlayerMoveAdaptor<Check>::isMember(state, m));
}

static bool isCheckDoUndo(const NumEffectState& state_org,Move move)
{
  NumEffectState state = state_org;
  state.makeMove(move);
  return state.hasEffectAt(move.player(),state.kingSquare(alt(move.player())));
}

static bool isCheck(Player turn, const NumEffectState& state,Move move)
{
  if(turn==BLACK)
    return Check<BLACK>::
      isMember(state,move.ptype(),move.from(),move.to());
  else
    return Check<WHITE>::
      isMember(state,move.ptype(),move.from(),move.to());
}

static void testMoveFile(const std::string& filename){
  auto record=CsaFileMinimal(filename).load();
  NumEffectState state=record.initialState();
  auto moves=record.moves;
  for(unsigned int i=0;i<moves.size();i++){
    MoveVector all_moves;
    state.generateAllUnsafe(all_moves);
    for(size_t j=0;j<all_moves.size();j++){
      Move move=all_moves[j];
      BOOST_CHECK(isCheckDoUndo(state,move) == isCheck(state.turn(), state,move));
    }
    state.makeMove(moves[i]);
  }
}
BOOST_AUTO_TEST_CASE(CheckTestFile){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=50;
  if (OslConfig::inUnitTestShort()) 
    count=5;
  std::string filename;
  while((ifs >> filename) && ++i<count){
    if(filename == "") 
      break;
    testMoveFile(OslConfig::testCsaFile(filename));
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
