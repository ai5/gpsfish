/* oracleAdjust.h
 */
#ifndef _ORACLEADUST_H
#define _ORACLEADUST_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace checkmate
  {
    struct OracleAdjust
    {
      static const Move attack(const NumEffectState& state, Move check_move) 
      {
	assert(check_move.isValid());
	if (! check_move.isDrop())
	{
	  // capture
	  {
	    const Piece p=state.pieceOnBoard(check_move.to());
	    if (p.isPtype<KING>())
	      return Move();
	    check_move=check_move.newCapture(p);
	  }

	  // from
	  if (state.pieceOnBoard(check_move.from()).ptype() != check_move.oldPtype()
	      && Ptype_Table.hasLongMove(check_move.ptype())) {
	    Piece p;
	    switch (unpromote(check_move.ptype())) {
	    case ROOK:
	    {
	      mask_t m = state.allEffectAt<ROOK>(check_move.player(), check_move.to());
	      while (m.any()) {
		const int num = m.takeOneBit()+PtypeFuns<ROOK>::indexNum*32;
		p = state.pieceOf(num);
		if (Board_Table.getShortOffsetNotKnight(Offset32(check_move.to(), check_move.from()))
		    == Board_Table.getShortOffsetNotKnight(Offset32(check_move.to(), p.square())))
		  break;
	      }
	      break;
	    }
	    case BISHOP: 
	    {
	      mask_t m = state.allEffectAt<BISHOP>(check_move.player(), check_move.to());
	      while (m.any()) {
		const int num = m.takeOneBit()+PtypeFuns<BISHOP>::indexNum*32;
		p = state.pieceOf(num);
		if (Board_Table.getShortOffsetNotKnight(Offset32(check_move.to(), check_move.from()))
		    == Board_Table.getShortOffsetNotKnight(Offset32(check_move.to(), p.square())))
		  break;
	      }
	      break;
	    }
	    case LANCE:  p = state.findAttackAt<LANCE>(check_move.player(), check_move.to()); 
	      break;
	    default:
	      assert(0);
	    }
	    if (p.isPiece()) {
	      if (check_move.oldPtype() == p.ptype())
		check_move=check_move.newFrom(p.square());
	      else if (check_move.ptype() == p.ptype())
		check_move = Move(p.square(), check_move.to(), check_move.ptype(), 
				  check_move.capturePtype(), false, check_move.player());
	      if (! state.isValidMoveByRule(check_move, false))
		return Move();
	    }
	  }
	}
	if (! state.isAlmostValidMove<false>(check_move))
	  return Move();
	return check_move;
      }
    };
  }
}


#endif /* _ORACLEADUST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
