#include "pvFile.h"
#include "pvVector.h"
#include "quiesce.h"
#include "eval/eval.h"
#include "eval/evalFactory.h"
#include "osl/numEffectState.h"
#include "osl/record/kisen.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"

#include <boost/program_options.hpp>
#include <unordered_set>
#include <valarray>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;

int main(int argc, char **argv)
{
  std::string kisen_filename;
  std::string eval_type, eval_file;
  std::vector<std::string> pv_filenames;
  int to_watch;

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
     po::value<std::string>(&eval_type)->default_value(std::string("kopenmidending")),
     "evaluation function (piece, rich0, rich1)")
    ("to-watch",
     po::value<int>(&to_watch)->default_value(10));
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

  std::unordered_set<int> watch_indices;
  {
    std::valarray<double> tmp(my_eval->dimension());
    my_eval->saveWeight(&tmp[0]);
    std::valarray<double> abs_weight = std::abs(tmp);
    std::sort(&abs_weight[my_eval->lambdaStart()],
	      &abs_weight[0] + abs_weight.size());
    const int threshold = abs_weight[abs_weight.size() - to_watch - 1];
    for (size_t i = my_eval->lambdaStart(); i < tmp.size(); ++i)
    {
      if (std::abs(tmp[i]) > threshold)
      {
	watch_indices.insert(i);
      }
    }
  }

  std::valarray<int> original_count(my_eval->dimension());
  std::valarray<int> new_count(my_eval->dimension());
  original_count = 0;
  new_count = 0;

  std::valarray<double> weights(my_eval->dimension());
  my_eval->saveWeight(&weights[0]);
  gpsshogi::Quiesce quiesce(my_eval.get(), 1, 4);

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
      gpsshogi::MoveData data;
      std::unordered_set<int> found_features;
      while (pr.readPv(pv))
      {
	osl::NumEffectState current_state(state);
	for (size_t j = 0; j < pv.size(); ++j)
	{
	  current_state.makeMove(pv[j]);
	}
	const int& value = data.value;
	const auto& diffs = data.diffs;
	my_eval->features(current_state, data);
	for (size_t j = 0; j < diffs.size(); ++j)
	{
	  if (watch_indices.find(diffs[j].first) !=
	      watch_indices.end())
	  {
	    found_features.insert(diffs[j].first);
	    ++original_count[diffs[j].first];
	  }
	}
	const Move first_move = pv[0];
	data.clear();
	for (auto it = found_features.begin();
	     it != found_features.end();
	     ++it)
	{
	  osl::NumEffectState current_state(state);
	  current_state.makeMove(first_move);
	  double original_value = weights[*it];
	  weights[*it] = 0;
	  my_eval->setWeight(&weights[0]);
	  int dummy1;
	  quiesce.quiesce(current_state, dummy1, pv);
	  for (size_t j = 0; j < pv.size(); ++j)
	  {
	    current_state.makeMove(pv[j]);
	  }
	  my_eval->features(current_state, data);
	  for (size_t j = 0; j < diffs.size(); ++j)
	  {
	    if (watch_indices.find(diffs[j].first) !=
		watch_indices.end())
	    {
	      found_features.insert(diffs[j].first);
	      ++new_count[diffs[j].first];
	    }
	  }

	  // Restore state
	  weights[*it] = original_value;
	  my_eval->setWeight(&weights[0]);
	  data.clear();
	}
	found_features.clear();
      }
    }
  }

  for (auto it = watch_indices.begin(); it != watch_indices.end(); ++it)
  {
    std::cout << "Index: " << *it
	      << " "
	      << std::get<0>(my_eval->findFeature(*it))
	      << " Original count: " << original_count[*it]
	      << " Cleared count: " << new_count[*it] << std::endl;
  }
  return 0;
}
