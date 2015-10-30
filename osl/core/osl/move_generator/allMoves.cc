#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/allMoves.tcc"

namespace osl
{
  namespace move_generator
  {
    template void AllMoves<move_action::Store>::generate<BLACK>(NumEffectState const&,move_action::Store&);
    template void AllMoves<move_action::Store>::generate<WHITE>(NumEffectState const&,move_action::Store&);
    template void AllMoves<move_action::Store>::generate(Player,NumEffectState const&,move_action::Store&);
  }
} // namespace osl

void osl::GenerateAllMoves::
generate(Player p, const NumEffectState& state, MoveVector& out)
{
  typedef move_action::Store store_t;
  store_t store(out);
  move_generator::AllMoves<store_t>::generate(p, state, store);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
