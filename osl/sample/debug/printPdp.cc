/* printPdp.cc
 */
#include "osl/checkmate/proofDisproof.h"
#include <iostream>

using namespace osl;
using namespace osl::checkmate;
int main()
{
  std::cout << "Checkmate "     << ProofDisproof::Checkmate()    .ulonglongValue() << "\n";
  std::cout << "NoEscape "      << ProofDisproof::NoEscape()     .ulonglongValue() << "\n";
  std::cout << "NoCheckmate "   << ProofDisproof::NoCheckmate()  .ulonglongValue() << "\n";
  std::cout << "LoopDetection " << ProofDisproof::LoopDetection().ulonglongValue() << "\n";
  std::cout << "PawnCheckmate " << ProofDisproof::PawnCheckmate().ulonglongValue() << "\n";
  std::cout << "ProofLimit "    << ProofDisproof::PROOF_LIMIT    << "\n";
  std::cout << "DisproofLimit " << ProofDisproof::DISPROOF_LIMIT << "\n";
  unsigned long long pdp;
  while (std::cin >> pdp)
  {
    std::cout << ProofDisproof::makeDirect(pdp) << "\n";
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
