/* dualCheckmateSearcher.tcc
 */
#ifndef _DUALCHECKMATESEARCHER_TCC
#define _DUALCHECKMATESEARCHER_TCC

#include "dualCheckmateSearcher.h"
#include "checkHistoryToTable.h"
#include <iostream>
#include <iomanip>

template <class Table,class HEstimator,class CostEstimator>
osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
CheckmatorWithOracle::
CheckmatorWithOracle(Player attacker, size_t total_nodelimit)
  : table(attacker), 
    searcher(attacker, table, total_nodelimit),
#ifdef OSL_SMP
    table_small(attacker),
    searcher_small(attacker, table_small, total_nodelimit),
#endif
    prover(table), disprover(table), num_cleared(0),
    shared(new SharedOracles(attacker))
{
}

template <class Table,class HEstimator,class CostEstimator>
osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
CheckmatorWithOracle::
CheckmatorWithOracle(const CheckmatorWithOracle& src)
  : table(src.table.getAttacker()), 
    searcher(src.table.getAttacker(), table, src.searcher.totalNodeLimit()),
#ifdef OSL_SMP
    table_small(src.table.getAttacker()), 
    searcher_small(src.table.getAttacker(), table_small, src.searcher_small.totalNodeLimit()),
#endif
    prover(table), disprover(table), num_cleared(0),
    shared(src.shared)
{
}

template <class Table,class HEstimator,class CostEstimator>
osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
CheckmatorWithOracle::
~CheckmatorWithOracle()
{
}

/* ------------------------------------------------------------------------- */
template <class Table,class HEstimator,class CostEstimator>
osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
DualCheckmateSearcher(size_t total_nodelimit)
  : black(BLACK, total_nodelimit), white(WHITE, total_nodelimit), 
    simulation_node_count(0),
    proof_by_oracle(0), unknown_by_oracle(0), disproof_by_oracle(0),
    proof_by_search(0), unknown_by_search(0), disproof_by_search(0)
{
}
template <class Table,class HEstimator,class CostEstimator>
osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
~DualCheckmateSearcher()
{
  if (get(BLACK).searcher.verbose() && (oracles(BLACK).oracles.keySize() > 1))
  {
    std::cerr << "oracle p " << std::setw(5) << proof_by_oracle 
	      << " u " << std::setw(5) << unknown_by_oracle
	      << " d " << std::setw(5) << disproof_by_oracle << "\n";
    std::cerr << "search p " << std::setw(5) << proof_by_search
	      << " u " << std::setw(5) << unknown_by_search 
	      << " d " << std::setw(5) << disproof_by_search << "\n";
    std::cerr << "oracles ";
    oracles(BLACK).showStatus();
    std::cerr << " ";
    oracles(WHITE).showStatus();
    std::cerr << "\n";
  }
}

template <class Table,class HEstimator,class CostEstimator>
template <osl::Player P>
inline
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isWinningStateByOracle(int node_limit, NumEffectState& state, 
		       const HashKey& key, const PathEncoding& path,
		       Move& best_move, ProofOracleAttack<P> oracle)
{
  if (! oracle.isValid())
    return false;

  if (node_limit > 120)
  {
    if (get(P).prover.proofWin(state, key, path, oracle, best_move))
    {
      ++proof_by_oracle;
      assert(state.isValidMove(best_move));
      return true;
    }
  }
  else 
  {
    OracleProverLight<P> prover;
    const bool result = prover.proofWin(state, oracle, best_move);
    simulation_node_count += prover.nodeCount();
    if (result)
    {
      ++proof_by_oracle;
      assert(state.isValidMove(best_move));
      return true;
    }
  }
  ++unknown_by_oracle;
  return false;
}

template <class Table,class HEstimator,class CostEstimator>
template <osl::Player P>
inline
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isNotWinningStateByOracle(NumEffectState& state, 
			  const HashKey& key, const PathEncoding& path,
			  const DisproofOracleAttack<P>& oracle)
{
  if (oracle.isValid())
  {
    if (get(P).disprover.proofNoCheckmate(state, key, path, oracle))
    {
      ++disproof_by_oracle;
      return true;
    }
    ++unknown_by_oracle;
    return false;
  }
  return false;
}

