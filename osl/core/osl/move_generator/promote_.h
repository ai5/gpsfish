#ifndef OSL_MOVE_GENERATOR_PROMOTE_H
#define OSL_MOVE_GENERATOR_PROMOTE_H
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 成る手を生成.
     * 生成される手はunique
     * 自殺手のチェックはしないので，後で修正する?
     * @param  noCapture - trueの時は駒を取る手は生成しない(取る手は別に生成される可能性が高いので)
     */
    template<Player P,bool NoCapture=true>
    class Promote
    {
    public:
      template<class Action, Ptype T>
      static void generateMovesPtype(const NumEffectState& state, Action& action);
      template<Ptype T>
      static void generatePtype(const NumEffectState& state, MoveVector& out)
      {
	move_action::Store store(out);
	generateMovesPtype<move_action::Store,T>(state, store);
      }
      template<class Action>
      static void generateMoves(const NumEffectState& state, Action& action);

      static void generate(const NumEffectState& state, MoveVector& out)
      {
	move_action::Store store(out);
	generateMoves(state, store);
      }
    };

    /**
     * Player で特殊化した Action でinstantiate すると無駄なので注意．
     */
    template<bool NoCapture>
    struct GeneratePromote
    {
      template<class Action>
      static void generate(Player p, const NumEffectState& state, Action& action)
      {
	if (p == BLACK)
	  Promote<BLACK,NoCapture>::generateMoves(state, action);
	else
	  Promote<WHITE,NoCapture>::generateMoves(state, action);
      }
    };
  }
}
#endif /* OSL_MOVE_GENERATOR_PROMOTE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
