/* searchRecorder.cc
 */
#include "osl/search/searchRecorder.h"
#include "osl/search/realizationProbability.h"
#include "osl/eval/evalTraits.h"
#include "osl/moveLogProb.h"
#include "osl/csa.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#ifndef _MSC_VER
#  include <unistd.h>
#endif

#ifndef MINIMAL
const char *checkmateFileName = "currentCheck.csa";
#endif

/** 以下を define しないと 詰将棋の結果を全て記録する */
#define SELECT_CHECKMATE_LOG

/** 以下を定義すると詰将棋に入った時の局面を別ファイルに保存する */
// #define CHECKMATE_SEARCHER_DEBUG

/* ------------------------------------------------------------------------- */

osl::search::CountRecorder::CountRecorder()
  : node_count(0), quiescence_count(0), checkmate_count(0)
{
}

osl::search::CountRecorder::~CountRecorder()
{
}

void osl::search::
CountRecorder::recordInvalidMoveInTable(const SimpleState& state,
					const MoveLogProb& move, int limit) const
{
  std::cerr << "?? invalid move in table " << move.move() << " " << move.logProb()
	    << " limit " << limit << "\n"
	    << state;
}

void osl::search::CountRecorder::resetNodeCount()
{
  node_count = quiescence_count = checkmate_count = 0;
}

void osl::search::
CountRecorder::finishSearch(Move /*best_move*/, double sec, bool verbose) const
{
  if (! verbose)
    return;
  reportCount(std::cerr, sec);
}

void osl::search::
CountRecorder::reportCount(std::ostream& os) const
{
  os << "#normal : " << nodeCount() << ", ";
  os << "#quiescence: " << quiescenceCount() << ", ";
  os << "#checkmate : " << checkmateCount() << "\n";
}

void osl::search::
CountRecorder::reportCount(std::ostream& os, double seconds) const
{
  const double total = nodeCount() + quiescenceCount() + checkmateCount();
  os << "#total : " << total
     << std::setprecision(10)
     << " in " << seconds << " sec., " << total/seconds << " nodes/sec."
     << std::setprecision(4)
     << " (quiesce " << 100.0*quiescenceCount()/total << "%,"
     << " checkmate " << 100.0*checkmateCount()/total << "%)\n";
}

/* ------------------------------------------------------------------------- */

#ifndef MINIMAL
static bool showAllValues = false;
struct osl::search::SearchRecorder::Recorder
{
  std::ofstream os;
  /** 現在の深さ(表示加工用) */
  int current_depth;
  /** 探索を開始した時点でのlimit */
  int initial_limit;
  int log_margin;
  
  Recorder(const char *filename) 
    : os(filename),
      current_depth(0), initial_limit(0),
      log_margin(RealizationProbability::TableMove)
  {
  }
  std::ostream& stream()
  {
    assert(os);
#if 0
    os << current_depth << ':';
#endif
    for (int i=0; i<=current_depth; ++i)
      os << '*';
    os << ' ';
    return os;
  }
  /** 変な名前だけど記録を取る深さに収まっている事を調べる */
  bool notSoDeep(int limit) const
  {
    return 
#ifdef SELECT_CHECKMATE_LOG
      (limit <= initial_limit) // SearchTable::CheckmateSpecialDepth が来ることがある
      && 
#endif
      (initial_limit - limit) <= log_margin;
  }
  void flush()
  {
#if 1
    os << std::flush;
#endif    
  }
};

osl::search::
SearchRecorder::SearchRecorder(const char *filename)
  : recorder(new Recorder(filename))
{
}

osl::search::
SearchRecorder::~SearchRecorder()
{
}

void osl::search::
SearchRecorder::setLogMargin(int margin)
{
  recorder->log_margin = margin;
}

void osl::search::
SearchRecorder::tryMove(const MoveLogProb& m, int last_f, int limit) const
{
  ++recorder->current_depth;
  if (recorder->notSoDeep(limit-100)) // 末端ではtryMove を無視
  {
    std::ostream& os = stream();
    os << "==> " << csa::show(m.move());
    os << " " << m.logProb() << "\t" 
       << "last_f: " << last_f << " limit: " << limit << "\n";
    recorder->flush();
  }
}

void osl::search::
SearchRecorder::retryMove(const MoveLogProb& m, int last_f, int limit,
			int retryCount) const
{
  ++recorder->current_depth;
  if (recorder->notSoDeep(limit)) // 再探索は必ず記録しないとややこしい
  {
    std::ostream& os = stream();
    os << "ex" << retryCount << "> " << csa::show(m.move());
    os << " " << m.logProb() << "\t" 
       << "last_f: " << last_f << " limit: " << limit << "\n";
    recorder->flush();
  }
}

