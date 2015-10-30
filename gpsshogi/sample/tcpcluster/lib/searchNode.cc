/* searchNode.cc
 */
#include "searchNode.h"
#include "searchTree.h"
#include "usiSlave.h"
#include "usiStatus.h"
#include "logging.h"
#include "osl/eval/evalTraits.h"
#include "osl/usi.h"
#include "osl/csa.h"
#include "osl/game_playing/usiResponse.h"
#include "osl/game_playing/gameState.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/enterKing.h"
#include "osl/eval/pieceEval.h"
#include "osl/progress.h"
#include "osl/move_probability/featureSet.h"
#include "osl/sennichite.h"
#include "osl/enter_king/simplePredictor.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
using boost::algorithm::join;

const int accept_other_margin = 35*gpsshogi::usi_pawn_value/100;

namespace gpsshogi 
{
  using namespace osl;
  int enterKingBonus24(const NumEffectState& state) {
    static enter_king::SimplePredictor Predictor;
    static const int enter_king_prob_weight
      = PieceEval::value(newPtypeO(BLACK, ROOK));
    static const int enter_king_major_weight
      = PieceEval::value(newPtypeO(BLACK, SILVER));
    static const int pawn
      = PieceEval::value(newPtypeO(BLACK, PAWN));
    double pb = Predictor.getProbability<BLACK>(state);
    double pw = Predictor.getProbability<WHITE>(state);

    Square kb = state.kingSquare(BLACK);
    Square kw = state.kingSquare(WHITE).rotate180();
    if (kb.y() > 5)      pb = 0.0;
    else if (kb.y() > 3) pb *= (7-kb.y())/4.0;
    if (kw.y() > 5)      pw = 0.0;
    else if (kw.y() > 3) pw *= (7-kw.y())/4.0;

    if (pb < 0.5 && pw < 0.5)
      return 0;
    if (pb < 0.5)
      pb = 0.0;
    if (pw < 0.5)
      pw = 0.0;

    int black_major_count = 0;
    for (int i=0; i<state.nthLimit<ROOK>(); ++i) {
      if (state.nth<ROOK>(i).owner() == BLACK)
	++black_major_count;
    }
    for (int i=0; i<state.nthLimit<BISHOP>(); ++i) {
      if (state.nth<BISHOP>(i).owner() == BLACK)
	++black_major_count;
    }
    return 100.0/pawn
      * (((pb - pw) * enter_king_prob_weight) // *NOT* relative value for turn
	 + (black_major_count - 2) * enter_king_major_weight);
  }

  int enter_king_bonus_csa(const NumEffectState& state) {
    static enter_king::SimplePredictor Predictor;
    static const int enter_king_weight = PieceEval::value(newPtypeO(BLACK, ROOK));
    double pb = Predictor.getProbability27<BLACK>(state);
    double pw = Predictor.getProbability27<WHITE>(state);

    Square kb = state.kingSquare(BLACK);
    Square kw = state.kingSquare(WHITE).rotate180();
    if (kb.y() > 5)      pb = 0.0;
    else if (kb.y() > 3) pb *= (7-kb.y())/4.0;
    if (kw.y() > 5)      pw = 0.0;
    else if (kw.y() > 3) pw *= (7-kw.y())/4.0;

    if (pb < 0.25 && pw < 0.25) return 0;
    if (pb < 0.25) pb = 0.0;
    if (pw < 0.25) pw = 0.0;

    static const int enter_king_major_weight =
      PieceEval::value(newPtypeO(BLACK,SILVER));

    int black_major_count = 0;
    for (int i=0; i<state.nthLimit<ROOK>(); ++i) {
      if (state.nth<ROOK>(i).owner() == BLACK)
	++black_major_count;
    }
    for (int i=0; i<state.nthLimit<BISHOP>(); ++i) {
      if (state.nth<BISHOP>(i).owner() == BLACK)
	++black_major_count;
    }

    return (int)((pb - pw) * enter_king_weight
		 + (std::max(pb,pw)
		    * (black_major_count - 2)
		    * enter_king_major_weight))
      / 2;
  }

}

void gpsshogi::ProbeData::
addFinish(const UsiSlavePtr& slave, const InterimReport& report)
{
  assert(probing.count(slave->id()));
  probing.erase(slave->id());
  if (! report.aborted)
    split_waiting.push_back(slave);
}    

const std::string gpsshogi::ProbeData::
status() const
{
  std::string msg 
    = to_s(probing.size()) + "/"+to_s(split_waiting.size());
  int number = 0;
  for (int id: probing) {
    if (++number > 10) {
      msg += "...";
      break;
    }
    msg += " " + to_s(id);
  }
  return msg;
}


int gpsshogi::SearchNode::draw_value = 0;

