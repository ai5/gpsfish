#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/pieceOnBoard.tcc"

namespace osl
{
  namespace move_generator
  {
    // explicit template instantiation
    template void PieceOnBoard<move_action::Store>::generate<BLACK,false>(const NumEffectState&,Piece,move_action::Store&,int);
    template void PieceOnBoard<move_action::Store>::generate<WHITE,false>(const NumEffectState&,Piece,move_action::Store&,int);
    template void PieceOnBoard<move_action::Store>::generatePtype<BLACK,KING,false>(const NumEffectState&,Piece,move_action::Store&,int);
    template void PieceOnBoard<move_action::Store>::generatePtype<WHITE,KING,false>(const NumEffectState&,Piece,move_action::Store&,int);

    template void PieceOnBoard<move_action::Store,true>::generate<BLACK,true>(const NumEffectState&,Piece,move_action::Store&,int);
    template void PieceOnBoard<move_action::Store,true>::generate<WHITE,true>(const NumEffectState&,Piece,move_action::Store&,int);
  } // namespace move_generator
} // namespace osl

void osl::move_generator::GeneratePieceOnBoard::
generate(Player turn, const NumEffectState& state, Piece target, MoveVector& out)
{
  move_action::Store store(out);
  if (turn == BLACK)
    PieceOnBoard<move_action::Store>::generate<BLACK,false>(state, target, store);
  else
    PieceOnBoard<move_action::Store>::generate<WHITE,false>(state, target, store);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
