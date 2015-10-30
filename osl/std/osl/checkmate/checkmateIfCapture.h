/* checkmateIfCapture.h
 */
#ifndef OSL_CHECKMATEIFCAPTURE_H
#define OSL_CHECKMATEIFCAPTURE_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace checkmate
  {

    struct CheckmateIfCapture
    {
      /**
       * move を指した後，alt(move.player())が取ると詰みかどうか.
       * - alt(move.player()) から利きがない場合はfalse
       * - 取る指手が全て取ると詰の場合は true
       * @param depth 0 なら ImmediateCheckmate のみ，2なら3手詰．
       */
      static bool effectiveAttack(NumEffectState& state, Move move, int depth);

      /**
       * 手番の側がSquare の駒を取っても詰みがないか．
       * - target に利きがない場合はfalse
       * - 取る指手が全て取ると詰の場合は true
       * @param depth 0 なら ImmediateCheckmate のみ，2なら3手詰．
       */
      static bool cannotCapture(NumEffectState& state, Square last_to, int depth);

      /** depth==0でeffectiveAttackになる可能性がなければfalse */
      static bool effectiveAttackCandidate0(const NumEffectState& state, Move move);
      struct CallDefense;
    };

  } // namespace checkmate
} // osl

#endif /* OSL_CHECKMATEIFCAPTURE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
