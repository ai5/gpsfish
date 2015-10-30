/* ntesukiSimulationDisproof.tcc
 */
#include "osl/ntesuki/ntesukiSimulationSearcher.h"
#include "osl/state/hashEffectState.h"

#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/ntesuki/ntesukiExceptions.h"
#include "osl/container/moveVector.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/apply_move/applyMoveWithPath.h"

#ifndef NDEBUG
#define RETURN \
  TRY_DFPN;\
  ntesuki_assert(result.isFinal());\
  CATCH_DFPN; \
  return
#else
#define RETURN return
#endif

using namespace osl;
using namespace osl::ntesuki;

template <class Searcher, Player P>
class
NtesukiSimulationSearcher::
AttackHelperDisproof
{
  Searcher* searcher;
  NtesukiRecord* record;
  const NtesukiRecord* record_orig;
  unsigned int pass_left;
  const Move last_move;

public:
  AttackHelperDisproof(Searcher* searcher,
		       NtesukiRecord* record,
		       const NtesukiRecord* record_orig,
		       unsigned int pass_left,
		       const Move last_move)
    : searcher(searcher),
      record(record), record_orig(record_orig),
      pass_left(pass_left),
      last_move(last_move)
  {}

  void operator()(Square p)
  {
    (*searcher).template defenseForDisproof<PlayerTraits<P>::opponent>
      (record, record_orig, pass_left, last_move);
  }
};
      
template <class Searcher, Player P>
class NtesukiSimulationSearcher::
DefenseHelperDisproof
{
  Searcher* searcher;
  NtesukiRecord* record;
  const NtesukiRecord* record_orig;
  unsigned int pass_left;
  const Move last_move;
public:
  DefenseHelperDisproof(Searcher* searcher,
			NtesukiRecord* record,
			const NtesukiRecord* record_orig,
			unsigned int pass_left,
			const Move last_move)
    : searcher(searcher),
      record(record), record_orig(record_orig),
      pass_left(pass_left),
      last_move(last_move)
  {}

	
  void operator()(Square p)
  {
    (*searcher).template attackForDisproof<PlayerTraits<P>::opponent>
      (record, record_orig, pass_left, last_move);
  }
};

/*======================================================================
 * Disproof
 *======================================================================
 */

