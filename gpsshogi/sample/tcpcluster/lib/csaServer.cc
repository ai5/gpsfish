/* csaServer.cc
 */
#include "csaServer.h"
#include "coordinator.h"
#include "searchNode.h"
#include "logging.h"
#include "osl/game_playing/usiState.h"
#include "osl/record/csaRecord.h"
#include "osl/usi.h"
#include "osl/book/openingBook.h"
#include "osl/game_playing/weightTracer.h"
#include "osl/hashKey.h"
#include "osl/oslConfig.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <thread>
#include <iostream>

void gpsshogi::CsaGameCondition::
addToCsaFile(const std::string& msg)
{
  csalines += msg;
  *csafile << msg;
}
void gpsshogi::CsaGameCondition::
csaFileFlush()
{
  *csafile << std::flush;
}


gpsshogi::CsaServer::Connection::
~Connection()
{
}

class gpsshogi::CsaServer::MultiTcpIp
  : public gpsshogi::CsaServer::Connection
{
  boost::ptr_vector<boost::asio::ip::tcp::socket> socket;
  boost::ptr_vector<boost::asio::streambuf> buffer_r, buffer_w;
  std::vector<Account> account;
  CsaConfig & config;		// overwrite limit_seconds..
  int selected;
  boost::asio::io_service& io;
  std::mutex& mutex;
  std::condition_variable& condition;
public:
  MultiTcpIp(CsaConfig & c, boost::asio::io_service& i,
	     std::mutex& m, std::condition_variable& cond)
    : config(c), selected(-1), io(i), mutex(m), condition(cond) {
  }
  bool loginWithRetry(const CsaConfig& config)
  {
    account = config.multi_login;
    Logging::notice("^connecting to csa server "
		    + config.servername + ":" + to_s(config.portnumber));
    boost::asio::ip::tcp::resolver resolver(io);
    boost::asio::ip::tcp::resolver::query
      query(boost::asio::ip::tcp::v4(), config.servername, to_s(config.portnumber));
    boost::asio::ip::tcp::endpoint server(*resolver.resolve(query));
    for (size_t i=0; i<account.size(); ++i)
      if (! loginWithRetry(i, server))
	return false;
    return true;
  }
  bool loginWithRetry(int n, boost::asio::ip::tcp::endpoint &server)
  {
    buffer_w.push_back(new boost::asio::streambuf);
    buffer_r.push_back(new boost::asio::streambuf);
    while (true) {
      const Account& account = this->account.at(n);
      Logging::notice("^login with user " + account.username);
      
      socket.push_back(new boost::asio::ip::tcp::socket(io));

      boost::system::error_code ec;      
      while (socket.back().connect(server, ec)) {
	std::cerr << "." << std::flush;
	std::this_thread::sleep_for(std::chrono::seconds(15));
      }
      std::cerr << "\n" << std::flush;
      writeLine(n, "LOGIN " + account.username + ' ' + account.password);
      std::string line;
      if (! readLine(n, line)) {
	Logging::error("^disconnected before LOGIN (retry)");
	std::this_thread::sleep_for(std::chrono::seconds(15));
	socket.pop_back();
	continue;
      }
      Logging::info("^"+line);
      if (line != "LOGIN:"+account.username+" OK") {
	Logging::error("^login failed "+line);
	return false;
      }
      return true;
    }
  }
  bool readLine(int n, std::string& line)
  {
    boost::system::error_code ec;
    boost::asio::read_until(socket[n], buffer_r[n], "\n", ec);
    if (ec) {
      Logging::error("^read " + to_s(n) + " " + ec.message());
      return false;
    }
    std::istream is(&buffer_r[n]);
    if (! getline(is, line)) {
      Logging::error("^readline " + to_s(n));
      return false;
    }
    return true;
  }
  void writeLine(int n, const std::string& msg)
  {
    std::ostream os(&buffer_w[n]);
    os << msg << "\n";
    boost::asio::write(socket[n], buffer_w[n]);
  }
  bool readLine(std::string& line)
  {
    if (selected >= 0)
      return readLine(selected, line);
    Logging::notice("^wait for first challenge");
    for (size_t i=0; i<account.size(); ++i) {
      boost::asio::async_read_until
	(socket[i], buffer_r[i], "\n",
	 [=](boost::system::error_code ec, size_t){ this->waitChallenge(i, ec); });
    }

    std::unique_lock<std::mutex> lk(mutex);
    while (selected < 0)
      condition.wait(lk);

    Logging::notice("^selected ("+to_s(selected)
		    +") "+account[selected].username);
    config.limit_seconds = account[selected].limit_seconds;
    config.limit_byoyomi = account[selected].limit_byoyomi;

    std::istream is(&buffer_r[selected]);
    if (! getline(is, line)) {
      Logging::error("^readline " + to_s(selected));
      return false;
    }
    return true;    
  }
  void writeLine(const std::string& msg)
  {
    assert(selected >= 0);
    writeLine(selected, msg);
  }
  //
  void waitChallenge(size_t called, const boost::system::error_code& ec) {
    if (ec || selected >= 0)
      return;
    std::lock_guard<std::mutex> lk(mutex);
    selected = called;
    for (size_t i=0; i<account.size(); ++i) {
      if (i == selected)
	continue;
      socket[i].cancel();
      socket[i].close();
    }
    condition.notify_all();
  }
};