void osl::search::
SearchRecorder::recordValue(const MoveLogProb& m, int val, bool betterMove, int limit) const
{
  if (recorder->notSoDeep(limit)
      && (showAllValues || betterMove))
  {
    std::ostream& os = stream();
    os << "<== " << val << "\t" << csa::show(m.move());
    os << "\n";
    recorder->flush();    
  }
  CountRecorder::recordValue(m,val,betterMove,limit);
  --recorder->current_depth;
}

static const char *lowerChar(osl::Player p) 
{
  return (p == osl::BLACK) ? "B (lb)>" : "W (lb)<";
}
static const char *higherChar(osl::Player p) 
{
  return (p == osl::BLACK) ? "B (ub)<" : "W (ub)>";
}

void osl::search::
SearchRecorder::tableHitLowerBound(Player p, int val, int last_f, int limit) const
{
  if (recorder->notSoDeep(limit))
  {
    stream() << "==| table answered " << lowerChar(p) << val
	     << " for " << p << " last_f " << last_f << "\n";
    recorder->flush();
  }
}

void osl::search::
SearchRecorder::tableHitUpperBound(Player p, int val, int last_f, int limit) const
{
  if (recorder->notSoDeep(limit))
  {
    stream() << "==| table answered " << higherChar(p) << val
	     << " for " << p << " last_f " << last_f << "\n";
    recorder->flush();
  }  
}

void osl::search::
SearchRecorder::tableStoreLowerBound(Player p, const MoveLogProb& best_move, int val, int limit) const
{
  const Move move = best_move.move();
  assert(move.isInvalid() || move.isValidOrPass()); 
  // TODO: lower bound は invalid はないはず?
  if (recorder->notSoDeep(limit-100)) // 末端は無視
  {
    std::ostream& os = stream();
    os << "|== table store " << lowerChar(p) << val << " " << csa::show(move);
    os << " limit " << limit << "\n";
    recorder->flush();
  }
}

void osl::search::
SearchRecorder::tableStoreUpperBound(Player p, const MoveLogProb& best_move, int val, int limit) const
{
  const Move move = best_move.move();
  assert(move.isInvalid() || move.isValidOrPass());
  if (recorder->notSoDeep(limit-100)) // 末端は無視
  {
    std::ostream& os = stream();
    os << "|== table store " << higherChar(p) << val << " " << csa::show(move);
    os << " limit " << limit << "\n";
    recorder->flush();
  }
}

void osl::search::
SearchRecorder::recordTopLevelLowFail(const MoveLogProb& /* best */, int last_f) const
{
  stream() << "low fail,  last_f=" << last_f << "\n";
  reportCount(stream());
}
void osl::search::
SearchRecorder::recordTopLevelHighFail(const MoveLogProb& best_move, int last_f) const
{
  stream() << "high fail, last_f=" << last_f << " " << best_move << "\n";
  reportCount(stream());
}

void osl::search::
SearchRecorder::startSearch(int limit) const
{
  stream() << "\nnew search: limit " << limit 
	   << ", log " << recorder->log_margin << "\n";
  recorder->initial_limit = limit;
  CountRecorder::startSearch(limit);
}
void osl::search::
SearchRecorder::finishSearch(Move best_move, double sec, bool verbose) const
{
  stream() << "search finished\t" << best_move << "\n";
  CountRecorder::finishSearch(best_move, sec, verbose);
}

void osl::search::
SearchRecorder::gotoCheckmateSearch(const SimpleState& 
#ifdef CHECKMATE_SEARCHER_DEBUG
				    state
#endif
				    , int 
#ifdef CHECKMATE_SEARCHER_DEBUG
				    nodeLimit
#endif
  ) const
{
#ifdef CHECKMATE_SEARCHER_DEBUG
  std::ofstream os(checkmateFileName, std::ios::app);
  os << state;
  os << nodeLimit << "\n";
#endif
}

void osl::search::
SearchRecorder::backFromCheckmateSearch() const
{
#ifdef CHECKMATE_SEARCHER_DEBUG
  std::ofstream os(checkmateFileName, std::ios::app);
  os << "done\n";
#endif
}

std::ostream& osl::search::
SearchRecorder::stream() const
{
  return recorder->stream();
}
#endif

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
