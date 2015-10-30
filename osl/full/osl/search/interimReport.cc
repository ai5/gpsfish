/* interimReport.cc
 */
#include "interimReport.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <functional>
#include <iostream>
#include <sstream>
#include <cassert>

std::function<void(std::string)> osl::search::InterimReport::info = [](std::string){};
std::function<void(std::string)> osl::search::InterimReport::warn = [](std::string){};
std::function<void(std::string)> osl::search::InterimReport::error = [](std::string){};

osl::search::InterimReport::
InterimReport(int o)
  : owner(o), depth_head(0),
    node_count(0), elapsed(0.0), stopped(false), aborted(false), 
    last_message_ignored(false)
{
}

osl::search::InterimReport::
~InterimReport()
{
}

void osl::search::InterimReport::
finished(const std::string& line)
{
  assert(line.find("checkmate ") == 0 || line.find("bestmove ") == 0);
  result_line = line;
  if (node_count == 0 && pv.score == 0
      && line.find("checkmate ") != 0) {
    info("$ finish without info, owner "+std::to_string(owner)+" "+line);
    if (line == "bestmove resign") {
      pv.score = -usi_win_value;
      pv.clear();
      pv.push_back("resign");
    }
    else if (line == "bestmove win") {
      pv.score = usi_win_value;
      pv.clear();
      pv.push_back("win");
    }
    else {
      error("$ unexpected finish without info, owner "+std::to_string(owner)
	    +" "+line);
    }
  }
}

bool osl::search::InterimReport::
updateByInfo(const std::string& line, int id)
{
  const bool overwrite = id < 0;
  const bool verbose = false;
  if (verbose)
    std::cerr << "$ " << line << "\n";
  bool important = false;
  std::istringstream is(line);
  std::string key, value;
  int64_t int_value;
  is >> key;
  assert(key == "info");
  bool same_best_move = false, same_score = false, better_score = false,
    has_score = false, enter_new_depth = false;
  int new_score = 0;
  std::vector<std::string> new_pv;
  while (is >> key) {
    if (key == "depth") {
      is >> int_value;
      if (! (overwrite || (is && int_value >= depth_head)))
	info("$depth decreased "+std::to_string(id)+" "+line);
      enter_new_depth = (int_value > pv.depth);
      depth_head = int_value;
    }
    else if (key == "time") {
      is >> int_value;
      assert(is && int_value > elapsed*1000-100);
      if (int_value == 0)
	int_value = 1;
      elapsed = int_value / 1000.0;
    }
    else if (key == "nodes") {
      is >> int_value;
      if (int_value < node_count)
	error("$node count decreased " + std::to_string(int_value) + ' '
	      + std::to_string(node_count) + " in " + line);
      assert(is && int_value >= node_count);
      node_count = int_value;
    }
    else if (key == "pv") {
      if (getline(is, value)) {
	std::string prev_best = pv.empty() ? "" : pv[0];
	boost::algorithm::trim(value);
	boost::algorithm::trim_left(value);
	boost::algorithm::split(new_pv, value, boost::algorithm::is_any_of(" "));
	same_best_move = !new_pv.empty() && (prev_best == new_pv[0]);
      }
      else if (! new_pv.empty()) {
	error("$warn empty pv: " + line);
      }
    }
    else if (key == "score") {
      is >> key >> int_value;
      assert(is);
      if (key == "cp") {
	has_score = true;
	better_score = (int_value > pv.score); // negamax at this point
	same_score = (pv.score == int_value);
	new_score = int_value;
      }
    }
    else if (key == "seldepth" || key == "multipv"
	     || key.find("currmove") == 0 || key == "hashfull"
	     || key == "nps" || key == "cpuload") {
      is >> value;		// ignore
      assert(is);
    }
    else if (key == "string") {
      getline(is, value);
      last_string = value;
      if (last_string == " loss by checkmate") {
	pv.score = -usi_win_value;
	pv.clear();
	pv.push_back("resign");
	important = true;
      }
      else {
	// warn("$warn string not supported yet " + line);
	// todo? important = true;
      }
    }
    else {
      error("$warn unknown info key " + key + " in " + line);
      assert(0);
    }
  }

  if (enter_new_depth || better_score || same_best_move || overwrite) {
    if (has_score && ! new_pv.empty()
	&& ! pv.empty() && ! same_best_move) {
      alternatives[pv[0]] = pv;
    }
    if (has_score) {
      pv.depth = depth_head;
      pv.score = new_score;
      pv.elapsed = elapsed;
      important = true;
    }
    if (! new_pv.empty()) {
      pv.depth = depth_head;
      pv.moves.swap(new_pv);
      pv.elapsed = elapsed;
      important = true;
    }
  } else if (has_score && !new_pv.empty()) {
    // info("subpv from "+std::to_string(id)+ " " + line);
    PVInfo& data = alternatives[new_pv[0]];
    data.depth = depth_head;
    data.score = new_score;
    data.moves = new_pv;
    data.elapsed = elapsed;
  }

  // gpsfish sometimes reaches depth 100 within 0.3ms in case of easy win,
  // reporting the same move and value
  if (depth_head > 20 && same_best_move && same_score
      && abs(pv.score) > 1000) {
    if (! last_message_ignored) {
      info("$ignored repeated massages from " +std::to_string(id)
	   + " " + std::to_string(pv.score));
      last_message_ignored = true;
    }
    return false;
  }
  if (important)
    last_message_ignored = false;
  return important;
}

std::string osl::search::InterimReport::
composeInfo(bool negate_score) const
{
  std::ostringstream msg;
  msg << "info";
  int dh = depth_head, pd = pv.depth;
  if (std::max(dh, pd) > 0)
    msg << " depth " << std::max(dh, pd);
  if (elapsed > 0.0)
    msg << " time " << static_cast<int>(elapsed*1000);
  msg << " score cp " << (negate_score ? -pv.score : pv.score);
  if (node_count > 0) {
    msg << " nodes " << node_count;
    if (elapsed > 0.0)
      msg << " nps " << static_cast<int>(node_count/elapsed);
  }
  if (! pv.empty())
    msg << " pv " << joinPV();
  return msg.str();
}

std::string osl::search::InterimReport::
joinPV() const
{
  return boost::algorithm::join_if
    (pv.moves, " ", [](std::string s){
      return !boost::algorithm::starts_with
	<const std::string,const std::string>(s, "ignore_moves");
    });				   
}

std::string osl::search::InterimReport::
makeSearchResult() const
{
  std::string result = "bestmove ";
  if (pv.empty())
    return result + "resign";
  if (pv[0].find("ignore_moves") == 0)
    return result + pv[1];
  return result + pv[0];
}

void osl::search::InterimReport::
set(osl::Player turn, const InterimReport& src)
{
  if (this == &src)
    return;

  *this = src;

  pv.score *= sign(turn);
  for (pv_table::value_type& v: alternatives) {
    v.second.score *= sign(turn);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
