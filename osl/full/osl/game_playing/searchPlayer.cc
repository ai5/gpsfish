/* searchPlayer.cc
 */
#include "osl/game_playing/searchPlayer.h"
#include "osl/game_playing/searchPlayer.tcc"
#include "osl/game_playing/historyToTable.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/pvHistory.h"
#include "osl/search/searchRecorder.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/timeControl.h"
#include "osl/search/searchTimer.h"
#include "osl/search/fixedEval.h"
#include "osl/search/bigramKillerMove.h"
#include "osl/enterKing.h"
#include "osl/progress/effect5x3.h"
#include "osl/container/moveStack.h"
#include "osl/hash/hashRandom.h"
#include <iostream>
#include <ctime>
#include <cmath>

osl::game_playing::SearchPlayer::
Config::Config()
  : limit(1200), node_limit(800000), table_size(10000),
    table_record_limit(0), initial_limit(600), deepening_step(200),
    verbose(0), next_iteration_coefficient(3), draw_coef(0),
    save_pv(true), node_count_hard_limit(std::numeric_limits<uint64_t>::max()),
    multi_pv_width(0)
{
}

bool osl::game_playing::operator==(const SearchPlayer::Config& l, 
				   const SearchPlayer::Config& r)
{
  return (l.limit == r.limit) && (l.node_limit == r.node_limit)
    && (l.table_size == r.table_size) 
    && (l.table_record_limit == r.table_record_limit)
    && (l.initial_limit == r.initial_limit)
    && (l.deepening_step == r.deepening_step)
    && (l.verbose == r.verbose)
    && (l.next_iteration_coefficient == r.next_iteration_coefficient)
    && (l.draw_coef == r.draw_coef);
}

osl::game_playing::
SearchPlayer::SearchPlayer()
  : recorder_ptr(new CountRecorder()),
    searching(0),
    plan_stop(false), root_ignore_moves(0), 
    prediction_for_speculative_search(0), 
    pv_history(new PVHistory)
{
}

osl::game_playing::
SearchPlayer::SearchPlayer(const SearchPlayer& copy)
  : ComputerPlayer(copy), ComputerPlayerSelectBestMoveInTime(copy),
    config(copy.config),
    recorder_ptr(new CountRecorder()),
    searching(0), plan_stop(false),
    root_ignore_moves(0), prediction_for_speculative_search(0),
    pv_history(new PVHistory(*copy.pv_history)),
    almost_resign_count(0)
{
}

osl::game_playing::
SearchPlayer::~SearchPlayer()
{
}

void osl::game_playing::
SearchPlayer::swapTable(SearchPlayer& other)
{
  table_ptr.swap(other.table_ptr);
}

bool osl::game_playing::
SearchPlayer::canStopSearch()
{
  return searching;
}

bool osl::game_playing::
SearchPlayer::stopSearchNow()
{
  plan_stop = true;
  if (! searching)
  {
    std::cerr << "SearchPlayer not searching ";
    const time_t now = time(0);
    char ctime_buf[64];
    std::cerr << ctime_r(&now, ctime_buf);
    return false;
  }
  searcher->stopNow();
  return true;
}

void osl::game_playing::
SearchPlayer::resetRecorder(search::CountRecorder *new_recorder)
{
  recorder_ptr.reset(new_recorder);
}

const osl::search::TimeAssigned osl::game_playing::
SearchPlayer::adjust(const search::TimeAssigned& org, const milliseconds& consumed)
{
  if (consumed < org.standard/8)
    return search::TimeAssigned(org.standard - consumed, org.max - consumed);
  // spent too much seconds in preparation
  search::SearchTimer::adjustMemoryUseLimit(0.9);
    return search::TimeAssigned(std::min(org.standard - org.standard/8, org.max - consumed),
				org.max - consumed);
}

