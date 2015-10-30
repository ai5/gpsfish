/* find-uplevel-king.cc
 */
#include "osl/game_playing/gameState.h"
#include "osl/record/kisen.h"
#include "osl/record/csaRecord.h"
#include "osl/sennichite.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <cmath>
namespace po = boost::program_options;

using namespace osl;
int count = 0;
bool run(const NumEffectState& initial, const std::vector<Move>& moves) 
{
  NumEffectState state(initial);
  for (size_t i=0; i<moves.size(); ++i){
    state.makeMove(moves[i]);
    Player P = alt(state.turn());
    if (state.kingSquare(P).squareForBlack(P).y() < 5) {
      // std::cerr << state << moves[i] << "\n";
      return true;
    }
  }
  return false;
}


int main(int argc, char **argv) {
  std::string kisen_filename;
  po::options_description options("Options");
  options.add_options()
    ("kisen,k", 
     po::value<std::string>(&kisen_filename),
     "kisen filename")
    ("csa-file", po::value<std::vector<std::string> >())
    ("help", "produce help message")
    ;
  po::positional_options_description p;
  p.add("csa-file", -1);

  po::variables_map vm;
  std::vector<std::string> filenames;
  try {
    po::store(po::command_line_parser(argc, argv).
	      options(options).positional(p).run(), vm);
    notify(vm);
    if (vm.count("help")) {
      std::cout << options << std::endl;
      return 0;
    }
    if (vm.count("csa-file"))
      filenames = vm["csa-file"].as<std::vector<std::string> >();
  }
  catch (std::exception& e) {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }

  if (kisen_filename != "") {
    KisenFile kisen(kisen_filename);
    for (size_t i=0; i<kisen.size(); ++i) {
      std::cerr << '.';
      NumEffectState state(kisen.initialState());
      std::vector<Move> moves = kisen.moves(i);
      if (run(state, moves))
	std::cout << i << "\n";
    }
  }
  for (size_t i=0; i<filenames.size(); ++i) {
    std::cerr << '.';
    CsaFile file(filenames[i].c_str());
    NumEffectState state(file.initialState());
    auto moves = file.load().moves();
    if (run(state, moves))
      std::cout << filenames[i] << "\n";
  }  
  std::cerr << "count = " << count << "\n";
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

