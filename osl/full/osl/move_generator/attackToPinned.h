#ifndef OSL_MOVE_GENERATOR_ATTACK_TO_PINNED_H
#define OSL_MOVE_GENERATOR_ATTACK_TO_PINNED_H
#include "osl/numEffectState.h"
#include "osl/move_generator/move_action.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 敵のpinされている駒を攻める.
     * pinの長い利きを付けている駒で攻める可能性もあるが稀
     * pinしている方向からは攻めてしまうこともある．
     * 歩も攻める．
     * 利きの数が勝っているかどうかはとりあえずは問わない．
     * P - 攻撃側のプレイヤ
     */
    template<Player P>
    class AttackToPinned
    {
    public:
      /**
       * 手を生成する．
       * @param state - 盤面
       * @param action - 生成時のcall back
       */
      template<class Action>
      static void generate(const NumEffectState& state,Action& action);
      static void generate(const NumEffectState& state,MoveVector& out)
      {
	move_action::Store store(out);
	generate(state, store);
      }
    };
    struct GenerateAttackToPinned
    {
      static void generate(Player player,const NumEffectState& state,
			   move_action::Store& store);
    };
  }
}

#endif /* OSL_MOVE_GENERATOR_ATTACK_TO_PINNED_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
