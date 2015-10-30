#include "osl/checkmate/proofDisproof.h"
#include <iostream>

void osl::checkmate::
ProofDisproof::testConsistency()
{
  static_assert((ProofDisproof::BigProofNumber > ProofDisproof::PAWN_CHECK_MATE_PROOF),"");
  static_assert((ProofDisproof::NO_CHECK_MATE_PROOF > ProofDisproof::PAWN_CHECK_MATE_PROOF),"");
}

std::ostream& osl::checkmate::
operator<<(std::ostream& os, const ProofDisproof& pdp)
{
  if (pdp == ProofDisproof::Checkmate())
    os << "Checkmate";
  else if (pdp == ProofDisproof::NoEscape())
    os << "NoEscape";
  else if (pdp == ProofDisproof::NoCheckmate())
    os << "NoCheckmate";
  else if (pdp == ProofDisproof::PawnCheckmate())
    os << "PawnCheckmate";
  else if (pdp == ProofDisproof::LoopDetection())
    os << "LoopDetection";
  else if (pdp == ProofDisproof::AttackBack())
    os << "AttackBack";
  else
    os << "pdp-" << pdp.proof() << "," << pdp.disproof();
  return os;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
