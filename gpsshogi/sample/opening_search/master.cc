#include "book_traverser.h"
#include "gpsshogi/redis/redis.h"
#include "gpsshogi/redis/searchResult.h"
#include "osl/move.h"
#include "osl/player.h"
#include "osl/record/record.h"
#include "osl/record/opening/openingBook.h"
#include <hiredis/hiredis.h>
#include <glog/logging.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>


namespace bp = boost::program_options;
bp::variables_map vm;

redisContext *c = NULL;


osl::Player the_player = osl::BLACK;
int is_determinate = 0;	   // test only top n moves.  0 for all
int max_depth, non_determinate_depth;
double ratio;		   // use moves[n+1] when the weight[n+1] >= ratio*weight[n]


const std::string getMovesStr(const Node::moves_t& moves) {
  std::ostringstream ss;
  BOOST_FOREACH(const osl::Move move, moves) {
    osl::record::writeInt(ss, move.intValue());
  }
  return ss.str();
}


void setupServer(osl::Player player) {
  std::string key;
  if (player == osl::BLACK) {
    key = "DEL tag:black-positions";
  } else {
    key = "DEL tag:white-positions";
  }
  assert(!key.empty());
  gpsshogi::redis::redisReplyPtr reply((redisReply*)redisCommand(c, key.c_str()),
                                       gpsshogi::redis::freeRedisReply);
  if (gpsshogi::redis::checkRedisReply(reply))
    exit(1);
}


bool isFinished(const std::string& state_key) {
  gpsshogi::redis::redisReplyPtr reply((redisReply*)redisCommand(c, "EXISTS %b",
                                                                 state_key.c_str(), state_key.size()),
                                       gpsshogi::redis::freeRedisReply);
  if (gpsshogi::redis::checkRedisReply(reply))
    exit(1);

  const bool ret = (reply->integer == 1L);
  return ret;
}


int debug_appendPosition(osl::Player player, const Node& node) {
  const std::string state_key = node.state_key;
  const std::string moves_str = getMovesStr(node.moves);
  std::cerr << player << ", " << state_key << ", " << moves_str << std::endl;
  return 3; // two commands
}

void printUsage(std::ostream& out, 
                char **argv,
                const boost::program_options::options_description& command_line_options) {
  out <<
    "Usage: " << argv[0] << " [options] <a_joseki_file.dat>\n"
      << command_line_options 
      << std::endl;
}


class AppendRedisVisitor : public AbstractVisitor
{
  int counter;

public:
  static const int ncommands_per_position = 3;

  AppendRedisVisitor(osl::Player _player)
    : AbstractVisitor(_player),
      counter(0)
  {}
  ~AppendRedisVisitor() {}

  int getCounter() const         {return counter;}
  int getPositionCounter() const {return getCounter()/ncommands_per_position;}

  void notify(const Node& node);
};

void AppendRedisVisitor::notify(const Node& node)
{
  const std::string state_key = node.state_key;
  const std::string moves_str = getMovesStr(node.moves);

  redisAppendCommand(c, "SADD %s %b", "tag:new-queue", state_key.c_str(), state_key.size());
  
  if (getPlayer() == osl::BLACK) {
    redisAppendCommand(c, "SADD %s %b", "tag:black-positions", state_key.c_str(), state_key.size());
  } else {
    redisAppendCommand(c, "SADD %s %b", "tag:white-positions", state_key.c_str(), state_key.size());
  }

  redisAppendCommand(c, "HMSET %b moves %b",
                     state_key.c_str(), state_key.size(),
                     moves_str.c_str(), moves_str.size());

  counter += ncommands_per_position;
}

void doMain(const std::string& file_name) {
  LOG(INFO) << boost::format("Opening... %s") % file_name;
  osl::record::opening::WeightedBook book(file_name.c_str());

  setupServer(the_player);

  AppendRedisVisitor visitor(the_player);
  BookTraverser traverser(book,
                          visitor,
                          is_determinate, max_depth, non_determinate_depth, ratio);
  traverser.traverse();

  /* check results */
  LOG(INFO) << "Checking processed positions...: " << visitor.getPositionCounter();
  if (c->err)
    LOG(ERROR) << c->errstr;
  for (int i=0; i<visitor.getCounter(); ++i) {
    void *r;
    redisGetReply(c, &r);
    gpsshogi::redis::redisReplyPtr reply((redisReply*)r,
                                         gpsshogi::redis::freeRedisReply);
    if (i % 3 != 2) {
      assert(reply->type == REDIS_REPLY_INTEGER);
      assert(0 <= reply->integer);
      assert(reply->integer < 2);
    } else {
      gpsshogi::redis::checkRedisReply(reply);
    }
  }
}


int main(int argc, char **argv)
{
  std::string player_str;
  std::string file_name;
  std::string redis_server_host = "127.0.0.1";
  int redis_server_port = 6379;
  std::string redis_password;

  /* Set up logging */
  FLAGS_log_dir = ".";
  google::InitGoogleLogging(argv[0]);

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("player,p", bp::value<std::string>(&player_str)->default_value("black"),
     "specify a player, black or white, in whose point of view the book is validated. "
     "default black.")
    ("input-file,f", bp::value<std::string>(&file_name)->default_value("./joseki.dat"),
     "a joseki file to validate.")
    ("determinate", bp::value<int>(&is_determinate)->default_value(0),
     "only search the top n moves.  (0 for all,  1 for determinate).")
    ("non-determinate-depth", bp::value<int>(&non_determinate_depth)->default_value(100),
     "use the best move where the depth is greater than this value")
    ("max-depth", bp::value<int>(&max_depth)->default_value(100),
     "do not go beyond this depth from the root")
    ("redis-host", bp::value<std::string>(&redis_server_host)->default_value(redis_server_host),
     "IP of the redis server")
    ("redis-password", bp::value<std::string>(&redis_password)->default_value(redis_password),
     "password to connect to the redis server")
    ("redis-port", bp::value<int>(&redis_server_port)->default_value(redis_server_port),
     "port number of the redis server")
    ("ratio", bp::value<double>(&ratio)->default_value(0.0),
     "skip move[i] (i >= n), if weight[n] < weight[n-1]*ratio")
    ("verbose,v", "output verbose messages.")
    ("help,h", "show this help message.");
  bp::positional_options_description p;
  p.add("input-file", 1);

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

  if (player_str == "black")
    the_player = osl::BLACK;
  else if (player_str == "white")
    the_player = osl::WHITE;
  else {
    printUsage(std::cerr, argv, command_line_options);
    return 1;
  }

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

  doMain(file_name);

  redisFree(c);
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
