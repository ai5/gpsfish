/* ntesukiSearcher.tcc
 */
#include "osl/ntesuki/ntesukiSearcher.h"
#include "osl/ntesuki/ntesukiMove.h"
#include "osl/ntesuki/ntesukiMoveList.h"
#include "osl/ntesuki/ntesukiSimulationSearcher.h"
#include "osl/apply_move/applyMoveWithPath.h"
#include "osl/effect_util/effectUtil.h"
// for release, inline the following templates as well
#ifdef NDEBUG
# include "osl/ntesuki/ntesukiMove.tcc"
# include "osl/ntesuki/ntesukiRecord.tcc"
#endif

#include "osl/record/csaRecord.h"
#include <climits>
#include <list>
#include <algorithm>

using namespace osl;
using namespace osl::ntesuki;

/* 409600 nodes at root node for MTDF
 * (/ 40960 8) => 5120
 *  cf. 1600 for brinkmate searcher
 */
const int READ_ATTACK_BACK_LIMIT = 5120;

#ifndef NDEBUG
#define RETURN(val)							\
  if (record->getValueWithPath<A>(pass_left, path).proof() == 0)\
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path).disproof() > ProofDisproof::DISPROOF_LIMIT);\
  if (record->getValueWithPath<A>(pass_left, path).disproof() == 0)\
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path).proof() > ProofDisproof::PROOF_LIMIT);\
  ntesuki_assert(val.isFinal() == record->getValueWithPath<A>(pass_left, path).isFinal());\
  return val
#else
#define RETURN(val) return val
#endif

#define RETURN_ON_STOP \
  if (node_count > read_node_limit || *stop_flag)\
    return

/* ===================
 * misc
 */
static
unsigned int addWithSaturation(unsigned int limit, 
			       unsigned int l, unsigned int r)
{
  if (limit < l)
    return limit;
  const unsigned int sum = l+r;
  if (limit < sum)
    return limit;
  // guard against overflow
  if (sum < l)
    return limit;
  ntesuki_assert(r <= sum);
  return sum;
}

struct PlayMoveLock
{
  std::vector<Move>& mlist;

  PlayMoveLock(std::vector<Move>& l,
	       const osl::Move& m)
    : mlist(l)
  {
    mlist.push_back(m);
  }

  ~PlayMoveLock()
  {
    mlist.pop_back();
  }
};

struct LockGC
{
  osl::ntesuki::NtesukiTable& table;
  LockGC(osl::ntesuki::NtesukiTable& t)
    : table(t)
  {
    table.lockGC();
  }

  ~LockGC()
  {
    table.unlockGC();
  }
};

/* ===================
 *  Attack helper
 */
template <class Search,Player T>
class 
osl::ntesuki::NtesukiSearcher::
AttackHelper
{
  unsigned int proof_limit, disproof_limit, pass_left;
  Search* searcher;
  NtesukiResult& result;
  NtesukiRecord* record;
  const NtesukiRecord* oracle_attack;
  const NtesukiRecord* oracle_defense;
  const Move last_move;
public:
  AttackHelper(Search* searcher,
	       NtesukiResult& result,
	       NtesukiRecord* record,
	       const NtesukiRecord* oracle_attack,
	       const NtesukiRecord* oracle_defense,
	       unsigned int proof_limit,
	       unsigned int disproof_limit,
	       unsigned int pass_left,
	       const Move last_move)
    : proof_limit(proof_limit), disproof_limit(disproof_limit),
      pass_left(pass_left),
      searcher(searcher),  result(result), record(record),
      oracle_attack(oracle_attack), oracle_defense(oracle_defense),
      last_move(last_move)
  {
  }
	
  void operator()(Square last_to)
  {
    result = (*searcher).template defense<PlayerTraits<T>::opponent>
      (record, oracle_attack, oracle_defense,
       proof_limit, disproof_limit, pass_left, last_move);
  }
}; // AttackHelper

/* ===================
 *  Call Simulation Attack
 */
template <class Search,Player T>
class 
osl::ntesuki::NtesukiSearcher::
CallSimulationAttack
{
  Search &simulator;
  NtesukiTable& table;
  NtesukiRecord *record;
  const NtesukiRecord *record_orig;
  unsigned int pass_left;
  bool& simulation_result;
  const Move last_move;

public:
  CallSimulationAttack(Search& simulator,
		       NtesukiTable& table,
		       NtesukiRecord *record,
		       const NtesukiRecord *record_orig,
		       unsigned int pass_left,
		       bool& simulation_result,
		       const Move last_move)
    : simulator(simulator), table(table),
      record(record), record_orig(record_orig),
      pass_left(pass_left),
      simulation_result(simulation_result),
      last_move(last_move)
  {
  }
	
  void operator()(Square last_to)
  {
    LockGC glock(table);
    simulation_result = simulator.template
      startFromDefenseDisproof<PlayerTraits<T>::opponent>
      (record, record_orig, pass_left, last_move);
  }
}; // Call Simulation Attack

/* ===================
  *  Defense helper
  */
template <class Search,Player T>
class
osl::ntesuki::NtesukiSearcher::
DefenseHelper
{
  unsigned int proof_limit, disproof_limit, pass_left;
  Search* searcher;
  NtesukiResult& result;
  NtesukiRecord* record;
  const NtesukiRecord* oracle_attack;
  const NtesukiRecord* oracle_defense;
  const Move last_move;

public:
  DefenseHelper(Search* searcher,
		NtesukiResult& result,
		NtesukiRecord* record,
		const NtesukiRecord* oracle_attack,
		const NtesukiRecord* oracle_defense,
		unsigned int proof_limit,
		unsigned int disproof_limit,
		unsigned int pass_left,
		const Move last_move)
    : proof_limit(proof_limit), disproof_limit(disproof_limit),
      pass_left(pass_left), searcher(searcher), result(result), record(record),
      oracle_attack(oracle_attack), oracle_defense(oracle_defense),
      last_move(last_move)
  {
  }

  void operator()(Square p)
  {
    (*searcher).template attack<PlayerTraits<T>::opponent>
      (record,  oracle_attack, oracle_defense,
       proof_limit, disproof_limit,
       pass_left, last_move);
  }
};// DefenseHelper

/* ===================
 *  Call Simulation Defense
 */
template <class Search,Player T>
class 
osl::ntesuki::NtesukiSearcher::
CallSimulationDefense
{
  Search &simulator;
  NtesukiTable &table;
  NtesukiRecord *record;
  const NtesukiRecord *record_orig;
  unsigned int pass_left;
  bool& simulation_result;
  const Move last_move;
public:
  CallSimulationDefense(Search& simulator,
			NtesukiTable& table,
			NtesukiRecord *record,
			const NtesukiRecord *record_orig,
			unsigned int pass_left,
			bool& simulation_result,
			const Move last_move)
    : simulator(simulator), table(table),
      record(record), record_orig(record_orig),
      pass_left(pass_left),
      simulation_result(simulation_result),
      last_move(last_move)
  {
  }
	
  void operator()(Square last_to)
  {
    LockGC glock(table);
    simulation_result = simulator.template
      startFromAttackProof<PlayerTraits<T>::opponent>
      (record, record_orig, pass_left, last_move);
  }
}; // Call Simulation Defense

/* ===================
 *  Call Simulation Defense Disproof
 */
template <class Search,Player T>
class 
osl::ntesuki::NtesukiSearcher::
CallSimulationDefenseDisproof
{
  Search &simulator;
  NtesukiTable &table;
  NtesukiRecord *record;
  const NtesukiRecord *record_orig;
  unsigned int pass_left;
  bool& simulation_result;
  const Move last_move;
public:
  CallSimulationDefenseDisproof(Search& simulator,
				NtesukiTable& table,
				NtesukiRecord *record,
				const NtesukiRecord *record_orig,
				unsigned int pass_left,
				bool& simulation_result,
				const Move last_move)
    : simulator(simulator), table(table),
      record(record), record_orig(record_orig),
      pass_left(pass_left),
      simulation_result(simulation_result),
      last_move(last_move)
  {
  }
	
  void operator()(Square last_to)
  {
    LockGC glock(table);
    simulation_result = simulator.template
      startFromAttackDisproof<PlayerTraits<T>::opponent>
      (record, record_orig, pass_left, last_move);
  }
}; // Call Simulation Defense Disproof

