/* alphaBeta2Parallel.cc
 */
#ifdef OSL_SMP

#include "osl/search/alphaBeta2Parallel.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/usiReporter.h"
#include <iostream>
#include <bitset>
#ifdef _WIN32
# include <malloc.h>
# include <chrono>
# include <thread>
#endif

#define DEBUG_SMP 0
#define ONLY_HELP_DESCENDANT
// #define OSL_BUSY_WAIT

/* ------------------------------------------------------------------------- */
struct osl::search::AlphaBeta2ParallelCommon::LivingThreadLock
{
  AlphaBeta2ParallelCommon *shared;
  LivingThreadLock(AlphaBeta2ParallelCommon *s) : shared(s)
  {
    std::lock_guard<std::mutex> lk(shared->living_threads_lock);
    shared->living_threads += 1;
    shared->living_threads_condition.notify_one();
  }
  ~LivingThreadLock()
  {
    std::lock_guard<std::mutex> lk(shared->living_threads_lock);
    shared->living_threads -= 1;
    shared->living_threads_condition.notify_one();
  }
};

/* ------------------------------------------------------------------------- */
template <class EvalT>
osl::search::AlphaBeta2Parallel<EvalT>::
Worker::Worker(int tid, AlphaBeta2Parallel *s) : shared(s), thread_id(tid)
{
}

template <class EvalT>
void
#ifdef __GNUC__
#  ifdef _WIN32
__attribute__((noinline))
__attribute__((force_align_arg_pointer)) 
#  endif
#endif
osl::search::AlphaBeta2Parallel<EvalT>::Worker::operator()()
{
#if DEBUG_SMP > 1
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    std::cerr << "thread " << thread_id << " started\n";
  }
#endif
  try {
    AlphaBeta2ParallelCommon::LivingThreadLock lk(shared);
    shared->threadWait(thread_id, -1);
  }
  catch (std::exception& e) {
    std::cerr << "warning caught exception in thread root " << e.what() << "\n";
  }
  catch (...) {
    std::cerr << "warning caught unknown exception in thread root\n";
  }
}

/* ------------------------------------------------------------------------- */

static osl::misc::AtomicCounter parallel_counter;

osl::search::AlphaBeta2ParallelCommon::
AlphaBeta2ParallelCommon() 
  : smp_idle(0), quit(false), 
    parallel_splits(0), max_split_depth(0), descendant_reject(0), descendant_test(0),
    living_threads(0), max_threads(OslConfig::concurrency()), max_thread_group(5),
    split_min_limit(400), my_id(parallel_counter.valueAndinc()), started(false)
{
  job.fill(0);
  info[0].thread_id = 0;
  info[0].used = true;
  info[0].parent = -1;
  waiting.fill(0);
  
  threads.fill(nullptr);
  checkmate.fill(nullptr);
  for (int i=1; i<max_threads; ++i) {
    threads[i] = 0;
    checkmate[i] = 0;
  }
}

osl::search::AlphaBeta2ParallelCommon::
~AlphaBeta2ParallelCommon() 
{
  waitAll();
  for (int i=1; i<max_threads; ++i) {
    delete checkmate[i];
  }  
#if DEBUG_SMP > 1
  std::cerr << "<= AlphaBeta2Parallel " << my_id << "\n";
#endif
#if DEBUG_SMP > 2
  std::cerr << "descendant_reject " << descendant_reject 
	    << " / " << descendant_test << " = " << (double)descendant_reject/descendant_test << "\n";
  std::cerr << "max_split_depth " << max_split_depth << "\n";
#endif
}

void osl::search::
AlphaBeta2ParallelCommon::waitAll() {
#ifndef OSL_BUSY_WAIT
  {
  std::lock_guard<std::mutex> lk(lock_smp);
#endif      
  quit = true;
  started = false;
#ifndef OSL_BUSY_WAIT
  condition_smp.notify_all();
  }
#endif
  std::unique_lock<std::mutex> lk(living_threads_lock);
  while (living_threads != (quit ? 0 : smp_idle)) {
    living_threads_condition.wait(lk);
  }
  for (int i=1; i<max_threads; ++i) {
    if (threads[i]) {
      threads[i]->join();
      delete threads[i];
    }
    threads[i] = 0;
  }  
}

