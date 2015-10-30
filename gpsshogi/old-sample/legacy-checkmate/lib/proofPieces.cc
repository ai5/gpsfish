/* proofPieces.cc
 */
#include "osl/checkmate/proofPieces.h"
#include "checkMoveList.h"
#include "checkHashRecord.h"

const osl::PieceStand osl::checkmate::
ProofPieces::defense(const CheckMoveList& moves, const NumEffectState& state,
		     PieceStand max)
{
  CheckMoveList::const_iterator p=moves.begin();
  assert(p != moves.end());
  assert(p->record);
  PieceStand result = p->record->proofPieces();
  ++p;
  for (; p!=moves.end(); ++p)
  {
    assert(p->record);
    result = result.max(p->record->proofPieces());
  }
  const Player defender = state.turn();
  if (! effect_util::UnblockableCheck::isMember(defender, state))
    ProofPiecesUtil::addMonopolizedPieces(state, alt(defender), max,
					  result);
  return result;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
