#include "myPublisher.h"
#include <zmq.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>

namespace bf = boost::filesystem;

/**
 * Return
 *  - 1 : Not found
 *  - 0 : already exist
 *  - -1: error
 */
int parseLine(boost::filesystem::path dir, 
              const std::string& line, 
              std::string& output)
{
  try {
    const bf::path request = dir / line;
    if (bf::exists(request)) {
      output.assign(line);
      return 0;
    } else {
      // output.assign("NULL");
      return 1;
    }
  } catch(std::exception& e) {
    std::cerr << "  Exception: " <<  line << ": " << 
                 e.what() << std::endl;
    return -1;
  }
  return 0;
}

int main(int argc, char* argv[]) 
{
  namespace bp = boost::program_options;
  std::string a_dir("/home/beatles/public_html/wdoor/images"); /// directory to put generated image files
  std::string host = "127.0.0.1";
  int port = 16049; /// Port for the server to listen

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("front-host", bp::value<std::string>(&host)->default_value(host), 
                   "Host address/name to connect to the broker serve")
    ("front-port", bp::value<int>(&port)->default_value(port),
                   "Port to connect to the broker server")
    ("dir,d",      bp::value<std::string>(&a_dir)->default_value(a_dir), 
                   "Directory to look for image files")
    ("help,h",     "Show help message")
    ;
  bp::variables_map vm;
  try {
    bp::store(bp::command_line_parser(argc, argv).
              options(command_line_options).run(), 
              vm);
    bp::notify(vm);
    if (vm.count("help")) {
      std::cout << command_line_options << std::endl;
      return 0;
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cout << command_line_options << std::endl;
    return 1;
  }

  const bf::path dir = bf::system_complete(bf::path(a_dir));
  if (!bf::exists(dir)) {
    std::cerr << "ERROR: Directory not found: " << dir << std::endl;
    std::cout << command_line_options << std::endl;
    return 1;
  }

  /** main */ 
  zmq::context_t context(1);
  MyPublisher publisher(&context, host, port, false);

  std::string line;
  while (std::getline(std::cin, line)) {
    boost::trim(line);
    std::string output("NULL");
    const int rc = parseLine(dir, line, output);
    if (rc == 1) {
      try {
        const bool rc = publisher.publish(line);
        if (!rc)
          std::cerr << "ERROR: send\n";
      } catch (zmq::error_t& ze) {
        std::cerr << ze.what() << std::endl;
      } catch (...) {
        // ingore errors.
        std::cerr << "ERROR: unknown\n";
      }
    }

    std::cout << output << std::endl; // flush 
    // "NULL<CR>" denotes null. 
  }

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
