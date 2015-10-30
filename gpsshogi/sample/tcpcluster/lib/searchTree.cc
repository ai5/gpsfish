/* searchTree.cc
 */
#include "searchTree.h"
#include "searchNode.h"
#include "inaniwa.h"
#include "usiSlave.h"
#include "slaveManager.h"
#include "interimReport.h"
#include "logging.h"
#include "osl/usi.h"
#include "osl/csa.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/searchPlayer.h"
#include "osl/sennichite.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <limits>
using boost::algorithm::join;

const int msec_max = 24*60*60*1000; // 1 day
const int SplitWidthDefault = 2;
static osl::time_point last_report;
int gpsshogi::SearchTree::human_rounding = 0; // only effective for human v.s. computer
int gpsshogi::SearchTree::draw_value_cp = 0;
const int rounding_msec = 300; // 1000 - rtt - margin

int grow_tree_margin(size_t slaves) 
{
  return 100+100*sqrt(slaves);
}

gpsshogi::SearchTree::
SearchTree(boost::asio::io_service& queue)
  : usi_queue(queue), split_width(SplitWidthDefault),
    ponder(false), status(TreeIdle)
{
  history.reserve(1024);
  history.push_back(SearchStatus());
  history.push_back(SearchStatus());
  searching().position_id = -1;
  searching().finished = true;
}

gpsshogi::SearchTree::
~SearchTree()
{
  if (root)
    stopTree();
  task_queue.clear();
  if (! stopped_slave.empty()) {
    std::string msg = "*waiting some slave at gameFinished";
    for (int pid: stopped_slave)
      msg += " " + to_s(pid);
    Logging::error(msg);
  }
}

void gpsshogi::SearchTree::
stopSubTree(SearchNode& node) 
{
  slave_set_t restart_cancelled, waited;
  node.stopSubTree(stopped_slave, restart_cancelled, waited);
  for (int slave: waited)
    scheduled_slave.erase(slave);

  std::string msg = "";
  for (int slave: waited)
    msg += " "+to_s(slave);
  if (msg != "")
    Logging::info("*collected waiting"+msg);

  msg = "";
  for (int slave: restart_cancelled) {
    msg += " "+to_s(slave);
    restart_other_waiting.erase(slave);
  }
  if (msg != "")
    Logging::info("*restart cancelled"+msg);
}

void gpsshogi::SearchTree::
gameFinished()
{
  Logging::notice("*game end: other/extended/prepared/search "+to_s(statistics.selected_other)
		  + "/" + to_s(statistics.extended)
		  + "/" + to_s(statistics.prepared)
		  + "/" + to_s(statistics.values.size()));
  this->ponder = false;
  status = TreeIdle;
  if (root)
    stopSubTree(*root);
  task_queue.clear();
  if (! stopped_slave.empty()) {
    std::string msg = "*waiting some slave at gameFinished";
    for (int pid: stopped_slave)
      msg += " " + to_s(pid);
    Logging::warn(msg);
  }
}

void gpsshogi::SearchTree::
showRootInfo()
{
  if (osl::elapsedSeconds(searching().info) > probe_msec*5.0/1000) {
    std::string info = root->report().composeInfo(root->key.isPlayerOfTurn(osl::WHITE));
    if (info != searching().info_string) {
      searching().info = osl::clock::now();
      progress(info);
      searching().info_string = info;
    }
  }
}

void gpsshogi::SearchTree::
nodeProgress(UsiSlavePtr slave, std::string node_id, InterimReport report)
{
  if (wasStopped(slave->id()))
    return;
  SearchNode *node = root ? root->findLeaf(node_id) : 0;
  if (! node) {
    Logging::error("*node not found in nodeProgress " + node_id
		   + slave->idMark());
    return;
  }
  node->main.set(node->turn(), report);
  node->main.pv.score += node->heuristic_cost; // adjust only at leaf
  
  if (node->update(progress)) {
    searching().info = osl::clock::now();
    if (osl::elapsedSeconds(searching().started)*5000 > msec()
	&& osl::elapsedSeconds(last_report) > probe_msec*5.0/1000) {
      last_report = searching().info;
      logStatus();
    }
  }
  else {
    showRootInfo();
  }
}

void gpsshogi::SearchTree::
nodeFinish(UsiSlavePtr slave, std::string node_id, InterimReport report) 
{
  if (slave->error()) {
    Logging::notice("*ignored finish by disconnected slave " + node_id
		    + slave->idMark());
    all_slave.erase(slave->id());
    stopped_slave.erase(slave->id());
    restart_other_waiting.erase(slave->id());
    SearchNode *node = root ? root->findLeaf(node_id) : 0;
    if (node)
      node->leaf.working[0].reset();
    return;
  }
  if (wasStopped(slave->id())) {
    Logging::info("*stopped nodeFinish " + slave->idMark());
    addReady(slave);
    return;
  }
  SearchNode *node = root ? root->findLeaf(node_id) : 0;
  if (! node) {
    Logging::error("*node not found in nodeFinish " + node_id);
  } else {
    node->leaf.working[0].reset();
    if (report.finishedNormally()) {
      node->solved = node->main;
      node->update(progress);
    }
  }
  current_idle.insert(slave);
}

void gpsshogi::SearchTree::
probeProgress(UsiSlavePtr slave, std::string node_id, InterimReport report)
{
  if (wasStopped(slave->id()))
    return;
  SearchNode *node = root ? root->findLeaf(node_id) : 0;
  if (! node) {
    Logging::error("*node not found in probeProgress " + node_id
		   + slave->idMark());
    return;
  }
  node->main.set(node->turn(), report);
  if (node->update(progress)
      && osl::elapsedSeconds(last_report) > probe_msec*5.0/1000) {
    logStatus();
    last_report = osl::clock::now();
  }
}

void gpsshogi::SearchTree::
probeFinish(UsiSlavePtr slave, std::string node_id, InterimReport report) 
{
  SearchNode *node = root ? root->findLeaf(node_id) : 0;
  if (slave->error()) {
    Logging::error("*ignored probe by disconnected slave " + node_id
		   + slave->idMark());
    all_slave.erase(slave->id());
    stopped_slave.erase(slave->id());
    if (node) {
      node->leaf.working[0].reset();
      if (node && node->parent)
	node->parent->probe.probing.erase(slave->id());
    }
    return;
  }
  if (wasStopped(slave->id())) {
    Logging::info("*stopped probeFinish " + slave->idMark());
    if (node) {
      node->leaf.working[0].reset();
      if (node && node->parent)
	node->parent->probe.probing.erase(slave->id());
    }
    addReady(slave);
    return;
  }
  if (! node || ! node->parent) {
    Logging::error("*node not found in probeFinish " + node_id);
    return;
  }
  node->leaf.working[0].reset();
  SearchNode *parent = node->parent;
  assert(parent);
  parent->probe.addFinish(slave, report);
  scheduled_slave.insert(slave->id());
  if (! parent->probe.probing.empty() || searching().stopped) {
    if (searching().stopped) 
      Logging::info("*split cancelled by global stop");
    return;
  }
  splitTree(*parent);
}

void gpsshogi::SearchTree::
stopTree() 
{
  if (! root) {
    Logging::error("*stop without root??");
    return;
  }
  searching().stopped = true;
  for (size_t i=0; i<task_queue.size(); ++i) {
    if (task_queue[i] == Think) {
      task_queue.erase(task_queue.begin()+i);
      usi_queue.post(std::bind(finish, "bestmove resign"));
      internalFinish(searching().position_id);
      return;
    }
  }

  task_queue.push_back(WaitSlave);
  task_queue.push_back(WaitSlave);
  task_queue.push_back(WaitSlave);
  stopSubTree(*root);
}

void gpsshogi::
SearchTree::internalFinish(int id)
{
  if (searching().position_id != id) {
    Logging::error("*finish id inconsistent " + to_s(id)
		   + " != " + to_s(searching().position_id));
    return;
  }
  searching().last_status = status;
  searching().finished = true;
  searching().node_count = root->nodeCount();
  searching().node_count_best = root->nodeCountBestMove();
  if (searching().node_count_best == 0)
    searching().node_count_best = root->nodeCount();
  if (root->isSolved() || root->moves.normal.size()<2)
    searching().forced_move_or_similar = true;
  searching().value = root->value();
  finish = 0;
  progress = [=](std::string msg){ this->ponderProgress(msg); };
}

