/**
 * @file
 * count.cc
 *
 * エントリが何回出現したかを数える
 * あまり出現しないものは重みの計算から除くため
 */

#include "osl/progress/ptypeoSquare.h"
#include "osl/progress/ptypeRank.h"
#include "osl/stat/activityCount.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/effectUtil.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] -o output-filename"
       << " -g gekisashi simulation\n"
       << endl;
  exit(1);
}

using namespace osl;
using namespace osl::progress;

typedef NumEffectState state_t;

stat::ActivityCount activities(3); // ここでfeature の数が必要 クラス化する?

void processState(state_t& state, Move nextMove)
{
}

void processRecord(state_t& state, const osl::vector<Move>& moves)
{
  for (size_t j=0;j<moves.size();j++)
  {
    const Player turn = state.turn();

    // 自分の手番で相手の王が利きがある => 直前の手が非合法手
    if (EffectUtil::isKingInCheck(alt(turn), state)
	|| (! state.isValidMove(moves[j]))) 
    {
      std::cerr << "e"; // eState;
      break;
    }

    processState(state, moves[j]);
    state.doMove(moves[j]);
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
  const char *countFileName = 0;
  while ((c = getopt(argc, argv, "HN:m:M:o:q:vh")) != EOF)
  {
    switch(c)
    {
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'o':   countFileName = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! countFileName))
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

    NumEffectState state(kisenFile.getInitialState());
    const osl::vector<Move> moves=kisenFile.getMoves(i);
    processRecord(state, moves);
  }

  activities.show(countFileName);
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
