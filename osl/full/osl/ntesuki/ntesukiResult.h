#ifndef _ID_NTESUKI_RESULT_H
#define _ID_NTESUKI_RESULT_H
#include "osl/checkmate/proofDisproof.h"
#include <cassert>
#include <iosfwd>

using namespace osl::checkmate;

namespace osl{
  namespace ntesuki{
    /**
     * n 手すきを探索した結果を保存しておくための型.
     * 現在では checkmate の ProofDisProof
     */
    typedef ProofDisproof NtesukiResult;
  }
}
#endif /* _ID_NTESUKI_RESULT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

