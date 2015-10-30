#include "osl/move_generator/addEffect8.h"
#include "osl/move_generator/addEffect8.tcc"


template void osl::move_generator::AddEffect8<osl::BLACK>::generate<osl::move_action::Store>(const NumEffectState&,move_action::Store&);
template void osl::move_generator::AddEffect8<osl::WHITE>::generate<osl::move_action::Store>(const NumEffectState&,move_action::Store&);
template void osl::move_generator::AddEffect8<osl::BLACK>::generateBigDrop(const NumEffectState&,move_action::Store&);
template void osl::move_generator::AddEffect8<osl::WHITE>::generateBigDrop(const NumEffectState&,move_action::Store&);
template void osl::move_generator::AddEffect8<osl::BLACK>::generateNotBigDrop(const NumEffectState&,move_action::Store&);
template void osl::move_generator::AddEffect8<osl::WHITE>::generateNotBigDrop(const NumEffectState&,move_action::Store&);

void 
osl::move_generator::GenerateAddEffect8::
generate(Player player, const NumEffectState& state,
	 move_action::Store& store){
  assert(state.turn()==player);
  if(player==BLACK)
    AddEffect8<BLACK>::generate(state,store);
  else
    AddEffect8<WHITE>::generate(state,store);
}

void 
osl::move_generator::GenerateAddEffect8::
generateBigDrop(Player player, const NumEffectState& state,
	 move_action::Store& store){
  assert(state.turn()==player);
  if(player==BLACK)
    AddEffect8<BLACK>::generateBigDrop(state,store);
  else
    AddEffect8<WHITE>::generateBigDrop(state,store);
}

void 
osl::move_generator::GenerateAddEffect8::
generateNotBigDrop(Player player, const NumEffectState& state,
	 move_action::Store& store){
  assert(state.turn()==player);
  if(player==BLACK)
    AddEffect8<BLACK>::generateNotBigDrop(state,store);
  else
    AddEffect8<WHITE>::generateNotBigDrop(state,store);
}

