/* coordinator.cc
 */
#include "coordinator.h"
#include "searchTree.h"
#include "usiServer.h"
#include "logging.h"
#include "osl/oslConfig.h"
#include <boost/filesystem/operations.hpp>
#include <thread>
#include <iostream>

static const boost::posix_time::time_duration msec100
= boost::posix_time::microseconds(100*1000);
static const boost::posix_time::time_duration msec50
= boost::posix_time::microseconds(50*1000);
static const boost::posix_time::time_duration msec25
= boost::posix_time::microseconds(25*1000);
static const boost::posix_time::time_duration poll_interval = msec50;

gpsshogi::Coordinator::
Coordinator(UpStream *u, int e, int p, const std::string& udp_dst,
	    bool slave_udp, const std::string& slave_log_dir,
	    int slave_tlp)
  : timer(search_io, poll_interval), upstream(u),
    manager((p>1) ? slave_io : search_io, search_io, slave_log_dir, slave_tlp), 
    expected_slaves(e), parallel_io(p),
    log_file(new std::ofstream("cluster.log", std::ios::app)),
    last_position(-1), quitting(false)
{
  osl::OslConfig::setUp();
  osl::search::InterimReport::info = [](std::string msg) { Logging::info(msg); };
  osl::search::InterimReport::warn = [](std::string msg) { Logging::warn(msg); };
  osl::search::InterimReport::error = [](std::string msg) { Logging::error(msg); };

  if (slave_log_dir != "")
    boost::filesystem::create_directory(slave_log_dir);
  
  u->bind(*this, search_io);
  Logging::setQueue(slave_io);
  Logging::rebind(log_file.get());
  if (udp_dst != "")
    Logging::startUdpLogging(udp_dst, slave_io);
  Logging::enableSlaveUdp(slave_udp);
  timer.async_wait(
    [=](boost::system::error_code ec){this->handleTimer(ec);});
}

gpsshogi::Coordinator::
~Coordinator()
{
}

void gpsshogi::Coordinator::
start()
{
  std::cerr << "#Coordinator::start\n";
  upstream->start();

  work_u.reset(new boost::asio::io_service::work(search_io));
  work_c.reset(new boost::asio::io_service::work(slave_io));

  for (int i=1; i<parallel_io; ++i)
    threads.push_back
      (new std::thread([=]() { slave_io.run(); }));
  search_io.run();
  Logging::quit();
}

void gpsshogi::Coordinator::
showStatus(std::ostream& os)
{
  os << "Upstream " << upstream->status()
     << " " << (!stop_handler.empty() ? "stopready" : "") << "\n";
  manager.showStatus(os);
  if (tree)
    tree->showStatus(os);
}

void gpsshogi::Coordinator::
waitReady()
{
  manager.waitSlaves(expected_slaves);
}

void gpsshogi::Coordinator::prepareQuit()
{
  quitting = true;
  timer.cancel();
  work_c.reset();
  manager.prepareQuit();
  for (std::thread& thread: threads)
    thread.join();
  work_u.reset();
}

void gpsshogi::Coordinator::
handleTimer(const boost::system::error_code& ec)
{
  if (quitting)
    return;
  if (ec) {
    Logging::error("%timer! " + ec.message());
    timer.expires_at(timer.expires_at() + poll_interval);
    timer.async_wait([this](boost::system::error_code ec){ this->handleTimer(ec);});
    return;
  }
  if (tree)
    tree->periodic();
  manager.periodic();
  timer.expires_at(timer.expires_at() + poll_interval);
  timer.async_wait([this](boost::system::error_code ec){ this->handleTimer(ec);});
}

void gpsshogi::Coordinator::
moveRoot(std::string move)
{
  if (tree)
    tree->moveRoot(move, manager.idleSlavesInQueue());
}

