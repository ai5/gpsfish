/* alphaBeta2Parallel.h
 */
#ifndef _ALPHABETA2PARALLEL_H
#define _ALPHABETA2PARALLEL_H

#ifdef OSL_SMP

#include "osl/search/alphaBeta2.h"
#include "osl/misc/atomicCounter.h"
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/ptr_container/ptr_vector.hpp>

#define SPLIT_STAT
// #define OSL_SMP_NO_SPLIT_ROOT

namespace osl
{
  namespace search
  {
    // block_id: -1 illegal, 0 not used (master tree), 1- parallel search
    struct AlphaBeta2ParallelCommon : boost::noncopyable
    {
      struct TreeInfoCommon
      {
	int thread_id;
	volatile int nprocs;
	MoveLogProb best_move;
	AlphaBeta2Window window;
	int value;
	size_t nodes_searched;
	bool is_root, in_pv;
	Player turn;
      };
      struct TreeInfo : public TreeInfoCommon
      {
	CArray<volatile int, OslConfig::MaxThreads> siblings;
	volatile int parent;
	MoveLogProbVector moves;
	volatile int move_index, search_value;
	int alpha_update, last_alpha_update;
	volatile bool used;
	LightMutex lock;

	TreeInfo() : used(0)
	{
	  siblings.fill(0);
	}
	void set(const TreeInfo& parent, int max_threads)
	{
	  TreeInfoCommon::operator=(parent);
	  used = true;
	  for (int i=0; i<max_threads; i++)
	    siblings[i] = 0;
	  search_value = 0;  
	}
      };
      struct LivingThreadLock;
      typedef SearchState2::checkmate_t checkmate_t;
      static const int MaxBlocksPerCpu = 16;
      static const int MaxBlocks = OslConfig::MaxThreads*MaxBlocksPerCpu+1;
      CArray<volatile int, OslConfig::MaxThreads> job;
      CArray<checkmate_t*, OslConfig::MaxThreads> checkmate;

      CArray<TreeInfo,MaxBlocks> info;
      /** job the thread waiting for, maybe obsolete */
      CArray<volatile int,OslConfig::MaxThreads> waiting;
      CArray<std::thread*,OslConfig::MaxThreads> threads;
      std::mutex lock_smp;
      std::condition_variable condition_smp;
      volatile int smp_idle;
      volatile bool quit;
      unsigned int parallel_splits;
      unsigned int max_split_depth, descendant_reject, descendant_test;

      std::mutex living_threads_lock;
      std::condition_variable living_threads_condition;
      volatile int living_threads;

      int max_threads, max_thread_group;
      int split_min_limit;

      int my_id;
      AtomicCounter parallel_abort;
      AtomicCounter cancelled_splits;
      bool started;

      AlphaBeta2ParallelCommon();
      ~AlphaBeta2ParallelCommon();
      static checkmate_t*& checkmateSearcher(SearchState2& state) { return state.checkmate_searcher; }

      void waitAll();
      bool isDescendant(int elder, int younger);

      struct SplitFailed {};
    };
    template <class EvalT>
    struct AlphaBeta2Parallel : public AlphaBeta2ParallelCommon
    {
      struct Worker
      {
	AlphaBeta2Parallel *shared;
	int thread_id;
	Worker(int tid, AlphaBeta2Parallel *shared);
	void operator()();
      };

      typedef AlphaBeta2Tree<EvalT> tree_t;
      CArray<tree_t*,MaxBlocks> tree;
      tree_t *master;

      explicit AlphaBeta2Parallel(tree_t *master);
      ~AlphaBeta2Parallel();
      void threadStart();

      void testStop();

      void search(int tree_id);
      void threadWait(int thread_id, int waiting);
      /** @return true if search finished after successful split, false if split failed */
      bool split(tree_t *tree, int tree_id, int thread_id, int max_split);
      void stopThread(int tree_id);
      void copyToParent(int parent, int child);
      /** @return 0 failed, block_id otherwise */
      int copyToChild(int parent, int thread_id);

      int treeId(tree_t *tree);
      int parentID(int tree_id) { return info[tree_id].parent; }
      TreeInfo* parent(int tree_id) { return &info[parentID(tree_id)]; }
      const std::pair<MoveLogProb,size_t> nextMove(int tree_id);
      size_t checkmateCount() const;
      size_t mainCheckmateCount() const;
    };
  }
}

#endif /* OSL_SMP */

#endif /* _ALPHABETA2PARALLEL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
