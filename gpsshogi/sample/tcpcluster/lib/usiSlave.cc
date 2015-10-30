/* UsiSlave.cc
 */
#include "usiSlave.h"
#include "slaveManager.h"
#include "logging.h"
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

static const int logging_id_limit = 1000;
std::string gpsshogi::UsiSlave::mate_solver_key = "gpsshogi";
static const boost::posix_time::time_duration msec500
= boost::posix_time::microseconds(500*1000);



gpsshogi::WorkSpace::
WorkSpace() : position_id(-1)
{
}
gpsshogi::WorkSpace::
~WorkSpace()
{
}

void gpsshogi::WorkSpace::
failed()
{
  interim_report.aborted = true;
  if (job_finished)
    job_finished(interim_report);
}

void gpsshogi::WorkSpace::
reset(int owner, const std::string& po, int id,
      std::function<void(InterimReport)> pr, 
      std::function<void(InterimReport)> s, 
      std::function<void(std::string)> l)
{
  position = po;
  position_id = id;
  interim_report = InterimReport(owner);
  job_progress = pr;
  job_finished = s;
  fail_handler = std::bind(&WorkSpace::failed, this);
  listener = l;
}

void gpsshogi::WorkSpace::
reset(int owner)
{
  reset(owner, "", 0, 0, 0, 0);
}

std::function<void(void)> gpsshogi::WorkSpace::
bindProgress()
{
  return std::bind(job_progress, interim_report);
}

std::function<void(void)> gpsshogi::WorkSpace::
bindFinished()
{
  return std::bind(job_finished, interim_report);
}



gpsshogi::SlaveStatus::
SlaveStatus()
  : node_count(0), elapsed(0.0), usi(WaitConnection),
    info_update_needed(false), timer_waiting(false)
{
}

double gpsshogi::SlaveStatus::nps() const
{
  return node_count / (elapsed+1.0);
}

void gpsshogi::SlaveStatus::update(const InterimReport& report)
{
  node_count += report.node_count;
  elapsed += report.elapsed;
}

gpsshogi::UsiSlave::
UsiSlave(SlaveManager& m,
	 boost::asio::io_service &io, boost::asio::io_service &u,
	 int id, const std::string& client_log_dir)
  : socket(io), usi_queue(u), local_queue(io), strand(io),
    manager(m), slave_id(id),
    message_waiting(""), sending_message(false), slave_tlp(0),
    timer(io)
{
  if (client_log_dir != "" && id < logging_id_limit)
    local_log.reset(new std::ofstream((client_log_dir+"/"+to_s(id)+".log").c_str(), std::ios::app));
}

gpsshogi::UsiSlave::
~UsiSlave() 
{
}

void gpsshogi::UsiSlave::
start()
{
  status.usi = Initializing;
  {
    std::ostream os(&buffer_w);
    os << "usi\n";
  }
  sending_message = true;
  boost::asio::async_write
    (socket, buffer_w,
     strand.wrap([=](boost::system::error_code ec, size_t)
		 { this->sentUsi(ec); }));
  if (local_log)
    *local_log << Logging::datetime() << "> " << "usi" << "\n" << std::flush;
}