bool osl::search::AlphaBeta2ParallelCommon::isDescendant(int elder, int younger)
{
#ifndef ONLY_HELP_DESCENDANT
  return true;
#else
  ++descendant_test;
  if (elder < 0)
    return true;
  while (younger >= 0) {
    if (elder == younger)
      return true;
    younger = info[younger].parent;
  } 
  ++descendant_reject;
  return false;
#endif
}

/* ------------------------------------------------------------------------- */

template <class EvalT>
osl::search::AlphaBeta2Parallel<EvalT>::
AlphaBeta2Parallel(AlphaBeta2Tree<EvalT> *m) 
  : AlphaBeta2ParallelCommon(), master(m)
{
#if DEBUG_SMP > 0
  std::cerr << "=> AlphaBeta2Parallel " << max_threads << " threads ";
# if DEBUG_SMP > 1
  std::cerr << " id " << my_id;
# endif
  std::cerr << "\n";
#endif
  tree.fill(nullptr);
  tree[0] = master;
  checkmate[0] = checkmateSearcher(*master);
}

template <class EvalT>
osl::search::AlphaBeta2Parallel<EvalT>::
~AlphaBeta2Parallel() 
{
}

template <class EvalT>
void osl::search::AlphaBeta2Parallel<EvalT>::
threadStart()
{
  if (started)
    return;
  started = true;
  quit = false;
  int i=1;
  for (; i<max_threads; ++i) {
    int j=0, max_retry=4;
    for (; j<max_retry; ++j) {
      try 
      {
	if (! checkmate[i])
	  checkmate[i] = new checkmate_t(master->checkmateSearcher());
	threads[i] = new std::thread(Worker(i, this));
	break;
      } 
      catch (std::exception& e)
      {
	std::cerr << e.what() << "\n";
      }
      std::cerr << "wait for thread " << i << " started\n";
      const int microseconds = (j+1)*100000;
#ifdef _WIN32
      std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
#else
      usleep(microseconds);
#endif
    }
    if (j == max_retry)
      break;
  }
  if (i < max_threads) 
  {
    std::cerr << "error in start thread #" << i << "\n";
    for (int j=i; j<max_threads; ++j) {
      delete checkmate[j];
      checkmate[j] = 0;
    }
    max_threads = i;
  }
  std::unique_lock<std::mutex> lk(living_threads_lock);
  while (living_threads+1 != max_threads) {
    living_threads_condition.wait(lk);
  }
}

template <class EvalT>
void osl::search::AlphaBeta2Parallel<EvalT>::
search(int tree_id)
{
  TreeInfo *info = &this->info[tree_id];
  assert(tree[tree_id]);
  if (info->is_root)
    tree[tree_id]->examineMovesRootPar(tree_id);
  else
    tree[tree_id]->examineMovesOther(tree_id);
}

template <class EvalT>
int osl::search::
AlphaBeta2Parallel<EvalT>::treeId(AlphaBeta2Tree<EvalT> *t) 
{
  if (t == master)
    return 0;
  for (size_t i=1; i<tree.size(); ++i)
    if (t == tree[i])
      return i;
  assert(0);
  abort();
}

template <class EvalT>
void osl::search::AlphaBeta2Parallel<EvalT>::
threadWait(int thread_id, int waiting)
{
#if DEBUG_SMP > 2
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    std::cerr << "thread " << thread_id << " ready, waiting " << waiting << "\n";
  }
