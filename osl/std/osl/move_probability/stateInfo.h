/* stateInfo.h
 */
#ifndef OSL_MOVE_PROBABILITY_STATEINFO_H
#define OSL_MOVE_PROBABILITY_STATEINFO_H

#include "osl/move_probability/pinnedGeneral.h"
#include "osl/numEffectState.h"
#include "osl/bits/king8Info.h"
#include "osl/progress.h"
#include "osl/container.h"
#include "osl/container/square8.h"
#include "osl/container/moveStack.h"
#include <vector>
#include <algorithm>

namespace osl
{
  namespace move_probability
  {
    struct StateInfo
    {
      const NumEffectState *state;
      const MoveStack *history;
      Progress16 progress16;
      PieceVector pin_by_opposing_sliders, king8_long_pieces;
      CArray<Piece,2> threatened;
      typedef FixedCapacityVector<int,8> long_attack_t;
      CArray2d<long_attack_t,40,8> long_attack_cache; // BlockLong
      typedef CArray<int,16> pattern_square_t;
      CArray<pattern_square_t,Square::SIZE> pattern_cache;
      CArray2d<bool,40,2> attack_shadow;
      PieceMask last_add_effect;
      Ptype last_move_ptype5;
      CArray<PieceMask,2> pin;
      Move threatmate_move;
      Square8 sendoffs;
      typedef FixedCapacityVector<PinnedGeneral,64> pinned_gs_t;
      CArray<pinned_gs_t,2> exchange_pins;
      CArray<bool,2> move_candidate_exists; // king => bool
      mutable NumEffectState copy;
      BoardMask changed_effects;
      CArray<std::pair<Piece,Square>,2> checkmate_defender;
      unsigned int possible_threatmate_ptype;
      CArray<Move,2> bookmove;
      bool dirty;

      StateInfo() : state(0), history(0), progress16(0), dirty(true)
      {
      }
      StateInfo(const NumEffectState& s, Progress16 p, const MoveStack& h,
		Move t=Move())
	: state(&s), history(&h), progress16(p), dirty(true)
      {
	clearOldCache();
	threatmate_move = t;
	finishUpdate();
      }
      void reset0(const NumEffectState& s, Progress16 p)
      {
	dirty = true;
	state = &s;
	progress16 = p;
	pin_by_opposing_sliders.clear();  
	king8_long_pieces.clear();
	long_attack_cache.fill(long_attack_t());
	clearOldCache();
      }
      void reset1(const MoveStack& h)
      {
	history = &h;
      }
      void finishUpdate();
      void reset(const NumEffectState& s, Progress16 p,
		 const MoveStack& h, Move threatmate_move=Move())
      {
	reset0(s, p);
	reset1(h);
	setThreatmate(threatmate_move);
	finishUpdate();
      }
      void setThreatmate(Move move) { threatmate_move = move; }
    
      bool pinByOpposingSliders(Piece p) const 
      {
	return std::find(pin_by_opposing_sliders.begin(), pin_by_opposing_sliders.end(),
			 p) != pin_by_opposing_sliders.end();
      }
      King8Info king8Info(Player pl) const
      {
	return King8Info(state->Iking8Info(pl));
      }
      int progress8() const { return progress16.value()/2; }
      static std::pair<Piece,Square> findCheckmateDefender(const NumEffectState& state, Player king);
      static Move findShortThreatmate(const NumEffectState&, Move last_move);
    private:
      void clearOldCache();
      void updateDelayed();
      void makePinOfLongPieces();
      void makeLongAttacks();
      void updatePinnedGenerals(Player owner);
    };
    bool operator==(const StateInfo& l, const StateInfo& r);
  }
}

#endif /* OSL_MOVE_PROBABILITY_STATEINFO_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
