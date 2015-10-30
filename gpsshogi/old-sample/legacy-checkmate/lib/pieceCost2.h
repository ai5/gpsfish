/* pieceCost2.h
 */
#ifndef _PIECECOST2_H
#define _PIECECOST2_H
#include "osl/checkmate/pieceCost.h"
namespace osl
{
  namespace checkmate
  {
    struct PieceCost2 : public PieceCost
    {	    
      /** 攻撃側の move に対する cost を計算する */
      template <class State>
      static void setAttackCost(Player attacker, const State&, 
				CheckMove& move);
      /** 防御側の move に対する cost を計算する */
      template <class State>
      static void setDefenseCost(Player, const State&, CheckMove&)
      {
      }
    };
  }
}

template<typename State> inline
void osl::checkmate::PieceCost2::
setAttackCost(Player attacker, const State& state, CheckMove& move)
{
  const Square from=move.move.from();
  const Square to=move.move.to();
  const Ptype capturePtype = move.move.capturePtype();
  const Player defender = alt(attacker);
  // 駒取り以外で，ただでとられる駒は後回し
  // おそらく歩頭桂や送り金など良くでる手筋をセットにするとなお良い
  if (capturePtype == PTYPE_EMPTY)
  {
    const int a = (state.countEffect(attacker,to) 
		   + (from.isPieceStand() ? 1 : 0));
    int d = state.countEffect(defender,to);
    if (a <= d)
    {
      const Ptype ptype = move.move.ptype();
      move.cost_proof = attack_sacrifice_cost[ptype] /* *8 */;
      if ((d >= 2) && (a == d))	// 追加利きとか利きがずれたりとか
	move.cost_proof /= 2;
    }
  }
}
#endif /* _PIECECOST2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