gpsshogi::SearchNode::
SearchNode(const std::string& po, const osl::HashKey& k,
	   SearchNode *pa, const std::string& pm, const std::string& i)
  : position(po), ignore_moves(i), key(k), parent(pa), parent_move(pm),
    scheduled_slave(-1), heuristic_cost(0), has_workers(false),
    mate_tested(false), committed(false), take_back_moves(0)
{
}

gpsshogi::SearchNode::
~SearchNode()
{
  typedef successor_table_t::value_type value_type;
  for (value_type& c: succ)
    if (c.second) {
      if (c.second->parent && c.second->parent != this)
	Logging::error("parent inconsistent "+path()+" -- "+c.first);
      c.second->parent = 0;
    }
}

osl::Move gpsshogi::SearchNode::findMove(const std::string& move) const
{
  const size_t i = std::find(moves.usi.begin(), moves.usi.end(), move)
    - moves.usi.begin();
  if (i >= moves.usi.size())
    return osl::Move();
  return moves.all[i];
}

gpsshogi::SearchNodePtr gpsshogi::SearchNode::
successor(const std::string& move)
{
  osl::Move osl_move = findMove(move);
  if (osl_move.isInvalid()) {
    Logging::error("| "+path()+" no osl move " + move);
    assert(0);
  }
  assert(! osl_move.isInvalid()); // not always isNormal e.g., %KACHI
  std::shared_ptr<SearchNode>& node = succ[move];
  if (! node) {
    if (probe.probed.count(move)) 
      node = probe.probed[move];
    else {
      node.reset(new SearchNode(append(position, move),
				key.newMakeMove(osl_move), this, move));
      // migration of pv_hint
      if (! leaf.pv_hint.empty() && leaf.pv_hint[0] == move) {
	node->leaf.pv_hint = leaf.pv_hint;
	node->leaf.pv_hint.erase(node->leaf.pv_hint.begin());
      }
      if (pv().size() > 1 && pv()[0] == move) {
	node->leaf.pv_hint = pv().moves;
	node->leaf.pv_hint.erase(node->leaf.pv_hint.begin());
	double elapsed = std::max(pv().elapsed, report().elapsed);
	int hint_max = (elapsed > 4*probe_msec/1000.0) ? 4 : 2;
	if (node->leaf.pv_hint.size() > hint_max)
	  node->leaf.pv_hint.resize(hint_max);
	if (node->leaf.pv_hint.size() >= 4)
	  Logging::info("|set long pv_hint at "+path()+" for "+toCSA(move)
			+ " " +to_s(round(elapsed*2)/2.0) + "s "
			+ boost::algorithm::join(node->leaf.pv_hint, " "));
      }
      if (node->leaf.pv_hint.empty()) {
	PVInfo new_pv = findAlternative(move);
	if (! new_pv.empty() && new_pv[0] != move) {
	  Logging::error("|error1 at " + path() + " " + move + "!="+new_pv[0]);
	  new_pv.clear();
	}
	if (new_pv.empty()) {
	  new_pv = findAlternativeInOther(move);
	  if (! new_pv.empty() && new_pv[0] != move) {
	    Logging::error("|error2 at " + path() + " " + move
			   + "!="+new_pv[0]);
	    new_pv.clear();
	  }	  
	}
	if (! new_pv.empty()) {
	  if (new_pv[0] == move) {
	    new_pv.moves.erase(new_pv.moves.begin());
	    int hint_max = (new_pv.elapsed > 4*probe_msec/1000.0) ? 4 : 2;
	    if (new_pv.size() > hint_max)
	      new_pv.moves.resize(hint_max);
	    Logging::info("|migration from subpv at "+path()+" for "+move
			  + " " +to_s(round(new_pv.elapsed*2)/2.0) + "s "
			  + boost::algorithm::join(new_pv.moves, " "));
	    node->leaf.pv_hint = new_pv.moves;
	  }
	}
      }
      // migration from "other" data
      bool migration_from_other = false;
      if (succ.count("other")) {
	SearchNodePtr other = succ["other"];
	if (other->bestMove() == move) {
	  node->sub = other->report();
	  migration_from_other = true;
	}
      }
      else if (probe.probed.count("other")) {
	SearchNode& other = *probe.probed["other"];
	if (!other.pv().empty() && other.pv()[0] == move) {
	  node->sub = other.report();
	  migration_from_other = true;
	}
      }
      if (migration_from_other) {
	std::vector<std::string>& pv = node->sub.pv.moves;
	if (! pv.empty())
	  pv.erase(pv.begin());
	node->sub.alternatives.clear();
	node->sub.node_count /= 2;
      }
    }
  }
  return node;
}

