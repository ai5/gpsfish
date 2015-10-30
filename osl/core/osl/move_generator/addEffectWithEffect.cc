#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_generator/addEffectWithEffect.tcc"
#ifdef RELEASE
#include "osl/move_generator/open.tcc"
#endif

namespace osl{
  // explicit template instantiation
  namespace move_generator
  {
    typedef move_action::Store store_t;
    template void GenerateAddEffectWithEffect::generate<true>
    (Player, const NumEffectState&, Square, store_t&);
    template void GenerateAddEffectWithEffect::generate<false>
    (Player, const NumEffectState&, Square, store_t&);
    // for fixed depth search
    template void AddEffectWithEffect<store_t>::generate<BLACK, true>(const NumEffectState&, Square, store_t&,bool&);
    template void AddEffectWithEffect<store_t>::generate<WHITE, true>(const NumEffectState&, Square, store_t&,bool&);
    //
    template void AddEffectWithEffect<store_t>::generate<BLACK, false>(const NumEffectState&, Square, store_t&,bool&);
    template void AddEffectWithEffect<store_t>::generate<WHITE, false>(const NumEffectState&, Square, store_t&,bool&);
  } // namespace move_generator
} // namespace osl
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