template <Player T>
void osl::ntesuki::NtesukiSearcher::
simulateSiblingsFail(NtesukiRecord *record,
		     NtesukiRecord *record_best,
		     int pass_left,
		     unsigned int& success_count,
		     unsigned int& total_count)
{
  LockGC glock(table);

  const Player A = T;
  if (!record_best) return;
  ntesuki_assert(record_best);

  NtesukiMoveList moves;
  mg->generate<T>(state, moves);

  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); move_it++)
  {
    NtesukiMove& move = *move_it;
    NtesukiRecord *record_child = table.allocateWithMove(record,
							 move);
    if (record_child == 0)
    {
      *stop_flag = TableLimitReached;
      return;
    }
    ntesuki_assert(record_child);
    if (record_child == record_best) continue;
    if (record_child->isVisited()) continue;
    if (move.isCheckmateFail<A>(pass_left)) continue;
    const PathEncoding path_child(path, move.getMove());
    const NtesukiResult result_child = record_child->getValueWithPath<A>(pass_left,
									 path_child);
    if (result_child.isFinal())
    {
      continue;
    }

    bool simulation_result;
    total_count++;
    CallSimulationAttack<NtesukiSimulationSearcher, T>
      helper(simulator, table, record_child, record_best,
	     pass_left, simulation_result, move.getMove());
    TRY_DFPN;
    ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, move.getMove(), helper);
    CATCH_DFPN;
    RETURN_ON_STOP;

    if (simulation_result)
    {
      success_count++;
      ntesuki_assert(record_child->getValueWithPath<A>(pass_left,
						       path_child).isCheckmateFail());
      TRY_DFPN;
      move.setBySimulation();
      move.setCheckmateFail<A>(pass_left);
      CATCH_DFPN;
    }
  }
}

/* ===================
 * Count the increase of child nodes
 */
class CountChildLock
{
public:
  CountChildLock(NtesukiRecord* r,
		 const NtesukiTable& t)
    : record(r), table(t)
  {
    size_start = table.size();
  }

  ~CountChildLock()
  {
    record->addChildCount(table.size() - size_start);
  }
private:
  osl::ntesuki::NtesukiRecord* record;
  const osl::ntesuki::NtesukiTable& table;
  unsigned int size_start;
};


/* ===================
 * Attack
 */
template <Player T>
NtesukiResult osl::ntesuki::NtesukiSearcher::
attack(NtesukiRecord* record,
       const NtesukiRecord* oracle_attack,
       const NtesukiRecord* oracle_defense,
       unsigned int proof_limit, unsigned int disproof_limit,
       const int pass_left, const Move last_move)
{
  CountChildLock cclock(record, table);
  const Player A = T;
#ifndef NDEBUG
  const Player D = PlayerTraits<T>::opponent;
#endif

  ntesuki_assert(T == state.turn());
  ntesuki_assert(!state.inCheck(D));
  ntesuki_assert(proof_limit < ProofDisproof::PROOF_LIMIT);
  ntesuki_assert(disproof_limit < ProofDisproof::DISPROOF_LIMIT);
  ntesuki_assert(record->getValueOr<A>(pass_left, path, iwscheme).proof()
		 < proof_limit);
  ntesuki_assert(record->getValueOr<A>(pass_left, path, iwscheme).disproof()
		 < disproof_limit);

  RETURN_ON_STOP (record->getValueOr<A>(pass_left, path, iwscheme));

  ntesuki_assert(record->getValueOr<A>(pass_left, path, iwscheme).proof()
		 < proof_limit);
  ntesuki_assert(record->getValueOr<A>(pass_left, path, iwscheme).disproof()
		 < disproof_limit);
  ntesuki_assert(record->getBestMove<A>(pass_left).isInvalid());
  
  ntesuki_assert(proof_limit > 0);
  ntesuki_assert(disproof_limit > 0);
  
  /* ノードの初期化． 必要なら FixedDepthSearcher も呼ばれる．
   */
  TRY_DFPN;
  if (record->setUpNode<T>())
  {
    const NtesukiResult result_cur =
      record->getValueWithPath<A>(pass_left, path);
    if (result_cur.isCheckmateSuccess())
    {
      /* By fixed searcher */
      ++immediate_win;
      RETURN (result_cur);
    }
    else if (result_cur.isCheckmateFail() && pass_left == 0)
    {
      RETURN (result_cur);
    }
  }
  CATCH_DFPN;

  /* Iterative Wideningc Scheme に応じて探索
   */
  NtesukiResult result_cur = record->getValueOr<A>(pass_left, path, iwscheme);
  while ((result_cur.proof() < proof_limit) &&
	 (result_cur.disproof() < disproof_limit))
  {
    if (iwscheme == NtesukiRecord::no_iw)
    {
      /* Order は変えない
       */
      TRY_DFPN;
      attackWithOrder<T>(record, NULL, NULL,
			 proof_limit, disproof_limit,
			 pass_left, last_move);
      
      CATCH_DFPN;
      RETURN_ON_STOP (result_cur);
    }// no_iw
    else if (iwscheme == NtesukiRecord::strict_iw)
    {
      /* 必ず小さい方から探索
       */
      for (int pass_left_child = 0; pass_left_child <= pass_left; pass_left_child++)
      {
	NtesukiResult result_st = record->getValueWithPath<A>(pass_left_child, path);
	if (!result_st.isCheckmateFail())
	{
	  ntesuki_assert(result_st.proof() < proof_limit);
	  ntesuki_assert(result_st.disproof() < disproof_limit);
	  TRY_DFPN;      
	  attackWithOrder<T>(record, NULL, NULL,
			     proof_limit, disproof_limit,
			     pass_left_child, last_move);
	  
	  CATCH_DFPN;
	  RETURN_ON_STOP (result_cur);
	  break;
	}
      }
    }// strict_iw
    else if (iwscheme == NtesukiRecord::pn_iw)
    {
      /* proof number の小さい方から探索
       */
      unsigned int p_min = ProofDisproof::BigProofNumber,
	p_2nd = ProofDisproof::BigProofNumber;
      int pass_left_best = -1;
      for (int pass_left_child = 0; pass_left_child <= pass_left; pass_left_child++)
      {
	NtesukiResult result_st = record->getValueWithPath<A>(pass_left_child, path);
	if (result_st.isCheckmateFail()) continue;
	
	const unsigned int proof = result_st.proof();
	if (proof < p_min)
	{
	  pass_left_best = pass_left_child;
	  p_2nd = p_min;
	  p_min = proof;
	}
	else if (proof < p_2nd)
	{
	  p_2nd = proof;
	}
      }

      unsigned int proof_limit_child = std::min(proof_limit, p_2nd + 1);
      unsigned int disproof_limit_child = disproof_limit;
      TRY_DFPN;      
      attackWithOrder<T>(record, NULL, NULL,
			 proof_limit_child, disproof_limit_child,
			 pass_left_best, last_move);
      
      CATCH_DFPN;
      RETURN_ON_STOP (result_cur);
    }// pn_iw
    result_cur = record->getValueOr<A>(pass_left, path, iwscheme);
  }
  return result_cur;
}

