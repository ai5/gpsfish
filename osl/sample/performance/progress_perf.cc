/* progress_perf.cc
 */
#include "osl/progress/effect5x3.h"
#include "osl/progress/effect5x3d.h"
#include "osl/csa.h"
#include "osl/record/csaRecord.h"
#include "osl/misc/perfmon.h"

#include <iostream>
#include <fstream>

using namespace osl;

void usage(const char *program_name)
{
  std::cerr << program_name << " csafiles\n";
  exit(1);
}

size_t skip_first = 0;
void run(const char *filename);
void finish();

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
    finish();
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

unsigned long long total_cycles=0;
unsigned long long total_cycles_naive=0;
unsigned long long positions = 0;
void run(const char *filename)
{
  CsaFile csa(filename);
  NumEffectState state(csa.initialState());
  const auto moves=csa.moves();

  size_t i=0;
  progress::Effect5x3 progress(state);
  while (true)
  {
    if (i >= moves.size())
      break;
    const Move move = moves[i++];
    state.makeMove(move);
    if (i >= skip_first) {
      misc::PerfMon clock;
      progress.update(state, move);
      total_cycles += clock.stop();
      ++positions;
    }
  } 
}

void finish()
{
  std::cerr << "p " << total_cycles << " / " << positions << " = " 
	    << total_cycles/(double)positions << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
