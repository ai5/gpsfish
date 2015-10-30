// showDifferences

#include "osl/record/kisen.h"
#include "osl/ppair/indexList.h"
#include "osl/ppair/pairDifference.h"
#include "osl/stat/activityCount.h"
#include "osl/ppair/discriminationInstance.h"
#include "osl/ppair/siblingUtil.h"
#include "osl/state/numEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/progress/ptypeProgress.h"
#include "moveCache.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] -a occurrence-filename"
       << " [-I] [-m min] [-M max] [-T threshold=20] "
       << " -I ignore siblings instead of using chain of sibsings\n"
       << endl;
  exit(1);
}


using namespace osl;
using namespace osl::eval;
using namespace osl::ppair;

int progressMin = 0;
int progressMax = 10000;
int infinite_loop = true;

typedef NumEffectState state_t;
stat::ActivityCount activities(PiecePairRawTable::maxPairIndex);

KisenFile kisenFile("../../data/kisen/01.kif");
size_t maxGames = 0;

void produce();

SiblingPolicy policy = CHAIN;

int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  const char *activityFileName = 0;
  size_t threshold=20;
  while ((c = getopt(argc, argv, "a:IN:m:M:T:t:o:vh")) != EOF)
  {
    switch(c)
    {
    case 'I':   policy = IGNORE;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'm':   progressMin = atoi(optarg);
      break;
    case 'M':   progressMax = atoi(optarg);
      break;
    case 'a':   activityFileName = optarg;
      break;
    case 'T':   threshold = atoi(optarg);
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! activityFileName))
    usage(program_name);

  activities.loadBinary(activityFileName, threshold);

  if (! maxGames)
    maxGames = kisenFile.size();
  produce();

  return 0;
}

void produce()
{
  MoveCache matches;
  matches.getAllMoves(kisenFile, maxGames, progressMin, progressMax);
  
  size_t count = 0;
  do
  {
    for (size_t i=0;i<maxGames;i++)
    {
      if (i % 1000 == 0)
	std::cerr << "\nprocessing " << i << "-" << i+1000 << " th record\n";
      if ((i % 100) == 0) 
	std::cerr << '.';
      NumEffectState initial((PawnMaskState(HIRATE)));
      const osl::vector<Move>& moves=matches.getMoves(initial, i);
      DiscriminationInstanceArray data;
      // oops, SiblingUtil is not maintained now (2003-10-05)
      SiblingUtil::generateInstances(initial, moves, activities, policy, data);
      ++count;
      for (size_t i=0; i<data.size(); ++i)
      {
	data[i].show();
      }
    }
  } while (infinite_loop);
  std::cerr << "produce " << count << " data\n";
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
