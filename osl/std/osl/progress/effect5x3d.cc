/* effect5x3d.cc
 */
#include "osl/progress/effect5x3d.h"
#include "osl/progress/effect5x3Table.h"
#include "osl/bits/centering5x3.h"

int osl::progress::
Effect5x3d::makeProgress(Player defense, const NumEffectState& state,
			 Square king)
{
  
  const Square center = Centering5x3::adjustCenter(king);

  const int min_x = center.x() - 2;
  const int min_y = center.y() - 1;

  // 利き
  int sum_effect = 0;

   for (int dx=0; dx<5; ++dx)
    {
      for (int dy=0; dy<3; ++dy)
	{
	  const Square target(min_x+dx,min_y+dy);
	  sum_effect += state.countEffect(defense, target) *
	    Effect5x3_Table.getDefenseEffect(defense,king,dx,dy);
	}
    }

  return sum_effect / 2;
}

void osl::progress::
Effect5x3d::update(const NumEffectState& new_state, Move /*last_move*/)
{
  const Square kb = new_state.kingSquare<BLACK>(), kw = new_state.kingSquare<WHITE>();
  BoardMask mb = new_state.changedEffects(BLACK), mw = new_state.changedEffects(WHITE);

  if (mb.anyInRange(Board_Mask_Table5x3_Center.mask(kb)))
    progresses[BLACK]=makeProgress(BLACK,new_state,new_state.kingSquare<BLACK>());
  if (mw.anyInRange(Board_Mask_Table5x3_Center.mask(kw)))
    progresses[WHITE]=makeProgress(WHITE,new_state,new_state.kingSquare<WHITE>());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
