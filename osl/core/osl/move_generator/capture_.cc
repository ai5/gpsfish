/* generateCaptureMoves.cc
 */
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/capture_.tcc"
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    using namespace move_action;
    // explicit template instantiation
    template void GenerateCapture::generate(Player,const NumEffectState&, 
					    Square, Store&);
    template void GenerateCapture::generate1(Player,const NumEffectState&, 
					     Square, Store&);
    template void Capture<Store>::escapeByCapture<BLACK>
    (const NumEffectState&, Square, Piece, Store&);
    template void Capture<Store>::escapeByCapture<WHITE>
    (const NumEffectState&, Square, Piece, Store&);

    template void Capture<Store>::generate<BLACK>(NumEffectState const&, Square, Store&);
    template void Capture<Store>::generate<WHITE>(NumEffectState const&, Square, Store&);

    template void Capture<Store>::generate1<BLACK>(NumEffectState const&, Square, Store&);
    template void Capture<Store>::generate1<WHITE>(NumEffectState const&, Square, Store&);

  } // namespace move_generator
} // namespace osl
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
