/**
 * 以前，ゲーム情報学研究会で発表した例題を高速に実行できるかどうかのチェック
 * なお，以前は
 * maxLevel=3,moveCount=444378267,dropCount=357602901,sec=11.630000,count/s=38209653.224420
 * という結果だった．
 */
#include "osl/csa.h"
#include "osl/numEffectState.h"

#ifdef MORE_STATE
#  include "osl/boardBitEffect.h"
#  include "osl/signatureEffect.h"
#endif 

#ifdef PPAIR_PERF
#  include "osl/piecePairEval.h"
#  include "osl/pieceEval.h"
#endif

#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/allMoves.tcc"
#include "osl/move_generator/move_action.h"
#include "osl/misc/perfmon.h"
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <cstdio>

using namespace osl;

int moveCount;
int dropCount;
int maxLevel;

int maxVal=0;

#ifdef PPAIR_PERF
bool noPair = false;
bool piecePair = false;
int value;
#endif

template<Player P,typename State,bool isMoveEval>
void nextMoves(State& state,int level);

template<class State, Player P,bool isMoveEval>
struct DoUndoHelper{
  State& state;
  int level;
    
  DoUndoHelper(State& s, int level) : state(s), level(level){}
  void operator()(Square p){
    nextMoves<P,State,isMoveEval>(state,level);
  }
};

template<Player P,typename State>
void callValWithMove(State& state,Move move,Int2Type<false>){
}

template<Player P,typename State>
void callValWithMove(State& state,Move move,Int2Type<true>){
  typename State::eval_t ev = state.getEval();
  const int ret = ev.template computeValWithMove<State,P>(state, move);
  if(ret>maxVal){maxVal=ret;}
}


template<Player P,typename State,bool isMoveEval>
void nextMoves(State& state,int level){
  if(level>maxLevel) return;
  MoveVector moves;
  DoUndoHelper<State,alt(P),isMoveEval> helper(state, level+1);
  GenerateAllMoves::generate(P,state,moves);

  size_t size=moves.size();
  for(size_t i=0;i<size;i++){
    callValWithMove<P,State>(state,moves[i],Int2Type<isMoveEval>());
    moveCount++;
#ifdef PPAIR_PERF
    const int prevValue = value;
    if (noPair)
      value += PieceEval::diffWithMove(state, moves[i]);
    else if (piecePair)
      value += PiecePairEval::diffWithMove(state, moves[i]);
#endif    
    state.makeUnmakeMove(Player2Type<P>(),moves[i],helper);
#ifdef PPAIR_PERF
    value = prevValue;
#endif    
  }
}

#ifdef ENABLE_DIRECT_MODE
template<Player P,typename State,bool isMoveEval>
void nextMovesDirect(State& state,int level);

template<class State,Player P,bool isMoveEval>
struct CallNextMoves{
  State& state;
  int level;
  CallNextMoves(State& s, int level) : state(s), level(level){}

  void operator()(Square p){
    nextMovesDirect<P,State,isMoveEval>(state,level);
  }
};

template<typename State,Player P,bool isMoveEval>
struct DirectCallAction{
  State& state;
  int level;
  DirectCallAction(State& s, int level) : state(s), level(level){}
  typedef CallNextMoves<State,PlayerTraits<P>::opponent,isMoveEval> next_t;
  void receiveSimpleMove(Square from,Square to,Ptype ptype, bool isPromote,Player p){

    moveCount++;
    next_t caller(state,level);
    ApplyDoUndoSimpleMove<P,State>::template doUndoSimpleMove<next_t>
      (state,from,to,promoteMask(isPromote),caller);
  }
  void receiveUnknownMove(Square from,Square to,Piece p1,Ptype ptype,bool isPromote,Player p){
    next_t  caller(state,level);
    moveCount++;
    if(p1==PIECE_EMPTY)
      ApplyDoUndoSimpleMove<P,State>::template doUndoSimpleMove<next_t>
	(state,from,to,promoteMask(isPromote),caller);
    else
      ApplyDoUndoCaptureMove<P,State>::template doUndoCaptureMove<next_t>
	(state,from,to,p1,promoteMask(isPromote),caller);
  }
  void receiveDropMove(Square to,Ptype ptype,Player p){
    moveCount++;
    next_t  caller(state, level);
    ApplyDoUndoDropMove<P,State>::template doUndoDropMove<next_t>(state,to,ptype,caller);
  }
};

