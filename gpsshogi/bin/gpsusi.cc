/* gpsusi.cc
 */
#include "gpsshogi/revision.h"
#include "benchmark.h"

// Note: -DMINIMAL の場合(i.e., gpsusione) の特別動作
// - setoption Thread を無視 (-N x は可)
// - setoption USI_Hash を無視 (常に最大を利用)

#include "osl/usi.h"
#include "osl/book/openingBook.h"
#include "osl/game_playing/usiState.h"
#include "osl/game_playing/speculativeSearchPlayer.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/weightTracer.h"
#include "osl/game_playing/bookPlayer.h"
#include "osl/game_playing/csaLogger.h"
#include "osl/game_playing/usiResponse.h"
#include "osl/search/usiReporter.h"
#include "osl/checkmate/dfpn.h"
#include "osl/checkmate/dfpnParallel.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/eval/progressEval.h"
#include "osl/rating/featureSet.h"
#include "osl/progress.h"
#include "osl/move_probability/featureSet.h"
#include "osl/oslConfig.h"
#include "osl/sennichite.h"
#include "osl/enterKing.h"
#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <condition_variable>
#include <future>
namespace po = boost::program_options;
#include <string>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#  include <malloc.h>
#endif
#include <unistd.h>

#ifndef NDEBUG
const bool debug=true;
#else
const bool debug=false;
#endif

// rewrite asio::tcp::iostream, as it stop support for parallel read/write
// at somepoint betweer 1.42-1.48.
class tcp_istream : public boost::iostreams::source
{
  boost::asio::ip::tcp::socket& socket;
public:
  tcp_istream(boost::asio::ip::tcp::socket& s) : socket(s) {
  }
  std::streamsize read(char* s, std::streamsize n) {
    n = boost::asio::read(socket, boost::asio::buffer(s, 1));
    return n;
  }
};
class tcp_ostream : public boost::iostreams::sink
{
  boost::asio::ip::tcp::socket& socket;
public:
  tcp_ostream(boost::asio::ip::tcp::socket& s) : socket(s) {
  }
  std::streamsize write(const char* s, std::streamsize n) {
    return boost::asio::write(socket, boost::asio::buffer(s, n));
  }
};
  
// ---


bool read_from_file = false;
volatile bool suspend_reading = false;
using namespace osl;
CArray<int,2> bwtime;
MoveVector ignore_moves;
int byoyomi, byoyomi_extension;
bool ponder_enabled = false;
int limit_depth = 10, book_depth = 30, multi_pv_width = 0;
std::string csa_file = "";
std::string input_log_file = "";
std::string error_log_file = "";
int verbose = 0;
// set option 対応にしても良いけどあまり需要ないかも
#ifdef MINIMAL
int book_width_root = 16, book_width = 3;
#else
int book_width_root = 16, book_width = 10;
#endif
uint64_t node_count_hard_limit = 0;
bool new_move_probability = true;
std::string udp_log = "";
boost::asio::io_service io_service;
using boost::asio::ip::udp;
std::unique_ptr<udp::socket> udp_socket;
std::unique_ptr<udp::endpoint> udp_endpoint;
std::string hostname = "";

bool is_verbose()
{
  return debug || verbose || ponder_enabled || error_log_file != "";
}
void send_udp_log(std::string line) 
{
  if (! udp_socket)
    return;
  assert(line.find('\n') == line.npos);
  boost::system::error_code ignored_error;
  line += "\n";
  if (hostname != "")
    line = hostname + " " + line;
  udp_socket->send_to(boost::asio::buffer(line.c_str(), line.length()),
		      *udp_endpoint, 0,
		      ignored_error);
}

std::atomic_int go_count(0);
std::atomic_int bestmove_count(0);
std::atomic_int search_count(0);
std::atomic_bool last_go_is_checkmate(false);
std::atomic_bool stop_checkmate(false);
std::mutex go_stop_mutex;
std::condition_variable go_stop_condition;

std::unique_ptr<std::ofstream> csa_stream;
std::unique_ptr<game_playing::CsaLogger> csa_logger;
struct StopWatch
{
  time_point prev;
  void init()
  {
    prev = clock::now();
  }
  int elapsedCSA() 
  {
    time_point now = clock::now();
    int result = std::max(1, (int)floor(toSeconds(now - prev)));
    prev = now;
    return result;
  }
} stop_watch;

