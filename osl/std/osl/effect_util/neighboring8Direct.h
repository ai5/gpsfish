/* neighboring8Direct.h
 */
#ifndef EFFECT_UTIL_NEIGHBORING8DIRECT_H
#define EFFECT_UTIL_NEIGHBORING8DIRECT_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace effect_util
  {
    /**
     * 8近傍への直接の利きを判定する
     */
    class Neighboring8Direct
    {
      class Table
      {
	struct Entry
	{
	  bool has_unblockable_effect;
	  Offset nearest;
	  Entry() : has_unblockable_effect(false), nearest(Offset::ZERO())
	  {
	  }
	};
	CArray2d<Entry,PTYPEO_SIZE,Offset32::SIZE> table;
	friend class Neighboring8Direct;
	void init(Player);
      public:
	bool hasEffect(const NumEffectState& state,
		       PtypeO ptypeo, Square from, 
		       Square target) const
	{
	  assert(from.isOnBoard());
	  assert(target.isOnBoard());
	  const Offset32 offset32 = Offset32(target, from);
	  const Entry& e = table[ptypeOIndex(ptypeo)][offset32.index()];
	  if (e.has_unblockable_effect)
	    return true;
	  if (e.nearest.zero())
	    return false;
	  assert(Ptype_Table.hasLongMove(getPtype(ptypeo)));
	  const Square nearest = from+e.nearest;
	  if (nearest.isEdge())
	  {
	    return false;
	  }
	  return state.isEmptyBetween(from, nearest, false);
	}
	bool hasEffectOrAdditional(const NumEffectState& state,
				   PtypeO ptypeo, Square from, 
				   Square target) const
	{
	  const Offset32 offset32 = Offset32(target, from);
	  const Entry& e = table[ptypeOIndex(ptypeo)][offset32.index()];
	  if (e.has_unblockable_effect)
	    return true;
	  if (e.nearest.zero())
	    return false;
	  assert(Ptype_Table.hasLongMove(getPtype(ptypeo)));
	  const Square nearest = from+e.nearest;
	  if (nearest.isEdge())
	  {
	    return false;
	  }
	  Offset offset=Board_Table.getShortOffset(Offset32(nearest,from));
	  assert(! offset.zero());
	  Square pos=from+offset;
	  Piece p = state.pieceAt(pos);
	  for (; p.isEmpty(); pos+=offset, p=state.pieceAt(pos)) {
	    if (pos==nearest)
	      return true;
	  }
	  assert(p.isPiece());
	  if (pos == nearest || state.hasEffectByPiece(p, nearest))
	    return true;
	  const Player attack = getOwner(ptypeo);
	  if (target != state.kingSquare(alt(attack)))
	    return false;
	  // new pin?
	  const Direction dir = longToShort(Board_Table.getLongDirection(attack,Offset32(nearest, from)));
	  return pos == state.kingMobilityOfPlayer(alt(attack), dir);
	}
	Square findNearest(const NumEffectState& state,
			     PtypeO ptypeo, Square from, 
			     Square target) const
	{
	  const Offset32 offset32 = Offset32(target, from);
	  const Entry& e = table[ptypeOIndex(ptypeo)][offset32.index()];
	  if (e.has_unblockable_effect)
	    return from;
	  if (e.nearest.zero())
	    return Square::STAND();
	  assert(Ptype_Table.hasLongMove(getPtype(ptypeo)));
	  const Square nearest = from+e.nearest;
	  if (!nearest.isEdge() && state.isEmptyBetween(from, nearest, false))
	    return nearest;
	  return Square::STAND();
	}
      };
      // tables.ccに入れればconstに出来る
      static Table table;
    public:
      /**
       * ptypeo の駒がfromからtargetの8近傍に直接の利きを持つか
       */
      static bool hasEffect(const NumEffectState& state,
			    PtypeO ptypeo, Square from, 
			    Square target)
      {
	return table.hasEffect(state, ptypeo, from, target);
      }
      /**
       * ptypeo の駒がfromからtargetの8近傍に直接の利きを持つか
       * そのような駒への追加/影利きになっている
       */
      static bool hasEffectOrAdditional(const NumEffectState& state,
					PtypeO ptypeo, Square from, 
					Square target)
      {
	return table.hasEffectOrAdditional(state, ptypeo, from, target);
      }
      static Square findNearest(const NumEffectState& state,
				  PtypeO ptypeo, Square from, 
				  Square target)
      {
	return table.findNearest(state, ptypeo, from, target);
      }
    private:
      static bool hasEffectFromTo(const NumEffectState& state,
				  PtypeO ptypeo, Square from, 
				  Square target, Direction d);
    public:
      static bool hasEffectNaive(const NumEffectState& state,
				 PtypeO ptypeo, Square from, 
				 Square target);
      static void init();
    };

  } // namespace effect_util
  using effect_util::Neighboring8Direct;
} // namespace osl

#endif /* EFFECT_UTIL_NEIGHBORING8DIRECT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
