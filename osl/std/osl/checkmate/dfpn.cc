/* dfpn.cc
 */
#include "osl/checkmate/dfpn.h"
#include "osl/checkmate/dfpnParallel.h"
#include "osl/checkmate/dfpnRecord.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/fixedDepthSolverExt.h"
#include "osl/checkmate/libertyEstimator.h"
#include "osl/checkmate/pieceCost.h"
#include "osl/checkmate/proofPieces.h"
#include "osl/checkmate/disproofPieces.h"
#include "osl/checkmate/oracleAdjust.h"
#include "osl/checkmate/pawnCheckmateMoves.h"
#include "osl/checkmate/proofTreeDepthDfpn.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_classifier/check_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/csa.h"

#include "osl/stat/ratio.h"
#include "osl/hash/hashRandomPair.h"
#include "osl/bits/align16New.h"
#include "osl/oslConfig.h"
#include <tuple>
#include <unordered_map>
#include <vector>
#include <forward_list>
#include <iostream>
#include <iomanip>
#include <bitset>

// see dfpnRecord.h for #defined NAGAI_DAG_TEST

#define GRAND_PARENT_SIMULATION
#define GRAND_PARENT_DELAY

#define INITIAL_DOMINANCE
/** root で打切る証明数の閾値 */
#define ROOT_PROOF_TOL 65536ul*1024
/** root で打切る反証数の閾値 */
#define ROOT_DISPROOF_TOL 65536ul*1024
// #define DELAY_UPWARD
// #define NO_IMMEDIATE_CHECKMATE
#define CHECKMATE_D2
// #define CHECKMATE_A3
#define PROOF_AVERAGE
#define DISPROOF_AVERAGE

#define KAKINOKI_FALSE_BRANCH_SEARCH
#define IGNORE_MONSTER_CHILD
#define KISHIMOTO_WIDEN_THRESHOLD
#define ADHOC_SUM_RESTRICTION
#define AGGRESSIVE_FIND_DAG
#define AGGRESSIVE_FIND_DAG2
#define CHECKMATE_A3_GOLD
#define CHECKMATE_A3_SIMULLATION

// 何番目に生成された指手が解決済みかを記録。生成順序は駒番号に依存するので注意。
#define MEMORIZE_SOLVED_IN_BITSET

// #define DFPN_STAT

static const int UpwardWeight = 2, SacrificeBlockCount = 0, LongDropCount = 1;
#ifdef MINIMAL
static const int MaxDagTraceDepth = 64;
#else
static const int MaxDagTraceDepth = 1600;
#endif
static const unsigned int NoPromoeIgnoreProofThreshold = 100;
static const unsigned int NoPromoeIgnoreDisproofThreshold = 200;
static const unsigned int IgnoreUpwardProofThreshold = 100;
static const unsigned int IgnoreUpwardDisproofThreshold = 100;
#ifdef MEMORIZE_SOLVED_IN_BITSET
static const unsigned int InitialDominanceProofMax = 35;
#else
static const unsigned int InitialDominanceProofMax = 20;
#endif
static const unsigned int InitialDominanceDisproofMax = 110;
static const unsigned int DagFindThreshold = 64;
static const unsigned int DagFindThreshold2 = 256;
static const int EnableGCDepth = 512;
static const int AdHocSumScale = 128;
static const size_t GrowthLimitInfty = std::numeric_limits<size_t>::max();
const int ProofSimulationTolerance = 1024;

// #define DFPN_DEBUG

#ifndef NDEBUG
static size_t timer = 0;
const size_t debug_time_start = 3851080;
#endif
/* ------------------------------------------------------------------------- */

namespace osl
{
  namespace checkmate
  {
    inline int log2(uint32_t n) 
    {
      return (n <= 1) ? 1 : 32 - __builtin_clz(n);
    }
    inline int slow_increase(uint32_t n)
    {
      return log2(n);
    }
#ifdef DFPN_DEBUG
    struct NodeIDTable : public std::unordered_map<HashKey, int, std::hash<HashKey>> 
    {
      size_t cur;
      NodeIDTable() : cur(0) {}
      int id(const HashKey& key) 
      {
	int& ret = (*this)[key];
	if (ret == 0)
	  ret = ++cur;
	return ret;
      }
    } node_id_table;
    CArray<int,3> debug_node = {{
      }};
    /** id => frequency */
    struct NodeCountTable : public std::unordered_map<int, std::pair<int,std::vector<Move> > >
    {
      typedef std::pair<int,std::vector<Move> > pair_t;
      ~NodeCountTable()
      {
	std::cerr << "timer " << timer << "\n";
	vector<std::pair<int,int> > all; 
	all.reserve(size());
	for (const auto& v: *this)
	  all.push_back(std::make_pair(-v.second.first, v.first));
	std::sort(all.begin(), all.end());
	for (size_t i=0; i<std::min((size_t)10, size()); ++i){
	  std::cerr << "freq " << -all[i].first << " id " << std::setw(5) << all[i].second << ' ';
	  for (Move m: (*this)[all[i].second].second)
	    std::cerr << record::csa::show(m);
	  std::cerr << "\n";
	}
      }
    } node_count_table;
#endif

    struct SimpleTwinList : std::forward_list<PathEncoding>
    {
    };

    struct DfpnPathRecord
    {
      static const int MaxDistance = 1024*128;
      SimpleTwinList twin_list;
      /** distance from root */
      int distance;
      bool visiting;
      size_t node_count;
      DfpnPathRecord() 
	: distance(MaxDistance), visiting(false), node_count(0)
      {
      }
    };
    template <bool Enabled=true>
    struct DfpnVisitLock 
    {
      DfpnVisitLock(const DfpnVisitLock&) = delete;
      DfpnVisitLock& operator=(const DfpnVisitLock&) = delete;

      DfpnPathRecord *record;
      DfpnVisitLock(DfpnPathRecord *r) : record(r)
      {
	if (! Enabled) return;
	assert(! record->visiting);
	record->visiting = true;
      }
      ~DfpnVisitLock()
      {
	if (! Enabled) return;
	assert(record->visiting);
	record->visiting = false;
      }
    };
    enum LoopToDominance { NoLoop=0, BadAttackLoop };
    struct DfpnPathList : public std::forward_list<std::pair<PieceStand, DfpnPathRecord>>
    {
      typedef std::forward_list<std::pair<PieceStand, DfpnPathRecord> > list_t;
    private:
      template <Player Attack>
      iterator find(PieceStand black, LoopToDominance& loop)
      {
	loop = NoLoop;
	iterator ret = end();
	for (iterator p=begin(); p!=end(); ++p) {
	  if (p->first == black) {
	    assert(p->second.distance != DfpnPathRecord::MaxDistance);
	    ret = p;
	    if (loop || p->second.visiting) break;
	  } 
	  if (! p->second.visiting)
	    continue;
	  if (p->first.isSuperiorOrEqualTo(black)) {
	    if (Attack == BLACK) {
	      loop = BadAttackLoop;
	      if (ret != end()) break;
	    } 
	  } 
	  else if (black.isSuperiorOrEqualTo(p->first)) {  
	    if (Attack == WHITE) {
	      loop = BadAttackLoop;
	      if (ret != end()) break;
	    }	    
	  }
	}
	return ret;
      }
    public:
      template <Player Attack>
      DfpnPathRecord *allocate(PieceStand black, int depth, LoopToDominance& loop,
			       size_t& size)
      {
	iterator ret = find<Attack>(black, loop);
	if (ret != end()) {
	  ret->second.distance = std::min(depth, ret->second.distance);
	  return &(ret->second);
	}
	++size;
	push_front(std::make_pair(black, DfpnPathRecord()));
	DfpnPathRecord *record = &(begin()->second);
	assert(record->distance == DfpnPathRecord::MaxDistance);
	record->distance = depth;
	return record;
      }
      const DfpnPathRecord *probe(PieceStand black) const
      {
	for (const auto& v: *this) {
	  if (v.first == black)
	    return &(v.second);
	}
	return 0;
      }
      static bool precious(const DfpnPathRecord& record, size_t threshold)
      {
	return record.visiting 
	  || record.node_count > threshold
	  || (! record.twin_list.empty() && record.node_count > threshold - 10);
      }
      size_t runGC(size_t threshold)
      {
	size_t removed = 0;
	list_t::iterator p=begin();
	while (p!=end()) {
	  list_t::iterator q=p;
	  ++q;
	  if (q == end())
	    break;
	  if (! precious(q->second, threshold)) {
	    erase_after(p);
	    ++removed;
	    continue;
	  }
	  p = q;
	}
	if (! empty() && ! precious(begin()->second, threshold)) {
	  pop_front(); // erase(begin());
	  ++removed;
	}
	return removed;
      }
    };
    class DfpnPathTable 
    {
      typedef std::unordered_map<BoardKey, DfpnPathList, std::hash<BoardKey>> table_t;
      table_t table;
      size_t total_size;
      size_t gc_threshold;
    public:
      DfpnPathTable() : total_size(0), gc_threshold(10)
      {
      }
      template <Player Attack>
      DfpnPathRecord *allocate(const HashKey& key, int depth, LoopToDominance& loop) 
      {
	DfpnPathList& l = table[key.boardKey()];
	return l.allocate<Attack>(key.blackStand(), depth, loop,
				  total_size);
      }
      const DfpnPathRecord *probe(const HashKey& key) const
      {
	table_t::const_iterator p = table.find(key.boardKey());
	if (p == table.end()) 
	  return 0;
	return p->second.probe(key.blackStand());
      }
      void clear() { table.clear(); }
      size_t runGC()
      {
	size_t removed = 0;
	for (table_t::iterator p=table.begin(); p!=table.end(); ++p)
	  removed += p->second.runGC(gc_threshold);
	total_size -= removed;
	gc_threshold += 15;
	static double memory_threshold = 0.8;
	double memory = OslConfig::memoryUseRatio();
	if (memory > memory_threshold) {
	  gc_threshold += 15;
	  memory_threshold += 1.0/128;
	}
	return removed;
      }
      size_t size() const { return total_size; }
      void rehash(size_t bucket_size) { table.rehash(bucket_size); }
    };

    int attackProofCost(Player attacker, const NumEffectState& state, Move move)
    {
      int proof = 0;
      if (! move.isCapture())
      {
	const Square from=move.from(), to=move.to();
	const int a = (state.countEffect(attacker,to) 
		       + (from.isPieceStand() ? 1 : 0));
	int d = state.countEffect(alt(attacker),to);
	if (a <= d)
	{
	  const Ptype ptype = move.ptype();
	  proof = PieceCost::attack_sacrifice_cost[ptype];
	  if ((d >= 2) && (a == d))	// 追加利きとか利きがずれたりとか
	    proof /= 2;
	}
      }
      return proof;
    }
  }
}

/* ------------------------------------------------------------------------- */
struct osl::checkmate::Dfpn::NodeBase
{
  // input
  HashKey hash_key;
  PathEncoding path;
  ProofDisproof threshold;
  Move moved;
  PieceStand white_stand;
  // work or output
  DfpnRecord record;
  DfpnPathRecord *path_record;
};

struct osl::checkmate::Dfpn::Node : NodeBase
{
  DfpnMoveVector moves;
  FixedCapacityVector<DfpnRecord,DfpnMaxUniqMoves> children;
  FixedCapacityVector<const DfpnPathRecord*,DfpnMaxUniqMoves> children_path;
  CArray<HashKey,DfpnMaxUniqMoves> hashes;
  FixedCapacityVector<int8_t,DfpnMaxUniqMoves> proof_cost; // only attack
  size_t visit_time;

  const PieceStand nextWhiteStand(Player P, Move move) const
  {
    assert(move.player() == P);
    return (P == WHITE) ? white_stand.nextStand(P, move) : white_stand;
  }
  void clear()
  {
    moves.clear();
    proof_cost.clear();
    children.clear();
    children_path.clear();
  }
  void allocate(int n)
  {
    while (n--) {
      proof_cost.push_back(0);
      children.push_back(DfpnRecord());
      children_path.push_back(0);
    }
  }
  void setLoopDetection()
  {
    assert(! (record.proof_disproof.isFinal()
	      && ! record.proof_disproof.isLoopDetection()));
    record.proof_disproof = ProofDisproof(1,1);
    path_record->twin_list.push_front(path);
  }
  const PathEncoding newPath(int c) const
  {
    PathEncoding n = path;
    n.pushMove(moves[c]);
    return n;
  }
  bool isLoop(int c) const
  {
    if (! children_path[c] || children[c].proof_disproof.isFinal())
      return false;
    if (children_path[c]->visiting)
      return true;
    const PathEncoding p = newPath(c);
    const SimpleTwinList& tl = children_path[c]->twin_list;
    return std::find(tl.begin(), tl.end(), p) != tl.end();
  }
  void setCheckmateAttack(Player attack, int best_i)
  {
    DfpnRecord& child = children[best_i];
    assert(child.proof_disproof.isCheckmateSuccess());
    record.proof_disproof = child.proof_disproof;
    record.best_move = moves[best_i];
    const PieceStand proof_pieces 
      = ProofPieces::attack(child.proofPieces(), record.best_move,
			    record.stands[attack]);
    record.setProofPieces(proof_pieces);
  }
  void setNoCheckmateDefense(Player attack, int best_i)
  {
    DfpnRecord& child = children[best_i];
    assert(child.proof_disproof.isCheckmateFail());
    assert(! child.proof_disproof.isLoopDetection());
    record.proof_disproof = child.proof_disproof;
    record.best_move = moves[best_i];
    const PieceStand disproof_pieces 
      = DisproofPieces::defense(child.disproofPieces(), record.best_move,
				record.stands[alt(attack)]);
    record.setDisproofPieces(disproof_pieces);
  }
  void setCheckmateDefense(Player attack, const NumEffectState& state) 
  {
    assert(moves.size());
    assert(record.proof_disproof.isCheckmateSuccess());
    record.proof_disproof = ProofDisproof::Checkmate(); // prevent backup of NoEscape
    PieceStand result = record.proof_pieces_candidate;
    const Player defender = alt(attack);
    if (! state.inUnblockableCheck(defender))
      ProofPiecesUtil::addMonopolizedPieces(state, attack, record.stands[attack],
					    result);
    record.setProofPieces(result);
  }
  void setNoCheckmateAttack(Player attack, const NumEffectState& state)
  {
    assert(moves.size());
    assert(record.proof_disproof.isCheckmateFail());
    assert(! record.proof_disproof.isLoopDetection());
    PieceStand result = record.proof_pieces_candidate;
    ProofPiecesUtil::addMonopolizedPieces(state, alt(attack), record.stands[alt(attack)],
					  result);
    record.setDisproofPieces(result);
  }
  void setCheckmateChildInDefense(size_t i) 
  {
    assert(children[i].proof_disproof.isCheckmateSuccess());
#ifdef MEMORIZE_SOLVED_IN_BITSET
    record.solved |= (1ull<<i);
#endif
    record.min_pdp = std::min(record.min_pdp, children[i].proof_disproof.disproof());
    record.proof_pieces_candidate
      = record.proof_pieces_candidate.max(children[i].proofPieces());
  }
  void setNoCheckmateChildInAttack(size_t i) 
  {
    assert(children[i].proof_disproof.isCheckmateFail());
#ifdef MEMORIZE_SOLVED_IN_BITSET
    record.solved |= (1ull<<i);
#endif
    record.min_pdp = std::min(record.min_pdp, children[i].proof_disproof.proof());
    record.proof_pieces_candidate
      = record.proof_pieces_candidate.max(children[i].disproofPieces());
  }
};

