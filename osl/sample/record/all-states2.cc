#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include "osl/record/kisen.h"
#include "osl/csa.h"
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>

struct hash
{
  unsigned long operator() (const osl::SimpleState &state) const
  {
    return osl::hash::HashKey(state).signature();
  }
};

void find_all(const int num_ply, const int threshold,
	      bool save, const std::vector<std::string> &files)
{
  std::unordered_set<osl::SimpleState, hash> states;

  for (size_t index = 0; index < files.size(); index++)
  {
    osl::record::KisenFile kisen(files[index]);
    for (size_t i = 0; i < kisen.size(); i++)
    {
      const auto moves = kisen.moves(i);
      osl::NumEffectState state(kisen.initialState());

      states.insert(state);
      for (size_t j = 0; j < moves.size() && j < static_cast<size_t>(num_ply);
	   j++)
      {
	const osl::Square opKingSquare 
	  = state.kingSquare(alt(state.turn()));
	if (state.hasEffectAt(state.turn(), opKingSquare))
	{
	  break;
	}
	state.makeMove(moves[j]);
	states.insert(state);
      }
    }
  }

  int index = 0;
  for (auto it=states.begin(); it != states.end(); ++it)
  {
    if (save)
    {
      std::ofstream output;
      output.open((boost::format("%05d.csa") % index++).str().c_str());
      output << *it;
      output.close();
    }
    else
    {
      std::cout << *it;
    }
  }
}

int main(int argc, char **argv)
{
  int num_ply;
  int threshold;
  bool save_moves;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("num-ply",
     boost::program_options::value<int>(&num_ply)->default_value(10),
     "Show states after this number of plies are played")
    ("threshold",
     boost::program_options::value<int>(&threshold)->default_value(10),
     "Each state must appear this number of times to be shown")
    ("save",
     boost::program_options::value<bool>(&save_moves)->default_value(false),
     "Save moves leading to states to files in CSA format")
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
  find_all(num_ply, threshold, save_moves, files);

  return 0;
}