void gpsshogi::UsiSlave::
sentUsi(const boost::system::error_code& ec)
{
  if (ec) {
    Logging::error("$sent usi failed" + idMark() + ec.message());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  sending_message = false;
  boost::asio::async_read_until
    (socket, buffer_r, "\n",
     strand.wrap([=](boost::system::error_code ec, size_t){
	 this->handleOptions(ec); }));
}

void gpsshogi::UsiSlave::
handleOptions(const boost::system::error_code& ec)
{
  std::istream is(&buffer_r);
  std::string line;
  if (! getline(is, line)) {
    Logging::error("$getline failed in options" + idMark());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  if (local_log)
    *local_log << Logging::time() << "< " << line << "\n" << std::flush;
  if (line == "usiok") {
    std::ostringstream ss;
    {
      // for gpsusi
      if (options.count("LimitDepth"))
	ss << "setoption name LimitDepth value 10\n";
      if (options.count("BookDepth"))
	ss << "setoption name BookDepth value 0\n";
      if (options.count("MultiPVWidth")) {
	// multi_pv_key = "MultiPVWidth"; // todo: 0 or 1?
	ss << "setoption name MultiPVWidth value 0\n";
      }
      if (options.count("InputLogFile"))
	ss << "setoption name InputLogFile value\n";
      if (options.count("ErrorLogFile"))
	ss << "setoption name ErrorLogFile value\n";
      if (options.count("Verbsse"))
	ss << "setoption name Verbose value 1\n";
      if (options.count("UsiOutputPawnValue"))
	ss << "setoption name UsiOutputPawnValue value "
	   << usi_pawn_value << "\n";
      // for gpsfish
      if (options.count("OwnBook"))
	ss << "setoption name OwnBook value false\n";
      if (options.count("Hash") && slave_tlp)
	ss << "setoption name Hash value 128\n"; // do not use to much memory
      if (options.count("Threads") && slave_tlp)
	ss << "setoption name Threads value " << slave_tlp << "\n";
      if (options.count("MultiPV")) {
	multi_pv_key = "MultiPV";
	ss << "setoption name MultiPV value 1\n";
      }
      ss << "isready\n";
    }
    std::ostream os(&buffer_w);
    os << ss.str();
    sending_message = true;
    boost::asio::async_write
      (socket, buffer_w,
       strand.wrap([=](boost::system::error_code ec, size_t)
		   {this->sentIsReady(ec);}));
    if (local_log)
      *local_log << Logging::datetime() << "> " << ss.str() << std::flush;
  }
  else {
    std::string option, name, key, type;
    std::istringstream is(line);
    if (is >> option >> name) {
      if (option == "id" && name == "name" && getline(is, client_name)) {
	Logging::info("$connection from " + to_s(socket.remote_endpoint())
		      + idMark() + client_name, true);
      }
      else if ((option == "option" && name == "name" && is >> key >> type)
	  && type == "type") {
	options.insert(key);
      }
    }
    if ((is >> option >> name >> key >> type)
	&& option == "option" && name == "name" && type == "type") {
      options.insert(key);
    }
    boost::asio::async_read_until
      (socket, buffer_r, "\n",
       strand.wrap([=](boost::system::error_code ec, size_t)
		   {this->handleOptions(ec);}));
  }
}

void gpsshogi::UsiSlave::
sentIsReady(const boost::system::error_code& ec)
{
  if (ec) {
    Logging::error("$send isready failed" + idMark() + ec.message());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  sending_message = false;
  boost::asio::async_read_until
    (socket, buffer_r, "\n",
     strand.wrap([=](boost::system::error_code ec, size_t){
	 this->handleReadyOK(ec);}));
}

void gpsshogi::UsiSlave::
handleReadyOK(const boost::system::error_code& ec)
{
  if (ec) {
    Logging::error("$failed to receive readyok" + idMark() + ec.message());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  std::istream is(&buffer_r);
  std::string line;
  if (! getline(is, line)) {
    Logging::error("$getline failed for readyok" + idMark());
    manager.removeSlave(this);
    return;
  }
  if (local_log)
    *local_log << Logging::time() << "< " << line << "\n" << std::flush;
  Logging::slaveUdp(to_s(socket.remote_endpoint()), line);
  if (line == "readyok") {
    std::ostream os(&buffer_w);
    os << "position startpos moves 9i9h\ngo byoyomi 1000\n";
    sending_message = true;
    boost::asio::async_write
      (socket, buffer_w,
       [=](boost::system::error_code ec, size_t){this->sentInitialGo(ec);});
  }
  else {
    Logging::error("$warn expected readyok got " + line);
    boost::asio::async_read_until
      (socket, buffer_r, "\n",
       [=](boost::system::error_code ec, size_t){this->handleReadyOK(ec);});
  }
}

void gpsshogi::UsiSlave::
sentInitialGo(const boost::system::error_code& ec)
{
  if (ec) {
    Logging::error("$send initial go failed" + idMark() + ec.message());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  sending_message = false;
  boost::asio::async_read_until
    (socket, buffer_r, "\n",
     [=](boost::system::error_code ec, size_t){this->handleInitialBestMove(ec);});
}

void gpsshogi::UsiSlave::
handleInitialBestMove(const boost::system::error_code& ec)
{
  if (ec) {
    Logging::error("$failed to receive initial bestmove" + idMark() + ec.message());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  std::istream is(&buffer_r);
  std::string line;
  if (! getline(is, line)) {
    Logging::error("$getline failed for readyok" + idMark());
    manager.removeSlave(this);
    return;
  }
  if (line.find("bestmove") == 0) {
    status.usi = Idle;
    status.last_access = osl::clock::now();
    if (isMateSolver())
      manager.addActiveMateSolver(shared_from_this());
    else
      manager.addActiveSlave(shared_from_this());
    boost::asio::async_read_until
      (socket, buffer_r, "\n",
       [=](boost::system::error_code ec, size_t){this->handleMessage(ec);});
  }
  else {
    boost::asio::async_read_until
      (socket, buffer_r, "\n",
       [=](boost::system::error_code ec, size_t){this->handleInitialBestMove(ec);});
  }
}

void gpsshogi::UsiSlave::
handleMessage(const boost::system::error_code& ec)
{
  // run in io thread
  if (status.usi >= Quitting)
    return;
  if (ec) {
    Logging::error("$error read" + idMark() + ec.message());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  std::istream is(&buffer_r);
  std::string line;
  if (! getline(is, line)) {
    Logging::error("$getline failed in handleMessage" + idMark());
    manager.removeSlave(this);
    return;
  }
  if (local_log)
    if (isMateSolver()
	|| line.find("info")==line.npos
	|| line.find("score")!=line.npos)
      *local_log << Logging::time() << "< " << line << "\n"; // avoid flush
  status.last_access = osl::clock::now();
  if (line.find("ping") == 0) {
    ;
  }
  else if (work_space.listener) {
    Logging::slaveUdp(to_s(socket.remote_endpoint()), line);
    work_space.listener(line);
  }
  else
    Logging::error("$unexpected slave message" + idMark() + line);

  boost::asio::async_read_until
    (socket, buffer_r, "\n",
     strand.wrap([=](boost::system::error_code ec, size_t){this->handleMessage(ec);}));
}

void gpsshogi::UsiSlave::
prepareQuit()
{
  status.usi = Quitting;
  if (!error())
    send("stop\nquit");
}

bool gpsshogi::UsiSlave::
stop(int stop_id)
{
  if (status.usi == Idle || stop_id != work_space.position_id) {
    Logging::warn("$inconsistent stop " + to_s(stop_id)
		  + " != " + to_s(work_space.position_id) + idMark());
    return false;
  }
  if (status.usi == Go || status.usi == SearchStop)
    status.usi = SearchStop;
  else if (status.usi == GoMate || status.usi == CheckmateStop)
    status.usi = CheckmateStop;
  else {
    std::cerr << "unexpected slave status" << idMark() << ' '
	      << status.usi << "\n";
    if (local_log)
      *local_log << "assert\n" << std::flush;
    if (status.usi == Disconnected)
      return false;
  }
  work_space.interim_report.stopped = true;
  send("stop");
  return true;
}

void gpsshogi::UsiSlave::
mateResult(std::function<void(std::string)> output, InterimReport report)
{
  if (report.result_line.empty())
    output("checkmate timeout");
  else
    output(report.result_line);
}

void gpsshogi::UsiSlave::
goMate(int id, const std::string& position, int millisec,
       std::function<void(std::string)> finish_handler)
{
  status.usi = GoMate;
  work_space.reset(this->id(), position, id, 0,
		   [=](InterimReport r){ UsiSlave::mateResult(finish_handler, r); },
		   [=](std::string s){ this->handleMateMessage(s); });
  send(position+"\n"
       +"go mate " + to_s(millisec));
}

void gpsshogi::UsiSlave::
handleMateMessage(const std::string& line)
{
  // run in io thread
  if (line.find("info") == 0) {
    work_space.interim_report.updateByInfo(line, id());
    return;
  }
  if (line.find("checkmate ") == 0) {
    // finished
    work_space.interim_report.finished(line);
    usi_queue.post(std::bind(&UsiSlave::finishedGoOrGoMate, this, line));
  }
  else {
    Logging::error("$unknown mate message" + idMark() + line);
    return;
  }
}

void gpsshogi::UsiSlave::
go(int id, const std::string& position,
   const std::string& ignore_moves, int millisec,
   std::function<void(InterimReport)> progress_handler,
   std::function<void(InterimReport)> finish_handler,
   int draw_value, int multi_pv)
{
  // Logging::info("$go" + idMark() + ' ' + position + ' ' + to_s(millisec));
  if (status.usi != Idle)
    Logging::error("$go not in idle" + idMark() + ' ' + to_s(status.usi));
  status.usi = Go;
  work_space.reset(this->id(), position, id, progress_handler, finish_handler,
		   [=](std::string s) { this->handleSearchMessage(s); });
  std::string msg = "";
  if (options.count("DrawValue") && draw_value)
    msg += "setoption name DrawValue value "+to_s(draw_value)+"\n";
  if (multi_pv_key != "")
    msg += "setoption name "+multi_pv_key+" value "+to_s(multi_pv)+"\n";
  msg += position+"\n";
  if (ignore_moves != "")
    msg += ignore_moves+"\n";
  send(msg + "go byoyomi " + to_s(millisec));
  if (status.timer_waiting)
    Logging::error("|go last timer active? " + idMark());
  timer.expires_from_now(msec500);
  timer.async_wait(strand.wrap
		   (std::bind(&UsiSlave::updateInfoByTimer, this)));
  status.timer_waiting = true;
}

void gpsshogi::UsiSlave::
updateInfoByTimer() {
  if (! status.timer_waiting) {
    // race between timer and timer.cancel() at receiving bestmove.
    // typically caused at winning positions.
    // see InterimReport::updateInfoByTimer "$ignored repeated massages .."
    Logging::info("|updateInfoByTimer not waiting " + idMark());
    return;
  }
  if (status.info_update_needed)
    usi_queue.post(work_space.bindProgress());
  status.timer_waiting = false;
}

void gpsshogi::UsiSlave::
handleSearchMessage(const std::string& line)
{
  // run in io thread
  const bool stopped = status.usi == SearchStop;
  if (line.find("info ") == 0) {
    if (! stopped && work_space.interim_report.updateByInfo(line, id())) {
      if (! status.timer_waiting)
	usi_queue.post(work_space.bindProgress());
      else
	status.info_update_needed = true; // do not disturb tree within 500ms
    }
    return;
  }
  if (line.find("bestmove ") == 0) {
    // finished
    if (! stopped)
      work_space.interim_report.finished(line);
    if (status.info_update_needed) {
      status.info_update_needed = false;
      usi_queue.post(work_space.bindProgress());
    }
    if (status.timer_waiting) {
      status.timer_waiting = false;
      timer.cancel();
    }
    usi_queue.post(std::bind(&UsiSlave::finishedGoOrGoMate, this, line));
  }
  else {
    Logging::error("$unknown search message from" + idMark()
		   + line);
    return;
  }
}

void gpsshogi::UsiSlave::
finishedGoOrGoMate(const std::string& line)
{
  status.update(work_space.interim_report);
  std::function<void(void)> f = work_space.bindFinished();
  work_space.reset(this->id());
  status.usi = Idle;
  f();
}

void gpsshogi::UsiSlave::
echoIfIdle(osl::time_point now)
{
  const double interval = osl::toSeconds(now - status.last_access);
  if (interval+5 < usi_keep_alive)
    return;
  send("echo ping");
}

void gpsshogi::UsiSlave::
send(const std::string& msg)
{
  local_queue.post(std::bind(&UsiSlave::sendLocal, this, msg));
}

void gpsshogi::UsiSlave::
sendLocal(const std::string& msg)
{
  if (error()) {
    Logging::error("|do not send message due to previous error" + idMark() + msg);
    return;
  }
  if (msg != "" && msg.find("\n")+1 == msg.size()) {
    Logging::error("|msg end with lf" + idMark() + msg);
    assert(msg.find("\n")+1 != msg.size());
  }
  if (sending_message) {
    if (msg != "") {
      if (message_waiting != "")
	message_waiting += "\n";
      message_waiting += msg;
    }
    return;
  }
  if (msg == "" && message_waiting == "") 
    return;
  sending_message = true;
  std::ostream os(&buffer_w);
  std::string sent = message_waiting + msg;
  os << sent << "\n";
  message_waiting = "";
  boost::asio::async_write
    (socket, buffer_w,
     [=](boost::system::error_code ec, size_t){
      this->messageSent(message_waiting+msg, ec);});
  if (local_log)
    *local_log << Logging::datetime() << "> " << sent << "\n";
}

void gpsshogi::UsiSlave::
messageSent(std::string msg, const boost::system::error_code& ec)
{
  if (msg == "quit" || msg.find("\nquit\n") == 0
      || boost::algorithm::ends_with(msg, "\nquit")) {
    socket.cancel();
    status.usi = Disconnected;
    return;
  }
  if (ec) {
    Logging::error("$send failed" + idMark() + msg + " " + ec.message());
    last_error = ec;
    manager.removeSlave(this);
    return;
  }
  Logging::slaveUdp(to_s(socket.remote_endpoint()), msg);
  status.last_access = osl::clock::now();
  sending_message = false;  
  if (! message_waiting.empty()) 
    usi_queue.post(std::bind(&UsiSlave::send, this, ""));
}

void gpsshogi::UsiSlave::
sentQuit(const boost::system::error_code& ec)
{
  if (ec) {
    Logging::error("$send quit failed" + idMark() + ec.message());
    last_error = ec;
  }
}

const std::string gpsshogi::UsiSlave::idMark() const
{
  return " [" + to_s(id()) + "] ";
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
