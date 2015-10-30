/* capture_.h
 */
#ifndef OSL_MOVE_GENERATOR_CAPTURE_H
#define OSL_MOVE_GENERATOR_CAPTURE_H

#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 駒を取る手を生成
     */
    template<class Action>
    class Capture
    {
    public:
      /**
       * @param target 取る駒の位置 (can be empty)
       */
      template<Player P>
      static void generate(const NumEffectState& state,Square target,
			   Action& action);
      /**
       * @param target 取る駒の位置 (can be empty)
       * @param piece  この駒以外で取る
       * before 2009/12/20 pinを考慮していなかった
       */
      template<Player P>
      static void escapeByCapture(const NumEffectState& state,Square target,
				  Piece piece,Action& action);
      /**
       * 取る手を1手だけ作る
       * @param target 取る駒の位置 (can be empty)
       */
      template<Player P>
      static void generate1(const NumEffectState& state,Square target,
			   Action& action);
    };

    /**
     * Capture の Player で特殊化できないバージョン．
     */
    struct GenerateCapture
    {
      template<class Action>
      static void generate(Player p, const NumEffectState& state,Square target,
			   Action& action)
      {
	if (p == BLACK)
	  Capture<Action>::template generate<BLACK>(state,target,action);
	else
	  Capture<Action>::template generate<WHITE>(state,target,action);
      }
      static void generate(Player P, const NumEffectState& state,Square target,
			   MoveVector& out)
      {
	using move_action::Store;
	Store store(out);
	generate(P, state, target, store);
      }
      static void generate(const NumEffectState& state,Square target,
			   MoveVector& out)
      {
	generate(state.turn(), state, target, out);
      }
      template<class Action>
      static void generate1(Player p, const NumEffectState& state,Square target,
			    Action& action)
      {
	if (p == BLACK)
	  Capture<Action>::template generate1<BLACK>(state,target,action);
	else
	  Capture<Action>::template generate1<WHITE>(state,target,action);
      }
      static void generate1(Player P, const NumEffectState& state,Square target,
			   MoveVector& out)
      {
	using move_action::Store;
	Store store(out);
	generate1(P, state, target, store);
      }

      template<class Action>
      static void escapeByCapture(Player p, const NumEffectState& state,Square target,
				  Piece piece,Action& action)
      {
	if (p == BLACK)
	  Capture<Action>::template escapeByCapture<BLACK>(state,target,piece,action);
	else
	  Capture<Action>::template escapeByCapture<WHITE>(state,target,piece,action);
      }
    };
    
  } // namespace move_generator
  using move_generator::GenerateCapture;
} // namespace osl


#endif /* OSL_MOVE_GENERATOR_CAPTURE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
