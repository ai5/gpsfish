/* csaClient.cc
 */
#include "osl/game_playing/csaClient.h"
#include "osl/game_playing/gnuShogiClient.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/csaLogger.h"
#include "osl/game_playing/csaStopwatch.h"
#include "osl/search/moveWithComment.h"
#include "osl/csa.h"
#include "osl/container/moveStack.h"
#include "osl/sennichite.h"
#include <iostream>
#ifdef _WIN32
#include <ctime>
#include <cstring>
#endif

osl::game_playing::
CsaClient::CsaClient(ComputerPlayer *black, ComputerPlayer *white,
		     CsaLogger *l, std::istream& is, std::ostream& os)
  : CuiClient(black, white, l, is, os),
    show_move_with_comment(false), silent(false), line(128,' ')
{
  setComputerPlayer(WHITE, true);
}

osl::game_playing::
CsaClient::~CsaClient()
{
}

bool osl::game_playing::
CsaClient::readAndProcessCommand()
{
  char ctime_buf[64];
  if (! silent) {
    std::cerr << "\nCsaClient start waiting ";
    const time_t now = time(0);
    std::cerr << ctime_r(&now, ctime_buf) 
	      << state->state()
	      << "TIME[" << time_keeper.timeElapsed(BLACK)
	      << ":" << time_keeper.timeElapsed(WHITE)
	      << "] ";
    const MoveStack& history = state->moveHistory();
    const std::vector<int>& eval_history = state->evalStack();
    for (int i=1; i<=8 && history.hasLastMove(i); ++i) {
      std::cerr << "(" << history.size() - i + 1 << ")" << csa::show(history.lastMove(i));
      if (i-1 < (int)eval_history.size() && eval_history[eval_history.size()-i])
	std::cerr << "<" << eval_history[eval_history.size()-i] << ">";
      std::cerr << " ";
    }
    std::cerr << std::endl << std::endl;
  }
  CsaStopwatch timer;
  std::getline(is, line);
  if (! silent) {
    std::cerr << "\nCsaClient read " << line << " ";
    const time_t now = time(0);
    std::cerr << ctime_r(&now, ctime_buf) 
	      << std::endl;
  }
  const long op_think_time = timer.read();
  if (! is)
  {
    const char *message = "istream error (maybe closed)";
    std::cerr << message << std::endl;
    logger->writeComment(message);
    throw EndGame();
  }
  
  if (line == "%TORYO")
  {
    logger->resign(state->state().turn());
    throw EndGame();
  }

  // TODO: %MATTA, %CHUDAN
  try
  {
    const Move op_move=csa::strToMove(line, state->state());
    const GameState::MoveType illegal_move = state->isIllegal(op_move);
    if (illegal_move)
    {
      std::cerr << "illegal move: " << line << "\n";
      logger->inputError(line.c_str());
      if (illegal_move == GameState::PAWN_DROP_FOUL)
	logger->writeComment("pawn drop foul");
      else if (illegal_move == GameState::UNSAFE_KING)
	logger->writeComment("unsafe king");
      else if (illegal_move == GameState::OTHER_INVALID)
	logger->writeComment("other illegal move");
      os << "%CHUDAN" << std::endl;
      throw EndGame();
    }
    const Sennichite result = pushMove(MoveWithComment(op_move), op_think_time);
    if (! result.isNormal())
    {
      os << "%SENNICHITE" << std::endl;
      logger->endByRepetition(result);
      throw EndGame();
    }
    if (! silent) {
      std::cerr << state->state()
		<< "TIME[" << time_keeper.timeElapsed(BLACK)
		<< ":" << time_keeper.timeElapsed(WHITE)
		<< "] ";
      const MoveStack& history = state->moveHistory();
      const std::vector<int>& eval_history = state->evalStack();
      for (int i=1; i<=8 && history.hasLastMove(i); ++i) {
	std::cerr << "(" << history.size() - i + 1 << ")" << csa::show(history.lastMove(i));
	if (i-1 < (int)eval_history.size() && eval_history[eval_history.size()-i])
	  std::cerr << "<" << eval_history[eval_history.size()-i] << "> ";
	std::cerr << " ";
      }
      std::cerr << std::endl << std::endl;
    }
  }
  catch (csa::CsaIOError&)
  {
    std::cerr << "bad input: " << line << "\n";
    logger->inputError(line.c_str());
    throw EndGame();
  }
  return false;
}

void osl::game_playing::
CsaClient::setShowMoveWithComment(bool value)
{
  show_move_with_comment = value;
}

void osl::game_playing::
CsaClient::processComputerMove(const MoveWithComment& selected, 
			       int my_think_time)
{
  static std::string reserved="+7776FU";
  const Move best_move = selected.move;
  if ((! best_move.isNormal())
      || (state->isIllegal(best_move)))
  {
    if (best_move == Move::DeclareWin())
    {
      os << "%KACHI\n";
      logger->endByDeclaration(state->state().turn());
    }
    else
    {
      if (best_move.isNormal()) {
	std::cerr << "error: prefer resign to playing illegal move " << best_move << " code " << state->isIllegal(best_move) << "\n";
	logger->writeComment("error: prefer abort to playing illegal move");
	abort();
      }
      os << "%TORYO\n";
      logger->resign(state->state().turn());
    }
    throw EndGame();
  }

  os << csa::show(best_move, reserved);
  if (show_move_with_comment && (! selected.moves.empty() || selected.value != 0))
  {
    os << ",'* " << selected.value;
    for (Move move: selected.moves)
    {
      os << " ";
      os << csa::show(move, reserved);
    }    
  }
  os << std::endl << std::flush;

  assert(isComputer(state->state().turn()));

  const Sennichite result = pushMove(selected, my_think_time);
  if (! result.isNormal())
  {
    logger->endByRepetition(result);
    throw EndGame();
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
