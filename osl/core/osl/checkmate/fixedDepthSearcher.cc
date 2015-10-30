/* fixedDepthSearcher.cc
 */
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/move_generator/addEffectWithEffect.tcc"
#include "osl/numEffectState.tcc"

const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
hasCheckmateMoveOfTurn(int depth, Move& best_move)
{
  if (state->turn() == BLACK)
    return hasCheckmateMove<BLACK>(depth, best_move);
  else
    return hasCheckmateMove<WHITE>(depth, best_move);
}

const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
hasEscapeMoveOfTurn(Move last_move, int depth)
{
  if (state->turn() == BLACK)
    return hasEscapeMove<BLACK>(last_move, depth);
  else
    return hasEscapeMove<WHITE>(last_move, depth);
}

const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSearcher::
hasEscapeByMoveOfTurn(Move next_move, int depth)
{
  if (state->turn() == BLACK)
    return hasEscapeByMove<WHITE>(next_move, depth);
  else
    return hasEscapeByMove<BLACK>(next_move, depth);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
