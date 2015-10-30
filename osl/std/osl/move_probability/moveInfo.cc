/* moveInfo.cc
 */
#include "osl/move_probability/moveInfo.h"
#include "osl/move_probability/stateInfo.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/check_.h"
#include "osl/eval/see.h"
#include "osl/eval/minorPiece.h" 
using namespace osl::move_classifier;

osl::move_probability::
MoveInfo::MoveInfo(const StateInfo& info, Move m) 
  : move(m),
    see(See::see(*info.state, move, info.pin[info.state->turn()],
		 info.pin[alt(info.state->turn())])),
    plain_see(see),
    check(PlayerMoveAdaptor<Check>::isMember(*info.state, move)),
    open_check(ConditionAdaptor<OpenCheck>::isMember(*info.state, move)),
    player(m.player()), stand_index_cache(-1)
{
  // ad-hoc adjustment
  if (adhocAdjustBishopFork(info))
    see = 0;
  else if (adhocAdjustSlider(info))
    see = plain_see / 8;
  else if (adhocAdjustBreakThreatmate(info))
    see = 0;
  else if (adhocAdjustAttackCheckmateDefender(info))
    see = 0;
  else if (adhocAdjustKeepCheckmateDefender(info))
    see = 0;
}

bool osl::move_probability::
MoveInfo::adhocAdjustSlider(const StateInfo& info) const
{
  if (plain_see >= 0)
    return false;
  const Piece attack = info.state->findCheapAttack(alt(player), move.to());
  return info.pinByOpposingSliders(attack)
    && (move.isDrop()
	|| ! info.state->hasEffectByPiece(info.state->pieceAt(move.from()),
					 attack.square()));
}

bool osl::move_probability::
MoveInfo::adhocAdjustBishopFork(const StateInfo& info) const
{
  if (plain_see >= 0 
      || !info.state->hasPieceOnStand<BISHOP>(info.state->turn()))
    return false;

  const Piece attack
    = info.state->findCheapAttack(alt(player), move.to());
  if (unpromote(attack.ptype()) == ROOK) {
    const Player defense = alt(info.state->turn());
    const Square king = info.state->kingSquare(defense);
    const Square center
      = eval::ml::BishopRookFork::isBishopForkSquare(*info.state, defense, king, move.to(), true);
    return ! center.isPieceStand();
  }
  return false;
}

bool osl::move_probability::
MoveInfo::adhocAdjustBreakThreatmate(const StateInfo& info) const
{
  if (! info.threatmate_move.isNormal())
    return false;
  
  const Piece attack
    = info.state->findCheapAttack(alt(player), move.to());
  if (attack.isPiece()		// break threatmate by sacrifice
      && info.state->hasEffectByPiece(attack, info.threatmate_move.to()))
    return ! info.state->hasEffectIf(attack.ptypeO(), move.to(),
					 info.threatmate_move.to());
  return false;
}

bool osl::move_probability::
MoveInfo::adhocAdjustAttackCheckmateDefender(const StateInfo& info) const
{
  if (plain_see >= 0)
    return false;
  const Piece defender = info.checkmate_defender[alt(player)].first;
  if (defender.isPiece()
      && info.state->countEffect(alt(player), move.to()) == 1
      && info.state->hasEffectByPiece(defender, move.to()))
    return true;
  return false;
}

bool osl::move_probability::
MoveInfo::adhocAdjustKeepCheckmateDefender(const StateInfo& info) const
{
  if (plain_see <= 0)
    return false;
  const Piece defender = info.checkmate_defender[player].first;
  const Square threat_at = info.checkmate_defender[player].second;
  if (defender.isPiece() && move.from() == defender.square()
      && ! info.state->hasEffectIf(move.ptypeO(), move.to(), threat_at))
    return true;
  return false;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
