/**
 * @file
 * count
 *
 * piece pair のエントリが何回出現したかを数える
 * あまり出現しないものは重みの計算から除くため
 */

#include "osl/ppair/captureAnnotations.h"
#include "osl/ppair/indexList.h"
#include "osl/ppair/pairDifference.h"
#include "osl/stat/activityCount.h"
#include "osl/record/kisen.h"
#include "osl/state/numEffectState.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/progress/ptypeProgress.h"
#include "osl/apply_move/applyMove.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] -o output-filename"
       << " [-m min] [-M max] \n"
       << " -q annotation-file-name\n"
       << " -H ignore hot moves\n"
       << endl;
  exit(1);
}

using namespace osl;
using namespace osl::eval;
using namespace osl::ppair;

int progressMin = 0;
int progressMax = 10000;

typedef NumEffectState state_t;

class PairCount : private stat::ActivityCount
{
  typedef stat::ActivityCount base_t;
public:
  PairCount() : base_t(PiecePairRawTable::maxPairIndex)
  {
  }
  void add(size_t index)
  {
    size_t i1, i2;
    PiecePairRawTable::meltIndex(index, i1, i2);
    add(i1, i2);
  }
  
  void add(size_t i1, size_t i2)
  {
    base_t::add(PiecePairRawTable::indexOf(i1,i2));
    base_t::add(PiecePairRawTable::indexOf(i2,i1));
  }
  using base_t::show;
  void stat();
};

struct CountHelper
{
  const int *data;
  int& count;
  int& total;
  CountHelper(const int *d, int& c, int& t) : data(d), count(c), total(t)
  {
  }
  void operator()(unsigned int index)
  {
    assert(index < PiecePairRawTable::maxPairIndex);
    if (data[index] > 200)
      ++count;
    ++total;
  }
};

void PairCount::stat()
{
  int count = 0, total = 0;
  PiecePairRawTable::forEachRelation(CountHelper(&counts[0], count, total));
  std::cerr << count << " / " << total << " = " << (double)count/total << "\n";
}

PairCount activities;

void processState(state_t& state, Move nextMove)
{
#ifdef PPAIR_COUNT_NO_INCREMENTAL
  // 多分このコードは使えない (差分でないと学習時間が現実的でないため)
  for(int i=0;i<PIECE_SIZE;i++)
  {
    const size_t index1 = PiecePairRawTable::getIndexOfPieceNum(state, i);
    for(int j=i;j<PIECE_SIZE;j++)
    {
      const size_t index2 = PiecePairRawTable::getIndexOfPieceNum(state, j);
      activities.add(index1, index2);
    }
  }
#else
    IndexList added, removed;
    PairDifference::diffWithMove(state, nextMove, 
				 added, removed);
    for (IndexList::const_iterator p=added.begin(); p!=added.end(); ++p)
      activities.add(*p);
    for (IndexList::const_iterator p=removed.begin(); p!=removed.end(); ++p)
      activities.add(*p);
#endif
}

bool ignoreHotMove = false;
const char *annotationFileName = 0;
void processRecord(state_t& state, const osl::vector<Move>& moves,
		   const CaptureAnnotations& annotations)
{
  for(size_t j=0;j<moves.size();j++)
  {
    const Player turn = state.turn();
    const int progress = PtypeProgress::getProgress(state);

    // 自分の手番で相手の王が利きがある => 直前の手が非合法手
    if (EffectUtil::isKingInCheck(alt(turn), state)
	|| (! state.isValidMove(moves[j]))) 
    {
      std::cerr << "e"; // eState;
      break;
    }

    if (annotationFileName && annotations.isTerminal(j))
      break;
    if (annotationFileName && ignoreHotMove && annotations.getAnnotation(j))
      goto next;
    if (progress < progressMin)
      goto next;
    if (progress >= progressMax)
      break;

    processState(state, moves[j]);
  next:
    ApplyMoveOfTurn::doMove(state, moves[j]);
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
    case 'H':   ignoreHotMove = true;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'm':   progressMin = atoi(optarg);
      break;
    case 'M':   progressMax = atoi(optarg);
      break;
    case 'o':   countFileName = optarg;
      break;
    case 'q':   annotationFileName = optarg;
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

  std::ifstream annotationIn(annotationFileName 
			     ? annotationFileName
			     : "/dev/null");
  CaptureAnnotations annotations;
  for (size_t i=0;i<maxGames;i++)
  {
    if (i % 1000 == 0)
      std::cerr << "\nprocessing " << i << "-" << i+1000 << " th record\n";
    if ((i % 100) == 0) 
      std::cerr << '.';

    annotations.loadFrom(annotationIn);
    if (annotationFileName && (! annotationIn))
      std::cerr << "cannot read " << annotationFileName << "\n";

    NumEffectState state(kisenFile.getInitialState());
    const osl::vector<Move> moves=kisenFile.getMoves(i);
    processRecord(state, moves, annotations);
  }

  activities.show(countFileName);
  activities.stat();
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
