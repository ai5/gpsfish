#include "coordinator.h"
#include "usiServer.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

int main(int argc, char **argv)
{
  unsigned int expected_slaves;
  int parallel_io, slave_tlp = 0;
  std::string udp_log = "", slave_log_dir="";

  namespace po = boost::program_options;
  po::options_description options("Options");
  options.add_options()
    ("slaves",
     po::value<unsigned int>(&expected_slaves)->default_value(1),
     "wait until specified number of slaves become ready")
    ("io-threads,N", po::value<int>(&parallel_io)->default_value(2),
     "parallel io.")
    ("udp-logging", po::value<std::string>(&udp_log)->default_value(""), "host:port for udp logging")
    ("slave-log-directory",
     po::value<std::string>(&slave_log_dir)->default_value("io"),
     "specify where slave logs to go (\"\" for disable logging)")
    ("slave-tlp",
     po::value<int>(&slave_tlp)->default_value(0),
     "specify number of threads for all slaves, 0 for default")
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
  
  // option upstream(tcp or stdio), threading
  gpsshogi::Coordinator world
    (new gpsshogi::UsiServer, expected_slaves, parallel_io, udp_log,
     false, slave_log_dir, slave_tlp);
  try {
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
