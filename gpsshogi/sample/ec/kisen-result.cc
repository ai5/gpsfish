#include "osl/apply_move/applyMove.h"
#include "osl/state/numEffectState.h"
#include "osl/state/hashEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/kisen.h"
#include "osl/checkmate/dualCheckmateSearcher.h"
#include <boost/program_options.hpp>

#include <iostream>

int main(int argc, char **argv)
{
  size_t start, end;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("start",
     boost::program_options::value<size_t>(&start)->default_value(0),
     "Start index of kisen file")
    ("end",
     boost::program_options::value<size_t>(&end)->default_value(60000),
     "End index of kisen file")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in kisen format")
    ("help", "Show help message");
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
      std::cerr << "Usage: " << argv[0] << " [options] kisen-file"
		<< std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] kisen-file" << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  const std::vector<std::string> files =
    vm["input-file"].as< std::vector<std::string> >();

  osl::record::KisenFile kisen(files[0]);
  std::string ipx(files[0]);
  ipx.replace(ipx.rfind("."), 4, ".ipx");
  osl::record::KisenIpxFile kisen_ipx(ipx);

  for (size_t i = start; i < kisen.size() && i < end; i++)
  {
    const osl::vector<osl::Move> moves = kisen.getMoves(i);
    osl::state::SimpleState simple_state = kisen.getInitialState();
    osl::state::NumEffectState state(simple_state);
    bool win_found = false;

    int result = kisen_ipx.getResult(i);
    if (result == osl::record::KisenIpxFile::JISHOGI ||
	result == osl::record::KisenIpxFile::JISHOGI_256 ||
	result == osl::record::KisenIpxFile::SENNNICHITE ||
	result == osl::record::KisenIpxFile::SENNNICHITE_256)
    {
      std::cout << "*" << std::endl;
      continue;
    }

    for (size_t j = 0; j < moves.size(); j++)
    {
      // win by foul
      if (osl::EffectUtil::isKingInCheck(osl::alt(state.turn()), state))
      {
	std::cout << state.turn() << state.turn() << std::endl;
	win_found = true;
	break;
      }
      if (!osl::EffectUtil::isKingInCheck(state.turn(), state))
      {
	osl::Move checkmate_move;
	osl::checkmate::AttackOracleAges age;
	const osl::PathEncoding path(state.turn());
	osl::checkmate::DualCheckmateSearcher<osl::checkmate::DominanceTable>
	  searcher;
	if (searcher.isWinningState(10000, state,
				    osl::state::HashEffectState(state).getHash(),
				    path, checkmate_move, age))
	{
	  std::cout << state.turn() << " " << j << std::endl;
	  win_found = true;
	  break;
	}
      }
      osl::apply_move::ApplyMoveOfTurn::doMove(state, moves[j]);
    }
    if (!win_found)
    {
      if (result == osl::record::KisenIpxFile::BLACK_WIN_256)
      {
	std::cout << osl::BLACK << std::endl;
      }
      else if (result == osl::record::KisenIpxFile::WHITE_WIN_256)
      {
	std::cout << osl::WHITE << std::endl;
      }
      else
      {
	std::cout << osl::alt(state.turn()) << std::endl;
      }
    }
  }

  return 0;
}
