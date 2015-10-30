#include "curl_util.h"
#include "game.h"
#include "exception.h"
#include "logging.h"
#include "tables.h"
#include "threat_search.h"
#include "osl/misc/iconvConvert.h"
#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/record/ki2.h"
#include "osl/usi.h"
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <curl/curl.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <cassert>
#include <exception>
#include <iostream>
#include <memory>    // unique_ptr
#include <sstream>
#include <string>
#include <vector>

namespace bf = boost::filesystem;
namespace bp = boost::program_options;

bp::variables_map vm;
sql::mysql::MySQL_Driver* driver;
std::unique_ptr<sql::Connection> con;
std::string mysql_host;

CurlPtr curl;

/**
 * Used in positionStatistics
 */
class Results
{
  int black_wins;
  int white_wins;
  int others;
  std::string move;

public:
  Results()
    : black_wins(0), white_wins(0), others(0)
  {}

  int getBlackWins() const { return black_wins; }
  int getWhiteWins() const { return white_wins; }
  int getOthers() const { return others; }

  void incBlackWins() { black_wins += 1; }
  void incWhiteWins() { white_wins += 1; }
  void incOthers() { others += 1; }

  void setMove(const std::string& str) { move = str; }
  std::string getMove() const { return move; }
    
  int getTotalGames() const
  {
    return black_wins + white_wins + others;
  }

  double getBlackWinRate() const
  {
    // Excludes others
    const int total = black_wins + white_wins;
    if (total == 0) {
      return 0;
    } else {
      return 1.0*black_wins/total;
    }
  }

  bool operator< (const Results& b) const {
    if (this->getTotalGames() != b.getTotalGames())
      return this->getTotalGames() < b.getTotalGames();
    else if (this->getBlackWins() != b.getBlackWins())
      return this->getBlackWins() < b.getBlackWins();
    else if (this->getWhiteWins() != b.getWhiteWins())
      return this->getWhiteWins() < b.getWhiteWins();
    else
      return this->getOthers() < b.getOthers();
  }
};

typedef std::map<std::string, Results> results_t;

struct GameNameParser
{
  const std::string game_name;

  std::string year_str;
  std::string month_str;
  std::string day_str;
  std::string hour_str;
  std::string min_str;
  std::string sec_str;

  GameNameParser(const std::string& _game_name);

  int year() const {
    return boost::lexical_cast<int>(year_str);
  }
  int month() const {
    return boost::lexical_cast<int>(month_str);
  }

  int day() const {
    return boost::lexical_cast<int>(day_str);
  }
};

GameNameParser::GameNameParser(const std::string& _game_name)
  : game_name(_game_name)
{
  const static std::string method = "GameNameParser";

  boost::regex r("^wdoor\\+[^\\+]+\\+[^\\+]+\\+[^\\+]+\\+(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})$");
  boost::smatch what;
  if (boost::regex_search(game_name.begin(), game_name.end(), what, r)) {
    year_str.assign(what[1].first, what[1].second);
    month_str.assign(what[2].first, what[2].second);
    day_str.assign(what[3].first, what[3].second);
    hour_str.assign(what[4].first, what[4].second);
    min_str.assign(what[5].first, what[5].second);
    sec_str.assign(what[6].first, what[6].second);
  } else {
    logWarn(method, boost::format("Invalid game name: %s") % game_name);
    return;
  }
}

std::string positionStatistics(const uint64_t position_id, const std::string& timestamp)
{
    std::vector<Moves> tmp;
    //selectMovesByPositionId(30, "2014-08-01 00:00:00", tmp);
    selectMovesByPositionId(position_id, timestamp, tmp);

    results_t results;
    for (const Moves& m : tmp) {
      const boost::optional<Game> game = selectGameById(m.game_id);
      std::istringstream csa_file(game->kifu);
      const osl::record::CsaFile csa(csa_file);

      osl::NumEffectState state;
      int i = 0;
      std::string next_move_str;
      osl::Move prev_move;
      for (const osl::Move move : csa.moves()) {
        if (++i == m.nth+1) {
          next_move_str = osl::IconvConvert::convert("EUC-JP", "UTF-8",
                                                osl::ki2::show(move, state, prev_move));
          break;
        }
        state.makeMove(move);
        prev_move = move;
      }

      Results& r = results[next_move_str];
      switch (csa.load().result) {
      case osl::record::Record::BlackWin:
        r.incBlackWins();
        break;
      case osl::record::Record::WhiteWin:
        r.incWhiteWins();
        break;
      default:
        r.incOthers();
        break;
      }
    }

    std::vector<Results> sorted;
    sorted.reserve(results.size());
    for (auto& kv : results) {
      kv.second.setMove(kv.first);
      sorted.push_back(kv.second);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const Results& a, const Results& b){ return b < a; });

    std::ostringstream out;
    out << "前例 | 先手勝数 | 後手勝数 | その他 | 先手勝率 \n" <<
           "-----|----------|----------|--------|--------- \n";
    for (const Results& r : sorted) {
      out << boost::str(boost::format("%s|%d|%d|%d|%0.3f \n") %
                        r.getMove() % r.getBlackWins() % r.getWhiteWins() %
                        r.getOthers() % r.getBlackWinRate());
    }

    return out.str();
}

