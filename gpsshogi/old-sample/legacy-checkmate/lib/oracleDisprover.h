/* oracleDisprover.h
 */
#ifndef _ORACLEDISPROVER_H
#define _ORACLEDISPROVER_H

#include "disproofOracle.h"
#include "osl/state/numEffectState.h"
#include "osl/hash/hashKey.h"
#include "osl/player.h"
#include "osl/pathEncoding.h"
namespace osl
{
  namespace checkmate
  {
    /**
     * Simulationによる不詰の確認.
     * 受方は類似局面で不詰となる手のみを試す．
     */
    template <class Table>
    class OracleDisprover
    {
    public:
      typedef NumEffectState state_t;
    private:
      Table& table;
      state_t *state;
      HashKey key;
      PathEncoding path;
      Player attacker;
      int node_count;
    public:
      explicit OracleDisprover(Table& t) 
	: table(t), attacker(t.getAttacker()), node_count(0)
      {
      }
      /** 
       * oracle に基づき逃げられるかどうかを判定する.
       * 手番は攻方
       */
      template <Player Attacker>
      bool proofNoCheckmate(state_t& state, 
			    const HashKey& key, const PathEncoding& path,
			    const DisproofOracleAttack<Attacker>& oracle);
      /** 
       * oracle に基づき詰んでいないかどうかを判定する.
       * 手番は受方
       * @param bestMove 不詰の場合に逃れる手で上書きする
       * @param state に alt(Attacker) に対して王手がかかっていること
       */
      template <Player Attacker>
      bool proofEscape(state_t& state, 
		       const HashKey& key, const PathEncoding& path,
		       const DisproofOracleDefense<Attacker>& oracle,
		       Move& best_move, Move last_move=Move::INVALID());

      int nodeCount() const { return node_count; }

      // private:
      template <Player P>
      void attack(CheckHashRecord *record, const DisproofOracleAttack<P>& oracle);
      template <Player P>
      void defense(CheckHashRecord *record, const DisproofOracleDefense<P>& oracle);

      // テスト兼用template instantiation 用
      bool proofNoCheckmate(state_t& state,  const PathEncoding& path,
			    const CheckHashRecord *oracle, const PathEncoding&);
      bool proofEscape(state_t& state, const PathEncoding& path,
		       CheckHashRecord *oracle, const PathEncoding&,
		       Move&, Move last_move=Move::INVALID());
    private:
      template <Player P>
      void confirmNoEscape(CheckHashRecord *record);
    };

  } // namespace checkmate
} // namespace osl

#endif /* _ORACLEDISPROVER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
