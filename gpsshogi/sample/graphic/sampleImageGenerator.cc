#include "pos2img.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <cassert>

/**
 * USAGE: LANG=C ./sampleImageGenerator -d images --checkmate-limit 100000 sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.png
 */

int main(int argc, char **argv)
{
  namespace bp = boost::program_options;
  namespace bf = boost::filesystem;
  std::string a_dir("."); 
  size_t checkmate_limit = 0;

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("checkmate-limit", bp::value<size_t>(&checkmate_limit)->default_value(checkmate_limit),
                     "Limits for checkmate search (ex. 100000). 0 (default) means disable.")
    ("dir,d",  bp::value<std::string>(&a_dir)->default_value(a_dir),
               "Directory to put out files")
    ("input-positions", bp::value<std::vector<std::string> >(),
                        "Input positions")
    ("help,h", "Show help message")
    ;
  boost::program_options::positional_options_description p;
  p.add("input-positions", -1);

  bp::variables_map vm;
  try {
    bp::store(bp::command_line_parser(argc, argv).
              options(command_line_options).positional(p).run(), 
              vm); bp::notify(vm);
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

  std::vector<std::string> input_positions;
  if (vm.count("input-positions")) {
    input_positions = vm["input-positions"].as<std::vector<std::string> >();
  }

  ProcessBoard pb(checkmate_limit);
  BOOST_FOREACH(std::string line, input_positions) {
    boost::trim(line);

    if (line.size() < 4) { // .png
      continue;
    }
    line.erase(line.size()-4, 4); // .png
    // ex. lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.png // Hirate 
    const std::string file_name = pb.generate(line, dir);
    if (file_name.empty())
      std::cout << "ERROR: failed to generate an image" << std::endl;
    else
      std::cout << file_name << std::endl;
  }

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
