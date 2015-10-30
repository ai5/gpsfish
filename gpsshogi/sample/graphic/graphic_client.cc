#include "myPublisher.h"
#include "mySubscriber.h"
#include "pos2img.h"
#include <zmq.hpp>
#include <Magick++.h> 
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <cassert>

namespace bf = boost::filesystem;

int main(int argc, char* argv[]) 
{
  namespace bp = boost::program_options;
  std::string back_host = "127.0.0.1";
  int back_port = 17049; /// Port for back end
  std::string file_host = "127.0.0.1";
  int file_port = 18049; /// Port to trasnfer binary files
  int checkmate_limit = 0;

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("checkmate-limit,c", bp::value<int>(&checkmate_limit)->default_value(checkmate_limit),
                     "Limit for checkmate search. 0 meands disable.")
    ("back-host",   bp::value<std::string>(&back_host)->default_value(back_host),
                     "Host address to connect to the back end server")
    ("back-port,b", bp::value<int>(&back_port)->default_value(back_port),
                     "Port to connect to the back end server")
    ("file-host",   bp::value<std::string>(&file_host)->default_value(file_host),
                     "Host address to connect to the file server")
    ("file-port,f", bp::value<int>(&file_port)->default_value(file_port),
                     "Port to connect to the file server")
    ("help,h", "Show help message")
    ;
  bp::variables_map vm;
  try {
    bp::store(bp::command_line_parser(argc, argv).
              options(command_line_options).run(), vm);
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

  zmq::context_t context(1);

  MySubscriber subscriber(&context, back_host, back_port, false);
  MyPublisher send_file(&context, file_host, file_port, false);
  ProcessBoard pb(checkmate_limit);

  while (true) {
    std::string line;
    subscriber.subscribe(line);
    // ex. sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.png
    // ex. sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.1.moves.7g7f.png
    if (line.size() < 4) { // .png
      continue;
    }
    line.erase(line.size()-4, 4); // .png
    Magick::Blob blob;
    const std::string& file_name = pb.generate(line, blob);
    if (file_name.empty()) {
      std::cerr << "WARNING: a line did not work: " << line << std::endl;
      continue;
    }

    if (!send_file.publish(file_name,
                           reinterpret_cast<const char*>(blob.data()),
                           blob.length())) {
      std::cerr << "WARNING: Failed to send a file\n";
    }
  }

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
