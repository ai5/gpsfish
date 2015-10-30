#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include "osl/record/kisen.h"
#include "osl/csa.h"

#include <boost/program_options.hpp>

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

struct hash
{
  unsigned long operator() (const osl::SimpleState &state) const
  {
    return osl::HashKey(state).signature();
  }
};

struct equalKey
{
  bool operator() (const osl::SimpleState &s1, const osl::SimpleState &s2) const
  {
    return s1 == s2;
  }
};

struct State
{
  State() : count(0)
  {
  }
  int count;
  std::vector<osl::Move> moves;
};

void find_all(const int num_ply, const int threshold,
	      bool save, const std::vector<std::string> &files)
{
  std::unordered_map<osl::SimpleState, State, hash, equalKey> states;

  for (size_t index = 0; index < files.size(); index++)
  {
    osl::record::KisenFile kisen(files[index]);
    for (size_t i = 0; i < kisen.size(); i++)
    {
      const auto moves = kisen.moves(i);
      osl::NumEffectState state(kisen.initialState());

      size_t j = 0;
      for (; j < moves.size() && (int)j < num_ply; j++)
      {
	const osl::Square opKingSquare 
	  = state.kingSquare(alt(state.turn()));
	if (state.hasEffectAt(state.turn(), opKingSquare))
	{
	  break;
	}
	state.makeMove(moves[j]);
      }
      if ((int)j == num_ply)
      {
	auto it = states.find(state);
	if (it != states.end())
	{
	  (it->second.count)++;
	}
	else
	{
	  State s;
	  s.count = 1;
	  for (int k = 0; k < num_ply; k++)
	  {
	    s.moves.push_back(moves[k]);
	  }
	  states[state] = s;
	}
      }
    }
  }

  int index = 1;
  for (auto it = states.cbegin(); it != states.cend(); ++it)
  {
    if (it->second.count >= threshold)
    {
      std::cout << index << " (" << it->second.count << ")" << std::endl;
      std::cout << it->first;
      std::ofstream output;
      if (save)
      {
	std::ostringstream oss(std::ostringstream::out);
	oss << index << ".csa";
	const std::string &filename = oss.str();
	output.open(filename.c_str());
	output << "PI" << std::endl
	       << "+" << std::endl;
      }
      const auto &moves = it->second.moves;
      for (size_t i = 0; i < moves.size(); i++)
      {
	std::cout << osl::csa::show(moves[i]) << " ";
	if (save)
	{
	  output << osl::csa::show(moves[i]) << std::endl;
	}
      }
      if (save)
      {
	output.close();
      }
      std::cout << std::endl;
      index++;
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