class gpsshogi::CsaServer::TcpIp : public gpsshogi::CsaServer::Connection
{
  std::unique_ptr<std::iostream> ios;
public:
  bool loginWithRetry(const CsaConfig& config)
  {
    while (true) {
      Logging::notice("^connecting to csa server "+config.servername);
      ios.reset(new boost::asio::ip::tcp::iostream
		(config.servername, to_s(config.portnumber)));
      if (! *ios)
	Logging::info("^waiting csaserver ready");
      while (! *ios) {
	std::this_thread::sleep_for(std::chrono::seconds(15));
	ios.reset(new boost::asio::ip::tcp::iostream
		  (config.servername, to_s(config.portnumber)));
      }
      writeLine("LOGIN " + config.username + ' ' + config.password);
      std::string line;
      if (! readLine(line)) {
	Logging::error("^disconnected before LOGIN (retry)");
	std::this_thread::sleep_for(std::chrono::seconds(15));
	continue;
      }
      Logging::info("^"+line);
      if (line != "LOGIN:"+config.username+" OK") {
	Logging::error("^login failed "+line);
	return false;
      }
      return true;
    }
  }
  bool readLine(std::string& line)
  {
    return getline(*ios, line);
  }
  void writeLine(const std::string& msg)
  {
    *ios << msg << "\n" << std::flush;
  }
};

class gpsshogi::CsaServer::StdIO : public gpsshogi::CsaServer::Connection
{
  std::unique_ptr<boost::asio::ip::tcp::iostream> ios;
public:
  bool loginWithRetry(const CsaConfig& config)
  {
    return true;
  }
  bool readLine(std::string& line)
  {
    return getline(std::cin, line);
  }
  void writeLine(const std::string& msg)
  {
    std::cout << msg << "\n" << std::flush;
  }
};

class gpsshogi::CsaServer::FileStream
  : public gpsshogi::CsaServer::Connection
{
  int in, out;
public:
  static std::string path()
  {
    return "remote-csa";
  }
  FileStream() : in(0), out(0) 
  {
  }
  static std::string id(int n) 
  {
    char s[128];
    sprintf(s, "%04d", n);
    return s;
  }
  bool loginWithRetry(const CsaConfig& config)
  {
    while (true) {
      Logging::notice("^connecting to csa server");
      in = out = 0;
      boost::filesystem::create_directory(path());
      boost::filesystem::create_directory(path()+"/in");
      boost::filesystem::create_directory(path()+"/out");
      writeLine("LOGIN " + config.username + ' ' + config.password);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::string line;
      if (! readLine(line)) {
	Logging::error("^retry LOGIN");
	std::this_thread::sleep_for(std::chrono::seconds(5));
	continue;
      }
      Logging::info("^"+line);
      if (line != "LOGIN:"+config.username+" OK") {
	Logging::error("^login failed "+line);
	return false;
      }
      return true;
    }

  }
  bool readLine(std::string& line)
  {
    while (true) {
      std::ifstream is((path()+"/in/"+id(in)+".txt").c_str());
      if (is && std::getline(is, line)) {
	++in;
	return true;
      }
      // todo: inotify
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      // retry if our first login message was cleared
      if (in == 0) {
	std::ifstream is((path()+"/out/"+id(0)+".txt").c_str());
	if (! is)
	  return false;
      }
    }
  }
  void writeLine(const std::string& msg)
  {
    std::string base = path()+"/out/"+id(out++);
    {
      std::ofstream os((base+".part").c_str());
      os << msg << "\n";
    }
    boost::filesystem::rename(base+".part", base+".txt");
  }
};