template <class Table,class HEstimator,class CostEstimator>
template <osl::Player P>
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isWinningStateByOracle(NumEffectState& state, 
		       const HashKey& key, const PathEncoding& path,
		       Move& best_move, unsigned short& oracle_age,
		       int node_limit)
{
  assert(state.turn() == P);
  const PieceStand black_stand = key.blackStand();
  while (true)
  {
#ifndef NDEBUG
    const unsigned int old_age = oracle_age;
#endif
    const CheckHashRecord *guide 
      = oracles(P).oracles.findProofOracle(state, black_stand, oracle_age);
    if (! guide)
      break;
    assert(old_age < oracle_age);
    ProofOracleAttack<P> oracle(guide);
    if (isWinningStateByOracle(node_limit, state, key, path, best_move, oracle))
      return true;
    if (node_limit <= 20 && oracle_age == 1)
      break;
  }
  return false;
}

template <class Table,class HEstimator,class CostEstimator>
template <osl::Player P>
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isWinningStateByOracleLastMove(NumEffectState& state, 
			       const HashKey& key, const PathEncoding& path,
			       Move& best_move, Move last_move,
			       unsigned short& oracle_age,
			       int node_limit)
{
  assert(state.turn() == P);
  if ((! last_move.isNormal()) || last_move.isDrop())
    return false;

  const PieceStand black_stand = key.blackStand();
  const CheckHashRecord *guide 
    = oracles(P).oracles_last_move.findOracle(state, last_move, black_stand,
					      oracle_age);
  ProofOracleAttack<P> oracle(guide);
  return isWinningStateByOracle(node_limit,
				state, key, path, best_move, oracle);
}

template <class Table,class HEstimator,class CostEstimator>
template <osl::Player P>
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isWinningState(int node_limit, NumEffectState& state, 
	       const HashKey& key, const PathEncoding& path,
	       Move& best_move, AttackOracleAges& oracle_age, Move last_move)
{
  assert(state.turn() == P);
  assert(path.turn() == P);
#ifdef USE_ORACLE
  if (isWinningStateByOracleLastMove<P>(state, key, path, best_move, last_move,
					oracle_age.proof_last_move,
					node_limit))
    return true;
  if (isWinningStateByOracle<P>(state, key, path, best_move,oracle_age.proof,
				node_limit))
    return true;
  if (node_limit == 0)
    return false;
  if (node_limit > disproof_oracle_record_limit)
  {
    if (isNotWinningStateByOracle<P>(state, key, path, oracle_age.disproof))
      return false;
  }
#endif
#ifdef OSL_SMP
  if (node_limit < 80+get(P).num_cleared*10)
  {
    // check if already recorded as checkmate
    const CheckHashRecord *record 
      = get(P).searcher.getTable().find(key);
    if (record && record->proofDisproof().isFinal()) {
      if (record->proofDisproof().isCheckmateSuccess()) {
	best_move = record->bestMove->move;
	return true;
      }
      return false;
    }
    // clear table?
    if (get(P).table_small.size() >= 200000) {
      if (get(P).table.size() > get(P).table_small.size())
	get(P).num_cleared++;
      std::cerr << "clear table: main " << get(P).table.size()
		<< " sub " << get(P).table_small.size() << "\n";
      get(P).table_small.clear();
    }
    // search
    const size_t node_count_before = searcher(P).totalNodeCount();
    const ProofDisproof proof_disproof 
      = get(P).searcher_small.template hasCheckmateMove<P>
      (state, key, path, node_limit, best_move);
    const size_t node_count_after = searcher(P).totalNodeCount();

    if (! proof_disproof.isCheckmateSuccess())
      return false;
    ++proof_by_search;
    // migrate proof tree
    record = get(P).table_small.find(key);
    assert(record);
#ifndef NDEBUG
    const bool win = 
#endif
      get(P).prover.proofWin(state, key, path, ProofOracleAttack<P>(record), best_move);
    assert(win);
    assert(state.isValidMove(best_move));
#ifdef USE_ORACLE
    record = get(P).searcher.getTable().find(key);
    assert(record);
    const int node_visited = node_count_after - node_count_before;
    oracles(P).oracles.addProofOracle(state, record, node_visited);
    if (last_move.isNormal() && (! last_move.isDrop()))
    {
      oracles(P).oracles_last_move.addOracle(state, last_move, record);
    }
#endif
    return true;
  }
#endif
  const size_t node_count_before = searcher(P).totalNodeCount();
  const ProofDisproof proof_disproof 
    = get(P).searcher.template hasCheckmateMove<P>
    (state, key, path, node_limit, best_move);
  const size_t node_count_after = searcher(P).totalNodeCount();
  const int node_visited = node_count_after - node_count_before;
  if (proof_disproof.isCheckmateSuccess())
  {
    assert(state.isValidMove(best_move));
    ++proof_by_search;
#ifdef USE_ORACLE
    const CheckHashRecord *record 
      = get(P).searcher.getTable().find(key);
    assert(record);
    oracles(P).oracles.addProofOracle(state, record, node_visited);
    if (last_move.isNormal() && (! last_move.isDrop()))
    {
      oracles(P).oracles_last_move.addOracle(state, last_move, record);
    }
#endif
    return true;
  }
  if (proof_disproof.isCheckmateFail())
  {
    ++disproof_by_search;
#ifdef USE_ORACLE
    if (node_visited > disproof_oracle_record_limit)
    {
      const CheckHashRecord *record 
	= get(P).searcher.getTable().find(key);
      assert(record);
      oracles(P).oracles.addDisproofOracle(state, record, node_visited);
    }
#endif
  }
  else
  {
    ++unknown_by_search;
  }
  return false;
}

