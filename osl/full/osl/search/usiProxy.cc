#include "osl/search/usiProxy.h"
#include "osl/search/interimReport.h"
#include "osl/search/searchMonitor.h"
#include "osl/search/moveWithComment.h"
#include "osl/search/simpleHashTable.h"
#include "osl/usi.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <queue>
#include <system_error>
#include <sys/types.h>
#ifdef _WIN32
#  include <io.h>
#  include <fcntl.h>
#  define pipe(fds) _pipe(fds, 5000, _O_TEXT) 
#else
#  include <sys/wait.h>
#endif
#include <unistd.h>
#include <libgen.h>

/*
 * Not yet supported in Windows. fork() should be replaced by
 * CreateProcess().
 */

struct osl::search::UsiProxy::Proxy
{
  pid_t pid;
  int pipe_in[2], pipe_out[2];
  std::string program;
  FILE *in, *out;
  UsiProxy *current;
  std::atomic_bool quit;
  
  Proxy(const std::string& name) : program(name), current(nullptr) {
#ifndef _WIN32
    ::pipe(pipe_in);
    ::pipe(pipe_out);
    std::cerr << "new proxy " << program << std::endl;
    if ( (pid = fork()) == 0 ) {
      // child
      if (dup2(pipe_out[0], 0) < 0
	  || dup2(pipe_in[1], 1) < 0)
	throw std::runtime_error("error dup2 in child");
      close(pipe_in[0]);
      close(pipe_in[1]);
      close(pipe_out[0]);
      close(pipe_out[1]);
      // close(2);
      char buf[1024]; // broken basename requring (non-const) char *
      strcpy(buf, program.c_str());
      if (chdir(::dirname(buf))) {
	std::cerr << "chdir failed " << dirname(buf) << std::endl;
	throw std::system_error(std::error_code(errno, std::system_category()), "chdir");
      }
      strcpy(buf, program.c_str());
      std::string base = std::string("./") + ::basename(buf);
      /*int err =*/ execlp( base.c_str(), base.c_str(), NULL );
      throw std::system_error(std::error_code(errno, std::system_category()), "execlp");
    }
    else {
      // parent
      close(pipe_in[1]);
      close(pipe_out[0]);
      out = fdopen(pipe_out[1], "w");
      in = fdopen(pipe_in[0], "r");
      setvbuf(out, NULL, _IONBF, 0);
      setvbuf(in, NULL, _IONBF, 0);
      std::thread([=](){
	  while (!quit) { push(this->readLineInThread()); }
	  fclose(in);
	}).detach();
      writeLine("usi");
      while (true) {
	auto line = readLineMayBlock();
	if (line == "usiok") break;
	if (line == "") throw std::runtime_error("UsiProxy usi failed");
	// std::cerr << "< " << line << std::endl;
      }
      writeLine("setoption name Threads value "
		+std::to_string(OslConfig::concurrency()));
      if (OslConfig::concurrency() == 1)
	writeLine("setoption name Hash value "
		  +std::to_string(OslConfig::memoryUseLimit()/1024/1024
				  / (std::thread::hardware_concurrency()*2)));
      for (auto command: initial_commands)
	writeLine(command);
      writeLine("isready");
      while (true) {
	auto line = readLineMayBlock();
	if (line == "readyok") break;
	if (line == "") throw std::runtime_error("UsiProxy isready failed");
	std::cerr << "< " << line << std::endl;
      }
    }
#endif
  }
  ~Proxy() {
    writeLine("quit");
    quit = true;
    std::this_thread::sleep_for(std::chrono::seconds(1));    
    fclose(out);
  }
  void attach(UsiProxy *proxy) {
    std::lock_guard<std::mutex> lk(mutex);
    if (current != nullptr)
      throw std::runtime_error("UsiProxy::Proxy::attach");
    current = proxy;
  }
  void detach(UsiProxy *proxy) {
    std::lock_guard<std::mutex> lk(mutex);
    if (current != proxy)
      throw std::runtime_error("UsiProxy::Proxy::detatch");
    current = nullptr;
  }
  void writeLine(const std::string& msg) {
    std::cerr << "> " << msg << std::endl;
    fprintf(out, "%s\n", msg.c_str());
  }
  bool messageAvailable() {
    std::lock_guard<std::mutex> lk(mutex);
    return queue.size() > 0;
  }
  std::string readLineMayBlock() {
    std::unique_lock<std::mutex> lk(mutex);
    condition.wait(lk, [&](){ return queue.size()>0; });
    auto ans = queue.front();
    queue.pop();
    return *ans;
  }
  std::string readLineInTime(milliseconds msec) {
    std::unique_lock<std::mutex> lk(mutex);
    if (! condition.wait_for(lk, msec, [&](){ return queue.size()>0; }))
      return ""; // timeout
    auto ans = queue.front();
    queue.pop();
    return *ans;
  }
  // private
  std::queue<std::shared_ptr<std::string>> queue;
  std::mutex mutex;
  std::condition_variable condition;
  std::string readLineInThread() {
    char buff[2048];
    if (quit)
      return "";
    if (fgets(buff, sizeof(buff), in) == NULL) {
      quit = true;
      std::cerr << "< (eof)\n";
      return "";
    }
    std::string ans = buff;
    boost::algorithm::trim(ans);
    if (ans.find("info ") != 0)
      std::cerr << "< " << ans << std::endl;
    return ans;
  }
  void push(std::string msg) {
    std::lock_guard<std::mutex> lk(mutex);
    queue.push(std::make_shared<std::string>(msg));
    condition.notify_one();
  }
};

