/**
 * 詰将棋の指手生成の速さを見る
 */
#include "osl/csa.h"

#define NO_SAFE_MOVE_ACTION_IN_LIBOSL
#define SIMPLE_STATE_ONLY

#include "osl/move_generator/addEffect_.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_generator/escape_.h"

#ifdef NO_SAFE_MOVE_ACTION_IN_LIBOSL
#  include "osl/move_generator/open.tcc"
#  include "osl/move_generator/allMoves.tcc"
#  include "osl/move_generator/escape_.tcc"
#  include "osl/move_generator/capture_.tcc"
#  include "osl/move_generator/addEffect_.tcc"
#  include "osl/move_generator/addEffectWithEffect.tcc"
#endif

#ifndef SIMPLE_STATE_ONLY
#  include "osl/boardBitEffect.h"
#  include "osl/evalHashEffectState.h"
#  include "osl/numEffectState.h"
#endif

#include "osl/applyMove.h"
#include "osl/move_action/store.h"
#include "osl/move_action/safeFilter.h"
#include "osl/perfmon.h"
#include <time.h>
#include <sys/time.h>

using namespace osl;
using namespace osl::move_generator;
using namespace osl::move_action;

int moveCount;
int maxLevel;

template<Player P,typename State, bool isAttack,bool withEffect>
void nextMoves(State& state,int level,Move lastMove);

template<class State, Player P, bool isAttack,bool withEffect>
struct DoUndoHelper
{
  State& state;
  int level;
  Move move;
    
  DoUndoHelper(State& s, int level) : state(s), level(level), move(MOVE_INVALID){}
  void operator()(Square p)
  {
    assert(move!=MOVE_INVALID);
    nextMoves<P,State,isAttack,withEffect>(state,level,move);
  }
};

template <Player P, typename State, bool isAttack,bool withEffect>
void nextMoves(State& state,int level,Move lastMove)
{
  typedef typename State::effect_state_t effect_t;
  if (level>maxLevel) 
    return;
  MoveVector moves;
  typedef DoUndoHelper<State,PlayerTraits<P>::opponent,!isAttack,withEffect> helper_t;
  helper_t helper(state, level+1);
  {
    Store store(moves);
    if (isAttack)
    {
      typedef SafeFilter<P,typename State::effect_state_t,Store> action_t;
      action_t safeAction(state,store);
      const Square opKingSquare
	=state.template kingSquare<PlayerTraits<P>::opponent>();
#if 1
      if (state.hasEffectAt(P,opKingSquare)) // 逃げる手になっていない
	return; // 詰
#else
      assert(!state.hasEffectAt(P,opKingSquare));
#endif
      if(withEffect)
	AddEffectWithEffect<typename State::effect_state_t,action_t>::
	  generateMoves(P,(effect_t)state,opKingSquare,safeAction);
#if 1
      else
	AddEffect<typename State::effect_state_t,action_t>::
	  generateMoves(P,state,opKingSquare,safeAction);
#endif
    }
    else
    {
      assert(!state.hasEffectAt(P,state.template kingSquare<PlayerTraits<P>::opponent>()));

      Escape<P,typename State::effect_state_t,Store>::
	generateKingEscape((effect_t)state, lastMove,store);
    }
  }
  size_t size=moves.size();
  for(size_t i=0;i<size;i++){
    // std::cerr << i << " " << moves[i] << "\n";
    moveCount++;
    helper.move=moves[i];
    ApplyMove<P>::doUndoMove(state,moves[i],helper);
  }
}

