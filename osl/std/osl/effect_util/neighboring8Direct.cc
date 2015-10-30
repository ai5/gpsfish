/* neighboring8Direct.cc
 */
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/oslConfig.h"
osl::effect_util::Neighboring8Direct::Table osl::effect_util::Neighboring8Direct::table;

namespace
{
  namespace Neighboring8Direct {
    osl::SetUpRegister _initializer([](){ osl::effect_util::Neighboring8Direct::init(); });
  }
}

void osl::effect_util::Neighboring8Direct::
init()
{
  table.init(BLACK); 
  table.init(WHITE);
}

void osl::effect_util::Neighboring8Direct::
Table::init(const Player player)
{
  for (int p=PTYPE_PIECE_MIN; p<=PTYPE_MAX; ++p)
  {
    const Ptype ptype = static_cast<Ptype>(p);
    assert(isPiece(ptype));
    const PtypeO ptypeo = newPtypeO(player, ptype);
    const int mask = Ptype_Table.getMoveMask(ptype);
    for (int d=DIRECTION_MIN; d<=DIRECTION_MAX; ++d)
    {
      const Direction direction = static_cast<Direction>(d);
      if (! (mask & (1<<direction)))
	continue;
      const Offset offset = Board_Table.getOffset(player, direction);
      assert(! offset.zero());
      const int x = offset.dx();
      const int y = offset.dy();
      for (int dy=-1; dy<=1; ++dy)
      {
	for (int dx=-1; dx<=1; ++dx)
	{
	  const Offset32 offset32 = Offset32(x+dx, y+dy);
	  table[ptypeOIndex(ptypeo)][offset32.index()].
	    has_unblockable_effect = true;
	}
      }
      if (isLong(direction))
      {
	assert(abs(x)<=1);
	assert(abs(y)<=1);
	for (int i=1; i<8; ++i)
	{
	  const int long_x = x*i;
	  const int long_y = y*i;
	  const int target_x = x*(i+1);
	  const int target_y = y*(i+1);
	  const Offset32 offset32 = Offset32(target_x, target_y);
	  Entry& e = table[ptypeOIndex(ptypeo)][offset32.index()];
	  e.nearest = Offset(long_x, long_y);
	}
	for (int i=1; i<9; ++i)
	{
	  const int long_x = x*i;
	  const int long_y = y*i;
	  for (int dy=-1; dy<=1; ++dy)
	  {
	    const int target_y = long_y+dy;
	    if ((target_y < -8) || (8 < target_y))
	      continue;
	    for (int dx=-1; dx<=1; ++dx)
	    {
	      const int target_x = long_x+dx;
	      if ((target_x < -8) || (8 < target_x))
		continue;
	      const Offset32 offset32 = Offset32(target_x, target_y);
	      Entry& e = table[ptypeOIndex(ptypeo)][offset32.index()];
	      // 近いところ優先
	      if (e.nearest.zero())
	      {		
		e.nearest = Offset(long_x, long_y);
	      }
	    }
	  }
	}
      }
    }
  }
}

bool osl::effect_util::Neighboring8Direct::
hasEffectFromTo(const NumEffectState& state, PtypeO ptypeo, Square from, 
		Square target, Direction d)
{
  target += Board_Table.getOffsetForBlack(d); // 8 近傍全て試すなら手番による符合変換は不要
  return target.isOnBoard()
    && state.hasEffectIf(ptypeo, from, target);
}

bool osl::effect_util::Neighboring8Direct::
hasEffectNaive(const NumEffectState& state, PtypeO ptypeo, Square from, 
	       Square target)
{
  const Ptype ptype = getPtype(ptypeo);
  if (! Ptype_Table.hasLongMove(ptype))
  {
    if (abs(from.y() - target.y()) > 3)	// knight だけ3
      return false;
    if (abs(from.x() - target.x()) > 2)
      return false;
  }
  else if (ptype == LANCE)
  {
    if (abs(from.x() - target.x()) > 1)
      return false;
  }

  // naive な実装
  return hasEffectFromTo(state, ptypeo, from, target, UL)
    || hasEffectFromTo(state, ptypeo, from, target, U)
    || hasEffectFromTo(state, ptypeo, from, target, UR)
    || hasEffectFromTo(state, ptypeo, from, target, L)
    || hasEffectFromTo(state, ptypeo, from, target, R)
    || hasEffectFromTo(state, ptypeo, from, target, DL)
    || hasEffectFromTo(state, ptypeo, from, target, D)
    || hasEffectFromTo(state, ptypeo, from, target, DR);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