class StatefulPlayer
{
  UsiState usi_state;
  game_playing::GameState state;
  std::unique_ptr<game_playing::ComputerPlayer> player;
  CArray<int,2> initial_time;
  Player my_turn;
  std::ostream& os;
public:
  mutable std::mutex mutex;
  StatefulPlayer(const UsiState& initial_state,
		 game_playing::ComputerPlayer *p,
		 Player turn, std::ostream& o)
    : usi_state(initial_state), state(initial_state.initial_state),
      player(p), my_turn(turn), os(o)
  {
    initial_time.fill(-1);
    if (csa_logger) {
      if (turn == BLACK)
	csa_logger->init("gpsshogi", "human", state.state());
      else
	csa_logger->init("human", "gpsshogi", state.state());
    }
    for (Move m: usi_state.moves) {
      state.pushMove(m);
      player->pushMove(m);
      if (csa_logger)
	csa_logger->pushMove(m, 0);
    }
    stop_watch.init();
  }
  void search()
  {
    if (is_verbose())
      std::cerr << state.state() << "\n";
    assert(state.state().turn() == my_turn);
    if (initial_time[BLACK] < 0) initial_time[BLACK] = bwtime[BLACK];
    if (initial_time[WHITE] < 0) initial_time[WHITE] = bwtime[WHITE];
    MoveWithComment ret;
    try
    {
      if (! ignore_moves.empty())
	player->setRootIgnoreMoves(&ignore_moves, false);
      {
	std::lock_guard<std::mutex> lk(go_stop_mutex);
	++search_count;
	go_stop_condition.notify_all();
      }
      if (byoyomi_extension > 0) {
	assert(byoyomi > 0);
	const osl::milliseconds b(byoyomi);
	const osl::milliseconds bmax(byoyomi_extension);
	osl::search::TimeAssigned msec(b, bmax);
	if (osl::game_playing::ComputerPlayerSelectBestMoveInTime *p
	    = dynamic_cast<osl::game_playing::ComputerPlayerSelectBestMoveInTime *>(player.get()))
	  ret = p->selectBestMoveInTime(state, msec);
	else
	  throw std::runtime_error("type error in byoyomi_extension");
      }
      else
	ret = player->selectBestMove(state, initial_time[my_turn]/1000, 
				     (initial_time[my_turn]-bwtime[my_turn])/1000, 
				     byoyomi/1000);
    }
    catch (std::exception& e) {
      std::cerr << "search failed by exception: " << e.what() << "\n";
      throw;
    }
    catch (...) {
      std::cerr << "search failed\n";
      throw;
    }
    if (ret.move.isNormal()) {
      std::lock_guard<std::mutex> lk(mutex);
      usi_state.moves.push_back(ret.move);
      state.pushMove(ret.move);
      player->pushMove(ret.move);
      if (csa_logger)
	csa_logger->pushMove(ret, stop_watch.elapsedCSA());
    } else {
      std::lock_guard<std::mutex> lk(mutex);
      usi_state.aborted = true;
    }
    player->setRootIgnoreMoves(0, false);
    ignore_moves.clear();
    std::string result = "bestmove " + usi::show(ret.move);
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      os << result << "\n" << std::flush;
    }
    if (is_verbose())
      std::cerr << "sent: " << result << "\n";
    send_udp_log(result);
    suspend_reading = false;
    std::lock_guard<std::mutex> lk(go_stop_mutex);
    ++bestmove_count;
    go_stop_condition.notify_all();
  }
  void newMove(Move move)
  {
    if (csa_logger)
      csa_logger->pushMove(move, stop_watch.elapsedCSA());
    {
      std::lock_guard<std::mutex> lk(mutex);
      usi_state.moves.push_back(move);
      state.pushMove(move);
      player->pushMove(move);
    }
    assert(move.player() != my_turn);
    search();
  }
  const UsiState current() const
  {
    std::lock_guard<std::mutex> lk(mutex);
    return usi_state;
  }
  void stopSearchNow()
  {
    player->stopSearchNow();
  }
  void needRestartNext()
  {
    usi_state.aborted = true;
  }
};
std::unique_ptr<StatefulPlayer> player;

struct Queue 
{
  typedef std::deque<std::shared_ptr<Move> > queue_t;
  queue_t data;
  typedef std::mutex Mutex;
  mutable Mutex mutex;
  std::atomic_bool finish;
  std::condition_variable condition;

  Queue() : finish(false) {
  }
  ~Queue() {
    if (! finish)
      quit();
  }
  size_t size() const {
    std::lock_guard<Mutex> lk(mutex);
    return data.size();
  }
  void push_back(std::shared_ptr<Move>&& ptr) {
    std::lock_guard<Mutex> lk(mutex);
    data.push_back(std::shared_ptr<Move>());
    data.back().swap(ptr);
    condition.notify_one();
  }
private:
  std::shared_ptr<Move> pop_front_in_lock()
    {
      if (! data.empty())
      {
	std::shared_ptr<Move> result = data.front();
	data.pop_front();
	return result;
      }
      return std::shared_ptr<Move>();
    }
public:
  std::shared_ptr<Move> pop_front_non_block() {
    std::lock_guard<Mutex> lk(mutex);
    return pop_front_in_lock();
  }
  std::shared_ptr<Move> pop_front() {
    while (! finish) {
      std::unique_lock<Mutex> lk(mutex);
      std::shared_ptr<Move> result = pop_front_in_lock();
      if (result.get() || finish)
	return result;
      condition.wait(lk);
    }
    return std::shared_ptr<Move>();
  }
  void quit(int seconds=0) {
    finish = true;
    if (seconds > 0)
      std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::unique_lock<Mutex> lk(mutex);
    condition.notify_all();
  }
  void pop(Move& move)  {
    move = *pop_front();
  }
  bool pop_if_present(Move& move)  {
    std::shared_ptr<Move> ret = pop_front_non_block();
    if (ret)
      move = *ret;
    return ret.get() != nullptr;
  }
  void push(Move move) {
    push_back(std::make_shared<Move>(move));
  }
};

Queue queue;

struct ThreadRun
{
  ThreadRun()
  {
  }
  void operator()() const
    __attribute__((noinline))
#ifdef _WIN32
    __attribute__((force_align_arg_pointer))
#endif
  {
    if (! goodAlignment())
      return;

    player->search(); 
    while (true) {
      if (debug)
	std::cerr << "wait for opponent\n";
      Move move;
      queue.pop(move);
      if (debug)
	std::cerr << "got " << move << "\n";
      if (! move.isNormal())
	return;
      player->newMove(move);
    }
  }
  static bool goodAlignment();
};

std::unique_ptr<UsiState> input_state;
std::unique_ptr<std::thread> search_thread;