void gpsshogi::
SearchTree::ponderProgress(std::string msg)
{
  if (! ponder) {
    Logging::notice("*ignored ponder progress "+msg);
    return;
  }
  InterimReport relative, info;
  relative.updateByInfo(msg, -1);
  osl::HistoryState copy(state.currentState());
  info.set(copy.state().turn(), relative);
  std::string pv = SearchNode::toCSA(copy, info.joinPV());
  static int last_value = 0;
  if (info.bestValue() != last_value
      && osl::elapsedSeconds(last_report) > probe_msec*5.0/1000) {
    std::ostringstream ss;
    ss << "ponder " << std::setfill(' ') << std::setw(5)
       << info.bestValue()*100/usi_pawn_value
       << ' ' << pv << " (" << info.pv.size()
       << ' ' << info.depth_head << ")";
    Logging::notice(ss.str());
    last_value = info.bestValue();
    last_report = osl::clock::now();
  }
}

std::pair<int,int> gpsshogi::SearchTree::
assignTimeOSL(const TimeCondition& config,
	      const osl::game_playing::GameState& game_state) const
{
  const int left = config.total - config.used;
  int seconds = osl::game_playing::SearchPlayer::
    secondsForThisMove(game_state, config.total, config.used,
		       config.byoyomi(), false);
  if (left*2 < (config.total - config.opponent_used))
    seconds = seconds/2;
  else if (left*3 < (config.total - config.opponent_used)*2)
    seconds = seconds*2/3;
  else if (left*5 < (config.total - config.opponent_used)*4)
    seconds = seconds*4/5;
  seconds = std::max(std::min(seconds, 30*config.total/1500), 1);
  if (state.moves.size() > 30 && left>=90)
    seconds = std::max(6*config.total/900, seconds);
  if (config.byoyomi_msec > 0)
    seconds = std::max(seconds, 2*config.byoyomi());
  seconds = std::max(seconds, config.byoyomi());
  seconds = std::min(seconds, left+config.byoyomi());
  const int seconds_max = (config.byoyomi_msec && (left<config.byoyomi()))
    ? (std::max(left,0) + config.byoyomi())
    : std::min((int)(seconds*2.5), std::max(seconds, left/2));
  return std::make_pair(seconds, seconds_max);
}

bool sufficientHandPieces(const osl::NumEffectState& state) 
{
  int pawns = 
    state.countPiecesOnStand(osl::BLACK, osl::PAWN)
    + state.countPiecesOnStand(osl::WHITE, osl::PAWN);
  if (pawns > 2)
    return true;
  int sum = 0;
  for (osl::Ptype ptype: osl::PieceStand::order) {
    sum += state.countPiecesOnStand(osl::BLACK, ptype)
      + state.countPiecesOnStand(osl::WHITE, ptype);
  }
  return (sum - pawns) > 0;
}

std::pair<int,int> gpsshogi::SearchTree::
assignTime(const TimeCondition& config,
	   const osl::game_playing::GameState& game_state) const
{
  int left = config.total - config.used;
  if (left < config.byoyomi()*2)
    return std::make_pair(config.byoyomi(), config.byoyomi());
  
  double nmoves = state.moves.size();
  if (config.byoyomi_msec > 0) {
    double a = config.byoyomi()*2;
    double tt = 120 + nmoves/5; // horizon
    double t = (nmoves < tt) ? (tt - nmoves)/2+1 : 10; // distance to horizon
    if (a < 1.0 * left / t)
      a = 1.0 * left / t;
    int hard = a*2 < left ? a*2 : left;
    return std::make_pair(a, hard);
  }
  if (left < 20) {
    return std::make_pair(1,1);
  } else if (left < 60) {
    return std::make_pair(1,2);
  }
  double p = this->ponder ? 2.2 : 2.0;
  double t = (nmoves < 104) ? ((125 - nmoves)/p+1) : nmoves/8;
  double reduce = (config.byoyomi() < 30) ? 0.85 : 1.0;
  double a = reduce*(left - 60) / t + 1;
  double scale = (config.byoyomi() < 30) ? 2.5 : 5;
  double standard = a;
  double hard = a*scale < (left-60) ? a*scale : (left-60);

  // adjustment
  // limit
  if (config.byoyomi() < 30) {
    double maximum = 24.0 * config.total / 1500;
    a = std::min(a, maximum);
  }
  // increase
  osl::NumEffectState state = game_state.state();
  if (left > config.total/2
      && sufficientHandPieces(state)
      && standard < config.total/60.0) {
    double a = config.total/60.0;
    standard = a;
    hard = a*2.5 < (left-60) ? a*2.5 : (left-60);
    Logging::info("increased " + to_s((int)standard) + " => " + to_s((int)a));
  }
  // decrease
  if (human_rounding == 0	// opponent's time is not always accurately measured when playing against human
      && config.used > config.total*0.2
      && standard > 5.0) {
    double op_left = config.total - config.opponent_used;
    double coef = 0;
    if (op_left*0.5 > left) {
      coef = 0.5;
    } else if (op_left*0.7 > left) {
      coef = 0.4;
    } else if (op_left*0.8 > left) {
      coef = 0.3;
    } else if (op_left*0.9 > left) {
      coef = 0.2;
    } else if (op_left > left) {
      coef = 0.1;
    }
    if (coef > 0) {
      Logging::info("decreased search time by "+to_s((int)(100*coef))
		    + "% " + to_s((int)standard * coef));
      standard *= (1-coef);
      hard *= (1-coef);
    }
  }
  return std::pair<int,int>(standard, hard);
}

