/* countEffect2.h
 */
#ifndef _COUNTEFFECT2_H
#define _COUNTEFFECT2_H

#include "osl/rating/ratingEnv.h"
#include "osl/numEffectState.h"
#include "osl/additionalEffect.h"

namespace osl
{
  namespace rating
  {
    struct CountEffect2
    {
      static const int Max = 2;
      int attack, defense;
      CountEffect2(int a, int d) : attack(a), defense(d)
      {
      }
      static std::pair<int,int> count(const NumEffectState& state, Square position,
				      const RatingEnv& env) 
      {
	int attack = 0, defense = 0;
	if (position.isOnBoard()) {
	  assert(position.isOnBoard());
	  const Player turn = state.turn();
	  attack = std::min(Max, state.countEffect(turn, position, env.my_pin));
	  defense = std::min(Max, state.countEffect(alt(turn), position, env.op_pin));
	  if (attack && (attack < Max))
	    attack += AdditionalEffect::hasEffect(state, position, turn);
	  if (defense && (defense < Max))
	    defense += AdditionalEffect::hasEffect(state, position, alt(turn));
	}
	return std::make_pair(attack, defense);
      }
      bool match(const NumEffectState& state, Square position, const RatingEnv& env) const
      {
	std::pair<int,int> ad = count(state, position, env);
	return attack == ad.first && defense == ad.second;
      }
      static std::string name(int attack, int defense);
      static int index(const NumEffectState& state, Square position, const RatingEnv& env) 
      {
	if (! position.isOnBoard())
	  return 0;
	if (env.counteffect2_cache[position.index()] < 0) 
	{
	  std::pair<int,int> ad = count(state, position, env);
	  env.counteffect2_cache[position.index()] = ad.first*(Max+1)+ad.second;
	}
	return env.counteffect2_cache[position.index()];
      }
    };
  }
}

#endif /* _COUNTEFFECT2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
