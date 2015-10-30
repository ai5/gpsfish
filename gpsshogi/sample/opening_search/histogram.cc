#include "gpsshogi/redis/redis.h"
#include "gpsshogi/redis/searchResult.h"
#include "osl/record/compactBoard.h"
#include "osl/record/csa.h"
#include "osl/record/kanjiPrint.h"
#include "osl/record/ki2.h"
#include "osl/state/simpleState.h"
#include <hiredis/hiredis.h>
#include <glog/logging.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

/**
 * Global variables
 */

namespace bp = boost::program_options;
bp::variables_map vm;
osl::Player the_player = osl::BLACK;
std::string the_player_str = "black";
int depth = 900;
int ban_criteria = 150;

redisContext *c = NULL;

void getAllBoards(std::vector<osl::record::CompactBoard>& boards)
{
  const std::string key = "tag:" + the_player_str + "-positions";
  gpsshogi::redis::redisReplyPtr reply((redisReply*)redisCommand(c, "SMEMBERS %s", key.c_str(),
                                       gpsshogi::redis::freeRedisReply));
  if (gpsshogi::redis::checkRedisReply(reply))
    exit(1);
  assert(reply->type == REDIS_REPLY_ARRAY);

  if (reply->elements == 0) {
    LOG(WARNING) << "No board found";
    return;
  }

  DLOG(INFO) << "size: " << reply->elements;
  boards.reserve(reply->elements);
  for(size_t i=0; i<reply->elements; ++i) {
    const redisReply *r = reply->element[i];
    assert(r->type == REDIS_REPLY_STRING);
    assert(r->len == 41*4);
    const std::string key(r->str, r->len);
    std::stringstream ss;
    ss << key;
    osl::record::CompactBoard cb;
    ss >> cb;
    boards.push_back(cb);
  }
}

void dump_score(const std::vector<gpsshogi::redis::SearchResult>& results, osl::Player player)
{
  const std::string file_name = "score_" + the_player_str + ".csv";
  std::ofstream out(file_name.c_str(), std::ios_base::trunc);

  LOG(INFO) << "Writing to " << file_name << "...";
  size_t missed = 0;

  /* Header */
  out << "EVAL" << std::endl;

  /* Rows */
  BOOST_FOREACH(const gpsshogi::redis::SearchResult& sr, results) {
    const osl::SimpleState state = sr.board.getState();
    /* Filter results */
    if (sr.depth >= depth) {
      out << sr.score << std::endl;
    } else {
      missed += 1;
    }
  }  

  LOG(INFO) << "  misses: " << missed;
}

void dump_position(const std::vector<gpsshogi::redis::SearchResult>& results, osl::Player player)
{
  const std::string file_name = "position_" + the_player_str + ".txt";
  std::ofstream out(file_name.c_str(), std::ios_base::trunc);

  LOG(INFO) << "Writing to " << file_name << "...";
  size_t missed = 0;

  /* Rows */
  BOOST_FOREACH(const gpsshogi::redis::SearchResult& sr, results) {
    if (sr.depth < depth) {
      missed += 1;
      continue;
    }

    const osl::SimpleState state = sr.board.getState();

    osl::Move last_move;
    if (!sr.moves.empty())
      last_move = sr.moves.back();

    /* parse pv */
    std::vector<osl::Move> pv_moves;
    {
      osl::NumEffectState state(sr.board.getState());
      for (size_t i=0; i<sr.pv.size(); ++i) {
        if (sr.pv[i] == '+' || sr.pv[i] == '-' || sr.pv[i] == '%') {
          for (size_t j=i+1; true; ++j) {
            if (j == sr.pv.size() || sr.pv[j] == '+' || sr.pv[j] == '-' || sr.pv[j] == '%') {
              const osl::Move move = osl::record::csa::strToMove(sr.pv.substr(i,(j-i)), state);
              pv_moves.push_back(move);
              state.makeMove(move);
              i = j-1;
              break;
            }
          } // for j
        }
      } // for i
    }
    assert(!pv_moves.empty());

    out << "score: " << sr.score << "\n" <<
           gpsshogi::redis::stateToString(state, last_move) <<
           "moves("<< sr.moves.size() << "): " <<
                        osl::record::ki2::show(&*sr.moves.begin(), &*sr.moves.end(),
                                               osl::NumEffectState()) << "\n" <<
           "depth: " << sr.depth << "\n" <<
           "secs:  " << sr.consumed_seconds << "\n" <<
           "pv:    " << osl::record::ki2::show(&*pv_moves.begin(), &*pv_moves.end(),
                                               osl::NumEffectState(state)) << "\n" <<
           "at:    " << sr.timeString() <<
           std::endl;
  }  

  LOG(INFO) << "  misses: " << missed;
}

