/* moveAdaptor.h
 */
#ifndef OSL_MOVE_CLASSIFIER_MOVE_ADAPTOR_H
#define OSL_MOVE_CLASSIFIER_MOVE_ADAPTOR_H

#include "osl/numEffectState.h"
#include "osl/move_classifier/classifierTraits.h"
namespace osl
{
  namespace move_classifier
  {
    template <class Classifier>
    struct MoveAdaptor
    {
      static bool isMember(const NumEffectState& state, Move m) 
      {
	return Classifier::isMember(state, m.ptype(), m.from(), m.to());
      }
    };

    template <template <Player> class Classifier>
    struct PlayerMoveAdaptor
    {
      template <class State>
      static bool isMember(const State& state, Move m) 
      {
	assert(m.player() == state.turn());
	if (state.turn() == BLACK)
	  return Classifier<BLACK>::isMember(state, m.ptype(), m.from(), m.to());
	else
	  return Classifier<WHITE>::isMember(state, m.ptype(), m.from(), m.to());
      }
    };

    /** drop の時は呼べないなどの条件を代わりにテスト */
    template <template <Player> class Classifier>
    struct ConditionAdaptor
    {
      template <class State>
      static bool isMember(const State& state, Move m) 
      {
	if (! ClassifierTraits<Classifier<BLACK> >::drop_suitable
	    && m.isDrop())
	  return ClassifierTraits<Classifier<BLACK> >::result_if_drop;
	return PlayerMoveAdaptor<Classifier>::isMember(state, m);
      }
    };
  } // namespace move_classifier
} // namespace osl

#endif /* OSL_MOVE_CLASSIFIER_MOVE_ADAPTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
