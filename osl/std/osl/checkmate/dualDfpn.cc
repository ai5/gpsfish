/* dualDfpn.cc
 */
#include "osl/checkmate/dualDfpn.h"
#include "osl/checkmate/dfpn.h"
#include "osl/checkmate/dfpnRecord.h"
#ifdef OSL_DFPN_SMP
#  include "osl/checkmate/dfpnParallel.h"
#endif
#include "osl/repetitionCounter.h"
#include "osl/stat/average.h"
#include "osl/centering3x3.h"
#include "osl/bits/centering5x3.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#  include <condition_variable>
#endif
#ifdef OSL_SHOW_PROOF_TREE_MIGRATION_STAT
#  include "osl/stat/ratio.h"
#endif
#include "osl/misc/milliSeconds.h"
#include "osl/oslConfig.h"
#include <unordered_map>
#include <forward_list>
#include <iostream>
#include <iomanip>

#define DFPN_SHARE_TABLE

static const int max_oracle_list_size = 2;
static const size_t local_table_growth_limit = 40000;
struct osl::checkmate::DualDfpn::OraclePool
{
  struct Element
  {
    Dfpn::ProofOracle oracle;
    PieceStand proof_pieces;
    unsigned int id;
    bool in_check;
    Element() : oracle(HashKey(), PieceStand()), id((unsigned int)-1), in_check(false)
    {
    }
    Element(const Dfpn::ProofOracle& o, PieceStand p, size_t i, bool c) : oracle(o), proof_pieces(p), id(i), in_check(c)
    {
    }
  };
  struct List : FixedCapacityVector<Element, max_oracle_list_size>
  {
    void add(const Element& e)
    {
      if (size() == capacity())
	back() = e;
      else
	push_back(e);
    }
  };
#ifdef OSL_SMP
  mutable std::mutex mutex;
#endif  
  typedef std::unordered_map<HashKey, List, std::hash<HashKey>> table_t;
  table_t table;
  Player defender;
  void setAttack(Player attack) 
  {
    defender = alt(attack);
  }
  void addProof(const NumEffectState& state, const HashKey& key, PieceStand proof_pieces) 
  {
    const Dfpn::ProofOracle oracle(key, PieceStand(WHITE, state));
    const std::pair<HashKey,HashKey> king = makeLargeKey(state);
#ifdef OSL_SMP
    std::lock_guard<std::mutex> lk(mutex);;
#endif
    const Element e(oracle, proof_pieces, table.size(), state.inCheck());
    table[king.first].add(e);
    table[king.second].add(e);
  }
  const List probe(const NumEffectState& state) const
  {
    const std::pair<HashKey,HashKey> key = makeLargeKey(state);
#ifdef OSL_SMP
    std::lock_guard<std::mutex> lk(mutex);;
#endif
    table_t::const_iterator p = table.find(key.first);
    if (p != table.end())
      return p->second;
    p = table.find(key.second);
    if (p != table.end())
      return p->second;
    return List();
  }