void tdiaryAppend(int year, int month, int day, const std::string& body)
{
  const static std::string method = "tdiaryAppend";

  CurlPtr curl(curl_easy_init());
  if (curl) {
     const boost::format ymd = boost::format("%04d%02d%02d") % year % month % day;
     std::ostringstream out;
     out << "old=" << boost::str(ymd) << 
            "&year=" << year << 
            "&month=" << month <<
            "&day=" << day <<
            "hide=false&append=add&title=" <<
            "&body=" << urlEncode(body);
     const std::string encoded_body = out.str();

     const std::string usrpwd = vm["httpauth-usrpwd"].as<std::string>();
     curl_easy_setopt(curl.get(), CURLOPT_USERPWD,    usrpwd.c_str());
     curl_easy_setopt(curl.get(), CURLOPT_HTTPAUTH,   (long)CURLAUTH_BASIC);
     curl_easy_setopt(curl.get(), CURLOPT_URL,        "http://b.kifupedia.org/update.rb");
     curl_easy_setopt(curl.get(), CURLOPT_REFERER,    "http://b.kifupedia.org/update.rb");
     curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, encoded_body.c_str());

     const CURLcode res = curl_easy_perform(curl.get());
     if (res != CURLE_OK) {
       const std::string msg(curl_easy_strerror(res));
       logError(method, msg); 
       throw GpsException(msg);
     }
  } else {
    const std::string msg("Failed to initialize cURL.");
    logError(method, msg);
    throw GpsException(msg);
  }
}

void postBlog(const std::string& method, const GameNameParser& gn, const std::string& body)
{
  if (vm.count("dry-run")) {
    logInfo(method, boost::format("%04d-%02d-%02d\n%s\n") %
            gn.year() % gn.month() % gn.day() % body);
  } else {
    tdiaryAppend(gn.year(), gn.month(), gn.day(), body);
  }
}

std::string link(const GameNameParser& gn, const int nth)
{
  const boost::format fmt = boost::format(
    "http://wdoor.c.u-tokyo.ac.jp/shogi/view/index.cgi?csa=http://wdoor.c.u-tokyo.ac.jp/shogi/LATEST/%s/%s/%s/%s.csa&move_to=%d") %
    gn.year_str % gn.month_str % gn.day_str %
    urlEncode(gn.game_name) %
    nth;

  return boost::str(fmt);
}

std::string image(const std::string& game_name, const int nth)
{
  return boost::str(boost::format("![board-image](http://i.kifupedia.org/%s/%d.png)\n") %
                    urlEncode(game_name) %
                    nth);
}

void processNewMoves(const MoveEntry& new_moves)
{
  const static std::string method = "processNewMoves";

  logInfo(method, boost::format("Processing game_id %d [%d]") % new_moves.game_id % new_moves.nth);

  boost::optional<Game> game = selectGameById(new_moves.game_id);
  const std::string game_name = game->game_name;
  GameNameParser gn(game_name);

  std::istringstream csa_file(game->kifu);
  osl::record::CsaFile csa(csa_file);

  // Title
  std::ostringstream out;
  out << boost::str(boost::format("# [新手] %s新手 %s-%s-%s %s:%s\n") %
                    urlEncode(new_moves.nth % 2 ? game->blackPlayer() : game->whitePlayer()) %
                    gn.year_str % gn.month_str % gn.day_str % gn.hour_str % gn.min_str); 
  // Body
  out << boost::str(boost::format("[%s 対 %s %d手目](%s)\n") %
                    urlEncode(game->blackPlayer()) %
                    urlEncode(game->whitePlayer()) %
                    new_moves.nth %
                    link(gn, new_moves.nth));

  // images
  out << image(game->game_name, new_moves.nth);

  // Statistics
  {
    const boost::optional<Moves> moves = selectMoves(new_moves.game_id, new_moves.nth-1);
    const boost::optional<Game> game = selectGameById(new_moves.game_id);
    out << "\n" << positionStatistics(moves->position_id, game->timestamp) << "\n\n";
  }

  postBlog(method, gn, out.str());
}

