/* searchNode.h
 */
#ifndef GPSSHOGI_SEARCHNODE_H
#define GPSSHOGI_SEARCHNODE_H

#include "interimReport.h"
#include "osl/hashKey.h"
#include "osl/state/historyState.h"
#include "osl/misc/milliSeconds.h"
#include <memory>
#include <functional>
#include <map>
#include <set>
#include <vector>

namespace gpsshogi
{
  typedef std::set<int> slave_set_t;
  class UsiSlave;
  typedef std::shared_ptr<UsiSlave> UsiSlavePtr;
  struct SearchNode;
  typedef std::shared_ptr<SearchNode> SearchNodePtr;
  typedef std::map<std::string, SearchNodePtr> successor_table_t;
  struct ProbeData
  {
    std::vector<std::string> target;
    std::set<int> probing;
    std::vector<UsiSlavePtr> split_waiting;
    osl::time_point start;
    successor_table_t probed;
    int onGoingCount() const
    {
      return probing.size() + split_waiting.size(); 
    }
    bool onGoing() const { return onGoingCount() > 0; }
    void clear()
    {
      target.clear();
      probing.clear();
      split_waiting.clear();
      probed.clear();
    }
    void addFinish(const UsiSlavePtr& slave, const InterimReport& report);
    void removeTarget(const std::string& move) 
    {
      target.erase(std::remove(target.begin(), target.end(), move),
		   target.end());
    }
    const std::string status() const;
  };
  struct LeafData
  {
    osl::CArray<UsiSlavePtr,2> working; // 0: search, 1: checkmate search
    std::vector<std::string> pv_hint;
  };
  struct MoveData
  {
    std::vector<osl::Move> all;
    std::vector<std::string> usi, normal;
    std::set<std::string> win, draw, loss, nopromote;
    bool hasDraw() const { return !draw.empty(); }
    bool empty() const { return all.empty(); }
  };
  /**
   * status
   * - vanilla
   * - moves_generated
   * - vacant (searched some periods but no workers now) 
   * - solved
   * - probing
   * - working
   * - working + probing
   */
  struct SearchNode : public std::enable_shared_from_this<SearchNode>
  {
    const std::string position;
    std::string ignore_moves;
    const osl::HashKey key;
    SearchNode *parent;
    std::string parent_move;
    successor_table_t succ;
    MoveData moves;
    LeafData leaf;
    InterimReport main, /** e.g., for probe */ sub, solved;
    /** 1s probe */
    ProbeData probe;
    /** this node (once) has permanent (not probing) worker(s) */
    std::vector<std::vector<std::string> > expanded_other_pv;
    osl::time_point split_time, last_tree_change;
    int scheduled_slave;
    /** adjustment of evaluation */
    int heuristic_cost;
    bool has_workers, mate_tested;
    bool committed;
    int take_back_moves;

    SearchNode(const std::string& po, const osl::HashKey& k,
	       SearchNode *pa, const std::string& pm, const std::string& i="");
    ~SearchNode();

    SearchNodePtr successor(const std::string& move);
    SearchNodePtr successorOther();

    int id() const { return key.signature() + ignore_moves.size(); }
    bool isLeaf() const { return succ.empty(); }
    bool hasMove(const std::string& move) const { 
      return succ.count(move) && succ.find(move)->second;
    }
    bool hasOther() const { return hasMove("other"); }
    osl::Player turn() const { return key.turn(); }
    SearchNode *findLeaf(const std::string& position);
    SearchNode *find(const std::string& position);

    bool hasScheduledSlave() const { return scheduled_slave>=0; }
    int scheduledSlave() const { return scheduled_slave; }
    void setScheduledSlave(int slave) { scheduled_slave=slave; }
    void resetScheduledSlave() { scheduled_slave=-1; }

