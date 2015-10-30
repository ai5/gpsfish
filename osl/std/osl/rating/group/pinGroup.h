/* pinGroup.h
 */
#ifndef _PINGROUP_H
#define _PINGROUP_H

#include "osl/rating/group.h"
#include "osl/rating/feature/pinAttack.h"

namespace osl
{
  namespace rating
  {
    struct PinGroup : public Group
    {
      PinGroup() : Group("PinAttack")
      {
	for (int s = PTYPE_PIECE_MIN; s<= PTYPE_MAX; ++s) {
	  for (int t = PTYPE_PIECE_MIN; t<= PTYPE_MAX; ++t) {
	    const Ptype self = static_cast<Ptype>(s);
	    const Ptype target = static_cast<Ptype>(t);
	    for (int p=0; p<8; ++p)	// progress8
	      push_back(new PinAttack(true,  self, target));
	    for (int p=0; p<8; ++p)	// progress8
	      push_back(new PinAttack(false, self, target));
	  }
	}
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	const int attack_index = PinAttack::index(state, move, env, true);
	if (attack_index >= 0)
	  return attack_index*8 + progress8;
	const int defense_index = PinAttack::index(state, move, env, false);
	if (defense_index >= 0)
	  return defense_index*8 + progress8;
	return -1;
      }
    };

    struct EscapePinGroup : public Group
    {
      EscapePinGroup() : Group("EscapePin")
      {
	for (int s = PTYPE_BASIC_MIN+1; s<= PTYPE_MAX; ++s) {
	  const Ptype self = static_cast<Ptype>(s);
	  for (int p=0; p<8; ++p)	// progress8
	    push_back(new EscapePin(self));
	}
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	if (move.ptype() != KING || ! env.my_pin.any())
	  return -1;
	int s;
	for (s = PTYPE_BASIC_MIN+1; s<= PTYPE_MAX; ++s) {
	  const Ptype pinned = static_cast<Ptype>(s);
	  if ((env.my_pin.getMask(Ptype_Table.getIndex(pinned))
	       & Ptype_Table.getMaskLow(pinned)).any())
	    break;
	}
	s -= PTYPE_BASIC_MIN+1;
	assert(s >= 0);
	const int progress8 = env.progress.value()/2;
	return s*8 + progress8;
      }
    };
  }
}


#endif /* _PINGROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