gpsshogi::CsaServer::
CsaServer(const CsaConfig& c)
  : verbose(true), config(c)
{
  config.send_info |= (config.servername == "wdoor.c.u-tokyo.ac.jp");
  current_status = WaitConnection;
}

gpsshogi::CsaServer::
~CsaServer()
{
}

void gpsshogi::CsaServer::
start()
{
  // todo: use its own io_service and remove std::condition_variable
  std::thread(&CsaServer::playGames, this, config.games).detach();
}

bool gpsshogi::CsaServer::
waitOpponent(int gameid)
{
  char buf[1024] = {0};
  sprintf(buf, "%03d.csa", gameid);
  game.csa_filename = buf;
  game.csafile.reset(new std::ofstream(game.csa_filename.c_str()));
  game.csalines = "";
  game.addToCsaFile("V2\n");

  current_status = Idle;
  std::string position = "", gamename, line, my_color;
  game.my_turn = osl::BLACK;
  game.initial_state = osl::NumEffectState();
  game.initial_moves.clear();
  game.seconds = 1500;
  game.byoyomi = 0;
  game.time_used.fill(0);

  if (config.stdio_config != "") {
    if (config.stdio_config.find("white") == 0
	|| config.stdio_config.find("WHITE") == 0)
      game.my_turn = osl::WHITE;
    return true;
  }

  bool in_game_summary = false, in_position = false;
  while (true) {
    if (! connection->readLine(line))
      return false;
    if (line.empty())
      continue;
    if (line.find("BEGIN Position") == 0) {
      in_position = true;
      // output game information to csa
      {
	game.addToCsaFile("$EVENT:" + gamename + "\n");
	boost::posix_time::time_duration td = boost::posix_time::seconds(game.seconds);
	sprintf(buf, "$TIME_LIMIT:%02d:%02d+%02d\n", 
		td.hours(), td.minutes(), game.byoyomi);
	// CAVEAT: csa format does not allow to record td.seconds()
	game.addToCsaFile(buf);
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	boost::gregorian::date date = now.date();
	td = now.time_of_day();
	sprintf(buf, "$START_TIME:%04d/%02d/%02d %02d:%02d:%02d\n", 
		static_cast<int>(date.year()), date.month().as_number(), date.day().as_number(),
	       td.hours(), td.minutes(), td.seconds());
	game.addToCsaFile(buf);
      }
      continue;
    }
    if (line.find("END Position") == 0) {
      in_position = false;
      game.addToCsaFile(position);
      continue;
    }
    if (line.find("BEGIN Game_Summary") == 0) {
      in_game_summary = true;
      continue;
    }
    if (line.find("END Game_Summary") == 0) {
      in_game_summary = false;
      game.csaFileFlush();
      try {
	std::cerr << position << "\n";
	std::istringstream is(position);
	osl::CsaFile csa(is);
	game.initial_state = csa.initialState();
	auto record = csa.load();
	std::vector<int> times;
	record.load(game.initial_moves, times);
	for (size_t i=0; i<times.size(); ++i) {
	  game.time_used[osl::indexToPlayer(i%2)] += times[i];
	}
	std::cerr << "used BLACK " << game.time_used[osl::BLACK]
		  << " WHITE " << game.time_used[osl::WHITE] << '\n';
	game.state = osl::HistoryState(game.initial_state);
	for (osl::Move move: game.initial_moves) {
	  if (! game.state.state().isValidMove(move))
	    throw osl::CsaIOError("not valid move "+osl::csa::show(move));
	  game.state.makeMove(move);
	}
	connection->writeLine("AGREE");
      }
      catch (osl::CsaIOError& e) {
	Logging::error(to_s("^parse failed in csa position ")+e.what());
	return false;
      }
      catch (...) {
	Logging::error("^unknown exception in csa position");
	return false;	
      }
      continue;			// wait start
    }
    if (line.find("REJECT") == 0) {
      Logging::error("^"+line);
      connection->writeLine("LOGOUT");
      return false;
    }
    if (line.find("START") == 0)
      break;
    if (! in_game_summary)
      Logging::info("^"+line);
    const char *key = "Game_ID:";
    if (line.find(key) == 0) {
      gamename = line.substr(strlen(key));
      continue;
    }
    key = "Name"; // intentionaly exclude ':'
    if (line.find(key) == 0) {
      game.addToCsaFile("N" + line.substr(strlen(key)) + "\n");
      continue;
    }
    key = "Total_Time:";
    if (line.find(key) == 0) {
      try {
	game.seconds = boost::lexical_cast<int>(line.substr(strlen(key)));
      }
      catch (...){
	Logging::error(std::string("^parse failed in ")+key+" "+line);
	return false;
      }
      continue;
    }
    key = "Byoyomi:";
    if (line.find(key) == 0) {
      try {
	game.byoyomi = boost::lexical_cast<int>(line.substr(strlen(key)));
      }
      catch (...){
	Logging::error(std::string("^parse failed in ")+key+" "+line);
	return false;
      }
      continue;
    }
    key = "Your_Turn:";
    if (line.find(key) == 0) {
      my_color = line.substr(strlen(key));
      boost::algorithm::trim(my_color);
      if (my_color != "+" && my_color != "-") {
	Logging::error("^unknown color "+my_color);
	return false;
      }
      game.my_turn = osl::csa::charToPlayer(my_color[0]);
      continue;
    }
    if (in_position)
      position += line + "\n";
  } // end while
  if (config.limit_seconds >= 0) {
    if (game.seconds > config.limit_seconds) {
      Logging::notice("limit seconds: " + to_s(game.seconds)
		      + " => " + to_s(config.limit_seconds));
      game.seconds = config.limit_seconds;
    }
    else if (game.seconds < config.limit_seconds) {
      Logging::warn("not limit seconds " + to_s(game.seconds)
		    + " < " + to_s(config.limit_seconds));
    }
  }
  if (config.limit_byoyomi >= 0) {
    if (game.byoyomi > config.limit_byoyomi) {
      Logging::notice("limit byoyomi: " + to_s(game.byoyomi)
		      + " => " + to_s(config.limit_byoyomi));
      game.byoyomi = config.limit_byoyomi;
    }
    else if (game.byoyomi < config.limit_byoyomi) {
      Logging::warn("not limit byoyomi " + to_s(game.byoyomi)
		    + " < " + to_s(config.limit_byoyomi));
    }
  }
  return true;
}

