#include "osl/checkmate/dfpn.h"
#include "osl/checkmate/dfpnParallel.h"
#include "osl/csa.h"
#include "osl/misc/perfmon.h"
#include "osl/misc/milliSeconds.h"

#include "osl/checkmate/dfpnRecord.h"
#include "osl/hash/hashRandomPair.h"
#include "osl/oslConfig.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

#include <bitset>

using namespace osl;
using namespace osl::checkmate;
using namespace osl::misc;

unsigned int dovetailing_seed = 0;
unsigned int dovetailing_prob = 0;

bool verbose=false;
unsigned long long total_cycles=0;
bool show_escape_filename = false;
bool force_attack = false;
int num_checkmate=0, num_nocheckmate=0, num_escape=0, num_unkown=0;
double total_nodes=0, total_tables=0;
int limit = 100000;
bool blocking_verify = true;
size_t table_growth_limit = 8000000;
bool debug = false;
int forward_moves = 0;

template<class DfpnSearch>
void search(DfpnSearch&, const char *filename);
void usage(const char *program_name)
{
  std::cerr << "usage: " << program_name << " [-d] [-v] [-f] [-l limit] [-N] csa-files\n";
}
int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  int parallel = 0;
  extern char *optarg;
  extern int optind;

  char c;
  while ((c = getopt(argc, argv, "dfl:N:F:s:p:t:vh")) != EOF)
  {
    switch(c)
    {
    case 's':   dovetailing_seed = static_cast<unsigned int>(atoi(optarg));
      break;
    case 'p':   dovetailing_prob = static_cast<unsigned int>(atoi(optarg));
      break;
    case 'd':	debug = true;
      break;
    case 'f':	force_attack = true;
      break;
    case 'F':	forward_moves = atoi(optarg);
      break;
    case 'l':	limit = atoi(optarg);
      break;
    case 'N':	parallel = atoi(optarg);
      break;
    case 't':	table_growth_limit = atoi(optarg);
      break;
#if 0
    case 'V':	blocking_verify = false;
      break;
#endif
    case 'v':	verbose = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (argc < 1)) {
    usage(program_name);
    return 1;
  }
  osl::OslConfig::setUp();
  OslConfig::setDfpnMaxDepth(1600);	// sufficient for microcosmos
  if (dovetailing_prob > 0)
    HashRandomPair::setUp(dovetailing_seed, dovetailing_prob);
  try
  {
    for (int i=0; i<argc; ++i)
    {
      if (parallel) 
      {
#ifdef OSL_DFPN_SMP
	DfpnParallel dfpn(parallel);
	search(dfpn, argv[i]);
#else
	std::cerr << "to use parallel dfpn, try compile with -DOSL_SMP or -DOSL_DFPN_SMP\n";
	return 1;
#endif
      }
      else 
      {
	Dfpn dfpn;
	search(dfpn, argv[i]);
      }
      total_cycles = 0;
    }
    std::cerr << "check " << num_checkmate << " nocheckmate " << num_nocheckmate << " escape " << num_escape
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

double real_seconds = 0.0;

template <class DfpnSearch>
void analyzeCheckmate(DfpnSearch& searcher, const NumEffectState& state, Move checkmate_move)
{
  NumEffectState new_state = state;
  std::cerr << state << " " << checkmate_move << "\n";
  new_state.makeMove(checkmate_move);
  HashKey key(new_state);
  const DfpnTable& table = searcher.currentTable();
  DfpnRecordBase record = table.probe(key, PieceStand(WHITE, new_state));
  std::cerr << record.proof_disproof << " " << std::bitset<64>(record.solved) << "\n";

  MoveVector moves;
  new_state.generateLegal(moves);
  for (size_t i=0; i<moves.size(); ++i) {
    NumEffectState tmp = new_state;
    tmp.makeMove(moves[i]);
    DfpnRecordBase record = table.probe(key.newHashWithMove(moves[i]), PieceStand(WHITE, tmp));
    std::cerr << moves[i] << " " << record.proof_disproof << " " << record.best_move << "\n";
  }

  {
    Dfpn::DfpnMoveVector moves;
    if (state.turn() == BLACK)
      Dfpn::generateEscape<BLACK>(new_state, false, Square(), moves);
    else
      Dfpn::generateEscape<WHITE>(new_state, false, Square(), moves);
    std::cerr << "Escape " << moves.size()<< "\n";
    moves.clear();
    if (state.turn() == BLACK)
      Dfpn::generateEscape<BLACK>(new_state, true, Square(), moves);
    else
      Dfpn::generateEscape<BLACK>(new_state, true, Square(), moves);
    std::cerr << "Escape full " << moves.size() << "\n";
  }
}

template <class DfpnSearch>
void testWinOrLose(const char *filename,
		   DfpnSearch& searcher,
		   const SimpleState& sstate, int limit,
		   ProofDisproof& result, Move& best_move,
		   const std::vector<Move>& moves)
{
  const Player P = sstate.turn();
  NumEffectState state(sstate);
  const PathEncoding path(state.turn());
  const Square my_king = state.kingSquare(P);
  if ((! force_attack) 
      && ! my_king.isPieceStand() && state.inCheck(P))
  {
    // 相手から王手がかかっている
    time_point timer = clock::now();
    misc::PerfMon clock;
    result = searcher.hasEscapeMove(state, HashKey(state), path, limit, Move::PASS(alt(P)));
    total_cycles += clock.stop();
    real_seconds = elapsedSeconds(timer);

    if (verbose)
      std::cerr << result << "\n";
    if (result.isCheckmateSuccess()) {
      ++num_checkmate;
    }
    else {
      if (result.isCheckmateFail())
	++num_escape;
      else {
	assert(! result.isFinal());
	++num_unkown;
      }
    }
    return;
  }

  Move checkmate_move;
  std::vector<Move> pv;
  time_point timer = clock::now();
  PerfMon clock;
  result = searcher.
    hasCheckmateMove(state, HashKey(state), path, limit, checkmate_move, Move(), &pv);
  total_cycles += clock.stop();
  real_seconds = elapsedSeconds(timer);
  if (verbose)
    std::cerr << result << "\n";

  if (result.isCheckmateSuccess()) {
    ++num_checkmate;
    best_move = checkmate_move;
    if (verbose) {
      std::cerr << checkmate_move << "\n";
      for (size_t i=0; i<pv.size(); ++i) {
	std::cerr << std::setw(4) << std::setfill(' ') << i+1
		  << ' ' << csa::show(pv[i]) << " ";
	if (i % 6 == 5) 
	  std::cerr << "\n";
      }
      if (pv.size() % 6 != 0)
	  std::cerr << "\n";
    }
    if (debug) {
      // analyzeCheckmate(searcher, state, checkmate_move);
      if (! moves.empty())
	searcher.analyze(path, state, moves);
    }
  }
  else {
    if (result.isFinal())
      ++num_nocheckmate;
    else
      ++num_unkown;
    if (debug)
      searcher.analyze(path, state, moves);
  }
}

template <class DfpnSearch>
void search(DfpnSearch& searcher, const char *filename)
{
  NumEffectState state;
  std::vector<Move> moves;
  try {
    CsaFileMinimal file(filename);
    state = file.initialState();
    moves = file.moves();
    int forward_here = std::min(forward_moves, (int)moves.size());
    for (int i=0; i<forward_here; ++i)
      state.makeMove(moves[i]);
    moves.erase(moves.begin(), moves.begin()+forward_here);
  }
  catch (CsaIOError&) {
    std::cerr << "\nskipping " << filename << "\n";
    return;
  }
  if (verbose)
    std::cerr << "\nsolving " << filename << "\n";
  
  // searcher.setBlockingVerify(blocking_verify);
  const bool attack = force_attack
    || state.kingSquare(state.turn()).isPieceStand()
    || ! state.inCheck(state.turn());
  DfpnTable table(attack ? state.turn() : alt(state.turn()));
  table.setGrowthLimit(table_growth_limit);
  searcher.setTable(&table);
  ProofDisproof result;
  Move best_move;
  testWinOrLose(filename, searcher, state, limit, result, best_move, moves);
  const size_t table_used = searcher.currentTable().size();
  total_nodes += searcher.nodeCount();
  total_tables += table_used;

  if (verbose) {
    PerfMon::message(total_cycles, "total ", 
		     searcher.nodeCount());
    PerfMon::message(total_cycles, "unique", table_used);
    std::cerr << "real " << real_seconds << " sec. nps " << searcher.nodeCount()/real_seconds << "\n";
  }
  std::cout << filename << "\t" << searcher.nodeCount() 
	    << "\t" << table_used << "\t" << real_seconds
	    << " " << result;
  if (best_move.isNormal())
    std::cout << " " << csa::show(best_move);
  std::cout << "\n" << std::flush;
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