game_playing::ComputerPlayer *makePlayer(Player turn, const SimpleState&, std::ostream&);
Player input_turn() 
{
  Player turn = input_state->initial_state.turn();
  if (! input_state->moves.empty())
    turn = alt(input_state->moves.back().player());
  return turn;
}
void do_search(std::ostream& os)
{
  if (search_thread && input_state->isSuccessorOf(player->current())) {
    if (debug)
      std::cerr << "game continued\n";
    queue.push(input_state->moves.back());
    return;
  }
  if (search_thread) {
    queue.push(Move());
    search_thread->join();
    Move dummy;
    while (queue.pop_if_present(dummy))
      ;
  }
  const Player turn = input_turn();
  if (debug)
    std::cerr << "new game " << turn << "\n";
  if (csa_file != "") {
    csa_stream.reset(new std::ofstream(csa_file.c_str()));
    csa_logger.reset(new game_playing::CsaLogger(*csa_stream));
  } 
  game_playing::ComputerPlayer *searcher = makePlayer(turn, input_state->initial_state, os);
  player.reset(new StatefulPlayer(*input_state, searcher, turn, os));
  search_thread.reset(new std::thread(ThreadRun()));  

  std::unique_lock<std::mutex> lk(go_stop_mutex);
  while (go_count > search_count)
    go_stop_condition.wait(lk);
}

#ifdef OSL_DFPN_SMP
struct GoMate
{
  checkmate::DfpnParallel& dfpn;
  NumEffectState& state;
  PathEncoding path;
  Move& checkmate_move;
  std::vector<Move>& pv;
  ProofDisproof& result;
  GoMate(checkmate::DfpnParallel& d, NumEffectState& s, PathEncoding p, 
	 Move& c, std::vector<Move>& v, ProofDisproof& r)
    : dfpn(d), state(s), path(p), checkmate_move(c), pv(v), result(r)
  {
  }
  void operator()()
  {
    result = dfpn.
      hasCheckmateMove(state, HashKey(state), path, 
		       std::numeric_limits<size_t>::max(), checkmate_move, Move(), &pv);
  }
};
#endif

struct GoMateThread
{
  struct NotifyLock
  {
    NotifyLock()
    {
      std::lock_guard<std::mutex> lk(go_stop_mutex);
      ++search_count;
      go_stop_condition.notify_all();
    }
    ~NotifyLock()
    {
      std::lock_guard<std::mutex> lk(go_stop_mutex);
      ++bestmove_count;
      go_stop_condition.notify_all();
      stop_checkmate = false;
    }
  };
  double seconds;
  std::ostream& os;
  explicit GoMateThread(double s, std::ostream& o) : seconds(s), os(o)
  {
  }
  void operator()();
};

void GoMateThread::operator()() 
{
  NotifyLock lock;
  NumEffectState state(input_state->currentState());
#if (! defined ALLOW_KING_ABSENCE)
  if (state.kingSquare(state.turn()).isPieceStand()) {
    const char *msg = "checkmate notimplemented";
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      os << msg << "\n";
    }
    send_udp_log(msg);
    return;
  }
#endif  
  checkmate::DfpnTable table(state.turn());
#ifndef MINIMAL
  table.setGrowthLimit(3000000ull*OslConfig::memoryUseLimit()/(1024*1024*1024));
#endif
  const PathEncoding path(state.turn());
  Move checkmate_move;
  std::vector<Move> pv;
  if (debug)
    std::cerr << state;
  checkmate::DfpnParallel dfpn(std::min(OslConfig::concurrency(), 8));
  dfpn.setTable(&table);
  time_point start = clock::now();

  auto future = std::async
    (std::launch::async,
     [&](){ return dfpn.hasCheckmateMove
	   (state, HashKey(state), path, 
	    std::numeric_limits<size_t>::max(), checkmate_move, Move(), &pv); });

  double wait = 450;
  for (int i=0; true; ++i) {
    if (future.wait_for(std::chrono::milliseconds((int)wait))
	!=std::future_status::timeout)
      break;
    double elapsed = elapsedSeconds(start);
    size_t node_count = dfpn.nodeCount();
    if (i % 4 == 0) {
      double memory = OslConfig::memoryUseRatio();
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      os << "info time " << static_cast<int>(elapsed*1000)
	 << " nodes " << node_count << " nps " << static_cast<int>(node_count/elapsed)
	 << " hashfull " << static_cast<int>(memory*1000) << "\n";
    }
    if ((seconds*0.94-elapsed)*1000 < 110 || stop_checkmate)
      break;
    wait = std::min((seconds*0.94-elapsed)*1000 - 100, 250.0);
  }
  dfpn.stopNow();
  ProofDisproof result = future.get();

  if (is_verbose())
    std::cerr << "elapsed " << elapsedSeconds(start) << "\n";
  if (! result.isFinal()) {
    const char *msg = "checkmate timeout";
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      os << msg << "\n" << std::flush;
    }
    send_udp_log(msg);
    return;
  }
  if (! result.isCheckmateSuccess()) {
    const char *msg = "checkmate nomate";
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      os << msg << "\n" << std::flush;
    }
    send_udp_log(msg);
    return;
  }
  std::string msg = "checkmate";
  for (size_t i=0; i<pv.size(); ++i)
    msg += " " + usi::show(pv[i]);
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    os << msg << "\n" << std::flush;
  }
  send_udp_log(msg);
}

void do_checkmate(double seconds, std::ostream& os)
{
  std::thread(GoMateThread(seconds, os)).detach();
}

