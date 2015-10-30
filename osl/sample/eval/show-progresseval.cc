/* show-progresseval.cc
 */
#include "osl/eval/progressEval.h"
#include "osl/eval/ppair/piecePairPieceEval.h"
#include "osl/record/csaRecord.h"
#include "osl/oslConfig.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace osl;
using namespace osl::eval;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " csa-filename"
       << endl;
  exit(1);
 }

void show(const char *filename);
int verbose = 0;
int max_progress = 8;

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  // extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "vh")) != EOF)
  {
    switch(c)
    {
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag)
    usage(program_name);

  eval::ProgressEval::setUp();
  eval::PiecePairPieceEval::setUp();
  
  for (int i=0; i<argc; ++i)
  {
    show(argv[i]);
  }
}

void show(const NumEffectState& state)
{
  const progress::Effect5x3 progress(state);
  if (progress.progress16().value() > max_progress)
    return;
  if (verbose)
    std::cout << state;
  const eval::ProgressEval eval(state);
  const PieceEval piece(state);
  const eval::PiecePairPieceEval ppair(state);

  if (verbose)
    std::cout << "progress piece ppair endgame safety pieceadjust total\n";
  std::cout << progress.progress16().value()
	    << " " << piece.value() << " " << ppair.value()
	    << " " << eval.endgameValue()
	    << " " << eval.attackDefenseBonus() << " " << eval.minorPieceValue()
	    << " " << eval.value() << "\n";
}

void show(const char *filename)
{
  CsaFile file(filename);
  const auto moves = file.load().moves();
  NumEffectState state(file.initialState());
  for (unsigned int i=0; i<moves.size(); i++)
  {
    show(state);
    const Move m = moves[i];
    state.makeMove(m);
  }
  show(state);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