gpsshogi::SearchNodePtr gpsshogi::SearchNode::
successorOther() 
{
  typedef successor_table_t::value_type value_type;
  std::string ignore_moves = "";
  for (value_type& v: succ) 
    if (v.first != "other")
      ignore_moves += " "+v.first;
  std::shared_ptr<SearchNode>& node = succ["other"];
  if (! node) 
    node.reset(new SearchNode(position, key, this, "other", ignore_moves));
  else if (node->ignore_moves != ignore_moves) {
    Logging::info("|adjust other " + path() + ' ' +toCSA(node->ignore_moves)
		  + " => " + toCSA(ignore_moves));
    if (node->leaf.working[0])
      Logging::error("|adjust other in working"
		     +node->leaf.working[0]->idMark());
    node->ignore_moves = ignore_moves;
    // node->sub = node->main;
    node->main = InterimReport();
  }
  return node;
}

void gpsshogi::SearchNode::
prepareProbe(slave_set_t& stopped) 
{
  if (hasWorker()) {
    slave_set_t worked, restart_cancelled, waited;
    stopSubTree(worked, restart_cancelled, waited);
    std::string msg = "|conflict in probe " + path();
    for (int id: worked)
      msg += " "+to_s(id);
    Logging::error(msg+" restartcancel "+to_s(restart_cancelled.size())+" "+to_s(waited.size()));
    stopped.insert(worked.begin(), worked.end());
  }
  probe.clear();
  probe.start = osl::clock::now();

  if (moves.empty())
    generateMoves();  
  if (succ.empty())
    probe.target = moves.normal;
  else {
    std::string ignored="";
    probe.target.reserve(moves.normal.size());
    for (const std::string& move: moves.normal)
      if (! hasMove(move))
	probe.target.push_back(move);
      else
	ignored += " "+move;
    Logging::info("|prepareProbe " + path() + " with children #"
		  + to_s(succ.size()) + ignored);    
  }
  if (! leaf.pv_hint.empty()) {
    std::string first = leaf.pv_hint[0];
    if (findMove(first).isNormal()) {
      std::vector<std::string>::iterator p
	= std::find(probe.target.begin(), probe.target.end(), first);
      if (p != probe.target.end()) {
	std::rotate(probe.target.begin(), p, p+1);
	if (probe.target[0] != first)
	  Logging::error("rotate failed "+first);
      }
    }
  }
}

gpsshogi::SearchNode *gpsshogi::SearchNode::
findLeaf(const std::string& position)
{
  SearchNode *ret = find(position);
  if (ret && ! ret->isLeaf()) {
    Logging::error("|findLeaf failed " + position + " in "
		   + ret->position + "ignore_moves " + ret->ignore_moves);
    return 0;
  }
  return ret;
}

gpsshogi::SearchNode * gpsshogi::SearchNode::
find(const std::string& target)
{
  if (target == position) 
    return this;
  if (target.find(position) != 0
      || target.size() < position.size()+1)
    return 0;
  std::string rest = target.substr(position.size()+1);
  boost::algorithm::trim(rest);
  boost::algorithm::trim_left(rest);
  std::vector<std::string> moves;
  boost::algorithm::split(moves, rest,
			  boost::algorithm::is_any_of(" "));
  if (moves.empty())
    return 0;
  size_t cur = 0;
  if (moves[0] == "moves")
    ++cur;
  if (cur >= moves.size() || ! hasMove(moves[cur]))
    return 0;
  return succ[moves[cur]]->find(moves, cur+1);
}

gpsshogi::SearchNode * gpsshogi::SearchNode::
find(const std::vector<std::string>& moves, size_t cur)
{
  if (moves.size() == cur)
    return this;
  successor_table_t::iterator p = succ.find(moves[cur]);
  if (p == succ.end())
    return 0;
  return p->second->find(moves, cur+1);
}

std::string gpsshogi::SearchNode::
append(const std::string& position, std::string move)
{
  std::string ret = position;
  boost::algorithm::trim(ret);
  if (ret.find("moves") == ret.npos)
    ret += " moves";
  ret += " " + move;
  return ret;
}

bool gpsshogi::SearchNode::
update(std::function<void(std::string)> progress)
{
  if (! parent) {
    progress(report().composeInfo(key.isPlayerOfTurn(osl::WHITE)));
    return true;
  }
  return parent->update(parent_move, report(), progress);
}

uint64_t gpsshogi::SearchNode::
recalculateNodeCount() const
{
  uint64_t sum = 0;
  typedef successor_table_t::value_type pair_t;
  for (const pair_t& v: succ) 
    sum += v.second->nodeCount();;
  return sum;
}

bool gpsshogi::SearchNode::
betterThan(int a, int b) const
{
  return osl::eval::betterThan(turn(), a, b);
}

