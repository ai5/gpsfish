#ifndef _GENERATE_CAPTURE_EFFECT_TO_AROUND_KING8_H
#define _GENERATE_CAPTURE_EFFECT_TO_AROUND_KING8_H
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_action/captureFrom.h"
#include "osl/move_action/store.h"
#include "osl/centering3x3.h"
#include "osl/state/numEffectState.h"
#include "osl/container/moveVector.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * Capture pieces that has effect to squares around King and to the King.
     * - TODO should be Capture Effecto To Around King 9?
     * - Using centering3x3, i.e. if the King is on the edge,
     *   the area is move towards the center of the board.
     */
    template <Player P>
    struct CaptureEffectToAroundKing8
    {
      template <class Action>
      static void generateTo(const NumEffectState& state,
			     Square p,
			     Action& action)
      {
	typedef move_action::CaptureFrom<P, Action>
	  capture_action;
	capture_action capture(state, action);
	  
	if (p.isEdge()) return;
	assert(p.isOnBoard());

	state.template forEachEffect<PlayerTraits<P>::opponent,
	  capture_action>(p, capture);
      }

      static void generate(const NumEffectState& state, MoveVector& moves)
      {
	const Square position_king = 
	  Centering3x3::adjustCenter(state.template kingSquare<P>());
	{
	move_action::Store action(moves);
	generateTo(state,
		   position_king,
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(UL),
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(U),
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(UR),
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(L),
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(R),
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(DL),
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(D),
		   action);
	generateTo(state,
		   position_king + Board_Table.getOffsetForBlack(DR),
		   action);
	}
	moves.unique();
      }
      
    };
  }
} // namespace osl

#endif /* _GENERATE_CAPTURE_EFFECT_TO_AROUND_KING8_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
