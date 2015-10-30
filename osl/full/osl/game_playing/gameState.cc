/* gameState.cc
 */
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/openingBookTracer.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/search/moveStackRejections.h"
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/state/historyState.h"
#include "osl/hash/hashKeyStack.h"
#include "osl/container/moveStack.h"
#include "osl/repetitionCounter.h"
#include "osl/sennichite.h"
#include "osl/enterKing.h"

struct osl::game_playing::GameState::State
#if OSL_WORDSIZE == 32
  : public osl::misc::Align16New
#endif
{
  HistoryState state;
  RepetitionCounter counter;
  MoveStack move_history;
  std::vector<int> eval_stack;
  
  State(const SimpleState& initial_state) 
    : state(initial_state), counter(state.state())
  {
    move_history.reserve(1024);
  }
};

osl::game_playing::
GameState::GameState(const SimpleState& initial_state) 
  : stack(new State(initial_state))
{
}

osl::game_playing::
GameState::GameState(const State& src) 
  : stack(new State(src))	// clone
{
}

osl::game_playing::
GameState::~GameState()
{
}

const osl::Sennichite osl::game_playing::
GameState::pushMove(Move m, int eval)
{
  stack->move_history.push(m);
  const Sennichite result
    = stack->counter.isSennichite(state(), m);
  stack->counter.push(state(), m);
  stack->state.makeMove(m);
  stack->eval_stack.push_back(eval);
  return result;
}

osl::game_playing::GameState::MoveType osl::game_playing::
GameState::isIllegal(Move m) const
{
  if (! state().isValidMove(m, false))
    return OTHER_INVALID;
  typedef move_classifier::PlayerMoveAdaptor<move_classifier::PawnDropCheckmate>
    PawnDropCheckmate_t;
  if (PawnDropCheckmate_t::isMember(state(), m))
    return PAWN_DROP_FOUL;

  stack->state.makeMove(m);
  const bool unsafe_king = state().inCheck(alt(state().turn()));
  stack->state.unmakeMove();
  
  if (unsafe_king)
    return UNSAFE_KING;
  
  return VALID;
}

const osl::Move osl::game_playing::
GameState::popMove()
{
  const Move result = stack->move_history.lastMove();
  assert(canPopMove());
  stack->move_history.pop();
  stack->counter.pop();
  stack->state.unmakeMove();
  stack->eval_stack.pop_back();
  return result;
}

const osl::NumEffectState& osl::game_playing::
GameState::state() const
{
  return stack->state.state(); 
}

int osl::game_playing::
GameState::moves() const 
{ 
  return stack->move_history.size();
}

const osl::MoveStack& osl::game_playing::
GameState::moveHistory() const 
{ 
  return stack->move_history;
}

const osl::hash::HashKeyStack& osl::game_playing::
GameState::hashHistory() const
{
  return stack->counter.history();
}

const osl::RepetitionCounter& osl::game_playing::
GameState::counter() const
{
  return stack->counter;
}

bool osl::game_playing::
GameState::canPopMove() const
{
  return ! stack->state.empty();
}

const std::shared_ptr<osl::game_playing::GameState> osl::game_playing::
GameState::clone() const
{
  std::shared_ptr<GameState> result(new GameState(*stack));
  return result;
}

const osl::SimpleState& osl::game_playing::
GameState::initialState() const
{
  return stack->state.initialState();
}

const std::vector<int>& osl::game_playing::
GameState::evalStack() const
{
  return stack->eval_stack;
}

void osl::game_playing::
GameState::generateMoves(MoveVector& normal, 
			 MoveVector& win, 
			 MoveVector& draw, 
			 MoveVector& loss) const
{
  MoveVector all;
  state().generateLegal(all);
  NumEffectState copy;
  const HashKey key(state());
  for (Move m: all) {
    if (isIllegal(m) != VALID) {
      loss.push_back(m);
      continue;
    }
    const Sennichite result
      = counter().isAlmostSennichite(key.newMakeMove(m));
    if (! result.isNormal()) {
      if (! result.hasWinner())
	draw.push_back(m);
      else {
	if (result.winner() == alt(state().turn()))
	  loss.push_back(m);
	else
	  win.push_back(m);
      }
      continue;
    } 
    if (rejectByStack(m)) {
      loss.push_back(m);
      continue;
    }
    copy.copyFrom(state());
    copy.makeMove(m);
    if (! copy.inCheck()) {
      if (checkmate::ImmediateCheckmate::hasCheckmateMove(copy.turn(), copy)
	  || EnterKing::canDeclareWin(copy)) {
	loss.push_back(m);
	continue;
      }
    }
    normal.push_back(m);
  }
}

void osl::game_playing::
GameState::generateNotLosingMoves(MoveVector& normal_or_win_or_draw, 
				  MoveVector& loss) const
{
  MoveVector win, draw;
  generateMoves(normal_or_win_or_draw, win, draw, loss);
  normal_or_win_or_draw.push_back(win.begin(), win.end());
  normal_or_win_or_draw.push_back(draw.begin(), draw.end());
}

bool osl::game_playing::
GameState::rejectByStack(Move move) const
{
  const int max_depth = 8;
  if (move.player() == BLACK)
    return search::MoveStackRejections::probe<BLACK>
      (state(), moveHistory(), std::min(max_depth,moves()), move, 0,
       counter().checkCount(alt(move.player())));
  else
    return search::MoveStackRejections::probe<WHITE>
      (state(), moveHistory(), std::min(max_depth,moves()), move, 0,
       counter().checkCount(alt(move.player())));
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