struct osl::checkmate::Dfpn::Tree
#if OSL_WORDSIZE == 32
  : public misc::Align16New
#endif
{
  NumEffectState state;
  int depth;
#ifdef MINIMAL
  enum { MinimalMaxDepth = 256 };
  Node node[MinimalMaxDepth];
#else
  boost::scoped_array<Node> node;
#endif
  const int MaxDepth;
  Tree(int
#ifndef MINIMAL
       max_depth
#endif
    ) : state(SimpleState(HIRATE)),
	MaxDepth(
#ifndef MINIMAL
	  max_depth
#else
	  MinimalMaxDepth
#endif
	  )
  {
#ifndef MINIMAL
    node.reset(new Node[max_depth]);
#endif
  }
  bool inCheck(Player P) const
  {
    return state.inCheck(P);
  }
  const Piece king(Player P) const { return state.kingPiece(P); }
  void newVisit(Player P, Move move, const HashKey& next_hash)
  {
    assert(P == move.player());
    const Node& node = this->node[depth];
    assert(next_hash == node.hash_key.newHashWithMove(move));
    Node& next = this->node[depth+1];
    next.moved = move;
    next.white_stand = node.nextWhiteStand(P, move);
    next.path = node.path;
    next.clear();
    next.hash_key = next_hash;
  }
  void setNoCheckmateChildInAttack(size_t best_i) 
  {
    Node &node = this->node[depth];
    node.setNoCheckmateChildInAttack(best_i);
  }
  void setNoCheckmateDefense(Player attack, int best_i)
  {
    Node &node = this->node[depth];
    node.setNoCheckmateDefense(attack, best_i);
  }
  void dump(int lines, int depth=0) const
  {
#ifndef NDEBUG
    if (depth == 0)
      depth = this->depth;
    for (int i=0; i<=depth; ++i) {
      std::cerr << "history " << i << " " << node[i].moved << " ";
      node[i].hash_key.dumpContentsCerr();
      std::cerr << "\n";
    }
    const int my_distance = node[depth].path_record ? node[depth].path_record->distance : -1;
    const Node &node = this->node[depth];
    std::cerr << "time " << node.visit_time << " (" << timer << ") here " << lines << "\n" << state;
    std::cerr << " false-branch? " << (bool)node.record.false_branch << "\n";
#ifdef MEMORIZE_SOLVED_IN_BITSET
    std::cerr << " solved " << std::bitset<32>(node.record.solved) << "\n";
#endif
    std::cerr << " dags   " << std::bitset<32>(node.record.solved) << "\n";
    std::cerr << " last_to " << node.record.last_to
	      << " threshold " << node.threshold
	      << " my_distance " << my_distance << "\n";
    for (size_t i=0; i<node.moves.size(); ++i) {
      std::cerr << "  " << i << " " << node.moves[i]
		<< " " << node.children[i].proof_disproof
		<< " " << (int)node.proof_cost[i]
		<< " " << node.children[i].best_move
		<< " depth " << (node.children_path[i] ? node.children_path[i]->distance : -1)
		<< " count " << node.children[i].node_count
		<< "\n";
    }
    std::cerr << node.record.proof_disproof << " " << node.record.best_move << "\n";
    std::cerr << "path " << node.path << " twins ";
    if (node.path_record) {
      for (const auto& path: node.path_record->twin_list)
	std::cerr << path << " ";
    }
    std::cerr << "\n";
#endif
  }
#ifdef DFPN_DEBUG
  void showPath(const char *message, size_t table_size) const
  {
    std::cerr << message << " depth " << depth << " node " << node_id_table.id(node[depth].hash_key)
	      << " time " << timer << " table " << table_size << ' ';
    for (int i=0; i<=depth; ++i)
      std::cerr << record::csa::show(node[i].moved);
    std::cerr << "\n";
  }
  struct Logging
  {
    const Tree *tree;
    const DfpnTable *table;
    const size_t old_table_size;
    Logging(Tree *tr, DfpnTable *tb, const char *message)
      : tree(tr), table(tb), old_table_size(table->size())
    {
      if (timer < debug_time_start)
	return;
      tree->showPath(message, old_table_size);
    }
    ~Logging()
    {
      if (timer < debug_time_start)
	return;
      const Node& node = tree->node[tree->depth];
      const int id = node_id_table.id(node.hash_key);
      std::cerr << " node " << id << " " << node.threshold
		<< " " << node.record.proof_disproof << "\n";
      if (std::find(debug_node.begin(), debug_node.end(), id)
	  != debug_node.end() && timer > debug_time_start)
	tree->dump(__LINE__);
      if (table->size() == old_table_size)
	countImmediateReturns(id);
    }
    void countImmediateReturns(int id)
    {
      NodeCountTable::pair_t& p = node_count_table[id];
      if (p.first == 0) {
	for (int i=0; i<=tree->depth; ++i)
	  p.second.push_back(tree->node[i].moved);
      }
      ++(p.first);
    }
  };
#endif
};

/* ------------------------------------------------------------------------- */
#ifdef DFPN_STAT
osl::CArray<osl::CArray<int,64>,2> count2proof, count2disproof, count2unknown;
#endif

struct osl::checkmate::DfpnTable::List : public std::forward_list<DfpnRecord>
{
  typedef std::forward_list<DfpnRecord> list_t;
#ifdef OSL_DFPN_SMP
  mutable Mutex mutex;
#endif
  List() {}
  List(const List& src) : list_t(src) {}

  template <Player Attack>
  const DfpnRecord probe(const HashKey& key, PieceStand white_stand) const;
  template <Player Attack>
  const DfpnRecord findProofOracle(const HashKey& key, PieceStand white_stand, Move last_move) const;
  template <Player Attack>
  void showProofOracles(const HashKey& key, PieceStand white_stand, Move last_move) const;
  bool store(DfpnRecord& value, int leaving_thread_id)
  {
#ifdef USE_TBB_HASH
    SCOPED_LOCK(lk,mutex);
#endif
    for (DfpnRecord& record: *this) {
      if (record.stands[BLACK] == value.stands[BLACK]) {
#ifdef OSL_DFPN_SMP
	if (record.proof_disproof.isFinal()) {
	  value = record;
	  record.working_threads &= ~(1u << leaving_thread_id);
	  return false;
	}
	if (! value.proof_disproof.isFinal()) {
	  value.min_pdp = std::min(value.min_pdp, record.min_pdp);
	  value.proof_pieces_candidate 
	    = value.proof_pieces_candidate.max(record.proof_pieces_candidate);
	  value.dag_moves |= record.dag_moves;
	  value.solved |= record.solved;
	  value.false_branch |= record.false_branch;
	}
	value.working_threads = record.working_threads;
	if (leaving_thread_id >= 0) {
	  assert(value.working_threads & (1u << leaving_thread_id));
	  value.working_threads &= ~(1u << leaving_thread_id);
	}
#endif
	record = value;
	return false;
      }
    }
    value.working_threads &= ~(1u << leaving_thread_id);
    push_front(value);
    return true;
  }
  void addDag(DfpnRecord& value)
  {
#ifdef USE_TBB_HASH
    SCOPED_LOCK(lk,mutex);
#endif
    for (DfpnRecord& record: *this) {
      if (record.stands[BLACK] == value.stands[BLACK]) {
#ifdef OSL_DFPN_SMP
	value.min_pdp = std::min(value.min_pdp, record.min_pdp);
	value.proof_pieces_candidate
	  = value.proof_pieces_candidate.max(record.proof_pieces_candidate);
	value.dag_moves |= record.dag_moves;
	value.solved |= record.solved;
	value.false_branch |= record.false_branch;
	value.working_threads = record.working_threads;
#endif
	record.dag_moves = value.dag_moves;
	return;
      }
    }
  }
  bool setWorking(const DfpnRecord& value, int thread_id)
  {
#ifdef USE_TBB_HASH
    SCOPED_LOCK(lk,mutex);
#endif
    for (DfpnRecord& record: *this) {
      if (record.stands[BLACK] == value.stands[BLACK]) {
	assert(! (value.working_threads & (1u << thread_id)));
	record.working_threads |= 1u << thread_id;
	return false;
      }
    }
    push_front(value);
    front().working_threads |= 1u << thread_id;
    return true;
  }
  void leaveWorking(PieceStand black, int thread_id)
  {
#ifdef USE_TBB_HASH
    SCOPED_LOCK(lk,mutex);
#endif
    for (DfpnRecord& record: *this) {
      if (record.stands[BLACK] == black) {
	// assert(p->working_threads & (1u << thread_id)); // fail when stop_all
	record.working_threads &= ~(1u << thread_id);
	return;
      }
    }
    // assert(0); // fail when stop_all
  }
  void testTable(const BoardKey& /*key*/) const
  {
#ifdef USE_TBB_HASH
    SCOPED_LOCK(lk,mutex);
#endif
    for (const DfpnRecord& record: *this) {
      if (record.working_threads)
	std::cerr << std::bitset<16>(record.working_threads) << "\n";
      assert(record.working_threads == 0);
#ifdef DFPN_STAT
      const int count = misc::BitOp::countBit(record.solved);
      if (record.proof_disproof.isCheckmateSuccess())
	count2proof[key.turn()][count]++;
      else if (record.proof_disproof.isCheckmateFail())
	count2disproof[key.turn()][count]++;
      else
	count2unknown[key.turn()][count]++;
#endif
    }
  }
  size_t smallTreeGC(size_t threshold)
  {
    size_t removed = 0;
#ifdef USE_TBB_HASH
    SCOPED_LOCK(lk,mutex);
#endif
    list_t::iterator p=begin();
    while (p!=end()) {
      list_t::iterator q=p;
      ++q;
      if (q == end())
	break;
      if (! q->proof_disproof.isFinal()
#ifdef OSL_DFPN_SMP
	  && q->working_threads == 0
#endif
	  && q->node_count < threshold) {
	erase_after(p);
	++removed;
	continue;
      }
      p = q;
    }
    if (! empty() && ! begin()->proof_disproof.isFinal()
#ifdef OSL_DFPN_SMP
	&& begin()->working_threads == 0
#endif
	&& begin()->node_count < threshold) {
      pop_front(); // erase(begin())
      ++removed;
    }
    return removed;
  }
  size_t estimateNodeCount(const HashKey& key, bool dominance_max) const;
};
template <osl::Player A>
const osl::checkmate::DfpnRecord osl::checkmate::DfpnTable::
List::probe(const HashKey& key, PieceStand white_stand) const
{
#ifdef USE_TBB_HASH
    SCOPED_LOCK(lk,mutex);
#endif
  DfpnRecord result(key.blackStand(), white_stand);
  const PieceStand attack_stand = (A == BLACK) ? key.blackStand() : white_stand;
  const PieceStand defense_stand = (A == BLACK) ? white_stand : key.blackStand();
#ifdef INITIAL_DOMINANCE
  unsigned int proof_ll = 1, disproof_ll = 1;
#endif
  for (const DfpnRecord& record: *this) {
    if (record.stands[BLACK] == key.blackStand()) {
      result = record;
      if (result.proof_disproof.isFinal())
	break;
      continue;
    }
    if (record.proof_disproof.isCheckmateSuccess()) {
      if (attack_stand.isSuperiorOrEqualTo(record.proofPieces())) {
	result.setFrom(record);
	break;
      }
    }
    else if (record.proof_disproof.isCheckmateFail()) {
      if (defense_stand.isSuperiorOrEqualTo(record.disproofPieces())) {
	result.setFrom(record);
	break;
      }
    }
#ifdef INITIAL_DOMINANCE
    else {
      if (record.stands[A].isSuperiorOrEqualTo(attack_stand)) {
	proof_ll = std::max(proof_ll, record.proof());
      }
      else if (attack_stand.isSuperiorOrEqualTo(record.stands[A])) {
	disproof_ll = std::max(disproof_ll, record.disproof());
      }
    }
#endif
  }
#ifdef INITIAL_DOMINANCE
  if (result.proof_disproof == ProofDisproof(1,1)) {
      result.proof_disproof = ProofDisproof(std::min(proof_ll, InitialDominanceProofMax), 
					    std::min(disproof_ll, InitialDominanceDisproofMax));
      result.node_count++;	// not suitable for proof_average
  }
#endif
  return result;
}

size_t osl::checkmate::DfpnTable::
List::estimateNodeCount(const HashKey& key, bool dominance_max) const
{
#ifdef USE_TBB_HASH
  SCOPED_LOCK(lk,mutex);
#endif
  size_t node_count = 0, exact = 0;
  for (const DfpnRecord& record: *this) {
    if (node_count < record.node_count)
      node_count = record.node_count;
    if (key.blackStand() == record.stands[BLACK])
      exact = record.node_count;
  }
  return dominance_max ? node_count : exact;
}

template <osl::Player A>
const osl::checkmate::DfpnRecord osl::checkmate::DfpnTable::
List::findProofOracle(const HashKey& key, PieceStand white_stand, Move last_move) const
{
#ifdef USE_TBB_HASH
  SCOPED_LOCK(lk,mutex);
#endif
  const PieceStand attack_stand = (A == BLACK) ? key.blackStand() : white_stand;
  DfpnRecord result(key.blackStand(), white_stand);
  for (const DfpnRecord& record: *this) {
    if (! record.proof_disproof.isCheckmateSuccess()) 
      continue;
    if (attack_stand.isSuperiorOrEqualTo(record.proofPieces())) {
      result.setFrom(record);
      ++record.node_count;
      if (record.last_move == last_move)
	break;
    }
  }
  return result;
}

