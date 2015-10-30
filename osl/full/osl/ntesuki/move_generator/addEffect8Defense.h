#ifndef _GENERATE_ADD_EFFECT8_DEFENSE_H
#define _GENERATE_ADD_EFFECT8_DEFENSE_H
#include "osl/move_generator/pieceOnBoard.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_action/store.h"
#include "osl/centering3x3.h"
#include "osl/state/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 玉の 8近傍に利きをつける手を生成
     * - 玉自身が動く手も生成
     * - 自殺手は生成しない
     */
    template <Player P>
    struct AddEffect8Defense
    {
      static void generateTo(const NumEffectState& state,
			     Square p,
			     move_action::Store& action)
      {
	if (!p.isOnBoard()) return;
	GenerateAddEffectWithEffect::generate<false>(P, state, p, action);
      }

      static void generate(const NumEffectState& state, MoveVector& moves)
      {
	const Square king =  state.template kingSquare<P>();
	const Square center = Centering3x3::adjustCenter(king);
	{
	move_action::Store action(moves);
	generateTo(state, center, action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(UL),
		   action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(U),
		   action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(UR),
		   action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(L),
		   action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(R),
		   action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(DL),
		   action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(D),
		   action);
	generateTo(state,
		   center + Board_Table.getOffsetForBlack(DR),
		   action);
	}
	moves.unique();
      }
    };
  }
} // namespace osl

#endif /* _GENERATE_ADD_EFFECT8_DEFENSE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
