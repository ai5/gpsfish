/* speculativeSearchPlayer.cc
 */
#include "osl/game_playing/speculativeSearchPlayer.h"
#include "osl/game_playing/speculativeAllMoves.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/searchPlayer.h"
#include "osl/hash/hashKeyStack.h"
#include "osl/container/moveStack.h"
#include "osl/sennichite.h"
#include <iostream>
#include <ctime>
#ifndef _MSC_VER
# include <unistd.h>
#endif


osl::game_playing::SpeculativeSearchPlayer::
SpeculativeSearchPlayer(Player my_turn, SearchPlayer *player)
  : main_player(player), 
    speculative(new SpeculativeAllMoves()),
    my_turn(my_turn)
{
}

osl::game_playing::
SpeculativeSearchPlayer::~SpeculativeSearchPlayer()
{
}

osl::game_playing::ComputerPlayer* 
osl::game_playing::SpeculativeSearchPlayer::clone() const
{
  return new SpeculativeSearchPlayer(my_turn, 
				     dynamic_cast<SearchPlayer*>(main_player->clone()));
}

void osl::game_playing::SpeculativeSearchPlayer::
setMaxThreads(int new_max_threads)
{
  speculative->setMaxThreads(new_max_threads);
}

void osl::game_playing::SpeculativeSearchPlayer::
pushMove(Move m)
{
  main_player->pushMove(m);
  if (m.player() == my_turn)
  {
    if (previous_state.get() && speculative_search_allowed)
    {
#ifndef GPSONE
      if (OslConfig::usiMode())
	OslConfig::setUsiSilent(true);
#endif
      try
      {
	previous_state->pushMove(m);
	speculative->startSpeculative(previous_state, *main_player);
      }
      catch (std::exception& e)
      {
	std::cerr << e.what() << " in SpeculativeSearchPlayer::pushMove\n";
	speculative->clearResource();
      }
      previous_state.reset();
    }
  }
  else
  {
    if (speculative_search_allowed)
      speculative->stopOtherThan(m);
  }
}

void osl::game_playing::SpeculativeSearchPlayer::
popMove()
{
  main_player->popMove();
  previous_state.reset();
  speculative->stopAll();
}

bool osl::game_playing::SpeculativeSearchPlayer::
stopSearchNow()
{
  return main_player->stopSearchNow();
}

osl::search::TimeAssigned osl::game_playing::SpeculativeSearchPlayer::
standardSearchSeconds(const GameState& state, int limit, int elapsed, int byoyomi) const
{
  search::TimeAssigned result = main_player->assignTime(state, limit, elapsed, byoyomi);
  if (result.standard > milliseconds(2000)) {
    result.standard = result.standard - milliseconds(500);
    result.max = result.max - milliseconds(500);
  }
  return result;
}

const osl::search::MoveWithComment
osl::game_playing::SpeculativeSearchPlayer::
selectBestMove(const GameState& state, int limit, int elapsed, int byoyomi)
{
  if (elapsed > limit)
    elapsed = limit;
  const time_t start_time = time(0);
  MoveWithComment result = MoveWithComment(Move::INVALID());
  const Move last_move = state.moveHistory().lastMove();

  const HashKey search_key(speculative->searchState());
  const HashKey now_key(last_move.isNormal() ? state.hashHistory().top(1) : HashKey());
  const bool consistent = (search_key == now_key);

  const search::TimeAssigned wait_for = consistent 
    ? standardSearchSeconds(state, limit, elapsed, byoyomi)
    : search::TimeAssigned(milliseconds(0));
  
  if (last_move.isNormal())
    result = speculative->waitResult(last_move, wait_for, *main_player,
				     byoyomi);

  const time_t now = time(0);
  char ctime_buf[64];
  if (! consistent && result.move.isNormal())
    std::cerr << "note: the current position differs from the one which previous prediction search ran on\n";
  if (result.move.isNormal() && consistent) {
#ifdef DEBUG_SPECULATIVE_EXECUTION
    std::cerr << "returned " << csa::show(result.move) 
	      << " " << ctime_r(&now, ctime_buf);
#endif
    selectBestMoveCleanUp(state);
    main_player->saveSearchResult(state, result);
    return result;
  }
  std::cerr << "search again " << ctime_r(&now, ctime_buf);
  selectBestMoveCleanUp(state);
#ifndef GPSONE
  if (OslConfig::usiMode())
    OslConfig::setUsiSilent(false);
#endif
  const int consumed = (now - start_time);
  if (byoyomi && (limit <= elapsed+consumed))
    byoyomi = std::max(1, byoyomi - (elapsed+consumed-limit));

  result = main_player->selectBestMove(state, limit, std::min(limit-1, elapsed+consumed), byoyomi);
  return result;
}

void osl::game_playing::SpeculativeSearchPlayer::
selectBestMoveCleanUp(const GameState& state)
{
  try
  {
    previous_state = state.clone();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    previous_state.reset();
  }
  speculative->selectBestMoveCleanUp();
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
