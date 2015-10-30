// findQuiescence
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/simpleHashTable.h"
#include "osl/record/kisen.h"
#include "osl/stat/histogram.h"
#include "osl/state/hashEffectState.h"
#include "osl/eval/pieceEval.h"
#include "osl/progress/ptypeProgress.h"
#include <iostream>
#include <iomanip>
#include <cstddef>
#include <unistd.h>
using namespace osl;

void usage(const char *program_name)
{
  std::cerr << program_name << " [-N maxGames]\n";
  exit(1);
}

typedef QuiescenceSearch2<PieceEval> qsearcher_t;
int qsearch(qsearcher_t& qs, Player P, int prevVal,
	     PieceEval ev, Move last_move)
{
  return qs.search(P, ev, last_move);
}

stat::Histogram progress0(10,30), progressTotal(10,30);
stat::Histogram moves0(10,30), movesTotal(10,30);

bool useCurVal = false;

void processRecord(const osl::vector<Move>& moves)
{
  search::SearchState2Core::checkmate_t  checkmate_searcher;
  HashEffectState state((SimpleState(HIRATE)));
  SimpleHashTable table;
  PieceEval ev(state);
  PtypeProgress progress(state);
  int prevVal = 0;
  for (size_t i=0; i<moves.size(); ++i)
  {
    const Player turn = state.turn();
    const Square opKingSquare 
      = state.kingSquare(alt(turn));
    // 自分の手番で相手の王が利きがある => 直前の手が非合法手
    if (state.hasEffectAt(turn,opKingSquare))
    {
      std::cerr << "e"; // state;
      break;
    }

    const int curVal = ev.value();

    ApplyMoveOfTurn::doMove(state, moves[i]);
    ev.update(state, moves[i]);
    progress.update(state, moves[i]);

    search::SearchState2Core core(state, checkmate_searcher);
    qsearcher_t qs(core,table);
    const int newVal = qsearch(qs, alt(turn), curVal,
			       ev, moves[i]);
#ifdef SHOW_RAW_DATA
    std::cout << std::setw(3) << i 
	      << std::setw(4) << progress.getVal()
	      << std::setw(5) << playerToMul(turn)*(newVal - curVal)
	      << std::setw(5) << playerToMul(turn)*(newVal - prevVal) 
	      << "\n";
#endif
    const int diff = (useCurVal ? newVal - curVal : newVal - prevVal);
    if (diff == 0)
    {
      moves0.add(i);
      progress0.add(progress.progress());
    }
    movesTotal.add(i);
    progressTotal.add(progress.progress());

    prevVal = newVal;
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

  size_t maxGames = 0;
  while ((c = getopt(argc, argv, "cN:vh")) != EOF)
  {
    switch(c)
    {
    case 'c':	useCurVal = true;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag)
    usage(program_name);

  KisenFile kisenFile("../../data/kisen/01.kif");
  if (! maxGames)
    maxGames = kisenFile.size();

  for (size_t i=0;i<maxGames;i++)
  {
    if (i % 1000 == 0)
      std::cerr << "\nprocessing " << i << "-" << i+1000 << " th record\n";
    if ((i % 100) == 0) 
      std::cerr << '.';
    const osl::vector<Move>& moves=kisenFile.getMoves(i);
    processRecord(moves);
  }

  std::cerr << "progress\n";
  progressTotal.showRatio(std::cerr,progress0);
  std::cerr << "moves\n";
  movesTotal.showRatio(std::cerr,moves0);
  

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
