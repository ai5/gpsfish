/* oracleProverLight.tcc
 */
#include "osl/ntesuki/oracleProverLight.h"
#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
#include "osl/checkmate/fixedDepthSearcher.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
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

/* Helper classes
 */
template <class Searcher, Player P>
class
OracleProverLight::
AttackHelper
{
  Searcher* searcher;
  const NtesukiRecord *record_orig;
  unsigned int pass_left;
public:
  bool result;

  AttackHelper(Searcher* searcher,
	       const NtesukiRecord* record_orig,
	       unsigned int pass_left)
    : searcher(searcher), record_orig(record_orig), pass_left(pass_left)
  {}

  void operator()(Square p)
  {
    const Player O = PlayerTraits<P>::opponent;
    result = (*searcher).template defense<O>(record_orig, pass_left);
  }
};
      
template <class Searcher, Player P>
class OracleProverLight::
DefenseHelper
{
  Searcher* searcher;
  const NtesukiRecord *record_orig;
  unsigned int pass_left;

public:
  bool result;

  DefenseHelper(Searcher* searcher,
		const NtesukiRecord* record_orig,
		unsigned int pass_left)
    : searcher(searcher), record_orig(record_orig), pass_left(pass_left)
  {}

  void operator()(Square p)
  {
    const Player O = PlayerTraits<P>::opponent;
    result = (*searcher).template attack<O>(record_orig, pass_left);
  }
};

/* Utility
 */

/* Still cannot see if is safe move */
template <Player P>
static bool
is_safe_move(const osl::ntesuki::OracleProverLight::state_t state,
	     const osl::Move& m,
	     int pass_left)
{
  if (!m.isValid()) return false;
  if (!state.isValidMove(m, false)) return false;
  if (m.isDrop()) return true;
  return move_classifier::SafeMove<P>::isMember(state, m.ptype(), m.from(), m.to());
}

/* adjust the captuer ptype of a move */
template <Player P>
static osl::Move
adjustMove(const osl::ntesuki::OracleProverLight::state_t state,
	   osl::Move candidate)
{
  if (! candidate.isDrop())
  {
    const osl::Piece p = state.pieceOnBoard(candidate.to());
    candidate=setCapture(candidate,p);
  }
  return candidate;
}


/* The worker
 */

/* attack */
template <Player P>
bool
OracleProverLight::
attack(const NtesukiRecord* record_orig,
       const unsigned int pass_left)
{
  const Player attacker = P;
  ntesuki_assert(P == state.turn());

  if (!record_orig ||
      !record_orig->getValue<attacker>(pass_left)
      .isCheckmateSuccess() ||
      !record_orig->getBestMove<attacker>(pass_left).isValid())
  {
    return false;
  }

  Move check_move;
  FixedDepthSearcher fixed_searcher(state);

  if (!state.inCheck(P) &&
      fixed_searcher.hasCheckmateMove<P>(NtesukiRecord::fixed_search_depth,
					 check_move).isCheckmateSuccess())
  {
    /* Immediate Checkmate */
    return true;
  }

  /* Simulation 元が immediate checkmate ならこの先は simulate できない */
  const NtesukiMove best_move_orig = record_orig->getBestMove<attacker>(pass_left);
  if (best_move_orig.isImmediateCheckmate())
  {
    return false;
  }

  /* n が少ないときの結果を参照 */
  if ((pass_left > 0) && 
      record_orig->getValue<attacker>(pass_left - 1).isCheckmateSuccess())
  {
    return attack<P>(record_orig, pass_left - 1);
  }

  const Move move = adjustMove<P>(state, best_move_orig.move());

  /* invalid move となってしまった */
  if (!is_safe_move<P>(state, move, pass_left))
  {
    return false;
  }
  const bool move_is_check = (move_classifier::PlayerMoveAdaptor<move_classifier::Check>::
			      isMember(state, move));
  /* 即詰探索中は王手のみ読む */
  if(0 == pass_left && !move_is_check)
  {
    return false;
  }

  /* 以前は check だったのが今は違ってしまった・その逆 */
  if (best_move_orig.isCheck() != move_is_check)
  {
    return false;
  }

  /* 以前の bestMove を実行 */
  const NtesukiRecord* record_child_orig = table.findWithMoveConst(record_orig,
								   best_move_orig);
  if (!record_child_orig)
  {
    //ntesuki_assert (record_orig->isBySimulation());
    return false;
  }
  //ntesuki_assert(record_child_orig);

  AttackHelper<OracleProverLight, P> helper(this,
					    record_child_orig,
					    pass_left);
  TRY_DFPN;
  ApplyMoveWithPath<P>::doUndoMove(state, path, move, helper);
  CATCH_DFPN;

  return helper.result;
}

