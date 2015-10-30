/* filter.cc
 */
#include "gpsshogi/recorddb/recordDb.h"
#include "osl/state/numEffectState.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

unsigned int threshold, verbose;
int main(int argc, char **argv)
{
  std::string src, dst;
  po::options_description options;
  options.add_options()
    ("in", po::value<std::string>(&src)->default_value(""),
     "filename for source database")
    ("out", po::value<std::string>(&dst)->default_value(""),
     "filename for destination database")
    ("threshold,t", po::value<unsigned int>(&threshold)->default_value(2),
     "threshold that new db includes positions with more than or equal to this frequency")
    ("verbose,v", po::value<unsigned int>(&verbose)->default_value(0),
     "set verbose level")
    ("help,h", "Show help message");

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
  if (vm.count("help") || src == "" || dst == "")
  {
    std::cerr << "Usage: " << argv[0] << " [options] inputfile outputfile" << std::endl;
    std::cout << options << std::endl;
    return 1;
  }

  RecordDB dbin(src.c_str(), true);
  RecordDB dbout(dst.c_str(), false);
  dbin.initIterator();
  osl::record::MiniBoardChar50 key((SimpleState(HIRATE)));
  SquareInfo value;
  size_t all = 0, added = 0, stat1 = 0, stat2 = 0;
  while (dbin.next(key, value)) 
  {
    ++all;
    if (value.win() + value.loss() >= threshold)
    {
      dbout.put(key.toString(), value);
      if (++added % 1024 == 0)
	dbout.optimize();
      if (value.win() + value.loss() >= threshold+1)
	++stat1;
      if (value.win() + value.loss() >= threshold+2)
	++stat2;
    }
  }
  std::cerr << "read " << all << " wrote " << added << "\n";
  std::cerr << "stat " << stat1 << " " << stat2 << "\n";
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
