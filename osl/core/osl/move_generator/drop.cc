#include "osl/move_generator/drop.h"
#include "osl/move_generator/drop.tcc"
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"


namespace osl
{
  namespace move_generator
  {
    // explicit template instantiation
    template class Drop<move_action::Store>;
    template void Drop<move_action::Store>::generate<BLACK>(const NumEffectState&,move_action::Store&);
    template void Drop<move_action::Store>::generate<WHITE>(const NumEffectState&,move_action::Store&);
  } // namespace move_generator
} // namespace osl
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
