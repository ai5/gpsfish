/* oracleProverLight.h
 */
#ifndef _ORACLEPROVERLIGHT_H
#define _ORACLEPROVERLIGHT_H

#include "proofOracle.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/state/numEffectState.h"
#include "osl/player.h"
namespace osl
{
  namespace checkmate
  {
    /**
     * 表を使わない OracleProver
     */
    template <Player Attacker>
    class OracleProverLight
    {
    public:
      typedef NumEffectState state_t;
    private:
      state_t *state;
      FixedDepthSearcher fixed_searcher;
      int node_count;
      Move best_move;
    public:
      OracleProverLight() : node_count(0)
      {
      }
      typedef ProofOracleAttack<Attacker> attack_oracle_t;
      typedef ProofOracleDefense<Attacker> defense_oracle_t;
      /** 
       * oracle に基づき詰められるかどうかを判定する
       * @param best_move 詰む場合に詰ます手で上書きする
       */
      bool proofWin(state_t& state, attack_oracle_t oracle, 
		    Move& best_move);
      /** 
       * oracle に基づき詰んでいるかどうかを判定する
       * @param state に alt(Attacker) に対して王手がかかっていること
       */
      bool proofLose(state_t& state, defense_oracle_t oracle,
		     Move last_move=Move::INVALID());

      int nodeCount() const { return node_count; }

      // private:
      const ProofDisproof attack(attack_oracle_t oracle);
      const ProofDisproof defense(defense_oracle_t oracle);
    };

  } // namespace checkmate
} // namespace osl

#endif /* _ORACLEPROVERLIGHT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