std::function<void(void)> gpsshogi::SearchTree::
go(int id, const std::string& position, 
   int active, const std::vector<UsiSlavePtr>& slaves,
   std::function<void(std::string)> p,
   std::function<void(std::string)> f,
   const TimeCondition& config,
   const std::vector<UsiSlavePtr>& mate_solvers)
{
  const bool pondered = this->ponder;
  if (pondered) {
    if (root && root->position != position) {
      Logging::error("*different root "+root->position + " != " + position);
      stopTree();
    }
  }
  mate_idle.insert(mate_solvers.begin(), mate_solvers.end());
  // ensure root is up to date
  const bool root_ready = root && root->position == position;
  if (! root_ready) {
    state.parseUsi(position);
    root.reset(new SearchNode(position,
			      osl::HashKey(state.currentState()), 0, ""));
  }
  // time assignment 
  SearchStatus new_status;
  new_status.position_id = id;
  new_status.config = config;
  new_status.sufficient_ponder = searching().sufficient_ponder;
  if (config.used >= config.total) {
    new_status.msec_standard = config.byoyomi_msec;
    new_status.msec_max = config.byoyomi_msec;
  }
  else {
    osl::game_playing::GameState game_state(state.initial_state);
    for (osl::Move move: state.moves)
      game_state.pushMove(move);
    std::pair<int,int> time = assignTime(config, game_state);
    new_status.msec_standard = std::max(1,time.first);
    new_status.msec_max = std::max(1,time.second);
    if (human_rounding > 0) {
      new_status.msec_standard
	= (new_status.msec_standard/60*60 + human_rounding);
      new_status.msec_max
	= (new_status.msec_max/60*60 + human_rounding);
    }
    new_status.msec_standard = new_status.msec_standard*1000 + rounding_msec;
    new_status.msec_max = new_status.msec_max*1000 + rounding_msec;
  }
  if (!root->needProbe()) {
    statistics.prepared++;
  }
  for (UsiSlavePtr slave: slaves) {
    if (all_slave.insert(slave->id()).second) {
      if (scheduled_slave.count(slave->id()) == 0 
	  && restart_other_waiting.count(slave->id()) == 0) {
	// these conditions should be true as slave->id() is unique
	current_idle.insert(slave);
      }
      else
	Logging::error("*inconsistency in slave id "+to_s(slave->id()));
    }
  }
  history.push_back(new_status);
  this->ponder = config.allow_ponder;
    // && (stopped_slave.size()+current_idle.size())>1;
  progress = p;
  finish = f;
  adjustSplitWidth(active, new_status.msec_standard);

  // draw value
  int draw_value = draw_value_cp*usi_pawn_value/100;
  if (config.total - config.used <= 10 && config.byoyomi_msec == 0)
    draw_value = std::max(draw_value, usi_pawn_value*10);
  SearchNode::setDrawValue(draw_value*sign(root->turn()));
  searching().early_win_needed
    = (config.byoyomi_msec > 0) ? false
    : osl::inaniwa::InaniwaDetection::IsInaniwa
    (state.currentState(), config.total, config.total,
     (config.total-config.used),
     (config.total-config.opponent_used));
    // = (config.total-config.used)*8 < (config.total-config.opponent_used);
  searching().in_check = state.currentState().inCheck();

  // root info
  if (root->moves.empty())
    root->generateMoves();
  {
    std::ostringstream ss;
    ss << "*rootmoves = "  << root->moves.all.size()
       << " draw " << root->moves.draw.size()
       << " loss " << root->moves.loss.size()
       << " nopromote " << root->moves.nopromote.size() << "\n";
    ss << state.currentState();
    if (! state.moves.empty())
      ss << "last move " << osl::csa::show(state.moves.back());
    ss << " used " << config.used << ' ' << config.opponent_used
       << " time " << msec()/1000 << ' ' << msecMax()/1000
       << " other/ext/pre/all " << statistics.selected_other
       << "/" << statistics.extended << "/" << statistics.prepared
       << "/" << statistics.values.size()
       << " width " << splitWidth(/*leaf*/128)
       << " # " + to_s(all_slave.size())
       << " draw " << draw_value*sign(root->turn())
       << " status " << to_s(status)
       << " ina " << to_s(searching().early_win_needed)
       << uptimeString();
    Logging::notice(ss.str());
  }
  {
    std::ostringstream ss;
    if (root->moves.draw.size()) 
      ss << " draw " + join(root->moves.draw, " ") << "\n";
    if (root->moves.loss.size()) 
      ss << " loss " + join(root->moves.loss, " ") << "\n";
    if (root->moves.nopromote.size()) 
      ss << " nopromote " + join(root->moves.nopromote, " ") << "\n";
    Logging::info(ss.str(), true);
  }

  // start thinking or continue ponder
  if (root_ready && pondered && root->hasWorker()) {
    if (stopped_slave.empty())
      while (! task_queue.empty() && task_queue.front() == WaitSlave)
	task_queue.pop_front();
    if (std::count(task_queue.begin(), task_queue.end(), GrowTree))
      task_queue.push_back(Think);
    else
      status = Thinking;
    std::string msg = "*pondering => thinking ";
    for (Task task: task_queue)
      msg += " "+to_s(task);
    Logging::info(msg);
    return std::bind(&SearchTree::stopTree, this);
  }

  // construct root
  if (root->isSolved() && root->isWinValue(root->value())) {
    Logging::notice("immediate response "+root->bestMove());
    usi_queue.post(std::bind(finish, "bestmove "+root->bestMove()));
    searching().forced_move_or_similar = true;
    internalFinish(id);
    return 0;      
  }
  if (root->moves.normal.empty()) {
    if (! root->moves.draw.empty()) {
      Logging::notice("no normal moves other than draw");
      usi_queue.post(std::bind(finish, "bestmove "
				 +*root->moves.draw.begin()));
      searching().forced_move_or_similar = true;
      internalFinish(id);
      return 0;
    }
    if (root->moves.all.empty() && root->moves.nopromote.empty()) {
      Logging::notice("no not-losing moves");
      usi_queue.post(std::bind(finish, "bestmove resign"));
      searching().forced_move_or_similar = true;
      internalFinish(id);
      return 0;
    }
    root->moves.normal.insert
      (root->moves.normal.end(), root->moves.nopromote.begin(), root->moves.nopromote.end());
    root->moves.nopromote.clear();
  }
  if (root->moves.all.size() == 1 && root->moves.draw.empty()) {
    Logging::notice("forced move");
    usi_queue.post(std::bind
		   (progress, "info string forced move at the root: "
		    + root->moves.normal[0]));
    usi_queue.post(std::bind
		   (finish, "bestmove "+root->moves.normal[0]));
    searching().forced_move_or_similar = true;
    internalFinish(id);
    return 0;
  }
  if (stopped_slave.empty())
    while (! task_queue.empty() && task_queue.front() == WaitSlave)
      task_queue.pop_front();
  if (! task_queue.empty()) {
    task_queue.push_back(WaitSlave);
    task_queue.push_back(WaitSlave);
    task_queue.push_back(WaitSlave);
    task_queue.push_back(Think);
    Logging::notice("*defer startThinking() in 150ms");
    return std::bind(&SearchTree::stopTree, this);
  }
  startThinking();
  return std::bind(&SearchTree::stopTree, this);
}

struct NpsCompare
{
  bool operator()(const gpsshogi::UsiSlavePtr& a,
		  const gpsshogi::UsiSlavePtr& b) const
  {
    return a->nps() > b->nps();
  }
};

std::vector<gpsshogi::UsiSlavePtr> gpsshogi::SearchTree::
selectIdle(std::set<UsiSlavePtr>& idle)
{
  std::vector<UsiSlavePtr> slaves;
  std::string reserved="", stopped="";
  for (UsiSlavePtr p: idle) {
    if (!p) {
      Logging::error("*slave null");
      continue;
    }
    if (restart_other_waiting.count(p->id())
	|| scheduled_slave.count(p->id())) {
      reserved += " "+to_s(p->id());
      continue;
    }

    if (p->error() || p->usiStatus() != Idle || wasStopped(p->id())) {
      if (p->error())
	Logging::error("*ignored disconnected slave " + to_s(p->id()));
      else if (p->usiStatus() != Idle || ! p->isMateSolver())
	Logging::info("*slave not idle " + to_s(p->id()) + ' '
		      + to_s(p->usiStatus()));
      if (wasStopped(p->id()))
	stopped += p->idMark();
    }
    else {
      slaves.push_back(p);
    }
  }
  if (reserved != "")
    Logging::info("*reserved slave(s)" + reserved);
  if (stopped != "")
    Logging::info("*stopped slave(s)" + stopped);
  idle.clear();
  std::stable_sort(slaves.begin(), slaves.end(), NpsCompare());
  return slaves;
}

void gpsshogi::SearchTree::
startThinking()
{
  Logging::info("*start thinking\n", true);
  std::vector<UsiSlavePtr> slaves = idleSlaves();
  if (slaves.empty()) {
    task_queue.push_back(Think);
    Logging::notice("*defer startThinking() again");
    return;
  }
  status = Thinking;
  // probe and split
  probeAndSplit(*root, slaves);
}

std::vector<gpsshogi::UsiSlavePtr> gpsshogi::SearchTree::idleSlaves() 
{
  std::vector<UsiSlavePtr> ret = selectIdle(current_idle);
#if 0
  // need to make consistency with grow tree etc.
  if (msec() <= 2000 && ret.size()>8)
    ret.resize(64);
#endif
  return ret;
}

