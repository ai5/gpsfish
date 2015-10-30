// weightstat.cc
// 棋譜(kisen or csa)を眺めて評価関数を評価する
#include "osl/ppair/captureAnnotations.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/simpleHashTable.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_action/store.h"
#include "osl/record/kisen.h"
#include "osl/record/csaRecord.h"
#include "osl/stat/histogram.h"
#include "osl/stat/average.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/container/moveVector.h"
#include "osl/state/hashEffectState.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/pieceEval.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/progress/ptypeProgress.h"
#include "osl/apply_move/doUndoMoveLock.h"
#include <boost/scoped_ptr.hpp>
#include <map>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-c csafile] [-N#games] [-OH] "
       << " [-r min] [-R max] [-S skip-first-n-matches] "
       << " [-w weights.dat] \n"
       << " [-t goodness threshold] \n"
       << " -q annotation-file-name\n"
       << endl;
  exit(1);
}


using namespace osl;
using namespace osl::eval;
using namespace osl::ppair;

int progressMin = 0;
int progressMax = 10000;
size_t movesMin = 0;
size_t movesMax = 10000;
bool ignoreHotMove = false;
int threshold = 0;

typedef NumEffectState state_t;

class RecordTest
{
public:
  virtual ~RecordTest()
  {
  }
  virtual void processRecord(const osl::vector<Move>& moves, 
			     const CaptureAnnotations& annotations) = 0;
};

/**
 * 棋譜の指手の前後で評価値がプレイヤにとって好転しているかどうかをテスト
 */
class ParentTest : public RecordTest
{
  double numSuccess, numTotal;
public:
  ParentTest() : numSuccess(0), numTotal(0)
  {
  }
  ~ParentTest()
  {
    std::cout << "success " << numSuccess << " total " << numTotal << "\n"
	      << "ratio " << numSuccess/numTotal << "\n";
  }
  void processRecord(const osl::vector<Move>& moves, 
		     const CaptureAnnotations& annotations);
};
void ParentTest::
processRecord(const osl::vector<Move>& moves, 
	      const CaptureAnnotations& annotations)
{
  NumEffectState state((PawnMaskState(HIRATE)));
  PtypeProgress progress(state);
  PiecePairRawEval ev(state);
  for (size_t i=0; i<moves.size(); ++i)
  {
    const Player turn = state.turn();
    const int prevVal = ev.rawValue();
    
    if (EffectUtil::isKingInCheck(alt(turn), state)
	|| (! state.isValidMove(moves[i]))) 
    {
      std::cerr << "e"; // eState;
      break;
    }
    ApplyMoveOfTurn::doMove(state, moves[i]);
    ev.update(state, moves[i]);
    if (progress.progress() < progressMin)
      goto next;
    if (progress.progress() >= progressMax)
      break;
    if (annotations.isTerminal(i))
      break;
    if (ignoreHotMove && annotations.getAnnotation(i))
      goto next;
    
    if (playerToMul(turn)*(ev.rawValue() - prevVal) > threshold)
    {
      ++numSuccess;
    }
    ++numTotal;
  next:
    progress.update(state, moves[i]);
  }
}

/**
 * 棋譜の指手が全体の何番目にくるかをテスト
 */
class OrderTest : public RecordTest
{
  stat::Average allMoves;
  stat::Average quietMoves;
  stat::Average order;
  stat::Average plus;
  size_t numTested, numTotal;
  bool verbose;
  search::SearchState2Core::checkmate_t  checkmate_searcher;
public:
  explicit OrderTest(bool v) : numTested(0), numTotal(0), verbose(v)
  {
  }
  ~OrderTest()
  {
    std::cout << "all    " << allMoves.getAverage() << "\n";
    std::cout << "quiet  " << quietMoves.getAverage() << "\n";
    std::cout << "order  " << order.getAverage() << "\n";
    std::cout << "plus   " << plus.getAverage() << "\n";
    std::cout << "plus   " << plus.numElements() << "\n";
    std::cout << "tested " << numTested << "\n";
    std::cout << "total  " << numTotal << " " << (double)numTested/numTotal 
	      << "\n";
  }
  void processRecord(const osl::vector<Move>& moves, 
		     const CaptureAnnotations& annotations);
  /** recorded が何番目に高得点だったかを返す */
  void getOrder(HashEffectState& state, Move recorded, int val);
  bool isQuietMove(HashEffectState& state, Move m, int ref) 
  {
    return evalAfterMove(state, m, ref) == ref;
  }
  