template <class Table,class HEstimator,class CostEstimator>
template <osl::Player P>
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isLosingState(int node_limit, NumEffectState& state, 
	      const HashKey& key, const PathEncoding& path, Move last_move)
{
  assert(state.turn() == P);
  assert(path.turn() == P);
  const Player attacker = PlayerTraits<P>::opponent;
  OraclePool& oracles = this->oracles(attacker).oracles_after_attack;
#ifdef USE_ORACLE
  unsigned short castle_oracle_age=0; // dummy
  const PieceStand black_stand = key.blackStand();
  const CheckHashRecord *guide 
    = oracles.findProofOracle(state, black_stand, castle_oracle_age);
  ProofOracleDefense<attacker> oracle(guide);
  if (oracle.isValid())
  {
    if (get(attacker).prover.proofLose(state, key,
				       path, oracle, last_move))
    {
      ++proof_by_oracle;
      return true;
    }
    ++unknown_by_oracle;
  }
#endif
  const size_t node_count_before = searcher(P).totalNodeCount();
  const size_t node_count_after = searcher(P).totalNodeCount();
  const ProofDisproof proof_disproof = get(attacker).searcher.template 
    hasEscapeMove<attacker>(state, key, path, 
			    node_limit, last_move);
  const int node_visited = node_count_after - node_count_before;
  if (proof_disproof.isCheckmateSuccess())
  {
    ++proof_by_search;
#ifdef USE_ORACLE
    const CheckHashRecord *record
      = get(attacker).searcher.getTable().find(key);
    assert(record);
    oracles.addProofOracle(state, record, node_visited);
#endif
    return true;
  }
  ++unknown_by_search;
  return false;
}


template <class Table,class HEstimator,class CostEstimator>
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isWinningState(int node_limit, NumEffectState& state, 
	       const HashKey& key, const PathEncoding& path,
	       Move& best_move, AttackOracleAges& age, Move last_move)
{
  if (state.turn() == BLACK)
    return isWinningState<BLACK>(node_limit, state, key, path, best_move, age, last_move);
  else
    return isWinningState<WHITE>(node_limit, state, key, path, best_move, age, last_move);
}

template <class Table,class HEstimator,class CostEstimator>
bool osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
isLosingState(int node_limit, NumEffectState& state, 
	      const HashKey& key, const PathEncoding& path,
	      Move last_move)
{
  if (state.turn() == BLACK)
    return isLosingState<BLACK>(node_limit, state, key, path, last_move);
  else
    return isLosingState<WHITE>(node_limit, state, key, path, last_move);
}

template <class Table,class HEstimator,class CostEstimator>
void osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
writeRootHistory(const RepetitionCounter& counter, const MoveStack& moves,
		 const SimpleState& state, Player attack)
{
  CheckHistoryToTable::write(getTable(attack), counter, moves,
			     state, attack);
}

#ifdef CHECKMATE_DEBUG
template <class Table,class HEstimator,class CostEstimator>
void osl::checkmate::DualCheckmateSearcher<Table,HEstimator,CostEstimator>::
undoWriteRootHistory(
#ifdef CHECKMATE_DEBUG
		     const RepetitionCounter& counter,
		     const MoveStack&,
		     const SimpleState&, Player attack
#else
		     const RepetitionCounter&,
		     const MoveStack&,
		     const SimpleState&, Player
#endif
)
{
  CheckHistoryToTable::undoWrite(getTable(attack), counter, attack);
}
#endif

#endif /* _DUALCHECKMATESEARCHER_TCC */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
