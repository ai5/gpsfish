/* show-ki2.cc
 */

#include "osl/record/kanjiMove.h"
#include "osl/record/ki2.h"
#include <boost/program_options.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace boost::lambda;

bool quiet = false;

void process( const std::string& file_name)
{
  std::cout << "Processing... " << file_name << std::endl;
  osl::Ki2File ki2(file_name, !quiet);
  if (quiet)
    return;

  const osl::Record record = ki2.load();
  const std::vector<osl::Move> moves = record.moves();
  std::for_each(moves.begin(), moves.end(),
                std::cout << _1 << "\n"  );
}

/**
 * Show .ki2 files (known as 2ch format).
 *
 * Record database: http://wiki.optus.nu/shogi/index.php?page=FrontPage
 * Archive to download: http://maris-stella.hp.infoseek.co.jp/
 * Note that the archiver called DGCA format only supports Windows.
 */
int main(int argc, char **argv)
{
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("quiet,q", "quiet output")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in ki2 format (.ki2)")
    ("help,h", "Show help message");
  boost::program_options::variables_map vm;
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
      std::cout << 
        "Usage: " << argv[0] << " [options] ki2-file [ki2-file...]"
		  << std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
    if (vm.count("quiet"))
      quiet = true;
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << 
        "Usage: " << argv[0] << " [options] ki2-file [ki2-file...]"
		  << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  const std::vector<std::string> files =
    vm["input-file"].as< std::vector<std::string> >();
  std::for_each(files.begin(), files.end(), [](std::string f){ process(f); });

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
