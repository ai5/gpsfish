/**
 * Validate moves in ki2 files.
 */

#include "osl/record/ki2.h"
#include "osl/record/checkDuplicate.h"
#include "osl/record/ki2IOError.h"
#include "osl/csa.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

// ----------------------------------------------
// Global variables
// ----------------------------------------------
boost::program_options::variables_map vm;
osl::record::CheckDuplicate check_duplicate;

void process( const std::string& file_name)
{
  try {
    bool verbose = false;
    if (vm.count("verbose"))
      verbose = true;
    bool show_each_move = vm.count("show-moves");
    if (verbose)
      std::cerr << "Processing...  " << file_name << std::endl;
    const osl::Ki2File ki2(file_name, verbose);
    const osl::Record& record = ki2.load();
    const auto& moves = record.moves();

    if (check_duplicate.regist(moves)) {
      std::cerr << "Found a duplicated play: " << file_name << "\n";
      return;
    }

    osl::NumEffectState state;
    for (const osl::Move move: moves)
    {
      if (!state.isValidMove(move, false))
      {
        std::cout << file_name << "\n";
        continue;
      }
      if (show_each_move)
	std::cout << move.from().x() << move.from().y()
		  << ' ' << move.to().x() << move.to().y()
		  << ' ' << osl::csa::show(move.oldPtype())
		  << ' ' << osl::csa::show(move.ptype())
		  << ' ' << osl::csa::show(move.capturePtype())
		  << std::endl;
      state.makeMove(move);
    }
  } catch (osl::Ki2IOError& err) {
    std::cerr << err.what() << "\n";
    std::cerr << "Found an error: " << file_name << "\n";
    return;
  }
}

int main(int argc, char **argv)
{
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in ki2 format (.ki2)")
    ("show-moves", "show each move")
    ("verbose,v", "Verbose mode")
    ("help,h", "Show this help message");
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  try
  {
    boost::program_options::store(
      boost::program_options::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
      std::cout << "Usage: " << argv[0] << " [options] ki2-file [ki2-file...]\n";
      std::cout << "       " << argv[0] << " [options]\n";
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] ki2-file [ki2-file...]\n";
    std::cerr << "       " << argv[0] << " [options]\n";
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  std::vector<std::string> files;
  if (vm.count("input-file"))
  {
    const std::vector<std::string> temp = vm["input-file"].as<std::vector<std::string> >();
    files.insert(files.end(), temp.begin(), temp.end());
  }
  else
  {
    std::string line;
    while(std::getline(std::cin , line))
    {
      boost::algorithm::trim(line);
      files.push_back(line);
    } 
  }

  // main
  std::for_each(files.begin(), files.end(), [](std::string f){ process(f); });

  check_duplicate.print(std::cout);

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
