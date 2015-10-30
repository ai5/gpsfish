#include "osl/game_playing/gameState.h"
#include "osl/game_playing/searchPlayer.tcc"
#include "osl/sennichite.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace game_playing;

BOOST_AUTO_TEST_CASE(GameStateTestCanPopMove)
{
  SimpleState initialState(HIRATE);
  GameState state(initialState);
  BOOST_CHECK(! state.canPopMove());
  const Move m76(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  state.pushMove(m76);
  BOOST_CHECK(state.canPopMove());
  BOOST_CHECK_EQUAL(initialState, state.initialState());
  
  std::shared_ptr<GameState> state_clone = state.clone();
  BOOST_CHECK(state_clone->canPopMove());
  const Move m84(Square(8,3),Square(8,4),PAWN,PTYPE_EMPTY,false,WHITE);
  state_clone->pushMove(m84);
  BOOST_CHECK(state_clone->canPopMove());

  BOOST_CHECK_EQUAL(m84, state_clone->popMove());
  BOOST_CHECK_EQUAL(m76, state.popMove());
  BOOST_CHECK_EQUAL(initialState, state_clone->initialState());
}

BOOST_AUTO_TEST_CASE(GameStateTestRepetition)
{
  GameState state((SimpleState(HIRATE)));
  const Move m48(Square(3,9),Square(4,8),SILVER,PTYPE_EMPTY,false,BLACK);
  const Move m39(Square(4,8),Square(3,9),SILVER,PTYPE_EMPTY,false,BLACK);
  const Move m42(Square(3,1),Square(4,2),SILVER,PTYPE_EMPTY,false,WHITE);
  const Move m31(Square(4,2),Square(3,1),SILVER,PTYPE_EMPTY,false,WHITE);

  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m48));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m42));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m39));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m31));
  
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m48));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m42));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m39));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m31));

  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m48));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m42));
  BOOST_CHECK_EQUAL(Sennichite::NORMAL(), state.pushMove(m39));
  BOOST_CHECK_EQUAL(Sennichite::DRAW(), state.pushMove(m31));
}


static void testRootHistoryRecord(const SimpleState& sstate, 
				  const std::vector<osl::Move>& moves)
{
  GameState state(sstate);
  for (unsigned int i=0;i<moves.size();i++)
  {
    const Move move = moves[i];
    state.pushMove(move);
    DualDfpn searcher;
    searcher.writeRootHistory(state.counter(), state.moveHistory(),
			      state.state(), state.state().turn());
  }    
}

BOOST_AUTO_TEST_CASE(GameStateTestRootHistoryImmediateUse)
{
  // 直前に取った駒を使っても大丈夫
  SimpleState state(CsaString(
		      "P1 * -KE *  *  *  * -KI *  * \n"
		      "P2 *  *  *  *  *  * -KI *  * \n"
		      "P3-FU *  * -FU *  * -KE * +HI\n"
		      "P4 *  * -FU *  * -FU * -FU * \n"
		      "P5 *  * +KI+FU * -KY *  *  * \n"
		      "P6 *  *  *  *  *  *  * +KE-OU\n"
		      "P7+FU *  *  * +FU * +FU+FU * \n"
		      "P8+KA * +GI+OU *  * -NG-HI-KY\n"
		      "P9+KY+KE *  *  * +KI *  * +KY\n"
		      "P+00KA00GI00GI\n"
		      "P-00FU00FU00FU00FU00FU00FU00FU00FU\n"
		      "-\n").initialState());
  std::vector<Move> moves;
  moves.push_back(Move(Square(1,5),PAWN,WHITE));
  moves.push_back(Move(Square(1,7),SILVER,BLACK));
  moves.push_back(Move(Square(1,6),Square(1,7),KING,SILVER,false,WHITE));
  moves.push_back(Move(Square(1,3),Square(1,5),PROOK,PAWN,true,BLACK));
  moves.push_back(Move(Square(1,6),SILVER,WHITE));

  testRootHistoryRecord(state, moves);

}
BOOST_AUTO_TEST_CASE(GameStateTestRootHistoryFiles)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0; i<(OslConfig::inUnitTestShort() ? 3: 300) && (ifs >> file_name); i++){
    if (file_name == "") 
      break;
    if (OslConfig::verbose())
      std::cerr << file_name << "\n";
    file_name = OslConfig::testCsaFile(file_name);
    auto record=CsaFileMinimal(file_name).load();
    const auto& moves=record.moves;
    testRootHistoryRecord(SimpleState(HIRATE), moves);
  }
}