template <Player P>
void
NtesukiSimulationSearcher::
attackForDisproof(NtesukiRecord* record,
		  const NtesukiRecord* record_orig,
		  const unsigned int pass_left,
		  const Move last_move)
{
  const Player attacker = P;
  ++node_count;
  ntesuki_assert(P == state.turn());
  
  ntesuki_assert(record);
  ntesuki_assert(record->getBestMove<attacker>(pass_left).isInvalid());
  ntesuki_assert(record_orig);

  const bool invalid_defense = state.inCheck(PlayerTraits<P>::opponent);
  if (invalid_defense)
  {
    result = ProofDisproof::Checkmate();
    RETURN;
  }

  ntesuki_assert (!record->isVisited());
  NtesukiRecord::VisitLock visitLock(record);

  if (record->setUpNode<P>())
  {
    result = record->getValueWithPath<attacker>(pass_left, path);
    if (result.isFinal())
    {
      /* result by fixed depth searcher */
      RETURN;
    }
  }

  /* n が少ないときの結果を確定 */
  if (pass_left > 0)
  {
    result = record->getValueWithPath<attacker>(pass_left - 1, path);
    if (!result.isFinal())
    {
      NtesukiRecord::UnVisitLock unVisitLock(record);
      attackForDisproof<P>(record, record_orig, pass_left - 1, last_move);
    }

    if (!result.isCheckmateFail())
    {
      RETURN;
    }
    ntesuki_assert(result.isCheckmateFail());
  }

  /* 攻め手の生成
   */
  NtesukiMoveList moves;
  record->generateMoves<P>(moves, pass_left, false);

  /* 攻める手の実行
   */
  bool has_loop = false;
  bool disproof_failed = false;
  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); move_it++)
  {
    NtesukiMove& move = *move_it;

    if (move.isPass()) continue;
    if (move.isCheckmateFail<attacker>(pass_left)) continue;
    if (!move.isCheck() && 0 == pass_left) continue;

    /* DANGEROUS  this time pawn checkmate might be avoidable */
    if (move.isNoPromote()) continue; 

    NtesukiRecord *record_child = table.allocateWithMove(record, move);
    if (record_child == 0)
    {
      result = ProofDisproof::Checkmate();
      RETURN;
    }
    ntesuki_assert(record_child);
    if(record_child->isVisited())
    {
      //move.setCheckmateFail<attacker>(pass_left);
      has_loop = true;
      continue;
    }

    PathEncoding path_child(path, move.getMove());
    result = record_child->getValueWithPath<attacker>(pass_left, path_child);

    if(result.isUnknown())
    {
      const NtesukiRecord* record_child_orig = table.findWithMoveConst(record_orig, move);
      if (!record_child_orig ||
	  !record_child_orig->getValue<attacker>(pass_left).isCheckmateFail())
      {
	/* a move that used not to be a check became a check, or
	 * might have been a loop
	 */
	result = ProofDisproof::Checkmate();
	RETURN;
      }
      
      AttackHelperDisproof<NtesukiSimulationSearcher, P> helper(this,
								record_child,
								record_child_orig,
								pass_left,
								move.getMove());
      TRY_DFPN;
      ApplyMoveWithPath<P>::doUndoMove(state, path, move.getMove(), helper);
      CATCH_DFPN;
      if (record->getValueWithPath<attacker>(pass_left, path).isFinal())
      {
	result = record->getValueWithPath<attacker>(pass_left, path);
	RETURN;
      }
    }

    if (result.isCheckmateSuccess())
    {
      disproof_failed = true;
      continue;
    }
    else if (result == ProofDisproof::LoopDetection())
    {
      has_loop = true;
    }
    
    ntesuki_assert(result.isCheckmateFail());
    move.setCheckmateFail<attacker>(pass_left);
  }

  if (disproof_failed)
  {
    result = ProofDisproof::Checkmate();
    RETURN;
  }


  if (has_loop)
  {
    record->setLoopWithPath<attacker>(pass_left, path);
    result = ProofDisproof::LoopDetection();
    TRY_DFPN;
    record->setResult<attacker>(pass_left, NtesukiResult(1, 1),
				NtesukiMove::INVALID(), false);
    CATCH_DFPN;
  }
  else
  {
    result = ProofDisproof::NoCheckmate();
    TRY_DFPN;
    record->setResult<attacker>(pass_left, result,
				NtesukiMove::INVALID(), true);
    CATCH_DFPN;
  }
  RETURN;
}