void gpsshogi::SearchTree::
probeAndSplit(SearchNode& node, std::vector<UsiSlavePtr> slaves)
{
  if (! current_idle.empty()) {
    std::vector<UsiSlavePtr> more_slaves = idleSlaves();
    slaves.insert(slaves.end(), more_slaves.begin(), more_slaves.end());
    std::stable_sort(slaves.begin(), slaves.end(), NpsCompare());
    slaves.erase(std::unique(slaves.begin(), slaves.end()), slaves.end());
  }  
  const std::string path = node.path();
  node.prepareProbe(stopped_slave); 
  {
    std::string msg
      = "*probe " + path+" "+ to_s(osl::msec(node.probe.start - started()))
      + "ms #";
    for (UsiSlavePtr slave: slaves) {
      current_idle.erase(slave);
      msg += " "+to_s(slave->id());
    }
    if (node.isSolved()) 
      msg += " already solved";
    if (node.moves.empty())
      msg += " no legal moves";
    Logging::info(msg);
  }
  if (node.isSolved()) {
    node.update(progress);
    current_idle.insert(slaves.begin(), slaves.end());
    return;
  }
  if (slaves.empty()){
    Logging::error("* no slaves?");
    return;
  }  
  if (node.moves.normal.size() == 1) {
    Logging::info("*probe "+path+" one-reply");
    node.split_time = node.probe.start;
    node.committed = true;
    node.setWorking();
    SearchNode& n = *node.successor(node.moves.normal[0]);
    probeAndSplit(n, slaves);
    return;
  }
  if (slaves.size() == 1) {
    goNode(slaves[0], node, msec_max);
    return;
  }
  // reuse tree?
  if (! node.needProbe()) {
    Logging::info("*probe "+path+" reuse tree " + to_s(node.succ.size()));
    node.probe.start = node.split_time; // reuse past split
    node.probe.split_waiting = slaves;
    node.probe.probed.swap(node.succ);
    node.succ.clear();
    splitTree(node);
    return;
  }
  if (! node.succ.empty()) {
    Logging::warn("*probe "+path+" cleared "+to_s(node.succ.size()));
    node.succ.clear();
    // moves in succ are ignored in prepareProbe() 
    node.probe.target = node.moves.normal; 
    node.committed = false;
  }
  // pv_hint?
  if (node.leaf.pv_hint.size() || node.pv().size()) {
    const std::string split_move = 
      node.pv().empty() ? node.leaf.pv_hint[0] : node.pv()[0];
    if (! node.findMove(split_move).isNormal()) {
      Logging::info("*ignore pvhint "+split_move);
    }
    else {
      std::vector<UsiSlavePtr> half;
      if (node.moves.normal.size() == 2
	  || node.isPrimaryTakeBack(split_move)) { // almost all and other
	std::swap(half, slaves);
	slaves.push_back(half[0]);
	half.erase(half.begin());
	if (node.isPrimaryTakeBack(split_move))
	  Logging::info("*primary takeback "+path+" "+node.toCSA(split_move));
      }
      else {
	int width = std::min(3, splitWidth(node.depth8()));
	for (size_t i=1; i<slaves.size(); i+=width) // reserve [0]
	  half.push_back(slaves[i]); 
	for (UsiSlavePtr slave: half)
	  std::remove(slaves.begin(), slaves.end(), slave);
	slaves.resize(slaves.size()-half.size());
      }
      Logging::info("*early split "+path+" "+node.toCSA(split_move)
		    + "#" + to_s(half.size())
		    + " and other #"+to_s(slaves.size()));
      node.committed = true;
      probeAndSplit(*node.successor(split_move), half);
      node.probe.removeTarget(split_move);
      node.probe.removeTarget(split_move);
      Logging::info("*removed target "+split_move+" "
		    +to_s(std::count(node.probe.target.begin(), 
				     node.probe.target.end(), split_move))
		    + " "+node.path());
      if (slaves.size() == 1) {
	SearchNode& other = *node.successorOther();
	Logging::info("*single slave left for others "+path+" "
		      +node.toCSA(other.ignore_moves)
		      +" "+slaves[0]->idMark());
	node.setWorking();
	node.split_time = node.probe.start;
	goNode(slaves[0], other, msec_max);
	return;
      }
    }
    // fall through
  }
  // enough time ?
  int elapsed=(status==Thinking) ? osl::msec(node.probe.start-started()) : 0;
  if ((msec() - elapsed) < probe_msec*4) {
    Logging::info("*split by probability " + node.moves.normal[0]
		  + " at " + path);
    node.probe.split_waiting.insert(node.probe.split_waiting.end(),
				    slaves.begin(), slaves.end());
    splitTree(node);
    return;
  }
  // normal probe 
  const std::vector<std::string>& moves = node.probe.target;
  const size_t nslaves = slaves.size();
  const bool need_others = moves.size() > nslaves;
  const size_t first_n = std::min(moves.size(), nslaves-need_others);
  Logging::info("*probe normal "+path+" #"+to_s(nslaves)+"/"+to_s(need_others)
		+"/"+to_s(first_n)+" "+fancyString(node, moves, first_n));
  size_t i=0;
  for (; i<first_n; ++i) {
    SearchNode& n = *node.successor(moves[i]);
    goProbe(slaves[i], n);
  }
  if (need_others) {
    SearchNode& n = *node.successorOther();
    goProbe(slaves[i], n);
  }
  else if (first_n < nslaves) {
    for (; i<nslaves; ++i)
      node.probe.split_waiting.push_back(slaves[i]);
  }
}

std::string gpsshogi::SearchTree::
fancyString(const SearchNode& node, const std::vector<std::string>& moves, size_t size_limit, const std::vector<int> *evals)
{
  std::string leaves;
  for (size_t i=0; i<std::min(moves.size(), size_limit); ++i) {
    leaves += node.toCSA(moves[i]);
    if (evals && i < evals->size())
      leaves += "("+to_s((*evals)[i])+")";
  }
  if (moves.size() > size_limit)
    leaves += "...";
  return leaves;
}

void gpsshogi::SearchTree::
flatSplit(SearchNode& node, const std::vector<std::string>& moves,
	  const std::vector<UsiSlavePtr>& slaves)
{
  const int nslaves = slaves.size();
  const bool need_others = moves.size() > nslaves;
  const int first_n = std::min((int)moves.size(), nslaves-need_others);
  Logging::info("*split leaf #"+to_s(nslaves)+"/"+to_s(need_others)
		+"/"+to_s(first_n)+" "+fancyString(node, moves, first_n));
  assert(first_n <= nslaves);
  size_t i=0;
  for (; i<first_n; ++i) {
    SearchNode& n = *node.successor(moves.at(i));
    goNode(slaves.at(i), n, msec_max);
  }
  if (need_others) {
    SearchNode& n = *node.successorOther();
    goNode(slaves.at(i), n, msec_max);
  }
}

void gpsshogi::SearchTree::splitTree(SearchNode& node) 
{
  const std::string path = node.path();
  node.prepareSplit();
  std::vector<UsiSlavePtr> slaves;
  std::vector<int> evals;
  std::vector<std::string> moves = node.sortByProbe(&evals);
  {
    std::string msg
      = "*split " + path + " " + to_s(osl::msec(node.split_time - started()))
      + "ms probe " + to_s(osl::msec(node.probe.start - started()))
      + "ms #";
    std::string slave_error="";
    for (UsiSlavePtr slave: node.probe.split_waiting) {
      if (slave->usiStatus() != Idle)
	slave_error += " "+slave->idMark()+to_s(slave->usiStatus());
      else {
	scheduled_slave.erase(slave->id());
	current_idle.erase(slave);
	slaves.push_back(slave);
	msg += " "+to_s(slave->id());
      }
    }
    if (slave_error != "")
      Logging::error("*slave not idle" + slave_error);
    if (! stopped_slave.empty()) {
      std::vector<UsiSlavePtr> more_slaves = idleSlaves();
      slaves.insert(slaves.end(), more_slaves.begin(), more_slaves.end());
      std::stable_sort(slaves.begin(), slaves.end(), NpsCompare());
      slaves.erase(std::unique(slaves.begin(), slaves.end()), slaves.end());
    }  

    msg += " "+fancyString(node, moves, slaves.size(), &evals);
    Logging::info(msg);
    if (! node.parent) {
      logStatus();
      last_report = osl::clock::now();
    }
  }
  assert(node.probe.start != osl::time_point()); // called after probe
  node.probe.split_waiting.clear();
  if (node.isSolved()) {
    Logging::info("*split cancelled at solved node "+node.path());
    node.update(progress);
    current_idle.insert(slaves.begin(), slaves.end());
    return;
  }
  if (moves.size() == 1) {
    SearchNode& n = *node.successor(moves[0]);
    probeAndSplit(n, slaves);
    return;
  }
  if (slaves.size() <= 3) {
    flatSplit(node, moves, slaves);
    return;
  }
  // split tree
  int width = std::min((int)slaves.size()-1, std::min((int)moves.size(), splitWidth(node.depth8())));
  for (int i=1; i<width; ++i) {
    if (abs(evals[0] - evals[i]) > grow_tree_margin(slaves.size())) {
      Logging::info("*narrowing split width "+to_s(width)+" to "+to_s(i)
		    +" at "+node.path());
      width = i;
      break;
    }
  }   
  const bool need_other = (moves.size() > width);
  std::vector<std::vector<UsiSlavePtr> > p(width);
  for (int i=0; i<width; ++i)
    p[i].reserve(slaves.size()/width + 1);
  if (need_other)
    for (size_t i=0; i+1<slaves.size(); ++i)
      p[i%width].push_back(slaves[i+1]); // reserve 0 for other
  else
    for (size_t i=0; i<slaves.size(); ++i)
      p[i%width].push_back(slaves[i]);
  // generate all child
  for (int i=0; i<width; ++i)
    node.successor(moves[i]);
  if (need_other)
    node.successorOther();
  // go for each node
  for (int i=0; i<width; ++i) {
    SearchNode& n = *node.successor(moves[i]);
    probeAndSplit(n, p[i]);
  }
  if (need_other) {
    SearchNode& n = *node.successorOther();
    goNode(slaves[0], n, msec_max);
  }
}

