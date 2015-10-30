#include "coordinator.h"
#include "csaServer.h"
#include "usiSlave.h"
#include "slaveManager.h"
#include "searchTree.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <signal.h>

bool read_multi_login(const std::string& multi_login_config, 
		      gpsshogi::CsaConfig& config)
{
  std::ifstream is(multi_login_config.c_str());
  if (!is)
    return false;
  std::string line;
  while (getline(is, line)) {
    std::cerr << "parse " << line << "\n";
    std::istringstream ss(line);
    gpsshogi::Account account;
    if (! (ss >> account.username >> account.password)) {
      std::cerr << "parse error " << line << "\n";
      return false;
    }
    int total, byoyomi;
    if (ss >> total >> byoyomi) { // optional
      account.limit_seconds = total;
      account.limit_byoyomi = byoyomi;
    }
    config.multi_login.push_back(account);
  }
  return ! config.multi_login.empty();
}

int main(int argc, char **argv)
{
  gpsshogi::CsaConfig config;
  unsigned int expected_slaves;
  int parallel_io, slave_tlp;
  bool slave_udp_log = false;
  std::string udp_log = "", slave_log_dir="", multi_login_config,
    mate_solver_key, accept_multi_slaves;
  int draw_value_cp;
  bool human_rounding=false;

  namespace po = boost::program_options;
  po::options_description options("Options");
  options.add_options()
    ("wcsc",
     po::value<int>(&config.wcsc)->default_value(0),
     "1 for wcsc (./remote-csa/{in,out} for io), fixed account.  "
     "2 for wcsc test (./remote-csa/{in,out} for io), configurable account.")
    ("server,S",
     po::value<std::string>(&config.servername)->default_value("wdoor.c.u-tokyo.ac.jp"),
     "csa server for game via tcp/ip")
    ("port,P",
     po::value<int>(&config.portnumber)->default_value(4081),
     "csa server for game via tcp/ip")
    ("username,u",
     po::value<std::string>(&config.username)->default_value("gpsshogi_expt"),
     "username for game via tcp/ip")
    ("password,p",
     po::value<std::string>(&config.password)->default_value("yowai_gps-1500-0"),
     "username for game via tcp/ip")
    ("multi-login-config",
     po::value<std::string>(&multi_login_config)->default_value(""),
     "multi-login via tcp/ip")
    ("limit-total-seconds",
     po::value<int>(&config.limit_seconds)->default_value(-1),
     "limit total time, regardless of that given by server")
    ("limit-byoyomi-seconds",
     po::value<int>(&config.limit_byoyomi)->default_value(-1),
     "limit byoyomi, regardless of that given by server")
    ("stdio",
     po::value<std::string>(&config.stdio_config)->default_value(""),
     "use stdio instead of tcp/ip (black or white)")
    ("games",
     po::value<int>(&config.games)->default_value(1),
     "number of games to play")
    ("slaves",
     po::value<unsigned int>(&expected_slaves)->default_value(1),
     "wait until specified number of slaves become ready")
    ("ponder", po::value<bool>(&config.ponder)->default_value(false),
     "ponder.")
    ("book-width-root", po::value<int>(&config.book_width_root)->default_value(16),
     "coefficient for minor moves to be considered (root).")
    ("book-width", po::value<int>(&config.book_width)->default_value(10),
     "coefficient for minor moves to be considered.")
    ("book-depth", po::value<int>(&config.book_depth)->default_value(35),
     "maximam depth for book.")
    ("send-pv", po::value<bool>(&config.send_info)->default_value(false),
     "send pv and evaluation value to server.")
    ("io-threads,N", po::value<int>(&parallel_io)->default_value(2),
     "parallel io.")
    ("udp-logging", po::value<std::string>(&udp_log)->default_value(""), "host:port for udp logging")
    ("slave-udp-log", po::value<bool>(&slave_udp_log)->default_value(false), "send udp log for slave-io")
    ("slave-log-directory",
     po::value<std::string>(&slave_log_dir)->default_value("io"),
     "specify where slave logs to go (\"\" for disable logging)")
    ("slave-tlp",
     po::value<int>(&slave_tlp)->default_value(0),
     "specify number of threads for all slaves, 0 for default")
    ("mate-solver-key", 
     po::value<std::string>(&mate_solver_key)->default_value("gpsshogi"),
     "if a slave's `id name' contains this keyword, the server treat him as a mate solver")
    ("accept-multiple-slaves-filename",
     po::value<std::string>(&accept_multi_slaves)->default_value(""),
     "filename of ip address where multiple slaves may login from, even when slave-tlp == 0")
    ("draw-value-cp",
     po::value<int>(&draw_value_cp)->default_value(0),
     "preference of draw in centi pawn (prefers if positive)")
    ("human-rounding",
     po::value<bool>(&human_rounding)->default_value(0),
     "specify true when time is measured in minutes, droping seconds)")
    ("help,h", "produce help message")
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
  if (vm.count("help")) {
    std::cout << options << std::endl;
    return 0;
  }

  if (multi_login_config != "") {
    if (config.wcsc) {
      std::cerr << "conflict options\n";
      return 1;
    }
    if (! read_multi_login(multi_login_config, config)) {
      std::cerr << "error in multi_login_config " << multi_login_config
		<< "\n";
      return 1;
    }
  }
  gpsshogi::UsiSlave::setMateSolverKey(mate_solver_key);
  gpsshogi::SearchTree::setDrawValueCP(draw_value_cp);
  gpsshogi::SearchTree::setHumanRounding(human_rounding);
  
  if (config.wcsc) {
    config.servername = "";		// do not send pv
    config.stdio_config = "";
    config.book_width_root = 8;
    config.book_width = 3;
    config.ponder = 1;
    slave_udp_log = true;
    udp_log = "gopteron0:4120";
    if (accept_multi_slaves == "")
      accept_multi_slaves = "multiple-slaves.txt";
    if (config.wcsc == 1) {
      expected_slaves = 600;
      config.username = "TeamGPS";
      config.password = "os4QRTvls";
    }
  }

  if (accept_multi_slaves != "")
    gpsshogi::SlaveManager::acceptMultipleSlaves(accept_multi_slaves);

  // option upstream(tcp or stdio), threading  
  gpsshogi::Coordinator world
    (new gpsshogi::CsaServer(config), expected_slaves, parallel_io, udp_log,
     slave_udp_log, slave_log_dir, slave_tlp);
  try {
    std::cerr << "From now on, this program ignores SIGINT\n";
    signal(SIGINT, SIG_IGN);
    world.start();
  }
  catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;    
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
