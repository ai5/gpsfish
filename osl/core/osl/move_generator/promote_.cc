#include "osl/move_generator/promote_.h"
#include "osl/move_generator/promote_.tcc"
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    typedef move_action::Store store_t;
    // explicit template instantiation
    template void Promote<BLACK, true>::generateMoves(const NumEffectState&, store_t&);
    template void Promote<WHITE, true>::generateMoves(const NumEffectState&, store_t&);
    template void Promote<BLACK, false>::generateMoves(const NumEffectState&, store_t&);
    template void Promote<WHITE, false>::generateMoves(const NumEffectState&, store_t&);

    template void Promote<BLACK, true>::generateMovesPtype<store_t,(Ptype)10>(NumEffectState const&, store_t&);
    template void Promote<BLACK, true>::generateMovesPtype<store_t,(Ptype)11>(NumEffectState const&, store_t&);
    template void Promote<BLACK, true>::generateMovesPtype<store_t,(Ptype)12>(NumEffectState const&, store_t&);
    template void Promote<BLACK, true>::generateMovesPtype<store_t,(Ptype)13>(NumEffectState const&, store_t&);
    template void Promote<BLACK, true>::generateMovesPtype<store_t,(Ptype)14>(NumEffectState const&, store_t&);
    template void Promote<BLACK, true>::generateMovesPtype<store_t,(Ptype)15>(NumEffectState const&, store_t&);

    template void Promote<WHITE, true>::generateMovesPtype<store_t,(Ptype)10>(NumEffectState const&, store_t&);
    template void Promote<WHITE, true>::generateMovesPtype<store_t,(Ptype)11>(NumEffectState const&, store_t&);
    template void Promote<WHITE, true>::generateMovesPtype<store_t,(Ptype)12>(NumEffectState const&, store_t&);
    template void Promote<WHITE, true>::generateMovesPtype<store_t,(Ptype)13>(NumEffectState const&, store_t&);
    template void Promote<WHITE, true>::generateMovesPtype<store_t,(Ptype)14>(NumEffectState const&, store_t&);
    template void Promote<WHITE, true>::generateMovesPtype<store_t,(Ptype)15>(NumEffectState const&, store_t&);
  } // namespace move_generator
} // namespace osl
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