bool gpsshogi::
CsaServer::makeMoveFromServer()
{
  std::string line;
  if (deffered_input != "") {
    line = deffered_input;
    deffered_input = "";
  }
  else if (! connection->readLine(line)) {
    Logging::error("^read move failed");
    return false;
  }
  game.addToCsaFile(line + "\n");
  game.csaFileFlush();
  Logging::notice("s< " + line);
  if (line.find("#") != 0) {
    std::vector<std::string> elements;
    boost::algorithm::split(elements, line, boost::algorithm::is_any_of(","));
    try {
      osl::Move move = osl::csa::strToMove(elements[0], game.state);
      int seconds = 0;
      if (elements.size() > 1 && elements[1][0] == 'T')
	seconds = boost::lexical_cast<int>(elements[1].substr(1));
      if (move.isNormal()) {
	if (my_last_move.isNormal() && move != my_last_move) {
	  Logging::error("^unexpected response " + osl::csa::show(move)
			 + " while expected " +  osl::csa::show(my_last_move));
	  if (my_last_move.player() != move.player()) {
	    deffered_input = line; // assume missing input
	    move = my_last_move;
	    seconds = 1;
	  }
	  game_history.push_back
	    (std::make_tuple(my_last_score, my_last_move, 1));
	  my_last_score = 0;
	}
	game_history.push_back(std::make_tuple
			       (my_last_score, move, seconds));
	std::ostringstream ss;
	for (int i=0; i<std::min(8,(int)game_history.size()); ++i) {
	  const int id = game_history.size()-i-1;
	  const std::tuple<int,osl::Move,int> data = game_history[id];
	  ss << (id+1) << "."
	     << osl::csa::show(std::get<1>(data))
	     << ",T" << std::get<2>(data);
	  if (std::get<0>(data))
	    ss << '<' << std::get<0>(data) << '>';
	  ss << ' ';
	}
	Logging::notice(ss.str());
	my_last_move = osl::Move();
	my_last_score = 0;
	game.state.makeMove(move);
	game.time_used[move.player()] += seconds;
	double margin = (game_history.size() > 30) ? 1.8 : 0.8;
	double elapsed = osl::elapsedSeconds(last_move_time);
	if (elapsed < margin) {
	  Logging::notice("sleep "+to_s((int)((margin-elapsed)*1000))
			  +"ms for "+osl::csa::show(move));
	  std::this_thread::sleep_for(std::chrono::milliseconds
				      ((int)((margin-elapsed)*1000)));
	}
	last_move_time = osl::clock::now();
	io->post(std::bind(&Coordinator::moveRoot, coordinator,
			     osl::usi::show(move)));
	return true;
      }
    }
    catch (osl::CsaIOError& e){
      Logging::error(std::string("^read move ") + line + " " + e.what());
    }
    catch (...) {
      Logging::error("^read move exception "+ line);
    }
  }
  Logging::notice("^gameend by "+line);
  if (config.stdio_config == "") {
    for (int i=0; i<2; ++i)
      if (connection->readLine(line)) {
	game.addToCsaFile(line + "\n");
	game.csaFileFlush();
	if (config.wcsc)
	  break;
      }
  }
  return false;
}

