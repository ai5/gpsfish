#include "dualCheckmateSearcher.h"
#include "dualCheckmateSearcher.tcc"
#include "checkmateSearcher.tcc"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "oracleProver.tcc"
#include "oracleDisprover.tcc"
#include "analyzer/checkTableAnalyzer.h"
#include "analyzer/proofTreeDepth.h"
#include "simpleCheckHashTable.h"
#include "dominanceTable.h"
#include "checkHashRecord.h"
#include "checkmateRecorder.h"
#include "osl/checkmate/libertyEstimator.h"
#include "nullEstimator.h"
#include "pieceCost2.h"
#include "nullCost.h"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"
#include "osl/state/numEffectState.h"
#include "osl/misc/perfmon.h"
#include "osl/misc/milliSeconds.h"

#include <boost/scoped_ptr.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

using namespace osl;
using namespace osl::checkmate;
using namespace osl::misc;
using osl::checkmate::analyzer::ProofTreeDepth;
void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-vpPEOt] [-T tree-size-out] [-F FILES] [-l nodelimit] [-L loglimit] [-o tree-output-filename] [-d logdepth] [-A depth(>0)] [-a depth(>0)] csa-filenames "
       << endl
       << "-v show result\n"
       << "-p show proof tree\n"
       << "-P (plain) don't use heuristics\n"
       << "-A depth  show all tree (expanding solved branch)\n"
       << "-a depth  show tree\n"
       << "-E show filnames that are solved and found to be 'escape'\n"
       << "-t force turn's player to attack even if he is in check\n"
       << "-O use outline format for dumping tree\n"
       << endl;
  exit(1);
}

bool verbose=false;
unsigned long long total_cycles=0;
size_t limit = 409600; // 3900;
bool showProofTree = false;
int showAllTreeDepth = 0;
int showTreeDepth = 0;
bool showEscapeFilename = false;
bool force_attack = false;
bool useDominanceRelation = false;
int num_checkmate=0, num_escape=0, num_unkown=0;
double total_nodes=0, total_tables=0;
bool useOutlineFormat = false;
size_t logThreshold = 0;
bool useHeuristics = true;

std::ostream *treeOut = 0;
std::ostream *tree_size_out = 0;

template <class Table, class H, class Cost>
void search(const char *filename);
void searchFile(const char *filename);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;

  const char *FILES = 0;
  treeOut = &std::cout;
  std::unique_ptr<std::ofstream> treefs, treesizefs;
    
  char c;
  while ((c = getopt(argc, argv, "A:a:Dd:F:EL:l:Oo:T:Pptvh")) != EOF)
  {
    switch(c)
    {
    case 'A':	showAllTreeDepth = atoi(optarg);
      assert(showAllTreeDepth);
      break;
    case 'a':	showTreeDepth = atoi(optarg);
      assert(showTreeDepth);
      break;
    case 'D':	useDominanceRelation = true;
      break;
    case 'd':	checkmate::CheckmateRecorder::DepthTracer::maxVerboseLogDepth 
							     = atoi(optarg);
      break;
    case 'E':	showEscapeFilename = true;
      break;
    case 'F':	FILES = optarg;
      break;
    case 'L':	logThreshold = atoi(optarg);
      assert(logThreshold);
      break;
    case 'l':	limit = atoi(optarg);
      assert(limit);
      break;
    case 'O':	useOutlineFormat = true;
      break;
    case 'o':	treefs.reset(new std::ofstream(optarg));
      treeOut = &*treefs;
      break;
    case 'P':	useHeuristics = false;
      break;
    case 'p':	showProofTree = true;
      break;
    case 't':	force_attack = true;
      break;
    case 'T':	treesizefs.reset(new std::ofstream(optarg));
      tree_size_out = &*treesizefs;
      break;
    case 'v':	verbose = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || ((argc < 1) && (! FILES)))
    usage(program_name);

  std::cerr << "limit " << limit << "\n";
  try
  {
    for (int i=0; i<argc; ++i)
    {
      searchFile(argv[i]);
      total_cycles = 0;
    }
    if (FILES)
    {
      std::ifstream is(FILES);
      std::string filename;
      size_t count=0;
      while (is >> filename)
      {
	if (tree_size_out && (++count % 512) == 0)
	  *tree_size_out << std::flush;
	searchFile(filename.c_str());
	total_cycles = 0;
      }
    }
    std::cerr << "check " << num_checkmate << " escape " << num_escape
	      << " unknown " << num_unkown << "\n";
    std::cerr << "total nodes " << total_nodes 
	      << " tables " << total_tables << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return 1;
  }
}

