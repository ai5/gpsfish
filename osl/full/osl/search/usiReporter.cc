/* usiReporter.cc
 */
#include "osl/search/usiReporter.h"
#include "osl/usi.h"
#include "osl/misc/lightMutex.h"
#include "osl/misc/milliSeconds.h"
#include "osl/oslConfig.h"
#include <iostream>

void osl::search::
UsiReporter::newDepth(std::ostream& os, int depth)
{
  if (OslConfig::usiModeInSilent())
    return;
  os << "info depth " << depth << "\n";
}
void osl::search::
UsiReporter::showPV(std::ostream& os, int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last, bool ignore_silent)
{
  if (OslConfig::usiModeInSilent() && ! ignore_silent)
    return;
  int seldepth = last-first;
  if (last == first || *first != cur)
    ++seldepth;
  if (ignore_silent)
    std::cerr << "info depth " << depth << " seldepth " << seldepth
	      << " time " << static_cast<int>(elapsed*1000) << " score cp " << value
	      << " nodes " << node_count
	      << " nps " << static_cast<int>(node_count/elapsed)
	      << " pv " << usi::show(cur) << "\n";
  os << "info depth " << depth << " seldepth " << seldepth
     << " time " << static_cast<int>(elapsed*1000) << " score cp " << value
     << " nodes " << node_count << " nps " << static_cast<int>(node_count/elapsed);
  os << " pv " << usi::show(cur);
  if (first != last) {
    if (cur == *first)
      ++first;
    while (first != last) {
      os << ' ' << usi::show(*first++);
    }
  }
  os << "\n";
}

void osl::search::
UsiReporter::showPVExtended(std::ostream& os, int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last, const bool *tfirst, const bool *tlast)
{
  if (OslConfig::usiModeInSilent())
    return;
  int seldepth = last-first;
  assert(seldepth == tlast-tfirst);
  if (last - first && *first != cur)
    ++seldepth;
  os << "info depth " << depth << " seldepth " << seldepth
     << " time " << static_cast<int>(elapsed*1000) << " score cp " << value
     << " nodes " << node_count << " nps " << static_cast<int>(node_count/elapsed);
  os << " pv " << usi::show(cur);
  if (first != last) {
    if (cur == *first)
      ++first, ++tfirst;
    while (first != last) {
      os << ' ' << usi::show(*first++);
      if (tfirst != tlast && *tfirst++)
	os << "(^)";
    }
  }
  os << "\n";
}

static osl::misc::LightMutex usi_root_move_mutex;
void osl::search::
UsiReporter::rootMove(std::ostream& os, Move cur, bool allow_frequent_display)
{
  if (OslConfig::usiModeInSilent())
    return;
  static time_point prev;
  if (! allow_frequent_display) 
  {
    time_point now = clock::now();
    {
      SCOPED_LOCK(lk, usi_root_move_mutex);
      if (toSeconds(now - prev) < 0.5)
	return;
      prev = now;
    }
  }
  os << "info currmove " << usi::show(cur) << "\n";
}

void osl::search::
UsiReporter::timeInfo(std::ostream& os, size_t node_count, double elapsed)
{
  if (OslConfig::usiModeInSilent())
    return;
  os << "info time " << static_cast<int>(elapsed*1000)
     << " nodes " << node_count << " nps " << static_cast<int>(node_count/elapsed) << "\n";
  os << std::flush;
}

void osl::search::
UsiReporter::hashInfo(std::ostream& os, double ratio)
{
  if (OslConfig::usiModeInSilent())
    return;
  os << "info hashfull " << static_cast<int>(ratio*1000) << "\n";
  os << std::flush;
}



osl::search::
UsiMonitor::UsiMonitor(bool ex, std::ostream& o, double s)
  : silent_period(s), extended(ex), os(o), udp_socket(0), udp_endpoint(0),
    client_id("")
{
}

osl::search::
UsiMonitor::~UsiMonitor()
{
}

