/* pin.cc
 */
#include "osl/effect_util/pin.h"

#ifndef MINIMAL
osl::PieceMask osl::effect_util::
Pin::makeNaive(const SimpleState& state, Square target, 
	       Player defense)
{
  assert(target.isOnBoard());
  PieceMask result;
  findDirection<UL>(state, target, defense, result);
  findDirection<U>(state, target, defense, result);
  findDirection<UR>(state, target, defense, result);
  findDirection<L>(state, target, defense, result);
  findDirection<R>(state, target, defense, result);
  findDirection<DL>(state, target, defense, result);
  findDirection<D>(state, target, defense, result);
  findDirection<DR>(state, target, defense, result);
  return result;
}

osl::PieceMask osl::effect_util::
Pin::makeByPiece(const NumEffectState& state, Square target, 
		 Player defense)
{
  assert(target.isOnBoard());
  const Player attack = alt(defense);
  PieceMask result;
  // 香車
  findDirection<U>(state, target, defense, result);
  // 飛車 角
  findPtype<ROOK>(state, target, attack, defense, result);
  findPtype<BISHOP>(state, target, attack, defense, result);
  return result;
}

osl::PieceMask osl::effect_util::
Pin::makeByPieceKing(const NumEffectState& state, Square target, 
		     Player defense)
{
  assert(target.isOnBoard());
  const Player attack = alt(defense);
  PieceMask result;
  // 香車
  if(defense==BLACK){
    findLance<BLACK>(state, target, result);
  }
  else{
    findLance<WHITE>(state, target, result);
  }
  // 飛車 角
  findPtype<ROOK>(state, target, attack, defense, result);
  findPtype<BISHOP>(state, target, attack, defense, result);
  return result;
}
#endif

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