void dumpTree(const CheckTableAnalyzer& analyzer,
	      const CheckHashRecord *record)
{
  if (! record)
    return;
  Move debug_move;		// place the first move for debug here
  for (CheckMoveList::const_iterator p=record->moves.begin(); p!=record->moves.end();
       ++p)
  {
    if (p->move == debug_move)
    {
      if (p->record)
      {
	p->record->dump();
	record = p->record;
      }
      break;
    } 
  }
  
  if (showAllTreeDepth)
  {
    analyzer.showTree(record,*treeOut,showAllTreeDepth,true,false,logThreshold);
  }
  if (showTreeDepth)
  {
    analyzer.showTree(record,*treeOut,showTreeDepth,false,false,logThreshold);
  }
}

void searchFile(const char *filename)
{
  if (useDominanceRelation)
  {
    if (useHeuristics)
      search<DominanceTable,LibertyEstimator,PieceCost2>(filename);
    else
      search<DominanceTable,PureLibertyEstimator,NullCost>(filename);
  }
  else
  {
    if (useHeuristics)
      search<SimpleCheckHashTable,LibertyEstimator,PieceCost2>(filename);
    else
      search<SimpleCheckHashTable,PureLibertyEstimator,NullCost>(filename);
  }
}

/**
 * CSA 形式のファイルから stream から1局面(だけを)読む
 */
void readCsaState(std::istream& is, SimpleState& state)
{
  Record rec;
  record::csa::InputStream irs(is);
  irs.load(&rec);
  state = rec.getInitialState();
}

double real_seconds = 0.0;

