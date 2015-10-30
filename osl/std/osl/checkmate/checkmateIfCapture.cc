/* checkmateIfCapture.cc
 */
#include "osl/checkmate/checkmateIfCapture.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/move_generator/capture_.h"
#include "osl/effect_util/neighboring8Direct.h"

struct osl::checkmate::CheckmateIfCapture::CallDefense
{
  NumEffectState *state;
  int depth;
  bool result;
  void operator()(Square last_to)
  {
    result = cannotCapture(*state, last_to, depth);
  }
};

bool osl::checkmate::
CheckmateIfCapture::effectiveAttackCandidate0(const NumEffectState& state, Move move)
{
  using namespace move_classifier;
  // depth 0 専用の枝刈
  const Player attacker = state.turn();
  const Player defender = alt(attacker);
  const Square king = state.kingSquare(defender);
  PieceMask pieces = state.effectSetAt(move.to())
    & state.piecesOnBoard(defender);
  if (pieces.none())
    return false;
  if (move.to().isNeighboring8(king))
    return true;
  const Piece captured = state.pieceOnBoard(move.to());
  if (move.isCapture()) {
    if (Neighboring8Direct::hasEffect(state, captured.ptypeO(), 
				      move.to(), king))
      return true;
  }
  if (! move.isDrop()
      && (state.longEffectAt(move.from(), attacker).any() // todo: refinement 開き王手pinとか8近傍くらい?
	  || (move.from().isNeighboring8(king)
	      && state.hasEffectAt(attacker, move.from()))))
    return true;

  const King8Info info = state.king8Info(defender);
  const CArray<Square,2> knight_position = {{
      Board_Table.nextSquare(defender, king, UUR),
      Board_Table.nextSquare(defender, king, UUL)
    }};
  if (state.inCheck()
      && (info.dropCandidate() || info.moveCandidate2()
	  || /* only when has knight or knight effect */info.liberty() == 0))
    return true;
  if (move.isCapture()) {
    if (info.dropCandidate())
      return true;
    if (info.liberty() == 0) {
      for (int i=0; i<2; ++i) {
	const Square kp = knight_position[i];
	const Piece kpp = state.pieceAt(kp);
	if (kpp.isEdge() || state.hasEffectNotBy(defender, captured, kp))
	  continue;
	if (kpp.isEmpty()
	    && unpromote(move.capturePtype()) == KNIGHT)
	  return true;
	if (state.hasEffectByPiece(captured, kp)
	    && (unpromote(move.capturePtype()) == KNIGHT
		|| state.hasPieceOnStand<KNIGHT>(attacker)
		|| state.hasEffectByPtypeStrict<KNIGHT>(attacker, kp)))
	  return true;
      }
    }
  } else if (info.liberty() == 0 && state.hasPieceOnStand<KNIGHT>(attacker)) {
      for (int i=0; i<2; ++i) {
	const Square kp = knight_position[i];
	const Piece kpp = state.pieceAt(kp);
	if (! kpp.isOnBoardByOwner(defender))
	  continue;
	if (state.hasEffectByPiece(kpp, move.to()))
	  return true;
      }
  }
  // テストでは出てこないが焦点もあるか?
  while (pieces.any())
  {
    const Piece p=state.pieceOf(pieces.takeOneBit());
    if (Neighboring8Direct::hasEffectOrAdditional(state, p.ptypeO(), p.square(), king)
	|| p.square().isNeighboring8(king))
      continue;		// i.e., need analyses
    if (state.longEffectAt(p.square(), attacker).any()) // todo: refinement
      continue;
    if (info.liberty() == 0) {
      int i=0;
      for (; i<2; ++i) {
	const Square kp = knight_position[i];
	const Piece kpp = state.pieceAt(kp);
	if (kpp.isEdge() || state.hasEffectNotBy(defender, p, kp))
	  continue;
	if (p.square() == kp
	    && state.hasPieceOnStand<KNIGHT>(attacker))
	  break;
	if (state.countEffect(defender, kp) == 1)
	  if ((kpp.canMoveOn(attacker)
	       && state.hasEffectByPtypeStrict<KNIGHT>(attacker, kp))
	      || (kpp.isEmpty()
		  && state.hasPieceOnStand<KNIGHT>(attacker)))
	    break;
      }
      if (i<2)
	continue;
    }
    // now we have safe takeback
    return false;
  }
  return true;
}

bool osl::checkmate::
CheckmateIfCapture::effectiveAttack(NumEffectState& state, Move move, int depth)
{
  assert(move.player() == state.turn());
  CallDefense defense = { &state, depth, false };
  state.makeUnmakeMove(move, defense);
#ifdef OSL_DEBUG
  if (defense.result && ! effectiveAttackCandidate0(state, move))
    std::cerr << state << move << "\n", assert(0);
#endif
  return defense.result;
}

bool osl::checkmate::
CheckmateIfCapture::cannotCapture(NumEffectState& state, 
				  Square last_to, int depth)
{
  if (state.inCheck(alt(state.turn())))
    return false;		// 前の手が自殺

  using namespace move_generator;
  using namespace move_action;
  MoveVector moves;		// may contain unsafe move
  GenerateCapture::generate(state, last_to, moves);

  if (moves.empty())
    return false;

  FixedDepthSearcher searcher(state);
  const Square king = state.kingSquare(state.turn());
  for (MoveVector::const_iterator p=moves.begin(); p!=moves.end(); ++p)
  {
    if (state.inCheck()) {
      if (state.countEffect(alt(state.turn()), king) > 1
	  || ! state.hasEffectByPiece(state.pieceOnBoard(last_to), king))
	if (p->ptype() != KING)
	  continue;
    }
    const bool checkmate
      = searcher.hasEscapeByMoveOfTurn(*p, depth).isCheckmateSuccess();
    if (! checkmate)
      return false;
  }

  return true;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
