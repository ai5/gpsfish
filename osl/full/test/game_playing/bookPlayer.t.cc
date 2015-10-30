#include "osl/game_playing/bookPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/recordTracer.h"
#include "osl/sennichite.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace osl;
using namespace osl::game_playing;

BOOST_AUTO_TEST_CASE(BookPlayerTestUndo) 
{
  std::vector<Move> moves;
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  const Move m34fu(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE);

  moves.push_back(m76fu);
  moves.push_back(m34fu);
    
  BookPlayer white(new RecordTracer(moves), new ResignPlayer());
  GameState state((SimpleState(HIRATE)));

  white.pushMove(m76fu);
  state.pushMove(m76fu);
  BOOST_CHECK_EQUAL(m34fu, white.selectBestMove(state, 20, 10, 0).move);
    
  white.popMove();

  white.pushMove(m76fu);
  BOOST_CHECK_EQUAL(m34fu, white.selectBestMove(state, 20, 10, 0).move);

  white.pushMove(m34fu);
  state.pushMove(m34fu);

  const Move m26fu(Square(2,7),Square(2,6),PAWN,PTYPE_EMPTY,false,BLACK);
  white.pushMove(m26fu);
  state.pushMove(m26fu);
  BOOST_CHECK_EQUAL(Move::INVALID(), 
		    white.selectBestMove(state, 20, 10, 0).move);
}
BOOST_AUTO_TEST_CASE(BookPlayerTestLimit)
{
  std::vector<Move> moves;
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  const Move m34fu(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE);

  moves.push_back(m76fu);
  moves.push_back(m34fu);

  BookPlayer player(new RecordTracer(moves), new ResignPlayer());
  BOOST_CHECK(player.bookAvailable());

  player.pushMove(m76fu);

  BOOST_CHECK(player.bookAvailable());
    
  player.setBookLimit(0);

  BOOST_CHECK(! player.bookAvailable());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
