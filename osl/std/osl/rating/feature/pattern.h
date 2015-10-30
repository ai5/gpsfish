/* patternLong2.h
 */
#ifndef _PATTERNLONG2_H
#define _PATTERNLONG2_H

#include "osl/rating/feature.h"
#include "osl/rating/feature/countEffect2.h"

namespace osl
{
  namespace rating
  {
    class Pattern : public Feature, CountEffect2
    {
    public:
      static const Direction INVALID = LONG_UL;
    private:
      Direction direction, direction2;
      Ptype self, target;
      bool same;
      static const std::string name(Direction d, Direction d2, Ptype self, Ptype target, bool same);
    public:
      Pattern(Direction d, Ptype s, Ptype t, bool ss, int attack, int defense) 
	: Feature(name(d, INVALID, s, t, ss)+CountEffect2::name(attack, defense)), CountEffect2(attack, defense),
	  direction(d), direction2(INVALID), self(s), target(t), same(ss)
      {
      }
      Pattern(Direction d, Direction d2, Ptype s, Ptype t, bool ss, int attack, int defense) 
	: Feature(name(d, d2, s, t, ss)+CountEffect2::name(attack, defense)), CountEffect2(attack, defense),
	  direction(d), direction2(d2), self(s), target(t), same(ss)
      {
      }
      static Square nextSquare(Player player, Square start, Direction direction, Direction direction2)
      {
	const Square p = Board_Table.nextSquare(player, start, direction);
	if (direction2 == INVALID || ! p.isOnBoard())
	  return p;
	return Board_Table.nextSquare(player, p, direction2);
      }
      static Square nextSquare(Move move, Direction direction, Direction direction2)
      {
	return nextSquare(move.player(), move.to(), direction, direction2);
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.ptype() != self)
	  return false;
	const Square position = nextSquare(move, direction, direction2);
	const Piece p = (position == move.from()) ? Piece::EMPTY() : state.pieceAt(position);
	if (p.ptype() != target)
	  return false;
	if (! CountEffect2::match(state, position, env))
	  return false;
	if (!isPiece(target))
	  return true;
	return same == (p.owner() == move.player());
      }
    };

    typedef std::pair<Piece,Square> PieceSquare;
    struct LongTarget : CountEffect2
    {
      Ptype target;
      bool promotable, same;
      LongTarget(Ptype t, bool p, bool s, int attack, int defense)
	: CountEffect2(attack, defense), target(t), promotable(p), same(s)
      {
      }
      static bool isPromotable(Move move, Square position) 
      {
	assert(position.isOnBoard());
	return position.canPromote(move.player()) || (! move.isDrop() && move.to().canPromote(move.player()));
      }
      bool match(const NumEffectState& state, Move move, PieceSquare p, const RatingEnv& env) const
      {
	if (p.first.ptype() != target)
	  return false;
	if (target == PTYPE_EDGE)
	  return true;
	if (isPromotable(move, p.second) != promotable)
	  return false;
	if (! CountEffect2::match(state, p.second, env))
	  return false;
	if (! p.first.isPiece())
	  return true;
	return same == (p.first.owner() == move.player());
      }
      bool matchOtherThanPromotable(const NumEffectState& state, Move move, PieceSquare p, const RatingEnv& env) const
      {
	if (p.first.ptype() != target)
	  return false;
	if (target == PTYPE_EDGE)
	  return true;
	if (! CountEffect2::match(state, p.second, env))
	  return false;
	if (! p.first.isPiece())
	  return true;
	return same == (p.first.owner() == move.player());
      }
      const std::string name() const;
    };

    struct LongTarget2
    {
      Ptype target;
      bool same;
      LongTarget2(Ptype t, bool s) : target(t), same(s)
      {
      }
      bool match(const NumEffectState&, Move move, Piece p) const
      {
	if (p.ptype() != target)
	  return false;
	assert(! p.isEmpty());
	if (! isPiece(target))
	  return true;
	return same == (p.owner() == move.player());
      }
      const std::string name() const;
    };

    class PatternLong : public Feature
    {
      Direction direction;
      Ptype self;
      LongTarget target;
      static const std::string name(Direction d, Ptype self);
    public:
      PatternLong(Direction d, Ptype s, LongTarget t);
      /**
       * direction方向に空白を進み、駒を探す
       * - 駒を発見 -> その駒
       * - 盤の外
       * -- その手前が空白 -> その空白
       * -- その手前がスタート地点 -> PTYPE_EDGE
       */
      static const PieceSquare nextPieceOrEnd(const SimpleState& state, Square start, Player player, Direction direction);
      static const PieceSquare nextPieceOrEnd(const SimpleState& state, Square start, Offset);
      static const PieceSquare find(const NumEffectState& state, Move move, Direction direction);
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.ptype() != self)
	  return false;
	PieceSquare pp = find(state, move, direction);
	return target.match(state, move, pp, env);
      }
    };

    class PatternLong2 : public Feature
    {
      Direction direction;
      Ptype self;
      LongTarget2 target2;
      static const std::string name(Direction d, Ptype self);
    public:
      PatternLong2(Direction d, Ptype s, LongTarget2 t2);
      static const Piece find(const NumEffectState& state, Move move, Direction direction) 
      {
	Piece p = state.nextPiece(move.to(), Board_Table.getOffset(move.player(), direction));
	if (p.isPiece() && p.square() == move.from())
	  p = state.nextPiece(move.to(), Board_Table.getOffset(move.player(), direction));
	return p;
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	if (move.ptype() != self)
	  return false;
	const Piece p = find(state, move, direction);
	return target2.match(state, move, p);
      }
    };

    class PatternBlock : public Feature
    {
      Ptype self, attack;
      LongTarget target;
    public:
      PatternBlock(Ptype s, Ptype a, LongTarget t);
      static const PieceSquare find(const NumEffectState& state, Move move, Ptype attacker_ptype);
      bool match(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (move.ptype() != self)
	  return false;
	PieceSquare pp = find(state, move, attack);
	return ! pp.first.isEdge() && target.matchOtherThanPromotable(state, move, pp, env)
	  && pp.second.canPromote(pp.first.isPiece() ? alt(pp.first.owner()) : alt(move.player()))
	  == target.promotable;
      }
    };
  }
}

#endif /* _PATTERNLONG2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
