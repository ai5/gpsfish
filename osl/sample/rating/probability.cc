/* probability.cc
 */
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/record/kisen.h"
#include "osl/progress/effect5x3.h"
#include "osl/stat/average.h"
#include "osl/stat/histogram.h"
#include <iostream>
#include <cmath>
using namespace osl;
using namespace osl::rating;

int verbose = 0;
const char *kisen_filename="0.kif";
size_t num_kisen = 4096;
double verbose_limit = 0;

void show(const NumEffectState&, Move, const progress::Effect5x3& progress);
void show_statistics();

int main()
{
  KisenFile kisen_file(kisen_filename);
  for (size_t i=0; i<num_kisen; i++) {
    if (i % 16 == 0) 
      std::cerr << '.';
    NumEffectState state(kisen_file.initialState());
    const std::vector<Move> moves = kisen_file.moves(i);
    RatingEnv env;
    env.make(state);
    progress::Effect5x3 progress(state);
    for (size_t i=0; i<moves.size(); ++i) {
      if (state.inCheck(alt(state.turn())))
	break;			// illegal

      const Move move = moves[i];    
      show(state, move, progress);
      state.makeMove(move);
      env.update(state, move);
      progress.update(state, move);
    }
  }
  std::cerr << "\n";
  show_statistics();
}

std::vector<double> rating_to_probability(const RatedMoveVector& moves)
{
  double sum = 0;
  for (size_t i=0; i<moves.size(); ++i)
    sum += pow(10, moves[i].rating()/400.0);
  std::vector<double> result(moves.size());
  for (size_t i=0; i<moves.size(); ++i) 
    result[i] = pow(10, moves[i].rating()/400.0) / sum;
  return result;
}

stat::Average top_rated[16];
stat::Histogram histogram(1,10,0,true);

void show(const NumEffectState& state, Move next, const progress::Effect5x3& progress)
{
  static const StandardFeatureSet& feature_set = StandardFeatureSet::instance();
  RatedMoveVector moves;
  RatingEnv env;
  env.make(state);

  feature_set.generateRating(state, env, 2000, moves);
  std::vector<double> probability = rating_to_probability(moves);
  const RatedMove *rm = moves.find(next);
  top_rated[progress.progress16().value()].add(rm && rm == &moves[0]);
  if (rm)
    histogram.add(-log10(probability[rm - &moves[0]]));
  if (verbose || (rm && probability[rm - &moves[0]] < verbose_limit)) {
    std::cout << state;
    if (verbose > 1) {
      for (size_t i=0; i<probability.size(); ++i) {
	std::cout << csa::show(moves[i].move()) << " " << probability[i] << "\n";
      }
    }
    std::cout << "max " << probability.front() << " min " << probability.back()
	      << " selected " << csa::show(next) << " ";
    if (rm)
      std::cout << probability[rm - &moves[0]] << "\n";
    else
      std::cout << "move-not-found?!\n";
    std::cout << "\n";
  }
}

void show_statistics()
{
  for (int i=0; i<16; ++i)
    std::cout << "top " << i << " " << top_rated[i].average() << "\n";
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
