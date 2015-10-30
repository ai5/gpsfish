/* virtualPin.cc
 */
#include "osl/effect_util/virtualPin.h"

bool osl::effect_util::
VirtualPin::find(const NumEffectState& state, Player defense, const PieceMask& remove)
{
  const Square target = state.kingSquare(defense);
  return findDirection<UL>(state, target, defense, remove)
    || findDirection<U>(state, target, defense, remove)
    || findDirection<UR>(state, target, defense, remove)
    || findDirection<L>(state, target, defense, remove)
    || findDirection<R>(state, target, defense, remove)
    || findDirection<DL>(state, target, defense, remove)
    || findDirection<D>(state, target, defense, remove)
    || findDirection<DR>(state, target, defense, remove);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
