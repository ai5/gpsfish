#include "osl/eval/see.h"
#include "osl/eval/pieceEval.h"
#include "osl/effect_util/pin.h"
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

/**
 * @file 
 * see付加の速度を測る
 */

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-v] [-f skip] [-o] csafiles\n"
       << endl;
  exit(1);
}

size_t first_skip = 0;
bool verbose = false;
bool old = false;

stat::Average moves, cycles, cycles_per_move;

void test_file(const char *filename);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
    
  char c;
  while ((c = getopt(argc, argv, "f:ovh")) != EOF)
  {
    switch(c)
    {
    case 'f':	first_skip = atoi(optarg);
      break;
    case 'v':	verbose = true;
      break;
    case 'o':	old = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (argc < 1))
    usage(program_name);

  for (int i=0; i<argc; ++i)
  {
    if (i % 128 == 0)
      std::cerr << '.';
    test_file(argv[i]);
  }

  std::cout << "average cycles/position " << cycles.average() << "\n"
	    << "average cycles/position/move " << cycles_per_move.average()
	    << "\n";
}

/* ------------------------------------------------------------------------- */

size_t num_positions = 0;
void test_position(const NumEffectState& state)
{
  MoveVector moves;
  state.generateLegal(moves);
  const PieceMask my_pin = effect_util::Pin::make(state, state.turn());
  const PieceMask op_pin = effect_util::Pin::make(state, alt(state.turn()));
  
  misc::PerfMon clock;
  for (size_t i=0; i<moves.size(); ++i) {
    if (old)
      PieceEval::computeDiffAfterMoveForRP(state, moves[i]);
    else
      See::see(state, moves[i], my_pin, op_pin);
  }
  const size_t consumed = clock.stop();
  cycles.add(consumed);
  cycles_per_move.add(consumed/moves.size());
  ++num_positions;
}

void test_file(const char *filename)
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
  const auto moves=record.moves;

  for (size_t i=0; i<moves.size(); ++i) {
    const Move move = moves[i];
    assert(state.isValidMove(move));
    if (i >= first_skip) {
      test_position(state);
    }
    state.makeMove(move);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