bool isDisadvantaged(const gpsshogi::redis::SearchResult& sr, osl::Player player)
{
  assert(ban_criteria >= 0);

  /**
   * 黒の手をbanする <=> 黒が指し終わった局面 <=> 白手番の局面
   */
  if (sr.board.turn() == player)
    return false;

  if (sr.depth >= depth &&
      ( (player == osl::BLACK && sr.score < (ban_criteria * -1)) ||
        (player == osl::WHITE && sr.score > ban_criteria))) {
    return true;
  }

  return false;
}

void appendBanFile(std::ostream& out, const gpsshogi::redis::SearchResult& sr)
{
  std::vector<std::string> csa_moves;
  csa_moves.reserve(sr.moves.size());
  BOOST_FOREACH(osl::Move move, sr.moves) {
    csa_moves.push_back(osl::record::csa::show(move));
  }
  out << "# score: " << sr.score << ", " <<
         "depth: "   << sr.depth << ", " <<
         "secs: "    << sr.consumed_seconds << "\n";
  out << boost::algorithm::join(csa_moves, " ") << std::endl;
}

void dump_ban_positions(const std::vector<gpsshogi::redis::SearchResult>& results, osl::Player player)
{
  const std::string file_name = "ban_" + the_player_str + ".txt";
  std::ofstream out(file_name.c_str(), std::ios_base::trunc);

  LOG(INFO) << "Writing to " << file_name << "...";
  BOOST_FOREACH(const gpsshogi::redis::SearchResult& sr, results) {
    /* Filter results */
    if (isDisadvantaged(sr, player)) {
      appendBanFile(out, sr);
    }
  }  
}

void doMain()
{
  /* Retreive search results */
  std::vector<gpsshogi::redis::SearchResult> results;
  {
    std::vector<osl::record::CompactBoard> boards;
    getAllBoards(boards);
    LOG(INFO) << "Loaded candidate boards: " << boards.size();

    results.reserve(boards.size());
    BOOST_FOREACH(const osl::record::CompactBoard& cb, boards) {
      results.push_back(gpsshogi::redis::SearchResult(cb));
    }
    gpsshogi::redis::querySearchResult(c, results);
  }

  LOG(INFO) << "Sorting search results...";
  if (the_player_str == "black")
    std::sort(results.begin(), results.end(), gpsshogi::redis::SearchResultCompare());
  else
    std::sort(results.rbegin(), results.rend(), gpsshogi::redis::SearchResultCompare());

  LOG(INFO) << "Dumping to files...";
  dump_score(results, the_player);
  dump_position(results, the_player);
  dump_ban_positions(results, the_player);
}

void printUsage(std::ostream& out, 
                char **argv,
                const boost::program_options::options_description& command_line_options)
{
  out <<
    "Usage: " << argv[0] << " [options]\n"
      << command_line_options 
      << std::endl;
}

int main(int argc, char **argv)
{
  std::string redis_server_host = "127.0.0.1";
  int redis_server_port = 6379;
  std::string redis_password;

  /* Set up logging */
  FLAGS_log_dir = ".";
  google::InitGoogleLogging(argv[0]);

  /* Parse command line options */
  bp::options_description command_line_options;
  command_line_options.add_options()
    ("ban-criteria", bp::value<int>(&ban_criteria)->default_value(ban_criteria),
     "list up positions which eval value is worse than ban-criteria in a black list file")
    ("depth", bp::value<int>(&depth)->default_value(depth),
     "depth to filter")
    ("player,p", bp::value<std::string>(&the_player_str)->default_value(the_player_str),
     "specify a player, black or white.")
    ("redis-host", bp::value<std::string>(&redis_server_host)->default_value(redis_server_host),
     "IP of the redis server")
    ("redis-password", bp::value<std::string>(&redis_password)->default_value(redis_password),
     "password to connect to the redis server")
    ("redis-port", bp::value<int>(&redis_server_port)->default_value(redis_server_port),
     "port number of the redis server")
    ("help,h", "show this help message.");
  bp::positional_options_description p;

  try {
    bp::store(
      bp::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    bp::notify(vm);
    if (vm.count("help")) {
      printUsage(std::cout, argv, command_line_options);
      return 0;
    }
  } catch (std::exception &e) {
    std::cerr << "error in parsing options\n"
	      << e.what() << std::endl;
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

  if (the_player_str == "black")
    the_player = osl::BLACK;
  else if (the_player_str == "white")
    the_player = osl::WHITE;
  else {
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

  if (ban_criteria < 0) {
    std::cerr << "Ban criteria should be positive." << std::endl;
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

  /* Connect to the Redis server */
  gpsshogi::redis::connectRedisServer(&c, redis_server_host, redis_server_port);
  if (!c) {
    LOG(FATAL) << "Failed to connect to the Redis server";
    exit(1);
  }
  if (!redis_password.empty()) {
    if (!gpsshogi::redis::authenticate(c, redis_password)) {
      LOG(FATAL) << "Failed to authenticate to the Redis server";
      exit(1);
    }
  }

  /* MAIN */
  doMain();

  /* Clean up things */
  redisFree(c);
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