#endif
  while (1) {
    this->waiting[thread_id] = waiting;
    {
      std::lock_guard<std::mutex> lk(lock_smp);
      smp_idle++;
    }
  {
#ifndef OSL_BUSY_WAIT
    std::unique_lock<std::mutex> lk(lock_smp);
#endif      
    while (! job[thread_id] 
	   && ! quit 
	   && (waiting < 0 || info[waiting].nprocs))
    {
#ifndef OSL_BUSY_WAIT
      condition_smp.wait(lk);
#endif      
    }
    
    if (quit) {
      {
#ifdef OSL_BUSY_WAIT
	std::lock_guard<std::mutex> lk(lock_smp);
#endif
	--smp_idle;
      }
#if DEBUG_SMP > 1
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "thread " << thread_id << " exiting\n";
#endif
      return;
    }
    {
#ifdef OSL_BUSY_WAIT
      std::lock_guard<std::mutex> lk(lock_smp);
#endif
      if (! job[thread_id])
	job[thread_id] = waiting;
      --smp_idle;
    }
  }

    if (job[thread_id] == waiting) {
#ifndef NDEBUG
      if (waiting >= 0) {
	for (int i=0; i<max_threads; ++i) {
	  assert(info[waiting].siblings[i] == 0);
	}
	assert(info[waiting].nprocs == 0);
      }
#endif
#if DEBUG_SMP > 3
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "thread " << thread_id << " go up " 
		<< waiting << " " << info[job[thread_id]].best_move << "\n";
#endif
      return;
    }

    if (quit || job[thread_id] == -1) {
      return;
    }
    int my_job = job[thread_id];
#if DEBUG_SMP > 3
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "thread " << thread_id << " go to job " << my_job << " waiting " << waiting << "\n";
      if (! tree[my_job]) {
	std::cerr << "thread " << thread_id << " null job " << my_job << " waiting " << waiting << "\n";
      }
    }
#endif

    assert(tree[my_job]);
    search(my_job);

    int parent = info[my_job].parent;
    std::lock_guard<std::mutex> lk(lock_smp);
    {
      SCOPED_LOCK(lk,info[parent].lock);
      copyToParent(parent, my_job);
      info[parent].nprocs--;
      info[parent].siblings[thread_id] = 0;
#ifndef OSL_BUSY_WAIT
      if (info[parent].nprocs == 0)
	condition_smp.notify_all();
#endif
    }
    job[thread_id] = 0;
    delete tree[my_job];
    tree[my_job] = 0;
#if DEBUG_SMP > 3
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "thread " << thread_id << " back from job " << my_job << " waiting " << waiting;
      if (waiting >= 0)
	std::cerr << " rest " << info[waiting].nprocs;
      std::cerr << "\n";
    }
#endif
  }
}

template <class EvalT>
bool osl::search::AlphaBeta2Parallel<EvalT>::
split(AlphaBeta2Tree<EvalT> *tree, int tree_id, int thread_id, int max_split)
{
  TreeInfo *pinfo = &info[tree_id];
#if DEBUG_SMP > 2
  {
    unsigned int depth = 0;
    int parent = pinfo->parent;
    while (parent >= 0)
      ++depth, parent = info[parent].parent;;
    max_split_depth = std::max(depth, max_split_depth);
  }
  for (int i=0; i<max_threads; ++i) {
    assert(pinfo->siblings[i] == 0);
  }
#endif
  assert(tree == master || tree == this->tree[tree_id]);
  {
    std::lock_guard<std::mutex> lk(lock_smp);
    {
      int tid=0;
      for (; tid<max_threads && job[tid]; ++tid)
	;
      if (tid == max_threads || tree->stop_tree)
	return false;
    }
  
    parallel_splits++;  
    job[pinfo->thread_id] = 0;
    pinfo->nprocs = 0;

    int nblocks = 0;
    if (const int child_id = copyToChild(tree_id, thread_id))
    {
      // first, assgin job to splitting thread
      nblocks++;
      pinfo->siblings[thread_id] = child_id;
      info[child_id].thread_id = thread_id;
      info[child_id].parent = tree_id;
      pinfo->nprocs++;
    }
    if (max_split <= 0)
      max_split = std::max(max_threads/2, max_thread_group);
    else
      max_split = std::min(max_split, std::max(max_threads/2, max_thread_group));
    for (int tid = 0;
	 tid < max_threads && nblocks < max_split;
	 ++tid) {    
      assert(pinfo->siblings[tid] == 0 || tid == thread_id);
      if (job[tid] || tid == thread_id) 	// he is working
	continue;
      if (! isDescendant(waiting[tid], pinfo->parent))
	continue;
      int child_id = copyToChild(tree_id, tid);
      if (!child_id)
	continue;
#if DEBUG_SMP > 3
      {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
	std::cerr << "split " << tree_id << " in " << thread_id << " => " << child_id << " in " << tid << "\n";
      }
#endif
      nblocks++;
      pinfo->siblings[tid] = child_id;
      info[child_id].thread_id = tid;
      info[child_id].parent = tree_id;
      pinfo->nprocs++;
    }  
    pinfo->search_value = pinfo->value;
  
    if (!nblocks) {    
      job[pinfo->thread_id] = tree_id;
      return false;
    }
  
    for (int tid=0; tid< max_threads; ++tid)
      if (pinfo->siblings[tid])
	job[tid] = pinfo->siblings[tid];  
  }
#ifndef OSL_BUSY_WAIT
  condition_smp.notify_all();
#endif
  threadWait(pinfo->thread_id, tree_id);
  
  return true;
}

