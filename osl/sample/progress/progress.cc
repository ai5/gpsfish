#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/record/kisen.h"
#include "osl/search/moveWithComment.h"
#include "osl/sennichite.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
  std::string kisen_filename;
  int kisen_index;
  po::options_description options("Options");
  options.add_options()
    ("kisen,k", 
     po::value<std::string>(&kisen_filename),
     "kisen filename")
    ("index,i",
     po::value<int>(&kisen_index)->default_value(0))
    ("help", "produce help message")
    ;
  po::positional_options_description p;
  po::variables_map vm;

  try
  {
    po::store(po::command_line_parser(argc, argv).
	      options(options).positional(p).run(), vm);
    notify(vm);
    if (vm.count("help"))
    {
      std::cout << options << std::endl;
      return 0;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }

  osl::record::KisenFile kisen(kisen_filename);
  osl::NumEffectState state(kisen.initialState());
  std::vector<osl::Move> moves = kisen.moves(kisen_index);

  osl::progress::ml::NewProgress::setUp();
  osl::progress::ml::NewProgress progress(state);

  for (size_t i = 0; i < moves.size() + 1; ++i)
  {
    if (!state.inCheck())
    {
      // 16().value
      std::cout << i << " " << progress.progress() << std::endl;
    }
    if (i < moves.size())
    {
      state.makeMove(moves[i]);
      progress.update(state, moves[i]);
    }
  }

  return 0;
}