void osl::search::
UsiMonitor::showDeferred(bool forced)
{
  if ((!forced && elapsedSeconds(depth0) < silent_period)
      || deferred.empty())
    return;
  os << deferred << std::flush;
  if (udp_socket) {
    if (client_id != "")
      deferred = client_id + " " + deferred;
    boost::system::error_code ignored_error;
    udp_socket->send_to(boost::asio::buffer(deferred.c_str(), deferred.length()),
                        *udp_endpoint,
			0, ignored_error);
  }
  deferred.clear();
}

void osl::search::
UsiMonitor::newDepth(int depth)
{
  last_root_move = Move();
  if (depth == 0) {
    depth0 = clock::now();
    return;
  }
  if (elapsedSeconds(depth0) >= silent_period) {
    showDeferred();
    UsiReporter::newDepth(os, depth);
  }
}

void osl::search::
UsiMonitor::showPV(int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last,
		   const bool *threatmate_first, const bool *threatmate_last)
{
  const bool defer = elapsedSeconds(depth0) < silent_period;
  std::ostringstream ss;
  if (extended)
    UsiReporter::showPVExtended(ss, depth, node_count, elapsed, value, cur, first, last, 
				threatmate_first, threatmate_last);
  else
    UsiReporter::showPV(ss, depth, node_count, elapsed, value, cur, first, last);
  if (defer)
    deferred = ss.str();
  else {
    std::string msg = ss.str();
    os << msg << std::flush;
    if (udp_socket) {
      if (client_id != "")
	msg = client_id + " " + msg;
      boost::system::error_code ignored_error;
      udp_socket->send_to(boost::asio::buffer(msg.c_str(), msg.length()),
                          *udp_endpoint, 0,
			  ignored_error);
    }
    deferred.clear();		// msg sent was newer than deferred
  }
}

void osl::search::UsiMonitor::
showFailLow(int depth, size_t node_count, double elapsed, int value, Move cur)
{
  showPV(depth, node_count, elapsed, value, cur, 0, 0, 0, 0);
}

void osl::search::
UsiMonitor::rootMove(Move cur)
{
  showDeferred();
  last_root_move = cur;
}

void osl::search::
UsiMonitor::rootFirstMove(Move cur)
{
  showDeferred();
  last_root_move = cur;
  UsiReporter::rootMove(os, cur, true);
}

void osl::search::
UsiMonitor::timeInfo(size_t node_count, double elapsed)
{
  showDeferred();
  {
    std::ostringstream ss;
    UsiReporter::timeInfo(ss, node_count, elapsed);
    std::string msg = ss.str();
    os << msg << std::flush;
    if (udp_socket) {
      if (client_id != "")
	msg = client_id + " " + msg;
      boost::system::error_code ignored_error;
      udp_socket->send_to(boost::asio::buffer(msg.c_str(), msg.length()),
                          *udp_endpoint, 0,
			  ignored_error);
    }    
  }
  UsiReporter::rootMove(os, last_root_move);
}

void osl::search::
UsiMonitor::hashInfo(double ratio)
{
  showDeferred();
  UsiReporter::hashInfo(os, ratio);
}

void osl::search::
UsiMonitor::rootForcedMove(Move the_move)
{
  if (OslConfig::usiModeInSilent())
    return;
  showDeferred();
  os << "info string forced move at the root: "
     << usi::show(the_move) << "\n";
  os << std::flush;
}

void osl::search::
UsiMonitor::rootLossByCheckmate()
{
  if (OslConfig::usiModeInSilent())
    return;
  deferred.clear();
  os << "info string loss by checkmate\n";
  os << std::flush;
}

void osl::search::
UsiMonitor::setUdpLogging(std::string& udp_client_id,
			  boost::asio::ip::udp::socket *s,
			  boost::asio::ip::udp::endpoint *e)
{
  client_id = udp_client_id;
  udp_socket = s;
  udp_endpoint = e;
}

void osl::search::
UsiMonitor::searchFinished()
{
  showDeferred(true);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
