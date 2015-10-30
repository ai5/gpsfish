/* oracleDisprover.tcc
 */
#ifndef _ORACLEDISPROVER_TCC
#define _ORACLEDISPROVER_TCC

#include "oracleDisprover.h"
#include "checkHashTable.h"
#include "checkMoveGenerator.h"
#include "checkTableUtil.h"
#include "checkmateRecorder.h"
#include "sameBoardList.h"
#include "osl/checkmate/proofPieces.h"
#include "osl/apply_move/applyMoveWithPath.h"
#include <iostream>

// 不変条件: (! record->needMoveGeneration()) の時は手生成が終了していること
namespace osl
{
  namespace checkmate
  {
    /**
     * OracleDisprover::defense を呼ぶhelper
     * @param P 攻撃側
     */
    template <Player P, class Disprover>
    struct OracleDisproverDefense
    {
      Disprover *prover;
      CheckHashRecord *record;
      const DisproofOracleDefense<P>& oracle;
      OracleDisproverDefense(Disprover *p, CheckHashRecord *r, 
			     const DisproofOracleDefense<P>& o)
	: prover(p), record(r), oracle(o)
      {
      }
      void operator()(Square)
      {
	assert(! record->isVisited);
	(*prover).template defense<P>(record, oracle);
	record->isVisited = false;
      }
    };

    /**
     * OracleDisprover::attack を呼ぶhelper
     * @param P 攻撃側
     */
    template <Player P, class Disprover>
    struct OracleDisproverAttack
    {
      Disprover *prover;
      CheckHashRecord *record;
      const DisproofOracleAttack<P>& oracle;
      OracleDisproverAttack(Disprover *p, CheckHashRecord *r, 
			    const DisproofOracleAttack<P>& o)
	: prover(p), record(r), oracle(o)
      {
      }
      void operator()(Square)
      {
	assert(! record->isVisited);
	(*prover).template attack<P>(record, oracle);
	record->isVisited = false;
      }
    };

  } // namespace checkmate
} // namespace osl

template <class Table>
template <osl::Player P>
bool osl::checkmate::OracleDisprover<Table>::
proofNoCheckmate(NumEffectState& state, 
		 const HashKey& key, const PathEncoding& root_path,
		 const DisproofOracleAttack<P>& oracle)
{
  check_assert(oracle.isValid());
  check_assert(state.turn() == P);
  path = root_path;
  this->state = &state;
  this->key = key;
  CheckmateRecorder::setState(&state);
  CheckHashRecord *record = table.find(key);
  if (! record)
  {
    const PieceStand white_stand(WHITE, state);
    CheckHashRecord *root = table.root();
    root->distance = path.getDepth()-1;
    CheckTableUtil::allocate(record, table, key, white_stand, path, root);
  }

  if ((! record->proofDisproof().isFinal())
      && (! record->findLoop(path, table.getTwinTable())))
  {
    const bool visitedBeforeAttack = record->isVisited;
    CheckmateRecorder::setNextMove(0);
    record->isVisited = false;
    attack<P>(record, oracle);
    record->isVisited = visitedBeforeAttack;
  }
  if (record->proofDisproof().isCheckmateFail()
      || record->findLoop(path, table.getTwinTable()))
    return true;

  return false;
}

