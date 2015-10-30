/* treePredictor.cc
 */
#include "osl/threatmate/treePredictor.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/progress.h"
#include "osl/bits/king8Info.h"
bool osl::threatmate::TreePredictor::predict(const NumEffectState& state, 
					       const Move move){
  const Player turn = alt(state.turn());
  progress::NewProgress tprogress(state);
  const int progress = (tprogress.progressAttack(alt(turn)).value());

  if (Neighboring8Direct::hasEffect(state, newPtypeO(turn, move.ptype()), move.to(), state.kingSquare(alt(turn)))){
    King8Info K(state.Iking8Info(alt(turn)));
    if(progress>=5 || K.dropCandidate()) return true;
    if(turn==BLACK)
      return K.hasMoveCandidate<BLACK>(state);
    else
      return K.hasMoveCandidate<WHITE>(state);
  }
  if (progress > 5 ) {
    if (! isPiece( move.capturePtype() ) )
      return false;
    if ( isMajor(move.capturePtype())    ||
	 (move.capturePtype() == SILVER) ||
	 (move.capturePtype() == PSILVER)||
	 (move.capturePtype() == GOLD) )
      return true;
  }
  return false;
}
double osl::threatmate::TreePredictor::probability(const NumEffectState& state,
						   const Move move){
  const Player turn = alt(state.turn());
  osl::progress::ml::NewProgress tprogress(state);
  const int progress = (tprogress.progressAttack(alt(turn)).value());
  if (Neighboring8Direct::hasEffect(state, newPtypeO(turn, move.ptype()), move.to(), state.kingSquare(alt(turn)))){
    if (progress > 4) {
      if (progress > 5)
	return 0.87601;
      return 0.69349;
    }
    King8Info K(state.Iking8Info(alt(turn)));
    if (K.dropCandidate() )
      return 0.5637;
    if(turn==BLACK){
      if(K.hasMoveCandidate<BLACK>(state))
	return 0.89933;
    }
    else if(K.hasMoveCandidate<WHITE>(state))
      return 0.89933;
    return 0.22403;
  }
  if (progress < 5 )
    return 0.041633;
  if (move.capturePtype() == GOLD)
    return 0.81872;
  if (move.capturePtype() == SILVER)
    return 0.78608;
  if (move.capturePtype() == ROOK)
    return 0.83592;
  if (move.capturePtype() == BISHOP)
    return 0.84542;
  return 0.14094;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