void gpsshogi::SearchTree::
setUpHeuristic(SearchNode& node) 
{
  if (node.heuristic_cost)
    return;
  if (searching().early_win_needed) {
    osl::UsiState usi_state;
    usi_state.parseUsi(node.position);
    osl::NumEffectState state = usi_state.currentState();
    osl::Player turn = searching().config.my_turn;
    int count = 0;
    for (osl::Ptype ptype: osl::PieceStand::order)
      count += state.countPiecesOnStand(turn, ptype);
    node.heuristic_cost = count*usi_pawn_value*sign(turn);
    if (node.heuristic_cost)
      Logging::info("*short time heuristic "+node.path()
		    +" "+to_s(node.heuristic_cost));
  } 
}

void gpsshogi::SearchTree::
goNode(const UsiSlavePtr& slave, SearchNode& node, int millisec)
{
  setUpHeuristic(node);
  int multi_pv = 1;
  if (searching().config.my_turn != node.turn()
      && node.parent_move == "other"
      && node.parent == root.get())
    multi_pv = 2;
  auto path = node.pathId();
  node.searchInLeaf
    ([=](InterimReport r) { this->nodeProgress(slave, path, r); },
     [=](InterimReport r) { this->nodeFinish(slave, path, r); },
     millisec, slave, stopped_slave, multi_pv);
}

void gpsshogi::SearchTree::
goProbe(const UsiSlavePtr& slave, SearchNode& child)
{
  if (wasStopped(slave->id())) {
    Logging::error("*go probe with stopped slave "+slave->idMark());
    stopped_slave.erase(slave->id());
  }
    
  auto path = child.pathId();
  child.parent->probe.probing.insert(slave->id());
  child.searchInLeaf
    ([=](InterimReport r) { this->probeProgress(slave, path, r); },
     [=](InterimReport r) { this->probeFinish(slave, path, r); },
     probe_msec, slave, stopped_slave, 1);
}

void gpsshogi::SearchTree::
showStatus(std::ostream& os)
{
  if (! root)
    return;
  os << Logging::datetime()
     << " status=" << to_s(status) << " elapsed "
     << osl::msec(osl::clock::now()-started())
     << " msec-max " << msecMax() << "\n";
  osl::HistoryState copy(state.currentState());
  assert(copy.state().turn() == root->turn());
  root->showStatus(os, copy, osl::clock::now());
}

void gpsshogi::SearchTree::
logStatus(const std::string& filename)
{
  if (! root)
    return;
  std::ostringstream ss;
  showStatus(ss);
  Logging::info(ss.str(), true);
  if (filename != "") {
    std::ofstream os((Logging::path()+filename).c_str());
    os << ss.str();
  }
}

void gpsshogi::SearchTree::
periodic() 
{
  if (stopped_slave.empty())
    while (! task_queue.empty() && task_queue.front() == WaitSlave)
      task_queue.pop_front();
  if (! task_queue.empty()) {
    const Task task = task_queue.front();
    task_queue.pop_front();
    switch (task) {
    case WaitSlave:
      return;
    case Think:
      startThinking();
      return;
    case GrowTree:
      growTree();
      return;
    case ExpandOther:
      expandOther();
      return;
    }
  }
  if (status == Thinking || status == PanicTimeOther) {
    finishIfStable();
    if (searching().finished)
      return;
  }
  if (std::count(task_queue.begin(), task_queue.end(), GrowTree)
      || std::count(task_queue.begin(), task_queue.end(), Think)
      || std::count(task_queue.begin(), task_queue.end(), ExpandOther))
    return;
  osl::time_point now = osl::clock::now();
  int elapsed=(status==Thinking) ? osl::msec(now-searching().started) : 0;
  static osl::time_point last_assign, last_health_test;
  if ((status == Thinking || this->ponder)
      && osl::msec(now-last_assign) > probe_msec*2
      && (msec() > 4000 || !root->hasWorker())
      && elapsed*2 < msec()
      && ! root->probe.onGoing()
      && current_idle.size()*4 >= all_slave.size()) {
    Logging::info("*reassign idles #"+to_s(current_idle.size()));
    growTree(status == Thinking || status == PanicTimeOther);
    last_assign = now = osl::clock::now();
  }
  if (! mate_idle.empty() && task_queue.empty() && status != TreeIdle)
    assignMate();
  if (status != TreeIdle
      && root && osl::msec(now-last_health_test) > probe_msec) {
    Logging::info("*run health test");
    last_health_test = now = osl::misc::clock::now();
    healthTest(*root, now);
  }
}

void gpsshogi::SearchTree::healthTest(SearchNode& node,
				      const osl::time_point &now) 
{
  if (node.moves.empty() || node.isSolved())
    return;
  if (node.probe.onGoing()) {
    int elapsed_msec = osl::msec(now-node.probe.start);
    if (elapsed_msec >= probe_msec*2) {
      std::string msg = "*delay in split "+to_s(elapsed_msec)
	+" msec "+node.path()
	+" "+node.probe.status();
      Logging::warn(msg);
      if (node.probe.probing.empty() && node.probe.split_waiting.size()>0) {
	Logging::warn("*forced split "+node.path()); // should not come here
	splitTree(node);
      }
    }
  }

  typedef successor_table_t::value_type value_type;
  for (value_type& child: node.succ) {
    if (child.first != "other" && child.second)
      healthTest(*child.second, now);
  }
}

void gpsshogi::SearchTree::assignMate() 
{
  std::vector<UsiSlavePtr> slaves = idleMateSolvers();
  if (slaves.empty())
    return;
  size_t i=0; for (;i<slaves.size(); ++i) {
    if (! assignMate(*root, slaves[i]))
      break;
  }
  mate_idle.insert(slaves.begin()+i, slaves.end());
}

bool gpsshogi::SearchTree::assignMate(SearchNode& node, UsiSlavePtr solver) 
{
  if (node.moves.empty())
    node.generateMoves();
  if (node.isSolved())
    return false;
  if (! node.mate_tested && ! node.leaf.working[1]) {
    auto path = node.pathId();
    node.runMate([=](std::string msg) { this->mateResult(solver, path, msg);},
		 std::min(msec(),probe_msec*10), solver);
    return true;
  }
  typedef successor_table_t::value_type value_type;
  for (value_type& child: node.succ) {
    if (child.first != "other" && child.second
	&& assignMate(*child.second, solver))
      return true;
  }
  return false;
}

void gpsshogi::SearchTree::
mateResult(UsiSlavePtr slave, std::string node_id, std::string msg)
{
  std::string warn = "";
  if (slave->error())
    warn = " error in mateslave" + slave->idMark();
  else
    mate_idle.insert(slave);
  if (wasStopped(slave->id())) {
    if (warn != "")
      Logging::warn(warn);
    stopped_slave.erase(slave->id());
    return;
  }
  SearchNode *node = root ? root->find(node_id) : 0;
  if (! node) {
    // this is not serious: may be vanished after probe
    Logging::info("*node not found in mate result " + node_id + ' '+msg
		  + slave->idMark() + warn);
    return;
  } 
  Logging::info("*mate " +node->path() + " " + msg + slave->idMark());
  if (node->leaf.working[1] && node->leaf.working[1] != slave) 
    Logging::info("*mate worker inconsistent" + slave->idMark()
		  + (node->leaf.working[1]?node->leaf.working[1]->idMark():" ? "));
  else
    node->leaf.working[1].reset();
  if (warn != "") {
    Logging::warn(warn);
    return;
  }  
  node->mate_tested = true;
  std::vector<std::string> moves;
  boost::algorithm::split(moves, msg, boost::algorithm::is_any_of(" "));
  if (moves.size()<2 || moves[0] != "checkmate") {
    Logging::error("*unexpected mate " +node->path()+" "+msg);
    return;
  } 
  if (moves[1] == "nomate" || moves[1] == "notimplemented"
      || moves[1] == "timeout" || node->isSolved())
    return;
  if (! node->findMove(moves[1]).isNormal()) {
    Logging::error("*mate move does not exist " +node->path()+" "+msg);
    return;
  }
  // mate found
  node->solved.node_count = 1;	// todo
  node->solved.pv.score = node->winValue(moves.size());
  node->solved.pv.moves = moves;
  node->solved.pv.moves.erase(node->solved.pv.moves.begin());
  if (!node->parent || !node->parent->probe.onGoing()) {
    Logging::info("*stop by mate "+node->path());
    stopSubTree(*node);
  }
  node->update(progress);
}