  template <Direction DIR>
  static void addKey(HashKey& key, const SimpleState& state, Square target)
  {
    const Offset offset = DirectionTraits<DIR>::blackOffset();
    target += offset;	// 8 近傍全て試すなら手番による符合変換は不要
    const Piece piece = state.pieceOnBoard(target);
    HashGenTable::addHashKey(key, target, piece.ptypeO());
  }
  template <Direction DIR, Direction DIR2>
  static void addKey(HashKey& key, const SimpleState& state, Square target)
  {
    const Offset offset = DirectionTraits<DIR>::blackOffset()
      + DirectionTraits<DIR2>::blackOffset();
    target += offset;
    const Piece piece = state.pieceOnBoard(target);
    HashGenTable::addHashKey(key, target, piece.ptypeO());
  }
  const HashKey makeKey(const SimpleState& state) const
  {
    const Square target_king=state.kingSquare(defender);
    const Square center = Centering3x3::adjustCenter(target_king);
    HashKey key;
    HashGenTable::addHashKey(key, center, 
				    state.pieceOnBoard(center).ptypeO());
    addKey<UL>(key, state, center); addKey<U> (key, state, center);
    addKey<UR>(key, state, center);
    addKey<L> (key, state, center); addKey<R> (key, state, center);
    addKey<DL>(key, state, center); addKey<D> (key, state, center);
    addKey<DR>(key, state, center);
    return key;
  }
  const std::pair<HashKey,HashKey> makeLargeKey(const SimpleState& state) const
  {
    HashKey key_small = makeKey(state), key_large;
    const Square target_king=state.kingSquare(defender);
    const Square center = Centering5x3::adjustCenter(target_king);
    HashGenTable::addHashKey(key_large, center, 
				    state.pieceOnBoard(center).ptypeO());
    addKey<UL>(key_large, state, center); addKey<U> (key_large, state, center);
    addKey<UR>(key_large, state, center);
    addKey<L> (key_large, state, center); addKey<R> (key_large, state, center);
    addKey<DL>(key_large, state, center); addKey<D> (key_large, state, center);
    addKey<DR>(key_large, state, center);
    addKey<L,UL>(key_large, state, center); addKey<L,L> (key_large, state, center);
    addKey<L,DL>(key_large, state, center);
    addKey<R,UR>(key_large, state, center); addKey<R,R> (key_large, state, center);
    addKey<R,DR>(key_large, state, center);
    return std::make_pair(key_large, key_small);
  }
};

struct osl::checkmate::DualDfpn::Shared
{
  CArray<DfpnTable,2> table;
  CArray<OraclePool,2> pool;
  size_t main_node_count;
  size_t simulation_count;
  volatile size_t last_gc, gc_threshold;
  CArray<stat::Average,max_oracle_list_size> proof_by_oracle;
  CArray<bool,2> blocking_verify;
#ifdef OSL_SMP
  std::mutex mutex;
  std::condition_variable condition;
  CArray<LightMutex,max_oracle_list_size> proof_by_oracle_mutex;
#endif
  volatile int shared_table_user, shared_table_gc_wait;
#ifdef OSL_DFPN_SMP
  std::unique_ptr<DfpnParallel> parallel_search;
#endif
  typedef std::forward_list<PathEncoding> disproof_list_t;
  typedef std::unordered_map<HashKey, disproof_list_t, std::hash<HashKey>> disproof_table_t;
  disproof_table_t disproof_table;

  Shared() : main_node_count(0), simulation_count(0), last_gc(0), gc_threshold(10),
	     shared_table_user(0), shared_table_gc_wait(0)
  {
    table[BLACK].setAttack(BLACK);
    table[WHITE].setAttack(WHITE);
    pool[BLACK].setAttack(BLACK);
    pool[WHITE].setAttack(WHITE);
    blocking_verify.fill(true);
  }
  ~Shared()
  {
    showStats();
  }
  void showStats()
  {
    if (main_node_count || simulation_count) {
#ifdef DFPN_DEBUG
      std::cerr << "shared " << main_node_count << " " << simulation_count << "\n";
      for (stat::Average& a: proof_by_oracle)
	std::cerr << a.getAverage() 
		  << " " << (int)(a.getAverage()*a.numElements()) << "\n";
      std::cerr << "oracles " << pool[BLACK].table.size() << " " << pool[WHITE].table.size() << "\n";
      std::cerr << "table " << table[0].totalSize() << " " << table[1].totalSize() << "\n";
      table[0].showStats();
      table[1].showStats();
#endif
    }
  }
  void addMainNodeCount(int add)
  {
#ifdef OSL_SMP
    std::lock_guard<std::mutex> lk(mutex);;
#endif
    main_node_count += add;
  }
  void addSimulationNodeCount(int add)
  {
#ifdef OSL_SMP
    std::lock_guard<std::mutex> lk(mutex);;
#endif
    simulation_count += add;
  }
  struct TableUseLock
  {
    TableUseLock(const TableUseLock&) = delete;
    TableUseLock& operator=(const TableUseLock&) = delete;
    