template <class EvalT>
void osl::search::
AlphaBeta2Parallel<EvalT>::stopThread(int tree_id)
{
  TreeInfo *info = &this->info[tree_id];
  AlphaBeta2Tree<EvalT> *tree = this->tree[tree_id];
  SCOPED_LOCK(lk,info->lock);
  tree->stop_tree = true;
  for (int tid = 0; tid<max_threads; tid++)
    if (info->siblings[tid])
      stopThread(info->siblings[tid]);
}

template <class EvalT>
void osl::search::
AlphaBeta2Parallel<EvalT>::copyToParent(int parent, int child) 
{
  TreeInfo *c = &info[child];
  AlphaBeta2Tree<EvalT> *cc = tree[child], *pp = tree[parent];
  c->used = 0;    
  pp->node_count += cc->nodeCount();
  pp->mpn.merge(cc->mpn);
  pp->mpn_cut.merge(cc->mpn_cut);
  pp->alpha_update.merge(cc->alpha_update);
  pp->last_alpha_update.merge(cc->last_alpha_update);
  pp->ext.merge(cc->ext);
  pp->ext_limit.merge(cc->ext_limit);
}

template <class EvalT>
int osl::search::
AlphaBeta2Parallel<EvalT>::copyToChild(int parent, int thread_id)
{  
  static int warnings = 0;  
  int first = thread_id * MaxBlocksPerCpu + 1;  
  int last = first + MaxBlocksPerCpu;
  int maxb = max_threads * MaxBlocksPerCpu + 1;

  int cid=first;
  for (; cid < last && info[cid].used; cid++)
    ;
  
  if (cid >= last) {    
    if (++warnings < 6) {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "WARNING.  optimal SMP block cannot be allocated, thread "
		<< thread_id << "\n";
    }
    for (cid=1; cid<maxb && info[cid].used; cid++)
      ;    
    if (cid >= maxb) {
      if (warnings < 6) {
	std::lock_guard<std::mutex> lk(OslConfig::lock_io);
        std::cerr << "ERROR.  no SMP block can be allocated\n";
      }
      return 0;      
    }    
  }

  TreeInfo *c = &info[cid], *p = &info[parent];
  try 
  {
    assert(tree[cid] == 0);
    tree[cid] = new AlphaBeta2Tree<EvalT>(*tree[parent], this);
  }
  catch (std::bad_alloc&)
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    std::cerr << "ERROR.  split failed due to bad_alloc\n";
    return 0;
  }
  c->set(*p, max_threads);
  tree[cid]->setCheckmateSearcher(checkmate[thread_id]);

  return cid;
}

template <class EvalT>
const std::pair<osl::MoveLogProb,size_t> osl::search::
AlphaBeta2Parallel<EvalT>::nextMove(int tree_id)
{
  int parent = info[tree_id].parent;
  TreeInfo *info = &this->info[parent];
  SCOPED_LOCK(lk,info->lock);
  const size_t old_index = info->move_index;
  if (tree[parent]->stop_tree)
    return std::make_pair(MoveLogProb(), old_index);
  if (info->is_root) {
    if (old_index < info->moves.size()) {
      ++(info->move_index);
      return std::make_pair(info->moves[old_index], old_index);
    }
    return std::make_pair(MoveLogProb(), old_index);
  }
  else {
    MoveLogProb m = (info->turn == BLACK) 
      ? tree[parent]->template nextMove<BLACK>() 
      : tree[parent]->template nextMove<WHITE>();
    if (m.validMove()) {
      assert(m.player() == info->turn);
      ++(info->move_index);
    }
    return std::make_pair(m, old_index);
  }
}

template <class EvalT>
size_t osl::search::
AlphaBeta2Parallel<EvalT>::checkmateCount() const
{
  return master->checkmateSearcher().totalNodeCount();
}

template <class EvalT>
size_t osl::search::
AlphaBeta2Parallel<EvalT>::mainCheckmateCount() const
{
  return master->checkmateSearcher().mainNodeCount();
}

