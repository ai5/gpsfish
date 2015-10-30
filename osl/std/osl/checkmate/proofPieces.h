/* proofPieces.h
 */
#ifndef _PROOFPIECES_H
#define _PROOFPIECES_H

#include "osl/checkmate/proofPiecesUtil.h"
namespace osl
{
  namespace checkmate
  {
    class CheckMoveList;
    struct ProofPieces
    {
      static const PieceStand leaf(const NumEffectState& state, 
				   Player attacker, const PieceStand max)
      {
	assert(state.turn() != attacker);
	PieceStand result;
	if (! state.inUnblockableCheck(alt(attacker)))
	  ProofPiecesUtil::addMonopolizedPieces(state, attacker, max, result);
	return result;
      }
      static const PieceStand
      attack(const PieceStand prev, Move move, const PieceStand max)
      {
	assert(move.isValid());
	PieceStand result = prev;
	if (move.isDrop())
	{
	  const Ptype ptype = move.ptype();
	  if (result.get(ptype) < max.get(ptype))
	    result.add(ptype);
	}
	else 
	{
	  const Ptype captured = move.capturePtype();
	  if (isPiece(captured))
	  {
	    const Ptype ptype = unpromote(captured);
	    result.trySub(ptype);
	  }
	}
	return result;
      }
      static const PieceStand
      defense(const CheckMoveList& moves, const NumEffectState& state,
	      PieceStand max);
    };
  
  } // namespace checkmate
} // osl

#endif /* _PROOFPIECES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
