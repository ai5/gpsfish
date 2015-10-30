/* quiescencestat.cc
 */
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/quiescenceLog.h"
#include "osl/search/simpleHashTable.h"
#include "osl/numEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/record/csaRecord.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/progressEval.h"
#include "osl/misc/perfmon.h"

#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::search;
using namespace osl::misc;

void qsearch(const char *filename);

void usage(const char *program_name)
{
  std::cerr << program_name << " [-C] [-P] [-d depth] [-s skip] [-v] csafiles\n";
  std::cerr << "-C comparison w,w/o table\n";
  exit(1);
}

int depth = -6;
bool verbose = false;
size_t skip_first = 0;
bool comparison = false;
bool use_progress_eval = false;

template <class Eval>
void qsearch(const char *filename);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "C:Pd:s:vh")) != EOF)
  {
    switch(c)
    {
    case 'C':   comparison = true;
      break;
    case 'd':	depth = atoi(optarg);
      break;
    case 'P':	use_progress_eval = true;
      break;
    case 's':	skip_first = atoi(optarg);
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
  try
  {
    for (int i=0; i<argc; ++i)
    {
      if (use_progress_eval)
	qsearch<eval::ProgressEval>(argv[i]);
      else
	qsearch<PieceEval>(argv[i]);
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

template <class Eval>
void qsearch(const char *filename)
{
  unsigned long long total_cycles=0;
  unsigned long long positions = 0;
  Record record=CsaFile(filename).load();
  NumEffectState state(record.initialState());
  const auto moves=record.moves();

  typedef QuiescenceSearch2<Eval> qsearch_t;

  SimpleHashTable table(1000000,depth,verbose);
  SimpleHashTable nulltable(0,0,false);
  SearchState2Core::checkmate_t  checkmate_searcher;
  Eval ev(state);
  size_t i=0;
  while (true)
  {
    if (i >= skip_first)
    {
      SearchState2Core core(state, checkmate_searcher);
      qsearch_t qs(core, table);
      qsearch_t qsnull(core, nulltable);
      const Move last_move = (i > 0) ? moves[i-1] : Move::PASS(alt(moves[0].player()));
      if (verbose)
	std::cerr << i << " " << last_move << "\n";
      if (verbose)
      {
	const char *logfile = "/tmp/q-w-table.log";
	unlink(logfile);
	QuiescenceLog::init(logfile);
      }
      PerfMon clock;
      const int val = qs.search(state.turn(), ev, last_move);
      total_cycles += clock.stop();
      if (comparison)
      {
	if (verbose)
	{
	  const char *logfile = "/tmp/q-wo-table.log";
	  unlink(logfile);
	  QuiescenceLog::init(logfile);
	}
	const int valnull = qsnull.search(state.turn(), ev, last_move);
	if (verbose || (valnull != val))
	{
	  std::cerr << state << "\n";
	  std::cerr << ev.value() << " " ;
	  if (! state.inCheck())
	    std::cerr  << ((state.turn() == BLACK)
			   ? qs.template staticValueWithThreat<BLACK>(ev) 
			   : qs.template staticValueWithThreat<WHITE>(ev)) << " ";
	  std::cerr << val << " " << valnull << "\n";
	  // break;
	}
      }
      positions += qs.nodeCount();
    }
    if (i >= moves.size())
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
