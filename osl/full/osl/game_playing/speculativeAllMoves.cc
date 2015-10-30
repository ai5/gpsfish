/* speculativeAllMoves.cc
 */
#include "osl/game_playing/speculativeAllMoves.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/searchPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/search/timeControl.h"
#include "osl/search/searchRecorder.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/usiReporter.h"
#include "osl/misc/milliSeconds.h"
#include "osl/csa.h"
#include "osl/sennichite.h"
#include "osl/misc/lightMutex.h"
#include <thread>
#include <exception>
#include <iostream>
#include <cmath>
#ifndef _MSC_VER
#  include <unistd.h>
#endif


osl::game_playing::SpeculativeAllMoves::
ResultVector::ResultVector()
{
}

osl::game_playing::SpeculativeAllMoves::
ResultVector::~ResultVector()
{
}

void osl::game_playing::SpeculativeAllMoves::ResultVector::
add(Move prediction, const MoveWithComment& result)
{
  SCOPED_LOCK(lk,mutex);
  data.push_back(std::make_pair(prediction, result));
}
const osl::search::MoveWithComment* osl::game_playing::SpeculativeAllMoves::ResultVector::
find(Move prediction) const
{
  SCOPED_LOCK(lk,mutex);
  for (const vector_t::value_type& v: data) {
    if (v.first == prediction)
      return &v.second;
  }
  return 0;
}
void osl::game_playing::SpeculativeAllMoves::ResultVector::
clear()
{
  SCOPED_LOCK(lk,mutex);
  data.clear();
}
void osl::game_playing::SpeculativeAllMoves::ResultVector::
show(std::ostream& os) const
{
  SCOPED_LOCK(lk,mutex);
  for (size_t i=0; i<data.size(); ++i) {
    if (i)
      os << ", ";
    os << csa::show(data[i].first) << "=>" << csa::show(data[i].second.move);
  }
  os << std::endl;
}

/* ------------------------------------------------------------------------- */

struct osl::game_playing::SpeculativeAllMoves::SearchAllMoves::StatusLock
{
  volatile Status& status;
  std::mutex& mutex;
  std::condition_variable *condition;
  const Status in, out;
  StatusLock(volatile Status *s, std::mutex *m, std::condition_variable* c, Status i, Status o)
    : status(*s), mutex(*m), condition(c), in(i), out(o)
  {
    std::lock_guard<std::mutex> lk(mutex);
    status = in;
    condition->notify_all();
  }
  StatusLock(volatile Status *s, std::mutex *m, Status i) 
    : status(*s), mutex(*m), condition(0), in(i), out(*s)
  {
    std::lock_guard<std::mutex> lk(mutex);
    status = in;
  }
  ~StatusLock()
  {
    status = out;
    if (condition)
      condition->notify_all();
  }
};

struct osl::game_playing::SpeculativeAllMoves::SearchAllMoves::Generator
{
  GameState& state;
  SearchPlayer& player;
  MoveVector tried_moves;
  volatile Status& status;
  std::mutex& mutex;
  int index, seconds;
  bool has_byoyomi;
  Generator(GameState& s, SearchPlayer& p, SearchAllMoves& parent, int sec, bool byoyomi) 
    : state(s), player(p), status(parent.status), mutex(parent.mutex), index(-1), seconds(sec),
      has_byoyomi(byoyomi)
  {
  }
  Move pickUpMove()
  {
    try
    {
      MoveWithComment result;
      {
	StatusLock lk(&status, &mutex, PREDICTION2);
	player.setRootIgnoreMoves(&tried_moves, true);
	player.setVerbose(0);
	const int sec = std::max(1, has_byoyomi ? seconds/10 : seconds/7);
	result = player.selectBestMove(state, 0, 0, sec);
	player.setRootIgnoreMoves(0, false);
	player.setVerbose(1);
      }
#ifndef NDEBUG
      if (result.move.isNormal()) {
	std::cerr << "search prediction ";
	std::cerr << csa::show(result.move);
	std::cerr << "\n";
      }
#endif
      return result.move;
    }
    catch (std::exception& e)
    {
      std::cerr << "Generator::bestMove " << e.what() << "\n";
    }
    return Move::INVALID();
  }
  const Move nextMove()
  {
    const Move move = pickUpMove();
    if (! move.isNormal()) {
      std::cerr << "finish\n";
      return Move::INVALID();
    }
    tried_moves.push_back(move);
    return move;
  }
};

osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::SearchAllMoves(ResultVector& r)
  : results(r), next_iteration_coefficient(1.0),
    current_move(Move::INVALID()), status(INITIAL), seconds(-1),
    stop_flag(false)
{
}

osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::~SearchAllMoves()
{
}

void osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::setUp(const GameState& main_state,
		      const SearchPlayer& main_player,
		      int standard_seconds, bool has_byoyomi)
{
  player.reset();
  next_iteration_coefficient = main_player.nextIterationCoefficient();
  try
  {
    player.reset(dynamic_cast<SearchPlayer*>(main_player.clone()));
    player->setVerbose(1);
    player->setNextIterationCoefficient(std::max(1.0, next_iteration_coefficient/2));
    state = main_state.clone();
    generator.reset(new Generator(*state, *player, *this, standard_seconds, has_byoyomi));
    seconds = standard_seconds;
    if (has_byoyomi)
      seconds += std::min(30, standard_seconds/2);
  }
  catch (std::exception& e)
  {
    player.reset();
    std::cerr << "setUp " << e.what() << "\n";
    throw;
  }
}

void osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::run()
{  
  StatusLock lk(&status, &mutex, &condition, RUNNING, FINISHED);
  if (! player)
    return;
  while (true) 
  {
    std::this_thread::yield(); // test whether opponent's move has arrived
    if (stop_flag)
      return;
    Move prediction;
    {
      StatusLock lk(&status, &mutex, PREDICTION1);
      prediction = generator->nextMove();
    }
    if (! prediction.isNormal())
      return;
    if (stop_flag)
      return;
    const MoveWithComment result = testMove(prediction);
    results.add(prediction, result);
    if (! stop_flag)
      results.show(std::cerr);
  }
}

const osl::search::MoveWithComment osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::testMove(Move predicted_move)
{
  StatusLock lk(&status, &mutex, SEARCH1);
  {
    std::lock_guard<Mutex> lk(mutex);
    current_move = predicted_move;
  }
  assert(state);
  state->pushMove(predicted_move);
  assert(player);
  player->pushMove(predicted_move);
  MoveWithComment result(Move::INVALID());
  std::cerr << "\nprediction (" << seconds << ") "
	    << csa::show(predicted_move) << " ";
  const time_t now = time(0);
  char ctime_buf[64];
  std::cerr << ctime_r(&now, ctime_buf);
  try
  {
    StatusLock lk(&status, &mutex, SEARCH2);
    if (seconds <= 0)
      seconds = 120;
    const milliseconds msec(seconds*1000);
    result = 
      player->searchWithSecondsForThisMove(*state, search::TimeAssigned(msec, msec*5));
  }
  catch (std::exception& e)
  {
    // TODO table full はclear?
    std::cerr << "error in speculative thread " << e.what() << "\n";
    stop_flag = true;
  }
  catch (...)
  {
    std::cerr << "error in speculative thread\n";
    stop_flag = true;
  }
  state->popMove();
  player->popMove();
  {
    std::lock_guard<Mutex> lk(mutex);
    current_move = Move::INVALID();
  }
  return result;
}

void osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::stopOtherThan(Move the_move)
{
  stop_flag = true;
  if (currentMove() != the_move)
    stopNow();
  else
  {
#ifndef NDEBUG
    std::cerr << "match " << csa::show(the_move) << "\n";
#endif
#ifndef GPSONE
    if (OslConfig::usiMode())
      OslConfig::setUsiSilent(false);
#endif
    assert(player);
    player->setNextIterationCoefficient(next_iteration_coefficient);
    player->setVerbose(2);
  }
}
      
void osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::stopNow()
{
  stop_flag = true;
  waitRunning();
  if (player && status != FINISHED)
  {
    std::cerr << "stopNow " << status << "\n";
    const bool success
      = player->stopSearchNow();
    if (! success)
      std::cerr << "stop search failed\n";
  }
}

void osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::waitRunning()
{
  std::unique_lock<Mutex> lk(mutex);
  while (status == INITIAL)
  {
    condition.wait(lk);
    if (!player)
      return;
  }
}

void osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::setTimeAssign(const search::TimeAssigned& new_assign)
{
  if (player && status != FINISHED)
  {
    waitRunning();
    player->setTimeAssign(new_assign);
  }
}
const osl::time_point osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::startTime()
{
  if (player && status != FINISHED)
  {
    waitRunning();
    return player->startTime();
  }
  return time_point();
}

const osl::Move osl::game_playing::SpeculativeAllMoves::
SearchAllMoves::currentMove() const
{
  std::lock_guard<Mutex> lk(mutex);
  return current_move;
}

/* ------------------------------------------------------------------------- */


struct osl::game_playing::SpeculativeAllMoves::Runner
{
  SpeculativeAllMoves *parent;
  Runner(SpeculativeAllMoves *p) : parent(p)
  {
  }
  void
#ifdef __GNUC__
#  ifdef _WIN32
__attribute__((noinline))
__attribute__((force_align_arg_pointer)) 
#  endif
#endif
  operator()()
  {
    parent->searcher->run();
  }
};

osl::game_playing::SpeculativeAllMoves::
SpeculativeAllMoves()
  : results(new ResultVector()), last_search_seconds(-1), has_byoyomi(false),
    allowed(true)
{
}

