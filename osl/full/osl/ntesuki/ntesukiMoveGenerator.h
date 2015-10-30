/* ntesukiMoveGenerator.h
 */
#ifndef _NTEUSKI_MOVE_GENERATOR_H
#define _NTEUSKI_MOVE_GENERATOR_H

#include "osl/ntesuki/ntesukiMoveList.h"
#include "osl/state/numEffectState.h"

namespace osl {
  namespace ntesuki {
    class NtesukiRecord;
    /**
     * ntesuki で使う move generator の base class.
     * - 基本的には全ての手を生成.
     * - 王手は CHECKMATE_FLAG をたてる.
     * - 攻撃に関係しそうな手は ATTACK_FLAG をたてる.
     */
    struct
    NtesukiMoveGenerator
    {
      bool verbose;

      NtesukiMoveGenerator(bool verbose = false);
      ~NtesukiMoveGenerator();
      template <Player T>
      void generate(const NumEffectState& state,
		    NtesukiMoveList& moves);
      void generateSlow(const Player T,
			const NumEffectState& state,
			NtesukiMoveList& moves)
      {
	if (T == BLACK)
	  generate<BLACK>(state, moves);
	else
	  generate<WHITE>(state, moves);
      }

      template <Player T>
      void generateWithRzone(const NumEffectState& state,
			     NtesukiRecord *record,
			     int pass_left,
			     NtesukiMoveList& moves);
      void generateWithRzoneSlow(const Player T,
				 const NumEffectState& state,
				 NtesukiRecord *record,
				 int pass_left,
				 NtesukiMoveList& moves)
      {
	if (T == BLACK)
	  generateWithRzone<BLACK>(state, record, pass_left, moves);
	else
	  generateWithRzone<WHITE>(state, record, pass_left, moves);
      }

    private:
      template <Player T>
      void setOrder(const NumEffectState& state,
		    NtesukiMoveList& moves);

    };

  } //ntesuki
} //osl
#endif /* _NTEUSKI_MOVE_GENERATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
