/* quiescencestat.cc
 */
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/searchState2.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/csaRecord.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/misc/perfmon.h"

#include <iostream>
#include <fstream>
#include <cstdio>

using namespace osl;
using namespace osl::search;
using namespace osl::misc;

void qsearch(const char *filename);

void usage(const char *program_name)
{
  std::cerr << program_name << " [-d depth] [-s skip] [-v] [-p] csafiles\n";
  exit(1);
}

int depth = -2;
bool verbose = false, problem_solving = false;
size_t skip_first = 0;

void qsearch(const char *filename);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "d:s:pvh")) != EOF)
  {
    switch(c)
    {
    case 'd':	depth = atoi(optarg);
      break;
    case 's':	skip_first = atoi(optarg);
      break;
    case 'p':	problem_solving = true;
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

  std::cerr << "using table record depth " << depth << "\n";
  eval::ml::OpenMidEndingEval::setUp();
  progress::ml::NewProgress::setUp();
  try
  {
    for (int i=0; i<argc; ++i)
    {
      qsearch(argv[i]);
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

void qsearch(const char *filename)
{
  if (verbose)
    std::cerr << filename;
  unsigned long long total_cycles=0;
  unsigned long long positions = 0;
  Record record=CsaFile(filename).load();
  NumEffectState state(record.initialState());
  const auto moves=record.moves();

  typedef QuiescenceSearch2<eval::ml::OpenMidEndingEval> qsearch_t;

  SimpleHashTable table(1000000,depth,verbose);
  SearchState2Core::checkmate_t  checkmate_searcher;
  eval::ml::OpenMidEndingEval ev(state);
  size_t i=0;
  Player initial_turn = state.turn();
  while (true)
  {
    if (i >= skip_first)
    {
      SearchState2Core core(state, checkmate_searcher);
      qsearch_t qs(core, table);
      const Move last_move = (i > 0) ? moves[i-1] : Move::PASS(alt(initial_turn));
      if (verbose)
	std::cerr << i << " " << last_move << "\n";
      if (problem_solving)
      {
	std::cout << state;
	table.allocate(HashKey(state), 1000);
      }
      PerfMon clock;
      const int val = qs.search(state.turn(), ev, last_move, 4);
      total_cycles += clock.stop();
      positions += qs.nodeCount();

      if (verbose || problem_solving)
      {
	const SimpleHashRecord *record = table.find(HashKey(state));
	std::cout << "result ";
	if (record)
	  std::cout << csa::show(record->qrecord.bestMove()) << " ";
	std::cout << val << "\n";
	if (i < moves.size())
	  std::cout << "recorded " << csa::show(moves[i]) << "\n";
      }
    }
    if (i >= moves.size() || problem_solving)
      break;
    const Move move = moves[i++];
    state.makeMove(move);
    ev.update(state, move);
  } 
  const size_t checkmate_count = checkmate_searcher.totalNodeCount();
  std::cerr << total_cycles << " / ( " << positions 
	    << " + " << checkmate_count << " ) = " 
	    << total_cycles/(double)(positions + checkmate_count) << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
