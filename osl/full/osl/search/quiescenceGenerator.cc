/* quiescenceGenerator.cc
 */
#include "osl/search/quiescenceGenerator.h"
#include "osl/search/quiescenceGenerator.tcc"
#include "osl/move_generator/promote_.tcc"
#include "osl/move_generator/pieceOnBoard.tcc"
#include "osl/eval/openMidEndingEval.h"
#include "osl/eval/progressEval.h"

namespace osl
{
  namespace search
  {
    template struct QuiescenceGenerator<BLACK>;
    template struct QuiescenceGenerator<WHITE>;

    template void QuiescenceGenerator<BLACK>::capture<PAWN, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<LANCE, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<KNIGHT, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<SILVER, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<GOLD, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<BISHOP, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<ROOK, true>(const NumEffectState&, MoveVector&, Piece);

    template void QuiescenceGenerator<WHITE>::capture<PAWN, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<LANCE, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<KNIGHT, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<SILVER, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<GOLD, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<BISHOP, true>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<ROOK, true>(const NumEffectState&, MoveVector&, Piece);

    template void QuiescenceGenerator<BLACK>::capture<PAWN, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<LANCE, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<KNIGHT, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<SILVER, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<GOLD, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<BISHOP, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<BLACK>::capture<ROOK, false>(const NumEffectState&, MoveVector&, Piece);

    template void QuiescenceGenerator<WHITE>::capture<PAWN, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<LANCE, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<KNIGHT, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<SILVER, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<GOLD, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<BISHOP, false>(const NumEffectState&, MoveVector&, Piece);
    template void QuiescenceGenerator<WHITE>::capture<ROOK, false>(const NumEffectState&, MoveVector&, Piece);
#ifndef MINIMAL
    template void QuiescenceGenerator<BLACK>::escapeFromLastMove<PieceEval>(const NumEffectState&, Move, MoveVector&);
    template void QuiescenceGenerator<WHITE>::escapeFromLastMove<PieceEval>(const NumEffectState&, Move, MoveVector&);

    template void QuiescenceGenerator<BLACK>::escapeFromLastMove<eval::ProgressEval>(const NumEffectState&, Move, MoveVector&);
    template void QuiescenceGenerator<WHITE>::escapeFromLastMove<eval::ProgressEval>(const NumEffectState&, Move, MoveVector&);
#endif
    template void QuiescenceGenerator<BLACK>::escapeFromLastMove<eval::ml::OpenMidEndingEval>(const NumEffectState&, Move, MoveVector&);
    template void QuiescenceGenerator<WHITE>::escapeFromLastMove<eval::ml::OpenMidEndingEval>(const NumEffectState&, Move, MoveVector&);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
