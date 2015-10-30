/* dfpn.h
 */
#ifndef OSL_DFPN_H
#define OSL_DFPN_H
#include "osl/checkmate/proofDisproof.h"
#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include "osl/pathEncoding.h"
#include "osl/config.h"
#include <boost/scoped_array.hpp>

#ifdef OSL_SMP
#  ifndef OSL_DFPN_SMP
#    define OSL_DFPN_SMP
#  endif
#endif

#ifdef OSL_DFPN_SMP
#  include "osl/misc/lightMutex.h"
#  include <mutex>
#endif

namespace osl
{
  namespace checkmate
  {
    class DfpnRecord;
    /** 詰探索局面表 -- 並列でも共有する部分 */
    class DfpnTable
    {
      struct Table;
      struct List;
      boost::scoped_array<Table> table;
      size_t total_size;
      int dfpn_max_depth;
      size_t growth_limit, gc_threshold;
    public:
      DfpnTable(Player attack);
      DfpnTable();
      ~DfpnTable();
      template <Player Attack>
      const DfpnRecord probe(const HashKey& key, PieceStand white) const;
      const DfpnRecord probe(const HashKey& key, PieceStand white) const;
      size_t estimateNodeCount(const HashKey& key, bool dominance_max=false) const;
      template <Player Attack>
      const DfpnRecord findProofOracle(const HashKey& key, PieceStand white, Move last_move=Move()) const;
      const DfpnRecord findProofOracle(const HashKey& key, PieceStand white, Move last_move=Move()) const;
      template <Player Attack>
      void showProofOracles(const HashKey& key, PieceStand white, Move last_move=Move()) const;
      size_t
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      size() const;
      void showStats() const;

      void setAttack(Player);
      void setWorking(const HashKey& key, const DfpnRecord& value, int thread_id);
      void leaveWorking(const HashKey& key, int thread_id);
      void store(const HashKey& key, DfpnRecord& value, int leaving_thread_id=-1);
      void addDag(const HashKey& key, DfpnRecord& value);
      void clear();
      size_t totalSize() { return total_size; }
      Player attack() const;

      void setMaxDepth(int);
      int maxDepth() const;

      void testTable();
      size_t smallTreeGC(size_t threshold=10);
      /** set the maximum size of table (otherwise infinity).  this is one of preconditions to enable GC inside dfpn. */
      void setGrowthLimit(size_t new_limit);
      size_t growthLimit() const { return growth_limit; }
      bool runGC();
    private:
#ifdef OSL_DFPN_SMP
      typedef osl::misc::LightMutex Mutex;
#  ifdef USE_TBB_HASH
      static const int DIVSIZE=1;
#  else
      static const int DIVSIZE=256;
      mutable CArray<Mutex,DIVSIZE> mutex;
#  endif
      // typedef boost::mutex Mutex;
      // TODO: boost::thread::shared_lock (available version >= 1.35) for multi read accessess
      LightMutex root_mutex;
#else
      static const int DIVSIZE=1;
#endif
      static int keyToIndex(const HashKey& key)
      {
	unsigned long val=key.signature();
	return (val>>24)%DIVSIZE;
      }
      template <Player Attack>
      List *find(const HashKey& key, int subindex);
      template <Player Attack>
      const List *find(const HashKey& key, int subindex) const;
      const List *find(const HashKey& key, int subindex) const;
    };
    /** 詰探索局面表 -- thread local */
    class DfpnPathTable;
    /** DfpnParallel の協調動作用 */
    class DfpnShared;
    /** 詰探索 */
    class Dfpn 
    {
      Dfpn(const Dfpn&) = delete;
      Dfpn& operator=(const Dfpn&) = delete;      
    public:
      enum { DfpnMaxUniqMoves = CheckOrEscapeMaxUniqMoves };
      typedef CheckMoveVector DfpnMoveVector;
      typedef DfpnTable table_t;
    private:
      DfpnTable *table;
      struct NodeBase;
      struct Node;
      struct Tree;
      std::unique_ptr<Tree> tree;
      std::unique_ptr<DfpnPathTable> path_table;
      size_t node_count;
      size_t node_count_limit;
      DfpnShared *parallel_shared;
      int thread_id;
      bool blocking_verify;
    public:
      Dfpn();
      ~Dfpn();
      void setTable(DfpnTable *new_table);
      void setIllegal(const HashKey& key, PieceStand white);
      void setBlockingVerify(bool enable=true) { blocking_verify = enable; }
      void setParallel(int id, DfpnShared *s) 
      {
	if (s)
	  assert(id >= 0);
	thread_id = id; 
	parallel_shared = s; 
      }
      const ProofDisproof 
      hasCheckmateMove(const NumEffectState& state, const HashKey& key,
		       const PathEncoding& path, size_t limit, Move& best_move,
		       Move last_move=Move::INVALID(), std::vector<Move> *pv=0);
      const ProofDisproof 
      hasCheckmateMove(const NumEffectState& state, const HashKey& key,
		       const PathEncoding& path, size_t limit, Move& best_move, PieceStand& proof,
		       Move last_move=Move::INVALID(), std::vector<Move> *pv=0);
      const ProofDisproof
      hasEscapeMove(const NumEffectState& state, 
		    const HashKey& key, const PathEncoding& path, 
		    size_t limit, Move last_move);

