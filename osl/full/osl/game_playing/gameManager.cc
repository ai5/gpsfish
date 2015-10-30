/* gameManager.cc
 */
#include "osl/game_playing/gameManager.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/computerPlayer.h"
#include "osl/game_playing/csaLogger.h"
#include "osl/game_playing/csaStopwatch.h"
#include "osl/record/csaRecord.h"
#include "osl/sennichite.h"
#include <iostream>

osl::game_playing::
GameManager::GameManager(ComputerPlayer *black, ComputerPlayer *white,
			 CsaLogger *l)
  : state(new GameState(SimpleState(HIRATE))), logger(l),
    byoyomi(0)
{
  players[BLACK] = black;
  players[WHITE] = white;
  computers[BLACK] = false;
  computers[WHITE] = false;
}

osl::game_playing::
GameManager::~GameManager()
{
}

void osl::game_playing::
GameManager::setComputerPlayer(Player turn, bool is_computer)
{
  computers[turn] = is_computer;
  if (players[turn])
    players[turn]->allowSpeculativeSearch(is_computer);
}

void osl::game_playing::
GameManager::resetLogger(CsaLogger *l)
{
  logger.reset(l);
}

void osl::game_playing::
GameManager::setTimeLeft(int black_time, int white_time)
{
  time_keeper.reset(black_time, white_time);
}

void osl::game_playing::
GameManager::load(const char *csa_filename, bool verbose)
{
  if (state->moves())
  {
    std::cerr << "GameManager: game already started, load failure\n";
    return;
  }
  if (verbose)
    std::cerr << "loading " << csa_filename << "\n";
  CsaFile csa_file(csa_filename);
  state.reset(new GameState(csa_file.initialState()));
  logger->init(csa_file.load().player[BLACK].c_str(), 
	       csa_file.load().player[WHITE].c_str(), state->state());
  if (player(BLACK))
    player(BLACK)->setInitialState(state->state());
  if (player(WHITE))
    player(WHITE)->setInitialState(state->state());
  std::vector<Move> moves;
  std::vector<int> times;
  csa_file.load().load(moves, times);
  assert(moves.size() == times.size());
  for (size_t i=0; i<moves.size(); ++i)
  {
    if (verbose)
      std::cerr << csa::show(moves[i]) << "\n"
		<< "T" << times[i] << "\n";
    if ((! state->state().isValidMove(moves[i]))
	|| (! pushMove(MoveWithComment(moves[i]), times[i]).isNormal()))
    {
      std::cerr << "invalid move " << i << " " << moves[i] << "\n";
      break;
    }
  }
}

const osl::search::MoveWithComment osl::game_playing::
GameManager::computeMove(int& consumed)
{
  const Player turn = state->state().turn();
  CsaStopwatch timer;
  const MoveWithComment best_move
    = player(turn)->selectBestMove(*state, time_keeper.timeLimit(turn), time_keeper.timeElapsed(turn), byoyomi);
  consumed = timer.read();
  return best_move;
}

const osl::Sennichite osl::game_playing::
GameManager::pushMove(const MoveWithComment& move, int seconds)
{
  assert(state->state().isValidMove(move.move));
  time_keeper.pushMove(move.move.player(), seconds);
  logger->pushMove(move, seconds);
  logger->showTimeLeft(time_keeper);
  const Sennichite result = state->pushMove(move.move, move.value);
  if (player(BLACK))
    player(BLACK)->pushMove(move.move);
  if (player(WHITE))
    player(WHITE)->pushMove(move.move);
  return result;
}

void osl::game_playing::
GameManager::popMove()
{
  time_keeper.popMove();
  logger->popMove();
  logger->showTimeLeft(time_keeper);
  state->popMove();
  if (player(BLACK))
    player(BLACK)->popMove();
  if (player(WHITE))
    player(WHITE)->popMove();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
