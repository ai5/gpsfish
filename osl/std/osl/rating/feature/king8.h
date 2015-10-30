/* king8.h
 */
#ifndef RATING_KING8_H
#define RATING_KING8_H

#include "osl/rating/feature.h"
#include "osl/rating/feature/countEffect2.h"
#include "osl/effect_util/neighboring8Direct.h"

namespace osl
{
  namespace rating
  {
    class AttackKing8 : public Feature, CountEffect2
    {
      Ptype self, target;
      bool same;
      static const std::string name(Ptype self, Ptype target, bool same);
    public:
      AttackKing8(Ptype s, Ptype t, bool ss, int attack, int defense) 
	: Feature(name(s, t, ss)+CountEffect2::name(attack, defense)), CountEffect2(attack, defense),
	  self(s), target(t), same(ss)
      {
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.ptype() != self)
	  return false;
	const Square position
	  = Neighboring8Direct::findNearest(state, move.ptypeO(), move.to(), state.kingSquare(alt(state.turn())));
	if (position.isPieceStand() || position == move.from())
	  return false;
	if (! move.isDrop() && state.hasEffectByPiece(state.pieceOnBoard(move.from()), position))
	  return false;
	const Piece p = state.pieceAt(position);
	if (p.ptype() != target)
	  return false;
	if (! CountEffect2::match(state, position, env))
	  return false;
	if (!isPiece(target))
	  return true;
	return same == (p.owner() == move.player());
      }
    };

    class DefenseKing8 : public Feature
    {
      Ptype self;
      bool drop;
      int danger;
    public:
      static const std::string name(Ptype self, bool drop, int danger);
      DefenseKing8(Ptype s, bool d, int dan) 
	: Feature(name(s,d,dan)), self(s), drop(d), danger(dan)
      {
      }
      static int count(const NumEffectState& state) 
      {
	const Player attack = alt(state.turn());
	const Square king = state.kingSquare(alt(attack));
	int count = 0;
	for (int dx=-1; dx<=1; ++dx) {
	  for (int dy=-1; dy<=1; ++dy) {
	    if (dx == 0 && dy ==0)
	      continue;
	    Square p = king + Offset(dx,dy);
	    if (! state.pieceAt(p).isEdge()
		&& state.hasEffectAt(attack, p))
	      ++count;
	  }
	}
	if (king.x() == 1 || king.x() == 9)
	  ++count;
	return std::min(3, count);
      }
      static bool blocking(const NumEffectState& state, Square king, Square to)
      {
	const Player attacker = alt(state.turn());
	Piece attack = state.findAttackAt<BISHOP>(attacker, to);
	if (attack.isPiece() 
	    && Neighboring8Direct::hasEffect(state, newPtypeO(attacker, BISHOP), attack.square(), king))
	  return true;
	attack = state.findAttackAt<ROOK>(attacker, to);
	if (attack.isPiece() 
	    && Neighboring8Direct::hasEffect(state, newPtypeO(attacker, ROOK), attack.square(), king))
	  return true;
	attack = state.findAttackAt<LANCE>(attacker, to);
	return attack.isPiece() && attack.ptype() == LANCE
	  && Neighboring8Direct::hasEffect(state, newPtypeO(attacker, LANCE), attack.square(), king);
      }
      static bool matchDrop(const NumEffectState& state, Move move)
      {
	if (! move.isDrop())
	  return false;
	const Square king = state.kingSquare(state.turn());
	if (king.isNeighboring8(move.to())
	    || Neighboring8Direct::hasEffect(state, move.ptypeO(), move.to(), king))
	  return true;
	return blocking(state, king, move.to());
      }
      static bool matchMove(const NumEffectState& state, Move move)
      {
	if (move.isDrop())
	  return false;
	if (move.ptype() == KING)
	  return true;
	const Square king = state.kingSquare(state.turn());
	if (king.isNeighboring8(move.to())
	    || king.isNeighboring8(move.from())
	    || Neighboring8Direct::hasEffect(state, move.ptypeO(), move.to(), king))
	  return true;
	return blocking(state, king, move.to());
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	if (move.ptype() != self)
	  return false;
	if (count(state) != danger)
	  return false;
	if (drop)
	  return matchDrop(state, move);
	return matchMove(state, move);
      }
    };
  }
}

#endif /* RATING_KING8_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
