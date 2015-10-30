/* moveInfo.h
 */
#ifndef OSL_MOVE_PROBABILITY_MOVEINFO_H
#define OSL_MOVE_PROBABILITY_MOVEINFO_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace move_probability
  {
    struct StateInfo;
    struct MoveInfo
    {
      Move move;
      int see, plain_see;
      bool check, open_check;
      Player player;
      mutable int stand_index_cache;
      MoveInfo(const StateInfo&, Move);

      int standIndex(const NumEffectState& state) const
      {
	if (stand_index_cache < 0) {
	  stand_index_cache = 0;
	  assert(PieceStand::order[6] == PAWN);
	  for (size_t i=0; i+1<PieceStand::order.size(); ++i) {
	    Ptype ptype = PieceStand::order[i];
	    int count = state.countPiecesOnStand(player, ptype);
	    if (move.isDrop() && ptype == move.ptype())
	      --count;
	    stand_index_cache = stand_index_cache * 2 + (count > 0);
	  }
	}
	return stand_index_cache;
      }
      bool adhocAdjustSlider(const StateInfo&) const;
      bool adhocAdjustBishopFork(const StateInfo&) const;
      bool adhocAdjustBreakThreatmate(const StateInfo&) const;
      bool adhocAdjustAttackCheckmateDefender(const StateInfo&) const;
      bool adhocAdjustKeepCheckmateDefender(const StateInfo&) const;
    };
  }
}

#endif /* OSL_MOVE_PROBABILITY_MOVEINFO_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