void processThreatEscapingThreat(const MoveEntry& entry)
{
  const static std::string method = "processThreatEscapingThreat";
  logInfo(method, boost::format("Processing game_id %d [%d]") % entry.game_id % entry.nth);

  boost::optional<Game> game = selectGameById(entry.game_id);
  const GameNameParser gn(game->game_name);

  osl::NumEffectState state;
  const std::vector<osl::Move> moves = game->moves();
  int i=0;
  for (; i<entry.nth-2; ++i) {
    if (state.isValidMove(moves[i])) {
      state.makeMove(moves[i]);
    } else {
      logWarn(method, boost::format("Found an invalid move: %d-th move") % (i+1));
      return;
    }
  }
  // Do not use the last move so that a full square appears in a move
  // string. 
  const std::string first_move_str =
    osl::IconvConvert::convert("EUC-JP", "UTF-8",
                               osl::ki2::show(moves[i], state));
  state.makeMove(moves[i++]);
  const std::string second_move_str =
    osl::IconvConvert::convert("EUC-JP", "UTF-8",
                               osl::ki2::show(moves[i], state, moves[i-1]));

  std::istringstream csa_file(game->kifu);
  osl::record::CsaFile csa(csa_file);

  // Title
  std::ostringstream out;
  out << boost::str(boost::format("# [詰めろ逃れの詰めろ] [%s vs %s戦](%s) %s-%s-%s %s:%s\n") %
                    urlEncode(game->blackPlayer()) %
                    urlEncode(game->whitePlayer()) %
                    link(gn, entry.nth) %
                    gn.year_str % gn.month_str % gn.day_str % gn.hour_str % gn.min_str);

  // Body
  out << boost::str(boost::format(
    "%d手目、%sの%sの詰めろに対し、%sは%sの詰めろ逃れの詰めろで応酬した。") %
    (entry.nth-1) %
    (entry.nth % 2 == 1 ? game->whitePlayer() : game->blackPlayer()) %
    first_move_str %
    (entry.nth % 2 == 1 ? game->blackPlayer() : game->whitePlayer()) %
    second_move_str);

  if (game->winner()) {
    out << boost::str(boost::format("結果は%s%sの勝ち。") %
      (game->winner() == osl::BLACK ? "先手" : "後手") %
      *game->winnerName());
  }

  // images
  out << "\n" << image(game->game_name, entry.nth);

  postBlog(method, gn, out.str());
}

void processCheckmate(const MoveEntry& entry)
{
  const static std::string method = "processCheckmate";
  logInfo(method, boost::format("Processing game_id %d [%d]") % entry.game_id % entry.nth);

  boost::optional<Game> game = selectGameById(entry.game_id);
  const GameNameParser gn(game->game_name);

  // 1. Get checkmate PV moves
  std::istringstream csa_file(game->kifu);
  osl::record::CsaFile csa(csa_file);
  rc::ThreatSearch threatSearch(70);
  threatSearch.setup();

  // 1-1. Move to the checkmate position
  osl::NumEffectState state;
  const std::vector<osl::Move>& moves = csa.moves();
  for (int i=0; i<entry.nth; ++i) {
    if (state.isValidMove(moves[i])) {
      state.makeMove(moves[i]);
    } else {
      logWarn(method, boost::format("Found an invalid move: %d-th move") % (i+1));
      return;
    }
  }

  // 1-2. Get the checkmate PV moves
  std::vector<osl::Move> pv;
  const int nodes = threatSearch.isCheckmate(state, pv);
  if (nodes == 0) {
    logError(method, "Checkmate is not found");
    return;
  }
  assert(!pv.empty());
  std::ostringstream pvstr;
  // 1-3. Get a string representing the checkmate last move (as a hint)
  osl::Move prev;
  for (const osl::Move move : pv) {
    pvstr << 
      osl::IconvConvert::convert("EUC-JP", "UTF-8",
                                 osl::ki2::show(move, state, prev));
    state.makeMove(move);
    prev = move;
  }
  std::string last_move =
      osl::IconvConvert::convert("EUC-JP", "UTF-8",
                                 osl::ki2::show(pv[pv.size()-1], state, osl::Move()));

  // Title
  std::ostringstream out;
  out << boost::str(boost::format("# [実戦詰将棋] [%s vs %s戦](%s) %s-%s-%s %s:%s\n") %
                    urlEncode(game->blackPlayer()) %
                    urlEncode(game->whitePlayer()) %
                    link(gn, entry.nth) %
                    gn.year_str % gn.month_str % gn.day_str % gn.hour_str % gn.min_str); 
  
  // Body
  out << boost::str(boost::format("%d手目を指し終えた局面、%s玉に即詰みが生じている。") %
                    entry.nth %
                    (entry.nth % 2 == 1 ? "先手" : "後手"));
  out << boost::str(boost::format("概ね%d手詰め（最終手は%s）。") %
                    pv.size() % last_move);
  if (game->winner()) {
    if ((game->winner() == osl::BLACK && entry.nth % 2 == 0) ||
        (game->winner() == osl::WHITE && entry.nth % 2 == 1)) {
      out << boost::str(boost::format("結果は%s%sの勝ち。") %
        (game->winner() == osl::BLACK ? "先手" : "後手") %
        *game->winnerName());
    } else {
      out << boost::str(boost::format("しかし、詰みを逃し、%s%sの逆転負け。") %
        (game->winner() == osl::BLACK ? "後手" : "先手") %
        *game->loserName());
    }
  }
  const std::string id = boost::str(boost::format("cmpv%d") % entry.game_id);
  out << boost::str(boost::format("\n<div id=\"%s\" class=\"checkmatepv\">%s</div>\n<form><input type=\"button\" value=\"詰み手順の表示/非表示\" onclick='toggleDisplay(\"%s\")'></form>") % id % pvstr.str() %id);


  // images
  out << "\n" << image(game->game_name, entry.nth);

  postBlog(method, gn, out.str());
}

