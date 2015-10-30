#include "osl/move_generator/attackToPinned.h"
#include "osl/move_generator/attackToPinned.tcc"

template void osl::move_generator::AttackToPinned<osl::BLACK>::generate<osl::move_action::Store>(const NumEffectState&, move_action::Store&);
template void osl::move_generator::AttackToPinned<osl::WHITE>::generate<osl::move_action::Store>(const NumEffectState&, move_action::Store&);

void 
osl::move_generator::GenerateAttackToPinned::
generate(Player player, const NumEffectState& state,
	 move_action::Store& store){
  assert(state.turn()==player);
  if(player==BLACK)
    AttackToPinned<BLACK>::generate(state,store);
  else
    AttackToPinned<WHITE>::generate(state,store);
}
