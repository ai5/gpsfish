/* king8Group.h
 */
#ifndef _KING8GROUP_H
#define _KING8GROUP_H

#include "osl/rating/group.h"
#include "osl/rating/feature/king8.h"

namespace osl
{
  namespace rating
  {
    struct AttackKing8Group : public Group
    {
      AttackKing8Group();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move m, const RatingEnv&) const;
      bool effectiveInCheck() const { return true; }
    };

    struct DefenseKing8Group : public Group
    {
      DefenseKing8Group();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move m, const RatingEnv&) const;
    };
  }
}

#endif /* _KING8GROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
