/* sgd.cc
 */
#include "featureSet.h"
#include "osl/eval/see.h"
#include "osl/record/kisen.h"
#include "osl/book/openingBook.h"
#include "osl/game_playing/weightTracer.h"
#include "osl/numEffectState.h"
#include "osl/misc/milliSeconds.h"
#include "osl/stat/average.h"
#include "osl/eval/pieceEval.h"
#include "osl/hashKey.h"
#include "osl/oslConfig.h"

#include <boost/program_options.hpp>
#include <unordered_set>
#include <thread>
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

size_t num_cpus, min_frequency, min_frequency_record;
size_t kisen_start, num_records;
std::string kisen_filename;
weight_t eta0;
const char *base_filename_order = "move-order";
const char *filename_prob = "move-tactical.txt";
int min_rating;
bool ignore_negative_samples = false;

valarray_t weight_order, weight_prob, partial;
stat::Average llikelihood, top_score, top4_score, top8_score, top16_score, order, skip_moves;

// todo: l1-regularizer e.g., tsuruoka et al. 2009
// p(y|x) = 1/Z(x) \exp(\sum_i w_i f_i(y,x)) ... y:move, x:position
// Z(x) = sum_y \exp(\sum_i w_i f_i(y,x))

// \log p(y_0|x) = \sum_i w_i f_i(y_0,x) - \log(Z(x))
// {\sum_i w_i f_i(y_0,x)}' = f_i(y_0,x)
// {\log(Z(x))}' = 1/Z(x) \sum_y (f_i(y,x) \exp(\sum_i w_i f_i(y,x)))
void run_order(const NumEffectState& state, const std::vector<Move>& history, int next_id, const StateInfo& info)
{
  const Move next(history[next_id]);
  if (! next.isNormal())
    return;
  MoveVector moves;
  state.generateLegal(moves);
  std::vector<std::pair<size_t,weight_t> > partial_a, partial_b;
  index_list_t features;
  weight_t selected = feature_set->matchExp(info, next, features, weight_order);
  for (const index_list_t::value_type& p: features) {
    partial_a.push_back(std::make_pair(p.first, (weight_t)p.second));
    partial_b.push_back(std::make_pair(p.first, selected*p.second));
  }
  weight_t z = selected+1e-10, best = selected;
  if (! std::isnormal(z)) {
    std::cerr << state << " " << next << ' ' << z << "\n";
    abort();
  }
  assert(std::isnormal(z));
  for (Move move: moves) {
    if (move == next)
      continue;
    weight_t s = feature_set->matchExp(info, move, features, weight_order);
    if (ignore_negative_samples && s < selected)
      continue;
    best = std::max(best, s);
    z += s;
    for (const index_list_t::value_type& p: features) {
      partial_b.push_back(std::make_pair(p.first, s*p.second));
    }
  }
  if (z < 1e-10)
    return;
  for (size_t i=0; i<partial_a.size(); ++i)
    partial[partial_a[i].first] += partial_a[i].second;
  for (size_t i=0; i<partial_b.size(); ++i)
    partial[partial_b[i].first] -= partial_b[i].second / z;
  llikelihood.add(log(selected/z));
  top_score.add(best == selected);
  assert(std::isnormal(llikelihood.average()));
}

// f(x,y) = \sum_i w_i f_i(x,y) // y:move, x:position
// p(x,y) = 1/(1 + \exp(-f(x,y))
// p' = p(1-p)f'
void run_prob(const NumEffectState& state, const std::vector<Move>& history, int next_id, const StateInfo& info)
{
  const Move next(history[next_id]);
  if (! next.isNormal())
    return;
  MoveLogProbVector moves;
  feature_set->generateLogProb(info, moves, weight_order);
  if (moves.empty())
    return;
  weight_t next_p = -1.0, see_best_sum = -1e9;
  Move see_best;
  for (MoveLogProb mp: moves) {
    Move target = mp.move();
    index_list_t dummy;
    double p = -1.0, sl = feature_set->matchLight(info, target, dummy, weight_order);
    const int see = osl::See::see(info.state, target);
    if (next_id > 0 && target.to() == history[next_id-1].to()) {
      p = PredictionModelLight::addGradientTakeBack(info.progress8(), next, target, sl,
						    weight_prob, partial);
      if (next == target)
	next_p = p;
    }
    if (see > 0) {
      if (see_best_sum < sl) {
	see_best_sum = sl;
	see_best = target;
      }
    }
    else
      continue;
#if 0
    if (next == target)
      break;
#endif
  }
  if (see_best.isNormal()) {
    weight_t p = PredictionModelLight::addGradientSeePlus(info, next, see_best, see_best_sum,
							  weight_prob, partial);
    
    if (next_p < 0 && next == see_best)
      next_p = p;
#if 0
      if (info.progress8() == 0 && next != target && osl::See::see(info.state, next) <= 0 && see >= eval::PtypeEvalTraits<LANCE>::val*2)
	std::cerr << state << next << ' ' << target << "\n";
#endif
  }
  if (next_p < 0)
    return;
  llikelihood.add(log(next_p));
}