template <Player P>
void
NtesukiSimulationSearcher::
defenseForDisproof(NtesukiRecord* record,
		   const NtesukiRecord* record_orig,
		   const unsigned int pass_left,
		   const Move last_move)
{
  const Player attacker = PlayerTraits<P>::opponent;
  ++node_count;
  ntesuki_assert(P == state.turn());

  ntesuki_assert(state.inCheck(P) || (pass_left > 0));

  ntesuki_assert(record);
  ntesuki_assert(record->getBestMove<attacker>(pass_left).isInvalid());
  ntesuki_assert(record_orig);
  ntesuki_assert(!record->isVisited());
  ntesuki_assert(record_orig->getValue<attacker>(pass_left).
		 isCheckmateFail());
  NtesukiRecord::VisitLock visitLock(record);

  if (record->setUpNode<P>())
  {
    result = record->getValueWithPath<attacker>(pass_left, path);
    if (result.isFinal())
    {
      /* result by fixed depth searcher */
      RETURN;
    }
  }

#ifndef NDEBUG
  /* 攻撃側に王手がかかっていないか調べる */
  ntesuki_assert(!state.inCheck(attacker));
#endif

  /* 以前の bestMove を実行
   */
  const NtesukiMove best_move =
    record_orig->getBestMove<attacker>(pass_left);
  if (best_move.isInvalid())
  {
    /* is by fixed depth searcher */
    result = ProofDisproof::Checkmate();
    RETURN;
  }
  const NtesukiRecord *record_child_orig = table.findWithMoveConst(record_orig, best_move);
  if (!record_child_orig ||
      !record_child_orig->getValue<attacker>(pass_left).isCheckmateFail())
  {
    /* fixed depth searcher 等での disproof */
    result = ProofDisproof::Checkmate();
    RETURN;
  }
  ntesuki_assert(record_child_orig);

  if (!best_move.isPass() &&
      !state.template isAlmostValidMove<false>(best_move.getMove()))
  {
    result = ProofDisproof::Checkmate();
    RETURN;
  }
  
  NtesukiRecord *record_child = table.allocateWithMove(record, best_move);
  if (record_child == 0)
  {
    result = ProofDisproof::Checkmate();
    RETURN;
  }
  ntesuki_assert(record_child);
  
  int pass_left_child = pass_left;
  if (best_move.isPass()) --pass_left_child;

  if (record_child->isVisited())
  {
    TRY_DFPN;
    result = ProofDisproof::LoopDetection();
    record->setLoopWithPath<attacker>(pass_left, path);
    record->setResult<attacker>(pass_left, NtesukiResult(1, 1),
				NtesukiMove::INVALID(), false);
    CATCH_DFPN;
    RETURN;
  }
  else if (!record_child_orig->getValue<attacker>(pass_left_child).isCheckmateFail())
  {
    result = ProofDisproof::Checkmate();
    RETURN;
  }

  const PathEncoding path_child(path, best_move.getMove());
  result = record_child->getValueWithPath<attacker>(pass_left_child, path_child);

  if (result.isUnknown())
  {
    DefenseHelperDisproof<NtesukiSimulationSearcher, P> helper(this,
							       record_child,
							       record_child_orig,
							       pass_left_child,
							       best_move.getMove());
    TRY_DFPN;
    ApplyMoveWithPath<P>::doUndoMoveOrPass(state, path, best_move.getMove(), helper);
    CATCH_DFPN;
    if (record->getValueWithPath<attacker>(pass_left, path).isFinal())
    {
      result = record->getValueWithPath<attacker>(pass_left, path);
      RETURN;
    }
  }

  if (result == ProofDisproof::LoopDetection())
  {
    TRY_DFPN;
    record->setLoopWithPath<attacker>(pass_left, path);
    record->setResult<attacker>(pass_left, NtesukiResult(1, 1),
				NtesukiMove::INVALID(), false);
    CATCH_DFPN;
    RETURN;
  }
  else if (result.isCheckmateFail())
  {
    NtesukiMove move(best_move.getMove());
    TRY_DFPN;
    move.setCheckmateFail<attacker>(pass_left);
    record->setResult<attacker>(pass_left, result, move, true);
    CATCH_DFPN;
    RETURN;
  }
  else
  {
    /* best_move is invalid : some rare casese, including
     * - the original state was invalid(attacker was under check)
     */
    //    ntesuki_assert(record_orig->getValue<attacker>(pass_left)
    //		   == ProofDisproof::AttackBack());
    result = ProofDisproof::Checkmate();
    RETURN;
  }
}

/* Start simulation to disproof, P as Attacker.
 * @return true, if nocheckmate is proven
 */
template <Player P>
bool
NtesukiSimulationSearcher::
startFromAttackDisproof(NtesukiRecord *record,
			const NtesukiRecord *record_orig,
			const unsigned int pass_left,
			const Move last_move)
{
  ++disproof_count;
  const Player attacker = P;
  ntesuki_assert(record_orig);
  if (!record_orig->getValue<attacker>(pass_left).isCheckmateFail())
    return false;

  TRY_DFPN;
  attackForDisproof<P>(record, record_orig, pass_left, last_move);
  CATCH_DFPN;
  if (result.isCheckmateFail())
  {
    ++disproof_success_count;
    return true;
  }
  return false;
}

/* Start simulation to disproof, P as Defender.
 * @return true, if nocheckmate is proven
 */
template <Player P>
bool
NtesukiSimulationSearcher::
startFromDefenseDisproof(NtesukiRecord *record,
			 const NtesukiRecord *record_orig,
			 const unsigned int pass_left,
			 const Move last_move)
{
  ntesuki_assert (P == state.turn());
  ++disproof_count;
  const Player attacker = PlayerTraits<P>::opponent;
  ntesuki_assert(record_orig);
  if (!record_orig->getValue<attacker>(pass_left).isCheckmateFail())
    return false;

  TRY_DFPN;
  defenseForDisproof<P>(record, record_orig, pass_left, last_move);
  CATCH_DFPN;
  if (result.isCheckmateFail())
  {
    ++disproof_success_count;
    return true;
  }
  return false;
}

#undef RETURN

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
