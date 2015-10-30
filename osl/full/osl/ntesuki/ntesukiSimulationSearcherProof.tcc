/* ntesukiSimulationProof.tcc
 */
#include "osl/ntesuki/ntesukiSimulationSearcher.h"
#include "osl/ntesuki/oracleProverLight.h"
#include "osl/ntesuki/ntesukiExceptions.h"
#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/container/moveVector.h"
#include "osl/move_classifier/safeMove.h"
#include "osl/apply_move/applyMoveWithPath.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/effect_util/effectUtil.h"
#ifdef NDEBUG
# include "osl/ntesuki/ntesukiMove.tcc"
# include "osl/ntesuki/ntesukiRecord.tcc"
//# include "osl/move_generator/escape.tcc"
#endif

using namespace osl;
using namespace osl::ntesuki;

#ifndef RETURN
#ifndef NDEBUG
#define RETURN \
  ntesuki_assert(result.isCheckmateSuccess() ==\
   record->getValueWithPath<A>(pass_left, path).isCheckmateSuccess());\
  if (record->getValueWithPath<A>(pass_left, path).proof() == 0)\
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path).disproof() > ProofDisproof::DISPROOF_LIMIT);\
  if (record->getValueWithPath<A>(pass_left, path).disproof() == 0)\
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path).proof() > ProofDisproof::PROOF_LIMIT);\
  return
#else
#define RETURN return
#endif
#endif


/* Helper classes
 */
template <class Searcher, Player P>
class
NtesukiSimulationSearcher::
AttackHelperProof
{
  Searcher* searcher;
  NtesukiRecord *record;
  const NtesukiRecord *record_orig;
  unsigned int pass_left;
  const Move last_move;
public:
  AttackHelperProof(Searcher* searcher,
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
    (*searcher).template defenseForProof<PlayerTraits<P>::opponent>
      (record, record_orig, pass_left, last_move);
  }
};
      
template <class Searcher, Player P>
class NtesukiSimulationSearcher::
DefenseHelperProof
{
  Searcher* searcher;
  NtesukiRecord *record;
  const NtesukiRecord *record_orig;
  unsigned int pass_left;
  const Move last_move;
public:
  DefenseHelperProof(Searcher* searcher,
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
    (*searcher).template attackForProof<PlayerTraits<P>::opponent>
      (record, record_orig, pass_left, last_move);
  }
};

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

/*======================================================================
 * Proof
 *======================================================================
 */

/**
 * Still cannot see if is safe move
 */
template <Player P>
bool
NtesukiSimulationSearcher::
isSafeMove(const Move move,
	   int pass_left)
{
  if (!state.isValidMove(move, false)) return false;

  if (move.isDrop()) return true;

  return move_classifier::SafeMove<P>::isMember(state, move.ptype(), move.from(), move.to());
}

