/* kingOpenMove.h
 */
#ifndef OSL_MOVE_CLASSIFIER_KING_OPEN_MOVE_H
#define OSL_MOVE_CLASSIFIER_KING_OPEN_MOVE_H

#include "osl/move_classifier/classifierTraits.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_classifier
  {
    /**
     * Pの王をopen checkにする手でないことをチェック.
     * - P==move playerの時は自殺手かどうかのチェックに使う.
     *      王が動く場合には呼べない
     * - P!=move playerの時は通常のopen checkかどうかに使う.
     * - DropMoveの時には呼べない
     */
    template <Player P>
    struct KingOpenMove
    {
      /**
       * king が59
       * rookが51->61の時，差は
       * OFFSET -8 -> U
       * OFFSET +8 -> D
       * とはなるので，一直線のような気がする．ただし，そもとも，
       * 59 - 51はpinにはならないし，今は U -> DはopenではないとしているのでOK
       */
      static bool isMember(const NumEffectState& state, 
			   Ptype /*ptype*/,Square from,Square to)
      {
	int num=state.pieceAt(from).number();
	assert(Piece::isPieceNum(num));
	if(!state.pinOrOpen(P).test(num)) return false;
	// from to kingが一直線に並べば false
	Square king=state.kingSquare<P>();
	return Board_Table.getShort8Unsafe<P>(king,to)
	  != Board_Table.getShort8<P>(king,from);
      }
      /**
       * @param exceptFor ここからの利きは除外
       */
      static bool isMember(const NumEffectState& state, 
			   Ptype ptype,Square from,Square to,
			   Square exceptFor)
      {
	return isMemberMain<true>(state, ptype, from, to, exceptFor);
      }
    private:
      template <bool hasException>
      static bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      isMemberMain(const NumEffectState& state, 
		   Ptype ptype,Square from,Square to,
		   Square exceptFor);
    };

    template <Player P> struct ClassifierTraits<KingOpenMove<P> >
    {
      static const bool drop_suitable = false;
      static const bool result_if_drop = false;
    };
    
  } // namespace move_classifier
} // namespace osl
#endif /* OSL_MOVE_CLASSIFIER_NOT_KING_OPEN_MOVE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
