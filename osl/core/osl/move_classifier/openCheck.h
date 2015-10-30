/* openCheck.h
 */
#ifndef _MOVE_CLASSIFIER_OPENCHECK_H
#define _MOVE_CLASSIFIER_OPENCHECK_H

#include "osl/move_classifier/classifierTraits.h"
#include "osl/move_classifier/kingOpenMove.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_classifier
  {
    template <Player P>
    struct OpenCheck
    {
      static bool isMember(const NumEffectState& state, 
			   Ptype ptype,Square from,Square to)
      {
	return KingOpenMove<alt(P)>::isMember(state,ptype,from,to);
      }
    };

    template <Player P> struct ClassifierTraits<OpenCheck<P> >
    {
      static const bool drop_suitable = false;
      static const bool result_if_drop = false;
    };
  } // namespace move_classifier
} // namespace osl

#endif /* _MOVE_CLASSIFIER_OPENCHECK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