#ifndef MINIMAL
template <osl::Player A>
void osl::checkmate::DfpnTable::
List::showProofOracles(const HashKey& key, PieceStand white_stand, Move last_move) const
{
#ifdef USE_TBB_HASH
  SCOPED_LOCK(lk,mutex);
#endif
  const PieceStand attack_stand = (A == BLACK) ? key.blackStand() : white_stand;
  for (const DfpnRecord& record: *this) {
    if (! record.proof_disproof.isCheckmateSuccess()) 
      continue;
    if (attack_stand.isSuperiorOrEqualTo(record.proofPieces())) {
      std::cerr << record.last_move << " " << record.best_move << " " << record.node_count << " " << record.proofPieces()
		<< " " << record.stands[BLACK] << " " << record.stands[WHITE] << "\n";
    }
  }
}
#endif

struct osl::checkmate::DfpnTable::Table : public std::unordered_map<BoardKey, List, std::hash<BoardKey>> 
{
  Player attack;
  explicit Table(Player a=BLACK) : attack(a) {}
};


osl::checkmate::
DfpnTable::DfpnTable(Player attack) 
  : table(new Table[DIVSIZE]), total_size(0), dfpn_max_depth(0),
    growth_limit(GrowthLimitInfty), 
    gc_threshold(10)    
{
  setAttack(attack);
}

osl::checkmate::
DfpnTable::DfpnTable() 
  : table(new Table[DIVSIZE]), total_size(0), dfpn_max_depth(0)
{
}
osl::checkmate::
DfpnTable::~DfpnTable()
{
}

void osl::checkmate::
DfpnTable::setGrowthLimit(size_t new_limit)
{
  growth_limit = new_limit;
  for (int i=0; i<DIVSIZE; ++i) {
    table[i].rehash(new_limit/DIVSIZE+new_limit/DIVSIZE/128+1);
  }
}

void osl::checkmate::
DfpnTable::showStats() const
{
  if (size()) {
    std::cerr << "total " << total_size << "\n";
    for (int i=0; i<DIVSIZE; ++i)
      std::cerr << "DfpnTable " << i << " " << table[i].size() << "\n";
  }
}

void osl::checkmate::
DfpnTable::setMaxDepth(int new_depth)
{
  dfpn_max_depth = new_depth;
}
int osl::checkmate::
DfpnTable::maxDepth() const
{
  return dfpn_max_depth;
}

void osl::checkmate::
DfpnTable::setAttack(Player a) 
{
  assert(size() == 0);
  for (int i=0; i<DIVSIZE; ++i)
    table[i].attack = a;
}

osl::Player osl::checkmate::
DfpnTable::attack() const
{
  return table[0].attack;
}

template <osl::Player Attack>
osl::checkmate::DfpnTable::List *
osl::checkmate::
DfpnTable::find(const HashKey& key, int subindex)
{
  assert(table[subindex].attack == Attack);
#ifdef USE_TBB_HASH
  Table::accessor it;
  if(!table[subindex].find(it,key.boardKey()))
    return 0;
  return &it->second;
#else
  Table::iterator p = table[subindex].find(key.boardKey());
  if (p == table[subindex].end())
    return 0;
  return &p->second;
#endif
}

template <osl::Player Attack>
const osl::checkmate::DfpnTable::List *
osl::checkmate::
DfpnTable::find(const HashKey& key, int subindex) const
{
  assert(table[subindex].attack == Attack);
  return find(key, subindex);
}

const osl::checkmate::DfpnTable::List *
osl::checkmate::
DfpnTable::find(const HashKey& key, int subindex) const
{
#ifdef USE_TBB_HASH
  Table::accessor it;
  if(!table[subindex].find(it,key.boardKey()))
    return 0;
  return &it->second;
#else
  Table::const_iterator p = table[subindex].find(key.boardKey());
  if (p == table[subindex].end())
    return 0;
  return &p->second;
#endif
}

template <osl::Player Attack>
const osl::checkmate::DfpnRecord osl::checkmate::
DfpnTable::probe(const HashKey& key, PieceStand white_stand) const
{
  const int i=keyToIndex(key);
#if (defined OSL_DFPN_SMP) && (! defined USE_TBB_HASH)
  SCOPED_LOCK(lk,mutex[i]);
#endif
  const List *l = find<Attack>(key, i);
  if (l == 0) 
    return DfpnRecord(key.blackStand(), white_stand);
  return l->probe<Attack>(key, white_stand);
}

const osl::checkmate::DfpnRecord osl::checkmate::
DfpnTable::probe(const HashKey& key, PieceStand white_stand) const
{
  if (table[0].attack == BLACK)
    return probe<BLACK>(key, white_stand);
  else
    return probe<WHITE>(key, white_stand);
}
template <osl::Player Attack>
const osl::checkmate::DfpnRecord osl::checkmate::
DfpnTable::findProofOracle(const HashKey& key, PieceStand white_stand, Move last_move) const
{
  const int i=keyToIndex(key);
#if (defined OSL_DFPN_SMP) && (! defined USE_TBB_HASH)
  SCOPED_LOCK(lk,mutex[i]);
#endif
  const List *l = find<Attack>(key, i);
  if (l == 0)
    return DfpnRecord(key.blackStand(), white_stand);
  return l->findProofOracle<Attack>(key, white_stand, last_move);
}
const osl::checkmate::DfpnRecord osl::checkmate::
DfpnTable::findProofOracle(const HashKey& key, PieceStand white_stand, Move last_move) const
{
  if (table[0].attack == BLACK)
    return findProofOracle<BLACK>(key, white_stand, last_move);
  else
    return findProofOracle<WHITE>(key, white_stand, last_move);
}

#ifndef MINIMAL
template <osl::Player Attack>
void osl::checkmate::
DfpnTable::showProofOracles(const HashKey& key, PieceStand white_stand, Move last_move) const
{
  const int i=keyToIndex(key);
#if (defined OSL_DFPN_SMP) && (! defined USE_TBB_HASH)
  SCOPED_LOCK(lk,mutex[i]);
#endif
  const List *l = find<Attack>(key, i);
  if (l == 0) 
    return;
  return l->showProofOracles<Attack>(key, white_stand, last_move);
}
#endif

size_t osl::checkmate::
DfpnTable::estimateNodeCount(const HashKey& key, bool dominance_max) const
{
  const int i=keyToIndex(key);
#if (defined OSL_DFPN_SMP) && (! defined USE_TBB_HASH)
  SCOPED_LOCK(lk,mutex[i]);
#endif
  const List *l = find(key, i);
  if (l == 0) 
    return 0;
  return l->estimateNodeCount(key, dominance_max);
}

void osl::checkmate::
DfpnTable::store(const HashKey& key, DfpnRecord& value, int leaving_thread_id)
{
  assert(key.blackStand() == value.stands[BLACK]);
  assert(! value.proof_disproof.isLoopDetection());
  const int i=keyToIndex(key);
#ifdef USE_TBB_HASH
  Table::accessor it;
  table[i].insert(it,key.boardKey());
  List& l = it->second;
#else
#  ifdef OSL_DFPN_SMP
  SCOPED_LOCK(lk,mutex[i]);
#  endif
  List& l = table[i][key.boardKey()];
#endif
  if (l.store(value, leaving_thread_id)) {
#ifdef OSL_USE_RACE_DETECTOR
    SCOPED_LOCK(lk,root_mutex);
    // __sync_fetch_and_add() ?
#endif
    total_size += 1;
  }
}
void osl::checkmate::
DfpnTable::addDag(const HashKey& key, DfpnRecord& value)
{
  assert(key.blackStand() == value.stands[BLACK]);
  assert(! value.proof_disproof.isLoopDetection());
  const int i=keyToIndex(key);
#ifdef USE_TBB_HASH
  Table::accessor it;
  table[i].insert(it,key.boardKey());
  List& l = it->second;
#else
#  ifdef OSL_DFPN_SMP
  SCOPED_LOCK(lk,mutex[i]);
#  endif
  List& l = table[i][key.boardKey()];
#endif
  l.addDag(value);
}

void osl::checkmate::
DfpnTable::setWorking(const HashKey& key, const DfpnRecord& value, int thread_id)
{
  assert(key.blackStand() == value.stands[BLACK]);
  const int i=keyToIndex(key);
#ifdef USE_TBB_HASH
  Table::accessor it;
  table[i].insert(it,key.boardKey());
  List& l = it->second;
#else
#  ifdef OSL_DFPN_SMP
  SCOPED_LOCK(lk,mutex[i]);
#  endif
  List& l = table[i][key.boardKey()];
#endif
  if (l.setWorking(value, thread_id)) {
#ifdef OSL_USE_RACE_DETECTOR
    SCOPED_LOCK(lk,root_mutex);
    // __sync_fetch_and_add() ?
#endif
    total_size += 1;
  }
}
void osl::checkmate::
DfpnTable::leaveWorking(const HashKey& key, int thread_id)
{
  const int i=keyToIndex(key);
#ifdef USE_TBB_HASH
  Table::accessor it;
  table[i].insert(it,key.boardKey());
  List& l = it->second;
#else
#  ifdef OSL_DFPN_SMP
  SCOPED_LOCK(lk,mutex[i]);
#  endif
  List& l = table[i][key.boardKey()];
#endif
  l.leaveWorking(key.blackStand(), thread_id);
}

void osl::checkmate::
DfpnTable::clear()
{
  total_size = 0;
  for (int i=0; i<DIVSIZE; ++i) {
#if (defined OSL_DFPN_SMP) && (! defined USE_TBB_HASH)
    SCOPED_LOCK(lk,mutex[i]);
#endif
    table[i].clear();
  }
}

void osl::checkmate::
DfpnTable::testTable()
{
  for (int i=0; i<DIVSIZE; ++i) {
#if (defined OSL_DFPN_SMP) && (! defined USE_TBB_HASH)
    SCOPED_LOCK(lk,mutex[i]);
#endif
    for (auto& v: table[i])
      v.second.testTable(v.first);
  }
#ifdef DFPN_STAT
  for (int i=0; i<16; ++i) {
    for (int j=0; j<2; ++j)
      std::cout << std::setw(9) << count2proof[j][i]
		<< std::setw(9) << count2disproof[j][i]
		<< std::setw(9) << count2unknown[j][i]
		<< "   ";
    std::cout << "\n";
  }
#endif
}

size_t osl::checkmate::
DfpnTable::smallTreeGC(size_t threshold)
{
  size_t removed = 0;
  for (int i=0; i<DIVSIZE; ++i) {
#if (defined OSL_DFPN_SMP) && (! defined USE_TBB_HASH)
    SCOPED_LOCK(lk,mutex[i]);
#endif
    Table::iterator p=table[i].begin();
    while (p!=table[i].end()) {
      removed += p->second.smallTreeGC(threshold);
      Table::iterator q = p;
      ++p;
      if (q->second.empty()) {
#ifdef USE_TBB_HASH
	table[i].erase(q->first);
#else
	table[i].erase(q);
#endif
      }
    }
  }
  total_size -= removed;
  return removed;
}

bool osl::checkmate::
DfpnTable::runGC()
{
  const size_t before = total_size;
  if (! (before >= growth_limit || (growth_limit - before) < growth_limit/8))
    return false;

  std::cerr << "run GC " << before << ' ' << gc_threshold << "\n";
  const size_t removed = smallTreeGC(gc_threshold);
  double memory = OslConfig::memoryUseRatio();
  std::cerr << " GC " << removed
	    << " entries "
	    << "collected " << std::setprecision(3)
	    << ((sizeof(HashKey)+sizeof(DfpnRecord)+sizeof(char*)*2)
		* removed / (1<<20)) << "MB "
	    << 100.0*removed/before << "%"
	    << " real " << memory*100 << "%"
    // << " (" << elapsed << " s)"
	    << "\n";
  gc_threshold += 15;
  static double memory_limit = 0.75;
  if (memory > memory_limit) {
    growth_limit -= growth_limit/8;
    gc_threshold += 15 + gc_threshold/4;
    memory_limit += 0.01;
  }
  if (removed < before*2/3)
    gc_threshold += 15 + gc_threshold/2;
  if ((removed < before*3/5 && memory > 0.75) || removed < before/2)
    throw Dfpn::DepthLimitReached();
  return true;
}


size_t osl::checkmate::
DfpnTable::size() const
{
  return total_size;
}

/* ------------------------------------------------------------------------- */

template <osl::Player P>
struct osl::checkmate::Dfpn::CallAttack
{
  Dfpn *search;
  CallAttack(Dfpn *s) : search(s)
  {
  }
  void operator()(Square) const 
  {
    search->attack<P>();
  }
};

template <osl::Player P>
struct osl::checkmate::Dfpn::CallDefense
{
  Dfpn *search;
  CallDefense(Dfpn *s) : search(s)
  {
  }
  void operator()(Square) const 
  {
    search->defense<P>();
  }
};

/* ------------------------------------------------------------------------- */


osl::checkmate::Dfpn::Dfpn()
  : table(0), tree(new Tree(OslConfig::dfpnMaxDepth())), path_table(new DfpnPathTable), parallel_shared(0),
    thread_id(-1), blocking_verify(true)
{
}
osl::checkmate::Dfpn::~Dfpn()
{
}

void osl::checkmate::Dfpn::setTable(DfpnTable *new_table)
{
  table = new_table; 
  table->setMaxDepth(tree->MaxDepth);
  if (tree->MaxDepth > EnableGCDepth
      && table->growthLimit() < GrowthLimitInfty)
    path_table->rehash(parallel_shared ? table->growthLimit()/4 : table->growthLimit());
}

void osl::checkmate::Dfpn::clear()
{
  path_table->clear();
}


void osl::checkmate::Dfpn::setIllegal(const HashKey& key, PieceStand white_stand)
{
  // path_table はDualDfpnでクリアされるのでこちらは現状ではおまじない
  LoopToDominance dummy;
  DfpnPathRecord *record = (table->attack() == BLACK)
    ? path_table->allocate<BLACK>(key, 0, dummy)
    : path_table->allocate<WHITE>(key, 0, dummy);
  record->visiting = true;

  // こちらが重要
  DfpnRecord result(key.blackStand(), white_stand);
  result.proof_disproof = ProofDisproof::NoCheckmate();
  result.setDisproofPieces((table->attack() == WHITE) ? key.blackStand() : white_stand);
  table->store(key, result);
}

