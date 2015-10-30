/* usiServer.cc
 */
#include "usiServer.h"
#include "coordinator.h"
#include "logging.h"
#include "osl/game_playing/usiState.h"
#include "osl/game_playing/usiResponse.h"
#include "osl/usi.h"
#include <thread>
#include <iostream>

gpsshogi::UsiServer::
UsiServer() : os(std::cout), is(std::cin), verbose(true)
{
  current_status = WaitConnection;
  positions.push_back("position startpos");
}

gpsshogi::UsiServer::
~UsiServer()
{
}

void gpsshogi::UsiServer::
start()
{
  std::thread(std::bind(&UsiServer::watchInput, this));
}

void gpsshogi::UsiServer::
watchInput()
{
  Logging::info("^waiting upstream");
  current_status = Initializing;
  osl::UsiState input_state;
  osl::UsiResponse lightweight_commands(input_state, true, verbose);
  std::string response;
  response.reserve(2000);
  osl::MoveVector ignore_moves;
  bool quit_by_command = false;
  // 
  std::string line;
  if (! getline(is, line)) {
    goto finish;
  }
  assert(line=="usi");
  os << "id name gpsshogi_tcluster\n";
  os << "id auther teamgps\n";
  os << "usiok\n" << std::flush;
  current_status = Idle;
  // from here different thread may want to write into os
  while (getline(is, line)) {
    if (line.find("setoption") == 0)
      continue;
    if (line.find("isready") == 0) {
      osl::MoveLogProbVector moves;
      lightweight_commands.genmoveProbability(2000, moves);
      coordinator->waitReady();
      std::lock_guard<std::mutex> lk(mutex);
      os << "readyok\n" << std::flush;
      continue;
    }
    Logging::info("S< " + line);
    if (line == "quit") {
      quit_by_command = true;
      io->post(std::bind(&Coordinator::prepareQuit, coordinator));
      break;
    }
    if (lightweight_commands.hasImmediateResponse(line, response)) {
      std::lock_guard<std::mutex> lk(mutex);
      os << response << "\n" << std::flush;
      continue;
    }
    if (line.find("position") == 0) {
      try {
	input_state.parseUsi(line);
	if (verbose)
	  Logging::notice("^new position " + to_s(positions.size())
			  + " " + line);
	std::lock_guard<std::mutex> lk(mutex);
	positions.push_back(line);
      }
      catch (std::exception& e) {
	Logging::error(std::string("^usi parse exception ") + e.what());
      }
      ignore_moves.clear();
      continue;
    }
    if (line.find("stop") == 0) {
      std::unique_lock<std::mutex> lk(mutex);
      if (bestmove_history.size() < go_history.size()) {
	assert(current_status == Go || current_status == GoMate);
	current_status = (current_status == Go)
	  ? SearchStop : CheckmateStop;
	io->post(std::bind(&Coordinator::handleStop, coordinator,
			    go_history.back().first));
      }
      const bool block_when_stop = false;
      if (block_when_stop) {
	while (bestmove_history.size() < go_history.size())
	  condition.wait(lk);
      }
      continue;
    }
    if (line.find("go") == 0) {
      {
	std::unique_lock<std::mutex> lk(mutex);
	// block if needed
	if (bestmove_history.size() < go_history.size()) {
	  Logging::error("^warn block go until bestmove reported for the last go");
	  while (bestmove_history.size() < go_history.size())
	    condition.wait(lk);
	}
      }
      // go mate
      if (line.find("go mate") == 0) {
	int limit = 60*1000, msec = limit;
	std::string option;
	std::istringstream is(line.substr(strlen("go mate")));
	if (is >> msec)
	  msec = std::max(std::min(limit, msec), 500);
	const int position_id = positions.size()-1;
	std::lock_guard<std::mutex> lk(mutex);
	current_status = GoMate;
	go_history.push_back(std::make_pair(position_id, true));

	io->post(std::bind(&Coordinator::handleGoMate, coordinator,
			    position_id, positions[position_id], msec));
	continue;
      }
      // go
      int msec = 30*1000;
      std::string option;
      std::istringstream is(line.substr(3));
      while (is >> option) {
	if (option == "btime" || option == "wtime") {
	  Logging::error("^warn ignored " + option);
	  is >> option;
	}
	else if (option == "byoyomi")
	  is >> msec;
	else if (option == "infinite") 
	  msec = 60*60*1000;
	else
	  Logging::error("^unknown option in go " + option);
      }
      const int position_id = positions.size()-1;
      std::lock_guard<std::mutex> lk(mutex);
      current_status = Go;
      go_history.push_back(std::make_pair(position_id, false));
      io->post(std::bind(&Coordinator::handleGo, coordinator,
			  position_id, positions[position_id], msec));
      continue;
    }
    // non-standard commands
    if (line.find("status") == 0) {
      io->post(std::bind(&Coordinator::showStatus, coordinator,
			 std::ref(std::cerr)));
      continue;
    }
    if (line.find("ignore_moves") == 0) {
      input_state.parseIgnoreMoves(line, ignore_moves);
      continue;
    }
    if (line.find("open ") == 0) {
      try {
	input_state.openFile(line.substr(5));
	if (verbose)
	  Logging::notice("^new position " + to_s(positions.size())
			  + " " + input_state.usiString());
	std::lock_guard<std::mutex> lk(mutex);
	positions.push_back(input_state.usiString());
      }
      catch (std::exception& e) {
	Logging::error(to_s("^usi parse exception ") + e.what());
      }
      ignore_moves.clear();
      continue;
    }
    Logging::error("^unknown server message " + line);
  }
  std::this_thread::sleep_for(std::chrono::seconds(5));
finish:
  current_status = Disconnected;
  if (! quit_by_command)
    Logging::error("^stream closed");
}

const std::string gpsshogi::UsiServer::
position(int pid) const
{
  std::lock_guard<std::mutex> lk(mutex);
  return positions.at(pid);
}

void gpsshogi::UsiServer::
outputSearchResult(int postion_id, const std::string& msg)
{
  std::lock_guard<std::mutex> lk(mutex);
  assert(bestmove_history.size() < go_history.size());
  assert(go_history.back().first == postion_id);
  bestmove_history.push_back(msg);
  os << msg << "\n" << std::flush;
  Logging::info("S> " + msg);
  current_status = Idle;
  condition.notify_all();
}

void gpsshogi::UsiServer::
outputSearchProgress(int postion_id, const std::string& msg)
{
  std::lock_guard<std::mutex> lk(mutex);
  assert(bestmove_history.size() < go_history.size());
  assert(go_history.back().first == postion_id);
  os << msg << "\n" << std::flush;
  Logging::info("S> " + msg);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