void gpsshogi::CsaServer::
playGames(int num_games)
{
  for (int i=0; i<num_games; ++i) {
    coordinator->waitReady();
    current_status = WaitConnection;
    if (config.wcsc)
      connection.reset(new FileStream());
    else if (config.stdio_config != "")
      connection.reset(new StdIO());
    else if (! config.multi_login.empty())
      connection.reset(new MultiTcpIp(config, *io, mutex, condition));
    else
      connection.reset(new TcpIp());
    if (! connection->loginWithRetry(config)) { // wrong password => no retry
      io->post(std::bind(&Coordinator::prepareQuit, coordinator));
      return;
    }
    current_status = Initializing;
    game_history.clear();
    if (! waitOpponent(i)) 
      goto cleanup;
    current_status = Idle;
    my_last_move = osl::Move();
    
    // new game
    Logging::notice("^new game");
    io->post(std::bind(&Coordinator::newGameStart, coordinator));
    while (1) {
      osl::NumEffectState current = game.state;
      info = InterimReport();
      // my move
      if (current.turn() == game.my_turn) {
	osl::Move move = searchBestMove(); // may block
	my_last_move = move;
	my_last_score = info.bestValue();
	std::string msg = osl::csa::show(move);
	std::string csa_info = "";
	if (info.bestValue() || ! info.pv.empty()) {
	  boost::algorithm::replace_all(last_pv, "+", " +"); 
	  boost::algorithm::replace_all(last_pv, "-", " -"); 
	  boost::algorithm::replace_all(last_pv, "%", " %");
	  csa_info = to_s(info.bestValue()*100/usi_pawn_value) + last_pv;
	}
	if (csa_info != "" && config.send_info)
	  msg += ",'* " + csa_info;
	connection->writeLine(msg);
	Logging::notice("S> " + msg);
	current_status = Idle;
	if (! makeMoveFromServer()) // may block
	  goto cleanup;
	if (csa_info != "")
	  game.addToCsaFile("'** " + csa_info + "\n");
      }
      // opponent's move
      assert(game.state.state().turn() == alt(game.my_turn));
      if (! makeMoveFromServer()) // may block
	goto cleanup;
      assert(game.state.state().turn() == game.my_turn);
    }
    connection->writeLine("LOGOUT");
  cleanup:
    {
      std::ofstream os((Logging::path()+"game.csa").c_str());
      os << game.csalines;
    }
    io->post(std::bind(&Coordinator::gameFinished, coordinator));
    logDisconnected();
    connection.reset();
    game.csafile.reset();
    std::this_thread::sleep_for(std::chrono::seconds(15));
  }
  io->post(std::bind(&Coordinator::prepareQuit, coordinator));
}