template <class Table>
template <osl::Player P>
bool osl::checkmate::OracleDisprover<Table>::
proofEscape(NumEffectState& state, 
	    const HashKey& key, const PathEncoding& root_path,
	    const DisproofOracleDefense<P>& oracle, 
	    Move& best_move, Move last_move)
{
  check_assert(oracle.isValid());
  check_assert(state.turn() == alt(P));
#ifdef CHECKMATE_DEBUG
  check_assert(oracle.guide->proofDisproof().isCheckmateFail()
	       || oracle.guide->findLoop(oracle.path, table.getTwinTable())
	       || ((! oracle.guide->twins.empty())
		   && oracle.guide->hasBestMove()
		   && oracle.guide->getBestMove()->findLoop(oracle.path, table.getTwinTable()))
	       || oracle.guide->proofDisproof() == ProofDisproof::NoEscape()
	       || (oracle.guide->dump(), 0));
#endif
  path = root_path;
  this->state = &state;
  this->key = key;
  CheckmateRecorder::setState(&state);
  CheckHashRecord *record = table.find(key);
  if (! record)
  {
    const PieceStand white_stand(WHITE, state);
    CheckHashRecord *root = table.root();
    root->distance = path.getDepth()-1;
    CheckTableUtil::allocate(record, table, key, white_stand, path, root);
  }

  if ((! record->proofDisproof().isFinal())
      && (! record->findLoop(path, table.getTwinTable())))
  {
    const bool visitedBeforeDefense = record->isVisited;
    CheckmateRecorder::setNextMove(0);
    record->isVisited = false;
    defense<P>(record, oracle);
    record->isVisited = visitedBeforeDefense;
  }
  if (record->proofDisproof().isCheckmateFail()
      || record->findLoop(path, table.getTwinTable()))
  {
    if (record->hasBestMove())
    {
      best_move = record->getBestMove()->move;
      check_assert(best_move.isValid());
      check_assert(best_move.player() == alt(P));
    }
    else
    {
      check_assert(record->findLoop(path, table.getTwinTable()));
    }
    return true;
  }
  if (record->proofDisproof().isPawnDropFoul(last_move))
    return true;
  return false;
}

