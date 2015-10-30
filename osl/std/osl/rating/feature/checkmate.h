/* checkmate.h
 */
#ifndef _CHECKMATE_H
#define _CHECKMATE_H

#include "osl/rating/feature.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/checkmateIfCapture.h"

namespace osl
{
  namespace rating
  {
    class CheckmateIfCapture : public Feature
    {
    public:
      CheckmateIfCapture() : Feature("CC") {}
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return state.hasEffectAt(alt(move.player()), move.to())
	  && checkmate::CheckmateIfCapture::effectiveAttackCandidate0(state, move)
	  && checkmate::CheckmateIfCapture::effectiveAttack
	  (const_cast<NumEffectState&>(state), move, 0); // XXX: evil cast
      }
    };

    class Threatmate : public Feature
    {
    public:
      Threatmate() : Feature("Tm") {}
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const;
      struct Helper;

      static bool isCandidate(const NumEffectState& state, Move move);
      static bool knight2Step(const NumEffectState& state, Move move, Square king);
      static bool captureForKnightCheck(const NumEffectState& state, Move move, Square king);
    };

  }
}


#endif /* _CHECKMATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
