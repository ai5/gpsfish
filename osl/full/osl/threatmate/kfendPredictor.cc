/* kfendPredictor.cc
 */
#include "osl/threatmate/kfendPredictor.h"
#include "osl/effect_util/neighboring8Direct.h"

bool osl::threatmate::KfendPredictor::predict(const NumEffectState& state, 
					      const Move move){
  const Player turn = alt(state.turn());
  const Square opKingSquare = state.kingSquare(alt(turn));

  // Capture Piece
  if (move.capturePtype())
    return true;

  // Add Effect for King's Neighboring8
  if ( Neighboring8Direct::hasEffect(state, newPtypeO(turn, move.ptype()),
				     move.to(), opKingSquare) )
    return true;
  return false;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
