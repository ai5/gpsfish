/* feature.h
 */
#ifndef _FEATURE_H
#define _FEATURE_H

#include "osl/rating/ratingEnv.h"
#include "osl/numEffectState.h"
#include <string>

namespace osl
{
  namespace rating
  {
    class Feature
    {
      std::string my_name;
    public:
      Feature(const std::string& name) : my_name(name)
      {
      }
      virtual ~Feature();
      virtual bool match(const NumEffectState& state, Move, const RatingEnv&) const =0;
      virtual bool effectiveInCheck() const { return false; }
      const std::string& name() const { return my_name; }
    };

    class TakeBack : public Feature
    {
    public:
      TakeBack() : Feature("TakeBack")
      {
      }
      bool match(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	return env.history.hasLastMove() && move.to() == env.history.lastMove().to();
      }
      virtual bool effectiveInCheck() const { return true; }
    };

    class TakeBack2 : public Feature
    {
    public:
      TakeBack2() : Feature("TakeBack2")
      {
      }
      bool match(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	return env.history.hasLastMove(2)
	  && move.to() == env.history.lastMove().to()
	  && move.to() == env.history.lastMove(2).to();
      }
      bool effectiveInCheck() const { return true; }
    };


    class Check : public Feature
    {
      int property;
    public:
      Check(int p);
      static bool openLong(const NumEffectState& state, Move move) 
      {
	if (move.isDrop())
	  return false;
	return state.hasEffectByPtype<LANCE>(move.player(), move.from())
	  || state.hasEffectByPtype<BISHOP>(move.player(), move.from())
	  || state.hasEffectByPtype<ROOK>(move.player(), move.from());
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const;
      static const CArray<const char*,4> check_property;
      bool effectiveInCheck() const { return true; }
    };

    class SendOff : public Feature
    {
      bool capture;
    public:
      SendOff(bool c) : Feature("SO"), capture(c) {}
      bool match(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	return env.sendoffs.isMember(move.to()) && (move.capturePtype() !=PTYPE_EMPTY) == capture;
      }
    };

    class Block : public Feature
    {
      int self, opponent;
    public:
      static const std::string name(int self, int opponent);
      Block(int s, int o) : Feature(name(s, o)), self(s), opponent(o) {}
      static int count(const NumEffectState& state, Square position, Player player) 
      {
	return (state.findAttackAt<LANCE>(player, position).ptype() == LANCE)
	  + state.hasEffectByPtype<BISHOP>(player, position)
	  + state.hasEffectByPtype<ROOK>(player, position);
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return count(state, move.to(), state.turn()) == self
	  && count(state, move.to(), alt(state.turn())) == opponent;
      }
      bool effectiveInCheck() const { return true; }
    };

    // { none, tate, naname, both }
    struct CountOpen
    {
      static int index(const NumEffectState& state, Player player, Square from) 
      {
	if (from.isPieceStand())
	  return -1;
	const bool vertical = state.hasEffectByPtype<LANCE>(player, from)
	  || state.hasEffectByPtype<ROOK>(player, from);
	const bool diagonal = state.hasEffectByPtype<BISHOP>(player, from);
	return diagonal*2+vertical;
      }
    };
    // { none, tate, naname, both } * { none, tate, naname, both }
    class Open : public Feature
    {
      int property;
    public:
      Open(int p) : Feature(name(p)), property(p) {}
      static int index(const NumEffectState& state, Move move)
      {
	if (move.isDrop())
	  return -1;
	return CountOpen::index(state, move.player(), move.from())*4
	  + CountOpen::index(state, alt(move.player()), move.from());
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	return index(state, move) == property;
      }
      static const std::string name(int property);
      bool effectiveInCheck() const { return true; }
    };

    class Chase : public Feature
    {
    public:
      enum OpponentType { CAPTURE, DROP, ESCAPE, OTHER, };
    private:
      Ptype self, target;
      bool drop;
      OpponentType opponent_type;
    public:
      Chase(Ptype s, Ptype t, bool d, OpponentType o) 
	: Feature(name(s,t,d,o)), self(s), target(t), drop(d), opponent_type(o) {}
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const Move last_move = env.history.lastMove();
	if (! last_move.isNormal())
	  return false;
	if (! (move.ptype() == self && last_move.ptype() == target
	       && drop == move.isDrop()))
	  return false;
	switch (opponent_type) {
	case CAPTURE:
	  if (last_move.capturePtype() == PTYPE_EMPTY)
	    return false;
	  break;
	case DROP:
	  if (! last_move.isDrop())
	    return false;
	  break;
	case ESCAPE:
	  if (last_move.isDrop() || last_move.capturePtype() != PTYPE_EMPTY
	      || ! state.hasEffectAt(state.turn(), last_move.from()))
	    return false;
	  break;
	case OTHER:
	  if (last_move.isDrop() || last_move.capturePtype() != PTYPE_EMPTY)
	    return false;
	  break;
	}
	return state.hasEffectIf
(move.ptypeO(), move.to(), last_move.to());
      }
      static const std::string name(Ptype, Ptype, bool, OpponentType);
    };

    class ImmediateAddSupport : public Feature
    {
      struct Test;
      Ptype self, attack;
    public:
      ImmediateAddSupport(Ptype self, Ptype attack);
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const;
      bool effectiveInCheck() const { return true; }
      static int index(const NumEffectState& state, Move move, const RatingEnv& env);
    };

    class RookDefense : public Feature
    {
    public:
      RookDefense() : Feature("RookDefense") 
      {
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.isDrop() || env.progress.value() > 8)
	  return false;
	Piece rook1 = state.pieceOf(PtypeTraits<ROOK>::indexMin);
	Piece rook2 = state.pieceOf(PtypeTraits<ROOK>::indexMin + 1);
	if (move.from() == rook2.square()) 
	  std::swap(rook1, rook2);
	if (move.from() != rook1.square() 
	    || rook2.square().isPieceStand()
	    || rook2.owner() == move.player()
	    || rook2.square().x() != move.to().x())
	  return false;
	return (move.to().y() - rook2.square().y())*sign(move.player()) > 0;
      }
    };

    class BadLance : public Feature
    {
      bool has_effect;
    public:
      explicit BadLance(bool h) : Feature(h ? "StrongBadLance" : "WeakBadLance"), has_effect(h)
      {
      }
      static bool basicMatch(const NumEffectState& state, Move move, Square front) 
      {
	if (! (move.isDrop() && move.ptype() == LANCE))
	  return false;
	return state.pieceOnBoard(front).isEmpty()
	  && state.hasPieceOnStand<PAWN>(alt(move.player()))
	  && !state.isPawnMaskSet(alt(move.player()), front.x());
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	const Square front = Board_Table.nextSquare(move.player(), move.to(), U);
	return basicMatch(state, move, front)
	  && ((!has_effect) ^ state.hasEffectAt(alt(move.player()), front));
      }
    };

    class PawnAttack : public Feature
    {
    public:
      PawnAttack() : Feature("PA") 
      {
      }
      bool match(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	if (! (move.isDrop() && move.ptype() == PAWN))
	  return false;
	const Move last_move = env.history.lastMove();
	if (! last_move.isNormal() || last_move.capturePtype() == PTYPE_EMPTY)
	  return false;
	return last_move.capturePtype() == PAWN && last_move.to().x() == move.to().x();
      }
    };
    
  }
}


#endif /* _FEATURE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