template <Player T>
void osl::ntesuki::NtesukiSearcher::
attackWithOrder(NtesukiRecord* record,
		const NtesukiRecord* oracle_attack,
		const NtesukiRecord* oracle_defense,
		unsigned int proof_limit, unsigned int disproof_limit,
		const int pass_left, const Move last_move)
{
  ++node_count;

  const Player A = T;
#ifndef NDEBUG
  const Player D = PlayerTraits<T>::opponent;
#endif
  ntesuki_assert(T == state.turn());
  ntesuki_assert(!state.inCheck(D));
  ntesuki_assert(proof_limit < ProofDisproof::PROOF_LIMIT);
  ntesuki_assert(disproof_limit < ProofDisproof::DISPROOF_LIMIT);

  RETURN_ON_STOP;
  const bool under_attack = state.inCheck(T);

  ntesuki_assert (record->getValueWithPath<A>(pass_left, path).proof()
		  < proof_limit);
  ntesuki_assert (record->getValueWithPath<A>(pass_left, path).disproof()
		  < disproof_limit);
  ntesuki_assert(record->getBestMove<A>(pass_left).isInvalid());
  
  NtesukiRecord::VisitLock visitLock(record);

  ntesuki_assert(proof_limit > 0);
  ntesuki_assert(disproof_limit > 0);
  
  NtesukiMoveList moves;
  /* 手の生成
   */
  TRY_DFPN;
  record->generateMoves<T>(moves, pass_left, false);
  CATCH_DFPN;

  /* collect statistic information */
  ++attack_node_count;
  if (under_attack)
  {
    ++attack_node_under_attack_count;
  }
  attack_node_moves_count += moves.size();

  /* 攻め手がなくなる→失敗 */
  if (moves.empty())
  {
    if (pass_left != 0 &&
	record->rzone_move_generation)
    {	
      /* まだ未生成の手がある */
      NtesukiResult r = record->getValueWithPath<A>(pass_left - 1, path);
      
      if (r.isCheckmateFail())
      {
	/* rzone はこれ以上は拡大しない */
	record->rzone_move_generation = false;
	TRY_DFPN;
	record->setResult<A>(pass_left, ProofDisproof(1,1),
			     NtesukiMove::INVALID(), false);
	CATCH_DFPN;
      }
      else
      {
	/* rzone が増えるまで，より低い order での探索を優先させる */
	TRY_DFPN;
	record->setResult<A>(pass_left, ProofDisproof(r.proof() + 2,
						      r.disproof() + 2),
			     NtesukiMove::INVALID(), false);
	CATCH_DFPN;
      }
      return;
    }
    TRY_DFPN;
    /* 双玉問題なら D の NoEscape だが，
     * 片玉問題で，全合法手を生成しない場合(pass_left==0 の場合など)では
     * ただの NoCheckmate */
    record->setResult<A>(pass_left, ProofDisproof::NoCheckmate(),
			 NtesukiMove::INVALID(), false);
    CATCH_DFPN;
    return;
  }

  ntesuki_assert(!moves.empty());
  ntesuki_assert(record->getValueWithPath<A>(pass_left, path).proof() != 0);
  ntesuki_assert(record->getValueWithPath<A>(pass_left, path).disproof() != 0);

  /* 攻める手の実行
   */
  for (;;)
  {
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path).proof() != 0);
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path).disproof() != 0);

    unsigned int best_proof = ProofDisproof::BigProofNumber,
      best_disproof = 0,
      second_proof = ProofDisproof::BigProofNumber,
      sum_disproof = 0;
    unsigned int step_cost = 1;

    NtesukiMove* best_move =
      selectMoveAttack<T>(record,
			  best_proof, sum_disproof,
			  second_proof, best_disproof,
			  step_cost,
			  moves,
			  pass_left);
    if (best_move == NULL)
    {
      if (pass_left != 0 &&
	  record->rzone_move_generation)
      {	
	/* まだ未生成の手がある */
	NtesukiResult r = record->getValueWithPath<A>(pass_left - 1, path);
	
	if (r.isCheckmateFail())
	{
	  /* rzone はこれ以上は拡大しない */
	  record->rzone_move_generation = false;
	  TRY_DFPN;
	  record->setResult<A>(pass_left, ProofDisproof(1,1),
			       NtesukiMove::INVALID(), false);
	  CATCH_DFPN;
	}
	else
	{
	  /* rzone が増えるまで，より低い order での探索を優先させる */
	  TRY_DFPN;
	  record->setResult<A>(pass_left, ProofDisproof(r.proof() + 2,
							r.disproof() + 2),
			       NtesukiMove::INVALID(), false);
	  CATCH_DFPN;
	}
      }
      else
      {
	ntesuki_assert(record->getValueWithPath<A>(pass_left, path).disproof() == 0);
      }
      return;
    }
    else if (best_move->isCheckmateSuccess<A>(pass_left))
    {
      return;
    }

    /* このノードの証明数・反証数を設定する
     */
    const NtesukiResult result_cur(best_proof, sum_disproof);
    record->setResult<A>(pass_left, result_cur,
			 NtesukiMove::INVALID(), false);

    /* 他を読む
     */
    if ((proof_limit <= best_proof) ||
	(disproof_limit <= sum_disproof))
    {
      ntesuki_assert(!result_cur.isFinal());
      return;
    }

    /* 手を試す
     */
    unsigned int proof_child = std::min(proof_limit, second_proof + step_cost);
    ntesuki_assert(disproof_limit > sum_disproof);// disproof_child
    unsigned int disproof_child =
      addWithSaturation(ProofDisproof::DISPROOF_LIMIT,
			disproof_limit, best_disproof)
      - sum_disproof;

    NtesukiRecord *record_child = table.allocateWithMove(record, *best_move);
    if (record_child == 0)
    {
      *stop_flag = TableLimitReached;
      return;
    }
    ntesuki_assert(record_child);
    const PathEncoding path_child(path, best_move->getMove());
    NtesukiResult result_child = record_child->getValueWithPath<A>(pass_left,
								   path_child);
    if (!result_child.isFinal())
    {
      if (best_move->isCheck())
      {
	oracle_attack = NULL;
      }
      if (ptt_aunt &&
	  pass_left != 0 &&
	  record->getValueWithPath<A>(pass_left - 1, path).isCheckmateFail())
      {
	oracle_defense = record;
      }

      AttackHelper<NtesukiSearcher, T> helper(this,
					      result_child,
					      record_child,
					      oracle_attack,
					      oracle_defense,
					      proof_child,
					      disproof_child,
					      pass_left,
					      best_move->getMove());
      PlayMoveLock pml(moves_played, best_move->getMove());
      TRY_DFPN;
      ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, best_move->getMove(), helper);
      CATCH_DFPN;
      record->updateWithChild(record_child, pass_left);
      RETURN_ON_STOP;

      const NtesukiResult result_cur = record->getValueWithPath<A>(pass_left, path_child);
      if (result_cur.isFinal())
      {
	return;
      }
    }

    if (result_child.isPawnDropFoul(best_move->getMove()))
    {
      best_move->setPawnDropCheckmate();
      best_move->setCheckmateFail<A>(pass_left);
    }
    else if (result_child.isCheckmateSuccess())
    {
      best_move->setCheckmateSuccess<A>(pass_left);
      TRY_DFPN;
      record->setResult<A>(pass_left, ProofDisproof::Checkmate(),
			   *best_move, false);
      CATCH_DFPN;
      return;
    }
    else if (result_child.isCheckmateFail())
    {
      if (result_child != ProofDisproof::LoopDetection())
      {
	best_move->setCheckmateFail<A>(pass_left);
      }

      if (result_child == ProofDisproof::PawnCheckmate())
      {
	best_move->setPawnDropCheckmate();
      }

      if (ptt_siblings_fail &&
	  !best_move->isCheck() &&
	  result_child != ProofDisproof::LoopDetection())
      {
	TRY_DFPN;
	simulateSiblingsFail<T>(record, record_child, pass_left,
				sibling_attack_success_count,
				sibling_attack_count);
	CATCH_DFPN;
      }
    }
  }//for(;;)
}

/* dynamic widening */
typedef std::pair<unsigned int, unsigned int> top_pdp_t;
static bool sorter(const top_pdp_t& lhs,
		   const top_pdp_t& rhs)
{
  if (lhs.first == rhs.first)
  {
    //return rhs.second > rhs.second;
    return rhs.second < rhs.second;
  }
  else
  {
    return lhs.first > rhs.first;
  }
}

