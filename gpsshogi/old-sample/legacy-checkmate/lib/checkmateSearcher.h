#ifndef _CHECKMATE_SEARHCER_H
#define _CHECKMATE_SEARHCER_H
#include "osl/checkmate/proofDisproof.h"
#include "checkHashTable.h"
#include "checkHashRecord.h"
#include "pieceCost2.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/state/numEffectState.h"
#include "osl/hash/hashKey.h"
#include "osl/pathEncoding.h"
#include "osl/move.h"
#include <cstddef>

#ifndef CHECKMATE_DEFAULT_TOTAL_NODE_LIMIT
#define CHECKMATE_DEFAULT_TOTAL_NODE_LIMIT (5000000*2)
// for 8gb?
// #define CHECKMATE_DEFAULT_TOTAL_NODE_LIMIT 5000000
// for 2gb memory
// #define CHECKMATE_DEFAULT_TOTAL_NODE_LIMIT 1000000
#endif

namespace osl
{
  namespace checkmate
  {
    class CheckMove;
    class CheckMoveList;
    class LibertyEstimator;
    class PieceCost2;
    /**
     * dfpn^+ extension.
     *
     * - extension by Kishimoto
     *   - avoid GHI problem by path-encoding and smulation
     *   - decrease ratio of re-extension of interior nodes
     * - evaluation functions
     *   - more pieces -> less proof number (Kishimoto)
     * @param Table SimpleCheckHashTable とか ArrayCheckHashTable など
     */
    template<class Table=CheckHashTable, 
	     class HEstimator=LibertyEstimator, class CostEstimator=PieceCost2>
    class CheckmateSearcher
    {
    public:
      typedef NumEffectState state_t;
      typedef Table table_t;
    private:
      state_t *state;		// acquaintance, delete しないこと
      Table& table;
      FixedDepthSearcher fixed_searcher;
      HashKey key;
      PathEncoding path;
      int depth;
      /** 現在の探索で消費したノード数 */
      size_t node_count;
      /** 現在の探索で消費可能なノード数 */
      size_t search_node_limit;
      /** objectが生成されてから消費したノード数 */
      size_t total_node_count;
      /** objectが生成されてから消費可能なノード数 */
      const size_t total_node_limit;
      bool verbose_destructor;
      const Player attacker;
      void clearNodeCount() { total_node_count+=node_count; node_count=0; }
    public:
      /**
       * @param attacker 攻撃側プレイヤー (オブジェクトはプレイヤ専用)
       */
      CheckmateSearcher(Player attacker, Table& t,
			size_t limit=CHECKMATE_DEFAULT_TOTAL_NODE_LIMIT);
      ~CheckmateSearcher();
      size_t totalNodeCount() const { return total_node_count; }
      size_t totalNodeLimit() const { return total_node_limit; }
      bool verbose() const { return verbose_destructor; }
      void setVerbose(bool verbose=true) { verbose_destructor = verbose; }
      /**
       * stateがPから詰む局面かを返す.
       * stateの手番はPと一致しているという前提
       * @param search_node_limit 探索するnode数の目安
       * Successの時はbest_moveにその手を
       */
      template <Player P>
      const ProofDisproof
      hasCheckmateMove(state_t& state, 
		       const HashKey& key, const PathEncoding& path, 
		       size_t search_node_limit, Move& best_move);
      const ProofDisproof 
      hasCheckmateMove(state_t& state, const HashKey& key,
		       const PathEncoding& path, size_t limit, Move& best_move)
      {
	if (attacker == BLACK)
	  return hasCheckmateMove<BLACK>(state, key, path, limit, best_move);
	else
	  return hasCheckmateMove<WHITE>(state, key, path, limit, best_move);
      }
      /**
       * stateがPによって詰んでいる局面かを返す
       * 今のところ打ち歩詰めには対応していない
       * 王手がかかっていない時には呼ばない
       * stateの手番はalt(P)と一致しているという前提
       * stateはPによって王手がかかっているという前提
       * @param search_node_limit 探索するnode数の目安
       * @param last_move 打ち歩詰めの判定に必要
       */
      template <Player P>
      const ProofDisproof
      hasEscapeMove(state_t& state, const HashKey&, const PathEncoding& path, 
		    size_t search_node_limit, Move last_move);
      const ProofDisproof
      hasEscapeMove(state_t& state, 
		    const HashKey& key, const PathEncoding& path, 
		    size_t limit, Move last_move)
      {
	if (attacker == BLACK)
	  return hasEscapeMove<BLACK>(state, key, path, limit, last_move);
	else
	  return hasEscapeMove<WHITE>(state, key, path, limit, last_move);
      }
      /**
       * attack側
       * @param record そのノードの CheckHashRecord のデータ
       *   0 以外で渡すと attack の中でテーブルをひかずに利用する
       *   0 で渡すと attack の中で新たにレコードを作った場合そのポインタが帰る
       *   ポインタの参照なので注意
       *   ProofDisproof は record->proofDisproof() に格納されて帰る
       * @param parent 親ノードのポインタ。合流チェック用。
       */
      template <Player P>
      void attack(unsigned int proofLimit, unsigned int disproofLimit, 
		  CheckHashRecord *parent, CheckHashRecord *record);
      /**
       * 詰みを逃れる手
       * alt(P)による
       * @param record そのノードの CheckHashRecord のデータ
       *   0 以外で渡すと attack の中でテーブルをひかずに利用する
       *   0 で渡すと attack の中で新たにレコードを作った場合そのポインタが帰る
       *   ProofDisproof は record->proofDisproof() に格納されて帰る
       */
      template <Player P>
      void defense(unsigned int proofLimit, unsigned int disproofLimit,
		   CheckHashRecord *parent, CheckHashRecord *record);

      const table_t& getTable() const { return table; }
    private:
      /**
       * attack の準備: 初めて訪れる局面で、合流チェック、データ確保、手生成などを行う
       * @param record 呼び出し時は0, 呼び出し後は確保したデータへのポインタ
       * @return true ならlimit over
       */
      template <Player P>
      bool setUpAttackNode(CheckHashRecord *record);
      /**
       * defense の準備: 初めて訪れる局面で、合流チェック、データ確保、手生成などを行う
       * @param record 呼び出し時は0, 呼び出し後は確保したデータへのポインタ
       * @return true ならlimit over
       */
      template <Player P>
      bool setUpDefenseNode(CheckHashRecord *record);

      bool exceedNodeCount(unsigned int futureCost) const;
      static bool exceedRootTolerance(unsigned int proofNumber, unsigned int disproofNumber,
				      unsigned int continuousNoExpandLoop);
    };
  } // namespace checkmate
  using checkmate::CheckmateSearcher;
} // namespace osl
#endif // _CHECKMATE_SEARHCER_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
