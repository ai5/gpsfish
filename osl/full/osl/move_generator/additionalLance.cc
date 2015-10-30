/* additionalLance.cc
 */
#include "osl/move_generator/additionalLance.h"


template <osl::Player P>
void osl::move_generator::AdditionalLance<P>::
generate(const NumEffectState& state, Square pawn, MoveVector& out)
{
  assert((state.hasPieceOnStand<LANCE>(P)));
  assert(state.pieceOnBoard(pawn).ptype() == PAWN);
  Square back_position = pawn + DirectionPlayerTraits<D,P>::offset();
  Piece target = state.pieceAt(back_position);
  while (target.isEmpty())
  {
    if (state.hasEffectAt<alt(P)>(back_position))
      break;
    out.push_back(Move(back_position, LANCE, P));
	  
    back_position = back_position + DirectionPlayerTraits<D,P>::offset();
    target = state.pieceAt(back_position);
  }     
}

template <osl::Player P>
void osl::move_generator::AdditionalLance<P>::
generateIfHasLance(const NumEffectState& state, Square pawn, 
		   MoveVector& out)
{
  if (state.hasPieceOnStand<LANCE>(P))
    generate(state, pawn, out);
}
namespace osl
{
  namespace move_generator
  {
    template struct AdditionalLance<BLACK>;
    template struct AdditionalLance<WHITE>;
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