std::queue<std::shared_ptr<osl::search::UsiProxy::Proxy>> osl::search::UsiProxy::proxy_pool;
std::string osl::search::UsiProxy::default_program = OSL_HOME "/../gpsfish_dev/src/gpsfish";
std::vector<std::string> osl::search::UsiProxy::initial_commands;
std::mutex osl::search::UsiProxy::mutex;

bool osl::search::UsiProxy::setUp(const std::string& program,
				  const std::vector<std::string>& commands) {
  if (program != ""
      && ! boost::filesystem::exists(program))
    throw std::runtime_error("UsiProxy::setUp "+program+" not found");

  std::lock_guard<std::mutex> lk(mutex);
  if (program != "")
    default_program = program;
  initial_commands = commands;

  return boost::filesystem::exists(default_program);
}

osl::search::UsiProxy::
UsiProxy(const NumEffectState& s, checkmate_t& checker,
	 SimpleHashTable *t, CountRecorder&)
 : root_ignore_moves(nullptr), prediction_for_speculative_search(false)
{
  {
    std::lock_guard<std::mutex> lk(mutex);
    if (proxy_pool.empty())
      proxy_pool.push(std::make_shared<Proxy>(default_program));
    proxy = proxy_pool.front();
    proxy_pool.pop();
  }
  proxy->attach(this);
  state = s;
  if (t->isVerbose())
    addMonitor(std::shared_ptr<SearchMonitor>(new CerrMonitor()));
}
osl::search::UsiProxy::
UsiProxy::UsiProxy(const UsiProxy& src) 
  : SearchTimer(src), FixedEval(src), state(src.state), history(src.history),
    root_ignore_moves(nullptr), prediction_for_speculative_search(false)
{
  {
    std::lock_guard<std::mutex> lk(mutex);
    if (proxy_pool.empty())
      proxy_pool.push(std::make_shared<Proxy>(default_program));
    proxy = proxy_pool.front();
    proxy_pool.pop();
  }
  proxy->attach(this);  
}