template <Player T>
osl::ntesuki::NtesukiMove * osl::ntesuki::NtesukiSearcher::
selectMoveAttack(NtesukiRecord* record,
		 unsigned int& best_proof,
		 unsigned int& sum_disproof,
		 unsigned int& second_proof,
		 unsigned int& best_disproof,
		 unsigned int& step_cost,
		 NtesukiMoveList& moves,
		 const int pass_left)
{
  const Player A = T;

  bool read_nopromote = false;

 re_select_move_attack:
  NtesukiMove *best_move = NULL;
  
  bool loop = false;
  bool pawn_checkmate = false;
  unsigned short min_child_age = SHRT_MAX;

  int average_cost = 0;
  int average_cost_count = 0;

  /* reset values */
  best_proof = ProofDisproof::BigProofNumber;
  best_disproof = 0;
  second_proof = ProofDisproof::BigProofNumber;
  sum_disproof = 0;

  /* dynamic Widening */
  std::list<top_pdp_t> pdps;

  /* 手を選ぶ
   */
  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); ++move_it)
  {
    NtesukiMove& move = *move_it;
    pawn_checkmate |= move.isPawnDropCheckmate();

    if (move.isPass())
    {
      continue;
    }
    
    if (!move.isCheck() && 0 == pass_left)
    {
      continue;
    }

    if (move.isCheckmateFail<A>(pass_left))
    {
      continue;
    }

    if (delay_nopromote &&
	!read_nopromote &&
	move.isNoPromote())
    {
      continue;
    }

    if (delay_non_attack &&
	!record->readNonAttack(pass_left) &&
	((move.getOrder() > pass_left) ||
	 move.isLameLong())
	)
    {
      continue;
    }
    
    unsigned int proof = move.h_a_proof;
    unsigned int disproof = move.h_a_disproof;

    if (tsumero_estimate && !move.isCheck())
    {
      proof = tsumero_estimate;
      disproof = 1;
    }
    
    average_cost += proof;
    average_cost_count++;

    NtesukiRecord *record_child = table.findWithMove(record, move);
    if (record_child)
    {
      if (record_child->isVisited())
      {
	/* ループ発見 */
	/* defender の方からの王手千日手の可能性アリ
	 */
	loop = true;
	continue;
      }
      
      const PathEncoding path_child(path, move.getMove());
      NtesukiResult result_child;
      TRY_DFPN;
      result_child =
	record_child->getValueAnd<A>(pass_left, path_child,
				     iwscheme, psscheme);
      CATCH_DFPN;
      proof = result_child.proof();
      disproof = result_child.disproof();
      
      if (0 == disproof)/* 不詰み */
      {
	if (result_child == ProofDisproof::LoopDetection())
	{
	  loop = true;
	  continue;
	}
	if (record_child->getValueWithPath<A>(pass_left, path_child) ==
	    ProofDisproof::PawnCheckmate())
	{
	  move.setPawnDropCheckmate();
	  pawn_checkmate = true;
	}
	
	ntesuki_assert(proof >= ProofDisproof::PROOF_LIMIT);
	move.setCheckmateFail<A>(pass_left);

	/* should do ptt_siblings_fail */
	continue;
      }
      else if (0 == proof)/* 詰み */
      {
	/* 打歩詰めだけチェック */
	if (record_child->getValueWithPath<A>(pass_left, path_child).
	    isPawnDropFoul(move.getMove()))
	{
	  move.setPawnDropCheckmate();
	  move.setCheckmateFail<A>(pass_left);
	  pawn_checkmate = true;
	  continue;
	}

	/* 間違いなく勝ち手 */
	ntesuki_assert(disproof >= ProofDisproof::DISPROOF_LIMIT);
	move.setCheckmateSuccess<A>(pass_left);
	TRY_DFPN;
	record->setResult<A>(pass_left, ProofDisproof::Checkmate(),
			     move, false);
	CATCH_DFPN;
	
	return &move;
      }

      min_child_age = std::min(record_child->distance,
			       min_child_age);
      if (record_child->distance <= record->distance)
      {
	if (!record->useOld<A>(pass_left))
	{
	  continue;
	}
      }

      /* the assertion:
       * use_old == (record_child->distance <= record->distance)
       * does not hold, as the distance gets updated to youngest.
       */
    }
    
    /* Proof Disproof の調整はここで */
    if (!move.isCheck())
    {
      ntesuki_assert(pass_left > 0);
      proof += tsumero_cost;
      disproof += tsumero_cost;
    }

    if (!record->useOld<A>(pass_left)
	&& !(NtesukiRecord::max_for_split && record->is_split))
    {
      sum_disproof = addWithSaturation(ProofDisproof::DISPROOF_LIMIT,
				       disproof, sum_disproof);
    }
    else
    {
      sum_disproof = std::max(disproof, sum_disproof);
    }

    /* best move の設定 */
    if (proof < best_proof)
    {
      best_move = &move;
      second_proof = best_proof;
      best_proof = proof;
      best_disproof = disproof;
    }
    else if (proof < second_proof)
    {
      second_proof = proof;
    }

    /* dynamic widening: 良い手の選定 */
    if (dynamic_widening_width > 0)
    {
      if (pdps.size() < dynamic_widening_width)
      {
	pdps.push_back(top_pdp_t(proof, disproof));
	//Sorter sorter;
	pdps.sort(sorter);
      }
      else
      {
	if (pdps.back().first > proof)
	{
	  pdps.pop_back();
	  pdps.push_back(top_pdp_t(proof, disproof));
	}// back().proof == proof だった場合は？
	pdps.sort(sorter);
      }
    }
  }

  /* dynamic widening: 良い手の集計 */
  if ((dynamic_widening_width > 0 &&
       dynamic_widening_width < moves.size())
      &&
      (!record->useOld<A>(pass_left)
       && !(NtesukiRecord::max_for_split && record->is_split)))
  {
    sum_disproof = 0;
    for (std::list<top_pdp_t>::const_iterator it = pdps.begin();
	 it != pdps.end(); ++it)
    {
      sum_disproof += it->second;
    }
  }

  /* 選んだ手を吟味する
   */
  if (best_move == NULL)
  {
    if (false == record->useOld<A>(pass_left))
    {
      if (SHRT_MAX != min_child_age)
      {
	record->setUseOld<A>(pass_left, true);

	ntesuki_assert(min_child_age <= record->distance);
	record->distance = min_child_age;

	goto re_select_move_attack;
      }
    }

    if (!record->readNonAttack(pass_left))
    {
      if (!read_attack_only)
      {
	record->setReadNonAttack(pass_left);
	if (ptt_non_attack &&
	    pass_left != 0)
	{
	  TRY_DFPN;
	  handleNonAttack<T>(record, pass_left);
	  CATCH_DFPN;
	  RETURN_ON_STOP NULL;
	}
	record->setUseOld<A>(pass_left, false);
	goto re_select_move_attack;
      }
    }

    if (delay_nopromote &&
	!read_nopromote &&
	(pass_left > 0 || pawn_checkmate))
    {
      read_nopromote = true;
      record->setUseOld<A>(pass_left, false);
      goto re_select_move_attack;
    }

    ntesuki_assert(best_proof == ProofDisproof::BigProofNumber);

    if (pass_left != 0 &&
	record->rzone_move_generation)
    {
      /* まだ未生成の手がある */
      return NULL;
    }
    /* 本当に disproof されてしまった */
    if (pawn_checkmate)/* 打歩詰めがあった */
    {
      if (delay_nopromote) assert(read_nopromote);

      TRY_DFPN;
      record->setResult<A>(pass_left, ProofDisproof::PawnCheckmate(),
			   NtesukiMove::INVALID(), false);
      CATCH_DFPN;
      return NULL;
    }
    // pawn checkmate and loop??
    else if (loop) /* ループがあった */
    {
      record->setLoopWithPath<A>(pass_left, path);
      TRY_DFPN;
      record->setResult<A>(pass_left, NtesukiResult(1, 1),
			   NtesukiMove::INVALID(), false);
      CATCH_DFPN;
      return NULL;
    }
    else /* 全ての手が普通に反証された */
    {
      TRY_DFPN;
      record->setResult<A>(pass_left, ProofDisproof::NoCheckmate(),
			   NtesukiMove::INVALID(), false);
      CATCH_DFPN;
      return NULL;
    }
  }

  ntesuki_assert(best_proof != 0);
  ntesuki_assert(sum_disproof != 0);
  ntesuki_assert(best_proof < ProofDisproof::PROOF_LIMIT);
  ntesuki_assert(sum_disproof < ProofDisproof::DISPROOF_LIMIT);

  if (record->useOld<A>(pass_left))
  {
    ntesuki_assert(min_child_age != SHRT_MAX);
    record->distance = min_child_age;
  }
  average_cost /= average_cost_count;
  step_cost = std::max(average_cost, 1);
  return best_move;
}