BOOST_AUTO_TEST_CASE(GameStateTestGenerateNotLosingMoves)
{
  {
    GameState state(CsaString(
		      "P1 *  *  *  *  *  *  * -KE-OU\n"
		      "P2 *  *  * -GI * -KI-KI *  * \n"
		      "P3 *  *  * -FU *  * -GI-FU+UM\n"
		      "P4-KY-FU *  * -FU+FU-FU * -KY\n"
		      "P5 *  *  *  *  * -FU * +KE * \n"
		      "P6-HI * -FU+FU+FU *  * +GI * \n"
		      "P7+KE+FU * +KI *  * +KE+FU * \n"
		      "P8+FU+OU+KI+KA * -NY *  *  * \n"
		      "P9 *  *  *  *  *  *  *  *  * \n"
		      "P+00FU00FU\n"
		      "P-00HI00GI00KY00FU00FU00FU\n"
		      "+\n").initialState());
    MoveVector normal_or_win_or_draw, loss;
    state.generateNotLosingMoves(normal_or_win_or_draw, loss);
    const Move m12fu(Square(1,2),PAWN,BLACK);
    BOOST_CHECK(! normal_or_win_or_draw.isMember(m12fu));
  }
  {
    // loss by kachi
    GameState state(CsaString(
		      "P1 *  * +RY *  * +KI * -KE * \n"
		      "P2+OU+TO+UM+UM+TO * -KI-OU * \n"
		      "P3-FU+FU+FU+FU+FU-GI-GI *  * \n"
		      "P4 *  *  *  * -FU-FU-FU-FU * \n"
		      "P5 *  * -RY-NY-GI *  *  *  * \n"
		      "P6 *  *  *  *  * +FU+FU+FU * \n"
		      "P7 *  *  *  *  * +KI+KE * -NY\n"
		      "P8 *  *  *  *  * +KI+GI *  * \n"
		      "P9 *  *  *  * -NY *  *  *  * \n"
		      "P+00FU00FU00FU00FU00KY00KE00KE\n"
		      "-\n").initialState());
    MoveVector normal_or_win_or_draw, loss;
    state.generateNotLosingMoves(normal_or_win_or_draw, loss);
    BOOST_CHECK(normal_or_win_or_draw.size() == 2); // -7573RY or -4352GI
  }  
}

BOOST_AUTO_TEST_CASE(GameStateTestGenerateMoves)
{
  {
    CsaString csa("P1-KY-KE-OU-KI *  *  *  * -KY\n"
		  "P2 *  * -GI *  *  *  * -KI * \n"
		  "P3-FU * -FU-FU-FU-FU *  *  * \n"
		  "P4 *  *  *  *  *  * -HI-GI-FU\n"
		  "P5 *  * +FU *  * +KE *  *  * \n"
		  "P6 *  * +KE *  * +FU-FU *  * \n"
		  "P7+FU+FU * +FU+FU *  * +HI+FU\n"
		  "P8 * +GI+KI+OU *  *  *  *  * \n"
		  "P9+KY+KE *  *  * +KI *  * +KY\n"
		  "P+00KA00FU00FU00FU\n"
		  "P-00KA00GI00FU\n"
		  "+\n"
		  "+0035FU\n"
		  "-3494HI\n");
    GameState state(csa.initialState());
    const auto& moves = csa.load().moves;
    BOOST_FOREACH(Move move, moves)
      state.pushMove(move);
    MoveVector normal, win, draw, loss;
    state.generateMoves(normal, win, draw, loss);
    BOOST_CHECK(loss.isMember(Move(Square(3,5),Square(3,4),PAWN,PTYPE_EMPTY,false,BLACK)));
    BOOST_CHECK(loss.size() == 1);
  }
  {
    CsaString csa("P1-KY *  * -KI *  *  * -KE-KY\n"
		  "P2 * -OU-GI * -KI-HI *  *  * \n"
		  "P3 * -FU-KE * -FU * -KA-FU * \n"
		  "P4-FU * -FU-FU-GI * -FU * -FU\n"
		  "P5 *  *  *  *  * -FU * +FU * \n"
		  "P6+FU+FU+FU+FU+FU * +FU *  * \n"
		  "P7 * +GI+KA+KI+GI+FU *  * +FU\n"
		  "P8 * +OU+KI *  *  *  * +HI * \n"
		  "P9+KY+KE *  *  *  *  * +KE+KY\n"
		  "+\n"
		  "+2838HI\n"
		  "-3344KA\n"
		  "+3828HI\n");
    GameState state(csa.initialState());
    const auto& moves = csa.load().moves;
    for (Move move: moves)
      state.pushMove(move);
    const Move m33ka(Square(4,4),Square(3,3),BISHOP,PTYPE_EMPTY,false,WHITE);

    MoveVector normal, win, draw, loss;
    state.generateMoves(normal, win, draw, loss);

    BOOST_CHECK(! loss.isMember(m33ka));
    BOOST_CHECK(draw.isMember(m33ka));
    BOOST_CHECK(loss.empty());
    BOOST_CHECK_EQUAL((size_t)1u, draw.size());
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
