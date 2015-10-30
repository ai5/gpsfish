/* gnuShogiClient.cc
 */
#include "osl/game_playing/gnuShogiClient.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/csaLogger.h"
#include "osl/game_playing/csaStopwatch.h"
#include "osl/search/moveWithComment.h"
#include "osl/usi.h"
#include "osl/sennichite.h"
#include <iostream>
#include <sstream>

osl::game_playing::
GnuShogiClient::GnuShogiClient(ComputerPlayer *black, ComputerPlayer *white,
			       CsaLogger *l,
			       std::istream& is, std::ostream& os)
  : CuiClient(black, white, l, is, os)
{
}

osl::game_playing::
GnuShogiClient::~GnuShogiClient()
{
}

bool osl::game_playing::
GnuShogiClient::readAndProcessCommand()
{
  CsaStopwatch timer;
  std::string line;
  std::getline(is, line);
  const long op_think_time = timer.read();
  if (! is) {
    logger->inputError("(stream not available)");
    throw EndGame();
  }

  if (line == "quit" || line == "exit")
    throw GnuShogiQuit();

  if (line == "undo")
  {
    logger->popMove();
    popMove();
    return false;
  }
  if (line == "force")
  {
    setComputerPlayer(BLACK, false);
    setComputerPlayer(WHITE, false);
    logger->breakGame();
    return false;
  }
  if (line == "black") {
    setComputerPlayer(BLACK, true);
    setComputerPlayer(WHITE, false);
    return true;
  }
  if (line == "white") {
    setComputerPlayer(BLACK, false);
    setComputerPlayer(WHITE, true);
    return true;
  }
  if (line == "go") {
    const Player turn = state->state().turn();
    if (! isComputer(turn)) {
      setComputerPlayer(turn, true);
      setComputerPlayer(alt(turn), false);
    }
    return false;
  }
  if (line == "new" || line == "beep" || line == "random")
    return false; // XXX
  if (line.find("time") == 0 || line.find("otime") == 0) {
    if (line.find("time") == 0) {
      std::istringstream ss(line);
      std::string dummy;
      int time;
      ss >> dummy >> time;
      setTimeLeft(time/100, time/100);
    }
    std::cerr << line << "\n";
    return false;
  }
  if (line.find("help") == 0) {
    std::cerr << "  2g7f        move a piece from 2g to 2f\n"
      "  P*2d        drop a pawn to 2d\n"
      "  undo        undo the last move\n"
      "  force       human plays both colors\n"
      "  black/white set computer's color\n"
      "  exit/quit   exit program\n";
    return true;
  }
  
  if (line.size() < 4)
    goto ignore;
  if (isdigit(line[0]) || line[1] == '*') // FIXME
  {
    const Move op_move=psn::strToMove(line, state->state());
    
    if (state->isIllegal(op_move))
    {
      os << "Illegal move\n";
      logger->inputError(line.c_str());
      return true;
    }
    const Sennichite result = pushMove(MoveWithComment(op_move), op_think_time);
    if (! result.isNormal())
    {
      if (result == Sennichite::BLACK_LOSE())
	os << "White mates!\n";
      else if (result == Sennichite::WHITE_LOSE())
	os << "Black mates!\n";
      else 
	os << "Black mates!\n";	// does xshogi know draw?
      setComputerPlayer(BLACK, false);
      setComputerPlayer(WHITE, false);
      logger->endByRepetition(result);
      throw EndGame();
    }
    const int chess_moves = state->chessMoves();
    os << chess_moves << ". " << psn::show(op_move)
       << " " << (time_keeper.timeLeft(op_move.player()) - op_think_time)*100
       << std::endl << std::flush;
    return false;
  }
ignore:
  std::cerr << line << "\n";
  std::string comment = "ignored line " + line;
  logger->writeComment(comment.c_str());
  return false;
}

void osl::game_playing::
GnuShogiClient::processComputerMove(const MoveWithComment& selected, 
				    int my_think_time)
{
  const Move best_move = selected.move;
  const Player turn = state->state().turn();
  if ((! best_move.isNormal())
      || (state->isIllegal(best_move)))
  {
    os << ((alt(turn) == BLACK) ? "Black" : "White")
	      << " mates!\n";
    logger->resign(turn);
    setComputerPlayer(BLACK, false);
    setComputerPlayer(WHITE, false);
    throw EndGame();
  }

  const int chess_moves = state->chessMoves();
  os << chess_moves << ". ... " << psn::show(best_move)
     << " " << (time_keeper.timeLeft(turn) - my_think_time)*100
     << std::endl << std::flush;

  const Sennichite result = pushMove(selected, my_think_time);
  if (! result.isNormal())
  {
    if (result == Sennichite::BLACK_LOSE())
      os << "White mates!\n";
    else if (result == Sennichite::WHITE_LOSE())
      os << "Black mates!\n";
    else 
      os << "Black mates!\n";	// does xshogi know draw?
    setComputerPlayer(BLACK, false);
    setComputerPlayer(WHITE, false);
    logger->endByRepetition(result);
    throw EndGame();
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