    Shared *shared;
    explicit TableUseLock(Shared *s) : shared(s)
    {
#  ifdef OSL_SMP
      std::unique_lock<std::mutex> lk(shared->mutex);
      while (shared->shared_table_user < 0) // in gc
	shared->condition.wait(lk);
      shared->shared_table_user++;
#  endif
    }
    ~TableUseLock()
    {
#  ifdef OSL_SMP
      std::lock_guard<std::mutex> lk(shared->mutex);
      assert(shared->shared_table_user > 0);
      shared->shared_table_user--;
      if (shared->shared_table_user == 0 && shared->shared_table_gc_wait)
	shared->condition.notify_all();
#  endif
    }
  };
};

struct osl::checkmate::DualDfpn::Local
{
  Dfpn dfpn;
#ifndef DFPN_SHARE_TABLE
  CArray<DfpnTable,2> table;
#endif
  CArray<DfpnTable,2> table_small;
  size_t local_node_count;
  Local() : local_node_count(0)
  {
#ifndef DFPN_SHARE_TABLE
    table[BLACK].setAttack(BLACK);
    table[WHITE].setAttack(WHITE);
#endif
    table_small[BLACK].setAttack(BLACK);
    table_small[WHITE].setAttack(WHITE);
  }
  ~Local()
  {
#ifdef DFPN_DEBUG
    std::cerr << "local " << table_small[0].totalSize()
	      << " " << table_small[1].totalSize() << "\n";
#endif
  }
};

/* ------------------------------------------------------------------------- */

osl::checkmate::
DualDfpn::DualDfpn(uint64_t /*limit*/)
  : shared(new Shared), local(new Local)
{
}

osl::checkmate::
DualDfpn::DualDfpn(const DualDfpn& src)
  : shared(src.shared), local(new Local)
{
}

osl::checkmate::
DualDfpn::~DualDfpn()
{
}

osl::checkmate::Dfpn& osl::checkmate::
DualDfpn::prepareDfpn(Player attack)
{
#ifdef DFPN_SHARE_TABLE
  local->dfpn.setTable(&(shared->table[attack]));
#else
  local->dfpn.setTable(&(local->table[attack]));
#endif
  local->dfpn.setBlockingVerify(shared->blocking_verify[attack]);
  return local->dfpn;
}

osl::checkmate::Dfpn& osl::checkmate::
DualDfpn::prepareDfpnSmall(Player attack)
{
  local->dfpn.setTable(&(local->table_small[attack]));
  local->dfpn.setBlockingVerify(shared->blocking_verify[attack]);
  return local->dfpn;
}

void osl::checkmate::
DualDfpn::runGC(bool verbose, size_t memory_use_ratio_1000)
{
#ifdef DFPN_SHARE_TABLE
  const size_t unit_size = (sizeof(HashKey)+sizeof(DfpnRecord)+sizeof(char*)*2);
  size_t removed = 0;
  size_t total = shared->table[BLACK].size() + shared->table[WHITE].size();
  size_t current_use = memory_use_ratio_1000*(OslConfig::memoryUseLimit()/1000);
  if (total < local_table_growth_limit*8
      || total*unit_size*64 < OslConfig::memoryUseLimit()
      || (total*unit_size*3 < current_use
	  && total*unit_size*8 < OslConfig::memoryUseLimit()))
    return;
  time_point start = clock::now();
  {
    {
#  ifdef OSL_SMP
      std::unique_lock<std::mutex> lk(shared->mutex);
#  endif
      total = shared->table[BLACK].size() + shared->table[WHITE].size();
      if (total < local_table_growth_limit*8
	  || (total*unit_size*3 < current_use
	      && total*unit_size*6 < OslConfig::memoryUseLimit()))
	return;
      if (total < shared->last_gc + local_table_growth_limit*2)
	return;
      if (shared->shared_table_user > 0
	  && memory_use_ratio_1000 < 650
	  && total < shared->last_gc*2)
	return;
      if (shared->shared_table_user < 0 || shared->shared_table_gc_wait > 0)
	return;
#  ifdef OSL_SMP
      while (shared->shared_table_user > 0) {
	++shared->shared_table_gc_wait;
	shared->condition.wait(lk);
	--shared->shared_table_gc_wait;
      }
      if (shared->shared_table_user < 0)
	return;
#  endif
      shared->shared_table_user--;
    }
    removed += shared->table[BLACK].smallTreeGC(shared->gc_threshold);
    removed += shared->table[WHITE].smallTreeGC(shared->gc_threshold);
    {
#  ifdef OSL_SMP
      std::lock_guard<std::mutex> lk(shared->mutex);
#  endif
      if (total > shared->last_gc*2) {
	if (100.0*removed/total < 70)
	  shared->gc_threshold += 15;
	else if (100.0*removed/total < 90)
	  shared->gc_threshold += 5;
      }
      shared->last_gc = total - removed;
      shared->shared_table_user++;
      assert(shared->shared_table_user == 0);
#  ifdef OSL_SMP
      shared->condition.notify_all();
#  endif
    }
  }
  if (! verbose) 
    return;
  const double elapsed = elapsedSeconds(start);
  if (removed > 10000 || elapsed > 0.1)
    std::cerr << " GC " << removed
	      << " entries " << std::setprecision(3)
	      << (unit_size * removed / (1<<20)) << "MB "
	      << 100.0*removed/total << "%"
	      << " (" << elapsed << " s)\n";
#endif
}


