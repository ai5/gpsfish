/* directCheck.h
 */
#ifndef OSL_MOVE_CLASSIFIER_DIRECTCHECK_H
#define OSL_MOVE_CLASSIFIER_DIRECTCHECK_H

#include "osl/basic_type.h"

namespace osl
{
  namespace move_classifier
  {
    template <Player P>
    struct DirectCheck
    {
      static bool isMember(const NumEffectState& state, Ptype ptype, Square to)
      {
	/**
	 * 最初から王手ということはない．
	 */
	assert(!state.template hasEffectAt<P>(state.template kingSquare<alt(P)>()));
	/**
	 * stateを動かしていないので，fromにある駒がtoからの利きを
	 * blockすることは
	 * あるが，blockされた利きが王手だったとすると，動かす前から王手
	 * だったとして矛盾するのでOK
	 */
	return state.hasEffectIf(newPtypeO(P,ptype),to,
				 state.template kingSquare<alt(P)>());
      }

      template<class State>
      static bool isMember(const State& state, Ptype ptype, Square /*from*/, Square to)
      {
	return isMember(state, ptype, to);
      }
    };
  } // namespace move_classifier
} // namespace osl

#endif /* _DIRECTCHECK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
