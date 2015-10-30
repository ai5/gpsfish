/* oracleProver.h
 */
#ifndef _ORACLEPROVER_H
#define _ORACLEPROVER_H

#include "proofOracle.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/state/numEffectState.h"
#include "osl/hash/hashKey.h"
#include "osl/player.h"
#include "osl/pathEncoding.h"
namespace osl
{
  namespace checkmate
  {
    /**
     * Simulationによる詰の確認.
     * 攻撃側は類似局面で詰める手のみを試す．
     */
    template <class Table>
    class OracleProver
    {
    public:
      typedef NumEffectState state_t;
    private:
      Table& table;
      state_t *state;
      HashKey key;
      PathEncoding path;
      Player attacker;
      FixedDepthSearcher fixed_searcher;
      int node_count;
    public:
      explicit OracleProver(Table& t) : table(t), attacker(t.getAttacker()),
					node_count(0)
      {
      }
      /** 
       * oracle に基づき詰められるかどうかを判定する
       * @param best_move 詰む場合に詰ます手で上書きする
       */
      template <Player Attacker>
      bool proofWin(state_t& state, const HashKey& key,
		    const PathEncoding& path,
		    ProofOracleAttack<Attacker> oracle, 
		    Move& best_move);
      /** 
       * oracle に基づき詰んでいるかどうかを判定する
       * @param state に alt(Attacker) に対して王手がかかっていること
       */
      template <Player Attacker>
      bool proofLose(state_t& state, const HashKey& key,
		     const PathEncoding& path,
		     ProofOracleDefense<Attacker> oracle,
		     Move last_move=Move::INVALID());

      int nodeCount() const { return node_count; }

      // private:
      template <Player P>
      void attack(CheckHashRecord *record, ProofOracleAttack<P> oracle);
      template <Player P>
      void defense(CheckHashRecord *record, ProofOracleDefense<P> oracle);
      template <Player P>
      void testFixedDepthAttack(CheckHashRecord *record, Move);
      template <Player P>
      void testFixedDepthDefense(CheckHashRecord *record, CheckMove&);

      // テスト兼template instantiation 用途
      bool proofWin(state_t& state, const PathEncoding& path,
		    const CheckHashRecord *oracle, Move& best_move);
      bool proofLose(state_t& state, const PathEncoding& path,
		     const CheckHashRecord *oracle, Move last_move=Move::INVALID());
    };

  } // namespace checkmate
} // namespace osl

#endif /* _ORACLEPROVER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
