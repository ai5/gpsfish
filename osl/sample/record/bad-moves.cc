#include <boost/program_options.hpp>
#include <iostream>
#include "osl/record/csaRecord.h"
#include "osl/record/record.h"
#include "osl/record/searchInfo.h"

static int eval_threshold = 128;
static int critical_drop = 64;

struct MoveData
{
  MoveData() : index(0), value(0), next_value(0) { }
  MoveData(size_t i, int v, int next_v)
    : index(i), value(v), next_value(next_v) { }
  size_t index;
  int value;
  int next_value;
};

void find_bad_moves(bool sente, const std::string &filename)
{
  osl::CsaFile file(filename);
  std::vector<osl::Move> moves;
  std::vector<std::string> dummy1;
  std::vector<int> time;
  std::vector<osl::record::SearchInfo> info;
  file.load().load(moves, time, dummy1, info);
  int prev_value = 0;
  std::vector<MoveData> bad_indices;
  
  for (size_t i = sente ? 0 : 1; i < info.size(); i += 2)
  {
    // skip joseki
    if (time[i] == 1 && info[i].value == 0 && prev_value == 0)
    {
    }
    else
    {
      if ((sente && info[i].value > -eval_threshold &&
	   info[i].value - prev_value < -critical_drop) ||
	  (!sente && info[i].value < eval_threshold &&
	   info[i].value - prev_value > critical_drop))
      {
	bad_indices.push_back(MoveData(i - 2, prev_value, info[i].value));
      }
    }
    prev_value = info[i].value;
  }
  osl::NumEffectState state(file.initialState());
  for (size_t i = 0, j = 0; i < moves.size() && j < bad_indices.size();
       i++)
  {
    if (bad_indices[j].index == i)
    {
      std::cout << state
		<< "' " <<  i << ": " << info[i].value << " -> "
		<< info[i+2].value<< std::endl
		<< osl::csa::show(moves[i]) << std::endl
		<< osl::csa::show(moves[i+1]) << std::endl
		<< osl::csa::show(moves[i+2]) << std::endl;
      std::vector<osl::Move> &pv_moves = info[i+2].moves;
      bool found_pass = false;
      for (size_t k = 0; k < pv_moves.size(); k++)
      {
	if (found_pass)
	  std::cout << "' ";
	if (pv_moves[k].isPass())
	{
	  if (!found_pass)
	    std::cout << "' ";
	  else
	    found_pass = true;
	  std::cout << "%PASS" << std::endl;
	}
	else
	{
	  std::cout << osl::csa::show(pv_moves[k]) << std::endl;
	}
      }
      j++;
    }
    state.makeMove(moves[i]);
  }
}

int main(int argc, char **argv)
{
  bool sente;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("sente",
     boost::program_options::value<bool>(&sente)->default_value(true),
     "Whether you want to check sente or gote moves")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in CSA format")
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
      std::cerr << "Usage: " << argv[0] << " [options] csa-file"
		<< std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] csa-file" << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  const std::vector<std::string> files =
    vm["input-file"].as< std::vector<std::string> >();
  for (size_t i = 0; i < files.size(); i++)
  {
    find_bad_moves(sente, files[i]);
  }
  return 0;
}