uint64_t gpsshogi::SearchTree::recentNodeCountBest() const {
  const size_t stable_size = 7;
  std::vector<uint64_t> ret;
  for (size_t i=0; i+3<std::min(stable_size+3,history.size()); ++i)
    if (history[history.size()-i-1].last_status != PanicTimeOther
	&& ! history[history.size()-i-1].forced_move_or_similar
	&& history[history.size()-i-1].sufficient_ponder == ""
	&& ! history[history.size()-i-1].in_check)
      ret.push_back(history[history.size()-i-1].node_count_best);
  if (ret.size() <= 1)
    return std::numeric_limits<uint64_t>::max();
  std::sort(ret.begin(), ret.end());
  return ret[ret.size()/2];
}

std::string treeFileName(const std::string& type, int move_number,
			 const std::string& additional="")
{
  char s[256];
  sprintf(s, "%03d", move_number);
  std::string result = gpsshogi::to_s(s) + "-" + type;
  if (additional != "")
    result += "-"
      + (isalnum(additional[0]) ? additional : additional.substr(1));
  return result+".txt";
}

void gpsshogi::SearchTree::
finishIfStable()
{
  assert(root && !searching().finished);
  // finish if stable
  osl::time_point now = osl::clock::now();
  int elapsed = osl::msec(now-started());
  bool finish_search = false;
  if (searching().sufficient_ponder != ""
      && searching().sufficient_ponder == root->bestMove()) {
    finish_search = true;
  }
  if (! finish_search
      && status == Thinking
      && osl::msec(now-root->lastTreeChangeForBestMove()) > msec()*3/4
      && root->betterThan(root->value()+usi_pawn_value/2, prev().value)
      && root->nodeCountBestMove() > recentNodeCountBest()) {
    if (! root->bestMoveIsOther()) {
      if (msec() < msecMax()) {
	Logging::info("*finish by node count "
		      + to_s(root->nodeCountBestMove())
		      + " > " + to_s(recentNodeCountBest())
		      + " " + to_s(elapsed) +"ms "+to_s(msec())+"ms");
	finish_search = true;
      }
    }
    else if (msec()>probe_msec*12 && elapsed > msec()/3
	     && elapsed + msec()/3 < msecMax()) {
      Logging::info("*early expand other by node count " + to_s(elapsed)
		    +" "+to_s(msec()));
      tryExpandOther(elapsed);
    }
  }
  if (! finish_search
      && msec() < msecMax()
      && status == Thinking
      && root->isPrimaryTakeBack(root->bestMove())
      && osl::msec(now-root->lastTreeChangeForBestMove()) > msec()/2
      && root->betterThan(root->value()+usi_pawn_value/2, prev().value)
      && root->nodeCountBestMove() > recentNodeCountBest()/2) {
    Logging::info("*early finish for takeback "+root->toCSA(root->bestMove())
		  + " " + to_s(elapsed) +"ms "+to_s(msec())+"ms");
    finish_search = true;
  }
  if (! finish_search && (status == Thinking || status == PanicTimeOther)
      && (searching().stopped || root->isSolved() || elapsed > msecMax()
	  || (elapsed > msecRestart() && ! root->bestMoveIsOther()
	      && std::count(task_queue.begin(), task_queue.end(),
			    ExpandOther) == 0
	      && root->bestMove() != ""
	      && root->bestMove() != "resign"))) {
    finish_search = true;
    std::string msg = "*finish search " + to_s(elapsed)
      + " " + root->bestMove();
    std::deque<Task>::iterator p
      = std::remove(task_queue.begin(), task_queue.end(), ExpandOther);
    if (p != task_queue.end()) {
      msg += " canceled ExpandOther";
      task_queue.erase(p, task_queue.end());
    }
    Logging::info(msg);
  }
  if (finish_search) {
    showRootInfo();
    finish(root->report().makeSearchResult());
    internalFinish(searching().position_id);
    statistics.values.push_back(root->value());
    if (root->bestMoveIsOther())
      statistics.selected_other++;
    if (ponder) {
      status = Pondering0;
    }
    else {
      stopTree();
      status = TreeIdle;
    }
    std::string filename = treeFileName
      ("gps", state.moves.size()+1, root->toCSA(root->bestMove()));
    logStatus(filename);
    std::ofstream os((Logging::path()+filename).c_str(), std::ios::app);
    os << state.currentState();
    return;
  }
  if (status == Thinking && elapsed > msec()
      && root->bestMoveIsOther()) {
    tryExpandOther(elapsed);
  }
}

void gpsshogi::SearchTree::tryExpandOther(double elapsed)
{
  logStatus();
  last_report = osl::clock::now();

  const std::vector<std::string> best_moves = root->sortBySearchValue();
  if (! root->findMove(root->bestMove()).isNormal()
      || best_moves.empty()
      || ! root->findMove(best_moves[0]).isNormal()) {
    std::string msg = "*unusual other bestmove "+root->bestMove()
      +to_s(best_moves.size());
    if (best_moves.size())
      msg += " "+best_moves[0];
    Logging::error(msg);
    return;
  }
  const std::string best_move2 = (best_moves.size() < 2)
    ? "" : best_moves[1]; // i.e., best move among normal children
  typedef successor_table_t::value_type value_type;
  for (value_type& child: root->succ)
    if (child.first != best_move2 && child.first != "other")
      stopSubTree(*child.second);
  const int nslaves = stopped_slave.size()+current_idle.size();
  task_queue.push_back(WaitSlave);
  task_queue.push_back(WaitSlave);
  task_queue.push_back(WaitSlave);
  if (nslaves >= 2) {
    task_queue.push_back(ExpandOther);
    root->expanded_other_pv.push_back(root->pv().moves);
    statistics.extended++;
    Logging::info("*extend other " + to_s(elapsed) + ' '
		  + root->toCSA(root->bestMove())
		  + " preserving " + root->toCSA(best_move2));
  } else {
    static SearchNode *last_msg = 0;
    if (last_msg != root.get()) {
      Logging::info("*extend other for "+root->toCSA(root->bestMove())
		    + " too few slaves " +to_s(nslaves));
      last_msg = root.get();
    }
  }
}

void gpsshogi::SearchTree::restartOther(SearchNodePtr node)
{
  assert(node->isLeaf() && node->parent_move == "other");
  UsiSlavePtr slave = node->leaf.working[0];
  if (!node->parent || (!slave && !node->hasScheduledSlave())) {
    Logging::error("*cannot restart "+node->path());
    return;
  }
  if (node->hasScheduledSlave()) {
    Logging::info("*restart already scheduled "+node->path()+" "
		  +to_s(node->scheduledSlave()));
    return;
  }
  stopSubTree(*node);
  scheduleOther(node->parent->shared_from_this(), slave);
}

void gpsshogi::SearchTree::
scheduleOther(SearchNodePtr node, UsiSlavePtr slave)
{
  Logging::info("*schedule restart " + node->path() + slave->idMark());
  restart_other_waiting[slave->id()] = node;
  node->successorOther()->setScheduledSlave(slave->id());
}

void gpsshogi::SearchTree::
addReady(UsiSlavePtr slave)
{  
  stopped_slave.erase(slave->id());
  if (slave->error()) {
    Logging::error("*not add error slave" + slave->idMark());
    restart_other_waiting.erase(slave->id());
    return;
  }
  if (restart_other_waiting.count(slave->id())) {
    SearchNode& node = *restart_other_waiting[slave->id()]->successorOther(); // adjust ignore_moves up to date
    Logging::info("*restarted "+node.path() + slave->idMark());
    if (node.scheduledSlave() != slave->id()) {
      Logging::error("*inconsistent restart "+to_s(node.scheduledSlave())
		     + " != " + to_s(slave->id()));
      if (node.hasScheduledSlave())
	restart_other_waiting.erase(node.scheduledSlave());
    }
    node.resetScheduledSlave();
    restart_other_waiting.erase(slave->id());
    goNode(slave, node, msec_max);
    return;
  }
  current_idle.insert(slave);
}

