/* escape.h
 */
#ifndef _ESCAPE_H
#define _ESCAPE_H

#include "osl/rating/feature.h"
#include "osl/rating/feature/countEffect2.h"

namespace osl
{
  namespace rating
  {
    /** 移動元へのきき。attack defense は言葉の意味と逆で自分がattack */
    class FromEffect : public Feature, CountEffect2
    {
    public:
      FromEffect(int attack, int defense) 
	: Feature("FE"+CountEffect2::name(attack, defense)), CountEffect2(attack, defense) {}
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	return ! move.isDrop() && CountEffect2::match(state, move.from(), env);
      }
    };

    class PtypeAttacked : public Feature
    {
      Ptype self, attack;
    public:
      PtypeAttacked(Ptype s, Ptype a) 
	: Feature(std::string(Ptype_Table.getCsaName(s))+"<"+Ptype_Table.getCsaName(a)), 
	  self(s), attack(a)
      {
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return move.oldPtype() == self
	  && ! move.isDrop()
	  && state.findCheapAttack(alt(move.player()), move.from()).ptype()== attack;
      }
    };

    class ToSupported : public Feature
    {
    public:
      ToSupported() : Feature("TS") {}
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return state.hasEffectAt(move.player(), move.to());
      }
    };

    class ImmediateEscape : public Feature
    {
      Ptype self, attack;
    public:
      ImmediateEscape(Ptype s, Ptype a) 
	: Feature(std::string(Ptype_Table.getCsaName(s))+"<"+Ptype_Table.getCsaName(a)),
	  self(s), attack(a)
      {
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.isDrop())
	  return false;
	if (move.ptype() != self)
	  return false;
	const Move last_move = env.history.lastMove();
	if (! last_move.isNormal() || last_move.ptype() != attack)
	  return false;
	return state.hasEffectIf(last_move.ptypeO(), last_move.to(), move.from());
      }
    };

    class KingEscape : public Feature
    {
      Ptype ptype;
    public:
      KingEscape(Ptype s) 
	: Feature(std::string(Ptype_Table.getCsaName(s))), ptype(s)
      {
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return state.inCheck()
	  && move.ptype() == ptype;
      }
      bool effectiveInCheck() const { return true; }
    };
  }
}

#endif /* _ESCAPE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