int main(int argc,char **argv){
  bool effectMode=false;
  bool hashMode=false;
  bool evalMode=false;
  bool nullMode=false;
  bool withEffectMode=false;
  bool simpleMode=false;
  bool boardBitMode=false;
  int level=3;
  extern char *optarg;

  char c;
  while ((c = getopt(argc, argv, "l:dehEnwsb")) != EOF)
  {
    switch(c)
    {
    case 'l':	level=atoi(optarg);
      break;
    case 'e':	effectMode=true;
      break;
    case 'h':	hashMode=true;
      break;
    case 'E':	evalMode=true;
      break;
    case 'n':	nullMode=true;
      break;
    case 'w':	withEffectMode=true;
      break;
    case 's':	simpleMode=true;
      break;
    case 'b':	boardBitMode=true;
      break;
    default:
      std::cerr << "unknown option\n";
      return 1;
    }
  }
  SimpleState state=CsaString(
"P1-KY *  *  * -KY * -FU-KE * \n"
"P2 *  *  *  * -OU *  *  *  * \n"
"P3 *  *  * -FU-FU+RY *  * -KY\n"
"P4-FU *  * -GI *  *  *  *  * \n"
"P5 *  *  *  *  *  *  *  *  * \n"
"P6+FU *  * +RY *  * +FU *  * \n"
"P7 * +FU * +FU+FU+FU *  *  * \n"
"P8 *  * +OU * -TO *  *  *  * \n"
"P9+KY *  *  *  *  *  * +KE * \n"
"P+00KI00GI00GI00GI00KE00KE00FU00FU00FU00KI\n"
"P-00KA00KA00KI00FU00FU00FU00FU00KI\n"
"-\n").getInitialState();
  maxLevel=level;
  moveCount=0;
  clock_start();

#ifndef SIMPLE_STATE_ONLY
  if(evalMode)
  {
    std::cerr << "evalMode" << std::endl;
    NumEffectState neState(state);
    HashEffectState hState(neState);
    EvalHashEffectState eState(hState);
    
    Move lastMove=newMove(newSquare(4,4),newSquare(4,3),PROOK,
			  PTYPE_EMPTY,false,BLACK);

    if(withEffectMode)
      nextMoves<WHITE,EvalHashEffectState,false,true>(eState,0,lastMove);
    else
      nextMoves<WHITE,EvalHashEffectState,false,false>(eState,0,lastMove);
  }
  else if(hashMode)
  {
    std::cerr << "hashMode" << std::endl;
    NumEffectState neState(state);
    HashEffectState hState(neState);
    
    Move lastMove=newMove(newSquare(4,4),newSquare(4,3),PROOK,
			  PTYPE_EMPTY,false,BLACK);
    if(withEffectMode)
      nextMoves<WHITE,HashEffectState,false,true>(hState,0,lastMove);
    else
      nextMoves<WHITE,HashEffectState,false,false>(hState,0,lastMove);
  }
  else if(effectMode);
  {
    assert(effectMode);
    
    std::cerr << "effectMode" << std::endl;
    NumEffectState neState(state);
    Move lastMove=newMove(newSquare(4,4),newSquare(4,3),PROOK,
			  PTYPE_EMPTY,false,BLACK);
    if(withEffectMode)
      nextMoves<WHITE,NumEffectState,false,true>(neState,0,lastMove);
    else
      nextMoves<WHITE,NumEffectState,false,false>(neState,0,lastMove);
  }
#if 0
  else 
    if(boardBitMode){
    std::cerr << "boardBitEffectMode" << std::endl;
    typedef BoardBitEffect<SimpleState> effect_state_t;
    effect_state_t neState(state);
    Move lastMove=newMove(newSquare(4,4),newSquare(4,3),PROOK,
			  PTYPE_EMPTY,false,BLACK);
    nextMoves<WHITE,effect_state_t,false,true>(neState,0,lastMove);
  }
#endif
#if 0
  else if(nullMode){
    std::cerr << "nullBoardEffectMode" << std::endl;
    typedef NullBoardEffect<SimpleState> effect_state_t;
    effect_state_t neState(state);
    Move lastMove=newMove(newSquare(4,4),newSquare(4,3),PROOK,
			  PTYPE_EMPTY,false,BLACK);
    nextMoves<WHITE,effect_state_t,false,true>(neState,0,lastMove);
  }
#endif
    else 
#else /*  SIMPLE_STATE_ONLY */
      if(simpleMode){
      std::cerr << "nullBoardEffectMode" << std::endl;
      typedef SimpleState effect_state_t;
      effect_state_t neState(state);
    Move lastMove=newMove(newSquare(4,4),newSquare(4,3),PROOK,
			  PTYPE_EMPTY,false,BLACK);
      nextMoves<WHITE,effect_state_t,false,true>(neState,0,lastMove);
    }
#endif
  clock_stop("total", moveCount+1);
  std::cerr << "maxLevel=" << maxLevel << ",moveCount=" << moveCount 
	    << std::endl;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