osl::search::UsiProxy::
~UsiProxy()
{
  proxy->writeLine("stop");
  proxy->detach(this);
  {
    std::lock_guard<std::mutex> lk(mutex);
    proxy_pool.push(proxy);
  }
  proxy.reset();
}
osl::Move osl::search::UsiProxy::
computeBestMoveIteratively(int limit, int step, 
			   int initial_limit,
			   size_t node_limit,
			   const TimeAssigned& assign,
			   MoveWithComment *additional_info)
{
  this->setStartTime(clock::now());
  this->setTimeAssign(assign);

  // mismatch: we need the initial state to utilize history
  std::string position;
  position.reserve(16+90+10+5*0);
  position = "position ";
  position += usi::show(state);
  position += " moves";
  proxy->writeLine(position);
  if (root_ignore_moves && !root_ignore_moves->empty()) {
    std::string ignore_moves = "ignore_moves";
    for (auto move:*root_ignore_moves) {
      ignore_moves += " " + usi::show(move);
    }
    proxy->writeLine(ignore_moves);
  }
  proxy->writeLine("go byoyomi "+std::to_string(assign.standard.count()));

  milliseconds msec100(100);
  std::string line;
  InterimReport report;
  while (true) {
    line = proxy->readLineInTime(msec100);
    if (!report.stopped && line.find("info ") == 0
	&& report.updateByInfo(line, -1)) {
      InterimReport info;
      info.set(state.turn(), report);
      std::vector<Move> pv = toCSA(info.pv.moves);
      for (const auto& monitor: this->monitors())
	monitor->showPV(info.depth_head, info.node_count,
			info.elapsed, info.pv.score, 
			pv.empty() ? Move() : *pv.begin(),
			&*pv.begin(), &*pv.begin()+pv.size(), nullptr, nullptr);      
      try {
	throwIfNoMoreTime(info.node_count);
      }
      catch (NoMoreMemory&) {
      }
      catch (misc::NoMoreTime&) {
      }
    }
    if (!report.stopped && this->stopping()) {
      report.stopped = true;
      proxy->writeLine("stop");
    }
    if (line.find("bestmove ") == 0)
      break;
  }

  if (additional_info) {
    InterimReport info;
    info.set(state.turn(), report);
    additional_info->move = info.pv.empty() ? Move() : usi::strToMove(info.pv.moves[0], state);
    additional_info->value = info.pv.score;
    additional_info->moves = toCSA(info.pv.moves);
    if (additional_info->moves.size() > 0)
      additional_info->moves.erase(additional_info->moves.begin());
    additional_info->root = HashKey(state);
    additional_info->node_count = info.node_count;
    additional_info->elapsed = info.elapsed;
    additional_info->root_limit = info.pv.depth;
  }
  for (const auto& monitor:this->monitors())
    monitor->searchFinished();
  std::vector<std::string> elements;
  boost::algorithm::split(elements, line, boost::algorithm::is_any_of(" "));
  if (elements.size() < 2 || elements[0] != "bestmove")
    throw std::logic_error("^unkown usi bestmove "+line);
  Move best_move = usi::strToMove(elements[1], state);
  if (root_ignore_moves && root_ignore_moves->isMember(best_move))
    return Move();
  return best_move;
}
bool osl::search::UsiProxy::
isReasonableMove(Move move, int pawn_sacrifice)
{
  return true;
}

void osl::search::UsiProxy::
setRootIgnoreMoves(const MoveVector *rim, bool prediction)
{
  assert(!prediction || rim);
  root_ignore_moves = rim;
  prediction_for_speculative_search = prediction;
}

void osl::search::UsiProxy::
setHistory(const MoveStack& h)
{
  history = h;
}

std::vector<osl::Move> osl::search::UsiProxy::toCSA(const std::vector<std::string>& pv) const
{
  std::vector<Move> ans;
  NumEffectState work = state;
  for (auto usi_move:pv) {
    try {
      Move move = usi::strToMove(usi_move, work);
      ans.push_back(move);
      work.makeMove(move);
    }
    catch (std::exception& e) {
      std::cerr << "UsiProxy::toCSA " << e.what() << std::endl;
      break;
    }
  }
  return ans;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