void play_by_book(int depth_limit, std::ostream& os)
{
  static osl::book::WeightedBook book(OslConfig::openingBook());
  if (HashKey(input_state->initial_state)
      == HashKey(SimpleState(HIRATE))
      && (int)input_state->moves.size() < depth_limit)
  {
    game_playing::GameState state(input_state->initial_state);
    game_playing::WeightTracer tracer
      (book, is_verbose(), book_width_root, book_width);
    for (Move m: input_state->moves) {
      tracer.update(m);
      state.pushMove(m);
    }
    if (! tracer.isOutOfBook()) {
      const Move best_move = tracer.selectMove();
      if (best_move.isNormal()
	  && (! state.isIllegal(best_move))) {
	std::string result = "bestmove " + usi::show(best_move);
	{
	  std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	  os << result << "\n" << std::flush;
	}
	if (is_verbose())
	  std::cerr << "sent: " << result << "\n";
	send_udp_log(result);
	return;
      }
    }
  }
  const char *msg = "bestmove pass";
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    os << msg << "\n" << std::flush;
  }
  if (is_verbose())
    std::cerr << "sent: " << msg << "\n";
  send_udp_log(msg);
}

void play_declare_win(std::ostream& os)
{
  const bool can_declare_win
    = EnterKing::canDeclareWin(input_state->currentState());
  const std::string msg = std::string("bestmove ")
    + (can_declare_win ? "win" : "pass");
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    os << msg << "\n" << std::flush;
  }
  if (is_verbose())
    std::cerr << "sent: " << msg << "\n";
  send_udp_log(msg);
}

std::string make_filename()
{
  return OslConfig::gpsusiConf();
}

void read_config_file()
{
  std::string filename = make_filename();
  if (debug)
    std::cerr << "reading " << filename << "\n";
  std::ifstream fs(filename.c_str());
  std::string key;
  std::string line;
  while (std::getline(fs, line)) {
    std::istringstream is(line);
    is >> key;
    if (key == "LimitDepth") {
      int value;
      if (is >> value)
	limit_depth = value;
      continue;
    }
    if (key == "BookDepth") {
      int value;
      if (is >> value)
	book_depth = value;
      continue;
    }
    if (key == "MultiPVWidth") {
      int value;
      if (is >> value)
	multi_pv_width = value;
      continue;
    }
    if (key == "CSAFile") {
      std::string value;
      if (is >> value)
	csa_file = value;
      continue;
    }
    if (key == "InputLogFile") {
      std::string value;
      if (is >> value)
	input_log_file = value;
      continue;
    }
    if (key == "ErrorLogFile") {
      std::string value;
      if (is >> value)
	error_log_file = value;
      continue;
    }
    if (key == "Verbose") {
      int value;
      if (is >> value)
	verbose = value;
      continue;
    }
    if (key == "UsiOutputPawnValue") {
      int value;
      if (is >> value)
	OslConfig::setUsiOutputPawnValue(value);
      continue;
    }
#ifdef OSL_SMP
    if (key == "Thread") {
      int value;
      if (is >> value) {
#  ifndef MINIMAL
	OslConfig::setNumCPUs(value);
#  else
	if (value != OslConfig::concurrency())
	  std::cerr << "ignored Thread config " << value << ' ' << OslConfig::concurrency()
		    << "\n";
#  endif
      }
      continue;
    }
#endif
  }
  
}
void write_config_file()
{
  std::string filename = make_filename();
  std::ofstream os(filename.c_str());
  os << "LimitDepth " << limit_depth << "\n";
  os << "BookDepth " << book_depth << "\n";
  os << "MultiPVWidth " << multi_pv_width << "\n";
  os << "CSAFile " << csa_file << "\n";
  os << "InputLogFile " << input_log_file << "\n";
  os << "ErrorLogFile " << error_log_file << "\n";
  os << "Verbose " << verbose << "\n";
  os << "UsiOutputPawnValue " << OslConfig::usiOutputPawnValue() << "\n";
  os << "Thread " << OslConfig::concurrency() << "\n";
}

void setOption(std::string& line) 
{
  std::istringstream ss(line);
  std::string set_option, name_str, name, value_str;
  ss >> set_option >> name_str >> name;
#ifndef MINIMAL
  // for safety against human errors, ignore options that can be fixed in advance.
  if (name == "USI_Hash") {
    size_t value;
    if (ss >> value_str >> value) {
      value = std::max(value, (size_t)300);
      value *= 1024*1024;
      value = std::min(value, OslConfig::memoryUseLimit());
      OslConfig::setMemoryUseLimit(value);
      return;
    } else {
      std::cerr << "error setoption " << line << "\n";
    }
  }
#endif
  if (name == "USI_Ponder") {
    std::string value;
    ss >> value_str >> value;
    if (ponder_enabled != (value == "true")
	&& player)
      player->needRestartNext();
    ponder_enabled = (value == "true");
    return;
  }
  if (name == "LimitDepth") {
    int value;
    if (ss >> value_str >> value) {
      if (limit_depth != value && player)
	player->needRestartNext();
      limit_depth = value;
    } else {
      std::cerr << "error setoption " << line << "\n";
    }
  }
  else if (name == "BookDepth") {
    int value;
    if (ss >> value_str >> value) {
      if (book_depth != value && player)
	player->needRestartNext();
      book_depth = value;
    } else {
      std::cerr << "error setoption " << line << "\n";
    }
  }
  else if (name == "MultiPVWidth") {
    int value;
    if (ss >> value_str >> value) {
      if (multi_pv_width != value && player)
	player->needRestartNext();
      multi_pv_width = value;
    } else {
      std::cerr << "error setoption " << line << "\n";
    }
  }
  else if (name == "CSAFile") {
    std::string value;
    ss >> value_str >> value;
    csa_file = value;
  }
  else if (name == "InputLogFile") {
    std::string value;
    ss >> value_str >> value;
    input_log_file = value;
  }
  else if (name == "ErrorLogFile") {
    std::string value;
    ss >> value_str >> value;
    error_log_file = value;
  }
  else if (name == "Verbose") {
    int value;
    if (ss >> value_str >> value) {
      if (verbose != value && player)
	player->needRestartNext();
      verbose = value;
    } else {
      std::cerr << "error setoption " << line << "\n";
    }
  }
  else if (name == "UsiOutputPawnValue") {
    int value;
    if (ss >> value_str >> value) {
      OslConfig::setUsiOutputPawnValue(value);
    } else {
      std::cerr << "error setoption " << line << "\n";
    }
  }
  else if (name == "Thread") {
    int value;
    if (ss >> value_str >> value) {
#ifndef MINIMAL
      OslConfig::setNumCPUs(value);
#else
      if (value != OslConfig::concurrency())
	std::cerr << "ignored Thread config " << value 
		  << " v.s. " << OslConfig::concurrency()
		  << "\n";
#endif
    } else {
      std::cerr << "error setoption " << line << "\n";
    }
  }
  write_config_file();
}

