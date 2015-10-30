/* pinAttack.h
 */
#ifndef _PINATTACK_H
#define _PINATTACK_H

#include "osl/rating/feature.h"
#include "osl/bits/ptypeTable.h"

namespace osl
{
  namespace rating
  {
    class PinAttack : public Feature
    {
      bool attack;
      Ptype self, target;
    public:
      PinAttack(bool a, Ptype s, Ptype t) 
	: Feature(name(a,s,t)), 
	  attack(a), self(s), target(t)
      {
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&, Piece p) const
      {
	if (target != p.ptype())
	  return false;
	return state.hasEffectIf(move.ptypeO(), move.to(), p.square())
	  && (move.isDrop() 
	      || ! state.hasEffectByPiece(state.pieceOnBoard(move.from()), p.square()));
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (self != move.ptype())
	  return false;
	if (state.countEffect(alt(state.turn()), move.to(), env.op_pin) > 0)
	  return false;
	PieceMask pins = (attack ? env.op_pin : env.my_pin);
	while (pins.any()) {	// pin が複数あると不正確?
	  const Piece p = state.pieceOf(pins.takeOneBit());
	  if (match(state, move, env, p))
	    return true;
	}
	return false;
      }
      static int index(const NumEffectState& state, Move move, const RatingEnv&, bool attack, Piece p) 
      {
	if (! (state.hasEffectIf(move.ptypeO(), move.to(), p.square())
	       && (move.isDrop()
		   || ! state.hasEffectByPiece(state.pieceOnBoard(move.from()), p.square()))))
	  return -1;
	int index = (move.ptype() - PTYPE_PIECE_MIN) * (PTYPE_MAX+1 - PTYPE_PIECE_MIN) + p.ptype() - PTYPE_PIECE_MIN;
	index *= 2;
	return attack ? index : index + 1;
      }
      static int index(const NumEffectState& state, Move move, const RatingEnv& env, bool attack) 
      {
	if (state.countEffect(alt(state.turn()), move.to(), env.op_pin) > 0)
	  return -1;
	PieceMask pins = (attack ? env.op_pin : env.my_pin);
	while (pins.any()) {	// pin が複数あると不正確?
	  const Piece p = state.pieceOf(pins.takeOneBit());
	  const int i = index(state, move, env, attack, p);
	  if (i >= 0)
	    return i;
	}
	return -1;
      }
      static const std::string name(bool attack, Ptype self, Ptype target) 
      {
	return std::string(Ptype_Table.getCsaName(self))+">"+Ptype_Table.getCsaName(target)+(attack ? "!" : "=");
      }
    };

    class EscapePin : public Feature
    {
      Ptype pinned;
    public:
      explicit EscapePin(Ptype p) : Feature(Ptype_Table.getCsaName(p)), pinned(p) {}
      bool match(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	if (move.ptype() != KING)
	  return false;
	return (env.my_pin.getMask(Ptype_Table.getIndex(pinned)) 
		& Ptype_Table.getMaskLow(pinned)).any();
      }
    };

  }
}

#endif /* _PINATTACK_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
