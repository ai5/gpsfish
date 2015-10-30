#include "moveData.h"
#include "pvFile.h"
#include "pvVector.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "osl/numEffectState.h"
#include "osl/record/kisen.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"

#include <boost/program_options.hpp>
#include <valarray>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;

int main(int argc, char **argv)
{
  std::string kisen_filename;
  std::string eval_type, eval_file;
  std::vector<std::string> pv_filenames;
  double positive_ratio, negative_ratio;

  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("pv-file", po::value<std::vector<std::string> >(),
     "filename containing PVs")
    ("kisen-file,k",
     po::value<std::string>(&kisen_filename)->default_value(""),
     "Kisen filename corresponding to pv file")
    ("eval-file,f",
     po::value<std::string>(&eval_file)->default_value(""),
     "Eval data filename")
    ("eval,e",
     po::value<std::string>(&eval_type)->default_value(std::string("piece")),
     "evaluation function (piece, rich0, rich1)")
    ("positive-ratio",
     po::value<double>(&positive_ratio)->default_value(0.9),
     "Show features with more than this positive example ratio")
    ("negative-ratio",
     po::value<double>(&negative_ratio)->default_value(0.9),
     "Show features with less than this negative example ratio")
    ;
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, command_line_options), vm);
    po::notify(vm);
    if (vm.count("pv-file"))
      pv_filenames = vm["pv-file"].as<std::vector<std::string> >();
    else
    {
      std::cerr << "PV file wasn't specified" << std::endl;
      return 1;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  OslConfig::setUp();
  if (!osl::eval::ml::OpenMidEndingEval::setUp()) {
    std::cerr << "OpenMidEndingEval set up failed";
    return 1;
  }
  if (!osl::progress::ml::NewProgress::setUp()) {
    std::cerr << "NewProgress set up failed";
    return 1;
  }

  std::unique_ptr<gpsshogi::Eval> my_eval;
  my_eval.reset(gpsshogi::EvalFactory::newEval(eval_type));
  if (my_eval == NULL)
  {
    std::cerr << "unknown eval type " << eval_type << std::endl;
    return 1;
  }
  if (!my_eval->load(eval_file.c_str()))
  {
    std::cerr << "failed to load eval data: "
	      << eval_type << " " << eval_file << std::endl;
    return 1;
  }

  osl::record::KisenFile kisen(kisen_filename);
  std::vector<osl::Move> moves;
  osl::NumEffectState state(kisen.initialState());
  std::valarray<int> positive_position_count(my_eval->dimension());
  std::valarray<int> negative_position_count(my_eval->dimension());
  positive_position_count = 0;
  negative_position_count = 0;

  for (size_t i = 0; i < pv_filenames.size(); ++i)
  {
    gpsshogi::PVFileReader pr(pv_filenames[i].c_str());
    int record, position;
    int cur_record = -1;
    int cur_position = 0;
    while (pr.newPosition(record, position))
    {
      if (record != cur_record)
      {
	cur_record = record;
	moves = kisen.moves(cur_record);
      }
      if (position == 0)
      {
	state = osl::NumEffectState(kisen.initialState());
	cur_position = 0;
      }
      else
      {
	while (position > cur_position)
	{
	  state.makeMove(moves[cur_position]);
	  ++cur_position;
	} 
      }
      gpsshogi::PVVector pv;
      bool first = true;
      gpsshogi::MoveData data;
      while (pr.readPv(pv))
      {
	osl::NumEffectState current_state(state);
	for (size_t j = 0; j < pv.size(); ++j)
	{
	  current_state.makeMove(pv[j]);
	}
	my_eval->features(state, data);

	if (first)
	{
	  for (size_t j = 0; j < data.diffs.size(); ++j)
	  {
	    ++positive_position_count[data.diffs[j].first];
	  }
	  first = false;
	}
	else
	{
	  for (size_t j = 0; j < data.diffs.size(); ++j)
	  {
	    ++negative_position_count[data.diffs[j].first];
	  }
	}
	pv.clear();
      }
    }
  }

  std::valarray<double> scaled_weights(my_eval->dimension());
  my_eval->saveWeight(&scaled_weights[0]);
  for (size_t i = 0; i < my_eval->dimension(); ++i)
  {
    const int sum = positive_position_count[i] + negative_position_count[i];
    if (sum > 10)
    {
      const double ratio =
	static_cast<double>(positive_position_count[i]) / sum;
      if (ratio < negative_ratio || ratio > positive_ratio)
      {
	std::cout << "index " << i << " "
		  << std::get<0>(my_eval->findFeature(i))
		  << " " << ratio
		  << " " << sum
		  << " " << positive_position_count[i]
		  << " " << negative_position_count[i]
		  << " " << scaled_weights[i]
		  << std::endl;
      }
    }
  }
  return 0;
}
