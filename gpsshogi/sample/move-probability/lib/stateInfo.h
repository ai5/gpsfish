/* stateInfo.h
 */
#ifndef GPSSHOGI_MOVE_PROBABILITY_STATEINFO_H
#define GPSSHOGI_MOVE_PROBABILITY_STATEINFO_H

#include "osl/move_probability/pinnedGeneral.h"
#include "osl/numEffectState.h"
#include "osl/progress.h"
#include "osl/bits/king8Info.h"
#include "osl/container/square8.h"
#include <algorithm>
#include <vector>

namespace gpsshogi
{
  using namespace osl;
  using move_probability::PinnedGeneral;
  struct StateInfo
  {
    const NumEffectState& state;
    const std::vector<Move>& moves;
    PieceVector pin_by_opposing_sliders, king8_long_pieces;
    int last_move;
    CArray<Piece,2> threatened;
    typedef FixedCapacityVector<int,8> long_attack_t;
    CArray2d<long_attack_t,40,8> long_attack_cache; // BlockLong
    typedef CArray<int,16> pattern_square_t;
    CArray<pattern_square_t,Square::SIZE> pattern_cache;
    CArray2d<bool,40,2> attack_shadow;
    progress::ml::NewProgress progress;
    PieceMask last_add_effect;
    Ptype last_move_ptype5;
    CArray<PieceMask,2> pin;
    Move threatmate_move;
    Square8 sendoffs;
    typedef FixedCapacityVector<PinnedGeneral,64> pinned_gs_t;
    CArray<pinned_gs_t,2> exchange_pins;
    CArray<bool,2> move_candidate_exists; // king => bool
    CArray<std::pair<Piece,Square>,2> checkmate_defender;
    unsigned int possible_threatmate_ptype;
    mutable NumEffectState copy;
    CArray<Move,2> bookmove;
    bool use_adhoc_adjust;
    
    StateInfo(const NumEffectState& s, const std::vector<Move>& m, int l)
      : state(s), moves(m), last_move(l), progress(s),
	possible_threatmate_ptype(0), use_adhoc_adjust(false)
    {
      setThreatmateAuto();
      updateAfterMove();
    }
    void update(Move moved);
    
    bool pinByOpposingSliders(Piece p) const 
    {
      return std::find(pin_by_opposing_sliders.begin(), pin_by_opposing_sliders.end(),
		       p) != pin_by_opposing_sliders.end();
    }
    King8Info king8Info(Player pl) const
    {
      return King8Info(state.Iking8Info(pl));
    }
    int progress8() const
    {
      return progress.progress16().value()/2;
    }
    static std::pair<Piece,Square> findCheckmateDefender(const NumEffectState& state, Player king);
  private:
    void updateAfterMove();
    void makePinOfLongPieces();
    void makeLongAttacks();
    void updatePinnedGenerals(Player owner);
    void setThreatmateAuto();
  };
  bool operator==(const StateInfo& l, const StateInfo& r);
}

#endif /* GPSSHOGI_MOVE_PROBABILITY_STATEINFO_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
