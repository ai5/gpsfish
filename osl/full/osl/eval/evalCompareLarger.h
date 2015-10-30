/* evalCompareLarger.h
 */
#ifndef _EVAL_COMPARE_LARGER_H
#define _EVAL_COMPARE_LARGER_H

#include "osl/eval/evalTraits.h"

namespace osl
{
  namespace eval
  {
    /**
     * 評価の高い順に並べる比較
     */
    template <Player P> struct EvalCompareLarger
    {
      bool operator()(int l, int r) const
      {
	return EvalTraits<P>::betterThan(l, r);
      }
    };

    /**
     * 評価の高い順に並べる比較
     */
    struct EvalCompareLargerNT
    {
      const Player player;
      EvalCompareLargerNT(Player p) : player(p)
      {
      }
      bool operator()(int l, int r) const
      {
	return betterThan(player, l, r);
      }
    };
  } // namespace eval
} // namespace osl

#endif /* _EVAL_COMPARE_LARGER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