/* ------------------------------------------------------------------------- */

template <class EvalT>
template <osl::Player P>
void osl::search::
AlphaBeta2Tree<EvalT>::testMoveRoot(int tree_id, const MoveLogProb& m)
{
  if (stop_tree) {
    std::cerr << "root tree stop\n";
    return;
  }
  
  Window w;
  AlphaBeta2ParallelCommon::TreeInfo *parent = shared->parent(tree_id);
  {
    SCOPED_LOCK(lk,parent->lock);
    w = parent->window;
    assert(w.isConsistent());
  }
  assert(P == m.player());
#ifndef GPSONE
  if (this->multi_pv) {
    int width = this->multi_pv*this->eval.captureValue(newPtypeO(P, PAWN))/200;
    if (width % 2 == 0) 
      width -= EvalTraits<P>::delta;
    w.alpha(P) = parent->search_value + width;
  }
#endif
  const int result = alphaBetaSearch<P>(m, w, false);

  if (eval::betterThan(P, result, parent->search_value)) {
    makePV(m.move());
    if (eval::betterThan(P, result, w.beta(P))) {
      {
	std::lock_guard<std::mutex> lk_smp(shared->lock_smp);
	SCOPED_LOCK(lk,parent->lock);
	if (! stop_tree) {
#if DEBUG_SMP > 2
	  std::cerr << "beta cut root " << tree_id << "\n";
#endif
	  for (int tid=0; tid<shared->max_threads; tid++)
	    if (parent->siblings[tid] && tid != shared->info[tree_id].thread_id)
	      shared->stopThread(parent->siblings[tid]);
	}
      }
      shared->parallel_abort.inc();
    }
    SCOPED_LOCK(lk,parent->lock);
    if (! stopping()
	&& (eval::betterThan(P, result, parent->search_value))) {
      assert(parent->window.isConsistent());
      parent->window.alpha(P) = result + EvalTraits<P>::delta;
      parent->best_move = m;
      parent->search_value = result;
      updateRootPV(P, std::cerr, result, m.move());
      assert(parent->window.isConsistent());
      shared->tree[shared->parentID(tree_id)]->pv[0] = pv[0];
    }
  }  
#ifndef GPSONE
  else if (this->multi_pv && !stopping() 
	   && eval::betterThan(P, result, w.alpha(P)))
    addMultiPV(P, result, m.move());
#endif
}

template <class EvalT>
template <osl::Player P>
void osl::search::AlphaBeta2Tree<EvalT>::
examineMovesRootPar(const MoveLogProbVector& moves, size_t start, Window window,
		    MoveLogProb& best_move, int& best_value)
{
  const int id = shared->treeId(this);
#if DEBUG_SMP > 3
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    std::cerr << "start split root " << id << " turn " << P << " parent " << shared->info[id].parent << "\n";
    history().dump();
  }
#endif
  AlphaBeta2ParallelCommon::TreeInfo *info = &shared->info[id];
  info->window = window;
  info->is_root = true;
  info->in_pv = false;
  info->value = best_value;
  info->moves = moves;
  info->move_index = start;
  info->turn = P;
  info->best_move = best_move;
  if (! shared->split(this, id, info->thread_id, -1)) {
    shared->cancelled_splits.inc();
    throw AlphaBeta2ParallelCommon::SplitFailed();
  }
  SCOPED_LOCK(lk,info->lock);
  best_value = info->search_value;
  best_move = info->best_move;
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::
examineMovesRootPar(int tree_id)
{
  AlphaBeta2ParallelCommon::TreeInfo *info = &shared->info[tree_id];
  const Player my_turn = info->turn;
  for (MoveLogProb m = shared->nextMove(tree_id).first; 
       m.validMove() && ! stopping();
       m = shared->nextMove(tree_id).first) {
#ifndef GPSONE
    if (this->elapsed() > 1.0)
    {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      for (const std::shared_ptr<SearchMonitor>& monitor:
		    this->monitors())
	monitor->rootMove(m.move());
    }
#endif
    try {
      if (my_turn == BLACK)
	testMoveRoot<BLACK>(tree_id, m);
      else
	testMoveRoot<WHITE>(tree_id, m);
      if (this->root_limit >= 1600)
	this->checkmate_searcher->runGC(this->table->isVerbose(),
					lastMemoryUseRatio1000());
    }
    catch (BetaCut& e) {
      std::cerr << "caught BetaCut at root " << info->thread_id << "\n";
      assert(stop_tree);
      break;
    }
    catch (std::runtime_error&) {
      stop_tree = true;
      this->stopNow();
      break;
    }
    catch (std::exception& e) {
#if DEBUG_SMP > 0
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "caught " << e.what() << " at root " << info->thread_id << "\n";
#endif
      stop_tree = true;
      this->stopNow();
      break;
    }
    catch (...) {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "caught something at root " << info->thread_id << "\n";
      stop_tree = true;
      this->stopNow();
      break;
    }
  }
  // cut or no more moves to search
}