void do_stop() 
{
  {
    std::lock_guard<std::mutex> lk(go_stop_mutex);
    if (go_count <= bestmove_count) {
      std::cerr << "warning stop ignored (go " << go_count
		<< " times, bestmove " << bestmove_count << " times)\n";
      return;
    }
  }
  if (last_go_is_checkmate)
    stop_checkmate = true;
  else {
    player->stopSearchNow();
    player->needRestartNext();
  }
  std::unique_lock<std::mutex> lk(go_stop_mutex);
  while (go_count > bestmove_count)
    go_stop_condition.wait(lk);
}

#ifdef _WIN32
void play(std::istream& is, std::ostream& os) __attribute__((noinline, force_align_arg_pointer));
#endif
void play(std::istream& is, std::ostream& os)
{
  if (udp_log != "" && ! udp_socket) {
    std::vector<std::string> args;
    boost::algorithm::split(args, udp_log, boost::algorithm::is_any_of(":"));

    using boost::asio::ip::udp;
    udp::resolver resolver(io_service);
    udp::resolver::query query(udp::v4(), args[0], args[1]);

    udp_socket.reset(new udp::socket(io_service));
    udp_socket->open(udp::v4());
    udp_endpoint.reset(new udp::endpoint(*resolver.resolve(query)));
  }
  std::string line;
  std::getline(is, line);
#ifdef GPSUSIONE
  std::string name = "id name gpsusione ";
#else
  std::string name = "id name gpsshogi ";
#endif
#ifdef OSL_SMP
  name += "(smp) ";
#endif
  if (hostname != "")
    name += hostname + " ";
  os << name << gpsshogi::gpsshogi_revision
     << " " << OslConfig::configuration()
     << "\n";
  os << "id author teamgps\n";
  os << "option name LimitDepth type spin default " << limit_depth << " min 4 max 10\n";
  os << "option name BookDepth type spin default " << book_depth << " min 0 max 100\n";
  os << "option name MultiPVWidth type spin default " << multi_pv_width << " min 0 max 1000\n";
  os << "option name CSAFile type string default "
     << ((csa_file != "") ? csa_file : std::string("<empty>")) << "\n";
  os << "option name InputLogFile type string default "
     << ((input_log_file != "") ? input_log_file : std::string("<empty>")) << "\n";
  os << "option name ErrorLogFile type string default "
     << ((error_log_file != "") ? error_log_file : std::string("<empty>")) << "\n";
  os << "option name Verbose type spin default " << verbose << " min 0 max 1\n";
  os << "option name UsiOutputPawnValue type spin default " << OslConfig::usiOutputPawnValue() << " min 100 max 10000\n";
#  ifdef OSL_SMP
  os << "option name Thread type spin default " << OslConfig::concurrency() << " min 1 max "
     << std::max(1u, std::thread::hardware_concurrency()) << "\n";
#  endif
  os << "usiok\n" << std::flush;
  std::unique_ptr<std::ofstream> input_log_stream;
  if (input_log_file != "") {
    input_log_stream.reset(new std::ofstream(input_log_file.c_str()));
    *input_log_stream << line << "\n" << std::flush;
  }
  input_state.reset(new UsiState);  
  UsiResponse lightweight_commands(*input_state,
				   new_move_probability,
				   is_verbose());
  std::string response;
  response.reserve(2000);
  bool show_date = false;
  while (true) {
    const bool prompt = (debug || &is != &std::cin);
    if (prompt && show_date)
      std::cerr << "readyinput\n";
    if (! std::getline(is, line))
      break;
    if (prompt) {
      show_date = line.find("echo")>0 && line.find("setoption")>0;
      if (show_date) {
	const time_t now = time(0);
	char ctime_buf[64];
	std::cerr << ctime_r(&now, ctime_buf);
      }
      std::cerr << "recv: " << line << "\n" << std::flush;
    }
    if (input_log_stream)
      *input_log_stream << line << "\n" << std::flush;
    send_udp_log(line);    
    if (line == "isready") {
      static bool initalized = false;
      if (! initalized) {
	if (error_log_file != "")
	  freopen(error_log_file.c_str(), "w", stderr);
	eval::ml::OpenMidEndingEval::setUp();
	progress::ml::NewProgress::setUp();
	rating::StandardFeatureSet::instance();
	lightweight_commands.hasImmediateResponse("genmove_probability", response); // setup
	initalized = true;
      }
      const char *msg = "readyok";
      os << msg << "\n" << std::flush;
      if (debug)
	std::cerr << msg << "\n";
      send_udp_log(msg);
      continue;
    }
    if (line.find("setoption") == 0) {
      setOption(line);
      continue;
    }
    if (line.find("usinewgame") == 0)
      continue;
    if (line.find("stop") == 0) {
      do_stop();
      continue;
    }
    if (line.find("quit") == 0) {
      if (search_thread) {
	queue.push(Move());
	search_thread->join();
      }
      return;
    }

    // extended commands
    if (line.find("sleep") == 0) {
      std::this_thread::sleep_for(std::chrono::seconds(60));
      continue;
    }

    if (lightweight_commands.hasImmediateResponse(line, response)) {
      {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	os << response << "\n" << std::flush;
      }
      send_udp_log(response);
      continue;
    }

    // search related commands 
    // do not read next search command while previous search is in progress
    while (read_from_file && suspend_reading)
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (line.find("position") == 0) {
      try {
	input_state->parseUsi(line);
	if (is_verbose())
	  std::cerr << "new position " << input_state->moves.size()
		    << " " << line << "\n";
      }
      catch (std::exception& e) {
	std::cerr << "usi parse error " << e.what() << "\n";
	throw e;
      }
      continue;
    }
    if (line.find("go") == 0) {
      std::unique_lock<std::mutex> lk(go_stop_mutex);
      OslConfig::setDfpnMaxDepth(256);
      while (go_count > bestmove_count) {
	std::cerr << "delay go (go " << go_count
		  << " times, bestmove " << bestmove_count << " times)\n";
	go_stop_condition.wait(lk);
      }
      last_go_is_checkmate = false;
    }
    if (line.find("go mate") == 0) {
      double seconds = 60, millisec;
      std::string option;
      std::istringstream is(line.substr(strlen("go mate")));
      if (is >> millisec)
	seconds = std::max(std::min(seconds, millisec/1000.0), 0.5);
      {
	std::lock_guard<std::mutex> lk(go_stop_mutex);
	++go_count;
	last_go_is_checkmate = true;
	OslConfig::setDfpnMaxDepth(1600);
      }
      do_checkmate(seconds, os);
      continue;
    }
    if (line.find("go book") == 0) {
      int limit = book_depth, value;
      std::istringstream is(line.substr(strlen("go book")));
      if (is >> value)
	limit = value;
      play_by_book(limit, os);
      continue;
    }
    if (line.find("go declare_win") == 0) {
      play_declare_win(os);
      continue;
    }
    if (line.find("go ") == 0) {
      bwtime.fill(1000);
      byoyomi = 0; byoyomi_extension = 0;
      std::string option;
      std::istringstream is(line.substr(3));
      while (is >> option) {
	if (option == "btime")
	  is >> bwtime[BLACK];
	else if (option == "wtime")
	  is >> bwtime[WHITE];
	else if (option == "byoyomi")
	  is >> byoyomi;
	else if (option == "byoyomi_extension")
	  is >> byoyomi_extension;
	else if (option == "infinite")
	  byoyomi = 60*60*1000; // 1hour
	else {
	  std::string err
	    = "unknown option in go " + option;
	  std::cerr << err << "\n";
	  throw std::runtime_error(err);
	}
      }
      if (byoyomi_extension && (byoyomi == 0 || ponder_enabled)) {
	std::cerr << "warning ignored byoyomi_extension\n";
	byoyomi_extension = 0;
      }
      suspend_reading = true;
      do_search(os);
      std::lock_guard<std::mutex> lk(go_stop_mutex);
      ++go_count;
      continue;
    }
    // ponderhit
    if (line.find("gameover") == 0)
      continue;
    // non-standard commands
    if (line.find("ignore_moves") == 0) {
      assert(ignore_moves.empty());
      if (book_depth > 0) {
	std::cerr << "error ignore_moves not supported with book\n";
	continue;
      }
      input_state->parseIgnoreMoves(line, ignore_moves);
      std::cerr << "accept ignore_moves\n";
      if (player)
	player->needRestartNext();
      continue;
    }
    if (line.find("query progress") == 0) {
      if (input_state) {
	NumEffectState state(input_state->currentState());
	const osl::progress::Effect5x3 progress(state);
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	os << "answer progress " << progress.progress16().value()
	   << ' ' << progress.progress16(BLACK).value()
	   << ' ' << progress.progress16(WHITE).value()
	   << "\n";
      }
      else {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	os << "error position not specified\n";
      }
      continue;
    }
    if (line.find("query eval") == 0) {
      if (input_state) {
	NumEffectState state(input_state->currentState());
	const osl::eval::ml::OpenMidEndingEval eval(state);
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	os << "answer eval " << eval.value()
	   << ' ' << eval.captureValue(osl::newPtypeO(osl::WHITE,osl::PAWN))/2
	   << ' ' << eval.progressIndependentValue()
	   << ' ' << eval.openingValue()
	   << ' ' << eval.midgameValue()
	   << ' ' << eval.midgame2Value()
	   << ' ' << eval.endgameValue()
	   << "\n";
      }
      else {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	os << "error position not specified\n";
      }
      continue;
    }
    if (line.find("query searchtime ") == 0) {
      if (input_state) {
	std::istringstream is(line);
	std::string dummy;
	int amount, byoyomi, elapsed;
	if (is >> dummy >> dummy >> amount >> byoyomi >> elapsed) {
	  osl::game_playing::GameState state(input_state->initial_state);
	  for (Move m: input_state->moves) 
	    state.pushMove(m);
	  const osl::search::TimeAssigned assigned
	    = osl::game_playing::SearchPlayer::assignTime(state, amount, elapsed, byoyomi, 0);
	  std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	  os << "answer searchtime " << assigned.standard.count()
	     << ' ' << assigned.max.count() << "\n";
	}
	else {
	  std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	  os << "error in arguments";
	}
      }
      else {
	os << "error position not specified\n";
      }
      continue;      
    }
    if (line.find("query") == 0) {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      os << "error unknown query\n";
      continue;      
    }    
    if (line.find("open ") == 0) {
      bool ok = true;
      try {
	input_state->openFile(line.substr(5));
      }
      catch (CsaIOError& e) {
	std::cerr << e.what() << "\n";
	ok = false;
      }
      if (ok) {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	os << "position "
	   << usi::show(input_state->initial_state)
	   << " moves";
	for (Move move: input_state->moves)
	  os << " " << usi::show(move);
	os << "\n";
	os << "openok\n";	
      }
      if (! ok)
	std::cerr << "open failed " << line << "\n";
      continue;
    }
    std::cerr << "unknown command " << line << "\n";
  }
  if (read_from_file) {
    std::cerr << "end of input\n";
  } else {
    std::cerr << "input stream closed\n";
  }
  const int sleep_seconds = 10;
  std::cerr << "--- trying safe shutdown in "
	    << sleep_seconds << "s. ---\n";
  do_stop();
  if (search_thread) {
    queue.push(Move());
    search_thread->join();
  }
  std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds));
}

