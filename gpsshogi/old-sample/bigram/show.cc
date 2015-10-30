/* show.cc
 */
#include "osl/category/bigramSquare.h"
#include "osl/move_generator/legalMoves.h"
#include "osl/container/moveVector.h"
#include "osl/record/csaRecord.h"
#include "osl/apply_move/applyMove.h"
#include "osl/oslConfig.h"
#include <boost/scoped_ptr.hpp>
#include <iostream>
using namespace osl;
using namespace osl::category;

std::unique_ptr<KingNeighborBigramTable> table;
std::unique_ptr<BigramHelper> helper;

void testFile(const char *file_name);

void usage(const char *prog)
{
  using namespace std;
  std::cerr << "Usage: " << prog << " csa_files\n"
	    << endl;
  exit(1);
}

int verbose = 1;

int main(int argc, char **argv)
{
  table.reset(new KingNeighborBigramTable(OslConfig::home_c_str(), "attack"));
  helper.reset(new BigramHelper);

  const char *program_name = argv[0];
  bool error_flag = false;
  // extern char *optarg;
  extern int optind;
    
  char c;
  while ((c = getopt(argc, argv, "vh")) != EOF) {
    switch(c) {
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (argc < 1))
    usage(program_name);

  for (int i=0; i<argc; ++i) {
    testFile(argv[i]);
  }
}

void testFile(const char *filename)
{
  const Record record = CsaFile(filename).getRecord();
  NumEffectState state(record.getInitialState());
  const vector<osl::Move> moves=record.getMoves();

  MoveStack history;
  CategoryEnv env(&state, 1000, &history);

  for (size_t i=0; i<moves.size(); ++i) {
    const Move move = moves[i];
    assert(state.isValidMove(move));

    ApplyMoveOfTurn::doMove(state, move);
    history.push(move);

    MoveLogProbVector out;
    table->generate(env, out, 2);
    int diff = 0;
    for (size_t i=0; i<out.size(); ++i) {
      const int prob = table->probability(env, out[i].getMove(), 2);
      if (out[i].getLogProb() != prob)
	++diff;
    }
    if (verbose > 1 || diff && verbose) {
      std::cerr << state;
      for (size_t i=0; i<out.size(); ++i) {
	const int prob = table->probability(env, out[i].getMove(), 2);
	if (verbose > 1 || out[i].getLogProb() != prob)
	  std::cerr << out[i].getMove() << " " << out[i].getLogProb() 
		    << " " << prob << "\n";
      }
#ifdef DEBUG
      MoveVector sibling;
      LegalMoves::generate(state, sibling);
      if (sibling.size()) {
	for (size_t j=0; j<sibling.size(); ++j) {	
	  const int prob = table->probability(env, sibling[j], 2);
	  if (prob < env.limit)
	    std::cerr << sibling[j] << " " << prob << "\n";
	}
      }
#endif
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