template <class EvalT>
template <osl::Player P>
bool osl::search::
AlphaBeta2Tree<EvalT>::testMoveOther(int tree_id, const MoveLogProb& m, size_t index,
			      bool in_pv)
{
  if (stopping())
    return false;

  Window w;
  AlphaBeta2ParallelCommon::TreeInfo *parent = shared->parent(tree_id);
  {
    SCOPED_LOCK(lk,parent->lock);
    w = parent->window;
  }
  assert(w.isConsistent() || stop_tree);
  if (stopping())
    return false;
  assert(P == m.player());
  const int result = alphaBetaSearch<P>(m, w, in_pv);
  if (stopping())
    return false;

  bool cut = false;
  int parent_search_value;
  {
#ifdef OSL_USE_RACE_DETECTOR
    SCOPED_LOCK(lk,parent->lock);
#endif
    parent_search_value = parent->search_value;
  }
  if (eval::betterThan(P, result, parent_search_value)) {
    makePV(m.move());
    if (eval::betterThan(P, result, w.beta(P))) {
      cut = true;
      {
	std::lock_guard<std::mutex> lk_smp(shared->lock_smp);
	SCOPED_LOCK(lk,parent->lock);
	if (! stop_tree) {
#if DEBUG_SMP > 2
	  std::cerr << "beta cut " << tree_id << "\n";
#endif
	  for (int tid=0; tid<shared->max_threads; tid++)
	    if (parent->siblings[tid] && tid != shared->info[tree_id].thread_id)
	      shared->stopThread(parent->siblings[tid]);
	}
      }
      shared->parallel_abort.inc();
    }
    SCOPED_LOCK(lk,parent->lock);
    if (! stopping() && eval::betterThan(P, result, parent->search_value)) {
      parent->window.alpha(P) = result + EvalTraits<P>::delta;
      parent->best_move = m;
      parent->search_value = result;
      parent->alpha_update++;
      parent->last_alpha_update = index;
      assert(cut || shared->tree[shared->info[tree_id].parent]->stop_tree
	     || parent->window.isConsistent());
      AlphaBeta2Tree *pp = shared->tree[shared->parentID(tree_id)];
      pp->pv[pp->curDepth()] = pv[curDepth()];
      if (cut)
	return true;
    }
  }  
  return false;
}

template <class EvalT>
template <osl::Player P>
bool osl::search::AlphaBeta2Tree<EvalT>::
examineMovesOther(Window& w, MoveLogProb& best_move, int& best_value, 
		  int& tried_moves, int& alpha_update, int& last_alpha_update)
{
  assert(w.isConsistent());

  const int id = shared->treeId(this);
#if DEBUG_SMP > 3
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    std::cerr << "start split at " << curLimit() << " " << id << " turn " << P 
	      << " move " << tried_moves 
	      << " parent " << shared->info[id].parent << "\n";
    history().dump();
  }
#endif
  AlphaBeta2ParallelCommon::TreeInfo *info = &shared->info[id];
  info->window = w;
  info->is_root = false;
  info->in_pv = (! w.null()) && (! best_move.validMove());
  info->value = best_value;
  info->move_index = tried_moves;
  info->turn = P;
  info->best_move = best_move;
  info->alpha_update = alpha_update;
  info->last_alpha_update = last_alpha_update;
  if (! shared->split(this, id, info->thread_id, shared->max_thread_group)) {
#if DEBUG_SMP > 2
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    std::cerr << "failed split " << id << " turn " << P << "\n";
    for (int i=0; i<shared->max_threads; ++i) {
      std::cerr << "  " << i << " " << shared->job[i] << "\n";
    }
#endif
    shared->cancelled_splits.inc();
    throw AlphaBeta2ParallelCommon::SplitFailed();
  }
  SCOPED_LOCK(lk,info->lock);
  best_value = info->search_value;
  best_move = info->best_move;
  w = info->window;
  tried_moves = info->move_index;
  alpha_update += info->alpha_update;
  last_alpha_update = info->last_alpha_update;