void gpsshogi::Coordinator::
newGameStart()
{
  Logging::udpLine("new game started");
  Logging::makeAndSetNewDirectory();
  Logging::setPrefix("0th");
  log_file_in_game.reset
    (new std::ofstream((Logging::path()+"cluster-game.log").c_str()));
  Logging::rebind(log_file_in_game.get());
  *log_file << "see game log in " << Logging::path() << "\n" << std::flush;
  if (tree)
    tree.reset();
}
void gpsshogi::Coordinator::gameFinished()
{
  if (tree)
    tree->gameFinished();
  Logging::udpLine("game finished");
  Logging::rebind(log_file.get());
  log_file_in_game.reset();
}

// go, go mate

void gpsshogi::Coordinator::
handleGoMate(int id, std::string position, int millisec)
{
  std::vector<UsiSlavePtr> slaves = manager.idleMateSolversInQueue();
  if (slaves.empty()) {
    Logging::error("no slave in go mate");
    mateResult(id, "checkmate timeout");
    return;
  }
  assert(slaves[0]);
  slaves[0]->goMate(id, position, millisec, 
		    [=](std::string msg){ this->mateResult(id, msg); });
  stop_handler[id]
    = std::bind(&UsiSlave::stop, slaves[0].get(), id);
}
void gpsshogi::Coordinator::
mateResult(int position_id, std::string msg)
{
  stop_handler.erase(position_id);
  upstream->outputSearchResult(position_id, msg);
}

void gpsshogi::Coordinator::
handleGo(int id, std::string position, int millisec)
{
  TimeCondition seconds;
  {
    osl::UsiState state;
    state.parseUsi(position);
    seconds.my_turn = state.currentState().turn();
  }
  seconds.byoyomi_msec = millisec;
  stop_handler[id]
    = assignGo(id, position, 
	       [=](std::string s){ upstream->outputSearchProgress(id, s); },
	       [=](std::string s){ this->searchResult(id, s); },
	       seconds);
  Logging::udpLine("go id " + to_s(id) + " msec " + to_s(millisec)
		   + " " + position);
}

void gpsshogi::Coordinator::
searchResult(int position_id, std::string msg)
{
  stop_handler.erase(position_id);
  upstream->outputSearchResult(position_id, msg);
  Logging::udpLine("bestmove " + msg + " id " + to_s(position_id));
}

void gpsshogi::Coordinator::
handleGoInGame(int id, std::string position, const TimeCondition& seconds)
{
  Logging::info(std::string(72, '-'));
  std::string msg = "go id " + to_s(id) + " total " + to_s(seconds.total)
    + " used " + to_s(seconds.used) + " " + position;
  Logging::info(msg);
  std::ostringstream ss;
  manager.showStatus(ss);
  Logging::info("\n"+ss.str());
  
  stop_handler[id]
    = assignGo(id, position, 
	       [=](std::string msg){ upstream->outputSearchProgress(id, msg); },
	       [=](std::string msg){ this->searchResult(id, msg); },
	       seconds);
  Logging::udpLine(msg);
}

std::function<void(void)> gpsshogi::Coordinator::
assignGo(int id, const std::string& position, 
	 std::function<void(std::string)> progress,
	 std::function<void(std::string)> finish,
	 const TimeCondition& seconds)
{
  if (! tree)
    tree.reset(new SearchTree(search_io));
  last_position = id;
  std::vector<UsiSlavePtr> slaves = manager.idleSlavesInQueue();
  for (UsiSlavePtr p: slaves)
    p->send("echo ping new position id "+to_s(id)+" "+position);
  Logging::udpLine("info start new master position "+position);
  return tree->go(id, position, manager.slavesEstimated(), slaves,
		  progress, finish, seconds,
		  manager.idleMateSolversInQueue());
}

void gpsshogi::Coordinator::
handleStop(int position_id)
{
  if (stop_handler[position_id]) {
    stop_handler[position_id]();
    stop_handler.erase(position_id);
  } else {
    std::cerr << "#error stop ignored for " << position_id << "\n";
  }
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
