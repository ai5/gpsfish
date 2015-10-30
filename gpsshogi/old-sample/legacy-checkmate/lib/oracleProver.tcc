/* oracleProver.tcc
 */
#ifndef _ORACLEPROVER_TCC
#define _ORACLEPROVER_TCC

#include "oracleProver.h"
#include "checkHashTable.h"
#include "checkMoveGenerator.h"
#include "checkTableUtil.h"
#include "checkmateRecorder.h"
#include "osl/checkmate/proofPieces.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/checkmate/oracleAdjust.h"
#include "osl/apply_move/applyMoveWithPath.h"
#include "osl/effect_util/effectUtil.h"


// 不変条件: (! record->needMoveGeneration()) の時は手生成が終了していること
namespace osl
{
  namespace checkmate
  {
    /**
     * OracleProver::defense を呼ぶhelper
     */
    template <Player P, class Prover>
    struct OracleProverDefense
    {
      Prover *prover;
      CheckHashRecord *record;
      ProofOracleDefense<P> oracle;
      OracleProverDefense(Prover *p, 
			  CheckHashRecord *r, ProofOracleDefense<P> o)
	: prover(p), record(r), oracle(o)
      {
      }
      void operator()(Square)
      {
	(*prover).template defense<P>(record, oracle);
	record->isVisited = false;
      }
    };

    /**
     * OracleProver::attack を呼ぶhelper
     */
    template <Player P, class Prover>
    struct OracleProverAttack
    {
      Prover *prover;
      CheckHashRecord *record;
      ProofOracleAttack<P> oracle;
      OracleProverAttack(Prover *p, 
			 CheckHashRecord *r, ProofOracleAttack<P> o)
	: prover(p), record(r), oracle(o)
      {
      }
      void operator()(Square)
      {
	(*prover).template attack<P>(record, oracle);
	record->isVisited = false;
      }
    };

  } // namespace checkmate
} // namespace osl

template <class Table>
template <osl::Player P>
bool osl::checkmate::OracleProver<Table>::
proofWin(state_t& state, const HashKey& key, const PathEncoding& root_path, 
	 ProofOracleAttack<P> oracle, Move& best_move)
{
  check_assert(oracle.isValid());
  check_assert(state.turn() == P);
  path = root_path;
  this->state = &state;
  fixed_searcher.setState(state);
  CheckmateRecorder::setState(&state);
  this->key = key;

  CheckHashRecord *record = table.find(key);
  if (! record)
  {
    const PieceStand white_stand(WHITE, state);
    CheckHashRecord *root = table.root();
    root->distance = path.getDepth()-1;
    CheckTableUtil::allocate(record, table, key, white_stand, path, root);
  }
  if (! record->proofDisproof().isFinal()
      && (! record->findLoop(path, table.getTwinTable())))
  {
    const bool visitedBeforeAttack = record->isVisited;
    CheckmateRecorder::setNextMove(0);
    record->isVisited = false;
    attack<P>(record, oracle);
    record->isVisited = visitedBeforeAttack;
  }
  if (record->proofDisproof().isCheckmateSuccess())
  {
    check_assert(record->hasBestMove());
    best_move = record->getBestMove()->move;
    check_assert(best_move.isValid());
    check_assert(best_move.player() == P);
    assert(state.isValidMove(best_move));
    return true;
  }
  return false;
}