#if DEBUG_SMP > 3
  {
    std::lock_guard<std::mutex> lk(OslConfig::lock_io);
    std::cerr << "back from split at " << curLimit() << " " << id << " turn " << P << " parent " << shared->info[id].parent << "\n";
  }
#endif
  testStop();
  return EvalTraits<P>::betterThan(best_value, w.beta(P));
}

template <class EvalT>
void osl::search::AlphaBeta2Tree<EvalT>::
examineMovesOther(int tree_id)
{
  AlphaBeta2ParallelCommon::TreeInfo *parent = shared->parent(tree_id);
  for (std::pair<MoveLogProb,size_t> m = shared->nextMove(tree_id); m.first.validMove() && !stopping(); 
       m = shared->nextMove(tree_id)) {
    bool in_pv = parent->in_pv;
    if (in_pv) {
      in_pv = ! parent->best_move.validMove();
    }
    assert(parent->turn == m.first.player());
    try {
      const bool cut_by_move =
	(parent->turn == BLACK)
	? testMoveOther<BLACK>(tree_id, m.first, m.second, in_pv)
	: testMoveOther<WHITE>(tree_id, m.first, m.second, in_pv);
      if (cut_by_move) {
	break;
      }
      testStop();
    }
    catch (BetaCut&) {
      assert(stop_tree);
    }
    catch (TableFull&) {
      stop_tree = true;
      this->stopNow();
      break;
    }
    catch (misc::NoMoreTime&) {
      stop_tree = true;
      this->stopNow();
#if DEBUG_SMP > 2
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "caught timeout in tree " << tree_id << " thread " << shared->info[tree_id].thread_id << "\n";
#endif
      break;
    }
    catch (NoMoreMemory&) {
      stop_tree = true;
      this->stopNow();
#if DEBUG_SMP > 2
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "caught memory full in tree " << tree_id << " thread " << shared->info[tree_id].thread_id << "\n";
#endif
      break;
    }
    catch (std::exception& e) {
      this->stopNow();
      stop_tree = true;
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "caught exception at " << tree_id << " " << e.what() << "\n";
      break;
    }
    catch (...) {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      std::cerr << "caught unknown exception at " << tree_id << "\n";
      throw;
    }
  }
  // cut or no more moves to search
}

namespace osl
{
  namespace search
  {
#ifndef MINIMAL
    template struct AlphaBeta2Parallel<eval::ProgressEval>;

    template 
    bool AlphaBeta2Tree<eval::ProgressEval>::examineMovesOther<BLACK>(Window&, MoveLogProb&, int&, int&, int&, int&);
    template 
    bool AlphaBeta2Tree<eval::ProgressEval>::examineMovesOther<WHITE>(Window&, MoveLogProb&, int&, int&, int&, int&);

    template
    void AlphaBeta2Tree<eval::ProgressEval>::examineMovesRootPar<BLACK>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
    template
    void AlphaBeta2Tree<eval::ProgressEval>::examineMovesRootPar<WHITE>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
#endif
    template struct AlphaBeta2Parallel<eval::ml::OpenMidEndingEval>;

    template 
    bool AlphaBeta2Tree<eval::ml::OpenMidEndingEval>::examineMovesOther<BLACK>(Window&, MoveLogProb&, int&, int&, int&, int&);
    template 
    bool AlphaBeta2Tree<eval::ml::OpenMidEndingEval>::examineMovesOther<WHITE>(Window&, MoveLogProb&, int&, int&, int&, int&);

    template
    void AlphaBeta2Tree<eval::ml::OpenMidEndingEval>::examineMovesRootPar<BLACK>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
    template
    void AlphaBeta2Tree<eval::ml::OpenMidEndingEval>::examineMovesRootPar<WHITE>(const MoveLogProbVector&, size_t, Window, MoveLogProb&, int&);
  }
}

#endif /* OSL_SMP */
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
