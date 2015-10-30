/* safeMove.h
 */
#ifndef OSL_MOVE_CLASSIFIER_SAFE_MOVE_H
#define OSL_MOVE_CLASSIFIER_SAFE_MOVE_H
#include "osl/move_classifier/kingOpenMove.h"
#include "osl/move_classifier/classifierTraits.h"
#include "osl/numEffectState.h"
namespace osl
{
  namespace move_classifier
  {
    /**
     * 元々，手番の玉に王手がかかっていない状態で自殺手でないことをチェック.
     * DropMoveの時には呼べない
     */
    template <Player P>
    struct SafeMove
    {
      static bool isMember(const NumEffectState& state, 
			   Ptype ptype,Square from,Square to)
      {
	assert(! from.isPieceStand());
	assert(state.pieceOnBoard(from).owner() == P);
	/**
	 * 元々王手がかかっていないと仮定しているので，自分を
	 * 取り除いた上でhasEffectByを呼ばなくても良い
	 */
	if (ptype==KING)
	  return ! state.template hasEffectAt<alt(P)>(to);
	return ! KingOpenMove<P>::isMember(state,ptype,from,to);
      }
    };

    template <Player P> struct ClassifierTraits<SafeMove<P> >
    {
      static const bool drop_suitable = false;
      static const bool result_if_drop = true;
    };
  }
}
#endif /* OSL_MOVE_CLASSIFIER_SAFE_MOVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
