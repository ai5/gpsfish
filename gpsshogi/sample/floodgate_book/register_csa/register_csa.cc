#include "exception.h"
#include "logging.h"
#include "tables.h"
#include "threat_search.h"
#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/usi.h"
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <cassert>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace bf = boost::filesystem;
namespace bp = boost::program_options;

bp::variables_map vm;
sql::mysql::MySQL_Driver* driver;
std::unique_ptr<sql::Connection> con;
std::string mysql_host;

struct SfenMove
{
  std::string sfen;
  std::string move;
  uint64_t position_id;

  SfenMove(const std::string& _sfen,
           const std::string& _move,
           const uint64_t _position_id)
    : sfen(_sfen), move(_move), position_id(_position_id)
  { }
};

const std::vector<osl::Move> readMoves(const std::string& csa_file)
{
  const osl::record::CsaFile csa(csa_file);
  const osl::record::Record& record = csa.load();
  return record.moves();
}

void convertToUsi(const std::string& csa_file,
                  const std::vector<osl::Move>& moves,
                  std::vector<SfenMove>& sfen_moves)
{
  osl::NumEffectState state;
  for(const osl::Move move : moves) {
    state.makeMove(move);

    std::string state_str = osl::usi::show(state);
    state_str.erase(0, 5); // remove starting "sfen "

    const uint64_t position_id = addPosition(state_str);

    const SfenMove sm(state_str,
                      osl::usi::show(move),
                      position_id);
    sfen_moves.push_back(sm);
  }
}

bool isRegistered(const Game& game, int nth)
{
  if (selectMoves(game.game_id, nth))
    return true;
  else
    return false;
}

void registerMoves(const Game& game, const std::vector<SfenMove>& sfen_moves)
{
  assert(!sfen_moves.empty());

  int nth = 1;
  for(const SfenMove& sfen_move : sfen_moves) {
    const Moves moves(game.game_id, nth, sfen_move.move, sfen_move.position_id);
    addMoves(moves);
    nth += 1;
  }
}

/**
 * Returns a pair of an index (starting with 0) and count. The index is the
 * previous index of a new move. In case of no new move, the index is -1.
 *
 * Note: binary search is used to locate a possible new move. Therefore it does
 * not always locate the first new move in a sequence of moves.
 */
std::pair<int,int> locateNewMove(const Game& game, const std::vector<SfenMove>& sfen_moves)
{
  assert(!sfen_moves.empty());
  const static std::string method = "locateNewMove";

  if (sfen_moves.size() < 2) {
    logWarn(method, boost::format("Too few moves: %d for %s") % sfen_moves.size() % game.game_id);
    return std::make_pair(-1, -1);
  }

  size_t start = 0;
  size_t end   = sfen_moves.size() - 1;
  std::vector<uint32_t> counts(sfen_moves.size(), 0);
  counts[start] = 1;
  counts[end] = countGames(game, sfen_moves[end].position_id);
  if (counts[end] != 0) {
    logWarn(method, boost::format("Found a duplicated game sequence: %s") % game.game_id);
    return std::make_pair(-1,-1);
  }

  while (true) {
    const size_t i = (start+end)/2;
    if (start == i) break;
    counts[i] = countGames(game, sfen_moves[i].position_id);

    if (counts[i] > 0) {
      start = i;
    } else {
      end = i;
    }
  }

  return std::make_pair(start, counts[start]);
}

void processFile(const std::string& csa_file)
{
  const static std::string method = "processFile";

  const Game game = addGame(csa_file);

  const std::vector<osl::Move> moves = readMoves(csa_file);

  if (moves.empty()) {
    logWarn(method, boost::format("Empty kufu file: %s") % csa_file);
    return;
  }

  if (!vm.count("force-update") && isRegistered(game, moves.size())) {
    logInfo(method, "Skipped a registered kifu file.");
    return;
  }

  std::vector<SfenMove> sfen_moves;
  convertToUsi(csa_file, moves, sfen_moves);
  if (!vm.count("dry-run")) {
    registerMoves(game, sfen_moves);
  }

  { /* New Move */
    const std::pair<int,int> p = locateNewMove(game, sfen_moves);
    const int i     = p.first;
    const int count = p.second;

    if (i >= 0 && count >= 20) {
      logInfo(method, boost::format("Found new move!!!  %s %dth move (%d)") %
                      game.game_id % (i+2) % count);
      if (!vm.count("dry-run")) {
        addMoveEntry("NEW_MOVES", game.game_id, i+2);
      }
    }
  }

  { /* Threat Escaping Threat */
    rc::ThreatSearch threatSearch(70);
    threatSearch.setup();
    rc::ThreatSearchResult result = threatSearch.search(moves);
    if (result.checkmate_limit > 400000 && !vm.count("dry-run")) {
      logInfo(method, boost::format("Found a diffucult checkmate!!!  %s %dth move") %
                      game.game_id % result.checkmate_nth);
      addMoveEntry("CHECKMATE", game.game_id, result.checkmate_nth);
    }
    for (const int i : result.moves) {
      logInfo(method, boost::format("Found threat-escaping-threat!!!  %s %dth move") %
                      game.game_id % i);
      if (!vm.count("dry-run")) {
        addMoveEntry("THREAT_ESCAPING_THREAT", game.game_id, i);
      }
    }
  }
}

/*========== MAIN ==========*/

void printUsage(std::ostream& out, 
                char **argv,
                const bp::options_description& command_line_options)
{
  out << 
    "Usage: " << argv[0] << " [options]" << " csa_file ...\n"
      << command_line_options 
      << std::endl;
}

int parseCommandLine(int argc, char **argv, 
                     bp::variables_map& vm)
{
  bp::options_description command_line_options;
  command_line_options.add_options()
    ("dry-run",        "Do not modify the MySQL database.")
    ("force-update,u", "force to update table records")
    ("input-file",     bp::value<std::vector<std::string> >(),
     "input files in the CSA format")
    ("mysql-host",     bp::value<std::string>(&mysql_host)->default_value("localhost:3306"),
                       "MySQL database server address to connect to")
    ("mysql-user",     bp::value<std::string>(),
                       "MySQL database user to connect")
    ("mysql-password", bp::value<std::string>(),
                       "MySQL database password to connect")
    ("delete,d",       "Delete completed files")
    ("help,h",         "Show help message");
  bp::positional_options_description p;
  p.add("input-file", -1);

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
    if (!vm.count("input-file")) {
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

  try {
    logInfo(method, boost::format("Connecting to %s...") % mysql_host);

    driver = sql::mysql::get_mysql_driver_instance();
    con.reset(driver->connect(
               boost::str(boost::format("tcp://%s") % mysql_host),
               vm["mysql-user"].as<std::string>(), 
               vm["mysql-password"].as<std::string>()));
    con->setSchema("book");

    logInfo(method, "  connected."); 
  } catch (std::exception& e) {
    logError(method, e);
    return 1;
  }

  for(const std::string& csa_file : vm["input-file"].as<std::vector<std::string> >()) {
    try {
      const bf::path file(csa_file);
      if (!bf::exists(file)) {
        logWarn(method, boost::format("File not found %s...") % csa_file);
        continue;
      }

      logInfo(method, boost::format("Reading %s...") % csa_file);
      processFile(csa_file);
      if (vm.count("delete")) {
        bf::remove(bf::path(csa_file));
      }
      logInfo(method, "  finished.");
    } catch (std::exception& e) {
      logError(method, boost::format("Failed %s\n%s") % csa_file % e.what());
    }
  }

  return 0;
}