    /** @return progress reported */
    bool update(std::function<void(std::string)> progress);
    bool update(const std::string& move, const InterimReport& child,
		std::function<void(std::string)> progress);
    static std::string append(const std::string& position,
			      std::string move);
    bool isSolved() const { return solved.node_count > 0; }
    const InterimReport& report() const
    {
      if (isSolved())
	return solved;
      return (main.node_count >= sub.node_count) ? main : sub;
    }
    /** base depth where the corresponding pv was obtained */
    int pvDepth() const { return report().pv.depth; }
    /** base depth at leaf or height in internal tree */
    int depthHead() const { return report().depth_head; }
    uint64_t nodeCount() const { return report().node_count; }
    uint64_t nodeCountBestMove() const { return nodeCountForMove(bestMove()); }
    uint64_t nodeCountForMove(const std::string& move) const;
    int value() const { return report().pv.score; }
    int sign() const { return osl::sign(turn()); }
    int relativeValue() const { return sign()*value(); }
    const PVInfo& pv() const { return report().pv; }
    const pv_table& alternatives() const { return report().alternatives; }
    std::string bestMove() const { return pv().empty() ? "" : pv()[0]; }
    bool bestMoveIsOther() const 
    {
      return !isLeaf() && (succ.count(bestMove()) == 0) && (bestMove() != ""); 
    }
    std::vector<std::string> sortByProbe(std::vector<int> *evals=0) const;
    std::vector<std::string> sortBySearchValue(std::vector<int> *evals=0) const
    {
      return sortByTable(succ, evals);
    }
    uint64_t recalculateNodeCount() const;

    void showStatus(std::ostream&, osl::HistoryState&,
		    osl::time_point now, int indent=0);
    /** @return a > b w.r.t. turn() */
    bool betterThan(int a, int b) const;
    int infty() const;
    int winValue(int mate_length=100) const;
    int isWinValue(int value) const { return !betterThan(winValue(), value); }
    static osl::Move parseMove(const osl::HistoryState& state,
			       std::string usi_move);
    static std::string toCSA(osl::HistoryState& state, std::string usi_moves_string, int limit=-1);
    std::string toCSA(const std::string& move) const;

    void prepareProbe(slave_set_t& stopped);
    void prepareSplit();
    osl::Move findMove(const std::string& move) const;
    void stopSubTree(slave_set_t& stopped, slave_set_t& restart_cancelled,
		     slave_set_t& waited);
    void generateMoves();

    bool needProbe() const { return split_time == osl::time_point(); }
    std::string path(const std::string& sofar="") const;
    void setWorking(bool add=true) { has_workers = add; }
    bool hasLeafWorker() const { return leaf.working[0] || probe.onGoing(); }
    bool hasWorker() const { return hasLeafWorker() || has_workers; }

    bool hasNormalPV() const 
    {
      return !pv().empty() && findMove(pv()[0]).isNormal(); 
    }
    std::string pathId() const 
    {
      return parent_move == "other" ? (position + " other") : position;
    }
    std::string ignoreMoves() const;
    void searchInLeaf(std::function<void(InterimReport)> progress,
		      std::function<void(InterimReport)> finish,
		      int msec, UsiSlavePtr slave, 
		      slave_set_t& stopped_slave, int multi_pv);
    void runMate(std::function<void(std::string)> finish,
		 int msec, UsiSlavePtr slave);
    static void setDrawValue(int new_value) { draw_value = new_value; }
    int depth8(int cur=0) const;
    osl::time_point lastTreeChange() const {
      return last_tree_change;
    }
    osl::time_point lastTreeChangeForBestMove() const {
      const std::string best_move = bestMove();
      if (isLeaf() || bestMoveIsOther() || !succ.count(best_move)
	  || ! succ.find(best_move)->second)
	return last_tree_change;
      return succ.find(best_move)->second->lastTreeChange();
    }
    bool hasAlternative(const std::string& move) const
    {
      const InterimReport& info = report();
      return info.alternatives.count(move);
    }
    const PVInfo findAlternative(const std::string& move, int allow_behind=1) const
    {
      const InterimReport& info = report();
      pv_table::const_iterator p = info.alternatives.find(move);
      if (p == info.alternatives.end())
	return PVInfo();
      if (p->second.depth+allow_behind < depthHead())
	return PVInfo();
      return p->second;
    }
    bool hasAlternativeInOther(const std::string& move) const
    {
      return hasOther()
	&& succ.find("other")->second->hasAlternative(move);
    }
    const PVInfo findAlternativeInOther(const std::string& move) const
    {
      successor_table_t::const_iterator p = succ.find("other");
      if (p == succ.end())
	return PVInfo();
      const SearchNodePtr& node = p->second;
      if (! node)
	return PVInfo();
      return node->findAlternative(move);
    }
    bool isPrimaryTakeBack(const std::string&) const;
    bool isTakeBack(const std::string&) const;
  private:
    void propagateTreeChangeTime();
    SearchNode *find(const std::vector<std::string>& moves, size_t cur);
    void stopProbe(slave_set_t& waited);
    std::vector<std::string> sortByTable(const successor_table_t& table, std::vector<int> *evals=0) const;
    static int draw_value;
  };
}

#endif /* GPSSHOGI_SEARCHNODE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
