#include "osl/kisen.h"
#include "osl/ppair/probability.h"
#include "osl/ppair/table.h"
#include "osl/numEffectState.h"
#include "osl/piecePairRawEval.h"
#include "osl/generateAllMoves.h"
#include "osl/moveVector.h"
#include "osl/storeMoveAction.h"
#include "osl/eval.h"
#include "osl/eval.tcc"
#include "osl/pieceEval.h"
#include "osl/pieceEval.tcc"
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] -f tableFilename"
       << endl;
  exit(1);
}

using namespace osl;
using namespace osl::ppair;

Table table;

typedef EvalState<NumEffectState,PieceEval> state_t;

// 駒の損得がある手かどうかの判定
// 0 なし
// + 得
// - 損
int valueOfMove(state_t& state, Move nextMove)
{
  return state.computeDiffAfterMoveForRP(state.turn(), nextMove);
}

const int hotMoveThreshold = -PtypeEvalTraits<PAWN>::val*2;	// 歩の丸損

std::pair<int,int> findPriorityOfMove(state_t& state, Move nextMove)
{
  const int valueOfTheMove = valueOfMove(state, nextMove);
  if (valueOfTheMove <= hotMoveThreshold)
    return;
  
  MoveVector comparableSiblings;
  computeComparableSiblings(state, nextMove, valueOfTheMove, comparableSiblings);
  comparableSiblings.add(nextMove);
  
  MoveLogProbVector moveWithValue; // ここでの使い方は大きい数字が確率が高い
  for (size_t i=0; i<comparableSiblings.size(); ++i)
  {
    const Move m = comparableSiblings[i];
    const int prob = assignProbability(state, table, m);
    moveWithValue.add(MoveLogProb(m,prob));
  }
  moveWithValue.sortByProbabilityReverse();
  for (size_t i=0; i<moveWithValue.size(); ++i)
  {
    const MoveLogProb& m = moveWithValue[i];
    if (verbose)
    {
      std::cerr << std::setw(3) << i << " ";
      csaShow(std::cerr, m.getMove());
      std::cerr << " " << m.getLogProb() << "\n";
    }
    if (m == nextMove)
      return std::make_pair(i,moveWithValue.size());
  }
  abort();
}

void processRecord(state_t& state, const osl::vector<Move>& moves)
{
  for(size_t j=0;j<moves.size();j++){
    const Player turn = state.turn();
    const int myKingIndex=ptypeTable.getKingIndex(turn);
    const int opKingIndex=ptypeTable.getKingIndex(alt(turn));
    const Square myKingSquare=getSquare(state.pieceOf(myKingIndex));
    const Square opKingSquare=getSquare(state.pieceOf(opKingIndex));

    // 自分の手番で相手の王が利きがある => 直前の手が非合法手
    if (state.hasEffectAt(turn,opKingSquare) 
	|| (! state.isAlmostValidMove(moves[j]))) {
      std::cerr << "e"; // eState;
      break;
    }
    // 王手から逃げることしかできないので統計からは外す
    if (state.hasEffectAt(alt(turn),myKingSquare))
      goto next;
    
    {
      std::pair<int,int> priorityAndSize = findPriorityOfMove(state, moves[j]);
      std::cout << priorityAndSize.first << " " << priorityAndSize.second
		<< "\n";
    }
  next:
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
  const char *tableFileName = 0;
  while ((c = getopt(argc, argv, "N:vh")) != EOF)
  {
    switch(c)
    {
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'f':	tableFileName = optarg;
      break;
    default:	
      error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! tableFileName))
    usage(program_name);

  std::ifstream is(tableFileName);
  table.binaryLoad(is);

  KisenFile kisenFile("../nodist/kisen/01.kif");
  if (! maxGames)
    maxGames = kisenFile.size();
  
  for(size_t i=0;i<maxGames;i++){
    if (i==172518) 
      continue;
    if (i % 1000 == 0)
      std::cerr << "\nprocessing " << i << "-" << i+1000 << " th record\n";
    if ((i % 100) == 0) 
      std::cerr << '.';
    PawnMaskState state=kisenFile.getInitialState();
    NumEffectState nState(state);
    state_t eState(nState);
    const osl::vector<Move> moves=kisenFile.getMoves(i);
    processRecord(eState, moves);
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
