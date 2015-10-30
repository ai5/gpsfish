/* dfpnParallel.cc
 */
#include "osl/checkmate/dfpnParallel.h"
#include "osl/checkmate/proofTreeDepthDfpn.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <thread>

#ifdef OSL_DFPN_SMP
osl::checkmate::
DfpnParallel::DfpnParallel(size_t threads)
  : table(0), num_threads(threads), state(0)
{
  if (threads == 0)
    num_threads = OslConfig::concurrency();
  workers.reset(new Dfpn[num_threads]);
  for (size_t i=0; i<num_threads; ++i)
    workers[i].setParallel(i, &shared);
}

osl::checkmate::
DfpnParallel::~DfpnParallel()
{
#ifdef DFPN_DEBUG
  table->testTable();
#endif
}

void osl::checkmate::
DfpnParallel::setTable(DfpnTable *new_table) 
{ 
  table = new_table; 
  for (size_t i=0; i<num_threads; ++i)
    workers[i].setTable(table);
}

const osl::checkmate::ProofDisproof 
osl::checkmate::
DfpnParallel::hasCheckmateMove(const NumEffectState& state, const HashKey& key,
			  const PathEncoding& path, size_t limit, Move& best_move,
			       Move last_move, std::vector<Move> *pv)
{
  PieceStand proof;
  return hasCheckmateMove(state, key, path, limit, best_move, proof, last_move, pv);
}

struct osl::checkmate::DfpnParallel::AttackWorker
{
  DfpnParallel *parent;
  int thread_id;
  
  AttackWorker(DfpnParallel *p, int id) 
    : parent(p), thread_id(id)
  {
  }
  void operator()() const 
  {
    assert(! parent->shared.data[thread_id].restart);
    WorkerData& work = parent->worker_data[thread_id];
    work.result = parent->workers[thread_id].hasCheckmateMove
      (*(parent->state), parent->key, parent->path,
       parent->limit, work.best_move, work.proof, parent->last_move);
    parent->workers[thread_id].clear();
  }
};

const osl::checkmate::ProofDisproof 
osl::checkmate::
DfpnParallel::hasCheckmateMove(const NumEffectState& state, const HashKey& key,
			  const PathEncoding& path, size_t limit, Move& best_move, PieceStand& proof,
			       Move last_move, std::vector<Move> *pv)
{
  this->state = &state;
  this->key = key;
  this->path = path;
  this->last_move = last_move;
  this->limit = limit;
  shared.clear();
  worker_data.reset(new WorkerData[num_threads]);
  boost::ptr_vector<std::thread> threads;
  for (size_t i=0; i<num_threads; ++i)
    threads.push_back(new std::thread(AttackWorker(this, i)));
  ProofDisproof ret;
  unsigned int min_proof = ProofDisproof::PROOF_MAX;
  for (size_t i=0; i<num_threads; ++i) {
    threads[i].join();
    if (ret.isFinal()
	|| (worker_data[i].result.proof() >= min_proof 
	    && ! worker_data[i].result.isFinal()))
      continue;    
    ret = worker_data[i].result;
    min_proof = ret.proof();
    best_move = worker_data[i].best_move;
    proof = worker_data[i].proof;
  }
  if (pv && ret.isCheckmateSuccess()) {
    ProofTreeDepthDfpn analyzer(*table);
    analyzer.retrievePV(state, true, *pv);
  }
  return ret;
}

struct osl::checkmate::DfpnParallel::DefenseWorker
{
  DfpnParallel *parent;
  int thread_id;
  
  DefenseWorker(DfpnParallel *p, int id) 
    : parent(p), thread_id(id)
  {
  }
  void operator()() const 
  {
    WorkerData& work = parent->worker_data[thread_id];
    work.result = parent->workers[thread_id].hasEscapeMove
      (*(parent->state), parent->key, parent->path,
       parent->limit, parent->last_move);
  }
};

const osl::checkmate::ProofDisproof
osl::checkmate::
DfpnParallel::hasEscapeMove(const NumEffectState& state, 
		       const HashKey& key, const PathEncoding& path, 
		       size_t limit, Move last_move)
{
  this->state = &state;
  this->key = key;
  this->path = path;
  this->last_move = last_move;
  this->limit = limit;
  shared.clear();
  worker_data.reset(new WorkerData[num_threads]);
  boost::ptr_vector<std::thread> threads;
  for (size_t i=0; i<num_threads; ++i)
    threads.push_back(new std::thread(DefenseWorker(this, i)));
  ProofDisproof ret;
  unsigned int min_disproof = ProofDisproof::DISPROOF_MAX;
  for (size_t i=0; i<num_threads; ++i) {
    threads[i].join();
    if (worker_data[i].result.disproof() >= min_disproof) 
      continue;
    ret = worker_data[i].result;
    min_disproof = ret.disproof();
  }   
  return ret;
}

size_t osl::checkmate::
DfpnParallel::nodeCount() const
{
  size_t sum = 0;
  for (size_t i=0; i<num_threads; ++i)
    sum += workers[i].nodeCount();
  return sum;
}

#ifndef MINIMAL
void osl::checkmate::
DfpnParallel::analyze(const PathEncoding& path,
		      const NumEffectState& src, const std::vector<Move>& moves) const
{
  if (num_threads > 0)
    workers[0].analyze(path, src, moves);
}
#endif
#endif

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
