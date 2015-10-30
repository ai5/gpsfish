#include "myPublisher.h"
#include "mySubscriber.h"
#include <zmq.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>

/**
 * Architecture:
 *
 * Frontend:
 *   graphic_rewritemap (apache)
 *     - connect to the broker server (front_host/port) [PUSH]
 *
 * Broker:
 *   graphic_broker
 *     - server port: 16049 (front_host/port) [PULL]
 *     - server port: 17049 (back_host/port)  [PUSH]
 *     - server port: 18049 (file_host/port)  [PULL]
 *
 * Backend:
 *   graphic_client
 *     - connect to the broker server [PULL]
 *     - connect to the file server   [PUSH]
 *
 * The frontend and broker run on the same box.
 * Multiple backends can run on separate/multile boxes.
 */


namespace bf = boost::filesystem;

void broker(zmq::context_t *context,
            const std::string& front_host,
            const int front_port,
            const std::string& back_host,
            const int back_port)
{
  MySubscriber subscriber(context, front_host, front_port, true);
  MyPublisher publisher(context, back_host, back_port, true);

  while (true) {
    std::string line;
    subscriber.subscribe(line);

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
}

void saveFile(zmq::context_t *context,
              const std::string& file_host,
              const int file_port,
              const bf::path dir /*copy*/)
{
  MySubscriber subscriber(context, file_host, file_port, true);

  while (true) {
    std::string filename;
    MySubscriber::binary_t binary;
    subscriber.subscribe(filename, binary);

    assert(!filename.empty());
    assert(!binary.empty());
    if (filename.empty() || binary.empty())
      continue;

    const bf::path f = dir / filename;
    std::ofstream out(f.string().c_str(), std::ios_base::binary | std::ios_base::trunc);
    out.write(&binary[0], binary.size());
  }
}

int main(int argc, char* argv[]) 
{
  namespace bp = boost::program_options;
  std::string a_dir("."); /// directory to put generated image files
  std::string front_host = "127.0.0.1";
  int front_port = 16049; /// Port for the frot end
  std::string back_host = "127.0.0.1";
  int back_port  = 17049; /// Port for the back end
  std::string file_host = "127.0.0.1";
  int file_port  = 18049; /// Port for file transfer.

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("front-host", bp::value<std::string>(&front_host)->default_value(front_host),
                     "Host address/name for the front end server")
    ("front-port", bp::value<int>(&front_port)->default_value(front_port),
                     "Port for the front end server")
    ("back-host", bp::value<std::string>(&back_host)->default_value(back_host),
                     "Host address/name for the back end server")
    ("back-port", bp::value<int>(&back_port)->default_value(back_port),
                     "Port for the back end server")
    ("file-host", bp::value<std::string>(&file_host)->default_value(file_host),
                     "Host address/name for the file server to receive binary files")
    ("file-port", bp::value<int>(&file_port)->default_value(file_port),
                     "Port for the file server to receive binary files")
    ("dir,d",  bp::value<std::string>(&a_dir)->default_value("."), 
               "Directory to put out files")
    ("help,h", "Show help message")
    ;
  try {
    bp::variables_map vm;
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

  boost::thread save_file(saveFile,
                          &context,
                          file_host,
                          file_port,
                          dir);
  boost::thread routing(broker,
                        &context,
                        front_host,
                        front_port,
                        back_host,
                        back_port);
  routing.join();
  save_file.join();

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