int gpsshogi::SearchNode::
infty() const
{
  return osl::eval::EvalTraits<osl::BLACK>::MAX_VALUE*sign();
}
int gpsshogi::SearchNode::
winValue(int mate_length) const
{
  if (mate_length > 100)
    mate_length = 100;
  return (usi_win_value-mate_length)*sign();
}

bool gpsshogi::SearchNode::
update(const std::string& move, const InterimReport& child,
       std::function<void(std::string)> progress)
{
  main.node_count = recalculateNodeCount();
  main.pv.elapsed = main.elapsed = std::max(main.elapsed, child.elapsed);
  if (&report() == &main && ! pv().empty() && pv()[0] != move
      && move != "other"
      && ! isWinValue(abs(child.bestValue())) // to determine solved
      && ! betterThan(child.bestValue(), value()))
    return false;
  const int old_value = value();
  std::string best_move = "", old_bestmove = bestMove();
  main.pv.score = -infty();
  if (! moves.draw.empty()) {
    main.pv.score = draw_value;
    best_move = *moves.draw.begin();
  }
  main.pv.clear();
  // find maximum value among children
  bool forced_win = false, solved_all = !probe.onGoing();
  typedef successor_table_t::value_type pair_t;
  uint64_t best_node_count = 0;
  for (const pair_t& v: succ) {
    if (! v.second->isSolved())
      solved_all = false;
    if (v.second->pv().empty()
	|| ! (betterThan(v.second->value(), main.pv.score)
	      || (v.second->value() == main.pv.score
		  && v.second->nodeCount() > best_node_count)))
      continue;
    if (v.first == "other"
	&& best_move != ""
	&& abs(main.pv.score - v.second->value()) < accept_other_margin) {
      if (! parent) 
	Logging::info("rejected other "+best_move+" "+to_s(main.pv.score)
		      +" v.s. other "
		      +(v.second->pv().empty() ? "" : v.second->pv()[0])
		      +" "+to_s(v.second->value()));
      continue;
    }
    main.pv.score = v.second->value();
    main.pv.depth = v.second->pvDepth();
    main.depth_head
      = v.second->isLeaf() ? 1 : (v.second->depthHead()+1);
    best_move = v.first;
    best_node_count = v.second->nodeCount();
    if (v.second->isSolved() && isWinValue(main.pv.score)) {
      forced_win = true;
      solved_all = false;
      break;
    }
  }
  // retrieve pv
  if (best_move != "") {
    if (succ.count(best_move))
      main.pv.moves = succ[best_move]->pv().moves;
    if (best_move != "other")
      main.pv.moves.insert(main.pv.moves.begin(), best_move);
  }
  // solved?
  if (forced_win || solved_all)
    solved = main;

  // update parent
  const int changed = value() - old_value;
  if (parent) 
    return changed && parent->update(parent_move, report(), progress);

  if (! moves.draw.empty() && main.pv.score == draw_value
      && best_move == *moves.draw.begin())
    Logging::info("|"+path()+" preferred draw " + best_move
		  + " " + to_s(draw_value));

  // root
  if (probe.start > split_time)
    return false;
  const int report_threshold = usi_pawn_value/2;
  if (old_bestmove != bestMove() || abs(changed) > report_threshold || isSolved()) {
    progress(report().composeInfo(key.isPlayerOfTurn(osl::WHITE)));
    return true;
  }
  return false;
}

void gpsshogi::SearchNode::
propagateTreeChangeTime()
{
  SearchNode *node = this->parent;
  while (node) {
    node->last_tree_change = this->last_tree_change;
    node = node->parent;
  }
}

void gpsshogi::SearchNode::
prepareSplit()
{
  committed = true;
  setWorking();
  if (main.node_count < sub.node_count/2)
    Logging::info("|"+path()+" node_count main < sub " + to_s(main.node_count)
		  + " " + to_s(sub.node_count));
  sub = main;
  main = InterimReport();
  last_tree_change = split_time = osl::clock::now();
  propagateTreeChangeTime();

  if (! probe.probed.empty()) {
    std::string msg = "";
    typedef successor_table_t::value_type pair_t;
    for (pair_t& v: probe.probed)
      msg += " "+v.first;
    Logging::info("|has past probe "+path()+msg);
  }
  std::string added = "";
  for (const std::string& move: probe.target) {
    if (hasMove(move)) {
      SearchNodePtr node = succ[move];
      if (node->leaf.working[0]) {
	Logging::warn("|inconsistent probe management"
		      + node->path() + " "+node->leaf.working[0]->idMark());
	continue;
      }
      if (node->leaf.working[1]) {
	Logging::info("|node moved when mate solver working"
		      + node->leaf.working[1]->idMark());
	node->leaf.working[1].reset();
      }
      probe.probed[move] = node;
      std::swap(node->main, node->sub);
      probe.probed[move]->main = InterimReport();
      if (succ.count(move)) {
	added += " "+move;
	succ.erase(move);
      }
    }
  }
  Logging::info("|erase from successor "+path()+added);
  if (hasOther()) {
    probe.probed["other"] = succ["other"];
    succ.erase("other");
  }
}

