#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <boost/progress.hpp>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::move_generator;
using namespace osl::move_action;

BOOST_AUTO_TEST_CASE(GenerateAllMovesTestOne){
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI-OU * -GI+RY-KY\n"
			   "P2 *  *  *  *  *  * -KI *  * \n"
			   "P3-FU * -GI-FU-FU-FU *  * -FU\n"
			   "P4 *  * -FU *  *  *  *  *  * \n"
			   "P5 *  *  *  * +KA *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			   "P8 * +GI+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  * +OU+KI+GI * +KY\n"
			   "P+00KE00FU00FU\n"
			   "P-00HI00KA00KE00FU00FU00FU\n"
			   "-\n"
			   ).initialState());
    MoveVector moves;
    GenerateAllMoves::generate(WHITE,state,moves);
    BOOST_CHECK(!moves.isMember(Move(Square(3,1),Square(2,2),SILVER,PTYPE_EMPTY,false,WHITE)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * +TO *  *  *  *  *  * \n"
			   "P2 *  *  *  *  *  *  * -GI * \n"
			   "P3 *  *  *  *  *  * -KE-FU * \n"
			   "P4-FU *  * +UM-FU-FU *  * +FU\n"
			   "P5 *  *  *  *  *  *  *  * -OU\n"
			   "P6 *  *  *  * +FU * -FU+KE+KE\n"
			   "P7+FU *  *  *  * +FU+FU+FU+KY\n"
			   "P8 *  *  *  *  *  * +KI *  * \n"
			   "P9+KY-RY *  *  *  * +KI+OU * \n"
			   "P+00HI00GI00KE00FU00FU00FU00FU00FU\n"
			   "P-00KA00KI00KI00GI00GI00KY00FU\n"
			   "+\n"
			   ).initialState());
    MoveVector moves;
    GenerateAllMoves::generate(BLACK,state,moves);
    BOOST_CHECK(moves.isMember(Move(Square(1,6),Square(2,4),KNIGHT,PTYPE_EMPTY,false,BLACK)));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  * -KA * -OU-KE-KY\n"
			   "P2-KY * -KI * -HI *  *  *  * \n"
			   "P3 *  * -KE-GI-KI *  * -FU-FU\n"
			   "P4-FU *  *  *  * -FU-FU *  * \n"
			   "P5 * -FU+KA-FU *  * -GI+FU+KY\n"
			   "P6+FU * +FU * +FU+FU *  * +HI\n"
			   "P7 * +FU * +FU * +GI *  *  * \n"
			   "P8 * +OU+KI+GI+KI *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  *  * \n"
			   "P+00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "-\n"
			   ).initialState());
    MoveVector moves;
    GenerateAllMoves::generate(WHITE,state,moves);
    BOOST_CHECK(moves.isMember(Move(Square(5,3),Square(6,4),GOLD,PTYPE_EMPTY,false,WHITE)));
  }
}

BOOST_AUTO_TEST_CASE(GenerateAllMovesTestMoveGen) {
  NumEffectState state0(CsaString(
			  "P1+NY+TO *  *  *  * -OU-KE-KY\n"
			  "P2 *  *  *  *  * -GI-KI *  *\n"
			  "P3 * +RY *  * +UM * -KI-FU-FU\n"
			  "P4 *  * +FU-FU *  *  *  *  *\n"
			  "P5 *  * -KE * +FU *  * +FU *\n"
			  "P6-KE *  * +FU+GI-FU *  * +FU\n"
			  "P7 *  * -UM *  *  *  *  *  *\n"
			  "P8 *  *  *  *  *  *  *  *  * \n"
			  "P9 * +OU * -GI *  *  *  * -NG\n"
			  "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			  "P-00KI00KY00FU00FU\n"
			  "P-00AL\n"
			  "+\n").initialState());
  MoveVector moves;
  state0.generateAllUnsafe(moves);
  for (Move move:moves) {
    BOOST_CHECK(state0.isValidMove(move));
    BOOST_CHECK(state0.isSafeMove(move));
    BOOST_CHECK(!move.ignoreUnpromote());
  }
  state0.makeMove(moves[0]);
  moves.clear();
  state0.generateAllUnsafe(moves);
  for (Move move:moves){
    BOOST_CHECK(state0.isValidMove(move));
  }
}

static void testMoveFile(const std::string& filename){
  RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initial_state);
  for (auto record_move:record.moves) {
    MoveVector all;
    state.generateAllUnsafe(all);
    // 動いた結果王が取られる手は生成しない
    for (Move move:all) {
      BOOST_CHECK(state.isSafeMove(move));
      BOOST_CHECK(state.isValidMove(move));
    }
    if (record_move.ignoreUnpromote()) {
      BOOST_CHECK(all.isMember(record_move.promote()));
    }
    else
      BOOST_CHECK(all.isMember(record_move));
    state.makeMove(record_move);
  }
}

BOOST_AUTO_TEST_CASE(GenerateAllMovesTestMoveMember){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=10000;
  if (OslConfig::inUnitTestShort()) 
    count=10;
  std::string filename;
  std::unique_ptr<boost::progress_display> progress;
  if (OslConfig::inUnitTestLong())
    progress.reset(new boost::progress_display(count, std::cerr));
  while((ifs >> filename) && filename !="" && ++i<count){
    if (progress)
      ++(*progress);
    testMoveFile(OslConfig::testCsaFile(filename));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