template <class Table>
template <osl::Player P>
void osl::checkmate::OracleDisprover<Table>::
attack(CheckHashRecord *record, const DisproofOracleAttack<P>& oracle)
{
  ++node_count;
#ifndef NDEBUG
  CheckmateRecorder::Tracer tracer("disproof oracle attack ", record, 
				   key, path, 0, 0);
#endif
  check_assert(oracle.isValid());
  check_assert(state->turn() == P);
  check_assert(record);
  check_assert(! record->isVisited);
  check_assert(! record->findLoop(path, table.getTwinTable()));
  record->isVisited = true;
  check_assert(! record->proofDisproof().isFinal());

#ifndef NDEBUG
  const Square target_king_square
    =(*state).template kingSquare<PlayerTraits<P>::opponent>();
  // 逃げる手になっていなかった場合
  check_assert(! state->hasEffectAt(P,target_king_square));
#endif

  if (record->sameBoards)
  {
    const CheckHashRecord *loop = 
      (*record->sameBoards).template 
      findIneffectiveDropLoop<P>(key.blackStand());
    if (loop)
    {
      record->setLoopDetection(path, loop);
      CheckmateRecorder::setLeaveReason("loop confirmation (drop)");
      return;
    }
  }
  
  if (record->needMoveGeneration())
  {
    bool has_pawn_checkmate=false;
    CheckMoveGenerator<P>::generateAttack(*state, table.listProvider(),
					  record->moves, has_pawn_checkmate);
    if (record->moves.empty()) 
    {
      record->setDisproofPieces(DisproofPieces::leaf(*state, alt(P), 
						     record->stand(alt(P))));
      record->propagateNoCheckmate<P>(has_pawn_checkmate
				      ? ProofDisproof::NoCheckmate()
				      : ProofDisproof::PawnCheckmate());
      CheckmateRecorder::setLeaveReason("no attack moves");
      return;
    }
    if (has_pawn_checkmate)
      record->updateBestResultInSolvedAttack(ProofDisproof::PawnCheckmate());
    check_assert(record->moves.size());
    record->setProofDisproof(1, record->moves.size());
  }
  record->filter = MoveFilter(); // danger?
  ProofDisproof bestResultInSolved = record->bestResultInSolved;
examine_moves:
  int disproved = 0;
  unsigned int sum_disproof = 0;	// 証明失敗があったかどうかにも使う
  unsigned int min_proof = ProofDisproof::PROOF_LIMIT;

  HashKey new_key;
  PathEncoding new_path;

  for (CheckMoveList::iterator p=record->moves.begin(); 
       p!=record->moves.end(); ++p)
  {
    check_assert(p->move.player() == P);
    if (! record->filter.isTarget(p->flags)) // 不成など
      continue;
    new_key = key.newHashWithMove(p->move);
    new_path = PathEncoding(path, p->move);
    if (! p->record)
    {
      CheckTableUtil::allocate(p->move, p->record, table, 
			       new_key, new_path, record);
    }
    
    if (const TwinEntry *loop 
	= p->findLoop(path, table.getTwinTable()))
    {
#ifdef PAWN_CHECKMATE_SENSITIVE
      if ((record->bestResultInSolved != ProofDisproof::PawnCheckmate())
	  && loop->move.record
	  && (loop->move.record->bestResultInSolved == ProofDisproof::PawnCheckmate()))
      {
	record->bestResultInSolved 
	  = record->bestResultInSolved.betterForAttack(ProofDisproof::PawnCheckmate());
      }
#endif
      bestResultInSolved
	= bestResultInSolved.betterForAttack(ProofDisproof::LoopDetection());
      continue;
    }
    if (p->record->isVisited)
    {
      if (p->record->proofDisproof() == ProofDisproof::PawnCheckmate())
	record->bestResultInSolved 
	  = record->bestResultInSolved.betterForAttack(ProofDisproof::PawnCheckmate());
      bestResultInSolved
	= bestResultInSolved.betterForAttack(ProofDisproof::LoopDetection());
      continue;
    }
    const ProofDisproof& pdp = p->record->proofDisproof();
    if (! pdp.isFinal())
    {
      DisproofOracleDefense<P> new_oracle = oracle.expandOracle(p->move);
      if (! new_oracle.isValid())
      {
	CheckmateRecorder::setLeaveReason("invalid oracle");
#ifdef CHECKMATE_DEBUG
	CheckmateRecorder::DepthTracer::stream() << p->move << "\n";
#endif
	return;
      }
      if (! pdp.isFinal())
      {
	OracleDisproverDefense<P,OracleDisprover> 
	  oracle_disprover(this, p->record, new_oracle);
	CheckmateRecorder::setNextMove(&*p);

	std::swap(key, new_key);
	std::swap(path, new_path);
	ApplyMove<P>::doUndoMove(*state, p->move,oracle_disprover);
	key = new_key;
	path = new_path;
      }
    }
    if (pdp.isCheckmateFail())
    {
      check_assert(! pdp.isLoopDetection());
      record->addToSolvedInAttack(*p, pdp);
      ++disproved;
      continue;
    }
    if (pdp.isPawnDropFoul(p->move))
    {
      record->addToSolvedInAttack(*p, ProofDisproof::PawnCheckmate());
      ++disproved;
      continue;
    }
    if (p->findLoop(path, table.getTwinTable()))
    {
      bestResultInSolved
	= bestResultInSolved.betterForAttack(ProofDisproof::LoopDetection());
      ++disproved;
      continue;
    }
    assert(! pdp.isCheckmateFail());
    record->bestMove = &*p;
    if (pdp.isCheckmateSuccess())
    {
      CheckmateRecorder::setLeaveReason("checkmate move found in attack");
#ifdef CHECKMATE_DEBUG
      CheckmateRecorder::DepthTracer::stream() << p->move << "\n";
#endif
      if (! record->proofDisproof().isCheckmateSuccess())
      {
	record->setProofPiecesAttack(P);
	record->propagateCheckmate<P>(pdp);
	check_assert(record->isConsistent());
      }
      return;
    }
    sum_disproof += pdp.disproof();
    min_proof = std::min(min_proof, pdp.proof());
  }
  if (sum_disproof && (disproved == 0))
  {
    CheckmateRecorder::setLeaveReason("no new nocheckmate found");
    assert(min_proof < ProofDisproof::PROOF_LIMIT);
    assert(sum_disproof < ProofDisproof::DISPROOF_LIMIT);
    check_assert(! record->proofDisproof().isFinal());
    record->setProofDisproof(std::max(record->proof(), min_proof),
			     std::max(record->disproof(), sum_disproof));
    return;
  }
  if (sum_disproof)
    goto examine_moves;
#ifdef DELAY_SACRIFICE
  if (! record->filter.isTarget(MoveFlags::SacrificeAttack))
  {
    record->filter.addTarget(MoveFlags::SacrificeAttack);
    goto examine_moves;
  }
#endif
  if ((record->bestResultInSolved == ProofDisproof::PawnCheckmate())
      && (! record->filter.isTarget(MoveFlags::NoPromote)))
  {
    if (! oracle.guide->filter.isTarget(MoveFlags::NoPromote))
    {
      CheckmateRecorder::setLeaveReason("NoPromote in oracle");
      return;
    }
    record->filter.addTarget(MoveFlags::NoPromote);
    goto examine_moves;
  }
  if (! record->filter.isTarget(MoveFlags::Upward))
  {
    record->filter.addTarget(MoveFlags::Upward);
    record->useMaxInsteadOfSum = true;
    goto examine_moves;
  }
  bestResultInSolved = 
    bestResultInSolved.betterForAttack(record->bestResultInSolved);
  check_assert(bestResultInSolved.isCheckmateFail());
  if (bestResultInSolved.isLoopDetection())
  {
    if (oracle.guide == record)
    {
      // twin simulation
      assert(! record->twins.empty());
      table.addLoopDetection(path);
    }
    else
    {
      record->setLoopDetectionInAttack<P>(path);
    }
  }
  else
  {
    record->setDisproofPieces(DisproofPieces::attack(record->moves, *state, 
						     record->stand(alt(P))));
    record->propagateNoCheckmate<P>(record->bestResultInSolved);
    record->twins.clear();
  }
  return;
}

