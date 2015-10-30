/* probability.cc
 */
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/progress/effect5x3.h"
#include "osl/stat/average.h"
#include "osl/stat/histogram.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <cmath>
using namespace osl;
using namespace osl::rating;
namespace po = boost::program_options;

size_t num_kisen, opening_skip, target_limit;

void run(NumEffectState& state, const std::vector<Move>& moves);
void show_statistics();

int main(int argc, char **argv)
{
  std::string kisen_filename;
  std::vector<std::string> filenames;
  po::options_description options("Options");
  options.add_options()
    ("csa-file", 
     po::value<std::vector<std::string> >(),
     "csa filename")
    ("kisen,k", 
     po::value<std::string>(&kisen_filename),
     "kisen filename")
    ("num-kisen", 
     po::value<size_t>(&num_kisen)->default_value(0),
     "number of records in kisen to be processed")
    ("target-limit", 
     po::value<size_t>(&target_limit)->default_value(1000),
     "ignore moves whose log-probability is greater than this threshold")
    ("opening-skip", 
     po::value<size_t>(&opening_skip)->default_value(20),
     "number of opening moves ignored in analysis");
  po::variables_map vm;
  po::positional_options_description p;
  p.add("csa-file", -1);
  try
  {
    po::store(po::command_line_parser(argc, argv).
	      options(options).positional(p).run(), vm);
    notify(vm);
    if (vm.count("help")) {
      std::cout << options << std::endl;
      return 0;
    }
    if (vm.count("csa-file"))
      filenames = vm["csa-file"].as<std::vector<std::string> >();
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    throw;
  }
  if (kisen_filename != "") {
    std::cerr << "kisen " << kisen_filename << "\n";
    KisenFile kisen_file(kisen_filename.c_str());
    if (num_kisen == 0)
      num_kisen = kisen_file.size();
    for (size_t i=0; i<num_kisen; i++) {
      if (i % 16 == 0) 
	std::cerr << '.';
      NumEffectState state(kisen_file.initialState());
      const std::vector<Move> moves = kisen_file.moves(i);
      run(state, moves);
    }
  }
  for (size_t i=0; i<filenames.size(); ++i) {
    if (i % 16 == 0) 
      std::cerr << '.';
    CsaFileMinimal file(filenames[i]);
    NumEffectState state(file.initialState());
    const std::vector<Move> moves = file.moves();
    run(state, moves);
  }
  std::cerr << "\n";
  show_statistics();
}

CArray<stat::Average,8> top_rated, active;

void show(const NumEffectState& state, Move next)
{
  static const StandardFeatureSet& feature_set = StandardFeatureSet::instance();
  MoveLogProbVector moves;
  RatingEnv env;
  env.make(state);

  feature_set.generateLogProb(state, env, 2000, moves);
  const MoveLogProb *rm = moves.find(next);
  if (! rm)
    return;
  int index = rm - &*moves.begin();
  for (size_t i=0; i<top_rated.size(); ++i) {
    if (i >= moves.size())
      break;
    bool a = moves[i].logProb() <= (int)target_limit;
    active[i].add(a);
    if (! a)
      continue;
    bool found = index == (int)i;
    top_rated[i].add(found);
  }
}

void run(NumEffectState& state, const std::vector<Move>& moves)
{
  for (size_t i=0; i<moves.size(); ++i) {
    if (state.inCheck(alt(state.turn())))
      break;			// illegal

    const Move move = moves[i];
    if (i >= opening_skip)
      show(state, move);
    state.makeMove(move);
  }
}

void show_statistics()
{
  for (size_t i=0; i<top_rated.size(); ++i)
    std::cout << "top " << i 
	      << " " << top_rated[i].average()*100.0 
	      << " " << active[i].average()*100.0 
	      << "\n";
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
