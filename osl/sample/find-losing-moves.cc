/* find-illegal-moves.cc
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
void run(const NumEffectState& initial, const std::vector<Move>& moves) 
{
  game_playing::GameState state(initial);
  for (size_t i=0; i<moves.size(); ++i){
    MoveVector normal, loss;
    state.generateNotLosingMoves(normal, loss);
    bool show = ! loss.empty() || ! normal.isMember(moves[i]);
    if (show) {
      std::cerr << state.state();
      std::cerr << "history ";
      for (size_t j=0; j<=i; ++j)
	std::cerr << csa::show(moves[j]);
      std::cerr << "\n";
    }
    if (! loss.empty()) {
      std::cerr << "losing moves ";
      for (Move m: loss) {
	std::cerr << csa::show(m);
      }
      std::cerr << "\n";
    }
    if (! normal.isMember(moves[i]))
      std::cerr << "error? " << moves[i] << "\n";
    state.pushMove(moves[i]);
    
  }
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
      auto moves = kisen.moves(i);
      run(state, moves);
    }
  }
  for (size_t i=0; i<filenames.size(); ++i) {
    std::cerr << '.';
    CsaFile file(filenames[i].c_str());
    NumEffectState state(file.initialState());
    auto moves = file.load().moves();
    run(state, moves);
  }  
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

