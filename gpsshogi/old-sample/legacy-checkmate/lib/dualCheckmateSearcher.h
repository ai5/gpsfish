/* dualCheckmateSearcher.h
 */
#ifndef _DUALCHECKMATESEARCHER_H
#define _DUALCHECKMATESEARCHER_H

#include "checkmateSearcher.h"
#include "checkHashTable.h"
#include "oraclePool.h"
#include "oraclePoolLastMove.h"
#include "oracleProver.h"
#include "oracleProverLight.h"
#include "oracleDisprover.h"
#include "oracleAges.h"

#ifdef CHECKMATE_DEBUG
#  include "osl/checkmate/analyzer/checkTableAnalyzer.h"
#endif

#include "osl/misc/carray.h"

#include <boost/shared_ptr.hpp>
#include <cassert>

/** ORACLE を最初に試す */
#define USE_ORACLE

namespace osl
{
  class RepetitionCounter;
  namespace container
  {
    class MoveStack;
  } // namespace container
  using container::MoveStack;
  namespace checkmate
  {
    inline size_t limitToCheckCount(int limit){
      assert(limit >= 0);
      extern CArray<size_t,32> limitToCheckCountTable;
      return limitToCheckCountTable[limit>>7];
    }

    struct SharedOracles
    {
      OraclePool oracles;
      OraclePoolLastMove oracles_last_move;
      OraclePool oracles_after_attack;
      void showStatus() const;

      SharedOracles(Player attack);
    };

    /**
     * 両方の Player の詰みを扱える詰将棋.
     * CAVEAT: コピーした場合は、全てのobjectの寿命を揃える必要がある.
     * 共有される SharedOracles がテーブル内部へのポインタを含むため.
     */
    template <class Table=CheckHashTable, class HEstimator=LibertyEstimator, class CostEstimator=PieceCost2>
    class DualCheckmateSearcher
    {
    public:
      typedef Table table_t;
      typedef CheckmateSearcher<table_t, HEstimator, CostEstimator> checkmate_t;
      typedef OracleProver<table_t> prover_t;
      typedef OracleDisprover<table_t> disprover_t;
      struct CheckmatorWithOracle
      {
	table_t table;
	checkmate_t searcher;
#ifdef OSL_SMP
	table_t table_small;
	checkmate_t searcher_small;	
#endif
	prover_t prover;
	disprover_t disprover;
	int num_cleared;
	std::shared_ptr<SharedOracles> shared;

	CheckmatorWithOracle(Player attacker, size_t total_nodelimit);
	/** shared をshareする以外は新規に作る */
	CheckmatorWithOracle(const CheckmatorWithOracle& src);
	~CheckmatorWithOracle();
      };
    private:
      CheckmatorWithOracle black, white;
      CheckmatorWithOracle& get(Player attacker)
      {
	return (attacker == BLACK) ? black : white;
      }
      const CheckmatorWithOracle& get(Player attacker) const
      {
	return (attacker == BLACK) ? black : white;
      }
      SharedOracles& oracles(Player attacker)
      {
	return *(get(attacker).shared);
      }
      static const int disproof_oracle_record_limit = 500;
      // statistics
      size_t simulation_node_count;
      unsigned int proof_by_oracle, unknown_by_oracle, disproof_by_oracle;
      unsigned int proof_by_search, unknown_by_search, disproof_by_search;
    public:
      /**
       * @param total_node_count このinstance経由で読む最大のノード数.
       * isWinningState などで適切な node_limit を与えていれば通常は
       * 非常に大きな値で良い.
       */
      explicit DualCheckmateSearcher(size_t total_node_limit=CHECKMATE_DEFAULT_TOTAL_NODE_LIMIT);
      
