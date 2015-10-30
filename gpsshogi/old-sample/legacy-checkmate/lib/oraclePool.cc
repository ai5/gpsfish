/* oraclePool.cc
 */
#include "oraclePool.h"
#include "checkHashRecord.h"
#include "osl/state/simpleState.h"
#include "osl/hash/hashKey.h"
#include "osl/centering3x3.h"
#include "osl/stl/hash_map.h"
#include "osl/stl/vector.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#endif
#ifdef CHECKMATE_DEBUG
#  include <iostream>
#endif
#include <algorithm>

// #define CHECKMATE_DEBUG_EXTRA
#ifdef CHECKMATE_DEBUG_EXTRA
#  include <sstream>
#endif

namespace osl
{
  namespace checkmate
  {
    struct AddOracleError
    {
    };

    struct OracleInfo
    {
      const CheckHashRecord *record;
      short attack_count, defense_count;
      int node_count;
#ifdef CHECKMATE_DEBUG_EXTRA
      std::string state;
#endif
      OracleInfo() : record(0), attack_count(0), defense_count(0), 
		     node_count(0) {}
      OracleInfo(const CheckHashRecord *oracle, int node,
		 const NumEffectState& state, Player defender)
	: record(oracle), attack_count(0), defense_count(0), node_count(node) 
      {
	if (record->hasBestMove()) {
	  const Move move = record->getBestMove()->move;
	  attack_count = state.countEffect(alt(defender), move.to());
	  defense_count = state.countEffect(defender, move.to());
	}			    
#ifdef CHECKMATE_DEBUG_EXTRA
	std::ostringstream os;
	os << state;
	this->state = os.str();
#endif
      }
      bool suitableInProof(const NumEffectState& state, 
			   Player defender) const
      {
	if (! record->hasBestMove())
	  return true;
	const Move move = record->getBestMove()->move;
	if (defense_count < state.countEffect(defender, move.to()))
	  return false;
	if (attack_count > state.countEffect(alt(defender), move.to()))
	  return false;
	return state.isAlmostValidMove<false>(move);
      }
    };
    // TODO: list の方がメモリは使うが速いはず
    typedef vector<OracleInfo> vector_t;
    typedef hash_map<HashKey,vector_t> hash_map_t;

    const unsigned int max_oracles_for_key_soft_limit = 128;
    const unsigned int max_oracles_for_key_hard_limit = 65534;
  }
} // namespace osl

/** 
 * とりあえず8近傍で判定してみる.
 * 持駒や25近傍などが他の候補
 */
struct osl::checkmate::OraclePool::Table 
{
#ifdef OSL_SMP
  typedef osl::misc::LightMutex Mutex;
  mutable Mutex mutex;
#endif

  hash_map_t table;
  const Player defender;
  explicit Table(Player attacker) : defender(alt(attacker))
  {
  }
  ~Table() 
  {
#ifdef CHECKMATE_DEBUG
    size_t max_length = 0;
    for (hash_map_t::const_iterator p=table.begin(); p!=table.end(); ++p) {
      max_length = std::max(p->second.size(), max_length);
    }
    if (max_length > 10)
      std::cerr << "OraclePool " << max_length << "\n";
#endif
  }
  template <Direction DIR>
  static void addKey(HashKey& key, const SimpleState& state, Square target)
  {
    const Offset offset = Board_Table.getOffsetForBlack(DIR);
    target += offset;	// 8 近傍全て試すなら手番による符合変換は不要
    const Piece piece = state.pieceOnBoard(target);
    hash::Hash_Gen_Table.addHashKey(key, target, piece.ptypeO());
  }
  const HashKey makeKey(const SimpleState& state) const
  {
    const Square target_king=state.kingSquare(defender);
    const Square center = Centering3x3::adjustCenter(target_king);

    HashKey key;
    hash::Hash_Gen_Table.addHashKey(key, center, 
				    state.pieceOnBoard(center).ptypeO());
    addKey<UL>(key, state, center);
    addKey<U> (key, state, center);
    addKey<UR>(key, state, center);
    addKey<L> (key, state, center);
    addKey<R> (key, state, center);
    addKey<DL>(key, state, center);
    addKey<D> (key, state, center);
    addKey<DR>(key, state, center);
    return key;
  }
  void dump(const NumEffectState& state, const CheckHashRecord* oracle,
	    const vector_t& v) const;
  void addOracle(const NumEffectState& state, const CheckHashRecord* oracle,
		 int node_count)
  {
#ifdef OSL_SMP
    SCOPED_LOCK(lk,mutex);
#endif
    const HashKey key = makeKey(state);
    vector_t& v = table[key];
    if (! v.empty()) {
      // perhaps, we should reduce entries more
      const vector_t::value_type& last_one = v.back();
      if (last_one.record->hasBestMove()
	  && last_one.node_count == node_count
	  && last_one.record->getBestMove()->move == oracle->getBestMove()->move)
	return;
    }
    if (v.size() > 16)
      return;
    if (v.size() >= max_oracles_for_key_soft_limit)
    {
#ifdef CHECKMATE_DEBUG
      dump(state, oracle, v);
      throw AddOracleError();
#endif
      return;
    }
    v.push_back(OracleInfo(oracle, node_count, state, defender));
  }
  template <bool is_attack>
  const CheckHashRecord * find(const NumEffectState& state, PieceStand stand,
			       unsigned short& age) const;
  size_t keySize() const
  {
    return table.size();
  }
  size_t totalSize() const
  {
#ifdef OSL_SMP
    SCOPED_LOCK(lk,mutex);
#endif
    size_t result = 0;
    for (hash_map_t::const_iterator p=table.begin(); p!=table.end(); ++p)
    {
      result += p->second.size();
    }
    return result;
  }
};