template <Player P>
void
NtesukiSimulationSearcher::
attackForProof(NtesukiRecord* record,
	       const NtesukiRecord* record_orig,
	       const unsigned int pass_left,
	       const Move last_move)
{
  CountChildLock cclock(record, table);
  const Player A = P;
#ifndef NDEBUG
  const Player O = PlayerTraits<P>::opponent;
#endif
  ++node_count;
  ntesuki_assert(P == state.turn());

  ntesuki_assert(record);
  ntesuki_assert(!record->getValueWithPath<A>(pass_left, path).isFinal());
  ntesuki_assert(record->getBestMove<A>(pass_left).isInvalid());

  ntesuki_assert(record_orig);
  ntesuki_assert(record_orig->getValueWithPath<A>(pass_left, path).isCheckmateSuccess());
  ntesuki_assert(record_orig->getBestMove<A>(pass_left).isValid());

  ntesuki_assert(!state.inCheck(O));

  if (record->isVisited())
  {
    result = ProofDisproof::LoopDetection();
    RETURN;
  }

  /* 深さ固定 checkmate searcher を呼び出す */
  if (record->setUpNode<P>())
  {
    const NtesukiResult result_cur = record->getValueWithPath<A>(pass_left, path);
    if (result_cur.isCheckmateSuccess())
    {
      /* Immediate Checkmate */
      result = ProofDisproof::Checkmate();  
      RETURN;
    }
    else if (result_cur.isFinal())
    {
      result = result_cur;
      RETURN;
    }
  }

  /* Simulation 元が immediate checkmate ならこの先は simulate できない */
  const NtesukiMove best_move_orig = record_orig->getBestMove<A>(pass_left);
  if (best_move_orig.isImmediateCheckmate())
  {
    result = ProofDisproof::NoCheckmate();
    RETURN;
  }

  NtesukiRecord::VisitLock visitLock(record);
  /* n が少ないときの結果を参照 */
  if ((pass_left > 0) && 
      record_orig->getValueWithPath<A>(pass_left - 1, path)
      .isCheckmateSuccess())
  {
    if (record->getValueWithPath<A>(pass_left - 1, path)
	.isCheckmateFail())
    {
      result = ProofDisproof::NoCheckmate();
      RETURN;
    }
    
    ntesuki_assert(!record->getValueWithPath<A>(pass_left - 1, path)
		   .isFinal());
    NtesukiRecord::UnVisitLock unVisitLock(record);

    TRY_DFPN;
    attackForProof<P>(record, record_orig, pass_left - 1, last_move);
    CATCH_DFPN;
    RETURN;
  }

  const Move move = adjustMove<P>(best_move_orig.getMove());

  /* invalid move となってしまった */
  if (!move.isValid() ||
      !isSafeMove<P>(move, pass_left))
  {
    result = ProofDisproof::NoCheckmate();
    RETURN;
  }

  const bool move_is_check = (move_classifier::PlayerMoveAdaptor<move_classifier::Check>::
			      isMember(state, move));
  if (0 == pass_left && !move_is_check)
  {
    result = ProofDisproof::NoCheckmate();
    RETURN;
  }
  if (move_is_check != best_move_orig.isCheck())
  {
    result = ProofDisproof::NoCheckmate();
    RETURN;
  }

  /* 以前の bestMove を実行
   */
  NtesukiRecord *record_child = table.allocateWithMove(record, move);
  if (record_child == 0)
  {
    result = ProofDisproof::NoCheckmate();
    RETURN;
  }
  const NtesukiRecord* record_child_orig = table.findWithMoveConst(record_orig,
								   best_move_orig);
  if (!record_child_orig)
  {
    result = ProofDisproof::NoCheckmate();
    RETURN;
  }

  result = record_child->getValueWithPath<A>(pass_left, path);
  if (result.isUnknown())
  {
    AttackHelperProof<NtesukiSimulationSearcher, P> helper(this,
							   record_child,
							   record_child_orig,
							   pass_left,
							   move);
    TRY_DFPN;
    ApplyMoveWithPath<P>::doUndoMove(state, path, move, helper);
    if (record->getValueWithPath<A>(pass_left, path).isFinal())
    {
      result = record->getValueWithPath<A>(pass_left, path);
      RETURN;
    }
    CATCH_DFPN;
  }

  if (result.isPawnDropFoul(move))
  {
    result = ProofDisproof::PawnCheckmate();
    RETURN;
  }
  else if (result.isCheckmateSuccess())
  {
    result = ProofDisproof::Checkmate();
    NtesukiMove best_move(move);

    TRY_DFPN;
    best_move.setCheckmateSuccess<A>(pass_left);
    record->setResult<A>(pass_left, result, best_move, true);
    CATCH_DFPN;
    RETURN;
  }
  else if (result == ProofDisproof::LoopDetection())
  {
    result = ProofDisproof::NoCheckmate();
  }

  ntesuki_assert(result.isCheckmateFail());
  RETURN;
}