int main(int argc, char **argv)
{
  OslConfig::setUp();
  int num_benchmark, benchmark_seconds;
  po::options_description options("Options");
  std::string io_stream;
#ifndef MINIMAL
  unsigned int eval_random;
#endif
#ifdef OSL_SMP
  int num_cpus;
#endif
  double memory_use_percent;
  bool extended_usi;
  options.add_options()
    ("benchmark", "test search performance")
    ("benchmark-single", "test search performance")
    ("benchmark-more", 
     po::value<int>(&num_benchmark)->default_value(0),
     "number of problems for benchmark")
    ("benchmark-seconds", 
     po::value<int>(&benchmark_seconds)->default_value(30),
     "seconds for benchmark")
    ("profile", "profile mode (run in 1 thread even when OSL_SMP is defined)")
    ("io-stream", po::value<std::string>(&io_stream)->default_value(""), "input for debug")
    ("udp-logging", po::value<std::string>(&udp_log)->default_value(""), "host:port for udp logging")
    ("book-width", 
     po::value<int>(&book_width)->default_value(book_width),
     "relative width of book")
    ("book-width-root", 
     po::value<int>(&book_width_root)->default_value(book_width_root),
     "relative width of book at the initial position")
#ifndef MINIMAL
    ("eval-randomness",
     po::value<unsigned int>(&eval_random)->default_value(0),
     "add random value generated by normal distribution of a given standard deviation, to evaluation values")
    ("node-count-hard-limit",
     po::value<uint64_t>(&node_count_hard_limit)->default_value(0),
     "nodes limit for each search (0 for infinity)")
#endif
#ifdef OSL_SMP
    ("num-cpus,N",
     po::value<int>(&num_cpus)->default_value(-1),
     "num cpus for parallel search")
#endif
    ("new-move-probability-in-genmove-logprobability",
     po::value<bool>(&new_move_probability)->default_value(true),
     "use new (experimental) move probability for genmove_probability")
    ("memory-use-percent",
     po::value<double>(&memory_use_percent)->default_value(100.0),
     "percentage for memory use (normally 100)")
    ("extended-usi",
     po::value<bool>(&extended_usi)->default_value(false),
     "use extended usi (currently, pv will be shown in extended format)")
    ("health-check", "test whether data files are properly placed")
    ("help,h", "produce help message")
    ("version", "show version info")
    ;
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    throw;
  }
  if (vm.count("health-check")) {
    bool ok = OslConfig::healthCheck();
    ok &= rating::StandardFeatureSet::healthCheck();
    ok &= move_probability::StandardFeatureSet::healthCheck();
    if (! ok) 
      std::cerr << "health check failed\n";
    return ok ? 0 : 1;
  }
  if (vm.count("help")) {
    std::cout << options << std::endl;
    return 0;
  }
  if (vm.count("version")) {
    std::cout << "gpsusi " << gpsshogi::gpsshogi_revision << "\n\n"
      << "Copyright (C) 2003-2011 Team GPS.\n";
    std::cout << "osl " << OslConfig::configuration() << '\n';
    return 0;
  }
  char name[128] = {0};
  if (gethostname(name, 100) == 0) {
    hostname = name;
    if (hostname.find(".") != hostname.npos)
      hostname.resize(hostname.find("."));
  }
  osl::OslConfig::setMemoryUsePercent(memory_use_percent);
