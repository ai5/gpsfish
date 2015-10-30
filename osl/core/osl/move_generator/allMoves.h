#ifndef OSL_GENERATE_ALL_MOVES_H
#define OSL_GENERATE_ALL_MOVES_H
#include "osl/numEffectState.h"
#include "osl/move_generator/move_action.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * Move::ignoreUnpromote() でないすべての手を生成
     * @param Action move_action
     */
    template<class Action>
    class AllMoves
    {
    public:
      /**
       * @param P - 手番のプレイヤ
       * state - 手を生成する局面．王手がかかっていないことを想定
       * action - 手正成用のcallback
       */
      template<Player P>
      static void generateOnBoard(const NumEffectState& state, Action& action);

      /**
       * @param P - 手番のプレイヤ
       * state - 手を生成する局面．王手がかかっていないことを想定
       * action - 手正成用のcallback
       */
      template<Player P>
      static void generate(const NumEffectState& state, Action& action);

      static void generate(Player p, const NumEffectState& state, Action& action)
      {
	if(p==BLACK)
	  generate<BLACK>(state,action);
	else
	  generate<WHITE>(state,action);
      }
    };

  } // namespace move_generator

  struct GenerateAllMoves
  {
    static void generate(Player p, const NumEffectState& state, MoveVector&);
    template <Player P>
    static void generate(const NumEffectState& state, MoveVector& out)
    {
      typedef move_action::Store store_t;
      store_t store(out);
      move_generator::AllMoves<store_t>::generate<P>(state, store);
    }
    template <Player P>
    static void generateOnBoard(const NumEffectState& state, MoveVector& out)
    {
      typedef move_action::Store store_t;
      store_t store(out);
      move_generator::AllMoves<store_t>::generateOnBoard<P>(state, store);
    }
  };
} // namespace osl

#endif /* OSL_GENERATE_ALL_MOVES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