  int evalAfterMove(HashEffectState& state, Move m, int ref)
  {
    typedef QuiescenceSearch2<PieceEval> qsearch_t;
    SimpleHashTable table;
    search::SearchState2Core core(state, checkmate_searcher);
    qsearch_t searcher(core, table);
    DoUndoMoveLock lock(state, m);
    PieceEval ev(state);
    const int val = (state.turn() == BLACK) 
      ? searcher.search<BLACK>(ref-2, ref+2, ev, m)
      : searcher.search<WHITE>(ref+2, ref-2, ev, m);

    return val;
  }
};

void OrderTest::getOrder(HashEffectState& state, Move recorded, int val)
{
  const Player turn = state.turn();
  MoveVector moves;
  {
    move_action::Store store(moves);
    move_generator::AllMoves<NumEffectState,move_action::Store>::
      generateMoves(turn, state, store);
  }
  this->allMoves.add(moves.size());

  typedef std::multimap<int,Move,std::greater<int> > sorter_t;
  sorter_t sorted;  
  for (size_t i=0; i<moves.size(); ++i)
  {
    if ((! ignoreHotMove) || isQuietMove(state, moves[i], val))
    {
      const int val 
	= playerToMul(turn)*PiecePairRawEval::diffWithMove(state, moves[i]);
      sorted.insert(std::make_pair(val,moves[i]));
    }
  }
  this->quietMoves.add(sorted.size());
  
  int order = 0;
  sorter_t::const_iterator p=sorted.begin();
  if (verbose)
  {
    std::cout << "top  ";
    csaShow(std::cout, p->second);
    std::cout << " " << p->first << "\n";
  }
  for (; p!=sorted.end(); ++p,++order)
  {
    if (p->second == recorded)
    {
      if (verbose)
      {
	std::cout << "real ";
	csaShow(std::cout, p->second);
	std::cout << " " << p->first << " " << order << "\n\n";
      }
      this->order.add(order);
      plus.add((p->first > threshold) ? 1 : 0);
      return;
    }
  }
  std::cerr << "recorded move not found (broken annotation?)\n";
#if 0  
  // std::cerr << moves;
  for (sorter_t::const_iterator p=sorted.begin(); p!=sorted.end(); ++p)
  {
    std::cerr << p->first << "\t" << p->second << "\n";
  }
  std::cerr << "target was " << recorded << "\n";
  std::cerr << val << "\t" << isQuietMove(state, recorded, val) << "\n";
  std::cerr << state << "\n";
  abort();
#endif
}

void OrderTest::processRecord(const osl::vector<Move>& moves, 
			      const CaptureAnnotations& annotations)
{
  HashEffectState state((SimpleState(HIRATE)));
  PtypeProgress progress(state);
  PiecePairRawEval ev(state);
  int valWithSearch = 0;
  for (size_t i=0; i<moves.size(); ++i)
  {
    const Player turn = state.turn();
    // const int prevVal = ev.getVal();
    valWithSearch += annotations.getAnnotation(i);
    
    if (EffectUtil::isKingInCheck(alt(turn), state)
	|| (! state.isValidMove(moves[i]))) 
    {
      std::cerr << "e"; // eState;
      break;
    }
    if (i < movesMin)
      goto next;
    if (i >= movesMax)
      break;
    if (progress.progress() < progressMin)
      goto next;
    if (progress.progress() >= progressMax)
      break;
    if (annotations.isTerminal(i))
      break;

    ++numTotal;
    if (annotations.getAnnotation(i))
      goto next;

    ++numTested;
    getOrder(state, moves[i], valWithSearch);
    
  next:
    ApplyMoveOfTurn::doMove(state, moves[i]);
    ev.update(state, moves[i]);
    progress.update(state, moves[i]);
  }
}

void kisenTest(RecordTest& tester,
	       size_t maxGames, size_t skipFirstNMatches, 
	       const char *annotationFileName);