template<Player P,typename State,bool isMoveEval>
void nextMovesDirect(State& state,int level){
  if(level>maxLevel) return;
  DirectCallAction<State,P,isMoveEval> action(state, level+1);
  typedef move_generator::AllMoves<State,DirectCallAction<State,P,isMoveEval> > generate_t;
  generate_t::template generateMoves<P>(state,action);
}
#endif /* ENABLE_DIRECT_MODE */

int main(int argc,char **argv){
  bool directMode=false;
  bool effectMode=false;
  bool hashMode=false;
  bool boardBitMode=false;
  bool signatureMode=false;
  int level=3;
  extern char *optarg;

  char c;
  while ((c = getopt(argc, argv, "l:dehpPbS")) != EOF)
    {
      switch(c)
	{
	case 'l':	level=atoi(optarg);
	  break;
	case 'd':	directMode=true;
	  break;
	case 'e':	effectMode=true;
	  break;
	case 'h':	hashMode=true;
	  break;
	case 'b':	boardBitMode=true;
	  break;
	case 'S':	signatureMode=true;
	  break;
#ifdef PPAIR_PERF
	case 'p':	noPair=true;
	  break;
	case 'P':	piecePair=true;
	  break;
#endif
	default:
	  std::cerr << "unknown option\n";
	  return 1;
	}
    }
  SimpleState state=CsaString(
			      "P1+RY *  *  *  *  *  * -KE-KY\n"
			      "P2 *  *  * +GI * -KI-KI-OU *\n"
			      "P3 *  * +TO *  *  * -GI-FU-FU\n"
			      "P4-FU * -UM * -FU * -FU *  *\n"
			      "P5 *  * +GI *  * -FU * +FU+FU\n"
			      "P6+FU+OU+GI+FU+FU * +FU *  *\n"
			      "P7 * +FU * +KE *  *  *  *  *\n"
			      "P8 *  *  *  *  *  *  *  *  * \n"
			      "P9-RY *  *  *  *  *  *  * +KY\n"
			      "P+00KA00KY00FU00FU00FU00FU\n"
			      "P-00KI00KI00KE00KE00KY\n"
			      "+\n").initialState();
  maxLevel=level;
  moveCount=0;
  dropCount=0;
  misc::PerfMon timer;
  if(effectMode){
    std::cerr << "effectMode" << std::endl;
    NumEffectState neState(state);
    
    if(directMode){
      std::cerr << "directMode" << std::endl;
#ifdef ENABLE_DIRECT_MODE
      nextMovesDirect<BLACK,NumEffectState,false>(neState,0);
#endif
    }
    else{
      nextMoves<BLACK,NumEffectState,false>(neState,0);
    }
  }
  else
#ifdef MORE_STATE
#ifndef PPAIR_PERF
    if(signatureMode){
      std::cerr << "signatureMode" << std::endl;
      SimpleState sState(state);
      typedef SignatureEffect<BoardBitEffect<SimpleState> > state_t;
      state_t bState(sState);
    
      if(directMode){
	std::cerr << "directMode" << std::endl;
#ifdef ENABLE_DIRECT_MODE
	nextMovesDirect<BLACK,state_t,false>(bState,0);
#endif
      }
      else{
	nextMoves<BLACK,state_t,false>(bState,0);
      }
    }
    else
      if(boardBitMode){
	std::cerr << "boardBitMode" << std::endl;
	SimpleState sState(state);
	typedef BoardBitEffect<SimpleState> state_t;
	state_t bState(sState);
    
	if(directMode){
	  std::cerr << "directMode" << std::endl;
#ifdef ENABLE_DIRECT_MODE
	  nextMovesDirect<BLACK,state_t,false>(bState,0);
#endif
	}
	else{
	  nextMoves<BLACK,state_t,false>(bState,0);
	}
      }
      else
#endif /* ifndef PPAIR_PERF */
	if(simpleMode){
	  std::cerr << "simpleMode" << std::endl;
	  SimpleState sState(state);
    
	  if(directMode){
	    std::cerr << "directMode" << std::endl;
#ifdef ENABLE_DIRECT_MODE
	    nextMovesDirect<BLACK,SimpleState,false>(sState,0);
#endif
	  }
	  else{
	    nextMoves<BLACK,SimpleState,false>(sState,0);
	  }
	}
	else
#endif /* MORE_STATE */
	{
	}
  timer.stop("total", moveCount+1);
  std::cerr << "maxLevel=" << maxLevel << ",moveCount=" << moveCount <<
    ",dropCount=" << dropCount << std::endl;

}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
