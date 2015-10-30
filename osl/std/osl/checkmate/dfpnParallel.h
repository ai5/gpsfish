/* dfpnParallel.h
 */
#ifndef OSL_DFPNPARALLEL_H
#define OSL_DFPNPARALLEL_H

#include "osl/checkmate/dfpn.h"
#include "osl/misc/lightMutex.h"

namespace osl
{
  namespace checkmate
  {
    class DfpnShared
    {
    public:
      struct ThreadData
      {
	HashKey restart_key;
	volatile int depth;
	volatile bool restart;
	LightMutex mutex;
	ThreadData() : depth(0), restart(false)
	{
	}
	void clear()
	{
	  restart = false;
	  restart_key = HashKey();
	}
      }
#ifdef __GNUC__
      __attribute__ ((aligned (64)))
#endif
	;
      volatile bool stop_all;
      CArray<ThreadData, 32> data;
      DfpnShared() : stop_all(false)
      {
      }
      void restartThreads(const HashKey& key, int depth, unsigned int threads)
      {
	for (int i=0; i<32; ++i)
	  if ((1u << i) & threads) {
	    SCOPED_LOCK(lk, data[i].mutex);
	    if (! data[i].restart || data[i].depth > depth) {
	      data[i].restart_key = key;
	      data[i].depth = depth;
	      data[i].restart = true;
	    }
	  }
      }
      void clear()
      {
	stop_all = false;
	for (size_t i=0; i<data.size(); ++i)
	  data[i].clear();
      }
    };
#ifdef OSL_DFPN_SMP
    class DfpnParallel 
    {
      DfpnParallel(const DfpnParallel&) = delete;
      DfpnParallel& operator=(const DfpnParallel&) = delete;
    private:
      DfpnTable *table;
      boost::scoped_array<Dfpn> workers;
      size_t num_threads;
      // working data
      const NumEffectState *state;
      HashKey key;
      PathEncoding path;
      Move last_move;
      size_t limit;
      struct WorkerData
      {
	Move best_move;
	PieceStand proof;
	ProofDisproof result;
      };
      boost::scoped_array<WorkerData> worker_data;
      DfpnShared shared;
    public:
      explicit DfpnParallel(size_t num_threads=0);
      ~DfpnParallel();
      void setTable(DfpnTable *new_table);
      
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

      size_t nodeCount() const;
      const DfpnTable& currentTable() const { return *table; }
      void analyze(const PathEncoding& path,
		   const NumEffectState& state, const std::vector<Move>& moves) const;

      void stopNow() 
      {
	shared.stop_all = true;
      }

      struct AttackWorker;
      struct DefenseWorker;
      friend struct AttackWorker;
      friend struct DefenseWorker;
    };
#endif
  }
}


#endif /* OSL_DFPNPARALLEL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
