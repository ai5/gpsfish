/* proofPiecesUtil.h
 */
#ifndef OSL_PROOFPIECESUTIL_H
#define OSL_PROOFPIECESUTIL_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace checkmate
  {
    class CheckMoveList;
    /**
     * ProofPieces と DisproofPieces に共通の関数
     */
    struct ProofPiecesUtil
    {
      /**
       * alt(player) が持っていない種類の持駒を playerが持っていたら
       * out に独占分を加算する．
       */
      static 
      void addMonopolizedPieces(const SimpleState& state, Player player,
				const PieceStand max, PieceStand& out)
      {
	const Player opponent = alt(player);
	for (Ptype ptype: PieceStand::order)
	{
	  if (! state.hasPieceOnStand(opponent, ptype))
	  {
	    const int diff = max.get(ptype) - out.get(ptype);
	    assert(diff >= 0);
	    if (diff)
	      out.add(ptype, diff);
	  }
	}
      }
    };
  } // namespace checkmate
} // osl

#endif /* OSL_PROOFPIECESUTIL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
