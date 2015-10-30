// scatter.cc
// ´ýÉè¤òÄ¯¤á¤ÆÉ¾²Á´Ø¿ô¤ÎÉ¾²ÁÃÍ¤ò½ÐÎÏ
#include "osl/ppair/captureAnnotations.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/record/kisen.h"
#include "osl/stat/twoDimensionalStatistics.h"
#include "osl/state/numEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/pieceEval.h"
#include "osl/progress/ptypeProgress.h"
#include "osl/apply_move/applyMove.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] "
       << " [-m min] [-M max] [-D] [-S skip-first-n-matches] "
       << " [-w weights.dat] "
       << " -q annotation-file-name\n"
       << " -D differential output"
       << endl;
  exit(1);
}

using namespace osl;
using namespace osl::eval;
using namespace osl::ppair;

int progressMin = 0;
int progressMax = 10000;
bool differentialMode = false;
typedef NumEffectState state_t;

stat::TwoDimensionalStatistics stat_ev, stat_pair;

void processRecord(const osl::vector<Move>& moves, 
		   const CaptureAnnotations& annotations)
{
  NumEffectState state((PawnMaskState(HIRATE)));
  PtypeProgress progress(state);
  PieceEval ev(state);
  PiecePairRawEval pair_ev(state);
  int captureVal = 0;
  for (size_t i=0; i<moves.size(); ++i)
  {
    const Player turn = state.turn();
    const int prevPieceVal = ev.value();
    const int prevPairVal = pair_ev.rawValue();
    ev.update(state, moves[i]);
    pair_ev.update(state, moves[i]);
    captureVal += annotations.getAnnotation(i);
    if (EffectUtil::isKingInCheck(alt(turn), state)
	|| (! state.isValidMove(moves[i]))) 
    {
      std::cerr << "e"; // eState;
      break;
    }
    if (progress.progress() < progressMin)
      goto next;
    if (progress.progress() >= progressMax)
      break;
    if (annotations.isTerminal(i))
      break;
    if (differentialMode)
    {
      const int ref = annotations.getAnnotation(i);
      const int piece = ev.value() - prevPieceVal;
      const int pair  = pair_ev.rawValue() - prevPairVal;
      std::cout << i << " " << progress.progress() << " "
		<< ref << " "
		<< piece << " " << pair << "\n";
      stat_ev.addInstance(piece, ref);
      stat_pair.addInstance(pair, ref);
    }
    else
    {
      std::cout << i << " " << progress.progress() << " " << captureVal << " "
		<< ev.value() << " " << pair_ev.rawValue() << "\n";
      stat_ev.addInstance(ev.value(), captureVal);
      stat_pair.addInstance(pair_ev.rawValue(), captureVal);
    }
  next:
    progress.update(state, moves[i]);
    ApplyMoveOfTurn::doMove(state, moves[i]);
  }
}

int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  const char *input_filename = "weights.txt";
  KisenFile kisenFile("../../data/kisen/01.kif");
  size_t maxGames = 0;
  size_t skipFirstNMatches = 0;
  const char *annotationFileName = 0;
  while ((c = getopt(argc, argv, "DN:m:M:q:S:w:vh")) != EOF)
  {
    switch(c)
    {
    case 'D':   differentialMode = true;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'm':   progressMin = atoi(optarg);
      break;
    case 'M':   progressMax = atoi(optarg);
      break;
    case 'q':   annotationFileName = optarg;
      break;
    case 'S':   skipFirstNMatches = atoi(optarg);
      break;
    case 'w':   input_filename = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! input_filename) || (! annotationFileName))
    usage(program_name);

  if (! maxGames)
    maxGames = kisenFile.size();
  else
    maxGames += skipFirstNMatches;

  std::ifstream annotationIn(annotationFileName);
  CaptureAnnotations annotations;
  const bool success = PiecePairRawEval::setUp(input_filename);
  assert(success);
  if (! success)
    abort();
  
  for (size_t i=0;i<maxGames;i++)
  {
    annotations.loadFrom(annotationIn);
    if (annotationFileName && (! annotationIn))
      std::cerr << "cannot read " << annotationFileName << "\n";

    if (i < skipFirstNMatches)
      continue;
    if (i % 1000 == 0)
      std::cerr << "\nprocessing " << i << "-" << i+1000 << " th record\n";
    if ((i % 100) == 0) 
      std::cerr << '.';
    
    const osl::vector<Move> moves=kisenFile.getMoves(i);

    processRecord(moves, annotations);
  }
  std::cerr << "piece " << sqrt(stat_ev.meanSquaredErrorsAdjustConstant()) 
	    << "\n";
  std::cerr << "piece " << stat_ev.correlation()
	    << "\n";
  std::cerr << "pair  " << sqrt(stat_pair.meanSquaredErrorsAdjustConstant()) 
	    << "\n";
  std::cerr << "pair  " << stat_pair.correlation()
	    << "\n";

  return 0;
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
