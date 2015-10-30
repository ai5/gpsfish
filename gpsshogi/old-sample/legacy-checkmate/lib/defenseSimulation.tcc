/* defenseSimulation.h
 */
#ifndef _DEFENSE_SIMULATION_TCC
#define _DEFENSE_SIMULATION_TCC

#include "defenseSimulation.h"
#include "checkHashRecord.h"
#include "oracleDisprover.h"
#include "checkTableUtil.h"
#include "osl/apply_move/applyMoveWithPath.h"
#ifdef CHECKMATE_DEBUG
#  include "osl/stat/ratio.h"
#endif

namespace osl
{
  namespace checkmate
  {
    template <class Disprover, Player P>
    struct CallDisprover
    {
      typedef typename Disprover::state_t state_t;
      bool *result;
      Disprover *prover;
      state_t *state;
      const HashKey& new_key;
      const PathEncoding& new_path;
      DisproofOracleDefense<P> oracle;
      Move last_move;
      CallDisprover(bool *r, Disprover *p, state_t *s, 
		    const HashKey& k, const PathEncoding& pa,
		    DisproofOracleDefense<P> o, Move last)
	: result(r), prover(p), state(s), new_key(k), new_path(pa), 
	  oracle(o), last_move(last)
      {
      }
      void operator()(Square)
      {
	Move escape_move; // 本当は不要 interface 増やす?
	*result = (*prover).template proofEscape<P>
	  (*state, new_key, new_path, oracle, escape_move, last_move);
      }
    };
  } // namespace checkmate
} // namespace osl

template <osl::Player P>
template <class Table>
inline
bool osl::checkmate::DefenseSimulation<P>::
disproof(NumEffectState& state, 
	 const HashKey& new_key, const PathEncoding& new_path, Table& table, 
	 const CheckMove& target, CheckHashRecord *guide, size_t& node_count)
{
  check_assert(target.record);
  check_assert(state.turn() == P);
  DisproofOracleDefense<P> oracle(guide);
  check_assert(oracle.isValid());
  typedef OracleDisprover<Table> disprover_t;
  disprover_t disprover(table);
  bool result;
  CallDisprover<disprover_t,P> 
    helper(&result, &disprover, &state, new_key, new_path, oracle, target.move);
  ApplyMove<P>::doUndoMove(state, target.move, helper);
  node_count += disprover.nodeCount();
  return result;
}

template <osl::Player P>
template <class Table>
bool osl::checkmate::DefenseSimulation<P>::
disproofNoPromote(NumEffectState& state, 
		  const HashKey& new_key, 
		  const PathEncoding& new_path,
		  CheckHashRecord *record, Table& table, 
		  CheckMove& target, const CheckMove& guide,
		  size_t& node_count)
{
#ifdef CHECKMATE_DEBUG
  static stat::Ratio disproven("defense disproof:nopromote");
#endif
  check_assert(guide.move.unpromote() == target.move);
  const bool result = disproof(state, new_key, new_path, table, target, 
			       guide.record, node_count);
  if (result)
  {
    if (target.record->proofDisproof().isCheckmateFail())
      record->addToSolvedInAttack(target, target.record->proofDisproof());
    else
      check_assert(target.record->findLoop(new_path, table.getTwinTable()));
  }
#ifdef CHECKMATE_DEBUG
  disproven.add(result);
#endif
  return result;
}

template <osl::Player P>
template <class Table>
void osl::checkmate::DefenseSimulation<P>::
disproofDropSibling(NumEffectState& state, 
		    const HashKey& key, const PathEncoding& path,
		    CheckHashRecord *record, 
		    Table& table, const CheckMove& guide, size_t& node_count)
{
#ifdef CHECKMATE_DEBUG
  static stat::Ratio disproven("defense disproof:drop sibling");
#endif
  check_assert(guide.move.isDrop());
  const Ptype ptype = guide.move.ptype();
  for (CheckMoveList::iterator p=record->moves.begin();
       p!=record->moves.end(); ++p)
  {
    if (! record->filter.isTarget(p->flags))
      continue;
    if (! p->move.isDrop())
      continue;

    const HashKey new_key = key.newHashWithMove(p->move);
    const PathEncoding new_path(path, p->move);
    if (p->move.ptype() == ptype)
    {
      if (! p->record)
      {
	CheckTableUtil::allocate
	  (p->move, p->record, table, new_key, new_path, record);
      }
      const bool result =
	disproof(state, new_key, new_path, table, *p, guide.record, node_count);
      if (result)
      {
	if (p->record->proofDisproof().isCheckmateFail())
	  record->addToSolvedInAttack(*p, p->record->proofDisproof());
	else
	  check_assert(p->findLoop(path, table.getTwinTable()));
      }
#ifdef CHECKMATE_DEBUG
      disproven.add(result);
#endif
    }
  }
}


#endif /* _DEFENSE_SIMULATION_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