template <class Table>
template <osl::Player P>
bool osl::checkmate::OracleProver<Table>::
proofLose(state_t& state, const HashKey& key, const PathEncoding& root_path, 
	  ProofOracleDefense<P> oracle, Move last_move)
{
  check_assert(oracle.isValid());
  check_assert(state.turn() == alt(P));
  path = root_path;
  this->state = &state;
  fixed_searcher.setState(state);
  CheckmateRecorder::setState(&state);
  this->key = key;

  CheckHashRecord *record = table.find(key);
  if (! record)
  {
    const PieceStand white_stand(WHITE, state);
    CheckHashRecord *root = table.root();
    root->distance = path.getDepth()-1;
    CheckTableUtil::allocate(record, table, key, white_stand, path, root);
  }
  if (record->findLoop(path, table.getTwinTable()))
    return false;
  if (! record->proofDisproof().isFinal())
  {
    const bool visitedBeforeDefense = record->isVisited;
    CheckmateRecorder::setNextMove(0);
    record->isVisited = false;
    defense<P>(record, oracle);
    record->isVisited = visitedBeforeDefense;
  }
  if (record->proofDisproof().isPawnDropFoul(last_move))
    return false;
  return record->proofDisproof().isCheckmateSuccess();
}

template <class Table>
template <osl::Player P>
void osl::checkmate::OracleProver<Table>::
testFixedDepthAttack(CheckHashRecord *record, Move guide)
{
  PieceStand proof_pieces;
  ProofDisproof pdp;
  if (guide.isNormal())
    pdp = fixed_searcher.hasCheckmateWithGuide<P>(2, guide, proof_pieces);
  else
    pdp = fixed_searcher.hasCheckmateMove<P>(2, guide, proof_pieces);
  if (pdp.isCheckmateSuccess())
  {
    CheckMove *attack = record->moves.find(guide);
    if (attack == 0) {
      CheckMove best_move(guide);
      record->moves.setOne(best_move, table.listProvider());
      attack = &*(record->moves.begin());
    }
    attack->flags.set(MoveFlags::ImmediateCheckmate);
    record->bestMove = attack;
    record->setProofPieces(proof_pieces);
    record->propagateCheckmate<P>(ProofDisproof::Checkmate());
  }
  else if (pdp.isCheckmateFail())
  {
    // 反証駒があれば登録できる
  }
  else
  {
    assert(pdp.isUnknown());
    record->setProofDisproof(std::max(pdp.proof(), record->proof()),
			     std::max(pdp.disproof(), record->disproof()));
  }
}

template <class Table>
template <osl::Player P>
void osl::checkmate::OracleProver<Table>::
testFixedDepthDefense(CheckHashRecord *record, CheckMove& next_move)
{
  PieceStand proof_pieces;
  Move check_move;
  const ProofDisproof pdp
    = fixed_searcher.hasEscapeByMove<P>(next_move.move, 2, check_move, 
					proof_pieces);
  if (pdp.isCheckmateSuccess())
  {
    CheckTableUtil::registerImmediateCheckmateInDefense<P,Table>
	(key, path, record, next_move, pdp, check_move, proof_pieces, table);
  }
  else if (pdp.isCheckmateFail())
  {
    // 反証駒があれば登録できる
  }
  else
  {
    assert(pdp.isUnknown());
    record->setProofDisproof(std::max(pdp.proof(), record->proof()),
			     std::max(pdp.disproof(), record->disproof()));
  }
}