#ifdef CHECKMATE_DEBUG
void osl::checkmate::OraclePool::
Table::dump(const NumEffectState& state, const CheckHashRecord* oracle,
	    const vector_t& v) const
{
  std::cerr << state;
  for (size_t i=0; i<v.size(); ++i) {
    std::cerr << "(" << i << ") " << v[i].record->getBestMove()->move
	      << " " << v[i].defense_count << " " << v[i].node_count << "\n";
    // v[i].record->dump();
#ifdef CHECKMATE_DEBUG_EXTRA
    std::cerr << v[i].state;
#endif
  }
  oracle->dump();
}
#endif

template <bool is_attack>
const osl::checkmate::CheckHashRecord *
osl::checkmate::OraclePool::
Table::find(const NumEffectState& state, PieceStand stand,
	    unsigned short& age) const
{
#ifdef OSL_SMP
  SCOPED_LOCK(lk,mutex);
#endif
  const HashKey key = makeKey(state);
  hash_map_t::const_iterator pos = table.find(key);
  if (pos == table.end())
    return 0;

  const vector_t& v = pos->second;
  assert(v.size() <= max_oracles_for_key_hard_limit);
  while (age<v.size())
  {
    const vector_t::value_type& record = v[age++];
    if (is_attack)
    {
      if (! record.suitableInProof(state, defender))
	continue;
      if (stand.hasMoreThan<BLACK>(record.record->proofPieces()))
	return record.record;
    }
    else
    {
      if (stand.hasMoreThan<BLACK>(record.record->disproofPieces()))
	return record.record;
    }
  }
  return 0;
}
/* ------------------------------------------------------------------------- */

osl::checkmate::
OraclePool::OraclePool(Player attacker)
  : proof_oracles(new Table(attacker)), disproof_oracles(new Table(attacker))
{
}
  
osl::checkmate::
OraclePool::~OraclePool()
{
}
  
void osl::checkmate::
OraclePool::addProofOracle(const NumEffectState& state, 
			   const CheckHashRecord* oracle,
			   int node_count)
{
  assert(oracle->hasProofPieces());
  proof_oracles->addOracle(state, oracle, node_count);
}
void osl::checkmate::
OraclePool::addDisproofOracle(const NumEffectState& state, 
			      const CheckHashRecord* oracle, int node_count)
{
  assert(oracle->proofDisproof().isCheckmateFail()
	 || (! oracle->twins.empty()));
  if (oracle->hasDisproofPieces()) // TODO: loop でも持たせると効率向上
    disproof_oracles->addOracle(state, oracle, node_count);
}
  
const osl::checkmate::CheckHashRecord *
osl::checkmate::
OraclePool::findProofOracle(const NumEffectState& state, PieceStand black_stand,
			    unsigned short& age) const
{
  const PieceStand attack_stand 
    = (proof_oracles->defender == WHITE) ? black_stand : PieceStand(WHITE, state);
  return proof_oracles->find<true>(state, attack_stand, age);
}

const osl::checkmate::CheckHashRecord *
osl::checkmate::
OraclePool::findDisproofOracle(const NumEffectState& state, PieceStand black_stand,
			       unsigned short& age) const
{
  const PieceStand defense_stand 
    = (disproof_oracles->defender == BLACK) ? black_stand : PieceStand(WHITE, state);
  return disproof_oracles->find<false>(state, defense_stand, age);
}

size_t osl::checkmate::
OraclePool::totalSize() const
{
  return proof_oracles->totalSize() + disproof_oracles->totalSize();
}

size_t osl::checkmate::
OraclePool::keySize() const
{
  return proof_oracles->keySize() + disproof_oracles->keySize();
}
  
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