const osl::checkmate::ProofDisproof 
osl::checkmate::
Dfpn::hasCheckmateMove(const NumEffectState& state, const HashKey& key,
		       const PathEncoding& path, size_t limit, Move& best_move, Move last_move,
		       std::vector<Move> *pv)
{
  PieceStand dummy;
  return hasCheckmateMove(state, key, path, limit, best_move, dummy, last_move, pv);
}

const osl::checkmate::ProofDisproof 
osl::checkmate::
Dfpn::hasCheckmateMove(const NumEffectState& state, const HashKey& key,
		       const PathEncoding& path, size_t limit, Move& best_move, PieceStand& proof_pieces,
		       Move last_move, std::vector<Move> *pv)
{
  assert(table);
  if (! table)
    return ProofDisproof();
  path_table->clear();
  
  node_count = 0;
  node_count_limit = limit;

  Node& root = tree->node[0];
  try {
    tree->state.copyFrom(state);
    tree->depth = 0;
    root.hash_key = key;
    root.path = path;
    root.clear();
    root.threshold = ProofDisproof(ROOT_PROOF_TOL, ROOT_DISPROOF_TOL);
    root.white_stand = PieceStand(WHITE, state);
    root.moved = last_move;
    if (state.turn() == BLACK)
      attack<BLACK>();
    else
      attack<WHITE>();
  }
  catch (DepthLimitReached&) {
    for (int i=0; i<=tree->depth; ++i)
      table->leaveWorking(tree->node[i].hash_key, thread_id);
    return ProofDisproof();
  }
  if (root.path_record
      && (std::find(root.path_record->twin_list.begin(), root.path_record->twin_list.end(), path)
	  != root.path_record->twin_list.end())) {
    if (parallel_shared)
      parallel_shared->stop_all = true;
    return ProofDisproof::LoopDetection();
  }
  if (parallel_shared && root.record.proof_disproof.isFinal())
    parallel_shared->stop_all = true;
  best_move = root.record.best_move;
  if (root.record.proof_disproof.isCheckmateSuccess())
    proof_pieces = root.record.proofPieces();
  // retrieve pv
  if (pv && root.record.proof_disproof.isCheckmateSuccess()) {
    ProofTreeDepthDfpn analyzer(*table);
    analyzer.retrievePV(state, true, *pv);
  }
  return root.record.proof_disproof;
}

const osl::checkmate::ProofDisproof 
osl::checkmate::
Dfpn::tryProof(const NumEffectState& state, const HashKey& key,
	       const PathEncoding& path, const ProofOracle& oracle, size_t oracle_id, Move& best_move,
	       Move last_move)
{
  return tryProofMain<true>(state, key, path, oracle, oracle_id, best_move, last_move);
}
const osl::checkmate::ProofDisproof 
osl::checkmate::
Dfpn::tryProofLight(const NumEffectState& state, const HashKey& key,
		    const PathEncoding& path, const ProofOracle& oracle, size_t oracle_id, Move& best_move,
		    Move last_move)
{
  return tryProofMain<false>(state, key, path, oracle, oracle_id, best_move, last_move);
}

static const size_t root_proof_simulation_limit = 999999999;// large enough

template <bool UseTable>
const osl::checkmate::ProofDisproof 
osl::checkmate::
Dfpn::tryProofMain(const NumEffectState& state, const HashKey& key,
		   const PathEncoding& path, const ProofOracle& oracle, size_t oracle_id, Move& best_move,
		   Move last_move)
{
  assert(table);
  if (! table)
    return ProofDisproof();
  path_table->clear();
  
  tree->state.copyFrom(state);
  node_count = tree->depth = 0;
  node_count_limit = root_proof_simulation_limit;

  Node& root = tree->node[0];
  root.hash_key = key;
  root.path = path;
  root.clear();
  root.threshold = ProofDisproof(ROOT_PROOF_TOL, ROOT_DISPROOF_TOL);
  root.white_stand = PieceStand(WHITE, state);
  root.moved = last_move;

  root.record = (state.turn() == BLACK) 
    ? table->probe<BLACK>(root.hash_key, root.white_stand)
    : table->probe<WHITE>(root.hash_key, root.white_stand);
  if (root.record.proof_disproof.isFinal() || root.record.tried_oracle > oracle_id) {
    best_move = root.record.best_move;
    return root.record.proof_disproof;
  }

  try {
    if (state.turn() == BLACK)
      proofOracleAttack<BLACK,UseTable>(oracle, ProofSimulationTolerance);
    else
      proofOracleAttack<WHITE,UseTable>(oracle, ProofSimulationTolerance);
  }
  catch (DepthLimitReached&) {
    for (int i=0; i<=tree->depth; ++i)
      table->leaveWorking(tree->node[i].hash_key, thread_id);
    return ProofDisproof();
  }
  if (UseTable && root.path_record
      && (std::find(root.path_record->twin_list.begin(), root.path_record->twin_list.end(), path)
	  != root.path_record->twin_list.end()))
    return ProofDisproof::LoopDetection();
  if (UseTable) {
    root.record.last_move = last_move;
    table->store(root.hash_key, root.record);
  }
  best_move = root.record.best_move;
  root.record.tried_oracle = oracle_id+1;
  return root.record.proof_disproof;
}


const osl::checkmate::ProofDisproof
osl::checkmate::
Dfpn::hasEscapeMove(const NumEffectState& state, 
		    const HashKey& key, const PathEncoding& path, 
		    size_t limit, Move last_move)
{
  assert(table);
  if (! state.hasEffectAt(alt(state.turn()), state.kingSquare(state.turn())))
    return ProofDisproof::NoCheckmate();
  if (! table)
    return ProofDisproof();
  path_table->clear();
  node_count = tree->depth = 0;
  node_count_limit = limit;

  Node& root = tree->node[0];  
  try {
    tree->state.copyFrom(state);
    tree->depth = 0;
    root.hash_key = key;
    root.path = path;
    root.moved = last_move;
    root.clear();
    root.threshold = ProofDisproof(ROOT_PROOF_TOL, ROOT_DISPROOF_TOL);
    root.white_stand = PieceStand(WHITE, state);
    if (state.turn() == BLACK)
      defense<WHITE>();
    else
      defense<BLACK>();

    if (root.record.need_full_width) {
      root.clear();
      if (state.turn() == BLACK)
	defense<WHITE>();
      else
	defense<BLACK>();
    }
  }
  catch (DepthLimitReached&) {
    return ProofDisproof();
  }
  if (root.record.proof_disproof == ProofDisproof::NoEscape()
      && last_move.isNormal() && last_move.isDrop() && last_move.ptype() == PAWN)
    return ProofDisproof::PawnCheckmate();
  if (root.path_record) {
    const SimpleTwinList& tl = root.path_record->twin_list;
    if (std::find(tl.begin(), tl.end(), root.path) != tl.end())
      return ProofDisproof::LoopDetection();
  }
  return root.record.proof_disproof;
}

namespace osl
{
  namespace 
  {
    typedef std::tuple<int,int,int> tuple_t;
    template <Player Turn>
    struct move_compare
    {
      const NumEffectState *state;
      move_compare(const NumEffectState& s) : state(&s)
      {
	assert(Turn == state->turn());
      }
      tuple_t convert(Move m) const
      {
	const int a = state->countEffect(Turn, m.to()) + m.isDrop();
	const int d = state->countEffect(alt(Turn), m.to());
	const int to_y = sign(Turn)*m.to().y();
	const int to_x = (5 - abs(5-m.to().x()))*2 + (m.to().x() > 5);
	int from_to = (to_y*16+to_x)*256;
	if (m.isDrop())
          from_to += m.ptype();
        else
          from_to += m.from().index();
	return std::make_tuple(a > d, from_to, m.isPromotion());
      }
      bool operator()(Move l, Move r) const
      {
	return convert(l) > convert(r);
      }
    };
  }
}

template <osl::Player Turn>
void osl::checkmate::
Dfpn::sort(const NumEffectState& state, DfpnMoveVector& moves)
{
#ifdef MEMORIZE_SOLVED_IN_BITSET
  int last_sorted = 0, cur = 0; 
  Ptype last_ptype = PTYPE_EMPTY;
  for (;(size_t)cur < moves.size(); ++cur) {
    if (moves[cur].isDrop() || moves[cur].oldPtype() == last_ptype)
      continue;
    std::sort(moves.begin()+last_sorted, moves.begin()+cur, move_compare<Turn>(state));
    last_sorted = cur;
    last_ptype = moves[cur].oldPtype();
  }
  std::sort(moves.begin()+last_sorted, moves.begin()+cur, move_compare<Turn>(state));
#endif
}

template <osl::Player P> 
void osl::checkmate::
Dfpn::generateCheck(const NumEffectState& state, DfpnMoveVector& moves, bool &has_pawn_checkmate)
{
  assert(moves.empty());
  if (state.inCheck(P))
  {
    using namespace osl::move_classifier;
    DfpnMoveVector escape;
    move_generator::GenerateEscape<P>::generateKingEscape(state, escape);
    for (Move move: escape) {
      if (MoveAdaptor<Check<P> >::isMember(state, move))
	moves.push_back(move);
    }
  }
  else
  {
    move_action::Store store(moves);
    move_generator::AddEffectWithEffect<move_action::Store>::template generate<P,true>
      (state,state.kingPiece(alt(P)).square(),store,has_pawn_checkmate);
  }
  for (Move move: moves)
  {
    if(move.hasIgnoredUnpromote<P>()){
      if(Ptype_Table.getEffect(unpromote(move.ptypeO()),move.to(),
			       state.kingSquare(alt(P))).hasEffect()
	 || (state.pinOrOpen(alt(P)).test
	     (state.pieceAt(move.from()).number())))
	moves.push_back(move.unpromote());
    }
  }
  sort<P>(state, moves);
}

void osl::checkmate::
Dfpn::findDagSource(const HashKey& terminal_key,
		    DfpnRecord& terminal_record,
		    PieceStand terminal_stand, int offset)
{
#ifdef NAGAI_DAG_TEST
  PieceStand white_stand = terminal_stand;
  HashKey key = terminal_key;
  DfpnRecord cur = terminal_record;
  
  for (int d=offset; d<std::min(tree->MaxDepth,MaxDagTraceDepth); ++d) {
    if (! cur.last_move.isNormal())
      return;
    assert(key.turn() == alt(cur.last_move.player()));
    HashKey parent_key = key.newUnmakeMove(cur.last_move);
    white_stand = white_stand.previousStand(WHITE, cur.last_move);
    DfpnRecord parent = table->probe(parent_key, white_stand);
    // loop invariants
    // { parent, parent_key, white_stand } --(cur.last_move)-> { cur, key }

    // some implementation test (parent.best_move == cur.last_move) here
    // but it seems to be not suitable for gpsshogi
    for (int i=tree->depth - 4 - (d%2); i>=0; i-=2) {
      if (parent_key == tree->node[i].hash_key) {
	for (size_t m=0; m<std::min(tree->node[i].moves.size(), (size_t)64); ++m) {
	  if (tree->node[i].moves[m] == tree->node[i+1].moved
	      || tree->node[i].moves[m] == cur.last_move)
	    tree->node[i].record.dag_moves |= (1ull << m);
	}
	if (parallel_shared)
	  table->addDag(tree->node[i].hash_key, tree->node[i].record);
	terminal_record.dag_terminal = true;
	return;
      }
    }
    key = parent_key;
    cur = parent;
  }
#endif
}

void osl::checkmate::
Dfpn::findDagSource()
{
  findDagSource(tree->node[tree->depth].hash_key, tree->node[tree->depth].record, 
		tree->node[tree->depth].white_stand, 1);
}