void gpsshogi::SearchTree::
expandOther()
{
  if (current_idle.size() <= 1) {
    Logging::notice("*panic extension cancelled");
    return;
  }
  assert(! root->expanded_other_pv.empty());
  assert(! root->expanded_other_pv.back().empty());
  const std::vector<std::string>& pv = root->expanded_other_pv.back();
  status = PanicTimeOther;
  searching().restarted.push_back(osl::clock::now());
  std::vector<UsiSlavePtr> slaves = idleSlaves();
  Logging::notice("*panic time extension for "
		  +root->toCSA(root->expanded_other_pv.back()[0])
		  +" by "+to_s(slaves.size()));
  const bool need_other = (root->moves.normal.size() > root->succ.size());
  // restart other before creation of n for stop consistency
  SearchNodePtr other = root->successorOther();
  if (need_other)
    restartOther(other); 
  else {
    stopSubTree(*other);
    root->succ.erase("other");
  }
  // expand n
  SearchNode& n = *root->successor(pv[0]);
  if (n.pv().empty() && n.leaf.pv_hint.empty()) 
    Logging::error("extend: pv migration failed");
  std::string msg = "*extended "+pv[0]+" nodecount "+to_s(n.nodeCount())
    +" pv "+to_s(n.pv().size()) + ' '+n.report().joinPV()
    +" pvhint "+to_s(n.leaf.pv_hint.size());
  Logging::info(msg);
  probeAndSplit(n, slaves);
}


// ponder related methods
void gpsshogi::SearchTree::
growTree(bool in_thinking)
{
  double started = osl::elapsedSeconds(searching().started);
  Logging::info("*grow tree status="+to_s(status)
		+" idle "+to_s(current_idle.size())
		+" mateidle "+to_s(mate_idle.size())
		+" stopping "+to_s(stopped_slave.size()));
  if (2*current_idle.size()+2 < stopped_slave.size()) {
    task_queue.push_front(GrowTree); // note: must try again before Think
    return;
  }
  if (! in_thinking) {
    if (status != Thinking || status != PanicTimeOther)
      status = Pondering1;
    else 
      Logging::error("race between think and grow?");
  }
  std::vector<UsiSlavePtr> slaves = idleSlaves();
  if (!slaves.empty()) 
    growTree(*root, slaves);
  else {
    Logging::info("grow cancelled, no slaves available");
    if (! root->hasWorker())
      Logging::warn("root has not workers");
  }
  for (size_t i=0; i<task_queue.size(); ++i) {
    if (task_queue[i] == Think) {
      task_queue.erase(task_queue.begin()+i);
      status = Thinking;
      Logging::info("*cancelled Think after grow tree");	
      break;
    }
  }

  logStatus();
  const osl::time_point now = osl::clock::now();
  last_report = now;
  double finished = osl::elapsedSeconds(searching().started);
  Logging::info("*grow tree done "
		+to_s(round(1000*(finished - started)))+"ms."
		+" ( "+to_s(round(1000*started))
		+" "+to_s(round(1000*finished))+" )");
}

void gpsshogi::SearchTree::
growTree(SearchNode& node, std::vector<UsiSlavePtr> slaves)
{
  if (! stopped_slave.empty()) {
    std::vector<UsiSlavePtr> more_slaves = idleSlaves();
    slaves.insert(slaves.end(), more_slaves.begin(), more_slaves.end());
    std::stable_sort(slaves.begin(), slaves.end(), NpsCompare());
    slaves.erase(std::unique(slaves.begin(), slaves.end()), slaves.end());
  }
  if (node.moves.empty())
    node.generateMoves();
  {
    std::string msg = "*grow " + node.path() + " #";
    for (UsiSlavePtr slave: slaves)
      msg += " "+to_s(slave->id());
    if (node.isSolved()) 
      msg += " already solved";
    if (node.moves.empty())
      msg += " no legal moves";
    Logging::info(msg);
  }
  if (node.isSolved()) {
    Logging::info("*no need for search");
    current_idle.insert(slaves.begin(), slaves.end());
    return;
  }
  // special case: pondering at root with not so many slaves
  // focus on best prediction to save time
  if (&node == root.get()
      // && msecMax() > msec()
      && root->turn() != searching().config.my_turn
      && slaves.size() < 8
      && root->findMove(root->bestMove()).isNormal()) {
    std::string best_move = root->bestMove();
    Logging::info("grow bestmove@root "+best_move
		  +" #slaves " +to_s(slaves.size()));
    if (root->hasLeafWorker()) {
      if (root->probe.onGoing()) {
	for (UsiSlavePtr slave: slaves)
	  root->probe.split_waiting.push_back(slave);
	return;
      }
      UsiSlavePtr slave = root->leaf.working[0];
      stopSubTree(*root);
      root->prepareSplit();
      scheduleOther(root, slave);
    }
    else {
      if (root->hasOther()
	  && root->successorOther()->hasLeafWorker())
	restartOther(root->successorOther());
    }
    growTree(*root->successor(root->bestMove()), slaves);
    return;
  }

  if (! node.hasWorker()) {
    Logging::info("grow for no worker");
    stopSubTree(node);     // may be in probe
    probeAndSplit(node, slaves);
    return;
  }
  if (node.isLeaf() && node.leaf.working[0] && slaves.size() == 1
      && node.hasNormalPV()) {
    PVInfo pv = node.pv();
    UsiSlavePtr slave = node.leaf.working[0];
    Logging::info("*grow leaf single" + slave->idMark()
		  + slaves[0]->idMark() + node.toCSA(pv[0]));
    stopSubTree(node);
    node.prepareSplit();
    SearchNode& n = *node.successor(pv[0]);
    goNode(slaves[0], n, msec_max);
    scheduleOther(node.shared_from_this(), slave);
    return;
  }
  if (node.isLeaf() || node.hasLeafWorker()) {
    std::string msg = "*grow leaf " + node.path();
    PVInfo pv = node.pv();
    if (node.probe.onGoing()) {
      if (! pv.empty() && node.succ.count(pv[0])
	  && (node.succ[pv[0]]->committed
	      || node.succ[pv[0]]->probe.onGoing())) {
	// we have both probing slaves and commited nodes (early split)
	std::vector<UsiSlavePtr> half;
	if (slaves.size() < 3)
	  half.swap(slaves);
	else {
	  int width = std::min(3, splitWidth(node.depth8()));
	  for (size_t i=0; i<slaves.size(); i+=width)
	    half.push_back(slaves[i]); 
	  for (UsiSlavePtr slave: half)
	    std::remove(slaves.begin(), slaves.end(), slave);
	  slaves.resize(slaves.size()-half.size());
	}
	growTree(*node.succ[pv[0]], half);
	msg += " assign "+pv[0]+" "+to_s(half.size());
	// fall through	
      }
      Logging::info(msg+" merged into split_waiting "+to_s(slaves.size())
		    +" => "+to_s(node.probe.onGoingCount()));
      for (UsiSlavePtr slave: slaves) {
	scheduled_slave.insert(slave->id());
	node.probe.split_waiting.push_back(slave);
      }
      return;
    }
    if (node.leaf.working[0]) {
      msg += node.leaf.working[0]->idMark();
      if (! node.pv().empty())
	msg += " "+node.toCSA(node.pv()[0]);
    }
    Logging::info(msg);
    stopSubTree(node);
    probeAndSplit(node, slaves);
    return;
  }

  std::vector<int> evals;
  std::vector<std::string> moves = node.sortBySearchValue(&evals);
  std::string other_move = "";
  if (node.hasOther()) {
    if (node.succ["other"]->pv().size()) {
      other_move = node.succ["other"]->pv()[0];
      if (std::count(moves.begin(), moves.end(), other_move) == 0)
	other_move = "";
    }
    else if (node.succ["other"]->leaf.working[0]) {
      // just after search started?
      std::vector<UsiSlavePtr> half, half2;
      divideSlaves(slaves, half, half2);
      slaves.swap(half);
      current_idle.insert(half2.begin(), half2.end()); // reserve for future extension
    }
  }
  const int other_rank = 
    std::find(moves.begin(), moves.end(), other_move)-moves.begin();
  int width = std::min((int)moves.size(), splitWidth(node.depth8()));
  if (node.isPrimaryTakeBack(node.bestMove()))
    width = std::min(width, 2);
  else if (node.isTakeBack(node.bestMove()))
    width = std::min(width, node.take_back_moves);

  if (node.parent || node.turn() == searching().config.my_turn) {
    int margin = std::min(350*usi_pawn_value/100,
			  grow_tree_margin(slaves.size()));
    for (int i=2; i<width; ++i) {
      if (abs(evals[0] - evals[i]) > margin) {
	if (! node.parent)
	  Logging::info("*narrowing grow width "+to_s(width)+" to "+to_s(i)
			+" at "+node.path());
	width = i;
	break;
      }
    }   
  }

  if (other_rank < width) {
    if (node.hasMove(other_move) && node.succ[other_move]->hasWorker()) {
      // "other" move has already been extended.
      // no need to expand, other node seems to have too much value
      Logging::info("not expand other "+node.toCSA(other_move));
      node.succ["other"]->main.pv.score = -node.winValue();
    }
    else if (node.succ["other"]->isSolved()) {
      Logging::info("ignore solved other "+node.toCSA(other_move));
    }
    else {
      Logging::info("expand other "+node.toCSA(other_move)+" r="+to_s(other_rank));
      if (moves.size() == 1) {
	growTreeOther(node, other_move, slaves);
	return;
      }
      std::vector<UsiSlavePtr> half, half2;
      divideSlaves(slaves, half, half2);
      growTreeOther(node, other_move, half);
      moves.erase(std::remove(moves.begin(), moves.end(), other_move),
		  moves.end());
      slaves.swap(half2);
      // fall through
    }
    // fall through
  }
  if (slaves.empty())
    return;
  if (moves.size() == 1
      || (node.hasMove(moves[1]) && node.succ[moves[1]]->isSolved())) {
    growTree(*node.successor(moves[0]), slaves);
    return;
  }
  std::vector<UsiSlavePtr> half, half2;
  divideSlaves(slaves, half, half2);
  growTree(*node.successor(moves[0]), half);
  if (! half2.empty())
    growTree(*node.successor(moves[1]), half2);

  // restart other and increase MultiPV when pondering at root
  if (&node == root.get()
      && root->turn() != searching().config.my_turn) {
    if (root->hasOther()
	&& root->successorOther()->hasLeafWorker())
      restartOther(root->successorOther());
  }
}