const osl::milliseconds osl::game_playing::
SearchPlayer::setUpTable(const GameState& gs, int pawn_value)
{
  const time_point started = clock::now();
  time_t t = time(0);
  char ctime_buf[64];
  if (table_ptr.get() && table_ptr->verboseLevel() > 2)
  {
    std::cerr << "setUpTable " << ctime_r(&t, ctime_buf)
	      << std::flush;
  }
  
  // release cherkmate_ptr first
  checkmate_ptr.reset();

  const int black_win = search::FixedEval::winByLoop(BLACK);
  const int white_win = search::FixedEval::winByLoop(WHITE);
  if (table_ptr.get()) {
    table_ptr->clear();
  }
  else {
    try 
    {
      table_ptr.reset(new SimpleHashTable(config.table_size, 
					  config.table_record_limit, config.verbose));
    }
    catch (std::bad_alloc&)
    {
      std::cerr << "\atable allocation failed, try agaian" << std::endl;
      table_ptr.reset(new SimpleHashTable(config.table_size, 
					  config.table_record_limit, config.verbose));
    }
  }
  table_ptr->setVerbose(config.verbose);

  HistoryToTable::adjustTable(gs, *table_ptr, black_win, config.draw_coef*pawn_value, white_win);
  if (config.save_pv)
    HistoryToTable::setPV(*pv_history, gs, *table_ptr);
  try 
  {
    checkmate_ptr.reset(new DualDfpn());
  }
  catch (std::bad_alloc&)
  {
    std::cerr << "\acheckmate allocation failed, try agaian" << std::endl;
    checkmate_ptr.reset(new DualDfpn());
  }
  checkmate_ptr->writeRootHistory(gs.counter(), gs.moveHistory(),
				  gs.state(), gs.state().turn());

  if (table_ptr->verboseLevel() > 2)
  {
    t = time(0);
    std::cerr << "setup done in " << toSeconds(clock::now() - started)
	      << " sec.  " << ctime_r(&t, ctime_buf) 
	      << std::flush;
  }
#ifndef MINIMAL
  if (OslConfig::evalRandom())
    HashRandom::setUp(1.0*OslConfig::evalRandom()*pawn_value/100);
#endif
  return duration_cast<milliseconds>(clock::now() - started);
}

void osl::game_playing::
SearchPlayer::setDepthLimit(int l, int il, int ds)
{
  config.limit = l;
  config.initial_limit = il;
  config.deepening_step = ds;
}

void osl::game_playing::
SearchPlayer::setNodeLimit(size_t nl)
{
  config.node_limit = nl;
}

void osl::game_playing::
SearchPlayer::setNodeCountHardLimit(size_t nl)
{
  config.node_count_hard_limit = nl;
}

void osl::game_playing::
SearchPlayer::setTableLimit(size_t size, int record_limit)
{
  config.table_size = size;
  config.table_record_limit = record_limit;

  table_ptr.reset();
}

void osl::game_playing::
SearchPlayer::setVerbose(int v)
{
  config.verbose = v;
  if (table_ptr)
    table_ptr->setVerbose(v);
}

void osl::game_playing::
SearchPlayer::setNextIterationCoefficient(double new_value)
{
  config.next_iteration_coefficient = new_value;
  if (searcher)
    searcher->setNextIterationCoefficient(new_value);
}

void osl::game_playing::
SearchPlayer::addMonitor(const std::shared_ptr<search::SearchMonitor>& m)
{
  config.monitors.push_back(m);
}


void osl::game_playing::
SearchPlayer::pushMove(Move /*move*/)
{
  checkmate_ptr.reset();
  if (speculative_search_allowed)
    table_ptr.reset();
}
void osl::game_playing::
SearchPlayer::popMove()
{
  checkmate_ptr.reset();
  if (speculative_search_allowed)
    table_ptr.reset();
}

int osl::game_playing::
SearchPlayer::secondsForThisMove(const GameState& state, int time_limit, int elapsed,
				 int byoyomi) const
{
  return secondsForThisMove(state, time_limit, elapsed, byoyomi,
			    table_ptr.get() ? table_ptr->verboseLevel() : 0);
}

int osl::game_playing::
SearchPlayer::secondsForThisMove(const GameState& state, int time_limit, int elapsed,
				 int byoyomi, int verboseness)
{
  if (byoyomi < 0)
    return -1; // 無限

  if (time_limit - elapsed < byoyomi)
    return byoyomi;
  const int time_limit_org = time_limit;
  const int moves = state.moveHistory().size();
  if (byoyomi == 0)
  {
    // 330手指せるようにする
    // http://www32.ocn.ne.jp/~yss/csa14rep.html
    // 90秒から1秒将棋に入るので240で良い (330-90)
    time_limit -= std::max(0, (240 - moves));
  }
  const int time_left = time_limit - elapsed;
  if (time_left <= 1)
    return 1;
  int seconds_for_this_move 
    = search::TimeControl::secondsForThisMove(time_left);

  // Think more if book leads us to near endgame
  const progress::Effect5x3 progress(state.state());
  if (time_left >= 600*time_limit_org/1500)
  {
    if ((progress.progress16().value() >= 15)
	&& ((progress.progress16(BLACK).value() >= 13)
	    || (progress.progress16(WHITE).value() >= 13))) {
      if (verboseness)
	std::cerr << "  time control endgame ext\n";
      return seconds_for_this_move*2;
    }
  }
  if (byoyomi == 0 || time_left >= byoyomi*60) 
  {
    // do not think too much in opening
    if (progress.progress16().value() == 0) {
      if (verboseness)
	std::cerr << "  time control progress0 limit " << 25*time_limit_org/1500 << "\n";
      return std::min(std::max(1, 25*time_limit_org/1500), seconds_for_this_move);
    }
    if (progress.progress16().value() <= 3 && moves <= 40) {
      if (verboseness)
	std::cerr << "  time control progress4 limit " << 38*time_limit_org/1500 << "\n";
      return std::min(std::max(1, 38*time_limit_org/1500), seconds_for_this_move);
    }
  }
  // others
  seconds_for_this_move += byoyomi;
  if (byoyomi >= 10 && time_left >= byoyomi*2)
    seconds_for_this_move += byoyomi/2;
  return seconds_for_this_move;
}

