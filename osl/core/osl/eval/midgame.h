/* midgame.h
 */
#ifndef OSL_MIDGAME_H
#define OSL_MIDGAME_H
#include "osl/bits/quadInt.h"

#define EVAL_QUAD

namespace osl
{
  const int NStages = 4;
  const int EvalStages = 4;
  typedef QuadInt MultiInt;
  typedef QuadIntPair MultiIntPair;

  const int EndgameIndex = EvalStages-1;
}

#endif /* OSL_MIDGAME_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
