#include "osl/move_classifier/safeMove.h"
#include "osl/move_generator/allMoves.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::move_action;
using namespace osl::move_generator;
using namespace osl::move_classifier;

BOOST_AUTO_TEST_CASE(SafeMoveTestIsSafe){
  {
    NumEffectState state=CsaString(
"P1+NY+TO *  *  *  * -OU-KE-KY\n"
"P2 *  *  *  *  * -GI-KI *  *\n"
"P3 * -RY *  *  *  * -KI-FU-FU\n"
"P4 * +FU * -FU *  *  *  *  *\n"
"P5 *  * -KE * +FU *  * +FU *\n"
"P6+KE *  * +FU+GI-FU *  * +FU\n"
"P7 *  * -UM-KA *  *  *  *  *\n"
"P8 *  * +FU *  *  *  *  *  * \n"
"P9 * +OU * -GI *  *  *  * -NG\n"
"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
"P-00KI00KY00FU00FU\n"
"P-00AL\n"
"+\n"
).initialState();
    // 王が動くが安全
    BOOST_CHECK((SafeMove<BLACK>::isMember(state,KING,Square(8,9),Square(7,9))));
    // 王が動いて危険
    BOOST_CHECK((!SafeMove<BLACK>::isMember(state,KING,Square(8,9),Square(8,8))));
    // 龍をblockしている歩が動くが相手の龍を取るので安全
    BOOST_CHECK((SafeMove<BLACK>::isMember(state,PAWN,Square(8,4),Square(8,3))));
    // 角をblockしている歩が動いて危険
    BOOST_CHECK(!(SafeMove<BLACK>::isMember(state,PAWN,Square(7,8),Square(7,7))));
  }
  {
    NumEffectState state=CsaString(
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
    // 安全
    BOOST_CHECK((SafeMove<WHITE>::isMember(state,PAWN,Square(4,3),Square(4,4))));
  }
  {
    NumEffectState state=CsaString(
"P1-KY-KE * -KI-OU * -GI+RY-KY\n"
"P2 *  *  *  *  *  * -KI *  * \n"
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
"-\n"
).initialState();
    // 安全ではない
    BOOST_CHECK(!(SafeMove<WHITE>::isMember(state,SILVER,Square(3,1),Square(2,2))));
  }
  {
    NumEffectState state=CsaString(
      "P1 *  *  *  *  *  *  *  * -KY\n"
      "P2 *  *  * +TO * -GI *  * -GI\n"
      "P3-OU+FU *  *  * -KI *  * -FU\n"
      "P4 *  *  * -GI-FU * -FU-FU * \n"
      "P5 * -HI *  *  *  *  *  *  * \n"
      "P6 *  * +OU+FU+FU * +FU *  * \n"
      "P7+FU *  *  * +KI *  *  * +FU\n"
      "P8 * -NY * -NK *  *  *  *  * \n"
      "P9+KY *  *  *  *  *  * +KE+KY\n"
      "P+00FU00FU\n"
      "P-00HI00KA00KA00KI00KI00GI00KE00KE00FU00FU00FU00FU00FU\n"
      "+\n").initialState();
    BOOST_CHECK(!(SafeMove<BLACK>::isMember(state,KING,Square(7,6),Square(8,6))));    
  }
}

static bool confirmSafeMove(const NumEffectState& state_org,Move move){
  NumEffectState state = state_org;
  state.makeMove(move);
  return !state.hasEffectAt(alt(move.player()),state.kingSquare(move.player()));
}

static void testMoveFile(const std::string& filename){
  auto record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  const auto& moves=record.moves;
  for(unsigned int i=0;i<moves.size();i++){
    MoveVector allMoves;
    Player turn=state.turn();
    // 敵から王手がかかっている時は扱わない
    if(state.hasEffectAt(alt(turn),state.kingSquare(turn)))
      continue;
    {
      Store store(allMoves);
      AllMoves<Store>::
	generate(turn,state,store);
    }
    for(size_t j=0;j<allMoves.size();j++){
      Move move=allMoves[j];
      // dropmoveは扱わない
      if (move.isDrop()) continue;
      if(turn==BLACK)
	BOOST_CHECK(confirmSafeMove(state,move) ==
		       (SafeMove<BLACK>::isMember(state,move.ptype(),move.from(),move.to())));
      else
	BOOST_CHECK(confirmSafeMove(state,move) ==
		       (SafeMove<WHITE>::isMember(state,move.ptype(),move.from(),move.to())));
    }
    state.makeMove(moves[i]);
  }
}

BOOST_AUTO_TEST_CASE(SafeMoveTestFile){
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