void csaTest(RecordTest& tester, const char *csaFileName);

int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  const char *input_filename = "weights.txt";
  size_t maxGames = 0;
  size_t skipFirstNMatches = 0;
  const char *annotationFileName = 0;
  bool isOrderTest = false;
  bool verbose = false;
  const char *csaFileName = 0;
  while ((c = getopt(argc, argv, "c:HN:m:M:Oq:r:R:S:t:w:vh")) != EOF)
  {
    switch(c)
    {
    case 'c':	csaFileName = optarg;
      break;
    case 'H':	ignoreHotMove = true;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'm':   progressMin = atoi(optarg);
      break;
    case 'M':   progressMax = atoi(optarg);
      break;
    case 'O':   isOrderTest = true;
      break;
    case 'q':   annotationFileName = optarg;
      break;
    case 'r':   movesMin = atoi(optarg);
      break;
    case 'R':   movesMax = atoi(optarg);
      break;
    case 'S':   skipFirstNMatches = atoi(optarg);
      break;
    case 't':   threshold = atoi(optarg);
      break;
    case 'v':   verbose = true;
      break;
    case 'w':   input_filename = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! input_filename))
    usage(program_name);
  if (csaFileName)
  {
    if (skipFirstNMatches || maxGames)
    {
      std::cerr << "options conflict\n";
      usage(program_name);
    }
  }
  else
  {
    if (! annotationFileName)
      usage(program_name);
  }

  const bool success = PiecePairRawEval::setUp(input_filename);
  assert(success);
  if (! success)
    abort();
  std::unique_ptr<RecordTest> tester;
  if (isOrderTest)
    tester.reset(new OrderTest(verbose));
  else
    tester.reset(new ParentTest());

  if (csaFileName)
    csaTest(*tester, csaFileName);
  else
    kisenTest(*tester, maxGames, skipFirstNMatches, annotationFileName);
  return 0;
}

void kisenTest(RecordTest& tester,
	       size_t maxGames, size_t skipFirstNMatches, 
	       const char *annotationFileName)
{
  KisenFile kisenFile("../../data/kisen/01.kif");

  if (! maxGames)
    maxGames = kisenFile.size();
  else
    maxGames += skipFirstNMatches;

  std::ifstream annotationIn(annotationFileName);
  CaptureAnnotations annotations;

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

    tester.processRecord(moves, annotations);
  }
}

typedef QuiescenceSearch2<PieceEval> qsearcher_t;
void csaTest(RecordTest& tester, const char *csaFileName)
{
  CsaFile file(csaFileName);
  SimpleState state = file.getInitialState();
  const Record r = file.getRecord();
  const osl::vector<osl::Move> moves=r.getMoves();
  CaptureAnnotations annotations;

  HashEffectState nstate((PawnMaskState(HIRATE)));
  SimpleHashTable table;
  search::SearchState2Core::checkmate_t  checkmate_searcher;
  search::SearchState2Core core(nstate, checkmate_searcher);
  qsearcher_t qs(core,table);
  PieceEval ev(nstate);
  int prevVal = 0;
  for (size_t i=0; i<std::min((size_t)256, moves.size()); ++i)
  {
    const Player turn = nstate.turn();
    const Square opKingSquare 
      = nstate.kingSquare(alt(turn));
    // 自分の手番で相手の王が利きがある => 直前の手が非合法手
    if (nstate.hasEffectAt(turn, opKingSquare))
    {
      std::cerr << "e"; // nstate;
      break;
    }

    ApplyMoveOfTurn::doMove(nstate, moves[i]);
    ev.update(nstate, moves[i]);
    const int newVal = qs.search(alt(turn), ev, moves[i]);
    if (qsearcher_t::isWinValue(turn, newVal))
    {
      annotations.setWin(i);
      break;
    }
    else if (qsearcher_t::isWinValue(alt(turn), newVal))
    {
      annotations.setLose(i);
      break;
    }
    else
    {
      const int diff = newVal - prevVal;
      annotations.setAnnotation(i, diff);
    }
    prevVal = newVal;
  }

  tester.processRecord(moves, annotations);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