template <class Table>
template <osl::Player P>
void osl::checkmate::OracleProver<Table>::
attack(CheckHashRecord *record, ProofOracleAttack<P> oracle)
{
  ++node_count;
#ifndef NDEBUG
  CheckmateRecorder::Tracer tracer("oracle attack ", record, 
				   key, path, 0, 0);
#endif
  check_assert(state->turn() == P);
  check_assert(record);
  check_assert(! record->proofDisproof().isFinal());
  check_assert(! record->findLoop(path, table.getTwinTable()));
  check_assert(! record->isVisited);
  check_assert(oracle.isValid());
  record->isVisited = true;
#ifdef CHECKMATE_DEBUG
  const Square target_king_square
    =(*state).template kingSquare<PlayerTraits<P>::opponent>();
  check_assert(! state->hasEffectAt(P,target_king_square)); // 逃げる手になっていなかった
#endif
  if (record->needMoveGeneration())
  {
    Move check_move;
    if ((! state->inCheck())
	&& ImmediateCheckmate::hasCheckmateMove<P>(*state, check_move))
    {
      CheckMove best_move(check_move);
      best_move.flags.set(MoveFlags::ImmediateCheckmate);
      record->moves.setOne(best_move, table.listProvider());
      record->bestMove = &*(record->moves.begin());
      PieceStand proof_pieces;
      if (check_move.isDrop())
	proof_pieces.add(check_move.ptype());
      record->setProofPieces(proof_pieces);
      record->propagateCheckmate<P>(ProofDisproof::Checkmate());
      return;
    }
    if (oracle.guide && oracle.guide->hasBestMove() 
	&& oracle.guide->getBestMove()->flags.isSet(MoveFlags::ImmediateCheckmate))
    {
      testFixedDepthAttack<P>(record, oracle.guide->getBestMove()->move);
      return;
    }
  }
  ProofOracleDefense<P> new_oracle = oracle.expandOracle();
  if (! new_oracle.isValid()) {
    testFixedDepthAttack<P>(record, Move::INVALID());
    return;
  }
  check_assert(oracle.oracle().player() == P);
  Move check_move = OracleAdjust::attack(*state, oracle.oracle());
  if (! check_move.isNormal())
    return;
  bool has_pawn_checkmate=false;
  if (record->moves.empty()) {
    CheckMoveGenerator<P>::generateAttack(*state, table.listProvider(),
					  record->moves, has_pawn_checkmate);
    if (has_pawn_checkmate)
      record->updateBestResultInSolvedAttack(ProofDisproof::PawnCheckmate());
  }
  
  record->bestMove = 0;
  for (CheckMoveList::iterator p=record->moves.begin();
       p!=record->moves.end(); ++p)
  {
    if (p->move == check_move)
    {
      if (! p->flags.isSet(MoveFlags::Solved))
	record->bestMove = &*p;
      break;
    }
  }
  unsigned int best_move_proof = 1;
  unsigned int best_move_disproof = 1;
  HashKey new_key;
  PathEncoding new_path;

  if (! record->hasBestMove()) // 合法な王手でなかった場合など
    goto failure_end;

  check_assert(record->hasBestMove()
	       && (record->getBestMove()->move == check_move));
  new_key = key.newHashWithMove(check_move);
  new_path = PathEncoding(path, check_move);
  
  if (! record->getBestMove()->record)
  {
    CheckTableUtil::allocate(check_move, record->getBestMove()->record, table,
			     new_key, new_path, record);
    if (record->proofDisproof().isFinal())
    {
      if (record->proofDisproof().isCheckmateSuccess())
	check_assert(record->hasBestMove());
      return;
    }
  }

  if ((! record->getBestMove()->record->isVisited)
      && (! record->getBestMove()->findLoop(path, table.getTwinTable())))
  {
    const ProofDisproof& pdp = record->getBestMove()->record->proofDisproof();
    if (! pdp.isFinal())
    {
      OracleProverDefense<P,OracleProver> 
	oracle_prover(this, record->getBestMove()->record, new_oracle);
      CheckmateRecorder::setNextMove(&*record->getBestMove());

      std::swap(key, new_key);
      std::swap(path, new_path);
      
      ApplyMove<P>::doUndoMove(*state, check_move, oracle_prover);

      key = new_key;
      path = new_path;

      if (record->proofDisproof().isFinal())
      {
	if (record->proofDisproof().isCheckmateSuccess())
	  check_assert(record->hasBestMove());
	return;
      }
    }
    if (pdp.isCheckmateSuccess() && (! pdp.isPawnDropFoul(check_move)))
    {
      if (! record->proofDisproof().isCheckmateSuccess()) 
      {
	check_assert(record->bestMove->record->hasProofPieces());
	record->setProofPiecesAttack(P);
	record->propagateCheckmate<P>(ProofDisproof::Checkmate());
	check_assert(record->isConsistent());
      }
      return;
    }
    if (! pdp.isFinal())
    {
      // 打歩詰か不詰は除く
      best_move_proof = pdp.proof();
      best_move_disproof = pdp.disproof();
    }
  }
  record->bestMove = 0;
failure_end:
  if (record->moves.empty())
  {
    record->setDisproofPieces(DisproofPieces::leaf(*state, alt(P), 
						   record->stand(alt(P))));
    record->propagateNoCheckmate<P>(has_pawn_checkmate 
				    ? ProofDisproof::PawnCheckmate()
				    : ProofDisproof::NoCheckmate());
  }
  else
  {
    check_assert(best_move_proof < ProofDisproof::PROOF_LIMIT);
    check_assert(best_move_disproof < ProofDisproof::DISPROOF_LIMIT);
    check_assert(! record->proofDisproof().isFinal());
    record->setProofDisproof(std::max(record->proof(),
				      best_move_proof/4), 
			     std::max((size_t)record->disproof(),
				      best_move_disproof + record->moves.size()));
  }
  return;
}

