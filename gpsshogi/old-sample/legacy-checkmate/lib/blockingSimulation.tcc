/* blockingSimulation.tcc
 */
#ifndef _BLOCKING_SIMULATION_TCC
#define _BLOCKING_SIMULATION_TCC

#include "blockingSimulation.h"
#include "checkHashRecord.h"
#include "oracleProver.h"
#include "checkTableUtil.h"
#include "osl/apply_move/applyMoveWithPath.h"
#ifdef CHECKMATE_DEBUG
#  include "osl/stat/ratio.h"
#endif

namespace osl
{
  namespace checkmate
  {
    template <class Prover, Player P>
    struct CallProver
    {
      typedef typename Prover::state_t state_t;
      bool *result;
      Prover *prover;
      state_t *state;
      const HashKey& key;
      const PathEncoding& path;
      ProofOracleAttack<P> oracle;
      CallProver(bool *r, Prover *p, state_t *s, const HashKey& k,
		 const PathEncoding& pa, ProofOracleAttack<P> o)
	: result(r), prover(p), state(s), key(k), path(pa), oracle(o)
      {
      }
      void operator()(Square)
      {
	Move check_move; // 本当は不要 interface 増やす?
	*result = (*prover).template proofWin<P>
	  (*state, key, path, oracle, check_move);
      }
    };
  } // namespace checkmate
} // namespace osl

template <osl::Player P>
template <class Table>
inline
bool osl::checkmate::BlockingSimulation<P>::
proof(NumEffectState& state, const HashKey& new_key,
      const PathEncoding& new_path, Table& table, 
      const CheckMove& target, const CheckHashRecord *guide,
      size_t& node_count)
{
  assert(state.turn() == alt(P));
  ProofOracleAttack<P> oracle(guide);
  assert(oracle.isValid());
  typedef OracleProver<Table> prover_t;
  prover_t prover(table);
  bool result;
  CallProver<prover_t,P> helper(&result, &prover, &state, new_key, new_path, oracle);
  ApplyMove<PlayerTraits<P>::opponent>::doUndoMove(state, target.move, helper);
  node_count += prover.nodeCount();
  return result;
}

template <osl::Player P>
template <class Table>
bool osl::checkmate::BlockingSimulation<P>::
proof(NumEffectState& state, const HashKey& new_key, const PathEncoding& new_path,
      const CheckHashRecord *record, Table& table, const CheckMove& move,
      size_t& node_count)
{
#ifdef CHECKMATE_DEBUG
  static stat::Ratio oracle_found("blocking proof:oracle found"), 
    proven("blocking proof:sim success");
#endif
  CheckMoveList::const_iterator p=record->moves.begin();
  for (; p!=record->moves.end(); ++p)
  {
    if (p->flags.isSet(MoveFlags::Solved)
	&& (p->move.to() == move.move.to()))
      break;
  }
#ifdef CHECKMATE_DEBUG
  oracle_found.add(p != record->moves.end());
#endif    
  if (p == record->moves.end())
    return false;
  
  const bool result = proof(state, new_key, new_path, 
			    table, move, p->record, node_count);
#ifdef CHECKMATE_DEBUG
  proven.add(result);
#endif
  return result;
}

template <osl::Player P>
template <class Table>
void osl::checkmate::BlockingSimulation<P>::
proofSibling(NumEffectState& state, 
	     const HashKey& key, const PathEncoding& path, 
	     CheckHashRecord *record, 
	     Table& table, const CheckMove& guide, size_t& node_count)
{
#ifdef CHECKMATE_DEBUG
  static stat::Ratio proven("blocking proof:sibling");
#endif
  const Square to = guide.move.to();
  for (CheckMoveList::iterator p=record->moves.begin();
       p!=record->moves.end(); ++p)
  {
    if (! record->filter.isTarget(p->flags))
      continue;
    if (p->move.to() == to)
    {
      const HashKey new_key = key.newHashWithMove(p->move);
      const PathEncoding new_path(path, p->move);
      if (! p->record)
      {
	CheckTableUtil::allocate
	  (p->move, p->record, table, new_key, new_path, record);
      }
      const bool result =
	proof(state, new_key, new_path, table, *p, guide.record, node_count);
      if (result)
	record->addToSolvedInDefense(*p, p->record->proofDisproof());
#ifdef CHECKMATE_DEBUG
      proven.add(result);
#endif
    }
  }
}


#endif /* _BLOCKING_SIMULATION_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
