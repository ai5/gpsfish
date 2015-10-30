/* slaveManager.cc
 */
#include "slaveManager.h"
#include "tcpcluster.h"
#include "logging.h"
#include "osl/numEffectState.h"
#include <boost/algorithm/string/join.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>
using namespace boost::asio::ip;
gpsshogi::SlaveManager::
SlaveManager(boost::asio::io_service &i,
	     boost::asio::io_service &u,
	     const std::string& c, int s)
  : io(i), usi_queue(u),
    acceptor(io, tcp::endpoint(tcp::v4(), gpsshogi::cluster_port)),
    client_log_dir(c), num_slave(0), quitting(false), slave_tlp(s)
{
  usi_queue.post(std::bind(&SlaveManager::newAccept, this));
  last_ping = osl::clock::now();
  connections.reserve(1024);
  active.reserve(1024);
  active_mate.reserve(128);
}

gpsshogi::SlaveManager::
~SlaveManager()
{
}

bool gpsshogi::SlaveManager::
sourceAlreadyUsed(const boost::asio::ip::address& address) const
{
  for (UsiSlavePtr slave: active) {
    if (dead_slave.count(slave.get()) == 0
	&& slave->socket.remote_endpoint().address() == address)
      return true;
  }
  for (UsiSlavePtr slave: active_mate) {
    if (dead_slave.count(slave.get()) == 0
	&& slave->socket.remote_endpoint().address() == address)
      return true;
  }
  return false;
}

static std::vector<std::string> accept_multiple_slaves;
void gpsshogi::SlaveManager::
acceptMultipleSlaves(const std::string& filename) 
{
  std::ifstream is(filename.c_str());
  std::string ip;
  while (is >> ip)
    accept_multiple_slaves.push_back(ip);
}
bool gpsshogi::SlaveManager::
allowMultipleLogin(const boost::asio::ip::address& address) 
{
  return std::count(accept_multiple_slaves.begin(),
		    accept_multiple_slaves.end(), gpsshogi::to_s(address));
}

void gpsshogi::SlaveManager::
handleAccept(const boost::system::error_code& ec,
	     UsiSlavePtr slave)
{
  if (quitting)
    return;
  if (ec) {
    Logging::error("%accept! " + ec.message());
    // should we exit here?
    return;
  }
  usi_queue.post(std::bind(&SlaveManager::newAccept, this));
  boost::asio::ip::address address
    = slave->socket.remote_endpoint().address();
  if (slave_tlp == 0 && sourceAlreadyUsed(address)
      && ! allowMultipleLogin(address)) {
    Logging::error("%accept refused as "+ to_s(address)+" already used.  "
		   + "give slave_tlp>=1 to skip this test.");
    return;
  }
  slave->start();
}

void gpsshogi::SlaveManager::
newAccept()
{
  // must be called within usi_queue::run
  UsiSlavePtr slave(new UsiSlave(*this, io, usi_queue,
				 connections.size(), client_log_dir));
  if (slave_tlp)
    slave->setSlaveTLP(slave_tlp);
  connections.push_back(slave);   
  acceptor.async_accept
    (slave->socket,
     [=](boost::system::error_code ec){this->handleAccept(ec, slave);});
}

void gpsshogi::SlaveManager::
removeSlave(UsiSlave *slave)
{
  usi_queue.post(std::bind(&SlaveManager::removeSlaveInQueue,
			     this, slave));
}

void gpsshogi::SlaveManager::
removeSlaveInQueue(UsiSlave *slave)
{
  // must be called within usi_queue::run
  Logging::error("%disconnect [" + to_s(slave->id()) + "]");
  if (quitting)
    return;
  if (slave->work_space.fail_handler) {
    slave->work_space.fail_handler();
    slave->work_space.fail_handler = 0;
  }
  --num_slave;
  // xxx: workaround: remove UsiSlavePtr often throws exception by unidentified reason
  dead_slave.insert(slave);
  std::lock_guard<std::mutex> lk(mutex);
  condition.notify_all();
}

void gpsshogi::SlaveManager::
addActiveSlave(UsiSlavePtr slave)
{
  usi_queue.post(std::bind(&SlaveManager::addActiveSlaveInQueue,
			     this, slave));
}

void gpsshogi::SlaveManager::
addActiveSlaveInQueue(UsiSlavePtr slave)
{
  // must be called within usi_queue::run
  boost::asio::ip::address address
    = slave->socket.remote_endpoint().address();
  if (slave_tlp == 0 && sourceAlreadyUsed(address)
      && ! allowMultipleLogin(address)) {
    Logging::error("%connection refused as "+ to_s(address)+" already used.  "
		   + "give slave_tlp>=1 to skip this test.");
    slave->prepareQuit();
    return;
  }
  active.push_back(slave);
  ++num_slave;
  std::lock_guard<std::mutex> lk(mutex);
  condition.notify_all();
}