#ifndef MINIMAL
  if (eval_random)
    OslConfig::setEvalRandom(eval_random);
#endif
  if (vm.count("benchmark")
      || vm.count("benchmark-single")
      || num_benchmark) {
#ifdef OSL_SMP
    if (num_cpus > 0 && num_cpus != OslConfig::concurrency()
	&& ! vm.count("profile")) {
      std::cerr << "set num-cpus " << OslConfig::concurrency() << " => " << num_cpus << "\n";
      OslConfig::setNumCPUs(num_cpus);
    }
#endif
    eval::ml::OpenMidEndingEval::setUp();
    progress::ml::NewProgress::setUp();
    rating::StandardFeatureSet::instance();
  }
  if (vm.count("benchmark") || vm.count("benchmark-single")) {
    game_playing::AlphaBeta2OpenMidEndingEvalPlayer player;
    player.setDepthLimit(2000, 400, 200);
    player.setNodeLimit(std::numeric_limits<size_t>::max());
    player.setTableLimit(std::numeric_limits<size_t>::max(), 200);
    benchmark(player, "", benchmark_seconds);
    return 0;
  }
  if (num_benchmark) {
#ifdef OSL_SMP
    if (vm.count("profile"))
      osl::OslConfig::setNumCPUs(1);
#endif
    game_playing::AlphaBeta2OpenMidEndingEvalPlayer player;
    player.setDepthLimit(2000, 400, 200);
    player.setNodeLimit(std::numeric_limits<size_t>::max());
    player.setTableLimit(std::numeric_limits<size_t>::max(), 200);
    benchmark_more(player, num_benchmark, benchmark_seconds);
    return 0;
  }

  OslConfig::setUsiMode(extended_usi ? OslConfig::ExtendedUSI : OslConfig::PortableUSI);
  OslConfig::setSearchExactValueInOneReply(true); // todo: option
  read_config_file();