osl::Move gpsshogi::SearchNode::
parseMove(const osl::HistoryState& state, std::string usi_move)
{
  if (usi_move == "other")
    return osl::Move();
  return osl::usi::strToMove(usi_move, state.state());
}

std::string gpsshogi::SearchNode::toCSA(const std::string& str) const
{
  std::vector<std::string> moves;
  boost::algorithm::split(moves, str, boost::algorithm::is_any_of(" "));
  for (std::string& move: moves) {
    const osl::Move osl_move = findMove(move);
    if (osl_move.isNormal())
      move = osl::csa::show(osl_move);
  }
  return join(moves, "");
}

std::string gpsshogi::SearchNode::
toCSA(osl::HistoryState& state, std::string usi_moves_string,
      int limit)
{
  boost::algorithm::trim(usi_moves_string);
  boost::algorithm::trim_left(usi_moves_string);
  if (usi_moves_string == "")
    return "";
  std::vector<std::string> string_moves;
  boost::algorithm::split(string_moves, usi_moves_string,
			  boost::algorithm::is_any_of(" "));
  if (limit >= 0 && (int)string_moves.size() > limit)
    string_moves.resize(limit);
  std::vector<osl::Move> moves;
  moves.reserve(string_moves.size());
  bool error = false;
  for (const std::string& usi: string_moves) {
    try {
      osl::Move move = parseMove(state, usi);
      if (! move.isNormal() && ! move.isPass())
	break;
      if (move.isNormal() && ! state.state().isValidMove(move))
	throw std::runtime_error("not valid move "+to_s(move));
      moves.push_back(move);
      state.makeMove(move);      
    }
    catch (std::exception& e) {
      error = true;
      Logging::error("|toCSA failed "+usi_moves_string+" "+e.what()+" in \n"
		     +to_s(state.state()));
    }
  }
  for (osl::Move move: moves) 
    state.unmakeMove();
  const std::string (*f)(osl::Move) = osl::csa::show;
  std::transform(moves.begin(), moves.end(), string_moves.begin(), f);
  return join(string_moves, "")+(error ? "csaerror" : "");
}

std::vector<std::string> gpsshogi::SearchNode::
sortByProbe(std::vector<int> *evals) const 
{
  successor_table_t new_probed;
  std::string excluded = "";
  typedef successor_table_t::value_type pair_t;
  for (const pair_t& v: probe.probed) {
    if (! hasMove(v.first)) {
      new_probed[v.first] = v.second;
    }
    else {
      excluded += " "+v.first;
    }
  }
  if (new_probed.size() != probe.probed.size()) {
    Logging::info("|sortByProbe "+to_s(new_probed.size())
		  +" "+to_s(probe.probed.size())+" excluded "+excluded);
  }
  std::vector<std::string> result
    = sortByTable(new_probed, evals);
  for (std::string move: moves.normal) {
    if (! hasMove(move)
	&& std::count(result.begin(), result.end(), move) == 0) {
      result.push_back(move);
      if (evals)
	evals->push_back(0);
    }
  }
  return result;
}

std::vector<std::string> gpsshogi::SearchNode::
sortByTable(const successor_table_t& table, std::vector<int> *evals) const
{
  if (evals)
    evals->clear();
  std::vector<std::pair<int, std::string> > sorted;
  sorted.reserve(table.size());
  typedef successor_table_t::value_type pair_t;
  const int sign = this->sign();
  for (const pair_t& v: table) {
    const int neg_preference = -v.second->value()*sign;
    if (v.first == "other" && v.second->pv().empty()) {
      Logging::info("|"+path()+" skipped other of empty pv");
      continue;
    }
    const std::string move = v.first == "other"
      ? v.second->pv()[0] : v.first;
    if (move == "resign")
      continue;
    if (move == "pass" || move == "win") {
      Logging::error("|sortByTable unexpected move " + move + ' ' + v.first);
      continue;
    }
    sorted.push_back(std::make_pair(neg_preference, move));
  }
  std::sort(sorted.begin(), sorted.end());  
  std::vector<std::string> ret;
  ret.reserve(moves.usi.size());
  for (size_t i=0; i<sorted.size(); ++i) {    
    std::string move = sorted[i].second;
    if (std::count(ret.begin(), ret.end(), move))
      Logging::info("|"+path()+" duplicated moves "+move);
    else {
      ret.push_back(move);
      if (evals)
	evals->push_back(-sorted[i].first*sign);
    }
  }
  if (sorted.size() < 2)
    Logging::info("|sortByTable " + to_s(sorted.size()) + ' ' + to_s(ret.size())
		  +' '+to_s(moves.normal.size())+' '+to_s(moves.usi.size()));
  return ret;
}

