/* show-eval.cc
 */
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress.h"
#include "osl/record/csaRecord.h"
#include "osl/container/pieceValues.h"
#include "osl/oslConfig.h"
#include "osl/record/kanjiPrint.h"
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
int piece_estimate_level = 2;

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "e:vh")) != EOF)
  {
    switch(c)
    {
    case 'e':
      if (atoi(optarg) > 0)
	piece_estimate_level = atoi(optarg);
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag)
    usage(program_name);

  eval::ml::OpenMidEndingEval::setUp();
  progress::ml::NewProgress::setUp();
  
  for (int i=0; i<argc; ++i)
  {
    show(argv[i]);
  }
}

void make1(const NumEffectState& state, const eval::ml::OpenMidEndingEval& eval, PieceValues& values)
{
  for (int i=0; i<Piece::SIZE; ++i) {
    const Piece piece = state.pieceOf(i);
    if (! piece.isOnBoard() || unpromote(piece.ptype()) == KING)
      continue;
    const NumEffectState removed(state.emulateCapture(piece, piece.owner()));
    const eval::ml::OpenMidEndingEval eval_removed(removed);
    values[piece.number()] = eval.value() - eval_removed.value();
  }
}

void make2(const NumEffectState& state, const eval::ml::OpenMidEndingEval& eval, PieceValues& values)
{
  CArray<int,40> count = {{ 0 }};
  for (int i=0; i<Piece::SIZE; ++i) {
    values[i] = 0;
    const Piece piece = state.pieceOf(i);
    if (! piece.isOnBoard() || unpromote(piece.ptype()) == KING)
      continue;
    const NumEffectState removed(state.emulateCapture(piece, piece.owner()));
    const eval::ml::OpenMidEndingEval eval_removed(removed);
    for (int j=0; j<Piece::SIZE; ++j) {
      const Piece piece2 = state.pieceOf(j);
      if (! piece2.isOnBoard() || unpromote(piece2.ptype()) == KING || i == j)
	continue;
      const NumEffectState removed2(removed.emulateCapture(piece2, piece2.owner()));
      const eval::ml::OpenMidEndingEval eval_removed2(removed2);
      values[j] += eval_removed.value() - eval_removed2.value();
      count[j]++;
    }
  }
  for (int i=0; i<Piece::SIZE; ++i)
    if (count[i])
      values[i] /= count[i];
}

void show(const NumEffectState& state)
{
  static const std::shared_ptr<osl::record::KIFCharacters> characters(new osl::record::KIFCharacters());
  static osl::record::KanjiPrint printer(std::cout, characters);
  static const double scale = 200.0 
    / eval::ml::OpenMidEndingEval::captureValue(newPtypeO(WHITE,PAWN));
  const eval::ml::OpenMidEndingEval eval(state);
  PieceValues values;
  if (piece_estimate_level == 1)
    make1(state, eval, values);
  else
    make2(state, eval, values);
  printer.print(state);
  for (int z=0; z<2; ++z) {
    for (size_t t=0; t<PieceStand::order.size(); ++t) {
      const Ptype ptype = PieceStand::order[t];
      bool shown = false;
      for (int i=Ptype_Table.getIndexMin(ptype); i<Ptype_Table.getIndexLimit(ptype); ++i) {
	const Piece piece = state.pieceOf(i);
	if (! piece.isOnBoard() || unpromote(piece.ptype()) != ptype
	    || piece.owner() != indexToPlayer(z))
	  continue;
	if (! shown)
	  std::cout << Ptype_Table.getCsaName(ptype);
	shown = true;
	std::cout << "  (" << piece.square().x() << "," << piece.square().y()
		  << ") " << (int)(values[piece.number()]*scale);
      }
      if (shown)
	std::cout << "\n";
    }
  }
  std::cout << "total " << (int)(eval.value()*scale)
	    << " " << eval.value() 
	    << " (progress: " << eval.progress16().value()
	    << " " << progress::ml::NewProgress(state).progress()
	    << ", open-mid-mid2-endgame: " << eval.openingValue()
	    << " " << eval.midgameValue()
	    << " " << eval.midgame2Value()
	    << " " << eval.endgameValue()
	    << " )\n";
}

void show(const char *filename)
{
  CsaFile file(filename);
  const auto moves = file.load().moves();
  NumEffectState state(file.load().initialState());
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
