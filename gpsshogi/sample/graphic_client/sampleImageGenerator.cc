#include "pos2img.h"
#include <osl/oslConfig.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <cassert>

/**
 * USAGE: % LANG=C ./sampleImageGenerator -d images --checkmate-limit 100000 "sfen lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b -"
 */

int main(int argc, char **argv)
{
  namespace bp = boost::program_options;
  namespace bf = boost::filesystem;
  std::string a_dir("."); 
  std::string title;
  std::string last_move;
  std::string black_name;
  std::string white_name;
  size_t checkmate_limit = 0;
  std::string file_name;

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("black-name,b", bp::value<std::string>(&black_name)->default_value(black_name),
                     "Black player's name")
    ("checkmate-limit", bp::value<size_t>(&checkmate_limit)->default_value(checkmate_limit),
                     "Limits for checkmate search (ex. 100000). 0 (default) means disable.")
    ("dir,d",  bp::value<std::string>(&a_dir)->default_value(a_dir),
               "Directory to put out files")
    ("input-positions", bp::value<std::vector<std::string> >(),
                        "Input positions")
    ("last-move,l",  bp::value<std::string>(&last_move)->default_value(last_move),
                     "Last move")
    ("output,o",     bp::value<std::string>(&file_name)->default_value(file_name),
                     "Output file name")
    ("title,t",      bp::value<std::string>(&title)->default_value(title),
                     "Title")
    ("white-name,w", bp::value<std::string>(&white_name)->default_value(white_name),
                     "White player's name")
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

  osl::OslConfig::setUp();

  ProcessBoard pb(checkmate_limit);
  for (std::string line : input_positions) {
    boost::trim(line);

//std::cout << line << std::endl;
    MetaData md;
    md.title      = title;
    md.black_name = black_name;
    md.white_name = white_name;
    md.last_move  = last_move;
    Magick::Blob blob;
    pb.generate(line, md, blob);
    if (blob.length() == 0)
      std::cout << "ERROR: failed to generate an image" << std::endl;
    else {
      std::ofstream out(file_name.c_str(), std::ios::binary);
      out.write((const char*)blob.data(), blob.length());
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