// P は攻撃側
template <osl::Player P> 
void osl::checkmate::
Dfpn::attack()
{
  assert(! tree->inCheck(alt(P)));
  Node& node = tree->node[tree->depth];
#if (! defined NDEBUG) && (! defined OSL_USE_RACE_DETECTOR)
  node.visit_time = ++timer;
#endif
#ifdef DFPN_DEBUG
  Tree::Logging logging(tree.get(), table, "attack");
#endif
  const int my_distance = tree->depth ? tree->node[tree->depth-1].path_record->distance+1 : node.path.getDepth();
  LoopToDominance loop;
  DfpnVisitLock<> lk(node.path_record = path_table->allocate<P>(node.hash_key, my_distance, loop));
  DfpnRecord& record = node.record;
  record = DfpnRecord();
  if (loop == BadAttackLoop) {
    node.setLoopDetection();
    return;
  }
  assert(node.white_stand == PieceStand(WHITE, tree->state));
  const size_t node_count_org = node_count++;
#if (! defined CHECKMATE_D2) && (! defined NO_IMMEDIATE_CHECKMATE)
  if (! tree->inCheck(P)
      && ImmediateCheckmate::hasCheckmateMove<P>(tree->state, record.best_move)) {
    PieceStand proof_pieces;	// Note: ImmediateCheckmate が合駒が必要な王手を使わないことに依存
    if (record.best_move.isDrop())
      proof_pieces.add(record.best_move.ptype());
    record.setProofPieces(proof_pieces);
    record.proof_disproof = ProofDisproof::Checkmate();
    return;
  }
#endif
  if (tree->depth + 2 >= tree->MaxDepth) {
    std::cerr << "throw " << thread_id << "\n";
    throw DepthLimitReached();
  }
  assert(tree->depth + 2 < tree->MaxDepth);
  record = table->probe<P>(node.hash_key, node.white_stand);
  assert(record.stands[BLACK] == node.hash_key.blackStand());
  assert(record.stands[WHITE] == node.white_stand);
  if (record.proof_disproof.isFinal())
    return;
  if (tree->depth == 0 && node_count_limit <= 50 && record.node_count >= node_count_limit)
    return;
  if (tree->depth == 0
#ifdef CHECKMATE_A3
      || true
#endif
#ifdef CHECKMATE_A3_GOLD
      || (record.proof_disproof == ProofDisproof(1,1) && tree->state.hasPieceOnStand<GOLD>(P)
	  && (tree->king(alt(P)).square().x() <= 3 || tree->king(alt(P)).square().x() >= 7
	      || tree->king(alt(P)).square().template squareForBlack<P>().y() <= 3))
#endif
    )
  {
#ifdef DFPN_STAT
    static stat::Ratio oracle_success("a3-gold");
#endif
    FixedDepthSolverExt fixed_solver(tree->state);
    PieceStand proof_pieces;
    ProofDisproof pdp = fixed_solver.hasCheckmateMove<P>(2, record.best_move, proof_pieces);
    ++node_count;
#ifdef DFPN_STAT
    oracle_success.add(pdp.isCheckmateSuccess());
#endif
    if (pdp.isCheckmateSuccess()) {
      record.node_count++;
      record.proof_disproof = pdp;
      record.setProofPieces(proof_pieces);
      record.last_move = node.moved;
      table->store(node.hash_key, record);
      return;
    }
  }
#ifndef MINIMAL
  if (tree->MaxDepth > EnableGCDepth && thread_id <= 0) {
    try {
      const size_t removed = table->runGC();
      if (removed > 0) {
#ifdef DFPN_DEBUG
	for (int i=1; i<tree->depth; ++i)
	  std::cerr << tree->node[i].threshold.proof() << ' '
		    << record::csa::show(tree->node[i].moved) << ' ';
	std::cerr << "\n";
#endif
      }
    }
    catch (...) { //  fail
      if (parallel_shared)
	parallel_shared->stop_all = true;
      throw;
    }
  }
  if (tree->MaxDepth > EnableGCDepth
      && (path_table->size() > table->growthLimit()
#ifdef OSL_DFPN_SMP
	  || (parallel_shared
	      && path_table->size() > table->growthLimit()/4)
#endif
	)) {
    const size_t before = path_table->size();
    const size_t removed = path_table->runGC();
    if (removed > 0) {
      if (thread_id <= 0)
	std::cerr << " GC-path collected "
		  << std::setprecision(3)
		  << ((sizeof(HashKey)+sizeof(DfpnPathRecord)+sizeof(char*)*2)
		      * removed / (1<<20)) << "MB "
		  << 100.0*removed/before << "%\n";
      for (int i=0; i<tree->depth; ++i) {
	for (size_t j=0; j<tree->node[i].moves.size(); ++j) {
	  tree->node[i].children_path[j] = 0;
	}
      }
    }
  }
#endif
  if (parallel_shared) {
    if (parallel_shared->stop_all) {
      // std::cerr << "throw " << thread_id << "\n";
      throw DepthLimitReached();
    }
    if (parallel_shared->data[thread_id].restart) {
      for (int i=0; i<tree->depth; ++i) {
	if (tree->node[i].hash_key
	    == parallel_shared->data[thread_id].restart_key)
	  return;
#if 0
	if (tree->node[i].record.dag_terminal)
	  break;		// ignore
#endif
      }
      // false alert
      parallel_shared->data[thread_id].clear();
    }
  }

  // move generation
  bool has_pawn_checkmate=false;
  generateCheck<P>(tree->state, node.moves,has_pawn_checkmate);
  if (node.moves.empty()) {
    record.setDisproofPieces(DisproofPieces::leaf(tree->state, alt(P), 
						  record.stands[alt(P)]));
    if(has_pawn_checkmate)
      record.proof_disproof = ProofDisproof::PawnCheckmate();
    else
      record.proof_disproof = ProofDisproof::NoCheckmate();
    return;
  }
  // probe all
#ifdef PROOF_AVERAGE
  int frontier_count = 0, sum_frontier_proof = 0;
#endif
  assert(node.children.empty());
  {
    node.allocate(node.moves.size());
    const King8Info info_modified 
      = Edge_Table.resetEdgeFromLiberty(alt(P), tree->king(alt(P)).square(), King8Info(tree->state.Iking8Info(alt(P))));
    for (size_t i=0; i<node.moves.size(); ++i) {
#ifdef MEMORIZE_SOLVED_IN_BITSET
      if (record.solved & (1ull << i))
	continue;
#endif
      const HashKey& new_key = node.hashes[i] = node.hash_key.newHashWithMove(node.moves[i]);
      node.children[i] = table->probe<P>(new_key, node.nextWhiteStand(P, node.moves[i]));
      if (node.children[i].proof_disproof == ProofDisproof(1,1)) {
	unsigned int proof, disproof;
	LibertyEstimator::attackH(P, tree->state, info_modified, 
				  node.moves[i], proof, disproof);
#ifndef MINIMAL
	if (HashRandomPair::initialized()) {
	  // randomness presented by Hoki2011 (zero by default)
	  std::pair<char,char> randomness = HashRandomPair::value(new_key);
	  proof    += randomness.first;
	  disproof += randomness.second;
	}
#endif
	node.children[i].proof_disproof = ProofDisproof(proof, disproof);
      }
      if (node.children[i].proof_disproof == ProofDisproof::NoEscape()
	  && node.moves[i].isDrop() && node.moves[i].ptype() == PAWN) {
	node.children[i].proof_disproof = ProofDisproof::PawnCheckmate();
#ifdef MEMORIZE_SOLVED_IN_BITSET
	record.solved |= (1ull << i);
#endif
	record.min_pdp = std::min(record.min_pdp, (unsigned int)ProofDisproof::PAWN_CHECK_MATE_PROOF);
      }
      else if (node.children[i].proof_disproof.isCheckmateFail())
	tree->setNoCheckmateChildInAttack(i);
      else if (node.children[i].proof_disproof.isCheckmateSuccess()) {
	record.node_count += node_count - node_count_org;
	node.setCheckmateAttack(P,i);
	record.last_move = node.moved;
	table->store(node.hash_key, record);
	node.path_record->node_count = 0;
	return;
      }
#ifdef PROOF_AVERAGE
      else if (node.children[i].node_count == 0) {
	++frontier_count;
	sum_frontier_proof += node.children[i].proof();
	assert(node.children[i].proof() < 128);
      }
#endif
#ifdef AGGRESSIVE_FIND_DAG2
      else if (!node.children[i].proof_disproof.isFinal()
	       && std::max(node.children[i].proof(), node.children[i].disproof()) >= DagFindThreshold2
	       && node.children[i].last_move.isNormal()
	       && node.children[i].last_move != node.moves[i]) {
	findDagSource(node.hashes[i], node.children[i],
		      node.nextWhiteStand(P, node.moves[i]));
      }
#endif
      node.children_path[i] = path_table->probe(new_key);
      node.proof_cost[i] = attackProofCost(P, tree->state, node.moves[i]);
    }
  }

  // hereafter, call leaveWorking before returning
  if (parallel_shared)
    table->setWorking(node.hash_key, record, thread_id);

  const Move recorded_last_move = record.last_move;
  record.last_move = node.moved;

  assert(node.children.size() == node.moves.size());
#ifdef PROOF_AVERAGE
  const size_t proof_average = frontier_count ? sum_frontier_proof/frontier_count : 1;
#else
  const size_t proof_average = 1;
#endif
  // main loop
#ifdef DFPN_DEBUG
  if (std::find(debug_node.begin(), debug_node.end(), node_id_table.id(node.hash_key))
      != debug_node.end() && timer > debug_time_start)
    tree->dump(__LINE__);
#endif
  for (int loop=0; true; ++loop) {
    unsigned int min_proof=record.min_pdp, min_proof2=record.min_pdp;
    size_t sum_disproof = 0, max_disproof = 0, max_disproof_dag = 0, next_i=node.children.size();
    size_t max_drop_disproof_rook = 0, max_drop_disproof_bishop = 0, max_drop_disproof_lance = 0;
    int max_children_depth = 0, upward_count = 0;
    for (size_t i=0; i<node.children.size(); ++i) {
#ifdef MEMORIZE_SOLVED_IN_BITSET
      if (record.solved & (1ull << i))
	continue;
#endif
      if (i > 0 && min_proof < ProofDisproof::PROOF_LIMIT
	  && node.moves[i].fromTo() == node.moves[i-1].fromTo()
	  && ! node.moves[i].isDrop()) {
	// ignore a no-promote move until it becomes the last one, if there is the corresponding promote move  
	assert(node.moves[i].ptype() == node.moves[i-1].oldPtype());
	record.dag_moves |= ((1ull << i) | (1ull << (i-1)));
	if (node.threshold.proof() < NoPromoeIgnoreProofThreshold
	    && node.threshold.disproof() < NoPromoeIgnoreDisproofThreshold)
	  continue;
	// fall through
      }
      size_t proof = node.children[i].proof();
      size_t disproof = node.children[i].disproof();
      if (proof && disproof) {
	proof += node.proof_cost[i];
#ifdef OSL_DFPN_SMP
	if (parallel_shared && node.children[i].working_threads) {
	  // proof += misc::BitOp::countBit(node.children[i].working_threads)/2+1;
	  proof += misc::BitOp::countBit(node.children[i].working_threads);
	}
#endif
      }
      if (node.children_path[i]) {	
	if (node.isLoop(i)) {
	  node.children[i].proof_disproof = ProofDisproof::LoopDetection();
	  assert(proof < ProofDisproof::LOOP_DETECTION_PROOF);
	  proof = ProofDisproof::LOOP_DETECTION_PROOF;
	  disproof = 0;
	} 
	else if (! node.children[i].proof_disproof.isFinal()) {
	  max_children_depth = std::max(max_children_depth, node.children_path[i]->distance);
#ifdef NAGAI_DAG_TEST
	  if (record.dag_moves & (1ull<<i)) {
	    max_disproof_dag = std::max(max_disproof_dag, disproof);
	    disproof = 0;
	  }
	  else
#endif
#ifdef DELAY_UPWARD
	    if (node.children_path[i]->distance <= node.path_record->distance) {
	      max_disproof = std::max(max_disproof, disproof);
	      ++upward_count;
	      disproof = UpwardWeight;
	    }
	    else
#endif
	    if (node.moves[i].isDrop()
		|| (isMajor(node.moves[i].ptype())
		    && ! node.moves[i].isCapture()
		    && ! node.moves[i].isPromotion() && isPromoted(node.moves[i].ptype())
		    && ! tree->state.hasEffectAt(alt(P), node.moves[i].to()))) {
	      const EffectContent e
		= Ptype_Table.getEffect(node.moves[i].ptypeO(),
					Offset32(tree->king(alt(P)).square(), node.moves[i].to()));
	      if (! e.hasUnblockableEffect()) {
		size_t *target = &max_drop_disproof_lance;
		if (unpromote(node.moves[i].ptype()) == ROOK)
		  target = &max_drop_disproof_rook;
		else if (unpromote(node.moves[i].ptype()) == BISHOP)
		  target = &max_drop_disproof_bishop;
		*target = std::max(*target, disproof);
		disproof = LongDropCount;
	      }
	    }
	} // ! isFinal
      }
      else {
	max_children_depth = node.path_record->distance+1;
      }
      if (proof < min_proof || (proof == min_proof && disproof && disproof < node.children[next_i].disproof())) {
	min_proof2 = min_proof;
	min_proof = proof;
	next_i = i;
      } else if (proof < min_proof2) {
	min_proof2 = proof;
      }
      sum_disproof += disproof;
    }
    sum_disproof += max_drop_disproof_rook + max_drop_disproof_bishop + max_drop_disproof_lance
      + max_disproof_dag;
    if (LongDropCount) {
      if (max_drop_disproof_rook)   sum_disproof -= LongDropCount;
      if (max_drop_disproof_bishop) sum_disproof -= LongDropCount;
      if (max_drop_disproof_lance)  sum_disproof -= LongDropCount;
    }
    if (upward_count) {
      if (sum_disproof == 0)
	sum_disproof = max_disproof;
    }
    if (node.path_record->distance >= max_children_depth) {
      node.path_record->distance = max_children_depth-1;
    }
#ifdef KISHIMOTO_WIDEN_THRESHOLD
    if (loop == 0 && sum_disproof >= node.threshold.disproof() && sum_disproof > IgnoreUpwardDisproofThreshold)
      node.threshold = ProofDisproof(node.threshold.proof(), sum_disproof+1);
#endif
#ifdef ADHOC_SUM_RESTRICTION
    if (sum_disproof < ROOT_DISPROOF_TOL && min_proof > 0 && sum_disproof > min_proof*AdHocSumScale) {
      sum_disproof = min_proof*AdHocSumScale
	+ slow_increase(sum_disproof-min_proof*AdHocSumScale);
    }
#endif
    if (min_proof >= node.threshold.proof()
	|| sum_disproof >= node.threshold.disproof()
	|| next_i >= node.children.size()
	|| node_count + min_proof >= node_count_limit) {
      record.proof_disproof = ProofDisproof(min_proof, sum_disproof);
      if (record.proof_disproof.isLoopDetection())
	node.setLoopDetection();
      else if (record.proof_disproof.isCheckmateFail()) {
	node.setNoCheckmateAttack(P, tree->state);
      } else if (! record.proof_disproof.isFinal()) {
	if (recorded_last_move.isNormal() && recorded_last_move != node.moved
	    && std::max(record.proof(), record.disproof()) >= DagFindThreshold)
	  findDagSource();
#ifdef AGGRESSIVE_FIND_DAG
	if (std::max(node.children[next_i].proof(), node.children[next_i].disproof()) >= DagFindThreshold
	    && node.children[next_i].last_move.isNormal()
	    && node.children[next_i].last_move != node.moves[next_i]) {
	  findDagSource(node.hashes[next_i], node.children[next_i],
			node.nextWhiteStand(P, node.moves[next_i]));
	  node.children[next_i].last_move = node.moves[next_i];
	  table->store(node.hashes[next_i], node.children[next_i]);
	}
#endif
      }
      record.node_count += node_count - node_count_org;
      table->store(node.hash_key, record, thread_id);
      node.path_record->node_count = record.node_count;
      if (parallel_shared && record.proof_disproof.isFinal())
	parallel_shared->restartThreads(node.hash_key, tree->depth, record.working_threads);
      return;
    }
#ifdef MEMORIZE_SOLVED_IN_BITSET
    assert(! (record.solved & (1ull << next_i)));
#endif
    record.best_move = node.moves[next_i];
    tree->newVisit(P, node.moves[next_i], node.hashes[next_i]);
    Node& next = tree->node[tree->depth+1];
    unsigned int disproof_c = node.threshold.disproof()
      - (sum_disproof - node.children[next_i].disproof());
#ifdef ADHOC_SUM_RESTRICTION
    if (disproof_c > node.threshold.disproof())
      disproof_c = node.children[next_i].disproof()
	+ (node.threshold.disproof() - sum_disproof);
#endif    
    next.threshold = ProofDisproof(std::min(min_proof2+proof_average, (size_t)node.threshold.proof())
				   - node.proof_cost[next_i], 
				   disproof_c);
    CallDefense<P> helper(this);
    tree->depth += 1;
    next.path.pushMove(next.moved);
    tree->state.makeUnmakeMove(Player2Type<P>(), next.moved, helper);
    tree->depth -= 1;
    node.children[next_i] = next.record;
    node.children_path[next_i] = next.path_record;
    if (next.record.proof_disproof == ProofDisproof::NoEscape()
	&& next.moved.isDrop() && next.moved.ptype() == PAWN)
      node.children[next_i].proof_disproof = ProofDisproof::PawnCheckmate();
    if (node.children[next_i].proof_disproof.isCheckmateSuccess()) {
      node.setCheckmateAttack(P,next_i);
      record.node_count += node_count - node_count_org;
      record.last_move = node.moved;
      table->store(node.hash_key, record, thread_id);
      node.path_record->node_count = 0;
      if (parallel_shared)
	parallel_shared->restartThreads(node.hash_key, tree->depth, record.working_threads);
      return;
    }      
    else if (next.record.proof_disproof.isCheckmateFail()
	     && ! next.record.proof_disproof.isLoopDetection())
      tree->setNoCheckmateChildInAttack(next_i);
    min_proof = std::min(min_proof2, node.children[next_i].proof());
    if (min_proof < ProofDisproof::PROOF_LIMIT
	&& node_count + min_proof >= node_count_limit) {
      record.proof_disproof = ProofDisproof(min_proof, sum_disproof);
      record.node_count += node_count - node_count_org;
      table->store(node.hash_key, record, thread_id);
      node.path_record->node_count = record.node_count;
      if (parallel_shared)
	parallel_shared->restartThreads(node.hash_key, tree->depth, record.working_threads);
      return;
    }
    if (parallel_shared && parallel_shared->data[thread_id].restart) {
      if (tree->depth == 0)
	parallel_shared->data[thread_id].clear();
      else {
	if (parallel_shared->data[thread_id].restart_key == node.hash_key) {
	  record = table->probe<P>(node.hash_key, node.white_stand);
	  if (! record.proof_disproof.isFinal()) 
	    continue;
	  parallel_shared->data[thread_id].clear();
	}
	table->leaveWorking(node.hash_key, thread_id);
	return;
      }
    } 
  }
}

