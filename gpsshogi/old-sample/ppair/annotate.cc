// annotate.cc
// 棋譜の指手に，取り合い探索による駒価値の増減を注釈する
#include "osl/ppair/captureAnnotations.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/simpleHashTable.h"
#include "osl/record/kisen.h"
#include "osl/stat/histogram.h"
#include "osl/state/hashEffectState.h"
#include "osl/eval/pieceEval.h"
#include <iostream>
#include <fstream>
#include <cstddef>
#include <unistd.h>

using namespace osl;
using namespace osl::ppair;

void usage(const char *program_name)
{
  std::cerr << program_name << " [-N maxGames][-c]\n";
  exit(1);
}

typedef QuiescenceSearch2<PieceEval> qsearcher_t;

stat::Histogram histogram(200, 40, -4000);
size_t numWin = 0, numLose = 0;
bool useCurVal = false;

void processRecord(const osl::vector<Move>& moves, std::ostream& out)
{
  CaptureAnnotations annotations;
  search::SearchState2Core::checkmate_t  checkmate_searcher;
  HashEffectState state((SimpleState(HIRATE)));
  SimpleHashTable table;
  PieceEval ev(state);
  int prevVal = 0;

  for (size_t i=0; i<std::min((size_t)256, moves.size()); ++i)
  {
    const Player turn = state.turn();
    const Square opKingSquare 
      = state.kingSquare(alt(turn));
    // 自分の手番で相手の王が利きがある => 直前の手が非合法手
    if (state.hasEffectAt(turn, opKingSquare))
    {
      std::cerr << "e"; // state;
      break;
    }

    const int curVal = ev.value();

    ApplyMoveOfTurn::doMove(state, moves[i]);
    ev.update(state, moves[i]);

    search::SearchState2Core core(state, checkmate_searcher);
    qsearcher_t qs(core, table);
    const int newVal = qs.search(alt(turn), ev, moves[i]);
    if (qsearcher_t::isWinValue(turn, newVal))
    {
      annotations.setWin(i);
      ++numWin;
      break;
    }
    else if (qsearcher_t::isWinValue(alt(turn), newVal))
    {
      annotations.setLose(i);
      ++numLose;
      break;
    }
    else
    {
      const int diff = (useCurVal ? newVal - curVal : newVal - prevVal);
      annotations.setAnnotation(i, diff);
      histogram.add(diff);
    }
    prevVal = newVal;
  }
  annotations.dumpTo(out);
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
  const char *outputFilename=0;
  
  while ((c = getopt(argc, argv, "cN:o:vh")) != EOF)
  {
    switch(c)
    {
    case 'c':	useCurVal = true;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'o':   outputFilename = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! outputFilename))
    usage(program_name);

  std::ofstream os(outputFilename);
  
  KisenFile kisenFile("../../data/kisen/01.kif");
  if (! maxGames)
    maxGames = kisenFile.size();
  
  for (size_t i=0;i<maxGames;i++)
  {
    if (i % 1000 == 0)
    {
      if (i % 10000 == 0)
      {
	std::cerr << "\n";
	histogram.show(std::cerr);
	std::cerr << "win " << numWin << " lose " << numLose << "\n";
      }
      std::cerr << "\nprocessing " << i << "-" << i+1000 << " th record\n";
    }
    if ((i % 100) == 0) 
      std::cerr << '.';
    const osl::vector<Move>& moves=kisenFile.getMoves(i);
    processRecord(moves, os);
  }
  std::cerr << "\n";
  histogram.show(std::cerr);
  std::cerr << "win " << numWin << " lose " << numLose << "\n";

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
