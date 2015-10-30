/* square.h
 */
#ifndef FEATURE_POSITION_H
#define FEATURE_POSITION_H

#include "osl/rating/feature.h"
#include "osl/basic_type.h"
#include "osl/bits/ptypeTable.h"
#include <string>

namespace osl
{
  namespace rating
  {
    struct DropPtype
    {
      Ptype ptype;
      bool drop;
      DropPtype(Ptype p, bool d) : ptype(p), drop(d)
      {
      }
      bool match(Move m) const
      {
	return (m.ptype() == ptype) && (m.isDrop() == drop);
      }
      static std::string name(Ptype ptype, bool drop) 
      {
	return std::string(Ptype_Table.getCsaName(ptype)) + (drop ? "d" : "m");
      }
      enum { UNIT = PTYPE_MAX+1-PTYPE_PIECE_MIN + (PTYPE_MAX+1 - (PTYPE_BASIC_MIN+1)) };
      static int index(Move move) 
      {
	const Ptype ptype = move.ptype();
	int index;
	if (! isBasic(ptype) || ptype == KING)
	  index = ptype - PTYPE_PIECE_MIN;
	else
	  index = (PTYPE_BASIC_MIN+1-PTYPE_PIECE_MIN) + (ptype - (PTYPE_BASIC_MIN+1))*2 + move.isDrop();
	return index;
      }
    };

    class RelativeKingX : public Feature
    {
      int x, old_x;
      Ptype ptype;
      bool attack;
    public:
      static const std::string name(int x, int old_x, bool /*attack*/, Ptype);
      RelativeKingX(int ix, int iox, bool a, Ptype p) 
	: Feature(name(ix,iox,a,p)), x(ix), old_x(iox), ptype(p), attack(a) {}
      // [0,8]
      static int makeX(bool attack, const NumEffectState& state, Move move) 
      {
	const Square king = state.kingSquare(attack ? alt(move.player()) : move.player());
	return abs(move.to().x() - king.x());
      }
      // [0,9]
      static int makeOldX(bool attack, const NumEffectState& state, Move move) 
      {
	if (move.isDrop())
	  return 9;
	const Square king = state.kingSquare(attack ? alt(move.player()) : move.player());
	return abs(move.from().x() - king.x());
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	if (ptype != move.ptype())
	  return false;
	return makeX(attack, state, move) == x && makeOldX(attack, state, move) == old_x;
      }
      static int index(bool attack, const NumEffectState& state, Move move)
      {
	const int x = makeX(attack, state, move), old_x = makeOldX(attack, state, move);
	const int ptype = move.ptype() - PTYPE_PIECE_MIN;
	return (x*10+old_x)*(PTYPE_MAX+1 - PTYPE_PIECE_MIN) + ptype;
      }
    };

    class RelativeKingY : public Feature
    {
      int y, old_y;
      Ptype ptype;
      bool attack;
    public:
      static const std::string name(int y, int old_y, bool /*attack*/, Ptype ptype);
      RelativeKingY(int iy, int ioy, bool a, Ptype p) 
	: Feature(name(iy,ioy,a,p)), y(iy), old_y(ioy), ptype(p), attack(a) {}
      // [-8,8]
      static int makeY(bool attack, const NumEffectState& state, Move move)
      {
	const Square king = state.kingSquare(attack ? alt(move.player()) : move.player());
	int diff = move.to().y() - king.y();
	if (move.player() == WHITE)
	  diff = -diff;
	return diff;
      }
      // [-8,9]
      static int makeOldY(bool attack, const NumEffectState& state, Move move)
      {
	if (move.isDrop())
	  return 9;
	const Square king = state.kingSquare(attack ? alt(move.player()) : move.player());
	int diff = move.from().y() - king.y();
	if (move.player() == WHITE)
	  diff = -diff;
	return diff;
      }
      bool match(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	if (ptype != move.ptype())
	  return false;
	return makeY(attack, state, move) == y && makeOldY(attack, state, move) == old_y;
      }
      static int index(bool attack, const NumEffectState& state, Move move)
      {
	const int y = makeY(attack, state, move) + 8;
	const int old_y = makeOldY(attack, state, move) + 8;
	const int ptype = move.ptype() - PTYPE_PIECE_MIN;
	return (y*18+old_y)*(PTYPE_MAX+1 - PTYPE_PIECE_MIN) + ptype;
      }
    };

    class SquareX : public Feature, DropPtype
    {
      int x;
      static const std::string name(int x);
    public:
      SquareX(int ix, Ptype ptype, bool drop) 
	: Feature(name(ix)+DropPtype::name(ptype, drop)), DropPtype(ptype, drop), x(ix) {}
      static int makeX(Move move)
      {
	int mx = move.to().x();
	if (mx > 5)
	  mx = 10-mx;
	return mx;
      }
      bool match(const NumEffectState& , Move move, const RatingEnv&) const
      {
	if (! DropPtype::match(move))
	  return false;
	return x == makeX(move);
      }
    };

    class SquareY : public Feature, DropPtype
    {
      int y;
      static const std::string name(int y);
    public:
      SquareY(int iy, Ptype ptype, bool drop) 
	: Feature(name(iy)+DropPtype::name(ptype, drop)), DropPtype(ptype, drop), y(iy) {}
      static int makeY(Move move) 
      {
	int my = move.to().y();
	if (move.player() == WHITE)
	  my = 10 - my;
	return my;
      }
      bool match(const NumEffectState&, Move move, const RatingEnv&) const
      {
	if (! DropPtype::match(move))
	  return false;
	return y == makeY(move);
      }
    };
  }
}

#endif /* FEATURE_POSITION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
