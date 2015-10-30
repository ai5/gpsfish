/* checkmateGroup.h
 */
#ifndef _CHECKMATEGROUP_H
#define _CHECKMATEGROUP_H

#include "osl/rating/group.h"
#include "osl/rating/feature/checkmate.h"

namespace osl
{
  namespace rating
  {
    class CheckmateIfCaptureGroup : public Group
    {
    public:
      CheckmateIfCaptureGroup() : Group("CheckmateCap")
      {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new CheckmateIfCapture);
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	if ((*this)[0].match(state, move, env))
	  return progress8;
	return -1;
      }
    };
    struct ThreatmateGroup : public Group
    {
      ThreatmateGroup() : Group("Threatmate")
      {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new Threatmate);
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	if ((*this)[0].match(state, move, env))
	  return progress8;
	return -1;
      }
    };
  }
} // name_width osl

#endif /* _CHECKMATEGROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