template <Player P>
void NtesukiSimulationSearcher::
defenseForProof(NtesukiRecord* record,
		const NtesukiRecord* record_orig,
		const unsigned int pass_left,
		const Move last_move)
{
  CountChildLock cclock(record, table);
  const Player A = PlayerTraits<P>::opponent;
  const Player O = PlayerTraits<P>::opponent;

  ++node_count;
  ntesuki_assert(P == state.turn());

  ntesuki_assert(record);
  if (!record_orig)
  {
    result = ProofDisproof::NoCheckmate();
    return;
  }

  if (state.inCheck(O))
  {
    //the previous move was a drop move that did not resolve a check
    result = ProofDisproof::NoCheckmate();
    return;
  }

  /* 深さ固定 checkmate searcher を呼び出す */
  if (record->setUpNode<P>())
  {
    result = record->getValueWithPath<A>(pass_left, path);
    if (result.isFinal())
    {
      return;
    }
  }

  /* 元のシナリオが間違っている */
  if (!record_orig->getValueWithPath<A>(pass_left, path).isCheckmateSuccess())
  {
    result = ProofDisproof::NoCheckmate();
    return;
  }

  if (record->isVisited())
  {
    result = ProofDisproof::LoopDetection();
    RETURN;
  }
  NtesukiRecord::VisitLock visitLock(record);

  /* 攻撃側に王手がかかっていないか調べる */
  const bool invalidAttack = state.inCheck(PlayerTraits<P>::opponent);
  if (invalidAttack)
  {
    result = ProofDisproof::AttackBack();
    RETURN;
  }

  /* n が少ないときの結果を参照 */
  if (pass_left > 0 &&
      record_orig->getValueWithPath<A>(pass_left - 1, path)
      .isCheckmateSuccess())
  {
    result = (record->getValueWithPath<A>(pass_left - 1, path));
    if(result.isCheckmateFail())
    {
      RETURN;
    }
    ntesuki_assert(!record->getValueWithPath<A>(pass_left - 1, path)
		   .isFinal());
    NtesukiRecord::UnVisitLock unVisitLock(record);

    TRY_DFPN;
    defenseForProof<P>(record, record_orig, pass_left - 1, last_move);
    CATCH_DFPN;
    RETURN;
  }

  /*
   * 受ける手の実行
   */
  /*
   * 守り側の手を生成する
   *  - 王手がかかっているか調べる(王手がかかっていた場合には逃げる手を生成する)
   *  - そうでないなら，通常の手生成を行う
   */
  NtesukiMoveList moves;
  record->generateMoves<P>(moves, 0, true);
  result = ProofDisproof::Checkmate();

  if (moves.empty())
  {
    result = ProofDisproof::NoEscape();
    TRY_DFPN;
    record->setResult<A>(pass_left, result,
			 NtesukiMove::INVALID(), false);
    
    CATCH_DFPN;
    RETURN;
  }

  bool some_moves_not_generated = false;
  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); move_it++)
  {
    NtesukiMove& move = *move_it;
    if (move.isCheckmateSuccess<A>(pass_left)) continue;

    /* 逆王手は読んでいない可能性がある */
    if (isscheme != NtesukiRecord::normal_is &&
	isscheme != NtesukiRecord::delay_is &&
	move.isCheck() && pass_left > 0) continue;

    /* シミュレーション元の子を探す */
    const NtesukiRecord* record_child_orig = table.findWithMoveConst(record_orig, move);
    if (!record_child_orig ||
	!record_child_orig->getValue<A>(pass_left).isCheckmateSuccess())
    {
      some_moves_not_generated = true;
      continue;
    }

    NtesukiRecord *record_child = table.allocateWithMove(record, move);
    if (record_child == 0)
    {
      result = ProofDisproof::NoCheckmate();
      RETURN;
    }

    if(record_child->isVisited())
    {
      result = ProofDisproof::LoopDetection();
      record->setLoopWithPath<A>(pass_left, path);
      TRY_DFPN;
      record->setResult<A>(pass_left, NtesukiResult(1, 1),
				  NtesukiMove::INVALID(), false);
      CATCH_DFPN;
      RETURN;
    }

    int pass_left_child = pass_left;
    if (move.isPass()) --pass_left_child;
    const PathEncoding path_child(path, move.getMove());
    result = record_child->getValueWithPath<A>(pass_left_child, path_child);
    if (result.isUnknown())
    {
      DefenseHelperProof<NtesukiSimulationSearcher, P> helper(this,
							      record_child,
							      record_child_orig,
							      pass_left_child,
							      move.getMove());
      TRY_DFPN;
      ApplyMoveWithPath<P>::doUndoMoveOrPass(state, path,
					     move.getMove(),
					     helper);
      CATCH_DFPN;
      if (record->getValueWithPath<A>(pass_left, path).isFinal())
      {
	result = record->getValueWithPath<A>(pass_left, path);
	RETURN;
      }
    }

    if (result.isCheckmateFail())
    {
      RETURN;
    }
  }

  ntesuki_assert(result.isCheckmateSuccess());

  if (some_moves_not_generated)
  {
    result = ProofDisproof::NoCheckmate();
  }
  else
  {
    TRY_DFPN;
    record->setResult<A>(pass_left, result,
			 NtesukiMove::INVALID(), true);
    CATCH_DFPN;
  }
  RETURN;
}