void gpsshogi::SlaveManager::
addActiveMateSolver(UsiSlavePtr slave)
{
  usi_queue.post(std::bind(&SlaveManager::addActiveMateSolverInQueue,
			     this, slave));
}

void gpsshogi::SlaveManager::
addActiveMateSolverInQueue(UsiSlavePtr slave)
{
  // must be called within usi_queue::run
  boost::asio::ip::address address
    = slave->socket.remote_endpoint().address();
  if (slave_tlp == 0 && sourceAlreadyUsed(address)) {
    Logging::error("%connection refused as "+ to_s(address)+" already used.  "
		   + "give slave_tlp>=1 to skip this test.");
    slave->prepareQuit();
    return;
  }
  Logging::info("%mate solver from "+ to_s(address)+slave->idMark(), true);
  active_mate.push_back(slave);
  ++num_slave;
  std::lock_guard<std::mutex> lk(mutex);
  condition.notify_all();
}

void gpsshogi::SlaveManager::
waitSlaves(int number)
{
  std::unique_lock<std::mutex> lk(mutex);
  while ((int)(active.size()+active_mate.size())-dead_slave.size() < number
	 || active.empty()) {
    Logging::info("%waiting clients " + to_s(active.size()) + " < " + to_s(number),
		  true);
    condition.wait(lk);
  }
}

std::vector<gpsshogi::UsiSlavePtr> gpsshogi::SlaveManager::
selectIdleInQueue(const std::vector<gpsshogi::UsiSlavePtr>& src,
		  const std::set<UsiSlave*>& dead_slave)
{
  std::vector<UsiSlavePtr> ret;
  ret.reserve(src.size());
  for (UsiSlavePtr slave: src) {
    if (dead_slave.count(slave.get())==0
	&& ! slave->error() && slave->usiStatus() == Idle)
      ret.push_back(slave);
  }
  return ret;
}

void gpsshogi::SlaveManager::
prepareQuit()
{
  quitting = true;
  acceptor.cancel();
  acceptor.close();
  std::lock_guard<std::mutex> lk(mutex);
  for (UsiSlavePtr slave: active) {
    slave->prepareQuit();
  }
  for (UsiSlavePtr slave: active_mate) {
    slave->prepareQuit();
  }
}

void gpsshogi::SlaveManager::
showStatus(std::ostream& os)
{
  if (quitting) {
    Logging::info("%quitting", true);
    return;
  }
  std::vector<std::pair<int,UsiSlavePtr> > slaves;
  slaves.reserve(active.size());
  for (UsiSlavePtr slave: active)
    if (dead_slave.count(slave.get()) == 0)
      slaves.push_back(std::make_pair(slave->id(), slave));
  std::sort(slaves.begin(), slaves.end());
  double nps_sum = 0.0;
  for (size_t i=0; i<slaves.size(); ++i) {
    const UsiSlave& slave = *slaves[i].second;
    os << "[" << slave.id() << "] "
       << slave.socket.remote_endpoint()
       << " e=" << osl::elapsedSeconds(slave.status.last_access)
       << " nps=" << slave.status.nps()
       << " (" << slave.status.elapsed << ")"
       << " " << slave.usiStatus();
    nps_sum += slave.status.nps();
    if (slave.usiStatus() != Idle)
      os << " n=" << slave.work_space.interim_report.node_count;
    if (slave.usiStatus() == Go || slave.usiStatus() == SearchStop) {
      os << " s=" << slave.work_space.interim_report.bestValue();
      const std::vector<std::string>& pv
	= slave.work_space.interim_report.pv.moves;
      os << " p=" << boost::algorithm::join
	(boost::make_iterator_range(pv.begin(), pv.begin()+std::min((size_t)4, pv.size())), " ");
    }
    os << "\n";
  }

  for (UsiSlavePtr slave: active_mate) {
    if (dead_slave.count(slave.get()))
      continue;
    os << "M [" << slave->id() << "] "
       << slave->socket.remote_endpoint()
       << " " << slave->usiStatus()
       << "\n";
  }
  os << "NPS = " << boost::format("%.2f") % (nps_sum / 1e6) << "M\n";
}

void gpsshogi::SlaveManager::
periodic() 
{
  if (quitting)
    return;
  osl::time_point now = osl::clock::now();
  if (osl::toSeconds(now - last_ping) > usi_keep_alive) {
    last_ping = now;
    Logging::info("%.");
    for (UsiSlavePtr slave: active)
      if (! dead_slave.count(slave.get()))
	slave->echoIfIdle(now);
    for (UsiSlavePtr slave: active_mate)
      if (! dead_slave.count(slave.get()))
	slave->echoIfIdle(now);
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