      size_t nodeCount() const { return node_count; }
      const DfpnTable& currentTable() const { return *table; }
      void analyze(const PathEncoding& path, 
		   const NumEffectState& state, const std::vector<Move>& moves) const;
      void clear();

      // private:
      template <Player P> void attack();
      template <Player P> void defense();
      template <Player P> struct CallAttack;      
      template <Player P> struct CallDefense;      
      struct DepthLimitReached {};

      struct ProofOracle;
      template <Player P, bool UseTable> void proofOracleAttack(const ProofOracle& oracle, int proof_limit);
      template <Player P, bool UseTable> void proofOracleDefense(const ProofOracle& oracle, int proof_limit);
      template <Player P, bool UseTable> struct CallProofOracleAttack;      
      template <Player P, bool UseTable> struct CallProofOracleDefense;      
      /** 合駒が詰と判った直後に、同じような合駒を詰める */
      template <Player P> void blockingSimulation(int seed, const ProofOracle&);
      template <Player P> void grandParentSimulation(int cur_move, const Node& gparent, int gp_move);
    private:
      template <bool UseTable>
      const ProofDisproof 
      tryProofMain(const NumEffectState& state, const HashKey& key,
		   const PathEncoding& path, const ProofOracle&, size_t oracle_id, Move& best_move,
		   Move last_move);
    public:
      const ProofDisproof 
      tryProof(const NumEffectState& state, const HashKey& key,
	       const PathEncoding& path, const ProofOracle&, size_t oracle_id, Move& best_move,
	       Move last_move=Move::INVALID());
      const ProofDisproof 
      tryProofLight(const NumEffectState& state, const HashKey& key,
		    const PathEncoding& path, const ProofOracle&, size_t oracle_id, Move& best_move,
		    Move last_move=Move::INVALID());

      // debug
      int distance(const HashKey&) const;
      /** Pは攻撃側 */
      template <Player P>
      static void generateCheck(const NumEffectState&, DfpnMoveVector&, bool&);
      /** Pは攻撃側 */
      template <Player P>
      static void generateEscape(const NumEffectState&, bool need_full_width, 
				 Square grand_parent_delay_last_to, DfpnMoveVector&);
      /** test suitability of simulation of grand-parent relation */
      bool grandParentSimulationSuitable() const;
      template <Player Turn>
      static void sort(const NumEffectState&, DfpnMoveVector&);
    private:
      void findDagSource();
      void findDagSource(const HashKey& terminal_key,
			 DfpnRecord& terminal_record,
			 PieceStand terminal_stand, int offset=0);
    };

  }
}

struct osl::checkmate::Dfpn::ProofOracle
{
  HashKey key;
  PieceStand white_stand;
  ProofOracle(const HashKey& k, PieceStand w) : key(k), white_stand(w)
  {
  }
  const ProofOracle newOracle(Player P, Move move) const
  {
    assert(P == move.player());
    return ProofOracle(key.newHashWithMove(move),
		       (P == WHITE) ? white_stand.nextStand(P, move) : white_stand);
  }
  bool traceable(Player P, Move move) const
  {
    assert(P == move.player());
    if (! move.isDrop()) 
      return true;
    if (P == BLACK) {
      if (key.blackStand().get(move.ptype()) == 0)
	return false;
    }
    else {
      if (white_stand.get(move.ptype()) == 0)
	return false;
    }
    return true;
  }
};

#endif /* OSL_DFPN_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