template <Player T>
void osl::ntesuki::NtesukiSearcher::
handleNonAttack(NtesukiRecord* record,
		int pass_left)
{
  const Player A = T;
  ntesuki_assert(T == state.turn());

  NtesukiMoveList moves;
  mg->generate<T>(state, moves);

  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); ++move_it)
  {
    NtesukiRecord *record_best = table.findWithMove(record,
						     *move_it);
    const PathEncoding path_best(path, move_it->getMove());
    if (!record_best ||
	!record_best->getValueWithPath<A>(pass_left,
					  path_best).isCheckmateFail()) continue;

    /* record_best is checkmate fail */
    for (NtesukiMoveList::iterator move_it2 = moves.begin();
	 move_it2 != moves.end(); ++move_it2)
    {
      if (move_it2->isPass())
      {
	continue;
      }
      if (*move_it2 == *move_it)
      {
	continue;
      }
      if (move_it2->isCheckmateFail<A>(pass_left))
      {
	continue;
      }
      NtesukiRecord *record_child = table.allocateWithMove(record,
							   *move_it2);
      if (record_child == 0)
      {
	*stop_flag = TableLimitReached;
	return;
      }
      ntesuki_assert(record_child);
      const PathEncoding path_child(path, move_it->getMove());
      if(record_child->getValueWithPath<A>(pass_left,
					   path_child).isFinal())
      {
	continue;
      }

      if (record_child->isVisited())
      {
	TRY_DFPN;
	move_it2->setCheckmateFail<A>(pass_left);
	record->setLoopWithPath<A>(pass_left, path_child);
	CATCH_DFPN;
	continue;
      }

      bool simulation_result;
      CallSimulationAttack<NtesukiSimulationSearcher, T>
	helper(simulator, table, record_child, record_best,
	       pass_left, simulation_result, move_it2->getMove());
      TRY_DFPN;
      ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, move_it2->getMove(), helper);
      CATCH_DFPN;
      RETURN_ON_STOP;

      if (simulation_result)
      {
	move_it2->setBySimulation();
	move_it2->setCheckmateFail<A>(pass_left);
      }
    }
  }
}

/* ===================
 * defense
 */
template <Player T>
NtesukiResult osl::ntesuki::NtesukiSearcher::
defense(NtesukiRecord* record,
	const NtesukiRecord* oracle_attack,
	const NtesukiRecord* oracle_defense,
	unsigned int proof_limit, unsigned int disproof_limit,
	int pass_left,
	const Move last_move)
{
  const Player A = PlayerTraits<T>::opponent;
  const Player D = T;
  CountChildLock cclock(record, table);

  ntesuki_assert(T == state.turn());
  ntesuki_assert(proof_limit    < ProofDisproof::PROOF_LIMIT);
  ntesuki_assert(disproof_limit < ProofDisproof::DISPROOF_LIMIT);
  ntesuki_assert (record->getValueAnd<A>(pass_left, path,
					 iwscheme, psscheme).proof()
		  < proof_limit);
  ntesuki_assert (record->getValueAnd<A>(pass_left, path,
					 iwscheme, psscheme).disproof()
		  < disproof_limit);
  
  ntesuki_assert(state.inCheck(T) || (pass_left > 0));
  ntesuki_assert(!state.inCheck(A));

  ++node_count;
  RETURN_ON_STOP (record->getValueAnd<A>(pass_left, path,
					 iwscheme, psscheme));

  ntesuki_assert(record);
  ntesuki_assert(!record->getValueWithPath<A>(pass_left, path).isFinal());
  ntesuki_assert(record->getBestMove<A>(pass_left).isInvalid());

  NtesukiRecord::VisitLock visitLock(record);

  /* ノードの初期化． 必要なら FixedDepthSearcher も呼ばれる．
   */
  TRY_DFPN;
  if (record->setUpNode<T>())
  {
    const NtesukiResult r = record->getValueWithPath<A>(pass_left, path);
    if (r.isCheckmateFail())
    {
      /* By fixed searcher */
      ++immediate_lose;
      RETURN (r);
    }
    else if (r.isFinal())
    {
      RETURN (r);
    }
  }
  CATCH_DFPN;

  /* Player Selection Scheme に応じて探索
   */
  NtesukiResult result_cur = record->getValueAnd<A>(pass_left, path, iwscheme, psscheme);
  while ((result_cur.proof() < proof_limit) &&
	 (result_cur.disproof() < disproof_limit))
  {
    bool read_attack_first = false;
    if (psscheme)
    {
      /* Dual Lambda 探索を行う:
       *  pn_D(n-1) < dn_A(n) なら order を下げて，攻守を入れ換える. ただし，
       *   pn_D(n-1) : 受け方の order n-1 における証明数
       *   dn_A(n)   : 攻め方の order n における反証数
       */
      if (pass_left > 0)
      {
	const NtesukiResult result_attacker =
	  record->getValueWithPath<A>(pass_left, path);
	const NtesukiResult result_defender =
	  record->getValueOr<D>(pass_left - 1, path, iwscheme);
	if (result_defender.proof() < result_attacker.disproof())
	{
	  read_attack_first = true;
	}
      }
    }

    if (read_attack_first)
    {
      NtesukiRecord::UnVisitLock unVisitLock(record);

      TRY_DFPN;
      attack<T>(record, NULL, NULL,
		disproof_limit, proof_limit,
		pass_left - 1, last_move);
      CATCH_DFPN;
      RETURN_ON_STOP (result_cur);
    }
    else
    {
      TRY_DFPN;
      defenseWithPlayer<T>(record, oracle_attack, oracle_defense,
			   proof_limit, disproof_limit,
			   pass_left, last_move);
      CATCH_DFPN;
      RETURN_ON_STOP (result_cur);
    }
    result_cur = record->getValueAnd<A>(pass_left, path, iwscheme, psscheme);
  }
  return result_cur;
}