      virtual ~DualCheckmateSearcher();
      void setVerbose(bool verbose=true) { 
	get(BLACK).searcher.setVerbose(verbose);
	get(WHITE).searcher.setVerbose(verbose);
      }
    private:
      template <Player P>
      bool isWinningStateByOracle(int node_limit, NumEffectState& state, 
				  const HashKey& key, const PathEncoding& path,
				  Move& best_move, ProofOracleAttack<P> oracle);
      template <Player P>
      bool isNotWinningStateByOracle(NumEffectState& state, 
				     const HashKey& key, const PathEncoding& path,
				     const DisproofOracleAttack<P>& oracle);
    public:
      /**
       * @return 詰かどうか
       */
      template <Player P>
      bool isWinningStateByOracle(NumEffectState& state, 
				  const HashKey& key, const PathEncoding& path,
				  Move& best_move, unsigned short& oracle_age,
				  int node_limit=0);
      /**
       * @param oracle_age 同じoracleを2度試さないための栞
       * @return 不詰かどうか
       */
      template <Player P>
      bool isNotWinningStateByOracle(NumEffectState& state, 
				     const HashKey& key, const PathEncoding& path,
				     unsigned short& oracle_age)
      {
	assert(state.turn() == P);
	const PieceStand black_stand = key.blackStand();
	const CheckHashRecord *guide 
	  = oracles(P).oracles.findDisproofOracle(state, black_stand, oracle_age);
	DisproofOracleAttack<P> oracle(guide);
	return isNotWinningStateByOracle(state, key, path, oracle);
      }
      /**
       * @param oracle_age 同じoracleを2度試さないための栞
       * @return 詰かどうか
       */
      template <Player P>
      bool isWinningStateByOracleLastMove(NumEffectState& state, 
					  const HashKey& key, const PathEncoding& path,
					  Move& best_move, Move last_move,
					  unsigned short& oracle_age,
					  int node_limit=0);
      bool isWinningStateByOracleLastMove(NumEffectState& state, 
					  const HashKey& key, const PathEncoding& path,
					  Move& best_move, Move last_move,
					  unsigned short& age)
      {
	if (state.turn() == BLACK)
	  return isWinningStateByOracleLastMove<BLACK>
	    (state, key, path, best_move, last_move, age);
	else
	  return isWinningStateByOracleLastMove<WHITE>
	    (state, key, path, best_move, last_move, age);
      }
      /**
       * @return 相手玉が詰み
       * @param oracle_age 入力:既に試したoracleの数，出力:returnまでに試したoracleの数. (oracleは順にならんでいるので増えたものだけ試す)
       */
      template <Player P>
      bool isWinningState(int node_limit, NumEffectState& state, 
			  const HashKey& key, const PathEncoding& path,
			  Move& best_move, AttackOracleAges& oracle_age,
			  Move last_move=Move::INVALID());
      bool isWinningState(int node_limit, NumEffectState& state, 
			  const HashKey& key, const PathEncoding& path,
			  Move& best_move, AttackOracleAges& oracle_age,
			  Move last_move=Move::INVALID());
      bool isWinningState(int node_limit, NumEffectState& state, 
			  const HashKey& key, const PathEncoding& path,
			  Move& best_move, Move last_move=Move::INVALID())
      {
	AttackOracleAges dummy_age;
	return isWinningState(node_limit, state, key, path, best_move, 
			      dummy_age, last_move);
      }
      /**
       * @return 自玉が詰み
       * 条件: 自玉に王手がかかっていること
       * @param P 手番 
       * @param oracle_age 入力:既に試したoracleの数，出力:returnまでに試したoracleの数. (oracleは順にならんでいるので増えたものだけ試す)
       */
      template <Player P>
      bool isLosingState(int node_limit, NumEffectState& state, 
			 const HashKey& key, const PathEncoding& path,
			 Move last_move=Move::INVALID());
      bool isLosingState(int node_limit, NumEffectState& state, 
			 const HashKey&, const PathEncoding& path, 
			 Move last_move=Move::INVALID());

      const checkmate_t& searcher(Player P) const { return get(P).searcher; }
      const table_t& getTable(Player P) const { return get(P).table; }
      table_t& getTable(Player P)  { return get(P).table; }
      size_t mainNodeCount() const
      {
	return searcher(BLACK).totalNodeCount() 
	  + searcher(WHITE).totalNodeCount();
      }
      size_t totalNodeCount() const
      {
	return (simulation_node_count
		+ searcher(BLACK).totalNodeCount() 
		+ searcher(WHITE).totalNodeCount()
#ifdef OSL_SMP
		+ get(BLACK).searcher_small.totalNodeCount()
		+ get(WHITE).searcher_small.totalNodeCount()
#endif
		+ get(BLACK).prover.nodeCount()
		+ get(WHITE).prover.nodeCount()
		+ get(BLACK).disprover.nodeCount()
		+ get(WHITE).disprover.nodeCount());
      }

      void writeRootHistory(const RepetitionCounter& counter,
			    const MoveStack& moves,
			    const SimpleState& state, Player attack);
      void undoWriteRootHistory(const RepetitionCounter& counter,
				const MoveStack& moves,
				const SimpleState& state, Player attack);
    };

  } // namespace checkmate
  using checkmate::DualCheckmateSearcher;
} // namespace osl

#endif /* _DUALCHECKMATESEARCHER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