template <osl::Player P> 
void osl::checkmate::
Dfpn::generateEscape(const NumEffectState& state, bool need_full_width, 
		     Square last_to, DfpnMoveVector& moves)
{
  assert(moves.empty());
  const Player AltP=alt(P);
#ifdef GRAND_PARENT_DELAY
  const bool delay_node = last_to != Square() 
      && state.hasEffectAt(alt(P), last_to)
      && (state.hasEffectNotBy(alt(P), state.kingPiece(alt(P)), last_to)
	  || ! state.hasEffectAt(P, last_to));
  if (delay_node)
  {
    DfpnMoveVector all;
    move_generator::GenerateEscape<AltP>::
	generateCheapKingEscape(state, all);

    for (Move move: all) {
      if (move.to() == last_to) {
	moves.push_back(move);
      }
    }
#ifdef MEMORIZE_SOLVED_IN_BITSET
    sort<AltP>(state, moves);
#endif
  }
  else 
#endif
  {
    move_generator::GenerateEscape<AltP>::
	generateCheapKingEscape(state, moves);
#ifdef MEMORIZE_SOLVED_IN_BITSET
    sort<AltP>(state, moves);
#endif
  }

  if (need_full_width) {
    DfpnMoveVector others;
    move_generator::GenerateEscape<AltP>::
      generateKingEscape(state, others);
#ifdef MEMORIZE_SOLVED_IN_BITSET
    sort<AltP>(state, others);
#endif
    const int org_size = moves.size();
    for (Move move: others) {
      if (std::find(moves.begin(), moves.begin()+org_size, move) == moves.begin()+org_size)
	moves.push_back(move);
    }
    for (Move move: moves)
    {
      if(move.hasIgnoredUnpromote<AltP>())
	moves.push_back(move.unpromote());
    }
  }
  // TODO: 受け方の打歩詰め王手
}

bool osl::checkmate::
Dfpn::grandParentSimulationSuitable() const
{
#ifdef GRAND_PARENT_SIMULATION
  Node& node = tree->node[tree->depth];
  if (tree->depth >= 2) {
    const Node& parent = tree->node[tree->depth-1];
    const Node& gparent = tree->node[tree->depth-2];
    const Move alm = node.moved; // attacker's last move
    const Move dlm = parent.moved; // defense's last move
    const Move alm2 = gparent.moved; // attacker's second-last move
    if (dlm.isNormal() && alm.to() == dlm.to() && ! dlm.isCapture()
	&& alm2.isNormal() && alm2.to() == alm.from()) {
      return true;
    }
  }
#endif
  return false;
}

template <osl::Player P> 
void osl::checkmate::
Dfpn::defense()
{
#if 0
  if (parallel_shared) {
    if (parallel_shared->stop_all)
      throw DepthLimitReached();
    if (parallel_shared->data[thread_id].restart) {
      for (int i=0; i<tree->depth; ++i) {
	if (tree->node[i].hash_key == parallel_shared->data[thread_id].restart_key)
	  return;
#if 0
	if (tree->node[i].record.dag_terminal)
	  break;
#endif
      }
      // false alert
      parallel_shared->data[thread_id].clear();
    }
  }
#endif
  Node& node = tree->node[tree->depth];
#if (! defined NDEBUG) && (! defined OSL_USE_RACE_DETECTOR)
  node.visit_time = ++timer;
#endif
#ifdef DFPN_DEBUG
  Tree::Logging logging(tree.get(), table, "defens");
#endif
  const int my_distance = tree->depth ? tree->node[tree->depth-1].path_record->distance+1 : node.path.getDepth();
  LoopToDominance loop;
  DfpnVisitLock<> lk(node.path_record = path_table->allocate<P>(node.hash_key, my_distance, loop));
  DfpnRecord& record = node.record;
  if (loop == BadAttackLoop) {
    record = DfpnRecord();
    node.setLoopDetection();
    return;
  }
  const size_t node_count_org = node_count++;
  assert(tree->inCheck(alt(P)));
  assert(node.white_stand == PieceStand(WHITE, tree->state));

  record = table->probe<P>(node.hash_key, node.white_stand);
  assert(record.stands[BLACK] == node.hash_key.blackStand());
  assert(record.stands[WHITE] == node.white_stand);
  if (record.proof_disproof.isFinal())
    return;
  const bool grand_parent_simulation = grandParentSimulationSuitable();
  if (record.last_to == Square())
    record.last_to = grand_parent_simulation ? node.moved.to() : tree->king(alt(P)).square();
  const Square grand_parent_delay_last_to
    = (record.last_to != tree->king(alt(P)).square()) ? record.last_to : Square();

  generateEscape<P>(tree->state, record.need_full_width, grand_parent_delay_last_to, node.moves);
  if (node.moves.empty() && ! record.need_full_width) {
    record.need_full_width = true;
    generateEscape<P>(tree->state, record.need_full_width, grand_parent_delay_last_to, node.moves);
  }
  if (node.moves.empty()) {
    record.setProofPieces(ProofPieces::leaf(tree->state, P, record.stands[P]));
    record.proof_disproof = ProofDisproof::NoEscape();
    return;
  }
  // probe all
#ifdef DISPROOF_AVERAGE
  int frontier_count = 0, sum_frontier_disproof = 0;
#endif
  assert(node.children.empty());
  {
    node.allocate(node.moves.size());
    for (size_t i=0;i <node.moves.size(); ++i) {
#ifdef MEMORIZE_SOLVED_IN_BITSET
      if (record.solved & (1ull << i))
	continue;
#endif
      const HashKey& new_key = node.hashes[i] = node.hash_key.newHashWithMove(node.moves[i]);
      node.children[i] = table->probe<P>(new_key, node.nextWhiteStand(alt(P), node.moves[i]));
      if (node.children[i].proof_disproof.isCheckmateSuccess()) {
	node.setCheckmateChildInDefense(i);	
      }
#ifdef CHECKMATE_D2
      else if (node.children[i].proof_disproof == ProofDisproof(1,1)) {
	FixedDepthSolverExt fixed_solver(tree->state);
	PieceStand proof_pieces;
	Move check_move;
	node.children[i].proof_disproof
	  = fixed_solver.hasEscapeByMove<P>(node.moves[i], 0, check_move, proof_pieces);
	++node_count;
	if (node.children[i].proof_disproof.isCheckmateSuccess()) {	  
	  node.children[i].best_move = check_move;
	  node.children[i].setProofPieces(proof_pieces);
	  node.children[i].node_count++;
	  node.setCheckmateChildInDefense(i);
	}
	else {
	  if (node.children[i].proof_disproof.isCheckmateFail()) {
	    node.children[i].proof_disproof = ProofDisproof(1,1);
	    if (i) {
	      node.moves[0] = node.moves[i];
	      node.children[0] = node.children[i];
	      node.children_path[0] = node.children_path[i];
	      const int old_size = (int)node.moves.size();
	      for (int j=1; j<old_size; ++j) {
		node.moves.pop_back();
		node.children.pop_back();
		node.children_path.pop_back();
	      }
	    }
	    break;
	  }
	  else {
#ifndef MINIMAL
	    if (HashRandomPair::initialized()) {
	      // randomness presented by Hoki2011 (zero by default)
	      std::pair<char,char> randomness = HashRandomPair::value(new_key);
	      if (randomness.first || randomness.second) {
		unsigned int proof    = node.children[i].proof();
		unsigned int disproof = node.children[i].disproof();
		proof    += randomness.first;
		disproof += randomness.second;
		node.children[i].proof_disproof = ProofDisproof( proof, disproof );
	      }
	    }
#endif
	  }	  
#ifdef DISPROOF_AVERAGE
	  ++frontier_count;
	  sum_frontier_disproof += node.children[i].proof_disproof.disproof();
#endif
	}
	// ++node_count;
      }
#endif
      if (! node.children[i].proof_disproof.isCheckmateFail()) {
	node.children_path[i] = path_table->probe(new_key);
	if (node.isLoop(i)) {
	  node.setLoopDetection();
	  return;
	}
#ifdef GRAND_PARENT_SIMULATION
	if (grand_parent_simulation && node.children[i].proof_disproof == ProofDisproof(1,1)) {
	  const Node& gparent = tree->node[tree->depth-2];
	  size_t gi=std::find(gparent.moves.begin(), gparent.moves.end(), node.moves[i]) - gparent.moves.begin();
	  if (gi < gparent.moves.size() 
	      && (
#ifdef MEMORIZE_SOLVED_IN_BITSET
		(gparent.record.solved & (1ull<<gi))
		|| 
#endif
		gparent.children[gi].proof_disproof.isCheckmateSuccess())) {
	    grandParentSimulation<P>(i, gparent, gi);
	    if (node.children[i].proof_disproof.isCheckmateSuccess())
	      node.setCheckmateChildInDefense(i);
	  }
	}
#endif
      }
      if (node.children[i].proof_disproof.isCheckmateFail()) {
	tree->setNoCheckmateDefense(P, i);
	table->store(node.hash_key, record);
	return;
      }
#ifdef AGGRESSIVE_FIND_DAG2
      if (!node.children[i].proof_disproof.isFinal()
	  && std::max(node.children[i].proof(),node.children[i].disproof()) >= DagFindThreshold2
	  && node.children[i].last_move.isNormal()
	  && node.children[i].last_move != node.moves[i]) {
	findDagSource(node.hashes[i], node.children[i],
		      node.nextWhiteStand(alt(P), node.moves[i]));
      }
#endif
    }
    if (record.need_full_width==1) {
      record.need_full_width++;
      for (size_t i=0;i <node.moves.size(); ++i) {
	if (
#ifdef MEMORIZE_SOLVED_IN_BITSET
	  ((record.solved & (1ull<<i))
	   || (i >= 64 && node.children[i].proof_disproof.isCheckmateSuccess()))
#else
	  node.children[i].proof_disproof.isCheckmateSuccess()
#endif
	    
	    && node.moves[i].isDrop()) {
	  blockingSimulation<P>(i, ProofOracle(node.hash_key.newHashWithMove(node.moves[i]), 
					       node.nextWhiteStand(alt(P), node.moves[i])));
	}
      }
    }
  }
  assert(node.children.size() == node.moves.size());

  // hereafter, call leaveWorking before return
  if (parallel_shared)
    table->setWorking(node.hash_key, record, thread_id);

  // for dag analyses
  const Move recorded_last_move = node.moved;
  record.last_move = node.moved;

#ifdef DISPROOF_AVERAGE
  const size_t disproof_average = frontier_count ? sum_frontier_disproof/frontier_count : 1;
#else
  const size_t disproof_average = 1;
#endif
  // main loop
#ifdef DFPN_DEBUG
  if (std::find(debug_node.begin(), debug_node.end(), node_id_table.id(node.hash_key))
      != debug_node.end() && timer > debug_time_start)
    tree->dump(__LINE__);
#endif
  CArray<char,DfpnMaxUniqMoves> target;
  for (int loop=0; true; ++loop) {
    std::fill(target.begin(), target.begin()+(int)node.moves.size(), false);
    unsigned int min_disproof=record.min_pdp, min_disproof2=record.min_pdp;
    size_t sum_proof = 0, max_upward_proof = 0, max_drop_proof = 0, next_i=node.children.size();
    size_t max_proof_dag = 0;
    int max_children_depth = 0, upward_count = 0;
#ifdef KAKINOKI_FALSE_BRANCH_SEARCH
    size_t max_proof = 0;
    bool false_branch_candidate = !record.false_branch;
#endif
    for (size_t i=0; i<node.children.size(); ++i) {
#ifdef MEMORIZE_SOLVED_IN_BITSET
      if (record.solved & (1ull << i))
	continue;
#endif
      if (i > 0 && min_disproof < ProofDisproof::DISPROOF_LIMIT
	  && node.moves[i].fromTo() == node.moves[i-1].fromTo()
	  && ! node.moves[i].isDrop()) {
	// ignore a no-promote move until it becomes the last one, if there is the corresponding promote move  
	assert(node.moves[i].ptype() == node.moves[i-1].oldPtype());
	continue;
      }
      size_t disproof = node.children[i].disproof();
      size_t proof = node.children[i].proof();
      if (node.children[i].proof_disproof.isCheckmateFail()) {
	// simulation で表を読んだら更新されていた等
	assert(! node.children[i].proof_disproof.isLoopDetection());
	tree->setNoCheckmateDefense(P, i);
	table->store(node.hash_key, record, thread_id);
	if (parallel_shared)
	  parallel_shared->restartThreads(node.hash_key, tree->depth, record.working_threads);
	return;
      }      
#ifdef OSL_DFPN_SMP
      if (proof && disproof) {
	if (parallel_shared && node.children[i].working_threads) {
	  // disproof += misc::BitOp::countBit(node.children[i].working_threads)/2+1;
	  disproof += misc::BitOp::countBit(node.children[i].working_threads);
	}
      }
#endif
      if (node.children_path[i]) {
	if (node.isLoop(i)) {
	  node.setLoopDetection();
	  if (parallel_shared)
	    table->leaveWorking(node.hash_key, thread_id);
	  return;
	}
	if (! node.children[i].proof_disproof.isFinal()) {
	  max_children_depth = std::max(max_children_depth, node.children_path[i]->distance);
#ifdef IGNORE_MONSTER_CHILD
	    if (node.children_path[i]->distance <= node.path_record->distance
		&& (! record.need_full_width || min_disproof < ProofDisproof::DISPROOF_LIMIT) // todo: this condition is not accurate 
		&& node.children[i].proof_disproof.proof() >= node.threshold.proof()
		&& node.threshold.proof() > IgnoreUpwardProofThreshold) {
	      false_branch_candidate = false;
	      continue;		// ignore upward move with too much pdp, untill it becomes the last one
	    }
	    else
#endif
#ifdef NAGAI_DAG_TEST
	  if (record.dag_moves & (1ull << i)) {
	    max_proof_dag = std::max(max_proof_dag, proof);
	    proof = 0;
	  }
	  else
#endif
#ifdef DELAY_UPWARD
	    if (node.children_path[i]->distance <= node.path_record->distance) {
	      max_upward_proof = std::max(max_upward_proof , proof);
	      ++upward_count;
	      proof = UpwardWeight;
	    }
	    else
#endif
	    if (node.moves[i].isDrop() && !tree->state.hasEffectAt(alt(P), node.moves[i].to())) {
	      max_drop_proof = std::max(max_drop_proof, proof);
	      proof = SacrificeBlockCount;
	    }
	}
      }
      else {
	max_children_depth = node.path_record->distance+1;
      }
      target[i] = true;
      if (disproof < min_disproof
	  || (disproof == min_disproof && proof && proof < node.children[next_i].proof())) {
	min_disproof2 = min_disproof;
	min_disproof = disproof;
	next_i = i;
      } else if (disproof < min_disproof2) {
	min_disproof2 = disproof;
      }
#ifdef KAKINOKI_FALSE_BRANCH_SEARCH
      if (false_branch_candidate && ! node.children[i].proof_disproof.isFinal()
	  && (node.children[i].node_count == 0
	      || ! node.children[i].best_move.isNormal()
	      || ! (node.moves[i].ptype() == KING && ! node.moves[i].isCapture())))
	false_branch_candidate = false;
      max_proof = std::max(max_proof, proof);
#endif
      sum_proof += proof;
    }
#ifdef KAKINOKI_FALSE_BRANCH_SEARCH
    if (false_branch_candidate) {
      record.false_branch = true;
      HashKey goal;
      for (size_t i=0; i<node.children.size(); ++i) {
	if (! target[i])
	  continue;
	HashKey key = node.hashes[i];
	key = key.newHashWithMove(node.children[i].best_move);
	if (goal == HashKey()) {
	  goal = key;
	  continue;
	}
	if (goal != key) {
	  record.false_branch = false;
	  break;
	}
      }
    }
    if (record.false_branch)
      sum_proof = max_proof;
#endif
    sum_proof += max_drop_proof + max_proof_dag;
    if (SacrificeBlockCount && max_drop_proof) 
      sum_proof -= SacrificeBlockCount;
    if (upward_count) {
      if (sum_proof == 0)
	sum_proof = std::max(sum_proof, max_upward_proof);
    }
    if (node.path_record->distance >= max_children_depth) {
      node.path_record->distance = max_children_depth-1;
    }
    if (min_disproof >= ProofDisproof::DISPROOF_MAX) {
      assert(! record.need_full_width);
      record.proof_disproof = ProofDisproof(1,1);
      record.need_full_width = 1;
      table->store(node.hash_key, record, thread_id);
      return;
    }
#ifdef KISHIMOTO_WIDEN_THRESHOLD
    if (loop == 0 && sum_proof >= node.threshold.proof() && sum_proof > IgnoreUpwardProofThreshold)
      node.threshold = ProofDisproof(sum_proof+1, node.threshold.disproof());
#endif
#ifdef ADHOC_SUM_RESTRICTION
    if (sum_proof < ROOT_PROOF_TOL && min_disproof > 0 && sum_proof > min_disproof*AdHocSumScale) {
      sum_proof = min_disproof*AdHocSumScale
	+ slow_increase(sum_proof-min_disproof*AdHocSumScale);
    }
#endif
    if (min_disproof >= node.threshold.disproof()
	|| sum_proof >= node.threshold.proof()
	|| next_i >= node.children.size()
	|| node_count + sum_proof >= node_count_limit) {
      record.proof_disproof = ProofDisproof(sum_proof, min_disproof);
      if (record.proof_disproof.isLoopDetection())
	node.setLoopDetection();
      else if (record.proof_disproof.isCheckmateSuccess()) {
	if (blocking_verify && ! record.need_full_width) {
	  // read again with full move generation
	  record.need_full_width = 1;
	  record.proof_disproof = ProofDisproof(1,1);
	  table->store(node.hash_key, record, thread_id);
	  return;
	}
	node.setCheckmateDefense(P, tree->state);
      } else if (! record.proof_disproof.isFinal()) {
	if (recorded_last_move.isNormal() && recorded_last_move != node.moved
	    && std::max(record.proof(), record.disproof()) >= DagFindThreshold)
	  findDagSource();
#ifdef AGGRESSIVE_FIND_DAG
	if (std::max(node.children[next_i].proof(), node.children[next_i].disproof()) >= DagFindThreshold
	    && node.children[next_i].last_move.isNormal()
	    && node.children[next_i].last_move != node.moves[next_i]) {
	  findDagSource(node.hashes[next_i], node.children[next_i],
			node.nextWhiteStand(alt(P), node.moves[next_i]));
	  node.children[next_i].last_move = node.moves[next_i];
	  table->store(node.hashes[next_i], node.children[next_i]);
	}
#endif
      }
      record.node_count += node_count - node_count_org;
      table->store(node.hash_key, record, thread_id);
      node.path_record->node_count = record.node_count;
      if (parallel_shared && record.proof_disproof.isFinal())
	parallel_shared->restartThreads(node.hash_key, tree->depth, record.working_threads);
      return;
    }
#ifdef MEMORIZE_SOLVED_IN_BITSET
    assert(! (record.solved & (1ull << next_i)));
#endif
    record.best_move = node.moves[next_i];
    tree->newVisit(alt(P), node.moves[next_i], node.hashes[next_i]);
    Node& next = tree->node[tree->depth+1];
    unsigned int proof_c = node.threshold.proof()
      - (sum_proof - node.children[next_i].proof());
#ifdef ADHOC_SUM_RESTRICTION
    if (proof_c > node.threshold.proof())
      proof_c = node.children[next_i].proof()
	+ (node.threshold.proof() - sum_proof);
#endif
    next.threshold = ProofDisproof(proof_c,
				   std::min(min_disproof2+disproof_average,
					    (size_t)node.threshold.disproof()));
    CallAttack<P> helper(this);
    tree->depth += 1;
    next.path.pushMove(node.moves[next_i]);
    tree->state.makeUnmakeMove(Player2Type<alt(P)>(), node.moves[next_i], helper);
    tree->depth -= 1;
    if (parallel_shared && parallel_shared->data[thread_id].restart) {
      if (tree->depth == 0)
	parallel_shared->data[thread_id].clear();
      else {
	if (parallel_shared->data[thread_id].restart_key == node.hash_key) {
	  record = table->probe<P>(node.hash_key, node.white_stand);
	  assert(record.proof_disproof.isFinal());
	  parallel_shared->data[thread_id].clear();
	}
	table->leaveWorking(node.hash_key, thread_id);
	return;
      }
    } 

    node.children[next_i] = next.record;
    node.children_path[next_i] = next.path_record;
    if (next.record.proof_disproof.isCheckmateFail()) {
      if (record.proof_disproof.isLoopDetection())
	node.setLoopDetection();
      else
	tree->setNoCheckmateDefense(P, next_i);
      record.node_count += node_count - node_count_org;
      table->store(node.hash_key, record, thread_id);
      node.path_record->node_count = record.node_count;
      if (parallel_shared && record.proof_disproof.isFinal())
	parallel_shared->restartThreads(node.hash_key, tree->depth, record.working_threads);
      return;
    }      
    if (next.record.proof_disproof.isCheckmateSuccess())
      node.setCheckmateChildInDefense(next_i);
    if (node_count >= node_count_limit) {
      record.proof_disproof = ProofDisproof(sum_proof, min_disproof);
      record.node_count += node_count - node_count_org;
      table->store(node.hash_key, record, thread_id);
      node.path_record->node_count = record.node_count;
      if (parallel_shared && record.proof_disproof.isFinal())
	parallel_shared->restartThreads(node.hash_key, tree->depth, record.working_threads);
      return;
    }
    if (next.moved.isDrop() && next.record.proof_disproof.isCheckmateSuccess()) {
      blockingSimulation<P>(next_i, ProofOracle(next.hash_key, next.white_stand));
    }
  }
}

