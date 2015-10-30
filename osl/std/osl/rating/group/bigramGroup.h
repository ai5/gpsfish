/* bigramGroup.h
 */
#ifndef _BIGRAMGROUP_H
#define _BIGRAMGROUP_H

#include "osl/rating/group.h"
#include "osl/rating/feature/bigramAttack.h"

namespace osl
{
  namespace rating
  {
    class BigramAttackGroup : public Group
    {
      bool same, focus_x;
    public:
      static std::string name(bool same, bool focus_x);
      BigramAttackGroup(bool same, bool focus_x);

      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move m, const RatingEnv& env) const;
    };
  }
}


#endif /* _BIGRAMGROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
