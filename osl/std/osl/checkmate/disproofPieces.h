/* disproofPieces.h
 */
#ifndef _DISPROOFPIECES_H
#define _DISPROOFPIECES_H

#include "osl/checkmate/proofPiecesUtil.h"
namespace osl
{
  namespace checkmate
  {
    class CheckMoveList;
    struct DisproofPieces
    {
      static const PieceStand leaf(const SimpleState& state, Player defender,
				   const PieceStand max)
      {
	assert(state.turn() != defender);
	PieceStand result;
	ProofPiecesUtil::addMonopolizedPieces(state, defender, max, result);
	return result;
      }
      static const PieceStand
      defense(const PieceStand prev, Move move, const PieceStand max)
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
      attack(const CheckMoveList& moves, const SimpleState& state, 
	     PieceStand max);
    };
  
  } // namespace checkmate
} // osl

#endif /* _DISPROOFPIECES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
