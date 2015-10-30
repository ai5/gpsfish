/* bigramGroup.cc
 */
#include "osl/rating/group/bigramGroup.h"
#include <sstream>

std::string osl::rating::BigramAttackGroup::name(bool same, bool focus_x)
{
  std::ostringstream ss;
  ss << "BigramA" << (same ? '=' : '!') << (focus_x ? 'X' : 'Y');
  return ss.str();
}

osl::rating::BigramAttackGroup::BigramAttackGroup(bool s, bool f) 
  : Group(name(s, f)), same(s), focus_x(f)
{
  for (int x=-2; x<=2; ++x) {
    for (int y=-2; y<=2; ++y) {
      for (int x2=-2; x2<=2; ++x2) {
	for (int y2=-2; y2<=2; ++y2) {
	  for (int king=0; king<5; ++king) {
	    push_back(new BigramAttack(x, y, x2, y2, king, same, focus_x));
	  }
	}
      }
    }
  }
}

int osl::rating::BigramAttackGroup::findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
{
  const int index = BigramAttack::index(state, move, env, same, focus_x);
  return index;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