#ifdef OSL_SMP
  if (num_cpus > 0 && num_cpus != OslConfig::concurrency()) {
    std::cerr << "set num-cpus " << OslConfig::concurrency() << " => " << num_cpus << "\n";
    OslConfig::setNumCPUs(num_cpus);
  }
#endif
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stdin, NULL, _IONBF, 0);
  if (io_stream.find("tcp:") == 0
      && std::count(io_stream.begin(), io_stream.end(), ':') == 2)
  {
    std::vector<std::string> args;
    boost::algorithm::split(args, io_stream, boost::algorithm::is_any_of(":"));
    assert(args.size() == 3);
    while (1) {
      using boost::asio::ip::tcp;
      tcp::resolver resolver(io_service);
      tcp::resolver::query query(tcp::v4(), args[1], args[2]);
      tcp::endpoint server(*resolver.resolve(query));
      tcp::socket socket(io_service);
      std::cerr << "connecting " << server << std::flush;
      boost::system::error_code ec;      
      while (socket.connect(server, ec)) {
	std::cerr << "." << std::flush;
	std::this_thread::sleep_for(std::chrono::seconds(15));
      }
      std::cerr << "\n" << std::flush;
      boost::iostreams::stream<tcp_istream> is(socket);
      boost::iostreams::stream<tcp_ostream> os(socket);
      play(is, os);
      // fall back to reconnect in case of server down
      std::this_thread::sleep_for(std::chrono::seconds(15));
    }
  }
  else if (io_stream != "") 
  {
    read_from_file = true;
    std::ifstream is(io_stream.c_str());
    play(is, std::cout);
  }
  else 
  {
    play(std::cin, std::cout);
  }
}

// 
bool ThreadRun::goodAlignment()
{
  int dummy __attribute__ ((aligned (16)));
  int offset = (reinterpret_cast<size_t>(&dummy) % 16);
  if (offset) {
    std::cerr << "stack adjust error " << offset << "\n";
    return false;
  }
  return true;
}

game_playing::ComputerPlayer *makePlayer(Player turn, const SimpleState& initial_state, std::ostream& os)
{
  static osl::book::WeightedBook book(OslConfig::openingBook());
  game_playing::SearchPlayer *search_player = new game_playing::AlphaBeta2OpenMidEndingEvalPlayer;
  dynamic_cast<game_playing::AlphaBeta2OpenMidEndingEvalPlayer&>(*search_player)
    .enableMultiPV(multi_pv_width);
  search_player->setNextIterationCoefficient(byoyomi > 0 ? 1.0 : 1.7);
  if (OslConfig::isMemoryLimitEffective()) 
  {
    search_player->setTableLimit(std::numeric_limits<size_t>::max(), 200);
    search_player->setNodeLimit(std::numeric_limits<size_t>::max());
  }
  else
  {
    search_player->setTableLimit(3000000, 200);
  }
  search_player->setVerbose(is_verbose() ? 2 : 0);
  search_player->setDepthLimit(limit_depth*200, 400, 200);
#ifndef MINIMAL
  if (node_count_hard_limit)
    search_player->setNodeCountHardLimit(node_count_hard_limit);
#endif
  const bool extended_usi = OslConfig::usiMode() == OslConfig::ExtendedUSI;
  std::shared_ptr<UsiMonitor> monitor(new UsiMonitor(extended_usi, os));
  if (udp_socket)
    monitor->setUdpLogging(hostname, udp_socket.get(), udp_endpoint.get());

  search_player->addMonitor(monitor);
  bool use_book = book_depth > 0;
  {
    const SimpleState usual(HIRATE);
    if (! (book::CompactBoard(initial_state) == book::CompactBoard(usual)))
      use_book = false;
  }
  game_playing::ComputerPlayer *result = search_player;
  if (ponder_enabled) {
    result = new game_playing::SpeculativeSearchPlayer(turn, search_player);
    result->allowSpeculativeSearch(true);
  }
  if (use_book) {
    game_playing::WeightTracer *tracer = new game_playing::WeightTracer
      (book, is_verbose(), book_width_root, book_width);
    game_playing::BookPlayer *player
      = new game_playing::BookPlayer(tracer, result);
    player->setBookLimit(book_depth);
    return player;
  }
  else
    return result;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

