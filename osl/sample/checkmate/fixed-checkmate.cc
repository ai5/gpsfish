#include "osl/checkmate/fixedDepthSearcher.h"

#include "osl/csa.h"
#include "osl/misc/perfmon.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace osl;
using namespace osl::checkmate;
using namespace osl::misc;
void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-d depth] csa-filenames "
       << endl
       << endl;
  exit(1);
}

bool verbose=false;
int num_checkmate=0, num_escape=0, num_unkown=0;

void search(int depth, const char *filename);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  int depth=2;
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  
  char c;
  while ((c = getopt(argc, argv, "d:vh")) != EOF)
  {
    switch(c)
    {
    case 'd':	depth = atoi(optarg);
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

  std::cerr << "depth " << depth << "\n";
  try
  {
    for (int i=0; i<argc; ++i)
    {
      search(depth, argv[i]);
    }
    std::cerr << "check " << num_checkmate << " escape " << num_escape
	      << " unknown " << num_unkown << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return 1;
  }
}

void search(int depth, const char *filename)
{
  const RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());

  FixedDepthSearcher searcher(state);

  Move best_move;
  PerfMon clock;
  const ProofDisproof pdp = searcher.hasCheckmateMoveOfTurn(depth, best_move);
  const unsigned long long total_cycles = clock.stop();
  const int count = searcher.getCount();

  if (pdp.isCheckmateSuccess())
  {
    ++num_checkmate;
    std::cerr << "win by " << best_move << "\n";
  }
  else if (pdp.isCheckmateFail())
  {
    ++num_escape;
    std::cerr << "no checkmate\n";
  }
  else
  {
    ++num_unkown;
    std::cerr << "unknown " << pdp << "\n";
  }

  PerfMon::message(total_cycles, "total ", count);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