void gpsshogi::SearchNode::
showStatus(std::ostream& os, osl::HistoryState& state,
	   osl::time_point now, int indent)
{
  assert(state.state().turn() == turn());
  const std::string spacer(indent*2, ' ');
  const InterimReport& data = report();
  os << spacer;
  if (parent_move.find("other") == 0)
    os << "others ";
  else if (parent_move != "")
    os << osl::csa::show(state.history().back());
  else
    os << "root";
  os << ' ' << turn() << ' '
     << std::setw(5) << data.bestValue()*100/usi_pawn_value;
  os << ' ' << toCSA(state, data.joinPV(), indent ? 6 : -1)
     << " (" << data.pv.size() << " " << data.depth_head << ")"
     << ' ' << nodeCount()/1000 << "k";
  // os << ' ' << main.node_count << ' ' << sub.node_count;
  if (data.owner >= 0)
    os << " [" << data.owner << "]";
  for (size_t i=0; i<leaf.working.size(); ++i)
    if (leaf.working[i])
      os << " w" << i << '=' << leaf.working[i]->id();
  if (heuristic_cost)
    os << " h=" << heuristic_cost;
  if (hasScheduledSlave())
    os << " res=" << scheduledSlave();
  if (probe.onGoing())
    os << " p " << probe.onGoingCount();
  if (!committed)
    os << " NC";
  if (&data == &sub)
    os << " sub";
  else if (&data == &solved)
    os << " solved";
  // seconds searched on this node
  if (! (split_time == osl::time_point()))
    os << " "
       << boost::format("%.1f") % osl::toSeconds(now - split_time)
       << "s";
  // stability of subtree
  if (! (last_tree_change == osl::time_point()))
    os << " ("
       << boost::format("%.1f") % osl::toSeconds(now - last_tree_change)
       << "s)";
  os << "\n";
  if (&data == &main && leaf.working[0] && leaf.working[0]->id() != data.owner)
    Logging::warn("|worker inconsistent "+path()+' '
		  +to_s(leaf.working[0]->id())+" != "+to_s(data.owner));

  if (parent && isSolved())
    return;
  // show sub-pv if any
  if (isLeaf()) {
    for (const pv_table::value_type& v: data.alternatives) {
      if (v.second.depth + 1 < data.depth_head)
	continue;
      os << spacer << "    alt " << turn() << ' '
	 << std::setw(5) << v.second.score << ' '
	 << toCSA(state, boost::algorithm::join(v.second.moves, " "), indent?4:-1)
	 << " (" << v.second.size()
	 << ' ' << v.second.depth << ")" << "\n";
    }
  }

  // recursively show subtree
  typedef std::pair<int,int64_t> key_t;
  typedef std::pair<key_t,SearchNodePtr> key_ptr_t;
  std::vector<key_ptr_t> sorted;
  sorted.reserve(succ.size());
  typedef successor_table_t::value_type pair_t;
  for (const pair_t& v: succ) {
    if (v.second->pv().empty())
      continue;
    const int neg_preference = -v.second->value()*sign();
    const key_t key(neg_preference, -v.second->nodeCount());
    sorted.push_back(std::make_pair(key, v.second));
  }
  std::sort(sorted.begin(), sorted.end());  
  for (const key_ptr_t& c: sorted) {
    try {
      std::string usi = c.second->parent_move;
      const osl::Move move = (usi == "other")
	? osl::Move() : parseMove(state, c.second->parent_move);
      if (move.isNormal()) {
	if (! state.state().isValidMove(move))
	  throw std::runtime_error("invalid move "+to_s(move));
	state.makeMove(move);
      }
      c.second->showStatus(os, state, now, indent+1);
      if (move.isNormal())
	state.unmakeMove();
    }
    catch (std::exception& e) {
      Logging::error("|showStatus tree corruption "+c.second->parent_move+" "+e.what()+" in \n"
		     +to_s(state.state()));
    }
  }
  assert(state.state().turn() == turn());
}

void gpsshogi::SearchNode::
stopSubTree(slave_set_t& stopped, slave_set_t& restart_cancelled,
	    slave_set_t& waited)
{
  has_workers = false;
  for (UsiSlavePtr& slave: leaf.working) {
    if (slave) {
      if (slave->stop(id()))	   // node id
	stopped.insert(slave->id()); // slave id
      else
	Logging::warn("|stop send failure"+slave->idMark());
      slave.reset();
    }
  }
  // clear data on this node
  stopProbe(waited);
  if (hasScheduledSlave()) {
    restart_cancelled.insert(scheduledSlave());
    resetScheduledSlave();
  }
  // recursively stop descendants
  typedef successor_table_t::value_type value_type;
  for (value_type& c: succ)
    if (c.second)
      c.second->stopSubTree(stopped, restart_cancelled, waited);
}

