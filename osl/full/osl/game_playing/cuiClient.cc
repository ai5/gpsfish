/* cuiClient.cc
 */
#include "osl/game_playing/cuiClient.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/csaLogger.h"
#include "osl/search/moveWithComment.h"

osl::game_playing::
CuiClient::CuiClient(ComputerPlayer *black, ComputerPlayer *white,
		     CsaLogger *l, std::istream& i, std::ostream& o)
  : GameManager(black, white, l), is(i), os(o), stop_by_outside(0)
{
}

osl::game_playing::
CuiClient::~CuiClient()
{
}

void osl::game_playing::
CuiClient::run(const char *black, const char *white)
{
  logger->init(black, white, state->state());
  run();
}

void osl::game_playing::
CuiClient::run()
{
  try
  {
    logger->writeComment("game start");
    while (1)
    {
      while (! isComputer(state->state().turn()))
      {
	while (readAndProcessCommand())
	  ;
      }
      int seconds=0;
      MoveWithComment best_move;
      if (! stop_by_outside)
      {
	best_move = computeMove(seconds);
      }
      else
      {
	best_move = MoveWithComment(Move::INVALID());
	logger->writeComment("forced resign");
      }
      processComputerMove(best_move, seconds);
    }
  }
  catch (EndGame&)
  {
    logger->writeComment("game end");
  }
  return;
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
