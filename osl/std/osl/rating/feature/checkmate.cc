/* checkmate.cc
 */
#include "osl/rating/feature/checkmate.h"
#include "osl/bits/king8Info.h"
#include "osl/effect_util/neighboring8Direct.h"

struct osl::rating::Threatmate::Helper
{
  bool *result;
  NumEffectState *state;
  void operator()(Square) 
  {
    if (state->inCheck(state->turn())
	|| state->inCheck(alt(state->turn()))) {
      *result = false;
      return;
    }
    state->changeTurn();
    *result = ImmediateCheckmate::hasCheckmateMove(state->turn(), *state);
    state->changeTurn();
  }
};

bool osl::rating::
Threatmate::knight2Step(const NumEffectState& state, Move move, Square king) 
{
  if (move.ptype() != KNIGHT)
    return false;
  const int y = king.y() + sign(state.turn())*4;
  if (y != move.to().y())
    return false;
  const int x = move.to().x();
  return (x == king.x() || abs(king.x() - x) == 2);
}
bool osl::rating::
Threatmate::captureForKnightCheck(const NumEffectState& state, Move move, Square king)
{
  const Player defender = alt(state.turn());
  const CArray<Square,2> knight_position = {{
      Board_Table.nextSquare(defender, king, UUR),
      Board_Table.nextSquare(defender, king, UUL)
    }};
  const Piece captured = state.pieceOnBoard(move.to());
  assert(captured.isPiece());
  for (int i=0; i<2; ++i) {
    const Square kp = knight_position[i];
    const Piece p = state.pieceAt(kp);
    if (state.hasEffectNotBy(defender, captured, kp))
      continue;
    if (p.isEmpty()
	&& (unpromote(move.capturePtype()) == KNIGHT
	    || state.hasPieceOnStand<KNIGHT>(state.turn())))
      return true;
    if (p.canMoveOn(state.turn())
	&& state.hasEffectByPtypeStrict<KNIGHT>(state.turn(), kp))
      return true;
  }
  return false;
}

bool osl::rating::Threatmate::isCandidate(const NumEffectState& state, Move move) 
{
  const Player defender = alt(state.turn());
  const Square king = state.kingSquare(defender);
  if (Neighboring8Direct::hasEffectOrAdditional(state, move.ptypeO(), move.to(), king)
      || move.to().isNeighboring8(king)
      || state.longEffectAt(move.to(), alt(state.turn())).any() // todo: refinement
      || (! move.isDrop() && state.longEffectAt(move.from(), state.turn()).any()) // todo: refinement
    )
    return true;
  if (move.capturePtype() != PTYPE_EMPTY
      && Neighboring8Direct::hasEffectOrAdditional(state, move.capturePtypeO(), move.to(), king))
    return true;

  const King8Info info(state.king8Info(defender));
  if (move.capturePtype() != PTYPE_EMPTY
      && (info.dropCandidate()
	  || (info.liberty() == 0 && captureForKnightCheck(state, move, king))))
    return true;
  if (state.inCheck()
      && (info.dropCandidate() || info.moveCandidate2() 
	  || /* only when hand knight or knight effect */info.liberty() == 0))
    return true;
  if (info.liberty() == 0
      && (knight2Step(state, move, king)
	  || (! move.isDrop()
	      && ((state.hasPieceOnStand<KNIGHT>(state.turn())
		   && state.hasEffectIf(newPtypeO(state.turn(),KNIGHT), move.from(), king))
		  || state.hasEffectByPtypeStrict<KNIGHT>(state.turn(), move.from())))))
    return true;
  return false;
}

bool osl::rating::Threatmate::match(const NumEffectState& cstate, Move move, 
				    const RatingEnv&) const
{
  NumEffectState& state = const_cast<NumEffectState&>(cstate);
  if (! isCandidate(cstate, move))
    return false;
  bool result = false;
  Helper helper = { &result, &state };
  state.makeUnmakeMove(move, helper);
#ifdef OSL_DEBUG
  if (result && ! isCandidate(cstate, move))
    std::cerr << cstate << move << "\n", assert(0);
#endif
  return result;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
