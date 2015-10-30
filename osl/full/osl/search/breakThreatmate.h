/* breakThreatmate.h
 */
#ifndef OSL_CATEGORY_BREAKTHREATMATE_H
#define OSL_CATEGORY_BREAKTHREATMATE_H

#include "osl/numEffectState.h"
#include "osl/container/moveLogProbVector.h"

namespace osl
{
  namespace search
  {
    struct BreakThreatmate
    {
      static void generateAddEffect(int limit, const NumEffectState&, Square to,
				    const MoveVector& src, MoveLogProbVector& out);
      static void generateBreakDrop(int limit, const NumEffectState&, Square to,
				    int default_prob, MoveLogProbVector& out);
      static void generateOpenRoad(int limit, const NumEffectState&, Square target,
				   MoveLogProbVector& out);
      static void generate(int limit, const NumEffectState&, Move threatmate_move,
			   MoveLogProbVector& out);

      static void findBlockLong(const NumEffectState& state, Move threatmate_move, MoveVector& out);
    };
  }
}


#endif /* OSL_CATEGORY_BREAKTHREATMATE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
