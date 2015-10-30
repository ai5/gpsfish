/* logging.cc
 */
#include "logging.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/filesystem/operations.hpp>
#include <memory>
#include <mutex>
#include <iostream>
#include <fstream>
#include <ostream>

// static const int debug = true;
static const int debug = false;
static const bool flush_log = false;
static std::mutex log_mutex;
static std::ostream *os = 0;
static boost::asio::io_service *log_queue = 0;
static std::unique_ptr<boost::asio::ip::udp::socket> udp_socket;
static std::unique_ptr<boost::asio::ip::udp::endpoint> udp_endpoint;
static bool slave_udp = false;
static std::unique_ptr<boost::asio::strand> log_strand;
static std::string prefix;
std::string gpsshogi::Logging::directory = ".";

void gpsshogi::
Logging::rebind(std::ostream *o)
{
  os = o;
}
void gpsshogi::
Logging::setQueue(boost::asio::io_service& queue)
{
  log_queue = &queue;
  if (log_queue)
    log_strand.reset(new boost::asio::strand(*log_queue));
  else
    log_strand.reset();
}

const std::string gpsshogi::
Logging::datetime() 
{
  boost::posix_time::ptime now
    = boost::posix_time::microsec_clock::local_time();
  std::string ret = to_iso_extended_string(now);
  if (prefix != "")
    ret += " " + prefix;
  return ret;
}

const std::string gpsshogi::
Logging::time() 
{
  boost::posix_time::ptime now
    = boost::posix_time::microsec_clock::local_time();
  std::string ret = to_simple_string(now.time_of_day());
  return ret;
}

void gpsshogi::
Logging::error(const std::string& msg)
{
  const std::string now = datetime();
  writeLine(std::cerr, now + " error " + msg, true);
  if (os)
    schedule(now, " error " + msg, log_queue, true, true);
}

void gpsshogi::
Logging::warn(const std::string& msg)
{
  const std::string now = datetime();
  schedule(now, " warning " + msg, log_queue, false);
  if (os)
    schedule(now, " warning " + msg, log_queue, true, true);
}

void gpsshogi::
Logging::notice(const std::string& msg)
{
  const std::string now = datetime();
  schedule(now, " " + msg, log_queue, false);
  if (os)
    schedule(now, " " + msg, log_queue, true, true);
}

void gpsshogi::
Logging::info(const std::string& msg, bool important)
{
  const std::string now = time();
  schedule(now, " info " + msg, log_queue, os, important);
}

void gpsshogi::
Logging::schedule(const std::string& time, const std::string& msg,
		  boost::asio::io_service *log_queue,
		  bool to_file, bool important)
{
  if (log_queue && !debug)
    log_queue->post(log_strand->wrap(std::bind(&Logging::writeLineTo, time,
						 msg, to_file, important)));
  else
    Logging::writeLineTo(time, msg, to_file && !debug, important);
}

void gpsshogi::
Logging::writeLine(std::ostream& os, const std::string& msg, bool important)
{
  os << msg << "\n";
  if (flush_log || important)
    os << std::flush;
}
void gpsshogi::
Logging::writeLineTo(std::string time, std::string msg, bool to_file, bool important)
{
  if (prefix != "")
    time += " " + prefix;
  if (to_file && os)
    writeLine(*os, time+msg, important);
  else
    writeLine(std::cerr, time+msg, important);
}

void gpsshogi::
Logging::udpLine(std::string msg) 
{
  std::vector<std::string> lines;
  boost::algorithm::split(lines, msg, boost::algorithm::is_any_of("\n"));

  std::lock_guard<std::mutex> lk(log_mutex);
  if (! udp_socket)
    return;
  for (std::string line: lines) {
    boost::system::error_code ignored_error;
    line = "server " + line + "\n";
    udp_socket->send_to(boost::asio::buffer(line.c_str(), line.length()),
			*udp_endpoint, 0,
			ignored_error);
  }
}

void gpsshogi::
Logging::startUdpLogging(const std::string& host_port,
			 boost::asio::io_service& udp_queue) 
{
  if (host_port == "")
    return;
  std::lock_guard<std::mutex> lk(log_mutex);
  std::vector<std::string> args;
  boost::algorithm::split(args, host_port, boost::algorithm::is_any_of(":"));

  using boost::asio::ip::udp;
  udp::resolver resolver(udp_queue);
  udp::resolver::query query(udp::v4(), args[0], args[1]);

  udp_socket.reset(new udp::socket(udp_queue));
  udp_socket->open(udp::v4());
  udp_endpoint.reset(new udp::endpoint(*resolver.resolve(query)));
}

void gpsshogi::
Logging::enableSlaveUdp(bool enable)
{
  slave_udp = enable;
}

void gpsshogi::
Logging::slaveUdp(const std::string& name, std::string msg)
{
  if (! slave_udp)
    return;
  std::vector<std::string> lines;
  boost::algorithm::split(lines, msg, boost::algorithm::is_any_of("\n"));
  
  std::lock_guard<std::mutex> lk(log_mutex);
  if (! udp_socket)
    return;
  for (std::string line: lines) {
    line = name + ' ' + line + "\n";
    boost::system::error_code ignored_error;
    udp_socket->send_to(boost::asio::buffer(line.c_str(), line.length()),
			*udp_endpoint, 0, ignored_error);
  }
}

void gpsshogi::
Logging::quit()
{
  std::lock_guard<std::mutex> lk(log_mutex);
  udp_socket.reset();
  udp_endpoint.reset();
  os = 0;
}

std::string gpsshogi::
Logging::makeAndSetNewDirectory()
{
  boost::posix_time::ptime now
    = boost::posix_time::second_clock::local_time();
  std::string year = to_s(now.date().year());
  boost::filesystem::create_directory(year);
  std::string monthday = to_s(now.date().month())+to_s(now.date().day());
  boost::filesystem::create_directory(year+"/"+monthday);
  std::string time = to_simple_string(now.time_of_day());
  boost::algorithm::replace_all(time, ":", "-"); 
  directory = year+"/"+monthday+"/"+time;
  boost::filesystem::create_directory(directory);
  return directory;
}

void gpsshogi::
Logging::setPrefix(std::string new_prefix)
{
  if (log_queue)
    log_queue->post(log_strand->wrap(std::bind(&Logging::setPrefixInQueue,
						 new_prefix)));
  else
    setPrefixInQueue(new_prefix);
}

void gpsshogi::
Logging::setPrefixInQueue(std::string new_prefix)
{
  prefix = new_prefix;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
