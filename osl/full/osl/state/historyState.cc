/* historyState.cc
 */
#include "osl/state/historyState.h"

osl::state::HistoryState::HistoryState()
  : dirty(false)
{
  assert(current.isConsistent());
  assert(initial_state.isConsistent());
}

osl::state::HistoryState::HistoryState(const SimpleState& initial)
  : initial_state(initial), current(initial), dirty(false)
{
  assert(current.isConsistent());
  assert(initial_state.isConsistent());
}

osl::state::HistoryState::~HistoryState()
{
}

void osl::state::HistoryState::setRoot(const SimpleState& initial)
{
  initial_state = current = NumEffectState(initial);
  moves.clear();
  dirty = false;
}

void osl::state::HistoryState::makeMove(Move move)
{
  if (dirty)
    update();
  moves.push_back(move);
  current.makeMove(move);
}

void osl::state::HistoryState::unmakeMove()
{
  dirty = true;
  moves.pop_back();
}

void osl::state::HistoryState::makeMovePass()
{
  makeMove(Move::PASS(state().turn()));
}

void osl::state::HistoryState::unmakeMovePass()
{
  assert(! moves.empty() && moves.back().isPass());
  if (! dirty) {
    moves.pop_back();
    current.changeTurn();
    return;
  }
  unmakeMove();
}

void osl::state::HistoryState::update() const
{
  current = initial_state;
  for (size_t i=0; i<moves.size(); ++i)
    current.makeMove(moves[i]);
  dirty = false;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
