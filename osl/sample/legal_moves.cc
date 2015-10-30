/* legal_moves.cc
 */
#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/search/quiescenceGenerator.tcc"
#include "osl/bits/king8Info.h"
#include <boost/program_options.hpp>
#include <iostream>
using namespace osl;

/**
 * @file 合法手の表示
 */

int main(int argc, char **argv)
{
  bool csa, generate_check, quiesce_check;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("csa",
     boost::program_options::value<bool>(&csa)->default_value(false),
     "Show legal moves in CSA format")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in kisen format")
    ("generate-check", 
     boost::program_options::value<bool>(&generate_check)->default_value(false),
     "generate only check moves instead of all legal moves")
    ("generate-quiesce-check", 
     boost::program_options::value<bool>(&quiesce_check)->default_value(false),
     "generate only check moves used in quiescence search")
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
      std::cerr << "Usage: " << argv[0] << " [options] CSA_FILE1 CSA_FILE2 ..."
		<< std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] CSA_FILE1 CSA_FILE2 ..."
	      << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  const std::vector<std::string> files =
    vm["input-file"].as< std::vector<std::string> >();

  for (size_t i = 0; i < files.size(); ++i)
  {
    const auto record = CsaFileMinimal(files[i]).load();
    NumEffectState state(record.initialState());
    const auto moves=record.moves;
    for (auto p=moves.begin(); p!=moves.end(); ++p)
    {
      state.makeMove(*p);
    }
    MoveVector legal_moves;
    if (generate_check)
    {
      move_action::Store store(legal_moves);
      move_generator::GenerateAddEffectWithEffect::generate<true>
	(state.turn(), state, state.kingPiece(alt(state.turn())).square(), store);
    }
    else if (quiesce_check) 
    {
      const checkmate::King8Info info(state.Iking8Info(state.turn()));
      if (state.turn() == BLACK)
	search::QuiescenceGenerator<BLACK>::check(state, state.pin(WHITE), legal_moves, info.libertyCount()==0);
      else
	search::QuiescenceGenerator<WHITE>::check(state, state.pin(WHITE), legal_moves, info.libertyCount()==0);
    }
    else 
    {
      state.generateLegal(legal_moves);
    }
    if (i > 0)
    {
      std::cout << std::endl;
    }
    if (csa)
    {
      for (size_t i = 0; i < legal_moves.size(); ++i)
      {
	std::cout << osl::csa::show(legal_moves[i]) << std::endl;
      }
    }
    else
    {
      std::cout << legal_moves;
    }
  }
}



/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
