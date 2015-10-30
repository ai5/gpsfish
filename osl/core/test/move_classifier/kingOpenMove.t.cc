#include "osl/move_classifier/kingOpenMove.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::move_action;
using namespace osl::move_classifier;

BOOST_AUTO_TEST_CASE(KingOpenMoveTestIsKingOpenMove){
  {
    // 王手がかかった状態での開き王手
    NumEffectState state=CsaString(
"P1+NY+RY *  *  * -FU-OU-KE-KY\n"
"P2 *  *  *  *  * +GI-KI *  *\n"
"P3 * -FU *  *  *  * -KI-FU-FU\n"
"P4 * +FU * -FU *  *  *  *  *\n"
"P5 *  * -KE * +FU *  * +FU *\n"
"P6+KE *  * +FU+GI *  *  * +FU\n"
"P7 *  * -UM-KA *  *  *  *  *\n"
"P8 *  * +FU *  *  *  *  *  * \n"
"P9 * +OU * -GI *  *  *  * -NG\n"
"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
"P-00KI00KY00FU00FU\n"
"P-00AL\n"
"-\n"
).initialState();
    // 開き王手ではない
    BOOST_CHECK(!(KingOpenMove<WHITE>::isMember(state,GOLD,Square(3,2),Square(4,2))));
    // 開き王手
    BOOST_CHECK((KingOpenMove<WHITE>::isMember(state,PAWN,Square(4,1),Square(4,2))));
  }
  {
    // 
    NumEffectState state=CsaString(
"P1+NY+RY *  *  * -FU-OU-KE-KY\n"
"P2 *  *  *  *  *  * -KI *  *\n"
"P3 * -FU *  *  * +GI-KI-FU-FU\n"
"P4 * +FU * -FU *  *  *  *  *\n"
"P5 *  * -KE * +FU-KA * +FU *\n"
"P6+KE * +FU+FU+GI *  *  * +FU\n"
"P7 *  * -UM *  *  *  *  *  *\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 * +OU * -GI *  *  *  * -NG\n"
"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
"P-00KI00KY00FU00FU\n"
"P-00AL\n"
"+\n"
).initialState();
    // 開き王手ではない
    BOOST_CHECK(!(KingOpenMove<BLACK>::isMember(state,SILVER,Square(5,6),Square(6,7))));
  }
}

static bool isKingOpenMove(const NumEffectState& state_org,Move move){
  NumEffectState state = state_org;
  state.makeMove(move);  
  return state.hasEffectAt(alt(move.player()),state.kingSquare(move.player()));
}

static void testMoveFile(const std::string& filename){
  auto record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  auto moves=record.moves;
  for(unsigned int i=0;i<moves.size();i++){
    MoveVector all_moves;
    Player turn=state.turn();
    // 敵から王手がかかっている時は扱わない
    // 本当は扱った方が良いが
    if(state.hasEffectAt(alt(turn),state.kingSquare(turn)))
      continue;
    state.generateAllUnsafe(all_moves);
    for(size_t j=0;j<all_moves.size();j++){
      Move move=all_moves[j];
      // dropmoveは扱わない
      if (move.isDrop()) 
	  continue;
      // Kingが動く手は扱わない
      if (move.ptype()==KING) 
	  continue;
      BOOST_CHECK_EQUAL(isKingOpenMove(state,move),
			   PlayerMoveAdaptor<KingOpenMove>
			   ::isMember(state,move));
    }
    state.makeMove(moves[i]);
  }
}

BOOST_AUTO_TEST_CASE(KingOpenMoveTestFile){
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
