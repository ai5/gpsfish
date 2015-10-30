#include "osl/eval/openMidEndingEval.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/progress.h"
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
  auto moves = kisen.moves(kisen_index);

  osl::eval::ml::OpenMidEndingEval::setUp();
  osl::progress::ml::NewProgress::setUp();

  osl::game_playing::GameState game_state(state);
  osl::game_playing::AlphaBeta2OpenMidEndingEvalPlayer player;
  player.setDepthLimit(600, 400, 200);
  if (osl::OslConfig::isMemoryLimitEffective()) 
  {
    player.setTableLimit(std::numeric_limits<size_t>::max(), 200);
    player.setNodeLimit(std::numeric_limits<size_t>::max());
  }

  for (size_t i = 0; i < moves.size() + 1; ++i)
  {
    if (!game_state.state().inCheck())
    {
      osl::search::MoveWithComment result = player.selectBestMove(game_state, 0, 0, 10);
      osl::NumEffectState state2(game_state.state());
      state2.changeTurn();
      osl::game_playing::GameState game_state2(state2);
      osl::search::MoveWithComment pass_result = player.selectBestMove(game_state2, 0, 0, 10);
      int diff = result.value - pass_result.value;

      std::cout << i << " " << result.value << " " << pass_result.value << " "
                << (i % 2 == 0 ? diff : -diff) << std::endl;
    }
    if (i < moves.size())
    {
      game_state.pushMove(moves[i]);
    }
  }

  return 0;
}
