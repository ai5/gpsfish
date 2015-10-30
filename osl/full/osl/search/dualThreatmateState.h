/* dualThreatmateState.h
 */
#ifndef OSL_SEARCH__DUALTHREATMATESTATE_H
#define OSL_SEARCH__DUALTHREATMATESTATE_H

#include "osl/search/threatmateState.h"
#include "osl/effect_util/sendOffSquare.h"
#include <iosfwd>

namespace osl
{
  namespace search
  {
    class DualThreatmateState
    {
      CArray<Move,2> threatmate_move;
      CArray<ThreatmateState,2> king_status;
      ThreatmateState& wstatus(Player king)
      {
	return king_status[king];
      }
    public:
      /** XXX: QuiescenceRecord への詰め込みのため */
      mutable SendOffSquare::SendOff8 sendoffs;
      struct Flags {
#ifdef OSL_SMP
      volatile
#endif
	bool is_king_in_check:4;
#ifdef OSL_SMP
      volatile
#endif
	char static_value_type:4;
      } flags;
      explicit DualThreatmateState(ThreatmateState::Status b=ThreatmateState::UNKNOWN,
				   ThreatmateState::Status w=ThreatmateState::UNKNOWN) 
	: sendoffs(SendOffSquare::invalidData())
      {
	wstatus(BLACK) = b;
	wstatus(WHITE) = w;
	flags.is_king_in_check = false;
	flags.static_value_type = 0;
      }
      const ThreatmateState& status(Player king) const
      {
	return king_status[king];
      }
      void setThreatmate(Player king, Move m) { 
	assert(m.isNormal());
	wstatus(king).setThreatmate(ThreatmateState::THREATMATE);
	threatmate_move[king] = m;
      }

      bool isThreatmate(Player king) const { 
	return status(king).isThreatmate();
      }
      const Move threatmateMove(Player king) const { 
	return threatmate_move[king];
      }
      bool maybeThreatmate(Player king) const { 
	return status(king).maybeThreatmate();
      }
      bool mayHaveCheckmate(Player king) const {
	return status(king).mayHaveCheckmate();
      }
      void updateInLock(Player turn, const DualThreatmateState *parent, bool in_check)
      {
	if (parent)
	{
	  if (! maybeThreatmate(turn))
	    wstatus(turn).update(&parent->status(turn), in_check);
	  if (! mayHaveCheckmate(alt(turn)))
	    wstatus(alt(turn)).update(&parent->status(alt(turn)), in_check);
	}
      }
    };
    std::ostream& operator<<(std::ostream&, DualThreatmateState);
  }
} // namespace osl

#endif /* OSL_SEARCH__DUALTHREATMATESTATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
