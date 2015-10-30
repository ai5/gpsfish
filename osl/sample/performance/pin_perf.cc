/* pin_perf.cc
 */
#include "osl/effect_util/pin.h"
#include "osl/csa.h"
#include "osl/record/csaRecord.h"
#include "osl/misc/perfmon.h"

#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::effect_util;

void usage(const char *program_name)
{
  std::cerr << program_name << " csafiles\n";
  exit(1);
}

size_t skip_first = 0;
void run(const char *filename);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "s:vh")) != EOF)
  {
    switch(c)
    {
    case 's':	skip_first = atoi(optarg);
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (argc < 1))
    usage(program_name);

  try
  {
    for (int i=0; i<argc; ++i)
    {
      run(argv[i]);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return 1;
  }
  catch (...)
  {
    throw;
  }
}

void run(const char *filename)
{
  unsigned long long total_cycles=0;
  unsigned long long total_cycles_naive=0;
  unsigned long long total_cycles_step=0;
  unsigned long long total_cycles_step1=0;
  unsigned long long positions = 0;
  CsaFileMinimal csa(filename);
  NumEffectState state(csa.initialState());
  const auto moves=csa.moves();

  size_t i=0;
  while (true)
  {
    if (i >= skip_first)
    {
      misc::PerfMon clock;
      const PieceMask black_pins = Pin::make(state, BLACK);
      const PieceMask white_pins = Pin::make(state, WHITE);
      total_cycles += clock.stop();
      clock.restart();
      const PieceMask black_pins_naive = Pin::makeNaive(state, BLACK);
      const PieceMask white_pins_naive = Pin::makeNaive(state, WHITE);
      total_cycles_naive += clock.stop();
      clock.restart();
      const PieceMask black_pins_step = Pin::makeStep(state, state.kingSquare<BLACK>(),BLACK);
      const PieceMask white_pins_step = Pin::makeStep(state, state.kingSquare<WHITE>(),WHITE);
      total_cycles_step += clock.stop();
      clock.restart();
      const PieceMask black_pins_step1 = Pin::makeStep1(state, state.kingSquare<BLACK>(),BLACK);
      const PieceMask white_pins_step1 = Pin::makeStep1(state, state.kingSquare<WHITE>(),WHITE);
      total_cycles_step1 += clock.stop();
      ++positions;
    }
    if (i >= moves.size())
      break;
    const Move move = moves[i++];
    state.makeMove(move);
  } 
  std::cerr << "p " << total_cycles << " / " << positions << " = " 
	    << total_cycles/(double)positions << "\n";
  std::cerr << "n " << total_cycles_naive << " / " << positions << " = " 
	    << total_cycles_naive/(double)positions << "\n";
  std::cerr << "n " << total_cycles_step << " / " << positions << " = " 
	    << total_cycles_step/(double)positions << "\n";
  std::cerr << "n " << total_cycles_step1 << " / " << positions << " = " 
	    << total_cycles_step1/(double)positions << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