void gpsshogi::SearchNode::
stopProbe(slave_set_t& waited)
{
  const std::string path=this->path();
  std::vector<std::string> ps, pp;
  if (! probe.split_waiting.empty()) {
    for (UsiSlavePtr slave: probe.split_waiting) {
      waited.insert(slave->id());
      ps.push_back(to_s(slave->id()));
    }
    probe.split_waiting.clear();
  }
  if (! probe.probing.empty()) {
    for (int pid: probe.probing)
      pp.push_back(to_s(pid));
    probe.probing.clear();
  }
  if (! ps.empty() || ! pp.empty())
    Logging::info("| stop "+path+(ps.empty() ? "" : " split_waiting "+join(ps, " "))
		  +(pp.empty() ? "" : " probing "+ join(pp, " ")));
}

void gpsshogi::SearchNode::
generateMoves()
{
  if (! moves.empty()) {
    Logging::info("| moves already generated "+path());
    return;
  }
  osl::UsiState usi_state;
  usi_state.parseUsi(position);
  osl::game_playing::GameState gstate(usi_state.initial_state);
  for (osl::Move m: usi_state.moves) 
    gstate.pushMove(m);

  osl::MoveVector normal, win, draw, loss;
  gstate.generateMoves(normal, win, draw, loss);

  const osl::NumEffectState& state= gstate.state();
  const osl::MoveStack& history = gstate.moveHistory();
  osl::progress::ml::NewProgress progress(state);
  osl::MoveLogProbVector rmoves;

  const osl::move_probability::StandardFeatureSet& feature_set
    = osl::move_probability::StandardFeatureSet::instance();
  osl::Move prev = history.lastMove();
  osl::Move threatmate = osl::move_probability::StateInfo::findShortThreatmate
    (state, prev);
  osl::move_probability::StateInfo info(state, progress.progress16(),
					history, threatmate);
  feature_set.generateLogProb(info, rmoves);
  take_back_moves = 0;
  for (osl::MoveLogProb m: rmoves) {
    if (prev.isNormal() && m.move().to() == prev.to())
      ++take_back_moves;
    const std::string usi = osl::usi::show(m.move());
    moves.all.push_back(m.move());
    moves.usi.push_back(usi);
    if (std::count(win.begin(), win.end(), m.move()))
      moves.win.insert(usi);
    else if (std::count(draw.begin(), draw.end(), m.move()))
      moves.draw.insert(usi);
    else if (std::count(loss.begin(), loss.end(), m.move()))
      moves.loss.insert(usi);
    else {
      moves.normal.push_back(usi);
      assert(state.inCheck() || ! m.move().ignoreUnpromote());
      if (!state.inCheck() && m.move().hasIgnoredUnpromote()) {
	osl::Move nopromote = m.move().unpromote();
	std::string nopromote_usi = osl::usi::show(nopromote);
	moves.all.push_back(nopromote);
	moves.usi.push_back(nopromote_usi);
	moves.nopromote.insert(nopromote_usi);
      }
    }
  }

  if (! moves.draw.empty()) {
    Logging::info("|"+path()+" has draw move(s) " + *moves.draw.begin());
  }
  if (moves.normal.empty()) {
    Logging::info("|"+path()+" has no normal move");
    moves.normal.insert(moves.normal.end(),
			moves.loss.begin(), moves.loss.end());
    moves.normal.insert(moves.normal.end(),
			moves.nopromote.begin(), moves.nopromote.end());
  }
  // win by enter king?
  // Is there any simple winning move?
  if (SearchTree::humanRounding() == 0) {
    int drops_required;
    bool win_by_declaration = osl::EnterKing::canDeclareWin(state, drops_required);
    if (win_by_declaration) {
      moves.win.insert("win");
    }
    else if (drops_required <= 10) {
      heuristic_cost
	= (11 - drops_required)*usi_pawn_value*sign()/2;
    }
    else {
      heuristic_cost = enter_king_bonus_csa(state); // absolute value
    }
  }
  else {
    heuristic_cost = enterKingBonus24(state); // absolute value
  }
  if (heuristic_cost)
    Logging::info("|near enter heuristic "+path()+" "+to_s(heuristic_cost));
  if (moves.win.empty()) {
    osl::NumEffectState s = usi_state.currentState();
    osl::checkmate::FixedDepthSearcher solver(s);
    osl::Move checkmate_move; 
    if (solver.hasCheckmateMoveOfTurn(2, checkmate_move)
	.isCheckmateSuccess())
      moves.win.insert(osl::usi::show(checkmate_move));
  }
  if (! moves.win.empty() && solved.node_count == 0) {
    Logging::info("|"+path()+" winning node");
    solved.node_count = 1;
    solved.pv.score = winValue(1);
    solved.pv.push_back(*moves.win.begin());
    return;
  }

  // no legal moves => loss or draw
  if (moves.normal.empty()) {
    solved.node_count = 1;
    solved.pv.clear();
    if (! moves.hasDraw()) {
      Logging::info("|"+path()+" losing node");
      solved.pv.score = -winValue(1);
      solved.pv.push_back("resign");
    }
    else {
      Logging::info("|"+path()+" draw node");
      solved.pv.score = draw_value;
      solved.pv.push_back(*moves.draw.begin());
    }
  }
}

