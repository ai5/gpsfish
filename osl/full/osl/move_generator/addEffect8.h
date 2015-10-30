#ifndef OSL_MOVE_GENERATOR_ADD_EFFECT8_H
#define OSL_MOVE_GENERATOR_ADD_EFFECT8_H
#include "osl/numEffectState.h"
#include "osl/move_generator/move_action.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 敵の8近傍に利きを付ける．
     * 王手の手は重複を避けるために，なるべく生成しない
     * (意図せずに王手になるのは仕方がない)
     * promote, captureは生成しない．
     * @param P(template) - 手番の側のプレイヤー
     */
    template<Player P>
    class AddEffect8
    {
    public:
      /**
       * 敵の8近傍に利きを付ける手の生成．
       * 欲しい仕様は8近傍のどこかにこれまで利きのなかった駒の利きが追加されること．
       * \li 全体としてそのマスへの利きが減っても可．
       * (例)
       *   *  *  *      *  * +GI
       *  -OU * +GI  ->-OU *  *
       *   *  *  *      *  *  *
       * 21,23の利きは減っているが22の利きが発生している
       * \li 王手は除外する．
       * \li 王によって8近傍に利きをつける手も除外する．
       * \li 自殺手は生成してしまってもよい．
       * \li 盤外に利きを付けてしまうこともある．
       * 追加利きで対応しない例
       *  +RY *  *  *       +RY+NG *  *  
       *   * +GI * -OU  ->   *  *  * -OU
       *   *  *  *  *        *  *  *  *
       * open attackは同じ手を2回生成してしまうことがある．
       * effectUtilを使えばOK -> 未
       * open attackは王手を生成してしまうことがある．
       * TODO: pawn,rook,bishopが成れるときは常に成る -> DONE
       *       (openによるattackの時は成らないこともある -> 困った)
       * 2段目の香車は必ず成る -> DONE
       * 飛車角は敵の利きのあるところには移動しない
       * 飛車角のdrop以外と飛車角のdrop 
       * 飛車角のdropは全部作らない
       * - 敵の利きのない一番近いところと二番目に近いところのみ作る -> DONE
       * -> 本当は自分のlongをふさがないものが欲しい -> 未
       * 追加利きもつける
       * 歩の前には歩以外は?? -> 未
       * 利きの数が(敵玉も含めて)even以上の時 -> 未
       * @param state - 対象とする局面
       * @param action - 手が存在した時に呼び出すコールバック関数
       */
      template<class Action>
      static void generate(const NumEffectState& state,Action& action);
      static void generate(const NumEffectState& state, MoveVector& out)
      {
	move_action::Store store(out);
	generate(state, store);
      }
      /**
       * 大駒のdrop以外
       */
      template<class Action>
      static void generateNotBigDrop(const NumEffectState& state,Action& action);
      /**
       * 大駒のdrop
       */
      template<class Action>
      static void generateBigDrop(const NumEffectState& state,Action& action);
    };

    struct GenerateAddEffect8
    {
      /**
       * 対象とするマスに利きを付ける手を生成する.
       * Note: Action がPlayerで型付けされていると，無駄があるのでStore限定に
       */
      static void generate(Player player, const NumEffectState& state,
			   move_action::Store& store);
      static void generateBigDrop(Player player, const NumEffectState& state,
			   move_action::Store& store);
      static void generateNotBigDrop(Player player, const NumEffectState& state,
			   move_action::Store& store);
    };
  }
}
#endif /* OSL_MOVE_GENERATOR_ADD_EFFECT8_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
