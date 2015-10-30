/* captureGroup.h
 */
#ifndef _CAPTUREGROUP_H
#define _CAPTUREGROUP_H

#include "osl/rating/group.h"
#include "osl/rating/feature/capture.h"

namespace osl
{
  namespace rating
  {
    class CaptureGroup : public Group
    {
    public:
      std::vector<int> see_range;
      CaptureGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int progress8 = env.progress.value()/2;
	const int see = Capture::see(state, move, env);
	size_t index;
	if (see > 50)
	  index = std::min(12, 7 + (see - 51) / 200);
	else if (see < -50)
	  index = std::max(0, (see + 1250) / 200);
	else
	  index = 6;
#ifndef NDEBUG
	for (size_t i=0; i<see_range.size()-1; ++i) {
	  if (see < see_range[i+1]) {
	    assert(i == index);
	    return i*8+progress8;
	  }
	}
	assert(0);
	abort();
#endif
	return index*8+progress8;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct ShadowEffectGroup : public Group
    {
      ShadowEffectGroup() : Group("ShadowEffect")
      {
	push_back(new ShadowEffect1());
	push_back(new ShadowEffect2());
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return ShadowEffect::count2(state, move.to(), move.player()) -1;
      }
    };

    struct ContinueCaptureGroup : public Group
    {
      ContinueCaptureGroup() : Group("Cont.Capture")
      {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new ContinueCapture());
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (! (*this)[0].match(state, move, env))
	  return -1;
	const int progress8 = env.progress.value()/2;
	return progress8;
      }
    };

    struct DropCapturedGroup : public Group
    {
      DropCapturedGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatchWithoutProgress(Move move, const RatingEnv& env) const
      {
	if (! (move.isDrop() && env.history.hasLastMove(2)))
	  return -1;
	const Move last2_move = env.history.lastMove(2);
	if (! (last2_move.isNormal()
	       && last2_move.capturePtype() != PTYPE_EMPTY
	       && unpromote(last2_move.capturePtype()) == move.ptype()))
	  return -1;
	return move.ptype() - PTYPE_BASIC_MIN;
      }
      int findMatch(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	const int index = findMatchWithoutProgress(move, env);
	if (index < 0)
	  return index;	
	const int progress8 = env.progress.value()/2;
	return index*8 + progress8;	
      }
    };
  }
}


#endif /* _CAPTUREGROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
