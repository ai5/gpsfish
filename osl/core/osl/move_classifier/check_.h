/* check_.h
 */
#ifndef OSL_MOVE_CLASSIFIER_CHECK_H
#define OSL_MOVE_CLASSIFIER_CHECK_H
#include "osl/move_classifier/openCheck.h"
#include "osl/move_classifier/directCheck.h"
namespace osl
{
  namespace move_classifier
  {
    /**
     * @param 指す側, alt(P)に王手をかけられるか判定
     */
    template <Player P>
    struct Check
    {
      /**
       * promote move の時 ptypeはpromote後のもの
       */
      static bool isMember(const NumEffectState& state, 
			   Ptype ptype,Square from,Square to){
	if (DirectCheck<P>::isMember(state,ptype,to)) 
	  return true;
	if (from.isPieceStand()) 
	  return false;
	return OpenCheck<P>::isMember(state,ptype,from,to);
      }
    };
  } // namespace move_classifier
} // namespace osl
#endif /* _MOVE_CLASSIFIER_CHECK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