template <Player T>
void osl::ntesuki::NtesukiSearcher::
defenseWithPlayer(NtesukiRecord* record,
		  const NtesukiRecord* oracle_attack,
		  const NtesukiRecord* oracle_defense,
		  unsigned int proof_limit, unsigned int disproof_limit,
		  int pass_left,
		  const Move last_move)
{
  const Player A = PlayerTraits<T>::opponent;
  const bool under_attack = state.inCheck(T);

  /* 受け手の生成
   */
  NtesukiMoveList moves;

  TRY_DFPN;
  record->generateMoves<T>(moves, pass_left, true);
  CATCH_DFPN;

  if (moves.empty())
  {
    TRY_DFPN;
    record->setResult<A>(0, ProofDisproof::NoEscape(),
			 NtesukiMove::INVALID(), false);
    CATCH_DFPN;
    return;
  }
  
  /* collect statistic information */
  ++defense_node_count;
  if (under_attack)
  {
    ++defense_node_under_attack_count;
  }
  defense_node_moves_count += moves.size();

  ntesuki_assert(!moves.empty());
  ntesuki_assert(record->getValueWithPath<A>(pass_left, path).isUnknown());

  /* Pass の disproof simulation. */
  if (!under_attack &&
      record->do_oracle_aunt &&
      oracle_defense)
  {
    record->do_oracle_aunt = false;
    
    NtesukiMove pass(Move::PASS(T));
    NtesukiRecord *record_pass = table.allocateWithMove(record,
							pass);
    if (record_pass == 0)
    {
      *stop_flag = TableLimitReached;
      return;
    }
    ntesuki_assert(record_pass);

    if (record_pass->isVisited())
    {
      /* ループ発見 */
      record->setLoopWithPath<A>(pass_left, path);
      assert(record->isLoopWithPath<A>(pass_left, path));
      TRY_DFPN;
      record->setResult<A>(pass_left, NtesukiResult(1, 1),
			   NtesukiMove::INVALID(), false);
      CATCH_DFPN;
      return;
    }
    
    ntesuki_assert(record_pass);
    const PathEncoding path_pass(path, pass.getMove());
    const NtesukiResult result_pass = record_pass->getValueWithPath<A>(pass_left - 1,
								       path_pass);
    if (!result_pass.isFinal())
    {
      bool simulation_result;
      CallSimulationDefenseDisproof<NtesukiSimulationSearcher, T>
	helper(simulator, table, record_pass, oracle_defense,
	       pass_left - 1, simulation_result, pass.getMove());
      TRY_DFPN;
      ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, pass.getMove(), helper);
      CATCH_DFPN;
      return;
      
      if (simulation_result)
      {
	ntesuki_assert(record_pass->getValueWithPath<A>(pass_left - 1,
							path_pass).isCheckmateFail());
	pass.setBySimulation();
	pass.setCheckmateFail<A>(pass_left);
      }
    }
  }

  /* IS 将棋の simulation */
  if (record->do_oracle_attack && oracle_attack)
  {
    record->do_oracle_attack = false;
    
    ntesuki_assert(ptt_uncle && !under_attack); // 本来は pass_left >= 1 か
    NtesukiMove pass(Move::PASS(T));
    
    NtesukiRecord *record_pass = table.allocateWithMove(record,
							pass);
    if (record_pass == 0)
    {
      *stop_flag = TableLimitReached;
      return;
    }
    ntesuki_assert(record_pass);

    if (record_pass->isVisited())
    {
      /* ループ発見 */
      record->setLoopWithPath<A>(pass_left, path);
      assert(record->isLoopWithPath<A>(pass_left, path));
      TRY_DFPN;
      record->setResult<A>(pass_left, NtesukiResult(1, 1),
			   NtesukiMove::INVALID(), false);
      CATCH_DFPN;
      return;
    }
    
    ntesuki_assert(record_pass);
    const PathEncoding path_pass(path, pass.getMove());
    const NtesukiResult result_pass = record_pass->getValueWithPath<A>(pass_left - 1,
								       path_pass);
    if (!result_pass.isFinal())
    {
      ++isshogi_attack_count;
      
      bool simulation_result;
      CallSimulationDefense<NtesukiSimulationSearcher, T>
	helper(simulator, table, record_pass, oracle_attack,
	       pass_left - 1, simulation_result, pass.getMove());
      TRY_DFPN;
      ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, pass.getMove(), helper);
      CATCH_DFPN;
      return;
      
      if (simulation_result)
      {
	++isshogi_attack_success_count;
	ntesuki_assert(record_pass->getValueWithPath<A>(pass_left - 1,
							path_pass).isCheckmateSuccess());
	pass.setBySimulation();
	pass.setCheckmateSuccess<A>(pass_left);
	record->setNtesuki<A>(pass_left);
	
	if (ptt_invalid_defense)
	{
	  TRY_DFPN;
	  simulateSiblingsSuccess<T>(record, record_pass, pass_left,
				     pass_success_count,
				     pass_count);
	  CATCH_DFPN;
	  return;
	}
      }
    }
  }

  for (;;)
  {
    unsigned int best_disproof = ProofDisproof::BigProofNumber,
      sum_proof = 0,
      second_disproof = ProofDisproof::BigProofNumber,
      best_proof = 0;

    unsigned int step_cost = 1;
    NtesukiMove *best_move = NULL;

    best_move = selectMoveDefense<T>(record,
				     best_disproof,
				     sum_proof,
				     second_disproof,
				     best_proof,
				     step_cost,
				     moves,
				     pass_left,
				     last_move);
    RETURN_ON_STOP;
    if (NULL == best_move)
    {
      ntesuki_assert(record->getValueWithPath<A>(pass_left, path).
		     isCheckmateSuccess());
      return;
    }
    else if (best_disproof == 0)
    {
      ntesuki_assert(best_move->isCheckmateFail<A>(pass_left) ||
		     record->isLoopWithPath<A>(pass_left, path));
      return;
    }

#ifndef NDEBUG
    {	  //XXXXXXX
      NtesukiRecord* best_child = table.findWithMove(record, *best_move);
      if (best_child)
      {
	const PathEncoding path_child(path, best_move->getMove());
	int pass_left_child = pass_left;
	if (best_move->isPass()) --pass_left_child;
	const NtesukiResult r =
	  best_child->getValueOr<A>(pass_left_child, path_child,iwscheme);
	ntesuki_assert(r.disproof() == best_disproof);
	ntesuki_assert(r.proof() <= sum_proof);
      }
    }	  //XXXXXXX
#endif

    /* このノードの証明数・反証数を設定する
     */
    const NtesukiResult result_cur = ProofDisproof(sum_proof, best_disproof);
    record->setResult<A>(pass_left, result_cur,
			 NtesukiMove::INVALID(), false);
   
    /* 他を読む
     */
    if ((disproof_limit <= best_disproof) ||
	(proof_limit <= sum_proof))
    {
      ntesuki_assert(!result_cur.isFinal());
      return;
    }

    unsigned int proof_child =
      addWithSaturation(ProofDisproof::DISPROOF_LIMIT, 
			proof_limit, best_proof) - sum_proof;
    unsigned int disproof_child = std::min(disproof_limit,
					   second_disproof + step_cost);

    /* 手を試す
     */
    int pass_left_child = pass_left;
    if (best_move->isPass())
    {
      --pass_left_child;
    }
    NtesukiRecord *record_child =
      table.allocateWithMove(record, *best_move);
    if (record_child == 0)
    {
      *stop_flag = TableLimitReached;
      return;
    }
    ntesuki_assert(record_child);
    const PathEncoding path_child(path, best_move->getMove());
    ntesuki_assert(pass_left_child >= 0);
    NtesukiResult result_child =
      record_child->getValueOr<A>(pass_left_child,
				  path_child,
				  iwscheme);

    if (!result_child.isFinal())
    {
      if (best_move->isCheck())
      {
	if (ptt_uncle &&
	    !under_attack &&
	    delay_non_pass)
	{
	  NtesukiMove& pass = moves.front();
	  ntesuki_assert(pass.isPass());
	  
	  oracle_attack = table.findWithMove(record, pass);
	  ntesuki_assert(oracle_attack);
	}
      }

      if (result_child.proof() >= proof_child)
      {
	std::cerr << *record_child
		  << result_child << "<- result \n"
		  << proof_child << "/" << disproof_child << "<- limit\n"
		  << *best_move << "\n"
		  << sum_proof << "/" << best_disproof << " <- cur\n";
      }
      DefenseHelper<NtesukiSearcher, T> helper(this,
					       result_child,
					       record_child,
					       oracle_attack,
					       oracle_defense,
					       proof_child,
					       disproof_child,
					       pass_left_child,
					       best_move->getMove());

      PlayMoveLock pml(moves_played, best_move->getMove());
      if (best_move->isPass())
      {
	NtesukiRecord::pass_count++;
      }
      TRY_DFPN;
      ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, best_move->getMove(), helper);
      CATCH_DFPN;

      if (best_move->isPass())
      {
	NtesukiRecord::pass_count--;
      }
      record->updateWithChild(record_child, pass_left);
      RETURN_ON_STOP;

      if (record->getValueWithPath<A>(pass_left, path).isFinal())
      {
	return;
      }
    }

    /* 結果を吟味する
     */
    if (result_child.isCheckmateFail())
    {
      if (result_child == ProofDisproof::AttackBack())
      {
	++disproof_by_inversion_count;
      }
      if (result_child == ProofDisproof::LoopDetection())
      {
	record->setLoopWithPath<A>(pass_left, path);
	TRY_DFPN;
	record->setResult<A>(pass_left, NtesukiResult(1, 1),
			     NtesukiMove::INVALID(), false);
	CATCH_DFPN;
	return;
      }

      best_move->setCheckmateFail<A>(pass_left);
      TRY_DFPN;
      record->setResult<A>(pass_left, result_child,
			   *best_move, false);
      CATCH_DFPN;
      return;
    }
    else if (result_child.isCheckmateSuccess())
    {
      best_move->setCheckmateSuccess<A>(pass_left);
      NtesukiRecord *best_record = table.findWithMove(record, *best_move);
      if ((ptt_invalid_defense && best_move->isPass()) ||
	  (ptt_siblings_success && !best_move->isCheck())
	  )
      {
	TRY_DFPN;
	simulateSiblingsSuccess<T>(record, best_record, pass_left,
				   sibling_defense_success_count,
				   sibling_defense_count);
	CATCH_DFPN;
	RETURN_ON_STOP;
      }
    }
  }//for(;;)
}

