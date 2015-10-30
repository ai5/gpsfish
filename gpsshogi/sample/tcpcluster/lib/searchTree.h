/* searchTree.h
 */
#ifndef GPSSHOGI_SEARCHTREE_H
#define GPSSHOGI_SEARCHTREE_H

#include "interimReport.h"
#include "timeCondition.h"
#include "searchNode.h"
#include "osl/game_playing/usiState.h"
#include "osl/game_playing/gameState.h"
#include "osl/misc/milliSeconds.h"
#include <boost/asio/io_service.hpp>
#include <functional>
#include <map>
#include <set>
#include <deque>

namespace gpsshogi
{
  class UsiSlave;
  typedef std::shared_ptr<UsiSlave> UsiSlavePtr;
  struct SearchStatus
  {
    int position_id, msec_standard, msec_max;
    std::string position;
    bool stopped, finished, early_win_needed, in_check;
    osl::time_point started, info;
    std::string info_string;
    std::vector<osl::time_point> restarted;
    TimeCondition config;
    uint64_t node_count, node_count_best;
    int value, last_status;
    std::string sufficient_ponder;
    bool forced_move_or_similar;
    SearchStatus() : position_id(0), msec_standard(0), msec_max(0),
		     stopped(0), finished(0), early_win_needed(0),
		     in_check(0), started(osl::clock::now()), 
		     node_count(0), node_count_best(0), 
		     value(0), last_status(0), forced_move_or_similar(false)
    {
    }
    osl::milliseconds elapsedBeforeLastRestart() const
    {
      return osl::duration_cast<osl::milliseconds>
	((restarted.empty() ? started : restarted.back()) - started);
    }
  };
  struct SearchStatistics
  {
    std::vector<int> values;
    int selected_other, extended, prepared;
    SearchStatistics() : selected_other(0), extended(0), prepared(0)
    {
    }
  };
  class SearchTree
  {
    boost::asio::io_service& usi_queue;
    osl::UsiState state;
    std::shared_ptr<SearchNode> root;
    std::vector<SearchStatus> history;
    std::function<void(std::string)> progress;
    std::function<void(std::string)> finish;
    /** slaves that are just stopped where their node-finish or mateResult will be ignored */
    slave_set_t stopped_slave;
    /** slaves waiting for next split */
    slave_set_t scheduled_slave; 
    std::map<int,SearchNodePtr> restart_other_waiting;
    /** slaves ready for tree growth */
    std::set<UsiSlavePtr> current_idle; 
    std::set<UsiSlavePtr> mate_idle;
    /** slaves under control, removed in case of disconnect */
    slave_set_t all_slave;
    int split_width;
    bool ponder;
    enum Task
    {
      WaitSlave,
      Think,
      GrowTree,
      ExpandOther,
    };
    std::deque<Task> task_queue;
    enum Status
    {
      TreeIdle, 
      Thinking,
      /** using panic time (reason: other move has the highest score) */
      PanicTimeOther,
      /** just after output bestmove (before any moveRoot) */
      Pondering0,
      /** after one (possibly two when waiting go) moveRoot */
      Pondering1,
    } status;
    SearchStatistics statistics;
  public:
    explicit SearchTree(boost::asio::io_service& queue);
    ~SearchTree();

    /** @return stop handler */
    std::function<void(void)> 
    go(int id, const std::string& position, 
       int active, const std::vector<UsiSlavePtr>& slaves, 
       std::function<void(std::string)> progress,
       std::function<void(std::string)> finish,
       const TimeCondition& seconds,
       const std::vector<UsiSlavePtr>& mate_solvers);
    void moveRoot(const std::string& move, 
		  const std::vector<UsiSlavePtr>& slaves);
    
    // run in usi_queue 
    void nodeProgress(UsiSlavePtr slave, std::string position, InterimReport);
    void nodeFinish(UsiSlavePtr slave, std::string position, InterimReport); 
    void probeProgress(UsiSlavePtr, std::string position, InterimReport);
    void probeFinish(UsiSlavePtr, std::string position, InterimReport);
    void stopTree();

    void showStatus(std::ostream&);
    /** called periodically by timer in Coordinator */
    void finishIfStable(int position_id);
    void periodic();
    void gameFinished();

    static void setDrawValueCP(int new_value) { draw_value_cp = new_value; }
    static void setHumanRounding(bool enable);
    static int humanRounding() { return human_rounding; }
  private:
    SearchStatus& searching() { return history.back(); }
    const SearchStatus& searching() const { return history.back(); }
    const SearchStatus& prev() const { return history[history.size()-2]; }
    void goNode(const UsiSlavePtr& slave, SearchNode& child, int millisec);
    void goProbe(const UsiSlavePtr&, SearchNode&);
    void flatSplit(SearchNode& node, const std::vector<std::string>& moves,
		   const std::vector<UsiSlavePtr>& slaves);
    void probeAndSplit(SearchNode& node, std::vector<UsiSlavePtr> slaves);
    void splitTree(SearchNode& node);
    void internalFinish(int id);
    void ponderProgress(std::string);
    bool wasStopped(int slave) const
    {
      return stopped_slave.count(slave);
    }
    void addReady(UsiSlavePtr);
    void growTree(bool in_thinking=false);
    void growTree(SearchNode& node, std::vector<UsiSlavePtr> slaves);
    void growTreeOther(SearchNode& node, const std::string& move,
		       std::vector<UsiSlavePtr> slaves);
    void startThinking();
    void finishIfStable();
    void expandOther();

    osl::time_point started() const { return searching().started; }
    int msec() const { return searching().msec_standard; }
    int msecMax() const { return searching().msec_max; }
    int msecRestart() const 
    { 
      return msec() + searching().elapsedBeforeLastRestart().count();
    }
    static const std::string uptimeString();
    std::vector<UsiSlavePtr> idleSlaves();
    std::vector<UsiSlavePtr> idleMateSolvers()
    { 
      return selectIdle(mate_idle);
    }
    std::vector<UsiSlavePtr> selectIdle(std::set<UsiSlavePtr>&);
    void restartOther(SearchNodePtr node);
    void scheduleOther(SearchNodePtr node, UsiSlavePtr slave);
    void divideSlaves(const std::vector<UsiSlavePtr>& all,
		      std::vector<UsiSlavePtr>& out0,
		      std::vector<UsiSlavePtr>& out1);
    int splitWidth(int depth) const;
    void adjustSplitWidth(int active, int msec);
    static std::string fancyString(const SearchNode& node, const std::vector<std::string>& moves, size_t size_limit,
				   const std::vector<int> *evals=0);
    void tryExpandOther(double elapsed);
    void stopSubTree(SearchNode& node);
    void setUpHeuristic(SearchNode& node);
    void logStatus(const std::string& filename="");
    void assignMate();
    bool assignMate(SearchNode& node, UsiSlavePtr solver);
    void healthTest(SearchNode& node, const osl::time_point &now);
    void mateResult(UsiSlavePtr slave, std::string node_id, std::string msg);
    uint64_t recentNodeCountBest() const;
    void showRootInfo();
    std::pair<int,int> assignTime(const TimeCondition&,
				  const osl::game_playing::GameState&) const;
    std::pair<int,int> assignTimeOSL
    (const TimeCondition&, const osl::game_playing::GameState&) const;
    static int draw_value_cp, human_rounding;
  };
}

#endif /* GPSSHOGI_SEARCHTREE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
