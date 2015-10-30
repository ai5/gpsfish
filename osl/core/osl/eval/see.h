/* see.h
 */
#ifndef OSL_SEE_H
#define OSL_SEE_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace eval
  {
    class PtypeEvalTable;
    struct See
    {
      static int see(const NumEffectState& state, Move move,
		     const PieceMask& my_pin=PieceMask(), const PieceMask& op_pin=PieceMask(), const PtypeEvalTable *table=0);

      // public for test
      template <Player P>
      static int seeInternal(const NumEffectState& state, Move move,
			     const PieceMask& my_pin, const PieceMask& op_pin,
			     const PtypeEvalTable &table);
      template <Player P>
      static void findEffectPieces(const NumEffectState& state, Square effect_to,
				   PtypeOSquareVector& my_pieces, PtypeOSquareVector& op_pieces);
      template <osl::Player P>
      static void findEffectPiecesAfterMove(const NumEffectState& state, Move move,
					    PtypeOSquareVector& my_pieces, PtypeOSquareVector& op_pieces);
      static void findAdditionalPieces(const NumEffectState& state, Player attack, Square target,
				       Square direct_attack_from,
				       PtypeOSquareVector& out);
      /**
       * PtypeOSquareVector をもとに取り返し値を計算する
       * @param P alt(P) からの取り返し
       * @param target ここに関する取り返し
       * @param ptypeo target にあると想定される駒
       */
      template <Player P>
      static int computeValue(const NumEffectState& state,
			      Move move,
			      PtypeOSquareVector& my_pieces, 
			      PtypeOSquareVector& op_pieces,
			      const PieceMask& my_pin, 
			      const PieceMask& op_pin,
			      const PtypeEvalTable &table);
      struct StorePtypeO;
      struct FindEffectMore;
    };
  };
  using eval::See;
}

#endif /* _SEE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
