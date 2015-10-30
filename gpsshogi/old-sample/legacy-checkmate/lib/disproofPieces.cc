/* disproofPieces.cc
 */
#include "osl/checkmate/disproofPieces.h"
#include "checkMoveList.h"
#include "checkHashRecord.h"

const osl::PieceStand osl::checkmate::
DisproofPieces::attack(const CheckMoveList& moves, const SimpleState& state,
		       const PieceStand max)
{
  PieceStand result = PieceStand();
  for (CheckMoveList::const_iterator p=moves.begin();
       p!=moves.end(); ++p)
  {
    assert(p->record || p->flags.isSet(MoveFlags::NoPromote));
    if (p->record)
    {
      if (! p->record->proofDisproof().isCheckmateFail())
      {
	// 打歩詰の場合
	assert((p->record->proofDisproof() == ProofDisproof::NoEscape())
	       || p->flags.isSet(MoveFlags::NoPromote));
	continue;
      }
      check_assert(p->record->hasDisproofPieces()
		   || (p->record->dump(2),0));
      result = result.max(p->record->disproofPieces());
    }
  }
  ProofPiecesUtil::addMonopolizedPieces(state, alt(state.turn()), max, 
					result);
  return result;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
