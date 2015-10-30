#include "osl/move_generator/escape_.h"
#include "osl/move_generator/escape_.tcc"
#include "osl/move_generator/pieceOnBoard.tcc"
#ifdef NDEBUG
# include "osl/move_generator/capture_.tcc"
#endif
#include "osl/numEffectState.h"

void osl::GenerateEscapeKing::generate(const NumEffectState& state, MoveVector& out)
{
  const size_t first = out.size();
  {
    typedef move_action::Store store_t;
    store_t store(out);
    const Piece king =state.kingPiece(state.turn());
    move_generator::GenerateEscapeOfTurn::generate(state, king, store);
  }
  MoveVector unpromote_moves;
  const size_t last = out.size();
  for (size_t i=first; i<last; ++i)
  {
    if(out[i].hasIgnoredUnpromote())
      unpromote_moves.push_back(out[i].unpromote());
  }
  out.push_back(unpromote_moves.begin(), unpromote_moves.end());
}

void osl::GenerateEscapeKing::generateCheap(const NumEffectState& state, MoveVector& out)
{
  if (state.turn() == BLACK)
    move_generator::GenerateEscape<BLACK>::generateCheapKingEscape(state, out);
  else
    move_generator::GenerateEscape<WHITE>::generateCheapKingEscape(state, out);
}

namespace osl
{
  // explicit template instantiation
  namespace move_generator
  {
    template class Escape<move_action::Store>;
    template void Escape<move_action::Store>::generate<BLACK,true,false>(const NumEffectState& state,Piece piece,move_action::Store& action);
    template void Escape<move_action::Store>::generate<WHITE,true,false>(const NumEffectState& state,Piece piece,move_action::Store& action);
    template void Escape<move_action::Store>::generateMoves<BLACK,true>(NumEffectState const&, Piece, Piece, move_action::Store&);
    template void Escape<move_action::Store>::generateMoves<WHITE,true>(NumEffectState const&, Piece, Piece, move_action::Store&);
    template void Escape<move_action::Store>::generateKingEscape<BLACK, false>(NumEffectState const&, move_action::Store&);
    template void Escape<move_action::Store>::generateKingEscape<WHITE, false>(NumEffectState const&, move_action::Store&);
    template void Escape<move_action::Store>::generateKingEscape<BLACK, true>(NumEffectState const&, move_action::Store&);
    template void Escape<move_action::Store>::generateKingEscape<WHITE, true>(NumEffectState const&, move_action::Store&);
    template void Escape<move_action::Store>::generateBlockingKing<BLACK,false>(const NumEffectState&,Piece,Square,move_action::Store&);
    template void Escape<move_action::Store>::generateBlockingKing<WHITE,false>(const NumEffectState&,Piece,Square,move_action::Store&);
    template void Escape<move_action::Store>::generateBlocking<BLACK,true>(const NumEffectState&,Piece,Square,Square,move_action::Store&);
    template void Escape<move_action::Store>::generateBlocking<WHITE,true>(const NumEffectState&,Piece,Square,Square,move_action::Store&);
  } // namespace move_generator
} // namespace osl
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
