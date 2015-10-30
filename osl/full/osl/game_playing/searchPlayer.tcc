/* searchPlayer.tcc
 */
#ifndef GAMEPLAYING_SEARCHPLAYER_TCC
#define GAMEPLAYING_SEARCHPLAYER_TCC
#include "osl/game_playing/searchPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/pvHistory.h"
#include "osl/search/searchState2.h"
#include "osl/eval/evalTraits.h"
#include "osl/container/moveStack.h"

#ifdef USE_NTESUKI
#  include "osl/ntesuki/ntesukiSearcher.h"
#  include "osl/ntesuki/ntesukiMoveGenerator.h"
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

#ifdef USE_NTESUKI
struct
osl::game_playing::SearchPlayer::NtesukiThread
{
  explicit NtesukiThread(Move& next_move,
			 volatile bool *thread_finished,
			 volatile bool *stop_flag,
			 NumEffectState state);

  void operator()();

  Move& next_move;
  volatile bool *thread_finished;
  volatile bool *stop_flag;
  NumEffectState state;
};
#endif

template <class Searcher>
osl::game_playing::ComputerPlayer* osl::game_playing::
SearchPlayer::cloneIt(const Searcher& copy) const
{
  return new Searcher(copy);
}

template <class Searcher>
int osl::game_playing::
SearchPlayer::pawnValue()
{
  typedef typename Searcher::eval_t eval_t;
  return abs(eval_t::captureValue(newPtypeO(BLACK,PAWN)))/2;
}
template <class Searcher>
int osl::game_playing::
SearchPlayer::pawnValueOfTurn(Player turn)
{
  return pawnValue<Searcher>() * eval::delta(turn);
}

template <class Searcher>
const osl::search::MoveWithComment osl::game_playing::
SearchPlayer::search(const GameState& state, const search::TimeAssigned& msec)
{
  Searcher& searcher = dynamic_cast<Searcher&>(*this->searcher);
  searcher.setRootIgnoreMoves(root_ignore_moves, prediction_for_speculative_search);
  
  typedef typename Searcher::eval_t eval_t;
  if (! eval_t::initialized())
    throw std::runtime_error("evaluation function not initialized");
  
  const MoveStack& history = state.moveHistory();

#ifdef USE_NTESUKI
  volatile bool ntesuki_thread_finished;
  volatile bool stop_ntesuki;
  Move ntesuki_next_move = Move::INVALID();

  NtesukiThread thread(ntesuki_next_move, &ntesuki_thread_finished,
		       &stop_ntesuki, state.state());
  boost::thread ntesuki_thread(thread);
#endif
  searcher.setHistory(history);
  searcher.enableMultiPV(config.multi_pv_width);
  for (const std::shared_ptr<search::SearchMonitor>& m: config.monitors)
    searcher.addMonitor(m);
  int deepening_step_for_this_move = config.deepening_step;
  if (msec.standard < milliseconds::max())
  {
    if (toSeconds(msec.standard) < 10.0)
      deepening_step_for_this_move = std::min(100, config.deepening_step);
  }
  searcher.setNextIterationCoefficient(config.next_iteration_coefficient);
  searcher.setNodeCountHardLimit(config.node_count_hard_limit);

  MoveWithComment best_move;
  best_move.root = HashKey(state.state());
  if (plan_stop)
    return best_move;

  searching = true;
  best_move.move 
    = searcher.computeBestMoveIteratively(config.limit, 
					  deepening_step_for_this_move,
					  config.initial_limit,
					  config.node_limit,
					  msec,
					  &best_move);
  searching = false;
#ifdef USE_NTESUKI
  if (ntesuki_thread_finished)
  {
    if (ntesuki_next_move.isNormal())
    {
      return ntesuki_next_move;
    }
    else
    {
      //ntesuki_finished
    }
  }
  else
  {
    //force_finish
    stop_ntesuki = true;
    ntesuki_thread.join();
  }
#endif
  saveSearchResult(state, best_move);
  static const int resign_value = OslConfig::resignThreshold();
  if (! state.state().inCheck() && best_move.move.isNormal()
      && best_move.value*sign(state.state().turn()) < -resign_value) 
  {
    ++almost_resign_count;
    if (almost_resign_count >= 3)
      best_move.move = Move();
  }
  else 
  {
    almost_resign_count = 0;
  }
  return best_move;
}

template <class Searcher>
bool osl::game_playing::
SearchPlayer::isReasonableMoveBySearch(Searcher& searcher, Move move, int pawn_sacrifice)
{
  return searcher.isReasonableMove(move, pawn_sacrifice);
}

#endif /* GAMEPLAYING_SEARCHPLAYER_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