void add_statistics(const NumEffectState& state, Move selected, const StateInfo& info)
{
  MoveLogProbVector moves;
  feature_set->generateLogProb(info, moves, weight_order);
  
  const MoveLogProb *p = moves.find(selected);
  if (!p || moves.empty())
    return;
  int rank = p-&*moves.begin();
  top4_score.add(rank < 4);
  top8_score.add(rank < 8);
  top16_score.add(rank < 16);
  order.add(rank);
}

void run(bool learn_order)
{
  static WeightedBook book(OslConfig::openingBook());
  std::unordered_set<HashKey,std::hash<HashKey>> visited;
  // partial.resize(feature_set->dimension());
  partial.resize(learn_order ? feature_set->dimension() : PredictionModelLight::dimension());
  partial = 0.0;
  KisenFile kisen_file(kisen_filename.c_str());
  std::unique_ptr<KisenIpxFile> ipx;
  if (min_rating)
    ipx.reset(new KisenIpxFile(KisenFile::ipxFileName(kisen_filename)));

  std::cerr << kisen_filename << ' ' << kisen_file.size() << "\n";
  if (num_records == 0)
    num_records = kisen_file.size();
  std::vector<size_t> orders;
  orders.reserve(std::min(num_records, kisen_file.size()));
  for (size_t i=kisen_start; i<std::min(kisen_start+num_records, kisen_file.size()); ++i)
    orders.push_back(i);
  random_shuffle(orders.begin(), orders.end());
  for (size_t i=0; i<orders.size(); ++i) {
    game_playing::WeightTracer book_tracer(book);
    NumEffectState state(kisen_file.initialState());
    const auto moves=kisen_file.moves(orders[i]);
    StateInfo info(state, moves, -1);
    CArray<int,2> rating = {{ 0,0 }};
    if (min_rating) {
      rating[BLACK] = ipx->rating(i, BLACK);
      rating[WHITE] = ipx->rating(i, WHITE);
    }
    for (size_t j=0; j<moves.size(); ++j) {
      HashKey key(state);
      key.add(moves[j]);
      const bool first_time = j>=40||book_tracer.isOutOfBook()||visited.insert(key).second;
      if (first_time && rating[state.turn()] >= min_rating) {
	if (learn_order)
	  run_order(state, moves, j, info);
	else
	  run_prob(state, moves, j, info);
	if ((i+j) % 16 == 15)
	  add_statistics(state, moves[j], info);
	skip_moves.add(0);
      } else {
	skip_moves.add(1);
      }
      state.makeMove(moves[j]);
      if (state.inCheck(alt(state.turn())))
	break;
      info.update(moves[j]);
      book_tracer.update(moves[j]);
    }
    if (i % 256 == 255) {
      valarray_t & target = learn_order ? weight_order : weight_prob;
      target += partial * (eta0/(1.0+(double)i/orders.size()));
      partial = 0.0;
      assert(llikelihood.average() == 0.0 || std::isnormal(llikelihood.average()));
      std::cerr << std::setw(6) << llikelihood.average()* 100.0 / log(0.5)
		<< std::setprecision(4) << std::setw(6)
		<< top_score.average()*100.0 << "%"
		<< " (4) " << top4_score.average()*100.0 << "%"
		<< " (8) " << top8_score.average()*100.0 << "%"
		<< " (16) " << top16_score.average()*100.0 << "%"
		<< "  rank " << order.average()
		<< " skip " << skip_moves.average();
      std::cerr << "\n";
      if (! learn_order) {
	for (size_t j=0; j<weight_prob.size(); ++j)
	  std::cerr << (j % 4 ? " " : "\n ") << weight_prob[j];
	std::cerr << "\n";
      }
      if (i % 1024 == 1023) {
	if (learn_order) {
	  feature_set->showSummary(target);
	  feature_set->save(base_filename_order, target);
	}
	else {
	  PredictionModelLight::save(filename_prob, target);
	}
      }
    }
  }
}

int main(int argc, char **argv)
{
  nice(20);
  srand(time(0));
  
  std::string learning_target;
  po::options_description options("all_options");

  options.add_options()
    ("num-records,n",
     po::value<size_t>(&num_records)->default_value(0),
     "number of records to be analyzed (all if 0)")
    ("num-cpus,N",
     po::value<size_t>(&num_cpus)->default_value(1),
     "number cpus to be used")
    ("kisen-start",
     po::value<size_t>(&kisen_start)->default_value(0),
     "start id of kisen records")
    ("kisen-file,k", po::value<std::string>(&kisen_filename),
     "filename for records to be analyzed")
    ("min-rating",
     po::value<int>(&min_rating)->default_value(0),
     "use records played by players with raing more than or equal to threshold")
    ("eta",
     po::value<weight_t>(&eta0)->default_value(0.001),
     "initial eta")
    ("learning-target,l",
     po::value<std::string>(&learning_target)->default_value("order"),
     "order | probability")
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
  if (num_cpus == 0 || (int)num_cpus > std::thread::hardware_concurrency()) {
    std::cerr << "do you really have so many cpus? " << num_cpus << "\n";
    return 1;
  }    
  OslConfig::setUp();
  feature_set.reset(new StandardFeatureSet);
  feature_set->load(base_filename_order, weight_order);
  PredictionModelLight::load(filename_prob, weight_prob);
  run(learning_target == "order");
  feature_set->save(base_filename_order, weight_order);
  PredictionModelLight::save(filename_prob, weight_prob);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