template <class Table>
template <osl::Player P>
void osl::checkmate::OracleProver<Table>::
defense(CheckHashRecord *record, ProofOracleDefense<P> oracle)
{
  ++node_count;
#ifndef NDEBUG
  CheckmateRecorder::Tracer tracer("oracle defense", record, 
				   key, path, 0, 0);
#endif
  check_assert(oracle.isValid());
  check_assert(record);
  check_assert(! record->findLoop(path, table.getTwinTable()));
  check_assert(! record->proofDisproof().isFinal());
  check_assert(! record->isVisited);
  record->isVisited = true;
#ifndef NDEBUG
  const Square attack_king_position=(*state).template kingSquare<P>();
  const Square defense_king_position=(*state).template kingSquare<PlayerTraits<P>::opponent>();
  // 王手をかけたら自玉が取られる状態 or 王手になっていなかった場合
  // 今はないはず
  check_assert((attack_king_position.isPieceStand()
		|| ! state->hasEffectAt(alt(P),attack_king_position))
	       && state->hasEffectAt(P,defense_king_position));
#endif
  if (record->needMoveGeneration())
  {
    CheckMoveGenerator<P>::generateEscape(*state, table.listProvider(),
					  record->moves);
    if (record->moves.empty()) 
    {
      record->setProofPieces(ProofPieces::leaf(*state, P, record->stand(P)));
      record->propagateCheckmate<P>(ProofDisproof::NoEscape());
      check_assert(record->isConsistent());
      return;
    }
    check_assert(record->moves.size());
    record->setProofDisproof(std::max((size_t)record->proofDisproof().proof(),
				      record->moves.size()), 
			     record->proofDisproof().disproof());
  }
examine_moves:
  int proved = 0;		// このループで証明が判明した数
  unsigned int sum_proof = 0;	// 証明失敗があったかどうかにも使う
  unsigned int min_disproof = ProofDisproof::DISPROOF_LIMIT;
  for (CheckMoveList::iterator p=record->moves.begin(); 
       p!=record->moves.end(); ++p)
  {
    check_assert(p->move.player() == alt(P));
    if (! record->filter.isTarget(p->flags)) // 中合など
      continue;

    HashKey new_key = key.newHashWithMove(p->move);
    PathEncoding new_path(path, p->move);
    if (! p->record)
    {
      CheckTableUtil::allocate(p->move, p->record, table, 
			       new_key, new_path, record);
    }
    if (p->record->isVisited)
    {
      record->bestMove = &*p;
      record->setLoopDetection(path, *p, p->record);
      return;
    }
    if (const TwinEntry *loop = p->findLoop(path, table.getTwinTable()))
    {
      record->bestMove = &*p;
      record->setLoopDetectionTryMerge<P>(path, *p, loop->loopTo);
      return;
    }
    const ProofDisproof& pdp = p->record->proofDisproof();
    if (! pdp.isFinal())
    {
      ProofOracleAttack<P> new_oracle = oracle.expandOracle(p->move);
      if (! new_oracle.isValid()) {
	testFixedDepthDefense<P>(record, *p);
	if (record->proofDisproof().isCheckmateSuccess())
	  continue;
	return;
      }
      OracleProverAttack<P,OracleProver> 
	oracle_prover(this, p->record, new_oracle);
      CheckmateRecorder::setNextMove(&*p);

      std::swap(key, new_key);
      std::swap(path, new_path);

      ApplyMove<PlayerTraits<P>::opponent>::
	doUndoMove(*state, p->move, oracle_prover);

      key = new_key;
      path = new_path;
    }
    if (! pdp.isCheckmateSuccess())
    {
      record->bestMove = &*p;
      if (pdp.isCheckmateFail())
      {
	if (! record->proofDisproof().isCheckmateFail())
	{
	  assert(! pdp.isLoopDetection());
	  record->setDisproofPiecesDefense(alt(P));
	  record->propagateNoCheckmate<P>(pdp);
	}
	return;
      }
      if (const TwinEntry *loop = p->findLoop(path, table.getTwinTable()))
      {
	record->bestMove = &*p;
	record->setLoopDetectionTryMerge<P>(path, *p, loop->loopTo);
	return;
      }
      sum_proof += pdp.proof();
      min_disproof = std::min(min_disproof, pdp.disproof());
      continue;
    }
    ++proved;
    record->addToSolvedInDefense(*p, p->record->proofDisproof());
  }
  if (sum_proof && (proved == 0))
  {
    CheckmateRecorder::setLeaveReason("no checkmate found");
    assert(min_disproof < ProofDisproof::DISPROOF_LIMIT);
    assert(sum_proof < ProofDisproof::PROOF_LIMIT);
    check_assert(! record->proofDisproof().isFinal());
    record->setProofDisproof(std::max(record->proof(), sum_proof),
			     std::max(record->disproof(), min_disproof));
    return;
  }
  if (sum_proof)
    goto examine_moves;
  if (! record->filter.isTarget(MoveFlags::BlockingBySacrifice))
  {
    record->filter.addTarget(MoveFlags::BlockingBySacrifice);
    goto examine_moves;
  }
  if (! record->filter.isTarget(MoveFlags::Upward))
  {
    record->filter.addTarget(MoveFlags::Upward);
    record->useMaxInsteadOfSum = true;
    goto examine_moves;
  }

  if (! record->proofDisproof().isCheckmateSuccess())
  {
    record->setProofPieces(ProofPieces::defense(record->moves, *state,
						record->stand(P)));
    record->propagateCheckmate<P>(record->bestResultInSolved);
  }
  check_assert(record->proofDisproof().isCheckmateSuccess());
  check_assert(record->isConsistent());
  return;
}

template <class Table>
bool osl::checkmate::OracleProver<Table>::
proofWin(state_t& state, const PathEncoding& path,
	 const CheckHashRecord *record, Move& best_move)
{
  const HashKey key(state);
  if (state.turn() == BLACK)
  {
    ProofOracleAttack<BLACK> oracle(record);
    return proofWin(state, key, path, oracle, best_move);
  }
  else
  {
    ProofOracleAttack<WHITE> oracle(record);
    return proofWin(state, key, path, oracle, best_move);
  }
}

template <class Table>
bool osl::checkmate::OracleProver<Table>::
proofLose(state_t& state, const PathEncoding& path,
	  const CheckHashRecord *record, Move last_move)
{
  const HashKey key(state);
  if (state.turn() == BLACK)
  {
    ProofOracleDefense<WHITE> oracle(record);
    return proofLose(state, key, path, oracle, last_move);
  }
  else
  {
    ProofOracleDefense<BLACK> oracle(record);
    return proofLose(state, key, path, oracle, last_move);
  }
}


#endif /* _ORACLEPROVER_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
