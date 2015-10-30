#ifndef OSL_MOVE_GENERATER_ADD_EFFECT_WITH_EFFECT_H
#define OSL_MOVE_GENERATER_ADD_EFFECT_WITH_EFFECT_H
#include "osl/move_generator/move_action.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 利きをつける手を生成 利きを持つstateでしか使えない.
     * アルゴリズム:
     * \li 利きをつけたいマスから8近傍方向(長い利きも)，桂馬近傍の自分の利きをチェック
     * \li 自分の利きがあった時に，そこに移動したら問題のマスに利きをつけられる駒の種類かをチェックする
     * 特徴:
     * \li 相手玉の自由度が小さく，近傍に自分の利きがない時は高速に判定
     * isAttackToKing == true の時
     *  既に王手がかかっている状態は扱わない
     *  自殺手は生成しない?
     * isAttackToKing == false の時
     *  Additional Effect(利きが付いている方向の後ろに長い利きを足す)はいくつでも扱う．
     *  Shadow Effect(相手の利きが付いている方向の後ろに味方の長い利きを足す)は相手が1つの時だけ足す．
     *  自殺手は生成しない．
     */
    template<class Action>
    class AddEffectWithEffect
    {
    public:
      template<Player P,bool isAttackToKing>
      static void generate(const NumEffectState& state,Square target,Action& action,bool& hasPawnCheckmate);
      template<Player P,bool isAttackToKing>
      static void generate(const NumEffectState& state,Square target,Action& action){
	bool dummy;
	generate<P,isAttackToKing>(state,target,action,dummy);
      }
    };
    struct GenerateAddEffectWithEffect
    {
      /**
       * 対象とするマスに利きを付ける手を生成する.
       * Note: Action がPlayerで型付けされていると，無駄があるのでStore限定に
       */
      template<bool isAttackToKing>
      static void generate(Player player, const NumEffectState& state, Square target, 
			   move_action::Store& store);
      template<bool isAttackToKing>
      static void generate(Player player,const NumEffectState& state,Square target,MoveVector& out, bool& has_pawn_checkmate) {
	move_action::Store store(out);
	if(player==BLACK)
	   AddEffectWithEffect<move_action::Store>::template generate<BLACK,isAttackToKing>(state,target,store,has_pawn_checkmate);
	else
	   AddEffectWithEffect<move_action::Store>::template generate<WHITE,isAttackToKing>(state,target,store,has_pawn_checkmate);
      }
      template<bool isAttackToKing>
      static void generate(Player player,const NumEffectState& state,Square target,MoveVector& out) {
	bool dummy;
	generate<isAttackToKing>(player,state,target,out,dummy);
      }
    };
  } // namespace move_generator
} // namespace osl
#endif /* OSL_MOVE_GENERATER_ADD_EFFECT_WITH_EFFECT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