template <osl::Player P>
osl::ProofDisproof osl::checkmate::
DualDfpn::findProof(int node_limit, const NumEffectState& state, 
		    const HashKey& key, const PathEncoding& path,
		    Move& best_move, Move last_move)
{
  assert(state.turn() == P);
  // oracle
  Dfpn& dfpn = prepareDfpn(P);
  const OraclePool::List l(shared->pool[P].probe(state));
  const PieceStand attack_stand = (P==BLACK) ? key.blackStand() : PieceStand(WHITE, state);
  int num_tried = 0;
  for (size_t i=0; i<l.size(); ++i)
  {
    if (! attack_stand.isSuperiorOrEqualTo(l[i].proof_pieces)
	|| l[i].in_check != state.inCheck())
      continue;
    ++num_tried;
    const ProofDisproof pdp = (node_limit > 20)
      ? dfpn.tryProof(state, key, path, l[i].oracle, l[i].id, best_move, last_move)
      : dfpn.tryProofLight(state, key, path, l[i].oracle, l[i].id, best_move, last_move);
    const size_t count = dfpn.nodeCount();
    local->local_node_count += count;
    shared->addSimulationNodeCount(count);
    if (count) {
#ifdef OSL_SMP
      SCOPED_LOCK(lk,shared->proof_by_oracle_mutex[i]);
#endif
      shared->proof_by_oracle[i].add(pdp.isCheckmateSuccess());
    }
    if (pdp.isCheckmateSuccess()) 
      assert(best_move.isNormal());
    if (pdp.isFinal())
      return pdp;
  }
  if (node_limit == 0 && num_tried) 
    return ProofDisproof(1,1);			// already tested table
  const ProofDisproof table_pdp = dfpn.hasCheckmateMove(state, key, path, 0, best_move, last_move);
  if (table_pdp.isCheckmateSuccess())
    return table_pdp;
  {
#ifdef OSL_SMP
    std::lock_guard<std::mutex> lk(shared->mutex);
#endif
    Shared::disproof_table_t::const_iterator p = shared->disproof_table.find(key);
    if (p != shared->disproof_table.end()) {
      for (const auto& ppath: p->second)
	if (ppath == path)
	  return ProofDisproof::LoopDetection();
    }
  }
#ifdef OSL_SHOW_PROOF_TREE_MIGRATION_STAT
  static stat::Ratio migration_success("migration_success", true);
  bool need_migration = false;
#endif
  // local
  if (node_limit < 80) {
    if (local->table_small[P].totalSize() >= local_table_growth_limit) {
      local->table_small[P].clear();
    }
    Dfpn& dfpn_small = prepareDfpnSmall(P);
    const ProofDisproof pdp = dfpn_small.hasCheckmateMove(state, key, path, node_limit, best_move, last_move);
    const size_t count = dfpn_small.nodeCount();
    local->local_node_count += count;
    shared->addMainNodeCount(count);
    if (pdp.isLoopDetection()) {
#ifdef OSL_SMP
      std::lock_guard<std::mutex> lk(shared->mutex);
#endif
      shared->disproof_table[key].push_front(path);
    }
    if (! pdp.isCheckmateSuccess())
      return pdp;
    assert(best_move.isNormal());
    // fall through if checkmate success (TODO: efficient proof tree migration)
#ifdef OSL_SHOW_PROOF_TREE_MIGRATION_STAT
    need_migration = true;
#endif
  }
  // main
  Shared::TableUseLock lk(&*shared);
  PieceStand proof_pieces;
  const ProofDisproof pdp = dfpn.hasCheckmateMove(state, key, path, node_limit, best_move, proof_pieces, last_move);
  const size_t count = dfpn.nodeCount();
  local->local_node_count += count;
  shared->addMainNodeCount(count);
  if (pdp.isCheckmateSuccess())
    shared->pool[P].addProof(state, key, proof_pieces);
#ifdef OSL_SHOW_PROOF_TREE_MIGRATION_STAT
  if (need_migration)
    migration_success.add(pdp.isCheckmateSuccess());
#endif
  if (pdp.isLoopDetection()) {
#ifdef OSL_SMP
    std::lock_guard<std::mutex> lk(shared->mutex);
#endif
    shared->disproof_table[key].push_front(path);
  }
  if (pdp.isCheckmateSuccess())
    assert(best_move.isNormal());
  return pdp;
}

