/* client.cc
 */
#include "protocol.h"

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;
namespace po = boost::program_options;

using namespace osl;
using namespace gpsshogi;
typedef std::valarray<double> valarray_t;


size_t num_cpus;
int main(int argc, char **argv)
{    
  std::string host, name;
  int port;
  po::options_description options("all_options");
  options.add_options()
    ("num-cpus,N",
     po::value<size_t>(&num_cpus)->default_value(1),
     "number cpus to be used")
    ("eval,e",
     po::value<std::string>(&config.eval_type)->default_value(std::string("piece")),
     "evaluation function (piece, kopenmidending, oslopenmidending)")
    ("host",
     po::value<std::string>(&host)->default_value(std::string("127.0.0.1")),
     "server host")
    ("port",
     po::value<int>(&port)->default_value(default_port),
     "port number")
    ("client-name",
     po::value<std::string>(&name)->default_value(std::string("")),
     "client name (required, should be unique)")
    ("help", "produce help message")
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
    return 1;
  }
  if (vm.count("help") || name == "") {
    std::cerr << options << std::endl;
    return 0;
  }
  try {
    ip::tcp::iostream s(host, boost::lexical_cast<std::string>(port));
    if (! s) {
      std::cerr << "connection failed\n";
      return 1;
    }
    EXPECT(s, server_version);
    write_line(s, "client " + name);
    EXPECT(s, client_ok);  
    write_line(s, bye);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  return 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
