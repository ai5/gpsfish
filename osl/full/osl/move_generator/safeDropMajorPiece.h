#ifndef OSL_SAFE_DROP_MAJOR_PIECE_H
#define OSL_SAFE_DROP_MAJOR_PIECE_H
#include "osl/numEffectState.h"
#include "osl/move_generator/move_action.h"

namespace osl
{
  namespace move_generator
  {
    template <Player P>
    struct SafeDropMajorPiece
    {      
      template <class Action>
      static void generate(const NumEffectState& state, Action& action)
      {
	const bool has_bishop = state.template hasPieceOnStand<BISHOP>(P);
	const bool has_rook = state.template hasPieceOnStand<ROOK>(P);

	if (!has_rook && !has_bishop)
	  return;

	int start_y;
	if (P == BLACK)
	  start_y = 1;
	else
	  start_y = 7;
	for (int x = 1; x <= 9; x++)
	{
	  for (int y = start_y; y < start_y + 3; y++)
	  {
	    Square position(x, y);
	    if (state.pieceOnBoard(position).isEmpty()
		&& !state.hasEffectAt(alt(P), position))
	    {
	      if (has_rook)
	      {
		action.dropMove(position, ROOK, P);
	      }
	      if (has_bishop)
	      {
		action.dropMove(position, BISHOP, P);
	      }
	    }
	  }
	}
      }
      template <size_t Capacity>
      static void generateMoves(const NumEffectState& state, 
				FixedCapacityVector<Move,Capacity>& out)
      {
	move_action::Store store(out);
	generate(state, store);
      }
    };
  }
} // namespace osl

#endif /* _GENERATE_SAFE_DROP_MAJOR_PIECE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