osl::checkmate::ProofDisproof osl::checkmate::
DualDfpn::findProof(int node_limit, const NumEffectState& state, 
		    const HashKey& key, const PathEncoding& path,
		    Move& best_move, Move last_move)
{
  if (state.turn() == BLACK)
    return findProof<BLACK>(node_limit, state, key, path, best_move, last_move);
  else
    return findProof<WHITE>(node_limit, state, key, path, best_move, last_move);
}

bool osl::checkmate::
DualDfpn::isWinningState(int node_limit, const NumEffectState& state, 
			   const HashKey& key, const PathEncoding& path,
			   Move& best_move, Move last_move)
{
  return findProof(node_limit, state, key, path, best_move, last_move)
    .isCheckmateSuccess();
}

#ifdef OSL_DFPN_SMP
template <osl::Player P>
bool osl::checkmate::
DualDfpn::isWinningStateParallel(int node_limit, const NumEffectState& state, 
				 const HashKey& key, const PathEncoding& path,
				 Move& best_move, Move last_move)
{
  PieceStand proof_pieces;
  size_t count;
  ProofDisproof pdp;
  {
#ifdef OSL_SMP
    std::lock_guard<std::mutex> lk(shared->mutex);
#endif
    if (! shared->parallel_search)
      shared->parallel_search.reset(new DfpnParallel(std::min(OslConfig::concurrency(), 8)));
#ifdef DFPN_SHARE_TABLE
    shared->parallel_search->setTable(&(shared->table[P]));
#else
    shared->parallel_search->setTable(&(local->table[P]));
#endif

    pdp = shared->parallel_search->hasCheckmateMove
      (state, key, path, node_limit, best_move, proof_pieces, last_move);
    count = shared->parallel_search->nodeCount();
  }
  shared->addMainNodeCount(count);
  if (pdp.isCheckmateSuccess())
    shared->pool[P].addProof(state, key, proof_pieces);
  if (pdp.isLoopDetection()) {
    shared->disproof_table[key].push_front(path);
  }
  if (pdp.isCheckmateSuccess())
    assert(best_move.isNormal());
  return pdp.isCheckmateSuccess();
}

bool osl::checkmate::
DualDfpn::isWinningStateParallel(int node_limit, const NumEffectState& state, 
				 const HashKey& key, const PathEncoding& path,
				 Move& best_move, Move last_move)
{
  if (state.turn() == BLACK)
    return isWinningStateParallel<BLACK>(node_limit, state, key, path, best_move, last_move);
  else
    return isWinningStateParallel<WHITE>(node_limit, state, key, path, best_move, last_move);
}
#endif

template <osl::Player P>
bool
 osl::checkmate::
