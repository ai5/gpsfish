/* validate.cc
 */
#include "featureSet.h"
#include "osl/record/kisen.h"
#include "osl/book/openingBook.h"
#include "osl/game_playing/weightTracer.h"
#include "osl/numEffectState.h"
#include "osl/stat/variance.h"
#include "osl/csa.h"
#include "osl/eval/see.h"
#include "osl/oslConfig.h"

#include <boost/program_options.hpp>
#include <valarray>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <ctime>
#include <unistd.h>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;
std::unique_ptr<FeatureSet> feature_set;

size_t kisen_start, num_records;
int threshold, threshold_rank, randomize;
std::string kisen_filename;
bool report_even_in_book;

valarray_t weight_order;
stat::Average llikelihood, top_score;

weight_t run(const NumEffectState& state, const std::vector<Move>& history, int next_id, const StateInfo& info)
{
  const Move next(history[next_id]);
  MoveLogProbVector moves;
  WeightedMoveVector internals;
  weight_t sum = feature_set->generateRating(info, internals, weight_order);
  feature_set->ratingToLogProb(internals, sum, moves);
  // feature_set->generateLogProb(info, moves, weight_order);
  const MoveLogProb *p = moves.find(next);
  if (!p || moves.empty())
    return -1;
  const int l = p->logProb(), rank = p-&*moves.begin();
  llikelihood.add(l);
  if (l >= threshold || rank >= threshold_rank)
  {
    std::cerr << state << next << ' ' << l
	      << " (" << rank << " / " << moves.size() << ") "
	      << moves[0].move() << ' ' << moves[0].logProb() << "\n";
    feature_set->analyze(info, p->move(), weight_order);
    feature_set->analyze(info, moves[0].move(), weight_order);
  }
  top_score.add(moves.front().move() == next);
  assert(std::isnormal(llikelihood.average()));
#if 0
  std::cerr << state;
  for (size_t i=0; i<std::min(8u, moves.size()); ++i) {
    std::cerr << "  " << record::csa::show(moves[i].getMove())
	      << ' ' << moves[i].getLogProb();
  }
  std::cerr << "\n";
  MoveLogProbVector fixed_moves;
  weight_t s = info.progress.progress16().value() == 0 ? 50.0 : 100.0;
  feature_set->ratingToLogProb(internals, s, fixed_moves);
  for (size_t i=0; i<std::min(8u, fixed_moves.size()); ++i) {
    std::cerr << "  " << record::csa::show(fixed_moves[i].getMove())
	      << ' ' << fixed_moves[i].getLogProb();
  }
  std::cerr << "\n";
#endif
  return internals[rank].first;
}

void run()
{
  static WeightedBook book(OslConfig::openingBook());
  KisenFile kisen_file(kisen_filename.c_str());
  if (num_records == 0)
    num_records = kisen_file.size();
  std::vector<size_t> orders;
  orders.reserve(num_records);
  for (size_t i=kisen_start; i<std::min(kisen_start+num_records, kisen_file.size()); ++i)
    orders.push_back(i);
  if (randomize)
    random_shuffle(orders.begin(), orders.end());
  CArray<stat::Variance,8> variances;
  stat::Average average;
  for (size_t i=0; i<orders.size(); ++i) {
    game_playing::WeightTracer book_tracer(book);
    NumEffectState state(kisen_file.initialState());
    const auto moves=kisen_file.moves(orders[i]);
    StateInfo info(state, moves, -1);
    info.use_adhoc_adjust = true;
    for (size_t j=0; j<moves.size(); ++j) {
      if (state.inCheck(alt(state.turn())))
	break;
      if (book_tracer.isOutOfBook() || report_even_in_book) {
	weight_t sum = run(state, moves, j, info);
	if (sum > 0 && moves[j].capturePtype() == PTYPE_EMPTY
	    && ! moves[j].isPromotion()) {
	  variances[info.progress8()].add(sum);
	  average.add(sum);
#if 0
	  if (sum > 10)
	    std::cerr << state << moves[j] << "\n";
	  std::cerr << j << ' ' << sum << "\n";
#endif
	}
      }
      state.makeMove(moves[j]);
      info.update(moves[j]);
      book_tracer.update(moves[j]);
    }
    if (i % 64 == 0) {
      assert(std::isnormal(llikelihood.average()));
      std::cerr << std::setprecision(4) << std::setw(6)
		<< top_score.average()*100.0 << '%'
		<< std::setw(6) << llikelihood.average();
      std::cerr << " average sum " << average.average();
      for (stat::Variance& v: variances) {
	std::cerr << " " << v.average() << " (" << sqrt(v.variance())
		  << ')';
      }
      std::cerr << "\n";
    }
  }
}

int main(int argc, char **argv)
{
  nice(20);
  
  std::string initial_value;
  po::options_description options("all_options");

  options.add_options()
    ("num-records,n",
     po::value<size_t>(&num_records)->default_value(0),
     "number of records to be analyzed (all if 0)")
    ("kisen-start",
     po::value<size_t>(&kisen_start)->default_value(0),
     "start id of kisen records")
    ("kisen-file,k", po::value<std::string>(&kisen_filename),
     "filename for records to be analyzed")
    ("initial-value-file,f",
     po::value<std::string>(&initial_value)->default_value(""),
     "File with initial eval values")
    ("threshold,t",
     po::value<int>(&threshold)->default_value(2000),
     "show state and move if scaled log likelihood is more than threshold")
    ("rank-threshold",
     po::value<int>(&threshold_rank)->default_value(200),
     "show state and move if its rank is more than threshold")
    ("randomize,r",
     po::value<int>(&randomize)->default_value(0),
     "seed for radomize records to be used, -1 for auto, 0 for no-randomness")
    ("report-even-in-book",
     po::value<bool>(&report_even_in_book)->default_value(true),
     "ignore book move if false")
    ("help", "produce help message")
    ;

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
    if (kisen_filename == "")
      kisen_filename = "../../../data/kisen/01.kif";
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }
  if (vm.count("help")) {
    std::cerr << options << std::endl;
    return 0;
  }
  OslConfig::setUp();
  feature_set.reset(new StandardFeatureSet);
  {
    const char *base_filename = "move-order";
    feature_set->load(base_filename, weight_order);
  }
  if (randomize < 0)
    srand(time(0));
  else if (randomize > 0)
    srand(randomize);
  run();
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