void gpsshogi::SearchTree::
divideSlaves(const std::vector<UsiSlavePtr>& all,
	       std::vector<UsiSlavePtr>& out0,
	       std::vector<UsiSlavePtr>& out1) 
{
  if (all.size() <= 3) {
    out0 = all;
    return;
  }
  for (size_t i=0; i<all.size(); ++i)
    (i%2 ? out1 : out0).push_back(all[i]);
}

void gpsshogi::SearchTree::
growTreeOther(SearchNode& node, const std::string& move,
	      std::vector<UsiSlavePtr> slaves)
{
  const bool need_other = (node.moves.normal.size() > node.succ.size());

  std::string msg = "*grow other "+node.path()+' '+node.toCSA(move);
  Logging::info(msg);
  SearchNodePtr other = node.successorOther();
  if (need_other)
    restartOther(other);
  else {
    stopSubTree(*other);
    node.succ.erase("other");
  }
  SearchNode& n = *node.successor(move);
  probeAndSplit(n, slaves);
}

void gpsshogi::SearchTree::
moveRoot(const std::string& move, 
	 const std::vector<UsiSlavePtr>& idles)
{
  Logging::info("*set new root for " + root->toCSA(move) + " ponder " + to_s(ponder)
		+ " my_turn " + to_s(searching().config.my_turn));
  if (ponder) {
    for (size_t i=0; i<task_queue.size(); ++i) {
      if (task_queue[i] == GrowTree) {
	task_queue.erase(task_queue.begin()+i);
	Logging::info("*cancelled GrowTree by next moveRoot "+root->toCSA(move));
	break;
      }
    }
  }

  if (! root) {
    Logging::notice("*no root for moveRoot");
    return;
  }  
  // stop if needed
  if (root->hasLeafWorker()) {
    Logging::info("*stop root in moveRoot");
    stopSubTree(*root);
  }
  if (ponder) {
    typedef successor_table_t::value_type value_type;
    for (value_type& child: root->succ)
      if (child.first != move)
	stopSubTree(*child.second);
    task_queue.push_back(WaitSlave);
    task_queue.push_back(WaitSlave);
    task_queue.push_back(WaitSlave);
  }
  for (UsiSlavePtr slave: idles)
    current_idle.insert(slave);

  if (root->hasScheduledSlave()) {
    Logging::info("*cancel restart "+root->path());
    restart_other_waiting.erase(root->scheduledSlave());
    root->resetScheduledSlave();
  }

  // make new root (note: stop "other" before possibly creating new node)
  SearchNodePtr new_root = root->successor(move);
  if (new_root->isSolved())
    Logging::notice("root solved");
  else if (new_root->nodeCount() == 0 && root->nodeCount() > 0)
    Logging::notice("*not expected move "+to_s(state.moves.size()+1)
		    +"th "+root->toCSA(move)
		    +" pv "+to_s
		    (std::max(new_root->pv().size(),
			      new_root->leaf.pv_hint.size())));


  // log tree (status of opponent move prediction)
  std::string filename = "";
  if (searching().position_id >= 0
      && root->turn() != searching().config.my_turn) {
    const std::string last_move_csa = root->toCSA(move);
    filename = treeFileName("opp", state.moves.size()+1, last_move_csa);
  }
  if (filename != "") {
    logStatus(filename);
    std::ofstream os((Logging::path()+filename).c_str(), std::ios::app);
    os << state.currentState();
  }

  // set up new root
  state.moves.push_back(root->findMove(move));
  assert(state.usiString() == new_root->position);
  new_root->parent = 0;
  new_root->parent_move = "";
  if (new_root->moves.empty())
    new_root->generateMoves();
  root = new_root;

  if (filename == "")
    logStatus();

  const osl::time_point now = osl::clock::now();
  bool time_management = searching().position_id >= 0
    && msec() < msecMax()
    && root->turn() == searching().config.my_turn;
  bool ponder_success = time_management
    && osl::msec(now-root->lastTreeChangeForBestMove()) > msec()*3/4
    && root->betterThan(root->value()+usi_pawn_value/2, prev().value)
    && root->nodeCountBestMove() > recentNodeCountBest()
    && ! root->bestMoveIsOther();
  if (time_management
      && root->moves.normal.size() == 1 && root->moves.draw.empty()
      && root->bestMove() == root->moves.normal[0]) {
    Logging::notice("one reply at root " + root->toCSA(root->bestMove()));
    ponder_success = true;
  }
  if (ponder_success) {
    Logging::notice("ponder sufficient " + root->toCSA(root->bestMove())
		    +" "+to_s(root->nodeCountBestMove())
		    +" "+to_s(recentNodeCountBest()));
    searching().sufficient_ponder = root->bestMove();
  }

  Logging::setPrefix(to_s(state.moves.size()+1)+"th");
  if (ponder && (stopped_slave.size()+current_idle.size())>1
      && searching().sufficient_ponder == "")
    task_queue.push_back(GrowTree);
  {
    std::ostringstream ss;
    if (! state.moves.empty())
      ss << "last move "
	 << to_s(state.moves.size()+1) << "th "
	 << osl::csa::show(state.moves.back());
    ss << " other/extended/prepared/search " << statistics.selected_other
       << "/" << statistics.extended << "/" << statistics.prepared
       << "/" << statistics.values.size()
       << uptimeString();
    ss << "\n" << state.currentState();
    ss << "stopping " << stopped_slave.size()
       << " idle " << current_idle.size()
       << " mateidle " << mate_idle.size()
       << " status " << to_s(status);
    Logging::info(ss.str());
  }
}

const std::string gpsshogi::SearchTree::
uptimeString()
{
  std::ifstream is("/proc/loadavg");
  std::string line;
  if (is && getline(is, line))
    return " uptime "+line;
  return "";
}

int gpsshogi::SearchTree::
splitWidth(int depth) const
{
  if (all_slave.size() < 8)
    return split_width;
  
  int adjust = 0;
  if (all_slave.size() < 12)
    adjust = 3;
  else if (all_slave.size() < 16)
    adjust = 2;
  else if (all_slave.size() < 32)
    adjust = 1;

  if (depth + adjust == 0)
    return split_width+7;
  if (depth + adjust <= 1)
    return split_width+7;
  if (depth + adjust <= 2)
    return split_width+4;
  if (depth + adjust <= 3)
    return split_width+1;
  return split_width;
}

void gpsshogi::SearchTree::
adjustSplitWidth(int active, int msec)
{
  if (msec <= 2000) 
    split_width = std::max(SplitWidthDefault+1, (int)ceil(sqrt(active)));
  else if (msec <= 8000) 
    split_width = std::max(SplitWidthDefault, (int)ceil(sqrt(sqrt(active))));
  else
    split_width = SplitWidthDefault;
}

void gpsshogi::SearchTree::setHumanRounding(bool enable) { 
  if (enable)
    human_rounding = 55; 
  else
    human_rounding = 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