#if (!defined MINIMAL) || (defined DFPNSTATONE)
void osl::checkmate::
Dfpn::analyze(const PathEncoding& path_src, 
	      const NumEffectState& src, const std::vector<Move>& moves) const
{
  NumEffectState state(src);
  HashKey key(state);
  PathEncoding path(path_src);
  for (size_t i=0; i<moves.size(); ++i) {
    if (! state.isAlmostValidMove(moves[i]))
      break;
    state.makeMove(moves[i]);
    key = key.newMakeMove(moves[i]);
    path.pushMove(moves[i]);
    DfpnRecord record = table->probe(key, PieceStand(WHITE, state));
    const DfpnPathRecord *path_record = path_table->probe(key);
    std::cerr << i << ' ' << moves[i] << " " << path
	      << ' ' << csa::show(record.best_move) << "\n";
    std::cerr << "  " << record.proof_disproof << ' ' << record.node_count;
    if (path_record) {
      std::cerr << " distance " << path_record->distance << " twins";
      for (SimpleTwinList::const_iterator p=path_record->twin_list.begin();
	   p!=path_record->twin_list.end(); ++p) {
	std::cerr << ' ' << *p;
      }
    }
    std::cerr << "\n";
    DfpnMoveVector moves;
    if (state.turn() == table->attack()) {
      bool has_pawn_checkmate=false;
      if (state.turn() == BLACK)
	generateCheck<BLACK>(state, moves, has_pawn_checkmate);
      else
	generateCheck<WHITE>(state, moves, has_pawn_checkmate);
    }    
    else {
      const Square grand_parent_delay_last_to
	= (record.last_to != state.kingSquare(state.turn())) ? record.last_to : Square();
      if (state.turn() == BLACK)
	generateEscape<WHITE>(state, true, grand_parent_delay_last_to, moves);
      else
	generateEscape<BLACK>(state, true, grand_parent_delay_last_to, moves);
    }
    for (size_t i=0; i<moves.size(); ++i) {
      const Move m = moves[i];
      std::cerr << "    " << m;
      DfpnRecord child = table->probe(key.newMakeMove(m), 
				      PieceStand(WHITE, state).nextStand(WHITE, m));
      std::cerr << ' ' << child.proof_disproof << ' ' << child.node_count;
      const DfpnPathRecord *child_path_record = path_table->probe(key.newMakeMove(m));
      if (child_path_record) {
	std::cerr << " d " << child_path_record->distance << " twins";
	for (const auto& path: child_path_record->twin_list) {
	  std::cerr << ' ' << path;
	}
      }
      if (record.dag_moves & (1ull << i))
	std::cerr << " (*)";
      std::cerr << "\n";
    }
  }
  std::cerr << state;
}
#endif
/* ------------------------------------------------------------------------- */
template <osl::Player P, bool UseTable>
struct osl::checkmate::Dfpn::CallProofOracleAttack
{
  Dfpn *search;
  ProofOracle oracle;
  int proof_limit;
  CallProofOracleAttack(Dfpn *s, const ProofOracle& o, int pl) : search(s), oracle(o), proof_limit(pl)
  {
  }
  void operator()(Square) const 
  {
    search->proofOracleAttack<P,UseTable>(oracle, proof_limit);
  }
};

template <osl::Player P, bool UseTable>
struct osl::checkmate::Dfpn::CallProofOracleDefense
{
  Dfpn *search;
  ProofOracle oracle;
  int proof_limit;
  CallProofOracleDefense(Dfpn *s, const ProofOracle& o, int pl) : search(s), oracle(o), proof_limit(pl)
  {
  }
  void operator()(Square) const 
  {
    search->proofOracleDefense<P,UseTable>(oracle, proof_limit);
  }
};

