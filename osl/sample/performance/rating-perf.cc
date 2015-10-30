#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/eval/progressEval.h"
#include "osl/effect_util/sendOffSquare.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/csaRecord.h"
#include "osl/stat/average.h"
#include "osl/misc/perfmon.h"

#include <boost/format.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdio>
using namespace osl;
using namespace osl::rating;

/**
 * @file 
 * rating付加の速度を測る
 */

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-v] [-f skip] csafiles\n"
       << endl;
  exit(1);
}

size_t first_skip = 0;
bool verbose = false;

stat::Average moves, cycles, cycles_per_move, probs, order, top_score, selected_score;
int min_selected = 1000;

void test_file(const FeatureSet&, const char *filename);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
    
  char c;
  while ((c = getopt(argc, argv, "f:vh")) != EOF)
  {
    switch(c)
    {
    case 'f':	first_skip = atoi(optarg);
      break;
    case 'v':	verbose = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (argc < 1))
    usage(program_name);

  eval::ProgressEval::setUp();
  StandardFeatureSet f;

  for (int i=0; i<argc; ++i)
  {
    if (i % 128 == 0)
      std::cerr << '.';
    test_file(f, argv[i]);
  }

  std::cout << "\n"
	    << "average moves/position " << moves.average() << "\n"
	    << "average order " << order.average() << "\n"
	    << "average selected score " << selected_score.average() << "\n"
	    << "min selected score " << min_selected << "\n"
	    << "average top score " << top_score.average() << "\n";
  std::cout << "average cycles/position " << cycles.average() << "\n"
	    << "average cycles/position/move " << cycles_per_move.average()
	    << "\n";
}

/* ------------------------------------------------------------------------- */

size_t num_positions = 0;
void test_position(const FeatureSet& f, Move next_move, Move last_move, const RatingEnv& env,
		   const NumEffectState& state)
{
  RatedMoveVector my_moves;

  misc::PerfMon clock;
  f.generateRating(state, env, 1400, my_moves);
  
  const size_t consumed = clock.stop();
  if (my_moves.size())
    top_score.add(my_moves[0].rating());
  const RatedMove *p = my_moves.find(next_move);
  int count = my_moves.size();
  int order = p ? p - &*my_moves.begin() +1 : count;
  if (p) {
    ::order.add(order);
    if (p->rating() < min_selected)
      min_selected = p->rating();
    if (p->rating() < -2000) {
      std::cerr << state << "selected " << *p << "\n" << my_moves;
    }
  }
  else {
    ::order.add(count);
  }
  moves.add(count);
  cycles.add(consumed);
  cycles_per_move.add(consumed/count);
  ++num_positions;

}

void test_file(const FeatureSet& f, const char *filename)
{
  RecordMinimal record;
  try {
    record = CsaFileMinimal(filename).load();
  }
  catch (CsaIOError& e) {
    std::cerr << "skip " << filename <<"\n";
    std::cerr << e.what() << "\n";
    return;
  }
  catch (...) {
    throw;
  }
  
  NumEffectState state(record.initialState());
  const auto& moves=record.moves;

  RatingEnv env;
  env.make(state);
  for (size_t i=0; i<moves.size(); ++i) {
    const Move move = moves[i];
    assert(state.isValidMove(move));
    if (i >= first_skip) {
      test_position(f, moves[i], (i>0 ? moves[i-1] : Move::PASS(alt(moves[i].player()))),
		    env, state);
    }
    state.makeMove(move);
    env.update(state, move);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