const osl::search::TimeAssigned osl::game_playing::
SearchPlayer::assignTime(const GameState& state, int limit, int elapsed,
			 int byoyomi) const
{
  return assignTime(state, limit, elapsed, byoyomi, 
		    table_ptr.get() ? table_ptr->verboseLevel() : 0);
}

const osl::search::TimeAssigned osl::game_playing::
SearchPlayer::assignTime(const GameState& state, int limit, int elapsed,
			 int byoyomi, int verboseness)
{
  const int seconds_for_this_move
    = secondsForThisMove(state, limit, elapsed, byoyomi, verboseness);
  const int seconds_max = (byoyomi && (limit - elapsed) < byoyomi)
    ? seconds_for_this_move
    : std::min(seconds_for_this_move*5, std::max(seconds_for_this_move, (limit-elapsed)/2));
  return search::TimeAssigned(milliseconds(seconds_for_this_move*1000),
			      milliseconds(seconds_max*1000));
}

const osl::search::MoveWithComment osl::game_playing::
SearchPlayer::selectBestMove(const GameState& state, int limit, int elapsed,
			     int byoyomi)
{
  return selectBestMoveInTime(state, assignTime(state, limit, elapsed, byoyomi));
}

const osl::search::MoveWithComment osl::game_playing::
SearchPlayer::selectBestMoveInTime(const GameState& state, const search::TimeAssigned& msec)
{
  if (EnterKing::canDeclareWin(state.state()))
    return MoveWithComment(Move::DeclareWin());
  if (msec.standard == msec.max
      && config.next_iteration_coefficient > 1.0)
    setNextIterationCoefficient(1.0);
  return searchWithSecondsForThisMove(state, msec);
}

void osl::game_playing::
SearchPlayer::saveSearchResult(const GameState& state, const MoveWithComment& best_move)
{
  (*pv_history)[state.moveHistory().size() % pv_history->size()] = best_move;
}

void osl::game_playing::
SearchPlayer::setTimeAssign(const search::TimeAssigned& new_assign)
{
  if (searcher)
  {
    searcher->setTimeAssign(new_assign);
  }
}
const osl::time_point osl::game_playing::
SearchPlayer::startTime() const
{
  if (searcher)
  {
    return searcher->startTime();
  }
  return time_point();
}

#ifdef USE_NTESUKI
osl::game_playing::SearchPlayer::
NtesukiThread::NtesukiThread(Move& next_move,
	      volatile bool *thread_finished,
	      volatile bool *stop_flag,
	      NumEffectState state)
  : next_move(next_move), thread_finished(thread_finished),
    stop_flag(stop_flag), state(state)
{
}

void osl::game_playing::SearchPlayer::
NtesukiThread::operator()()
{
  std::cerr << "start ntesuki search\n";
  *thread_finished = false;

  const Player P = state.turn();
  const HashKey key = osl::HashKey::calcHash(state);;

  std::unique_ptr<osl::ntesuki::NtesukiAttackMoveGenerator>
    gam(new osl::ntesuki::GetAttackMoves());
  std::unique_ptr<osl::ntesuki::NtesukiDefenseMoveGenerator>
    gdm(new osl::ntesuki::GetDefenseMoves());

  osl::ntesuki::NtesukiSearcher
    searcher(state, gam.get(), gdm.get(), 500000u, stop_flag, true, 2);
   
  try
  {
    int ntesuki_num = searcher.searchSlow(P, 10000000);
    if (-1 != ntesuki_num)
    {
      const osl::ntesuki::PdNtesukiTable& table
	= searcher.getTableSlow(P);
      const osl::ntesuki::PdNtesukiRecord *record
	= table.find(key);
      next_move = record->getBestMove(ntesuki_num).move();
    }
  }
  catch (ntesuki::ReadNodeLimit& e)
  {
  }
  catch (ntesuki::TableFull& e)
  {
  }
  catch (std::runtime_error& e)
  {
    std::cerr << e.what() << "\n";
  }
  std::cerr << "end ntesuki search\n";
  *thread_finished = true;
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