template <osl::Player P, bool UseTable> 
void osl::checkmate::
Dfpn::proofOracleAttack(const ProofOracle& key, int proof_limit)
{
#ifdef DFPN_DEBUG
  Tree::Logging logging(tree.get(), table, UseTable ? "tpatta" : "pattac");
#endif
  assert(! tree->inCheck(alt(P)));
  const int my_distance = (UseTable && tree->depth) ? (tree->node[tree->depth-1].path_record->distance+1) : 0;
  Node& node = tree->node[tree->depth];
  DfpnRecord& record = node.record;
  LoopToDominance loop;
  DfpnVisitLock<UseTable> lk((node.path_record = (UseTable ? path_table->allocate<P>(node.hash_key, my_distance, loop) : 0)));
  if (UseTable && loop == BadAttackLoop) {
    record = DfpnRecord();
    node.setLoopDetection();
    return;
  }
  assert(node.white_stand == PieceStand(WHITE, tree->state));
  const size_t node_count_org = node_count++;
  if (node_count_limit == root_proof_simulation_limit
      && node_count > 100000) {
    std::cerr << "dfpn proof simulation > 100000 throw " << thread_id << "\n";
    throw DepthLimitReached();
  }
  assert(tree->depth + 2 < tree->MaxDepth);
  if (tree->depth + 2 >= tree->MaxDepth) {
    std::cerr << "throw " << thread_id << "\n";
    throw DepthLimitReached();
  }
  record = table->probe<P>(node.hash_key, node.white_stand);
  if (record.proof_disproof.isFinal())
    return;
#if (defined CHECKMATE_A3_SIMULLATION) || (defined CHECKMATE_A3)
  if (record.node_count == 0)
  {
#ifdef DFPN_STAT
    static stat::Ratio oracle_success("a3-simulation");
#endif
    FixedDepthSolverExt fixed_solver(tree->state);
    PieceStand proof_pieces;
    ProofDisproof pdp = fixed_solver.hasCheckmateMove<P>(2, record.best_move, proof_pieces);
    ++node_count;
#ifdef DFPN_STAT
    oracle_success.add(pdp.isCheckmateSuccess());
#endif
    if (pdp.isCheckmateSuccess()) {
      record.proof_disproof = pdp;
      record.setProofPieces(proof_pieces);
      record.node_count++;
      return;
    }
  }
#elif (!defined CHECKMATE_D2) && (!defined NO_IMMEDIATE_CHECKMATE)
  if (! tree->inCheck(P)
      && ImmediateCheckmate::hasCheckmateMove<P>(tree->state, record.best_move)) {
    PieceStand proof_pieces;	// Note: ImmediateCheckmate が合駒が必要な王手を使わないことに依存
    if (record.best_move.isDrop())
      proof_pieces.add(record.best_move.ptype());
    record.setProofPieces(proof_pieces);
    record.proof_disproof = ProofDisproof::Checkmate();
    return;
  }
#endif
#ifdef DFPN_DEBUG
  if (tree->depth > 1000) {
    std::cerr << tree->state;
    node.hash_key.dumpContents(std::cerr);
    std::cerr << "\n";
    table->showProofOracles<P>(key.key, key.white_stand, node.moved);
  }
#endif
  DfpnRecord oracle = table->findProofOracle<P>(key.key, key.white_stand, node.moved);
  if (! oracle.proof_disproof.isCheckmateSuccess() || ! oracle.best_move.isNormal())
    return;
  const Move check_move = OracleAdjust::attack(tree->state, oracle.best_move);
  if (! check_move.isNormal() || ! key.traceable(P, check_move))
    return;
  
  node.allocate(1);
  node.moves.clear();
  node.moves.push_back(check_move);
  const HashKey new_key = node.hash_key.newHashWithMove(node.moves[0]);
  if (UseTable) {
    node.children[0] = table->probe<P>(new_key, node.nextWhiteStand(P, node.moves[0]));
    node.children_path[0] = path_table->probe(new_key);
    if (node.isLoop(0))
      return;
  } else {
    node.children[0] = DfpnRecord();
    node.children_path[0] = 0;
  }

  if (! UseTable || ! node.children[0].proof_disproof.isFinal()) {
    Node& next = tree->node[tree->depth+1];
    tree->newVisit(P, node.moves[0], new_key);

    CallProofOracleDefense<P,UseTable> helper(this, key.newOracle(P, check_move), proof_limit);
    tree->depth += 1;
    next.path.pushMove(next.moved);
    tree->state.makeUnmakeMove(Player2Type<P>(), next.moved, helper);
    tree->depth -= 1;
    node.children[0] = next.record;
    node.children_path[0] = next.path_record;

    if (next.record.proof_disproof == ProofDisproof::NoEscape()
	&& next.moved.isDrop() && next.moved.ptype() == PAWN)
      node.children[0].proof_disproof = ProofDisproof::PawnCheckmate();
  }  
  if (node.children[0].proof_disproof.isCheckmateSuccess()) {
    node.setCheckmateAttack(P,0);
    record.node_count += node_count - node_count_org;
    if (UseTable || node_count - node_count_org > 32) {
      record.last_move = node.moved;
      table->store(node.hash_key, record);
    }
  }
  else if (UseTable) {
    // dag analyses
    if (record.last_move.isNormal() && record.last_move != node.moved
	&& std::max(record.proof(), record.disproof()) >= 128)
      findDagSource();
    record.last_move = node.moved;
  }
}

template <osl::Player P, bool UseTable> 
void osl::checkmate::
Dfpn::proofOracleDefense(const ProofOracle& key, int proof_limit)
{
#ifdef DFPN_DEBUG
  Tree::Logging logging(tree.get(), table, UseTable ? "tpdefe" : "pdefen");
#endif
  const int my_distance = (UseTable && tree->depth) ? (tree->node[tree->depth-1].path_record->distance+1) : 0;
  Node& node = tree->node[tree->depth];
  LoopToDominance loop;
  DfpnVisitLock<UseTable> lk((node.path_record = (UseTable ? path_table->allocate<P>(node.hash_key, my_distance, loop) : 0)));
  DfpnRecord& record = node.record;
  if (UseTable && loop == BadAttackLoop) {
    record = DfpnRecord();
    node.setLoopDetection();
    return;
  }
  if (! UseTable && tree->depth >= 4) {
    if (tree->node[tree->depth-4].hash_key == node.hash_key
	|| (tree->depth >= 6 && tree->node[tree->depth-6].hash_key == node.hash_key)) {
      record = DfpnRecord();
      return;
    }
  }
  const size_t node_count_org = node_count++;
  assert(node.white_stand == PieceStand(WHITE, tree->state));
  if (! tree->inCheck(alt(P)) || tree->inCheck(P)) {
    record = DfpnRecord();
    record.proof_disproof = ProofDisproof::NoCheckmate();
    return;
  }

  record = table->probe<P>(node.hash_key, node.white_stand);
  if (record.proof_disproof.isFinal())
    return;
  if (proof_limit > ProofSimulationTolerance)
    proof_limit = ProofSimulationTolerance;
  // move generation
  const bool grand_parent_simulation = grandParentSimulationSuitable();
  if (record.last_to == Square())
    record.last_to = grand_parent_simulation ? node.moved.to() : tree->king(alt(P)).square();
  const Square grand_parent_delay_last_to
    = (record.last_to != tree->king(alt(P)).square()) ? record.last_to : Square();
  generateEscape<P>(tree->state, true, grand_parent_delay_last_to, node.moves);
  if (node.moves.empty()) {
    record.setProofPieces(ProofPieces::leaf(tree->state, P, record.stands[P]));
    record.proof_disproof = ProofDisproof::NoEscape();
    return;
  }

  // probe all
  assert(node.children.empty());
  {
    node.allocate(node.moves.size());
    for (size_t i=0;i <node.moves.size(); ++i) {
#ifdef MEMORIZE_SOLVED_IN_BITSET
      if (record.solved & (1ull << i))
	continue;
#endif
      const HashKey& new_key = node.hashes[i] = node.hash_key.newHashWithMove(node.moves[i]);
      node.children[i] = UseTable 
	? table->probe<P>(new_key, node.nextWhiteStand(alt(P), node.moves[i])) 
	: DfpnRecord();
      if (node.children[i].proof_disproof.isCheckmateSuccess()) {
	node.setCheckmateChildInDefense(i);
      }
#ifdef CHECKMATE_D2
      else if (node.record.node_count == 0 && node.children[i].node_count == 0) {
	FixedDepthSolverExt fixed_solver(tree->state);
	PieceStand proof_pieces;
	Move check_move;
	node.children[i].proof_disproof
	  = fixed_solver.hasEscapeByMove<P>(node.moves[i], 0, check_move, proof_pieces);
	if (node.children[i].proof_disproof.isCheckmateSuccess()) {	  
	  node.children[i].best_move = check_move;
	  node.children[i].setProofPieces(proof_pieces);
	  node.setCheckmateChildInDefense(i);
	}
	else {
	  if (node.children[i].proof_disproof.isCheckmateFail())
	    node.children[i].proof_disproof = ProofDisproof(1,1);
	}
	++node_count;
      }
#endif
      if (node.children[i].proof_disproof.isCheckmateFail()) {
	tree->setNoCheckmateDefense(P, i);
	if (UseTable)
	  table->store(node.hash_key, record);
	return;
      }      
      node.children_path[i] = UseTable ? path_table->probe(new_key) : 0;
    }
  }
  assert(node.children.size() == node.moves.size());
  if (UseTable) {
    for (size_t i=0; i<node.children.size(); ++i) {
      if (node.isLoop(i)) {
	node.setLoopDetection();
	return;
      }
    }
  }
  unsigned int sum_proof=0, min_disproof=record.min_pdp;
  int num_proof = 0;
  for (size_t next_i=0; next_i<node.children.size(); ++next_i) {
#ifdef MEMORIZE_SOLVED_IN_BITSET
    if (record.solved & (1ull << next_i))
      continue;
#endif
    if (node.children[next_i].proof_disproof.isCheckmateSuccess()) {
      min_disproof = std::min(min_disproof, node.children[next_i].disproof());
      continue;
    }
    if (! key.traceable(alt(P), node.moves[next_i])) {
      ++sum_proof;
      min_disproof = 1;
      if (! UseTable)
	break;
      continue;
    }
    const Square next_to = node.moves[next_i].to();
    if (sum_proof && tree->state.hasEffectAt(P, next_to)
	&& (! tree->state.hasEffectAt(alt(P), next_to)
	    || (tree->state.countEffect(alt(P), next_to) == 1
		&& ! node.moves[next_i].isDrop())))
      continue;
    assert(! node.isLoop(next_i));
    Node& next = tree->node[tree->depth+1];
#ifdef MEMORIZE_SOLVED_IN_BITSET
    assert(! (record.solved & (1ull << next_i)));
#endif
    tree->newVisit(alt(P), node.moves[next_i], node.hashes[next_i]);

    CallProofOracleAttack<P,UseTable> helper(this, key.newOracle(alt(P), node.moves[next_i]), proof_limit-sum_proof);
    tree->depth += 1;
    next.path.pushMove(node.moves[next_i]);
    tree->state.makeUnmakeMove(Player2Type<alt(P)>(), node.moves[next_i], helper);
    tree->depth -= 1;

    node.children[next_i] = next.record;
    node.children_path[next_i] = next.path_record;
    if (next.record.proof_disproof.isCheckmateFail()) {
      if (record.proof_disproof.isLoopDetection())
	node.setLoopDetection();
      else
	tree->setNoCheckmateDefense(P, next_i);
      record.node_count += node_count - node_count_org;
      if (UseTable)
	table->store(node.hash_key, record);
      return;
    }
    if (next.record.proof_disproof.isCheckmateSuccess()) {
      node.setCheckmateChildInDefense(next_i);
      ++num_proof;
    }
    sum_proof += next.record.proof();
    min_disproof = std::min(min_disproof, next.record.disproof());
    if ((sum_proof && ! UseTable) || (int)sum_proof > proof_limit)
      break;
  }
  if (sum_proof == 0) {
    node.record.proof_disproof = ProofDisproof(sum_proof, min_disproof);
    node.setCheckmateDefense(P, tree->state);
  }
  else if (UseTable) {
    // dag analyses
    if (record.last_move.isNormal() && record.last_move != node.moved
	&& std::max(record.proof(), record.disproof()) >= 128)
      findDagSource();
    record.last_move = node.moved;
  }
}

template <osl::Player P> 
void osl::checkmate::
Dfpn::blockingSimulation(int oracle_i, const ProofOracle& oracle)
{
#ifdef DFPN_DEBUG
  Tree::Logging logging(tree.get(), table, "blocks");
#endif
#ifdef DFPN_STAT
  static stat::Ratio oracle_success("blocking proof");
#endif
  Node& node = tree->node[tree->depth];
  Node& next = tree->node[tree->depth+1];
  const Move oracle_move = node.moves[oracle_i];
  const Square to = oracle_move.to();
  assert((node.record.solved & (1ull << oracle_i))
	 || node.children[oracle_i].proof_disproof.isCheckmateSuccess());
  for (size_t i=0; i<node.moves.size(); ++i) {
#ifdef MEMORIZE_SOLVED_IN_BITSET
    if (node.record.solved & (1ull << i))
      continue;
#endif
    if (node.isLoop(i))
      break;
    if (node.children[i].proof_disproof.isFinal() || node.moves[i].to() != to)
      continue;
    if (! oracle.traceable(alt(P), node.moves[i]))
      continue;
#ifdef MEMORIZE_SOLVED_IN_BITSET
    assert(! (node.record.solved & (1ull << i)));
#endif
    tree->newVisit(alt(P), node.moves[i], node.hashes[i]);
    CallProofOracleAttack<P,true> helper(this, oracle, node.threshold.proof());

    tree->depth += 1;
    next.path.pushMove(node.moves[i]);
    tree->state.makeUnmakeMove(Player2Type<alt(P)>(), node.moves[i], helper);
    tree->depth -= 1;

    node.children[i] = next.record;
    node.children_path[i] = next.path_record;    

#ifdef DFPN_STAT
    oracle_success.add(next.record.proof_disproof.isCheckmateSuccess());
#endif
    if (next.record.proof_disproof.isCheckmateSuccess()) {
      node.setCheckmateChildInDefense(i);
    }
  }
}

template <osl::Player P> 
void osl::checkmate::
Dfpn::grandParentSimulation(int cur_i, const Node& gparent, int gp_i)
{
#ifdef DFPN_DEBUG
  Tree::Logging logging(tree.get(), table, "grands");
#endif
#ifdef DFPN_STAT
  static stat::Ratio oracle_success("grandparent proof", true);
#endif
  Node& node = tree->node[tree->depth];
  Node& next = tree->node[tree->depth+1];

  const Move move = gparent.moves[gp_i];
  assert(move == node.moves[cur_i]);
  const HashKey& oracle_hash = (gparent.record.solved & (1ull << gp_i)) 
    ? gparent.hash_key.newHashWithMove(move)
    : gparent.hashes[gp_i];
  const ProofOracle oracle(oracle_hash, gparent.nextWhiteStand(alt(P), move));

  tree->newVisit(alt(P), node.moves[cur_i], node.hashes[cur_i]);
  CallProofOracleAttack<P,true> helper(this, oracle, gparent.threshold.proof());

  tree->depth += 1;
  next.path.pushMove(move);
  tree->state.makeUnmakeMove(Player2Type<alt(P)>(), move, helper);
  tree->depth -= 1;

  node.children[cur_i] = next.record;
  node.children_path[cur_i] = next.path_record;    
#ifdef DFPN_STAT
  oracle_success.add(next.record.proof_disproof.isCheckmateSuccess());
#endif
}

// debug
int osl::checkmate::
Dfpn::distance(const HashKey& key) const
{
  const DfpnPathRecord *record = path_table->probe(key);
  if (record)
    return record->distance;
  return -1;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