DualDfpn::isLosingState(int node_limit, const NumEffectState& state, 
			const HashKey& key, const PathEncoding& path,
			Move last_move)
{
  Shared::TableUseLock lk(&*shared);
  assert(state.turn() == P);
  Dfpn& dfpn = prepareDfpn(alt(P));
  const ProofDisproof pdp = dfpn.hasEscapeMove(state, key, path, node_limit, last_move);
  const size_t count = dfpn.nodeCount();
  local->local_node_count += count;
  shared->addMainNodeCount(count);
  return pdp.isCheckmateSuccess();
}

bool osl::checkmate::
DualDfpn::isLosingState(int node_limit, const NumEffectState& state, 
			  const HashKey& key, const PathEncoding& path,
			  Move last_move)
{
  if (state.turn() == BLACK)
    return isLosingState<BLACK>(node_limit, state, key, path, last_move);
  else
    return isLosingState<WHITE>(node_limit, state, key, path, last_move);
}

void osl::checkmate::
DualDfpn::writeRootHistory(const RepetitionCounter& counter,
			   const MoveStack& moves,
			   const SimpleState& state, Player attack)
{
  // TODO: 局面表をクリアしてしまうと忘れられる => DualDfpn 内で記憶した方が良い
  Shared::TableUseLock lk(&*shared);
  Dfpn& dfpn = prepareDfpn(attack);
  PieceStand white_stand(WHITE, state);
  for (int i=0; i<counter.checkCount(attack); ++i)
  {
    const HashKey& key = counter.history().top(i);
    if (key != counter.history().top(0)) // ignore current state
    {
      dfpn.setIllegal(key, white_stand);
    }
    assert(moves.hasLastMove(i+1)); // oops, different index
    if (! moves.hasLastMove(i+1))
      break;
    const Move last_move = moves.lastMove(i+1);
    if (last_move.isNormal())
      white_stand = white_stand.previousStand(WHITE, last_move);
  }
}

void osl::checkmate::
DualDfpn::setRootPlayer(Player root)
{
  shared->blocking_verify[root] = true;
  shared->blocking_verify[alt(root)] = true; // TODO: set false when issues around proof pieces are corrected
}

void osl::checkmate::
DualDfpn::setVerbose(int /*level*/)
{
}

int osl::checkmate::
DualDfpn::distance(Player attack, const HashKey& key)
{
  Shared::TableUseLock lk(&*shared);
  return prepareDfpn(attack).distance(key);
}

size_t osl::checkmate::
DualDfpn::mainNodeCount() const
{
#ifdef OSL_USE_RACE_DETECTOR
  std::lock_guard<std::mutex> lk(shared->mutex);
#endif
  return shared->main_node_count;
  // return shared->table[BLACK].totalSize() + shared->table[WHITE].totalSize();
}

size_t osl::checkmate::
DualDfpn::totalNodeCount() const
{
#ifdef OSL_USE_RACE_DETECTOR
  std::lock_guard<std::mutex> lk(shared->mutex);
#endif
  return shared->main_node_count + shared->simulation_count;
}

const osl::checkmate::DfpnTable& osl::checkmate::
DualDfpn::table(Player attack) const
{
  return shared->table[attack];
}

namespace osl
{
  template ProofDisproof checkmate::DualDfpn::findProof<BLACK>
  (int, const NumEffectState&, const HashKey&, const PathEncoding&,
   Move&, Move);
  template ProofDisproof checkmate::DualDfpn::findProof<WHITE>
  (int, const NumEffectState&, const HashKey&, const PathEncoding&,
   Move&, Move);


  template bool checkmate::DualDfpn::isLosingState<BLACK>
  (int, const NumEffectState&, const HashKey&, const PathEncoding&, Move);
  template bool checkmate::DualDfpn::isLosingState<WHITE>
  (int, const NumEffectState&, const HashKey&, const PathEncoding&, Move);

#ifdef OSL_DFPN_SMP
  template bool checkmate::DualDfpn::isWinningStateParallel<BLACK>
  (int, const NumEffectState&, const HashKey&, const PathEncoding&, Move&, Move);
  template bool checkmate::DualDfpn::isWinningStateParallel<WHITE>
  (int, const NumEffectState&, const HashKey&, const PathEncoding&, Move&, Move);
#endif
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