template <Player T>
osl::ntesuki::NtesukiMove* osl::ntesuki::NtesukiSearcher::
selectMoveDefense(NtesukiRecord* record,
		  unsigned int& best_disproof,
		  unsigned int& sum_proof,
		  unsigned int& second_disproof,
		  unsigned int& best_proof,
		  unsigned int& step_cost,
		  NtesukiMoveList& moves,
		  const int pass_left,
		  const Move last_move)
{
  const Player A = PlayerTraits<T>::opponent;
  const bool under_attack = state.inCheck(T);

  bool read_interpose = record->readInterpose(pass_left);
  /* GCによって情報がかわっている可能性が
   * bool read_non_pass = record->isNtesuki<A>(pass_left);*/

  bool read_non_pass = under_attack;
  if (pass_left > 0 && !under_attack)
  {
    NtesukiMove pass(Move::PASS(T));
    NtesukiRecord *record_pass = table.findWithMove(record, pass);
    if (record_pass)
    {
      const PathEncoding path_child(path, pass.getMove());
      read_non_pass =
	record_pass->getValueWithPath<A>(pass_left - 1,
					 path_child).isCheckmateSuccess();
    }
  }
  if (under_attack) ntesuki_assert(read_non_pass);

  bool read_check_defense = record->readCheckDefense(pass_left);
  if (isscheme == NtesukiRecord::normal_is)
  {
    read_check_defense = true;
  }

 re_select_move_defense:
  unsigned short min_child_age = SHRT_MAX;
  NtesukiMove *best_move = NULL;

  int average_cost = 0;
  int average_cost_count = 0;

  /* reset values */
  best_disproof = ProofDisproof::BigProofNumber;
  sum_proof = 0;
  second_disproof = ProofDisproof::BigProofNumber;
  best_proof = 0;

  /* dynamic Widening */
  std::list<top_pdp_t> pdps;

  /* 手を選ぶ
   */
  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); ++move_it)
  {
    NtesukiMove& move = *move_it;
    if (move.isCheckmateSuccess<A>(pass_left))
    {
      continue;
    }
    ntesuki_assert(!move.isCheckmateFail<A>(pass_left));
    
    if (delay_non_pass &&
	!read_non_pass &&
	!move.isPass())
    {
      continue;
    }

    if (delay_interpose &&
	(move.isInterpose() ||
	 move.isLameLong()) &&
	!read_interpose)
    {
      continue;
    }

    if (move.isCheck() &&
	!under_attack &&
	!read_check_defense)
    {
      continue;
    }
    
    unsigned int proof = move.h_d_proof;
    unsigned int disproof = move.h_d_disproof;
    
    average_cost += disproof;
    average_cost_count++;

    NtesukiRecord *record_child = table.findWithMove(record, move);
    if (record_child)
    {
      int pass_left_child = pass_left;
      if (move.isPass()) --pass_left_child;
      const PathEncoding path_child(path, move.getMove());
      NtesukiResult result_child;
      TRY_DFPN;
      result_child =
	record_child->getValueOr<A>(pass_left_child, path_child,
				    iwscheme);
      CATCH_DFPN;
      
      if (record_child->isVisited())
      {
	/* ループ発見 */
	record->setLoopWithPath<A>(pass_left, path);
	ntesuki_assert(record->isLoopWithPath<A>(pass_left, path));
	TRY_DFPN;
	record->setResult<A>(pass_left, NtesukiResult(1, 1),
			     NtesukiMove::INVALID(), false);
	CATCH_DFPN;
	best_disproof = 0;
	return &move;
      }

      proof    = result_child.proof();
      disproof = result_child.disproof();

      if (result_child.isCheckmateSuccess())//証明
      {
	ntesuki_assert(disproof >= ProofDisproof::DISPROOF_LIMIT);
	move.setCheckmateSuccess<A>(pass_left);
	if  (move.isPass())
	{
	  /* 既にパスの後が証明されていた */
	  record->setNtesuki<A>(pass_left);

	  if (ptt_invalid_defense)
	  {
	    TRY_DFPN;
	    simulateSiblingsSuccess<T>(record, record_child, pass_left,
				       pass_success_count,
				       pass_count);
	    CATCH_DFPN;
	    RETURN_ON_STOP(NULL);
	    goto re_select_move_defense;
	  }
	}
	if (ptt_siblings_success && !move.isCheck())
	{
	  TRY_DFPN;
	  simulateSiblingsSuccess<T>(record, record_child, pass_left,
				     sibling_defense_success_count,
				     sibling_defense_count);
	  CATCH_DFPN;
	  RETURN_ON_STOP(NULL);
	  //re search as simulation is done.
	  goto re_select_move_defense;
	}
	continue;
      }
      else if (result_child.isCheckmateFail())//反証
      {
	if (move.isCheck() && read_check_defense)
	{
	  ++disproof_by_inversion_count;
	}
	if (result_child == ProofDisproof::LoopDetection())
	{
	  record->setLoopWithPath<A>(pass_left, path);
	  TRY_DFPN;
	  record->setResult<A>(pass_left, NtesukiResult(1, 1),
			       NtesukiMove::INVALID(), false);
	  CATCH_DFPN;
	  best_disproof = 0;
	  return &move;
	}

	ntesuki_assert(proof >= ProofDisproof::PROOF_LIMIT);
	move.setCheckmateFail<A>(pass_left);
	TRY_DFPN;
	record->setResult<A>(pass_left, result_child,
			     move, false);
	CATCH_DFPN;
	best_disproof = 0;
	return &move;
      }

      min_child_age = std::min(min_child_age,
			       record_child->distance);
      if ((record_child->distance <= record->distance) &&
	  !move.isPass())
      {
	if (!record->useOld<A>(pass_left))
	{
	  continue;
	}
      }
    }/* has record */

    /* Proof Disproof の調整はここでする */
    if (record->useOld<A>(pass_left))
    {
      sum_proof = std::max(proof, sum_proof);
    }
    else if (NtesukiRecord::max_for_split && record->is_split)
    {
      sum_proof = std::max(proof, sum_proof);
    }
    else
    {
      sum_proof = addWithSaturation(ProofDisproof::PROOF_LIMIT, 
				    proof, sum_proof);
    }

    if (disproof < best_disproof)
    {
      best_move = &move;
      second_disproof = best_disproof;
      best_disproof = disproof;
      best_proof = proof;
    }
    else if (disproof < second_disproof)
    {
      second_disproof = disproof;
    }

    /* dynamic widening: 良い手の選定 */
    if (dynamic_widening_width > 0)
    {
      if (pdps.size() < dynamic_widening_width)
      {
	pdps.push_back(top_pdp_t(disproof, proof));
	pdps.sort(sorter);
      }
      else
      {
	if (pdps.back().first > disproof)
	{
	  pdps.pop_back();
	  pdps.push_back(top_pdp_t(disproof, proof));
	}// back().disproof == disproof だった場合は？
	pdps.sort(sorter);
      }
    }
  }/* foreach move */

  /* dynamic widening: 良い手の集計 */
  if (dynamic_widening_width > 0 &&
       dynamic_widening_width < moves.size())
  {
    sum_proof = 0;
    for (std::list<top_pdp_t>::const_iterator it = pdps.begin();
	 it != pdps.end(); ++it)
    {
      sum_proof += it->second;
    }
  }

  /* 選んだ手を吟味する
   */
  if (NULL == best_move)
  {
    ntesuki_assert(sum_proof == 0);
    /* パスだけ先に読む enhancement */
    if (delay_non_pass &&
	read_non_pass == false)
    {
      ntesuki_assert(!under_attack);
      
      read_non_pass = true;
      record->setUseOld<A>(pass_left, false);
      record->setNtesuki<A>(pass_left);

      if (ptt_invalid_defense)
      {
	NtesukiMove move_pass = moves.front();
	ntesuki_assert(move_pass.isPass());
	NtesukiRecord *record_pass = table.findWithMove(record, move_pass);
	const PathEncoding path_child(path, move_pass.getMove());
	
	ntesuki_assert(record_pass->getValueWithPath<A>(pass_left - 1,
							path_child).isCheckmateSuccess());
	TRY_DFPN;
	simulateSiblingsSuccess<T>(record, record_pass, pass_left,
				   pass_success_count,
				   pass_count);
	CATCH_DFPN;
	RETURN_ON_STOP(NULL);
      }
      goto re_select_move_defense;
    } /* delay non pass */

    if (!record->useOld<A>(pass_left))
    {
      if (SHRT_MAX != min_child_age)
      {
	record->setUseOld<A>(pass_left, true);

	ntesuki_assert(min_child_age <= record->distance);
	record->distance = min_child_age;

	goto re_select_move_defense;
      }
    }

    if (delay_interpose &&
	read_interpose == false)
    {
      read_interpose = true;
      record->setUseOld<A>(pass_left, false);
      record->setReadInterpose(pass_left);
      TRY_DFPN;
      handleInterpose<T>(record, pass_left);
      CATCH_DFPN;
      RETURN_ON_STOP NULL;

      goto re_select_move_defense;
    }

    /* 逆王手の扱い */
    switch(isscheme)
    {
    case NtesukiRecord::no_is:
      ntesuki_assert(read_check_defense == false);
      break;
    case NtesukiRecord::tonshi_is:
      handleTonshi<T>(record, pass_left, last_move);
      RETURN_ON_STOP NULL;
      break;
    case NtesukiRecord::delay_is:
      if (read_check_defense == false)
      {
	++proof_without_inversion_count;
	read_check_defense = true;
	record->setReadCheckDefense(pass_left);
	goto re_select_move_defense;
      }
      break;
    case NtesukiRecord::normal_is:
      ntesuki_assert(read_check_defense == true);
      break;
    }

    /* 全ての手が普通に証明された */
    TRY_DFPN;
    record->setResult<A>(pass_left, ProofDisproof::Checkmate(),
			 NtesukiMove::INVALID(), false);
    CATCH_DFPN;
    return NULL;
  }
  ntesuki_assert(best_move);
  ntesuki_assert(sum_proof != 0);
  ntesuki_assert(best_disproof != 0);

  if (record->useOld<A>(pass_left))
  {
    ntesuki_assert(min_child_age != SHRT_MAX);
    record->distance = min_child_age;
  }
  average_cost /= average_cost_count;
  step_cost = std::max(average_cost, 1);
  return best_move;
}