/*========== MAIN ==========*/

void printUsage(std::ostream& out, 
                char **argv,
                const bp::options_description& command_line_options)
{
  out << 
    "Usage: " << argv[0] << " [options]\n"
      << command_line_options 
      << std::endl;
}

int parseCommandLine(int argc, char **argv, 
                     bp::variables_map& vm)
{
  bp::options_description command_line_options;
  command_line_options.add_options()
    ("dry-run",           "Dry run")
    ("httpauth-usrpwd,p", bp::value<std::string>(),
                          "user:password for HTTP Auth")
    ("mysql-host",        bp::value<std::string>(&mysql_host)->default_value("localhost:3306"),
                          "MySQL database server address to connect to")
    ("mysql-user",        bp::value<std::string>(),
                          "MySQL database user to connect")
    ("mysql-password",    bp::value<std::string>(),
                          "MySQL database password to connect")
    ("help,h",            "Show help message");
  bp::positional_options_description p;

  try
  {
    bp::store(
      bp::command_line_parser(
        argc, argv).options(command_line_options).positional(p).run(), vm);
    bp::notify(vm);
    if (vm.count("help")) {
      printUsage(std::cerr, argv, command_line_options);
      return 0;
    }
    if (!vm.count("httpauth-usrpwd")) {
      printUsage(std::cerr, argv, command_line_options);
      return 0;
    }
    if (!vm.count("mysql-user")) {
      printUsage(std::cerr, argv, command_line_options);
      return 0;
    }
    if (!vm.count("mysql-password")) {
      printUsage(std::cerr, argv, command_line_options);
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << "\n"
              << e.what() << std::endl;
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

  return 1;
}

int main(int argc, char** argv)
{
  const static std::string method = "main";
  namespace bp = boost::program_options;

  if (!parseCommandLine(argc, argv, vm))
    exit(1);

  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    logError(method, "Failed to curl_global_init.");
    exit(1);
  }

  try {
    logInfo(method, boost::format("Connecting to %s...") % vm["mysql-host"].as<std::string>());

    driver = sql::mysql::get_mysql_driver_instance();
    con.reset(driver->connect(
               boost::str(boost::format("tcp://%s") % mysql_host),
               vm["mysql-user"].as<std::string>(), 
               vm["mysql-password"].as<std::string>()));
    con->setSchema("book");

    logInfo(method, "  connected."); 

    { /* New Moves */
      const std::string table = "NEW_MOVES";
      MoveEntry entry;
      while (pickUpMoveEntry(table, entry)) {
        logInfo(method, boost::format("Picked up a new move: %d %d") % entry.game_id % entry.nth);
        processNewMoves(entry);
        if (!vm.count("dry-run")) {
          setFlagMoveEntry(table, entry);
        }
        sleep(3);
      }
    }
    { /* Threat Escaping Threat */
      const std::string table = "THREAT_ESCAPING_THREAT";
      MoveEntry entry;
      while (pickUpMoveEntry(table, entry)) {
        logInfo(method, boost::format("Picked up a threat-escaping-threat move: %d %d") % entry.game_id % entry.nth);
        processThreatEscapingThreat(entry);
        if (!vm.count("dry-run")) {
          setFlagMoveEntry(table, entry);
        }
        sleep(3);
      }
    }
    { /* Difficult Checkmate */
      const std::string table = "CHECKMATE";
      MoveEntry entry;
      while (pickUpMoveEntry(table, entry)) {
        logInfo(method, boost::format("Picked up a checkmate move: %d %d") % entry.game_id % entry.nth);
        processCheckmate(entry);
        if (!vm.count("dry-run")) {
          setFlagMoveEntry(table, entry);
        }
        sleep(3);
      }
    }
  } catch (std::exception& e) {
    logError(method, e);
    return 1;
  }

  curl_global_cleanup();

  return 0;
}