/* defense */
template <Player P>
bool OracleProverLight::
defense(const NtesukiRecord* record_orig,
	const unsigned int pass_left)
{
  const Player attacker = PlayerTraits<P>::opponent;

  ntesuki_assert(P == state.turn());
  if (!record_orig ||
      !record_orig->getValue<attacker>(pass_left)
      .isCheckmateSuccess())
  {
    return false;
  }

  /* 攻撃側に王手がかかっていないか調べる */
  if (state.inCheck(attacker))
  {
    return false;
  }

  /* 現在王手になっているかどうか */
  if ((pass_left == 0) &&
      !state.inCheck(P))
  {
    return false;
  }

  /* n が少ないときの結果を参照 */
  if (pass_left > 0 &&
      record_orig->getValue<attacker>(pass_left - 1).isCheckmateSuccess())
  {
    return defense<P>(record_orig, pass_left - 1);
  }

  /* 手の生成 */
  NtesukiMoveList moves;
  mg->generateSlow(P, state, moves);
  if (moves.empty()) return true;

  /* 受ける手の実行 */
  for (NtesukiMoveList::iterator move_it = moves.begin();
       move_it != moves.end(); move_it++)
  {
    NtesukiMove& move = *move_it;
    if (isscheme != NtesukiRecord::normal_is &&
	isscheme != NtesukiRecord::delay_is &&
	move.isCheck() && pass_left > 0) continue;

    ntesuki_assert(move.isPass() || move.isNormal());

    const NtesukiRecord *record_child_orig = table.findWithMoveConst(record_orig, move);
    if (!record_child_orig ||
	!record_child_orig->getValue<attacker>(pass_left).isCheckmateSuccess())
    {
      return false;
    }

    int pass_left_child = pass_left;
    if (move.isPass()) --pass_left_child;
    DefenseHelper<OracleProverLight, P> helper(this, record_child_orig, pass_left_child);
    TRY_DFPN;
    ApplyMoveWithPath<P>::doUndoMoveOrPass(state, path, move.move(), helper);
    CATCH_DFPN;
    if (false == helper.result) return false;
  }

  return true;
}

/* Publice interface
 */
template <Player P>
bool
OracleProverLight::
startFromAttack(NtesukiRecord* record,
		const NtesukiRecord* record_orig,
		const unsigned int pass_left)
{
  const Player attacker = P;
  if (!record || !record_orig) return false;
  
  if (!record->getPieceStand<attacker>().isSuperiorOrEqualTo
      (record_orig->getPDPieces<attacker>(pass_left)))
  {
    return false;
  }

  ntesuki_assert(record_orig->getValue<attacker>(pass_left).isCheckmateSuccess());

  if (attack<P>(record_orig, pass_left))
  {
#ifndef NDEBUG
    const NtesukiMove m = (record_orig->getBestMove<attacker>(pass_left));
    ntesuki_assert(m.isValid());
    ntesuki_assert(!m.isImmediateCheckmate());
#endif

    TRY_DFPN;
    const PieceStand ps = record->getPieceStand<attacker>();
    record->setResult<attacker>(pass_left, ProofDisproof::Checkmate(),
				record_orig->getBestMove<attacker>(pass_left),
				true, &ps);
    CATCH_DFPN;
    return true;
  }
  return false;
}

template <Player P>
bool
OracleProverLight::
startFromDefense(NtesukiRecord* record,
		 const NtesukiRecord* record_orig,
		 const unsigned int pass_left)
{
  const Player attacker = PlayerTraits<P>::opponent;
  if (!record || !record_orig) return false;

  if (!record->getPieceStand<attacker>().isSuperiorOrEqualTo
      (record_orig->getPDPieces<attacker>(pass_left)))
  {
    return false;
  }

  ntesuki_assert(record_orig->getValue<attacker>(pass_left).isCheckmateSuccess());

  if (defense<P>(record_orig, pass_left))
  {
    TRY_DFPN;
    const PieceStand ps = record->getPieceStand<attacker>();
    record->setResult<attacker>(pass_left, ProofDisproof::Checkmate(),
				record_orig->getBestMove<attacker>(pass_left),
				true, &ps);
    CATCH_DFPN;
    return true;
  }
  return false;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
