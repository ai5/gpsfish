#include "osl/checkmate/fixedDepthSolverExt.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/checkmate/proofPieces.h"
#include "osl/checkmate/proofNumberTable.h"

struct osl::checkmate::FixedDepthSolverExt::SetProofPieces
{
  static void setAttackLeaf(Move best_move, PieceStand& proof_pieces) {
    proof_pieces = PieceStand();
    if (best_move.isDrop())
      proof_pieces.add(best_move.ptype());
  }
  static void attack(Move best_move, PieceStand stand, PieceStand& proof_pieces) {
    proof_pieces = ProofPieces::attack(proof_pieces, best_move, stand);
  }
  static void setLeaf(const NumEffectState& state, Player P, PieceStand stand, PieceStand& proof_pieces) {
    proof_pieces = ProofPieces::leaf(state, P, stand);
  }
  static void clear(PieceStand& proof_pieces) {
    proof_pieces = PieceStand();
  }
  static void updateMax(PieceStand child, PieceStand& proof_pieces) {
    proof_pieces = proof_pieces.max(child);
  }
  static void addMonopolizedPieces(const NumEffectState& state, Player P, 
				   PieceStand stand, PieceStand& proof_pieces) {
    ProofPiecesUtil::addMonopolizedPieces(state, P, stand, proof_pieces);
  }
  static ProofDisproof attackEstimation(const NumEffectState& state, Player P, King8Info info) {
    auto target_king = state.kingSquare(alt(P));
    const King8Info info_modified 
      = Edge_Table.resetEdgeFromLiberty(alt(P), target_king, info);
    return Proof_Number_Table.attackEstimation(state, P, info_modified, target_king);
  }
};

osl::checkmate::FixedDepthSolverExt::FixedDepthSolverExt()
{
}

osl::checkmate::FixedDepthSolverExt::
FixedDepthSolverExt(NumEffectState& s) : FixedDepthSearcher(s)
{
}

template <osl::Player P>
const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSolverExt::
hasEscapeByMove(Move next_move, int depth, Move& check_move,
		PieceStand& proof_pieces)
{
  typedef FixedDefenseHelper<P,SetProofPieces,true> helper_t;
  proof_pieces = PieceStand();
  ProofDisproof pdp;
  helper_t helper(*this, depth+1, pdp, proof_pieces);
  state->makeUnmakeMove(Player2Type<alt(P)>(),next_move,helper);
  check_move = helper.best_move;
  return pdp;
}

const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSolverExt::
hasCheckmateMoveOfTurn(int depth, Move& best_move, PieceStand& proof_pieces)
{
  if (state->turn() == BLACK)
    return hasCheckmateMove<BLACK>(depth, best_move, proof_pieces);
  else
    return hasCheckmateMove<WHITE>(depth, best_move, proof_pieces);
}

const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSolverExt::
hasCheckmateWithGuideOfTurn(int depth, Move& guide, PieceStand& proof_pieces)
{
  if (state->turn() == BLACK)
    return hasCheckmateWithGuide<BLACK>(depth, guide, proof_pieces);
  else
    return hasCheckmateWithGuide<WHITE>(depth, guide, proof_pieces);
}

const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSolverExt::
hasEscapeByMoveOfTurn(Move next_move, int depth, 
		      Move& check_move, PieceStand& proof_pieces)
{
  if (state->turn() == BLACK)
    return hasEscapeByMove<WHITE>(next_move, depth, check_move, proof_pieces);
  else
    return hasEscapeByMove<BLACK>(next_move, depth, check_move, proof_pieces);
}

template <osl::Player P>
const osl::checkmate::ProofDisproof 
osl::checkmate::FixedDepthSolverExt::
hasCheckmateWithGuide(int depth, Move& guide, PieceStand& proof_pieces)
{
  assert(guide.isNormal());
  if (! guide.isDrop())
  {
    const Piece p=state->pieceOnBoard(guide.to());
    if (!p.isPtype<KING>())
      guide=guide.newCapture(p);
  }
  if (state->template isAlmostValidMove<false>(guide)
      && move_classifier::Check<P>
      ::isMember(*state, guide.ptype(), guide.from(), guide.to()))
    return attack<P,SetProofPieces,true>(depth, guide, proof_pieces);
  return attack<P,SetProofPieces,false>(depth, guide, proof_pieces);
}

template <osl::Player P>
const osl::checkmate::ProofDisproof osl::checkmate::
FixedDepthSolverExt::hasCheckmateMove(int depth, Move& best_move,
				      PieceStand& proof_pieces) {
  return attack<P,SetProofPieces,false>(depth, best_move, proof_pieces);
}

template <osl::Player P>
const osl::checkmate::ProofDisproof osl::checkmate::
FixedDepthSolverExt::hasEscapeMove(Move last_move,int depth,
				   PieceStand& proof_pieces) {
  return defense<P,SetProofPieces>(last_move, depth, proof_pieces);
}

namespace osl
{
  template const checkmate::ProofDisproof checkmate::FixedDepthSolverExt::hasCheckmateMove<BLACK>(int, Move&, PieceStand&);
  template const checkmate::ProofDisproof checkmate::FixedDepthSolverExt::hasCheckmateMove<WHITE>(int, Move&, PieceStand&);
  template const checkmate::ProofDisproof checkmate::FixedDepthSolverExt::hasEscapeMove<BLACK>(Move, int, PieceStand&);
  template const checkmate::ProofDisproof checkmate::FixedDepthSolverExt::hasEscapeMove<WHITE>(Move, int, PieceStand&);
  template const checkmate::ProofDisproof checkmate::FixedDepthSolverExt::hasEscapeByMove<BLACK>(Move, int, Move&, PieceStand&);
  template const checkmate::ProofDisproof checkmate::FixedDepthSolverExt::hasEscapeByMove<WHITE>(Move, int, Move&, PieceStand&);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