osl::game_playing::
SpeculativeAllMoves::~SpeculativeAllMoves()
{
  stopAll();
  selectBestMoveCleanUp();
}

void osl::game_playing::SpeculativeAllMoves::
startSpeculative(const std::shared_ptr<GameState> state,
		 const SearchPlayer& main_player)
{
  std::lock_guard<std::mutex> lk(mutex);
  if (! allowed)
    return;
  
  try
  {
    search_state = HashKey(state->state());
    results->clear();
    searcher.reset(new SearchAllMoves(*results));
    searcher->setUp(*state, main_player, last_search_seconds, has_byoyomi);
    thread.reset(new std::thread(Runner(this)));
  }
  catch (std::exception& e)
  {
    std::cerr << "startSpeculative " << e.what();
    searcher.reset();
  }
}

void osl::game_playing::SpeculativeAllMoves::
clearResource()
{
  std::lock_guard<std::mutex> lk(mutex);
  assert(! thread);
  searcher.reset();
}

void osl::game_playing::SpeculativeAllMoves::
stopOtherThan(Move the_move)
{
  std::lock_guard<std::mutex> lk(mutex);
  if (searcher)
    searcher->stopOtherThan(the_move);
}

void osl::game_playing::SpeculativeAllMoves::
stopAll()
{
  std::lock_guard<std::mutex> lk(mutex);
  if (searcher)
    searcher->stopNow();
}

const osl::search::MoveWithComment
osl::game_playing::SpeculativeAllMoves::
waitResult(Move last_move, search::TimeAssigned wait_for,
	   SearchPlayer& main_player, int byoyomi)
{
  {
    std::lock_guard<std::mutex> lk(mutex);
    if (! allowed || ! searcher)
      return MoveWithComment(Move::INVALID());
  }
  last_search_seconds = (int)ceil(toSeconds(wait_for.standard));
  has_byoyomi = (byoyomi > 0);
  
  const time_t start_time = time(0);
  const time_point start_time_msec = clock::now();
  assert(last_move.isNormal());
  const MoveWithComment *result = 0;
  bool stop_now = false;
  if (searcher->currentMove() != last_move)
  {
    stop_now = true;
    wait_for = search::TimeAssigned(milliseconds(0));
  }
  // const time_t standard_time = start_time + ceil(wait_for.standard.toSeconds());
  const time_t stop_time = start_time + static_cast<int>(ceil(toSeconds(wait_for.max)));

  const time_point started = searcher->startTime();
  const bool already_finished = searcher->isFinished();
  if (! already_finished)
  {
    char ctime_buf[64];
    std::cerr << "wait for (" << toSeconds(wait_for.standard)
	      << "/" << toSeconds(wait_for.max)
	      << ") "
	      << ctime_r(&stop_time, ctime_buf);
    const milliseconds diff = duration_cast<milliseconds>(start_time_msec - started);
    wait_for.standard = wait_for.standard + diff;
    wait_for.max = wait_for.max + diff;
    searcher->setTimeAssign(wait_for);
  }
  {
    int wait_count = 0;
    while (true)
    {
      const bool now_finished = searcher->isFinished();
      if ((result = results->find(last_move))) {
#ifndef GPSONE
	if (wait_count == 0 && OslConfig::usiMode()) {
	  search::UsiReporter::showPV(std::cout, result->root_limit/200, result->node_count, result->elapsed,
				      last_move.player() == BLACK ? -result->value : result->value,
				      result->move, &*result->moves.begin(), &*result->moves.end(),
				      true);
	}
#endif
	break;
      }
      assert(searcher);
      if (now_finished)
	return MoveWithComment(Move::INVALID());
      if (stop_now && ++wait_count > 60) {
	std::cerr << "error stop now failed for 60 times\n";
	abort();
      }
    
      if (! stop_now)
      {
	time_t now = time(0);
	stop_now = now >= stop_time;
      }
      if (stop_now) {
	searcher->stopNow();
      }
      std::unique_lock<std::mutex> lk(searcher->mutex);
      auto const timeout= std::chrono::steady_clock::now()
	+((wait_count <= 10) 
	  ? std::chrono::milliseconds(1000)
	  : std::chrono::milliseconds(2000));
      searcher->condition.wait_until(lk, timeout);
    }
  }
  if (! searcher->isFinished())
    searcher->stopNow();
  if (result->move.isNormal()) {
    SearchPlayer *player = searcher->currentPlayer();
    if (player)
      main_player.swapTable(*player);
  }
  return *result;
}
  
void osl::game_playing::SpeculativeAllMoves::
selectBestMoveCleanUp()
{
  if (! thread)
    return;
  
  {
    std::lock_guard<std::mutex> lk(mutex);
    if (searcher && ! searcher->isFinished())
      searcher->stopNow();
  }
  thread->join();
  thread.reset();
  if (searcher)
    searcher.reset();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
