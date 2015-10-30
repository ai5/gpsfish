#include <tcutil.h>
#include <tchdb.h>
#include "moves.pb.h"
#include "osl/record/compactBoard.h"
#include "osl/apply_move/applyMove.h"
#include "osl/state/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "gpsshogi/dbm/tokyoCabinet.h"

#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace osl;

static void valueToMoves(std::string str, std::vector<Move> &moves)
{
  Moves pv_moves;
  if (pv_moves.ParseFromString(str))
  {
    for (int i = 0; i < pv_moves.moves_size(); ++i)
    {
      moves.push_back(Move::makeDirect(pv_moves.moves(i)));
    }
  }
}

int main(int argc, char **argv)
{
  bool validate;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("validate",
     po::value<bool>(&validate)->default_value(false),
     "validate PV file")
    ;

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, command_line_options),
	      vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  gpsshogi::dbm::TokyoCabinetWrapper tc("pvs.tch", HDBOREADER);

  tc.initIterator();
  std::string key, value;
  while (tc.next(key, value))
  {
    record::CompactBoard board;
    std::istringstream iss(key);
    iss >> board;
    SimpleState state = board.getState();
    std::vector<Move> moves;
    valueToMoves(value, moves);
    if (!validate)
    {
      std::cout << state;
      for (size_t i = 0; i < moves.size(); ++i)
      {
	std::cout << record::csa::show(moves[i]) << " ";
      }
      std::cout << std::endl;
    }
    else
    {
      SimpleState new_state(state);
      for (size_t i = 0; i < moves.size(); ++i)
      {
	if (moves[i].isPass())
	{
	  new_state.changeTurn();
	  continue;
	}
	else if (!new_state.isValidMove(moves[i]))
	{
	  std::cout << "Invalid move" << moves[i] << std::endl
		    << new_state << std::endl;
	  break;
	}
	apply_move::ApplyMoveOfTurn::doMove(new_state, moves[i]);
      }
    }
  }
  return 0;
}
