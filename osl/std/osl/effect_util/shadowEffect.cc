#include "osl/effect_util/shadowEffect.h"
#include "osl/bits/additionalOrShadow.h"
#include "osl/additionalEffect.h"

template <int count_max>
int osl::effect_util::
ShadowEffect::count(const NumEffectState& state, Square target, 
		    Player attack)
{
  PieceVector direct_pieces;
  state.findEffect(alt(attack), target, direct_pieces);
  return AdditionalOrShadow::count<count_max>
    (direct_pieces, state, target, attack);
}

bool osl::effect_util::
ShadowEffect::hasEffect(const NumEffectState& state, Square target, 
			Player attack)
{
  return count<1>(state, target, attack);
}

int osl::effect_util::
ShadowEffect::count2(const NumEffectState& state, Square target, 
		     Player attack)
{
  return count<2>(state, target, attack);
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
