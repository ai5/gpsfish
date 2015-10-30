/* capture.h
 */
#ifndef _CAPTURE_H
#define _CAPTURE_H

#include "osl/rating/feature.h"
#include "osl/eval/see.h"
#include "osl/effect_util/shadowEffect.h"
#include "osl/additionalEffect.h"

namespace osl
{
  namespace rating
  {
    class Capture : public Feature
    {
    public:
      enum { INF = 999999 };
    private:
      int first, last;
      static const std::string name(int first, int last);
    public:
      Capture(int f, int l) : Feature(name(f, l)), first(f), last(l) {}
      static int see(const NumEffectState& state, Move move, const RatingEnv& env)
      {
	int see = See::see(state, move, env.my_pin, env.op_pin);
	see = see*100/128;
	return see;
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	int see = this->see(state, move, env);
	return first <= see && see < last;
      }
    };

    class ShadowEffect1 : public Feature
    {
    public:
      ShadowEffect1() : Feature("ShadowEffect1") {}
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return ShadowEffect::count2(state, move.to(), move.player()) == 1;
      }
    };

    class ShadowEffect2 : public Feature
    {
    public:
      ShadowEffect2() : Feature("ShadowEffect2") {}
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return ShadowEffect::count2(state, move.to(), move.player()) == 2;
      }
    };

    class ContinueCapture : public Feature
    {
    public:
      ContinueCapture() : Feature("Cont.C") {}
      bool match(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	return env.history.hasLastMove(2) && env.history.lastMove(2).to() == move.from()
	  && move.capturePtype() != PTYPE_EMPTY;
      }
    };

    /** 取った駒をすぐ使う */
    class DropCaptured : public Feature
    {
      Ptype ptype;
    public:
      DropCaptured(Ptype ptype);
      bool match(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	return move.isDrop() && move.ptype() == ptype
	  && env.history.hasLastMove(2) && env.history.lastMove(2).isNormal()
	  && env.history.lastMove(2).capturePtype() != PTYPE_EMPTY
	  && unpromote(env.history.lastMove(2).capturePtype()) == ptype;
      }
    };

  }
}


#endif /* _CAPTURE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