std::string gpsshogi::SearchNode::
path(const std::string& sofar) const
{
  std::string my_parent
    = (parent_move == "other")
    ? "(+i)" : (parent ? parent->toCSA(parent_move) : parent_move);
  if (parent)
    return parent->path(my_parent+sofar);
  return my_parent+sofar;
}

void gpsshogi::SearchNode::
searchInLeaf(std::function<void(InterimReport)> progress,
	     std::function<void(InterimReport)> finish,
	     int msec, UsiSlavePtr slave,
	     slave_set_t& stopped_slave, int multi_pv)
{
  if (msec > probe_msec)
    committed = true;
  if (hasWorker())
    Logging::error("|searchInLeaf slave conflict "+path()
		   +(leaf.working[0] ? leaf.working[0]->idMark() : "?"
		     +(slave ? slave->idMark() : "?")));
  if (! succ.empty() || hasWorker()) {
    slave_set_t restart_cancelled, waited;
    stopSubTree(stopped_slave, restart_cancelled, waited);
    std::string msg ="|searchInLeaf cleanup "+path()+' '+to_s(succ.size())
		    +' '+to_s(stopped_slave.size())+" "
      +to_s(restart_cancelled.size())+" "+to_s(waited.size());
    if (!waited.empty() || !restart_cancelled.empty())
      Logging::error(msg);
    else
      Logging::notice(msg);
    succ.clear();
  }
  if (main.owner != -1 && main.owner != slave->id()) {
    Logging::info("|ownership changed "+path()+" "+to_s(main.owner)
		  +" =>"+slave->idMark());
    main.owner = slave->id();
  }
  leaf.working[0] = slave;
  slave->go(id(), position, ignoreMoves(), msec, progress, finish,
	    sign()*draw_value, multi_pv);
  if (msec > probe_msec) {
    last_tree_change = osl::clock::now();
    propagateTreeChangeTime();
  }
}

void gpsshogi::SearchNode::
runMate(std::function<void(std::string)> finish,
	int msec, UsiSlavePtr solver)
{
  if (leaf.working[1]) {
    Logging::error("|run mate slave conflict "+path()
		   +leaf.working[1]->idMark()+solver->idMark());
    return;
  }
  leaf.working[1] = solver;
  solver->goMate(id(), position, msec, finish);
}

std::string gpsshogi::SearchNode::ignoreMoves() const
{
  std::string ret = ignore_moves;
  const MoveData *moves = (parent_move == "other")
    ? (parent ? &parent->moves : 0)
    : &this->moves;
  if (parent) {
    for (std::string move: moves->draw)
      ret += " " + move;
    for (std::string move: moves->loss)
      ret += " " + move;
    for (std::string move: moves->nopromote)
      ret += " " + move;
  }
  return ret == "" ? ret : ("ignore_moves" + ret);
}

int gpsshogi::SearchNode::depth8(int cur) const
{
  if (!parent || cur >= 8)
    return cur;
  return parent->depth8(cur+1);
}

uint64_t gpsshogi::SearchNode::
nodeCountForMove(const std::string& move) const
{
  if (isSolved())
    return std::max((uint64_t)1, nodeCount());
  if (hasMove(move))
    return succ.find(move)->second->nodeCount();
  if (hasOther() && succ.find("other")->second->bestMove() == move)
    return succ.find("other")->second->nodeCount();
  return (bestMove() == move) ? nodeCount() : 0;
}

bool gpsshogi::SearchNode::isPrimaryTakeBack(const std::string& move) const
{
  return take_back_moves == 1 && isTakeBack(move);
}

bool gpsshogi::SearchNode::isTakeBack(const std::string& move) const
{
  if (! parent || parent_move == "other" || take_back_moves == 0)
    return false;
  const osl::Move osl_move = findMove(move);
  if (! osl_move.isNormal() || ! osl_move.isCapture()
      || osl_move.capturePtype() == osl::PAWN)
    return false;
  const osl::Move osl_prev = parent->findMove(parent_move);
  if (! osl_prev.isNormal() || ! osl_prev.isCapture())
    return false;
  return osl_prev.to() == osl_move.to();
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