osl::Move gpsshogi::CsaServer::
searchBestMove() 
{
  static osl::book::WeightedBook book(osl::OslConfig::openingBook());
  // book
  if (osl::HashKey(game.initial_state)
      == osl::HashKey(osl::SimpleState(osl::HIRATE))
      && (int)game.state.history().size() < config.book_depth)
  {
    osl::game_playing::WeightTracer tracer
      (book, false, config.book_width_root, config.book_width);
    for (osl::Move m: game.state.history())
      tracer.update(m);
    if (! tracer.isOutOfBook()) {
      const osl::Move best_move = tracer.selectMove();
      if (best_move.isNormal() && game.state.state().isValidMove(best_move))
	return best_move;
    }
  }

  // search
  // do not block even when some slaves have gone .. //coordinator->waitReady();
  osl::UsiState usi_state;
  usi_state.reset(game.state.initialState(), game.state.history());
  TimeCondition seconds;
  seconds.total = game.seconds;
  seconds.byoyomi_msec = game.byoyomi*1000;
  seconds.used = game.time_used[game.my_turn];
  seconds.opponent_used = game.time_used[alt(game.my_turn)];
  seconds.allow_ponder = config.ponder;
  seconds.my_turn = game.my_turn;
  std::unique_lock<std::mutex> lk(mutex);
  current_status = Go;
  go_positions.push_back(usi_state.usiString());
  io->post(std::bind(&Coordinator::handleGoInGame, coordinator,
		       go_positions.size(), go_positions.back(), seconds));
  while (bestmove_history.size() < go_positions.size())
    condition.wait(lk);
  current_status = Idle;
  return bestmove_history.back();
}

void gpsshogi::CsaServer::
logDisconnected()
{
  Logging::notice("^stream closed");
  current_status = Disconnected;
}

void gpsshogi::CsaServer::
outputSearchResult(int position_id, const std::string& msg)
{
  osl::Move move;
  try {
    std::vector<std::string> elements;
    boost::algorithm::split(elements, msg, boost::algorithm::is_any_of(" "));
    if (elements.size() < 2 || elements[0] != "bestmove")
      throw std::logic_error("^unkown usi bestmove "+msg);
    move = osl::usi::strToMove(elements[1], game.state);
  }
  catch (...) {
    Logging::error("^treat %TORYO for unexpected seacrh result "+msg);
  }
  std::lock_guard<std::mutex> lk(mutex);
  bestmove_history.push_back(move);
  condition.notify_all();
}

void gpsshogi::CsaServer::
outputSearchProgress(int position_id, const std::string& msg)
{
  static osl::time_point last_report;
  osl::time_point now = osl::clock::now();
  InterimReport report;
  report.updateByInfo(msg, -1);
  osl::HistoryState copy(game.state);
  std::lock_guard<std::mutex> lk(mutex);
  info.set(copy.state().turn(), report);
  last_pv = SearchNode::toCSA(copy, info.joinPV());
  std::ostringstream ss;
  ss << std::setfill(' ') << std::setw(5)
     << info.bestValue()*100/usi_pawn_value
     << ' ' << last_pv << " (" << info.pv.size()
     << ' ' << info.depth_head << ")";
  if (now - last_report >= osl::milliseconds(5000))
    Logging::notice(ss.str());
  else
    Logging::info(ss.str());
  last_report = now;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
