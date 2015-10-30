/* annotate.cc
 */
#include "osl/annotate/facade.h"
#include "osl/record/kakinoki.h"
#include "osl/record/record.h"
#include "osl/record/ki2.h"
#include "osl/usi.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

namespace po = boost::program_options;
using namespace osl;

void analyze_root(const NumEffectState& state, const std::vector<Move>& moves, int move_number);
int main(int argc, char **argv)
{
  po::options_description options;
  std::string filename;
  size_t start, end;
  options.add_options()
    ("filename,f", po::value<std::string>(&filename),
     "specify .kif or .ki2 file to be analyzed")
    ("start,s", po::value<size_t>(&start)->default_value(35),
     "skip first moves")
    ("end,e", po::value<size_t>(&end)->default_value(350),
     "skip first moves")
    ("help,h", "Show help message");
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
    if (vm.count("help")) {
      std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
      std::cout << options << std::endl;
      return 1;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (filename.empty())
    return 1;
  std::vector<Move> moves;
  NumEffectState state;
  try 
  {
    if (filename.find(".kif") == filename.size()-4) 
    {
      KakinokiFile file(filename);
      moves = file.moves();
      state = file.initialState();
    } 
    else if (filename.find(".ki2") == filename.size()-4) 
    {
      Ki2File file(filename);
      moves = file.moves();
      state = file.initialState();
    }
  }
  catch (KakinokiIOError&) 
  {
    return 1;
  }

  for (size_t i=0; i<moves.size(); ++i) 
  {
    state.makeMove(moves[i]);
    if (i+1 < start) 
      continue;
    std::cerr << i+1 << "\n";
    analyze_root(state, moves, i+1);
    if (i+1 >= end) 
      break;
  }
}

void analyze_root(const NumEffectState& src, const std::vector<Move>& moves, int move_number)
{
  std::ostringstream ret;
  ret << "[(" << move_number << ") ";
  NumEffectState s;
  if (move_number) 
  {
    for (int i=0; i<move_number-1; ++i)
      s.makeMove(moves[i]);
    ret << ki2::show(moves[move_number-1], s) << "]\n";
    s.makeMove(moves[move_number-1]);
  }

  annotate::AnalysesResult result;
  annotate::analyze(src, moves, move_number-1, result);
  if (result == annotate::AnalysesResult())
    return;
  ret << result;  
  std::cout << ret.str() << std::endl;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