/* Public interface
 */
template <Player P>
bool
NtesukiSimulationSearcher::
startFromAttackProof(NtesukiRecord* record,
		     const NtesukiRecord* record_orig,
		     const unsigned int pass_left,
		     const Move last_move)
{
  assert(P == state.turn());

  const Player A = P;
  ntesuki_assert(record);
  if (!record_orig)
  {
    return false;
  }

  if (!record_orig->getValueWithPath<A>(pass_left, path).isCheckmateSuccess())
  {
    return false;
  }

  if (!record->isDominatedByProofPieces<A>(record_orig, pass_left))
  {
    return false;
  }

  TRY_DFPN;
  if (record->setUpNode<P>())
  {
    if (record->getValueWithPath<A>(pass_left, path).isCheckmateSuccess())
    {
      /* Immediate Checkmate */
      result = ProofDisproof::Checkmate();  
      return true;
    }
    else if (record->getValueWithPath<A>(pass_left, path).isCheckmateFail())
    {
      result = ProofDisproof::NoCheckmate();  
      return false;
    }
  }
  CATCH_DFPN;

  TRY_DFPN;
  const NtesukiMove m = (record_orig->getBestMove<A>(pass_left));
  if (m.isImmediateCheckmate()) return false;
  CATCH_DFPN;

  ++proof_count;

  TRY_DFPN;
  OracleProverLight light(state, mg, path, table,isscheme);
  if (light.startFromAttack<P>(record, record_orig, pass_left))
  {
    ++proof_success_count;
    ++light_proof_success_count;
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path)
		   .isCheckmateSuccess());
    return true;
  }
  else if (record->getValueWithPath<A>(pass_left, path).isCheckmateFail())
  {
    result = ProofDisproof::NoCheckmate();  
    return false;
  }
  CATCH_DFPN;

  TRY_DFPN;
  attackForProof<P>(record, record_orig, pass_left, last_move);
  CATCH_DFPN;
  if (result.isCheckmateSuccess())
  {
    ++proof_success_count;
    return true;
  }
  return false;
}

template <Player P>
bool
NtesukiSimulationSearcher::
startFromDefenseProof(NtesukiRecord* record,
		      const NtesukiRecord* record_orig,
		      const unsigned int pass_left,
		      const Move last_move)
{
  assert(P == state.turn());

  const Player A = PlayerTraits<P>::opponent;
  ntesuki_assert(record);
  if (!record_orig ||
      !record_orig->getValueWithPath<A>(pass_left, path).
      isCheckmateSuccess())
  {
    return false;
  }

  if (!record->isDominatedByProofPieces<A>(record_orig, pass_left))
  {
    return false;
  }

  if (record->setUpNode<P>())
  {
    if (record->getValueWithPath<A>(pass_left, path).isCheckmateSuccess())
    {
      /* Immediate Checkmate */
      result = ProofDisproof::Checkmate();  
      return true;
    }
    else if (record->getValueWithPath<A>(pass_left, path).isCheckmateFail())
    {
      result = ProofDisproof::NoCheckmate();  
      return false;
    }
  }

  const NtesukiMove m = (record_orig->getBestMove<A>(pass_left));
  if (m.isImmediateCheckmate()) return false;

  ++proof_count;

  OracleProverLight light(state, mg, path, table, isscheme);
  TRY_DFPN;
  if (light.startFromDefense<P>(record, record_orig, pass_left))
  {
    ++proof_success_count;
    ++light_proof_success_count;
    ntesuki_assert(record->getValueWithPath<A>(pass_left, path).
		   isCheckmateSuccess());
    return true;
  }
  else if (record->getValueWithPath<A>(pass_left, path).isCheckmateFail())
  {
    result = ProofDisproof::NoCheckmate();  
    return false;
  }
  CATCH_DFPN;

  TRY_DFPN;
  defenseForProof<P>(record, record_orig, pass_left, last_move);
  CATCH_DFPN;

  if (result.isCheckmateSuccess())
  {
    ++proof_success_count;
    return true;
  }
  return false;
}

#undef RETURN

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