template <Player P, class Searcher>
void testWinOrLose(const char *curFilename,
		   Searcher& searcher,
		   const SimpleState& sstate, int limit)
{
  typedef typename Searcher::table_t table_t;
  NumEffectState state(sstate);
  const PathEncoding path(state.turn());
  const Square my_king = state.template kingSquare<P>();
  if ((! force_attack) 
      && ! my_king.isPieceStand() && state.hasEffectAt(alt(P), my_king))
  {
    // 相手から王手がかかっている
    time_point timer = clock::now();
    misc::PerfMon clock;
    const bool lose
      = searcher.template isLosingState<P>(limit, state, HashKey(state), path);
    total_cycles += clock.stop();
    real_seconds = timer.elapsedSeconds();

    const HashKey key(state);
    const table_t& table = searcher.getTable(alt(P));
    const CheckHashRecord *record = table.find(key);
    if (lose) {
      ++num_checkmate;
    }
    else {
      if (record->proofDisproof().isCheckmateFail()
	  || record->findLoop(path, table.getTwinTable()))
	++num_escape;
      else {
	assert(! record->proofDisproof().isFinal());
	++num_unkown;
      }
    }
      
    if (! verbose)
      return;

    CheckTableAnalyzer analyzer(table.getTwinTable(), useOutlineFormat);
    dumpTree(analyzer, record);
    if (lose)
    {
      size_t leaf_size;
      const size_t tree_size 
	= analyzer.proofTreeSize(record,key,path,false,leaf_size);
      ProofTreeDepth depth_analyzer;
      const unsigned int depth = depth_analyzer.depth(record, false);
      std::cerr << "lose\n";
      std::cerr << "proof tree " << tree_size 
		<< " leaf " << leaf_size
		<< " depth " << depth << "\n";
      if (tree_size_out)
      {
	*tree_size_out << tree_size << " " << table.size() << "\n";
      }
      if (showProofTree)
	analyzer.showProofTree(record,key,path,false,*treeOut);
      return;
    }
    else
    {
      assert(record);
      if (showEscapeFilename)
	std::cerr << curFilename << "\n";
      if (record->proofDisproof().isCheckmateFail()
	  || record->findLoop(path, table.getTwinTable()))
      {
	size_t leaf_size;
	const size_t tree_size 
	  = analyzer.disproofTreeSize(record,key,path,true, leaf_size);
	std::cerr << "escape\n";
	std::cerr << "disproof tree " << tree_size 
		  << " leaf " << leaf_size << "\n";
	if (tree_size_out)
	  *tree_size_out << tree_size << " " << table.size() << "\n";

	if (showProofTree)
	  analyzer.showProofTree(record,key,path,true,*treeOut);
	return;
      }
      else
      {
	assert(! record->proofDisproof().isFinal());
	std::cerr << "unknown " << record->proofDisproof() << "\n";
	return;
      }
    }
  }
  else
  {
    Move checkmateMove;
    time_point timer=clock::now();
    PerfMon clock;
    AttackOracleAges age;
    const bool win = searcher.
      template isWinningState<P>(limit, state, HashKey(state), path, checkmateMove, age);
    total_cycles += clock.stop();
    real_seconds = timer.elapsedSeconds();
#if 0
    std::cerr << " done\n";
#endif
    const HashKey key(state);
    const table_t& table = searcher.getTable(P);
    const CheckHashRecord *record = table.find(HashKey(state));
    if (win) {
      ++num_checkmate;
    }
    else {
      assert(record);
      if (record->proofDisproof().isFinal()
	  || record->findLoop(path, table.getTwinTable()))
	++num_escape;
      else
	++num_unkown;
    }
    if (! verbose)
      return;

    CheckTableAnalyzer analyzer(table.getTwinTable(), useOutlineFormat);
    if ((record->proofDisproof() != ProofDisproof::Checkmate())
	&& (record->proofDisproof() != ProofDisproof::NoCheckmate()))
      std::cerr << record->proofDisproof() << "\n";
    // std::cerr << record->bestResultInSolved << "\n";
    dumpTree(analyzer, record);
    if (win)
    {
      size_t leaf_size;
      const size_t tree_size = analyzer.proofTreeSize(record,key,path,true,
						      leaf_size);
      ProofTreeDepth depth_analyzer;
      const unsigned int depth = depth_analyzer.depth(record, true);
      std::cerr << "win by " << checkmateMove << "\n";
      std::cerr << "proof tree " << tree_size 
		<< " leaf " << leaf_size 
		<< " depth " << depth << "\n";
      if (tree_size_out)
	*tree_size_out << tree_size << " " << table.size() << "\n";
      if (showProofTree)
	analyzer.showProofTree(record,key,path,true,*treeOut);
      return;
    }
    else
    {
      assert(record);
      if (record->proofDisproof().isFinal()
	  || record->findLoop(path, table.getTwinTable()))
      {
	size_t leaf_size;
	const size_t tree_size 
	  = analyzer.disproofTreeSize(record,key,path,false, leaf_size);
	std::cerr << "no checkmate\n";
	std::cerr << "disproof tree " << tree_size 
		  << " leaf " << leaf_size << "\n";
	if (tree_size_out)
	  *tree_size_out << tree_size << " " << table.size() << "\n";
	if (showProofTree)
	  analyzer.showProofTree(record,key,path,false,*treeOut);
	return;
      }
      else
      {
	std::cerr << "unknown " << record->proofDisproof() << "\n";
	return;
      }
    }
  }
}

template <class Table, class H, class Cost>
void search(const char *filename)
{
  std::ifstream is(filename);
  if (! is) {
    std::cerr << "\nskipping " << filename << "\n";
    return;
  }
  if (verbose)
    std::cerr << "\nsolving " << filename << "\n";
  
  typedef DualCheckmateSearcher<Table, H, Cost> searcher_t;
  searcher_t searcher(std::max(limit, (size_t)20000000));
  searcher.setVerbose(verbose);

  SimpleState state;
  readCsaState(is, state);

  if (state.turn() == BLACK)
    testWinOrLose<BLACK,searcher_t>(filename, searcher, state, limit);
  else
    testWinOrLose<WHITE,searcher_t>(filename, searcher, state, limit);

  const size_t table_used = searcher.getTable(BLACK).size()
    + searcher.getTable(WHITE).size();

  total_nodes += searcher.totalNodeCount();
  total_tables += table_used;

  if (verbose) {
    PerfMon::message(total_cycles, "total ", 
		     searcher.totalNodeCount());
    PerfMon::message(total_cycles, "unique", table_used);
    std::cerr << "real " << real_seconds << " sec.\n";
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
