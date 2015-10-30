/* historyState.h
 */
#ifndef _HISTORYSTATE_H
#define _HISTORYSTATE_H
#include "osl/numEffectState.h"
#include <vector>
namespace osl
{
  namespace state
  {
    class HistoryState
#if OSL_WORDSIZE == 32
      : public misc::Align16New
#endif
    {
      NumEffectState initial_state;
      mutable NumEffectState current;
      mutable bool dirty;
      std::vector<Move> moves;
    public:
      HistoryState();
      explicit HistoryState(const SimpleState& initial);
      ~HistoryState();

      void setRoot(const SimpleState&);
      void makeMove(Move move);
      void unmakeMove();

      void makeMovePass();
      void unmakeMovePass();
      
      const NumEffectState& state() const {
	if (dirty)
	  update();
	return current; 
      }
      operator const NumEffectState& () const { return state(); }
      const NumEffectState& initialState() const { return initial_state; }
      bool empty() const { return moves.empty(); }
      const std::vector<Move>& history() const { return moves; }
      bool isConsistent() const { return state().isConsistent(); }
    private:
      void update() const;
    };
    class DoUndoMoveLock 
    {
      HistoryState& state;
    public:
      DoUndoMoveLock(HistoryState& s, Move move) : state(s)
      {
	state.makeMove(move);
      }
      ~DoUndoMoveLock() 
      {
	state.unmakeMove();
      }
    };
  }
  using state::HistoryState;
  using state::DoUndoMoveLock;
}


#endif /* _HISTORYSTATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
