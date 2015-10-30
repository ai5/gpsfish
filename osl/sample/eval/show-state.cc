/* show-state.cc
 */
#include "osl/eval/ppair/piecePairPieceEval.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/endgame/attackDefense.h"
#include "osl/eval/endgame/attackKing.h"
#include "osl/eval/endgame/defenseKing.h"
#include "osl/eval/endgame/kingPieceValues.h"
#include "osl/eval/progressEval.h"
#include "osl/container/pieceValues.h"
#include "osl/record/csaRecord.h"
#include "osl/oslConfig.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::endgame;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-a] [-t raw|piece|attack|defense|endgame|progress] [-f pair-file-name] csa-filename"
       << endl;
  exit(1);
 }

void show(const char *filename, const std::string&);

bool show_all_states = false;
int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  std::string table_filename
    = OslConfig::home() + "/data/sibling-attack.pair";
  std::string eval_type = "piece";
  
  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "at:f:vh")) != EOF)
  {
    switch(c)
    {
    case 'a':	show_all_states = true;
      break;
    case 'f':	table_filename = optarg;
      break;
    case 't':	eval_type = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag)
    usage(program_name);

  PiecePairPieceTable::Table.setUp(table_filename.c_str());

  for (int i=0; i<argc; ++i)
  {
    show(argv[i], eval_type);
  }
}

void show(const NumEffectState& state, const std::string& eval_type)
{
  std::cout << state;
  const progress::Effect5x3 progress((NumEffectState(state)));
  std::cout << "black " << progress.progress16(BLACK).value()
	    << " white " << progress.progress16(WHITE).value() 
	    << " average " << progress.progress16().value() 
	    << "\n";
  PieceValues values;
  if (eval_type == "progress")
    ProgressEval::setValues(state, values);
  else if (eval_type == "endgame")
    AttackDefense::setValues(state, values);
  else if (eval_type == "attack")
    KingPieceValues<AttackKing>::setValues(state, values);
  else if (eval_type == "defense")
    KingPieceValues<DefenseKing>::setValues(state, values);
  else if (eval_type == "piece")
    PiecePairPieceEval::setValues(state, values);
  else if (eval_type == "raw")
    PiecePairRawEval::setValues(state, values);
  else
    throw std::runtime_error("unknown function type "+eval_type);
  values.showValues(std::cout, state);
}

void show(const char *filename, const std::string& eval_type)
{
  CsaFile file(filename);
  const auto moves = file.load().moves();
  NumEffectState state(file.initialState());
  for (unsigned int i=0; i<moves.size(); i++)
  {
    if (show_all_states)
      show(state, eval_type);
    const Move m = moves[i];
    state.makeMove(m);
  }
  show(state, eval_type);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