template <Player T>
void osl::ntesuki::NtesukiSearcher::
handleTonshi(NtesukiRecord *record,
	     int pass_left,
	     const Move last_move)
{
  const Player A = PlayerTraits<T>::opponent;
  const Player D = T;

  if (pass_left > 0)
  {
    NtesukiResult result_defender =
      record->getValueWithPath<D>(pass_left - 1, path);
    if (!result_defender.isFinal())
    {
      /* to make sure not to come back here
       */
      record->setResult<A>(pass_left, ProofDisproof::Bottom(),
			   NtesukiMove::INVALID(), false);
      const unsigned int read_node_limit_orig = read_node_limit;
      int ratio = 1;
      
      if ((record->distance / 2) == 0)
	ratio = 8;
      else if ((record->distance / 2) == 1)
	ratio = 2;
      
      read_node_limit = node_count + READ_ATTACK_BACK_LIMIT * ratio;
      
      NtesukiRecord::UnVisitLock unVisitLock(record);
      TRY_DFPN;
      result_defender = attack<T>(record, NULL, NULL,
				  INITIAL_PROOF_LIMIT, INITIAL_PROOF_LIMIT,
				  pass_left - 1, last_move);
      CATCH_DFPN;
      
      if (result_defender.isCheckmateSuccess())
      {
	++attack_back_count;
      }
      
      read_node_limit = read_node_limit_orig;
      RETURN_ON_STOP;
    }
    
    if (result_defender.isFinal())
    {
      return;
    }
  }
}

template <Player T>
void osl::ntesuki::NtesukiSearcher::
simulateSiblingsSuccess(NtesukiRecord *record,
			NtesukiRecord *record_best,
			int pass_left,
			unsigned int& success_count,
			unsigned int& total_count)
{
  LockGC glock(table);

  const Player A = PlayerTraits<T>::opponent;
  if (!record_best) return;
  ntesuki_assert(record_best);
  ntesuki_assert(record_best->getValue<A>(pass_left).isCheckmateSuccess());

  NtesukiMoveList moves;
  mg->generate<T>(state, moves);

  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); ++move_it)
  {
    NtesukiMove& move = *move_it;
    NtesukiRecord *record_child = table.allocateWithMove(record,
							 move);
    if (record_child == 0)
    {
      *stop_flag = TableLimitReached;
      return;
    }
    ntesuki_assert(record_child);
    if (record_child == record_best) continue;
    if (record_child->isVisited()) continue;
    
    ntesuki_assert(record_child);
    const PathEncoding path_child(path, move.getMove());
    const NtesukiResult result_child = record_child->getValueWithPath<A>(pass_left,
									 path_child);
    if (result_child.isFinal())
    {
      continue;
    }

    bool simulation_result;
    total_count++;
    CallSimulationDefense<NtesukiSimulationSearcher, T>
      helper(simulator, table, record_child, record_best,
	     pass_left, simulation_result, move.getMove());
    TRY_DFPN;
    ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, move.getMove(), helper);
    CATCH_DFPN;
    RETURN_ON_STOP;

    if (simulation_result)
    {
      success_count++;
      ntesuki_assert(record_child->getValueWithPath<A>(pass_left,
						       path_child).isCheckmateSuccess());
      move.setBySimulation();
      move.setCheckmateSuccess<A>(pass_left);
    }
  }
}

template <Player T>
void osl::ntesuki::NtesukiSearcher::
handleInterpose(NtesukiRecord* record,
		int pass_left)
{
  const Player A = PlayerTraits<T>::opponent;
  ntesuki_assert(T == state.turn());

  NtesukiMoveList moves;
  mg->generate<T>(state, moves);

  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); ++move_it)
  {
    if (move_it->isInterpose() &&
	!move_it->isCheckmateSuccess<A>(pass_left))
    {
      NtesukiRecord *record_child = table.allocateWithMove(record,
							   *move_it);
      if (record_child == 0)
      {
	*stop_flag = TableLimitReached;
	return;
      }
      ntesuki_assert(record_child);

      const PathEncoding path_child(path, move_it->getMove());
      if(record_child->getValueWithPath<A>(pass_left,
					   path_child).isFinal())
      {
	continue;
      }
      ntesuki_assert(record_child->getBestMove<A>(pass_left).isInvalid());
      
      NtesukiMoveList::iterator best_it = moves.begin();
      for (; best_it != moves.end(); ++best_it)
      {
	if (best_it->to() == move_it->to() &&
	    best_it->isCheckmateSuccess<A>(pass_left)) break;
      }
      if (best_it == moves.end())
      {
	continue;
      }
      const NtesukiRecord* record_best = table.findWithMove(record, *best_it);
      ntesuki_assert(record_best);

      bool simulation_result;
      CallSimulationDefense<NtesukiSimulationSearcher, T>
	helper(simulator, table, record_child, record_best,
	       pass_left, simulation_result, move_it->getMove());
      TRY_DFPN;
      ApplyMoveWithPath<T>::doUndoMoveOrPass(state, path, move_it->getMove(), helper);
      CATCH_DFPN;
      RETURN_ON_STOP;

      if (simulation_result)
      {
	move_it->setBySimulation();
	move_it->setCheckmateSuccess<A>(pass_left);
      }
      else if (record_child->getValue<A>(pass_left).isCheckmateFail())
      {
	break;
      }
    }
  }
}

/* 外から呼ばれる関数.
 */
template <Player A>
int  osl::ntesuki::NtesukiSearcher::
search()
{
  NtesukiRecord::pass_count = 0;
  const Player D = PlayerTraits<A>::opponent;
  //const HashKey key = HashKey::calcHash(state);  
  const HashKey key(state);  

  NtesukiRecord *record = table.allocateRoot(key, PieceStand(WHITE, state),
					     0, &state);
  ntesuki_assert(record);

  NtesukiResult result;
  if (A == state.turn())
  {
    const Player T = A;
    result = attack<T>(record, NULL, NULL,
		       INITIAL_PROOF_LIMIT,
		       INITIAL_DISPROOF_LIMIT,
		       max_pass - 1, Move::INVALID());
  }
  else//A != turn
  {
    const Player T = D;
    if (0 == (max_pass - 1) &&
	!state.inCheck(D))
    {
      if (verbose) std::cerr << "No Check" << std::endl;
      return NtesukiNotFound;
    }
    else
    {
      result = defense<T>(record, NULL, NULL,
			  INITIAL_PROOF_LIMIT,
			  INITIAL_DISPROOF_LIMIT,
			  max_pass - 1,
			  Move::INVALID());
    }
  }

  if (node_count > read_node_limit || *stop_flag)
  {
    if (verbose) std::cerr << "Limit Reached\t" << result << std::endl;
    return ReadLimitReached;
  }
  else
  {
    if (verbose) std::cerr << "result:\t" << result << std::endl;
    if (result.isCheckmateSuccess())
    {
      for (unsigned int i = 0; i < max_pass; ++i)
      {
	if (record->getValue<A>(i).isCheckmateSuccess())
	{
	  return i;
	}
      }
    }
  }
  
  ntesuki_assert(result.isCheckmateFail());
  return NtesukiNotFound;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