namespace osl
{
  namespace checkmate
  {
    template <class State, Player Attacker>
    struct ConfirmNoEscape
    {
      State *state;
      bool *result;
      ConfirmNoEscape(State *s, bool *r) : state(s), result(r)
      {
      }
      void operator()(Square)
      {
	const Square defense_king_position
	  = (*state).template kingSquare<PlayerTraits<Attacker>::opponent>();
	*result = state->hasEffectAt(Attacker, defense_king_position);
      }
    };
  } // namespace checkmate
} // namespace osl

template <class Table>
template <osl::Player P>
void osl::checkmate::OracleDisprover<Table>::
confirmNoEscape(CheckHashRecord *record)
{
  if (record->moves.empty())
    CheckMoveGenerator<P>::generateEscape(*state, table.listProvider(),
					  record->moves);
  if (record->moves.empty()) 
  {
    record->setProofPieces(ProofPieces::leaf(*state, P, record->stand(P)));
    record->propagateCheckmate<P>(ProofDisproof::NoEscape());
    check_assert(record->isConsistent());
    return;
  }
  // safe move ならここの処理は不要
}

template <class Table>
template <osl::Player P>
void osl::checkmate::OracleDisprover<Table>::
defense(CheckHashRecord *record, const DisproofOracleDefense<P>& oracle)
{
  ++node_count;
#ifndef NDEBUG
  CheckmateRecorder::Tracer tracer("disproof oracle defense", record, 
				   key, path, 0, 0);
#endif
  check_assert(oracle.isValid());
  check_assert(record);
  check_assert(! record->isVisited);
  record->isVisited = true;
  check_assert(! record->proofDisproof().isFinal());
  check_assert(! record->findLoop(path, table.getTwinTable()));
#ifdef CHECKMATE_DEBUG
  const Square attack_king_position=(*state).template kingSquare<P>();
  const Square defense_king_position=(*state).template kingSquare<PlayerTraits<P>::opponent>();
  check_assert(attack_king_position.isPieceStand()
	       || ! state->hasEffectAt(alt(P),attack_king_position)); // 王手をかけたら自玉が取られる
  check_assert(state->hasEffectAt(P,defense_king_position)); // 王手になってない
#endif
  if (oracle.guide->proofDisproof() == ProofDisproof::NoEscape())
  {
    confirmNoEscape<P>(record);
    return;
  }

  if (record->sameBoards)
  {
    const CheckHashRecord *loop = 
      (*record->sameBoards).template
      findIneffectiveDropLoop<P>(key.blackStand());
    if (loop)
    {
      if (oracle.guide == record)
      {
	// twin simulation
	assert(! record->twins.empty());
	table.addLoopDetection(path, loop);
      }
      else
      {
	record->setLoopDetection(path, loop);
      }
      CheckmateRecorder::setLeaveReason("loop confirmation (drop)");
      return;
    }
  }
  
  if (oracle.best_move.isInvalid())
  {
    // TODO: この分岐は事前にとれそうな気がする
    CheckmateRecorder::setLeaveReason("empty defense oracle");
    return;
  }
  assert(! oracle.best_move.isPass());
  Move escape_move = oracle.best_move;
  check_assert(escape_move.player() == alt(P));
  assert(escape_move.isValid());
  if (! escape_move.isDrop())
  {
    const Piece existing = state->pieceOnBoard(escape_move.to());
    if (existing.ptype() == KING)
      return;
    escape_move = escape_move.newCapture(existing);
  }
  if (! (*state).template isAlmostValidMove<false>(escape_move))
  {
    CheckmateRecorder::setLeaveReason("invalid defense oracle");
    return;
  }
  
  if (record->moves.empty())
    CheckMoveGenerator<P>::generateEscape(*state, table.listProvider(),
					  record->moves);

  record->bestMove = 0;
  for (CheckMoveList::iterator p=record->moves.begin();
       p!=record->moves.end(); ++p)
  {
    if (p->move == escape_move)
    {
      if (! p->flags.isSet(MoveFlags::Solved))
	record->bestMove = &*p;
      break;
    }
  }
  unsigned int best_move_proof = 1;
  unsigned int best_move_disproof = 1;
  HashKey new_key = key.newHashWithMove(escape_move);
  PathEncoding new_path(path, escape_move);
  
  if (! record->hasBestMove()) // 合法な受手でなかった場合など
    goto failure_end;
  check_assert(record->hasBestMove()
	       && (record->getBestMove()->move == escape_move));
  if (! record->getBestMove()->record)
  {
    CheckTableUtil::allocate(escape_move, record->getBestMove()->record, 
			     table, new_key, new_path, record);
    if (record->proofDisproof().isFinal())
      return;
  }

  if (record->getBestMove()->record->isVisited)
  {
    if (oracle.guide == record)
    {
      // twin simulation
      assert(! record->twins.empty());
      table.addLoopDetection(path, 
			     *record->getBestMove(), record->getBestMove()->record);
    }
    else
    {
      record->setLoopDetection(path, *record->getBestMove(), 
			       record->getBestMove()->record);
    }
    CheckmateRecorder::setLeaveReason("loop confirmation (v)");
    return;
  }
  if (const TwinEntry *loop = record->getBestMove()->findLoop(path, table.getTwinTable()))
  {
    if (oracle.guide == record)
    {
      // twin simulation
      assert(! record->twins.empty());
      table.addLoopDetection(path, *record->getBestMove(), loop->loopTo);
    }
    else
    {
      record->setLoopDetectionTryMerge<P>
	(path, *record->getBestMove(), loop->loopTo);
    }
    CheckmateRecorder::setLeaveReason("loop confirmation (t)");
    return;
  }
  
  {
    const ProofDisproof& pdp = record->getBestMove()->record->proofDisproof();
    if (! pdp.isFinal())
    {
      DisproofOracleAttack<P> new_oracle = oracle.expandOracle();
      if (new_oracle.isValid())
      {
	OracleDisproverAttack<P,OracleDisprover> 
	  oracle_disprover(this, record->getBestMove()->record, new_oracle);
	CheckmateRecorder::setNextMove(&*record->getBestMove());

	std::swap(key, new_key);
	std::swap(path, new_path);
	
	ApplyMove<PlayerTraits<P>::opponent>::doUndoMove
	  (*state, escape_move,oracle_disprover);

	key = new_key;
	path = new_path;

	if (record->proofDisproof().isFinal())
	  return;
      }
      else
      {
#ifdef CHECKMATE_DEBUG
	CheckmateRecorder::DepthTracer::stream() << "oracle expand fail\n";
	if (oracle.guide)
	  oracle.guide->dump(CheckmateRecorder::DepthTracer::stream(),1);
	else
	  CheckmateRecorder::DepthTracer::stream() << "no guide\n";
	if (oracle.next_guide)
	  oracle.next_guide->dump(CheckmateRecorder::DepthTracer::stream(),1);
	else
	  CheckmateRecorder::DepthTracer::stream() << "no next guide\n";
#endif
      }
    }
    if (pdp.isCheckmateFail())
    {
      if (! record->proofDisproof().isCheckmateFail())
      {
	check_assert(! pdp.isLoopDetection());
	record->setDisproofPiecesDefense(alt(P));
	record->propagateNoCheckmate<P>(pdp);
	record->twins.clear();
      }
      return;
    }
    if (const TwinEntry *loop
	= record->getBestMove()->findLoop(path, table.getTwinTable()))
    {
      if (oracle.guide == record)
      {
	// twin simulation
	assert(! record->twins.empty());
	table.addLoopDetection(path, *record->getBestMove(), loop->loopTo);
      }
      else
      {
	record->
	  setLoopDetectionTryMerge<P>(path, *record->getBestMove(), loop->loopTo);
      }
      CheckmateRecorder::setLeaveReason("loop confirmation after move");
      return;
    }
    if (! pdp.isFinal())
    {
      best_move_proof = pdp.proof();
      best_move_disproof = pdp.disproof(); // 本当は全てのminだが，代用
    }
  }
  record->bestMove = 0;
failure_end:
  CheckmateRecorder::setLeaveReason("disproof oracle not found");
#ifdef CHECKMATE_DEBUG
  CheckmateRecorder::DepthTracer::stream() << escape_move << " " << path << " " << oracle.path << "\n";
#endif
  if (record->proofDisproof().isFinal())
    return;
  if (record->moves.empty())
  {
    record->setProofPieces(ProofPieces::leaf(*state, P, record->stand(P)));
    record->propagateCheckmate<P>(ProofDisproof::NoEscape());
    check_assert(record->isConsistent());
  }
  else
  {
    check_assert(best_move_proof < ProofDisproof::PROOF_LIMIT);
    check_assert(best_move_disproof < ProofDisproof::DISPROOF_LIMIT);
    check_assert(! record->proofDisproof().isFinal());
    record->setProofDisproof(std::max((size_t)record->proof(),
				      best_move_proof + record->moves.size()), 
			     std::max(record->disproof(),
				      best_move_disproof/4));
  }

  return;
}

template <class Table>
bool osl::checkmate::OracleDisprover<Table>::
proofNoCheckmate(NumEffectState& state, const PathEncoding& path,
		 const CheckHashRecord *record, const PathEncoding& opath)
{
  const HashKey key(state);
  if (state.turn() == BLACK)
  {
    DisproofOracleAttack<BLACK> oracle(record, opath);
    return proofNoCheckmate(state, key, path, oracle);
  }
  else
  {
    DisproofOracleAttack<WHITE> oracle(record, opath);
    return proofNoCheckmate(state, key, path, oracle);
  }
}

template <class Table>
bool osl::checkmate::OracleDisprover<Table>::
proofEscape(NumEffectState& state, const PathEncoding& path,
	    CheckHashRecord *record, const PathEncoding& opath,
	    Move& best_move, Move last_move)
{
  const HashKey key(state);
  if (state.turn() == BLACK)
  {
    DisproofOracleDefense<WHITE> oracle(record, opath);
    return proofEscape(state, key, path, oracle, best_move, last_move);
  }
  else
  {
    DisproofOracleDefense<BLACK> oracle(record, opath);
    return proofEscape(state, key, path, oracle, best_move, last_move);
  }
}

#endif /* _ORACLEDISPROVER_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
