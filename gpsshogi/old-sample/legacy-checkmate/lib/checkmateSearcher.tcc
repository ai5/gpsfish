#ifndef _CHECKMATE_SEARHCER_TCC
#define _CHECKMATE_SEARHCER_TCC

#include "checkmateSearcher.h"
#include "checkMoveList.h"
#include "checkHashRecord.h"
#include "checkmateRecorder.h"
#include "checkHashTable.h"
#include "checkMoveGenerator.h"
#include "checkTableUtil.h"
#include "osl/checkmate/libertyEstimator.h"
#include "osl/checkmate/pieceCost.h"
#include "oracleProver.h"
#include "oracleDisprover.h"
#include "sameBoardList.h"
#include "blockingSimulation.h"
#include "blockingSimulation.tcc"
#include "defenseSimulation.h"
#include "defenseSimulation.tcc"
#include "osl/checkmate/pawnCheckmateMoves.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/checkmate/fixedDepthSearcher.tcc"
#include "osl/apply_move/applyMoveWithPath.h"
#include "osl/effect_util/effectUtil.h"
#ifdef CHECKMATE_DEBUG
#  include "osl/stat/ratio.h"
#endif
#include <algorithm>

// CHECKMATE_A3 or CHECKMATE_D2
// see kaneko's paper@gpw2005
#define CHECKMATE_D2
// #define CHECKMATE_A3

/** 証明のシミュレーションを有効に */
#define PROOF_SIMULATION
/** 反証のシミュレーションを有効に */
#define DISPROOF_SIMULATION
/** twin ノードを利用した反証のシミュレーションを有効に */
#define DISPROOF_TWIN_SIMULATION

/** root で未展開のままだったら打切る閾値 */
#define ROOT_NOEXPAND_TOL 100u
/** root で打切る証明数の閾値 */
#define ROOT_PROOF_TOL 65536ul*1024
/** root で打切る反証数の閾値 */
#define ROOT_DISPROOF_TOL 65536ul*1024

#define max_twin_simulation 16

namespace osl
{
  namespace checkmate
  {
    template<Player P, class CheckmateSearcher>
    struct ChildDefenseHelper
    {
      typedef CheckmateSearcher searcher_t;
      searcher_t& searcher;
      int proof_number, disproof_number;
      CheckHashRecord *parent;
      CheckHashRecord *record;
      ChildDefenseHelper(searcher_t& s,int pn, int dn,
			 CheckHashRecord *p, CheckHashRecord *r)
	: searcher(s),proof_number(pn),disproof_number(dn),
	  parent(p), record(r)
      {
	check_assert(record && (! record->proofDisproof().isFinal()));
      }
      void operator()(Square)
      {
	searcher.template defense<P>
	  (proof_number, disproof_number, parent, record);
	record->isVisited = false;
      }
    };

    template <Player P, class CheckmateSearcher> 
    struct ChildAttackHelper
    {
      typedef CheckmateSearcher searcher_t;
      searcher_t& searcher;
      int proof_number, disproof_number;
      CheckHashRecord *parent;
      CheckHashRecord *record;
      ChildAttackHelper(searcher_t& s,int pn, int dn,
			CheckHashRecord *p, CheckHashRecord *r)
	: searcher(s),proof_number(pn),disproof_number(dn),
	  parent(p), record(r)
      {
	check_assert(record && (! record->proofDisproof().isFinal()));
      }
      void operator()(Square)
      {
	searcher.template attack<P>(proof_number,disproof_number,parent,record);
	record->isVisited = false;
      }
    };

    inline
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
      check_assert(r < sum);
      return sum;
    }

  } // namespace checkmate
} // namespace osl

template <class Table,class HEstimator,class CostEstimator>
osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
CheckmateSearcher(Player a, Table& t, size_t limit)
  : state(0), table(t), node_count(0), search_node_limit(0), 
    total_node_count(0), total_node_limit(limit), verbose_destructor(false),
    attacker(a)
{
  check_assert(t.getAttacker() == attacker);
}

template <class Table,class HEstimator,class CostEstimator>
osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
~CheckmateSearcher()
{
  clearNodeCount();
#ifndef MINIMAL
  if (verbose_destructor && total_node_count)
    CheckmateRecorder::stat("~CheckmateSearcher", attacker, table.size(), 
			    total_node_count, total_node_limit);
#endif
}

template <class Table,class HEstimator,class CostEstimator>
inline
bool osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
exceedNodeCount(unsigned int future_cost) const
{
  return (node_count + future_cost > search_node_limit) 
    || (node_count > search_node_limit);
}

template <class Table,class HEstimator,class CostEstimator>
template<osl::Player P>
bool osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
setUpAttackNode(CheckHashRecord *record)
{
  check_assert(record->needMoveGeneration()); // 初めて訪れる局面    
  const Square op_king_position
    =(*state).template kingSquare<PlayerTraits<P>::opponent>();
#ifndef NDEBUG
  check_assert(! state->hasEffectAt(P,op_king_position)); // 逃げる手になっていなかった
#endif  
  const bool in_check = state->inCheck();
  King8Info info = state->king8Info(alt(P));
#if ((! defined CHECKMATE_A3) || (! defined CHECKMATE_D2))
  // fixed_searcher を使う場合は不要
  Move check_move;
  if (in_check)
    record->updateBestResultInSolvedAttack(ProofDisproof::PawnCheckmate());
  if ((! in_check)
      && ImmediateCheckmate::hasCheckmateMove<P>(*state, info, 
						 op_king_position, check_move))
  {
    CheckMove best_move(check_move);
    best_move.flags.set(MoveFlags::ImmediateCheckmate);
    record->moves.setOne(best_move, table.listProvider());
    record->bestMove = &*(record->moves.begin());
    PieceStand proof_pieces;	// Note: ImmediateCheckmate が合駒が必要な王手を使わないことに依存
    if (check_move.isDrop())
      proof_pieces.add(check_move.ptype());
    record->setProofPieces(proof_pieces);
    record->propagateCheckmate<P>(ProofDisproof::Checkmate());
    return true;
  }
#endif
  bool has_pawn_checkmate=false;
  CheckMoveGenerator<P>::generateAttack(*state, table.listProvider(),
					record->moves, has_pawn_checkmate);
  for (CheckMoveList::iterator p=record->moves.begin(); p!=record->moves.end(); ++p)
  {
    CostEstimator::setAttackCost(P, *state, *p);
    {
      unsigned int proof, disproof;
      HEstimator::attackH(attacker, *state, info, p->move, 
			  proof, disproof);
      p->h_proof = proof;
      p->h_disproof = disproof;
      check_assert(p->h_proof && p->h_disproof);
    }
  }
  if (has_pawn_checkmate)
    record->updateBestResultInSolvedAttack(ProofDisproof::PawnCheckmate());
  if (record->moves.empty())		// 王手をかける手がない
  {
    record->setDisproofPieces(DisproofPieces::leaf(*state, alt(P), 
						   record->stand(alt(P))));
    record->propagateNoCheckmate<P>((in_check || has_pawn_checkmate)
				    ? ProofDisproof::PawnCheckmate()
				    : ProofDisproof::NoCheckmate());
    return true; // 不詰み
  }
  return false;
}

template <class Table,class HEstimator,class CostEstimator>
template<osl::Player P>
void osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
attack(unsigned int proof_limit, unsigned int disproof_limit, 
       CheckHashRecord *parent, CheckHashRecord *record)
{
#ifndef NDEBUG
  CheckmateRecorder::Tracer tracer("attack ", record, key, path,
				   proof_limit, disproof_limit);
#endif
  node_count++;

  check_assert(proof_limit < ProofDisproof::PROOF_LIMIT);
  check_assert(disproof_limit < ProofDisproof::DISPROOF_LIMIT);
  check_assert(record && parent && record->parent);
  check_assert(! record->isVisited);
  check_assert(! record->findLoop(path, table.getTwinTable()));
  check_assert(! record->proofDisproof().isFinal());
  check_assert((record->proof() < proof_limit));
  check_assert((record->disproof() < disproof_limit));
  check_assert(record->isConsistent());

  record->confirmParent(parent);
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
  if (record->needMoveGeneration()) // 初めて訪れる局面
  {
    {
      Move check_move;
#ifdef CHECKMATE_A3
      if (record->distance >= 0)
      {
	PieceStand proof_pieces;
	const ProofDisproof pdp
	  = fixed_searcher.hasCheckmateMove<P>(2, check_move, proof_pieces);
	if (pdp.isCheckmateSuccess())
	{
	  CheckMove best_move(check_move);
	  best_move.flags.set(MoveFlags::ImmediateCheckmate);
	  record->moves.push_back(best_move);
	  record->bestMove = &*(record->moves.begin());
	  record->setProofPieces(proof_pieces);
	  record->propagateCheckmate<P>(ProofDisproof::Checkmate());
	  return;
	}
	else if (pdp.isCheckmateFail())
	{
	  // 本当はここで return したいが反証駒がない
	}
	else
	{
	  assert(pdp.isUnknown());
	  record->setProofDisproof(std::max(pdp.proof(), record->proof()),
				   std::max(pdp.disproof(), record->disproof()));
	  if ((pdp.proof() >= proof_limit)
	      || (pdp.disproof() >= disproof_limit))
	    return;
	}
      }
#endif
    }
    if (record->needMoveGeneration() && setUpAttackNode<P>(record))
      return;
  }
#ifdef DISPROOF_SIMULATION
#  ifdef DISPROOF_TWIN_SIMULATION
  if (! record->twins.empty())
  {
#    ifdef CHECKMATE_DEBUG
    static stat::Ratio ratio("disproof by twins (attack)");
#    endif
    TwinAgeEntry& age = table.allocateTwin(path);
    check_assert(! age.hasTwinEntry());
    for (TwinList::const_iterator p=record->twins.begin(); 
	 (p!=record->twins.end()) && ((size_t)age.age < std::min((unsigned int)max_twin_simulation, record->twins.size())); 
	 ++p, ++age.age)
    {
#    ifdef CHECKMATE_DEBUG
      ratio.add(record->proofDisproof().isFinal()|| record->findLoop(path, table.getTwinTable()));
#    endif      
      DisproofOracleAttack<P> oracle(record, p->path);
      assert(oracle.isValid());
      typedef OracleDisprover<Table> disprover_t;
      disprover_t disprover(table);
      const bool disproved = disprover.proofNoCheckmate(*state, key,
							path, oracle);
#    ifdef CHECKMATE_DEBUG
      ratio.add(disproved);
#    endif      
      if (disproved)
	return;
      node_count += disprover.nodeCount()/4;
    }
  }
#  endif
#endif
  if (record->proofDisproof().isFinal())
    return;

  check_assert(! record->needMoveGeneration());
  check_assert(! record->isVisited);
  check_assert(proof_limit && disproof_limit);
  record->isVisited = true;
  check_assert(record->isConsistent());
  for (;;)
  {
    check_assert(! record->findLoop(path, table.getTwinTable()));
    unsigned int child_best_proof_number, child_second_best_proof_number;
    unsigned int current_disproof_number, child_best_disproof_number;
    CheckMove *best_move;
    ProofDisproof bestResultInSolved;
#ifdef CHECK_EXTRA_LIMIT_PROOF
    const unsigned int proof_average =
#endif
      record->selectBestAttackMove(path, table.getTwinTable(),
				   child_best_proof_number,
				   child_second_best_proof_number,
				   current_disproof_number, child_best_disproof_number,
				   bestResultInSolved, best_move);
    if (! best_move)
    {
      if (! bestResultInSolved.isLoopDetection())
      {
	record->setDisproofPieces(DisproofPieces::attack(record->moves, *state, 
							 record->stand(alt(P))));
	record->twins.clear();
	record->propagateNoCheckmate<P>(bestResultInSolved);
	return;
      }
      else
      {
	record->setLoopDetectionInAttack<P>(path);
	return;
      }
    }
    check_assert(best_move);
    record->bestMove = best_move;

    HashKey new_key = key.newHashWithMove(best_move->move);
    PathEncoding new_path = PathEncoding(path, best_move->move);
    if (! best_move->record)
    {
      CheckTableUtil::allocate(best_move->move, best_move->record, table,
			       new_key, new_path, record);
      if (best_move->record->isVisited
	  || best_move->findLoop(path, table.getTwinTable())
	  || best_move->record->proofDisproof().isPawnDropFoul(best_move->move)
	  || (best_move->record->proof()
	      && ((best_move->record->proofDisproof()
		   != CheckHashRecord::initialProofDisproof())
		  || (best_move->record->distance <= record->distance))))
	continue; // confluence or something found by using dominance, so perform selection again
    }

    check_assert(best_move->record);
    check_assert(! best_move->record->isVisited);
    check_assert(! best_move->findLoop(path, table.getTwinTable()));
    check_assert(best_move->record->isConsistent());
    if (best_move->record->proof()==0)
    {
      check_assert(! (best_move->record->proofDisproof()
		      .isPawnDropFoul(best_move->move)));
      if (record->bestMove == 0)
	record->bestMove = best_move;
      check_assert(best_move == record->bestMove);
      check_assert(best_move->record->hasProofPieces());
      record->setProofPiecesAttack(P);
      record->propagateCheckmate<P>(ProofDisproof::Checkmate());
      CheckmateRecorder::setLeaveReason("checkmate found");
      check_assert(record->isConsistent());
      return; // 詰み
    }

    if (child_best_proof_number>=proof_limit 
	|| current_disproof_number>=disproof_limit
	|| current_disproof_number==0
	|| exceedNodeCount(child_best_proof_number))
    {
      CheckmateRecorder::setLeaveReason("limit over");
      check_assert(current_disproof_number <= ProofDisproof::DISPROOF_LIMIT);
      check_assert(child_best_proof_number && current_disproof_number);
      record->setProofDisproof(child_best_proof_number, current_disproof_number);
      return;
    }

    const ProofDisproof& pdp = best_move->record->proofDisproof();
    check_assert(! pdp.isFinal());
    check_assert(record->useMaxInsteadOfSum
		 || (best_move->record->distance > record->distance));
#ifdef CHECK_EXTRA_LIMIT_PROOF
    const unsigned int next_proof_limit
      = addWithSaturation(proof_limit, child_second_best_proof_number, proof_average)
      - best_move->cost_proof;
#else
    const unsigned int next_proof_limit
      = ((proof_limit <= child_second_best_proof_number)
	 ? proof_limit
	 : child_second_best_proof_number +1)
      - best_move->cost_proof;
#endif
    const unsigned int next_disproof_limit
      = disproof_limit-current_disproof_number+child_best_disproof_number;
    ChildDefenseHelper<P,CheckmateSearcher>
      helper(*this, next_proof_limit, next_disproof_limit, 
	     record, best_move->record);
    CheckmateRecorder::setNextMove(&*best_move);

    std::swap(key, new_key);
    std::swap(path, new_path);

    ApplyMove<P>::doUndoMove(*state, best_move->move, helper);

    key = new_key;
    path = new_path;

    check_assert(best_move->record->isConfluence
		 || (best_move->record->parent == record));
    if (record->proofDisproof().isFinal())
      return;
    if (pdp.proof() == 0)
    {
      if (! pdp.isPawnDropFoul(best_move->move))
      {
	if (record->bestMove == 0) // TODO: 2057:72
	  record->bestMove = best_move;
	check_assert(best_move == record->bestMove);
	check_assert(best_move->record->hasProofPieces());
	record->setProofPiecesAttack(P);
	record->propagateCheckmate<P>(ProofDisproof::Checkmate()); // Checkmate instead of NoEscape
	CheckmateRecorder::setLeaveReason("checkmate found after move");
	return; // 詰み
      }
      // 打歩詰め
      record->addToSolvedInAttack(*best_move, ProofDisproof::PawnCheckmate());
      record->filter.addTarget(MoveFlags::NoPromote);
    }
    else if (pdp.disproof() == 0) // 不詰
    {
      check_assert(! pdp.isLoopDetection());
      record->addToSolvedInAttack(*best_move, pdp);
#ifdef DISPROOF_SIMULATION
      assert(best_move->move.isValid());
      if (best_move->move.isDrop())
      {
	const Ptype ptype = best_move->move.ptype();
	if ((ptype == ROOK) || (ptype == BISHOP))
	{
	  DefenseSimulation<P>::disproofDropSibling
	    (*state, key, path, record, table, *best_move, node_count);
	}
      }
#endif
    }
#ifdef PAWN_CHECKMATE_SENSITIVE
    const TwinEntry *loopDetected = best_move->findLoop(path, table.getTwinTable());
    if (((pdp == ProofDisproof::PawnCheckmate()) 
	 || (loopDetected && loopDetected->move.record
	     && (loopDetected->move.record->bestResultInSolved
		 == ProofDisproof::PawnCheckmate())))
	&& PawnCheckmateMoves::hasParingNoPromote(best_move->move))
    {
      const Move try_nopromote = best_move->move.unpromote();
      for (CheckMoveList::iterator p=record->moves.begin(); 
	   p!=record->moves.end(); ++p)
      {
	if (p->move == try_nopromote)
	{
	  if (! p->record)
	  {
	    p->flags.unset(MoveFlags::NoPromote); // 一つだけ候補に加える
	    const HashKey new_key = key.newHashWithMove(try_nopromote);
	    const PathEncoding new_path(path, try_nopromote);
	    CheckTableUtil::allocate
	      (p->move, p->record, table, new_key, new_path, record);
#ifdef DISPROOF_SIMULATION
	    DefenseSimulation<P>::disproofNoPromote
	      (*state, new_key, new_path, record, table, *p, *best_move, node_count);
#endif	    
	  }
	  break;
	}
      }
    }
#endif
  }
}

template <class Table,class HEstimator,class CostEstimator>
template<osl::Player P>
bool osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
setUpDefenseNode(CheckHashRecord *record)
{
  check_assert(record->needMoveGeneration()); // 初めて訪れる局面
#ifdef CHECKMATE_DEBUG
  const Square op_king_position=(*state).template kingSquare<P>();
  // 王手をかけたら自玉が取られる状態
  check_assert(op_king_position.isPieceStand()
	       || ! state->hasEffectAt(alt(P),op_king_position));
#endif
  const unsigned int simple_king_moves =
    CheckMoveGenerator<P>::generateEscape(*state, table.listProvider(),
					  record->moves);
  if (record->moves.empty()) { // 逃げる手がない
    assert(! record->proofDisproof().isFinal());
    record->setProofPieces(ProofPieces::leaf(*state, P, record->stand(P)));
    record->propagateCheckmate<P>(ProofDisproof::NoEscape());
    check_assert(record->isConsistent());
    return true; // 一手ばったり
  }
  if (simple_king_moves == 2)
    record->false_branch_candidate = true;
  
  for (CheckMoveList::iterator p=record->moves.begin();
       p!=record->moves.end(); ++p)
  {
    check_assert(p->move.isNormal());
#ifdef CHECKMATE_D2
    PieceStand proof_pieces;
    Move check_move;
    const ProofDisproof pdp
      = fixed_searcher.hasEscapeByMove<P>(p->move, 0, check_move, proof_pieces);

    if (pdp.isCheckmateSuccess())
    {
      CheckTableUtil::registerImmediateCheckmateInDefense<P,Table>
	(key, path, record, *p, pdp, check_move, proof_pieces, table);
    }
    else if (pdp.isCheckmateFail())
    {
      CheckMove best_move = *p; // copy
      record->moves.setOne(best_move, table.listProvider());
      break;
    }
    else
    {
      p->h_proof = pdp.proof();
      p->h_disproof = pdp.disproof();
    }
#endif
#if (! defined CHECKMATE_D2)
    unsigned int proof, disproof;
    HEstimator::defenseH(attacker, *state, p->move, 
			 proof, disproof);
    p->h_proof = proof;
    p->h_disproof = disproof;
#endif
    check_assert(p->h_proof && p->h_disproof);
  }

  for (CheckMoveList::iterator p=record->moves.begin();
       p!=record->moves.end(); ++p)
  {
    CostEstimator::setDefenseCost(P, *state, *p);
  }
  return false;
}
  
template <class Table,class HEstimator,class CostEstimator>
template<osl::Player P>
void osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
defense(unsigned int proof_limit, unsigned int disproof_limit, 
	CheckHashRecord *parent, CheckHashRecord *record)
{
#ifndef NDEBUG
  CheckmateRecorder::Tracer tracer("defense", record, key, path,
				   proof_limit, disproof_limit);
#endif
  node_count++;

  check_assert(proof_limit < ProofDisproof::PROOF_LIMIT);
  check_assert(disproof_limit < ProofDisproof::DISPROOF_LIMIT);
  check_assert(record && parent && record->parent);
  check_assert(! record->findLoop(path, table.getTwinTable()));
  check_assert(! record->isVisited);
  check_assert(! record->proofDisproof().isFinal());
  check_assert(! (record->proof()>proof_limit));
  check_assert(! (record->disproof()>disproof_limit));
  check_assert(record->isConsistent());

  record->confirmParent(parent);
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
    if (setUpDefenseNode<P>(record))
      return;
  }
#ifdef DISPROOF_SIMULATION
#  ifdef DISPROOF_TWIN_SIMULATION
  if (! record->twins.empty())
  {
#    ifdef CHECKMATE_DEBUG
    static stat::Ratio ratio("disproof by twins (defense)");
#    endif
    TwinAgeEntry& age = table.allocateTwin(path);
    check_assert(! age.hasTwinEntry());
    for (TwinList::const_iterator p=record->twins.begin(); 
	 (p!=record->twins.end()) && ((size_t)age.age < std::min((unsigned int)max_twin_simulation, record->twins.size()));
	 ++p, ++age.age)
    {
      DisproofOracleDefense<P> oracle(record, p->path);
      assert(oracle.isValid());
      typedef OracleDisprover<Table> disprover_t;
      disprover_t prover(table);
      Move escape_move;
      const bool disproved
	= prover.proofEscape(*state, key, path, oracle, escape_move, 
			     record->parent->bestMove->move);
#    ifdef CHECKMATE_DEBUG
      ratio.add(disproved);
#    endif      
      if (disproved)
	return;
      node_count += prover.nodeCount()/4;
    }
  }
#  endif
#endif
  if (record->proofDisproof().isFinal())
    return;

  check_assert(! record->needMoveGeneration());
  check_assert(proof_limit && disproof_limit);
  record->isVisited = true;
  check_assert(record->isConsistent());
  for (;;)
  {
    if (record->findLoop(path, table.getTwinTable()))
      return;
    unsigned int child_best_disproof_number, child_second_best_disproof_number;
    unsigned int current_proof_number, child_best_proof_number;
    ProofDisproof bestResultInSolved;
    CheckMove *best_move;
#ifdef CHECK_EXTRA_LIMIT_DISPROOF
    const unsigned int disproof_average =
#endif
      record->selectBestDefenseMove(path, table.getTwinTable(),
				    child_best_disproof_number,
				    child_second_best_disproof_number,
				    current_proof_number, child_best_proof_number,
				    bestResultInSolved, best_move);
    if (! best_move)
    {
      if (! record->filter.isTarget(MoveFlags::BlockingBySacrifice))
      {
	record->filter.addTarget(MoveFlags::BlockingBySacrifice);
	for (CheckMoveList::iterator p=record->moves.begin(); 
	     p!=record->moves.end(); ++p)
	{
	  if (p->flags.isSet(MoveFlags::BlockingBySacrifice))
	  {
	    const HashKey new_key = key.newHashWithMove(p->move);
	    const PathEncoding new_path(path, p->move);
	    if (! p->record)
	    {
	      CheckTableUtil::allocate
		(p->move, p->record, table, new_key, new_path, record);
	    }
#ifdef PROOF_SIMULATION
	    if (BlockingSimulation<P>::proof(*state, new_key, new_path, 
					     record, table, *p, node_count))
	      p->flags = MoveFlags::Solved;
	    else
#endif
	      best_move = &*p;
	  }
	}
      }
      if ((! best_move) && (! record->filter.isTarget(MoveFlags::Upward)))
      {
	record->filter.addTarget(MoveFlags::Upward);
	record->useMaxInsteadOfSum = true;
	continue;
      }
      if (! best_move)
      {
	CheckmateRecorder::setLeaveReason("no unsolved moves");
	check_assert(record->bestResultInSolved.proof() == 0);
	if (! record->proofDisproof().isCheckmateSuccess())
	{
	  assert(! record->proofDisproof().isFinal());
	  record->setProofPieces(ProofPieces::defense(record->moves, *state,
						      record->stand(P)));
	  record->propagateCheckmate<P>(record->bestResultInSolved);
	}
	return;
      }
      continue;
    }
    check_assert(best_move);
    record->bestMove = &*best_move;

    HashKey new_key = key.newHashWithMove(best_move->move);
    PathEncoding new_path(path, best_move->move);
    if (! best_move->record)
    {
      CheckTableUtil::allocate(best_move->move, best_move->record, table,
			       new_key, new_path, record);
      if ((! best_move->record->isVisited)
	  && (! best_move->findLoop(path, table.getTwinTable()))
	  && best_move->record->disproof()
	  && ((best_move->record->proofDisproof() 
	       != CheckHashRecord::initialProofDisproof())
	      || (best_move->record->distance <= record->distance)))
	continue; // confluence or something found by using dominance, so perform selection again
    }

    check_assert(best_move->record);
    if (best_move->record->disproof()==0)
    {
      record->setDisproofPiecesDefense(alt(P));
      record->twins.clear();
      record->propagateNoCheckmate<P>(best_move->record->proofDisproof());
      CheckmateRecorder::setLeaveReason("no checkmate found");
      return;
    }
    if (best_move->record->isVisited)
    {
      record->setLoopDetection(path, *best_move, best_move->record);
      CheckmateRecorder::setLeaveReason("loop to visited");
      return;
    }
    if (const TwinEntry *loop = best_move->findLoop(path, table.getTwinTable()))
    {
      record->setLoopDetectionTryMerge<P>
	(path, *best_move, loop->loopTo);
      CheckmateRecorder::setLeaveReason("loop found");
      return;
    }

    if (child_best_disproof_number>=disproof_limit 
	|| current_proof_number>=proof_limit
	|| exceedNodeCount(current_proof_number))
    {
      check_assert(current_proof_number);
      check_assert(current_proof_number<=ProofDisproof::PROOF_LIMIT);
      check_assert(child_best_disproof_number);
      record->setProofDisproof(current_proof_number, child_best_disproof_number);
      CheckmateRecorder::setLeaveReason("limit over");
      return;
    }
    check_assert(best_move->record && (! best_move->record->isVisited)
		 && (! best_move->findLoop(path, table.getTwinTable())));
    check_assert(! best_move->record->proofDisproof().isFinal());
    const unsigned int next_proof_limit
      = proof_limit-current_proof_number+child_best_proof_number;
#ifdef CHECK_EXTRA_LIMIT_DISPROOF
    const unsigned int next_disproof_limit
      = addWithSaturation(disproof_limit, 
			  child_second_best_disproof_number, disproof_average)
      - best_move->cost_disproof;
#else
    const unsigned int next_disproof_limit
      = ((disproof_limit <= child_second_best_disproof_number)
	 ? disproof_limit 
	 : child_second_best_disproof_number +1)
      - best_move->cost_disproof;
#endif
    ChildAttackHelper<P,CheckmateSearcher> 
      helper(*this, next_proof_limit, next_disproof_limit, 
	     record, best_move->record);
    CheckmateRecorder::setNextMove(&*best_move);

    std::swap(key, new_key);
    std::swap(path, new_path);
    ApplyMove<PlayerTraits<P>::opponent>
      ::doUndoMove(*state,best_move->move,helper);
    key = new_key;
    path = new_path;

    check_assert(best_move->record->isConfluence
		 || (best_move->record->parent == record)
		 || record->finalByDominance());
    if (record->proofDisproof().isFinal())
      return;
    if (best_move->record->disproof()==0)
    {
      record->setDisproofPiecesDefense(alt(P));
      record->twins.clear();
      record->propagateNoCheckmate<P>(best_move->record->proofDisproof());
      CheckmateRecorder::setLeaveReason("no checkmate after move");
      return; // 不詰
    }
#ifdef PROOF_SIMULATION
    if ((best_move->record->proof()==0)
	&& best_move->move.isDrop())
    {
      BlockingSimulation<P>::proofSibling(*state, key, path, record, table, 
					  *best_move, node_count);
    }
#endif
  }
}

template <class Table,class HEstimator,class CostEstimator>
inline
bool osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
exceedRootTolerance(unsigned int proof_number, unsigned int disproof_number,
		    unsigned int continuousNoExpandLoop)
{
  return (proof_number >= ROOT_PROOF_TOL)
    || (disproof_number >= ROOT_DISPROOF_TOL)
    || (continuousNoExpandLoop >= ROOT_NOEXPAND_TOL);
}

template <class Table,class HEstimator,class CostEstimator>
template<osl::Player P>
const osl::checkmate::ProofDisproof osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
hasCheckmateMove(state_t& s, const HashKey& root_key,
		 const PathEncoding& root_path, 
		 size_t search_node_limit, Move& best_move)
{
  assert(P == attacker);
  check_assert(s.turn()==P);
  assert(node_count == 0);
  state = &s;
  path = root_path;
  key = root_key;
  CheckmateRecorder::setState(state);
  fixed_searcher.setState(*state);

  CheckHashRecord *record = table.find(key);
  if (! record)
  {
    const PieceStand white_stand(WHITE, *state);

    CheckHashRecord *root = table.root();
    root->distance = path.getDepth()-1;
    CheckTableUtil::allocate(record, table, key, white_stand, path, root);
  }
  if (record->proofDisproof().isFinal())
  {
    if (record->hasBestMove())
      best_move = record->bestMove->move;
    check_assert((record->proof() != 0) || 
		 (record->hasBestMove() && best_move.isValid()));
    return record->proofDisproof();
  }
  assert(record->parent);
  if (record->findLoop(path, table.getTwinTable()))
    return ProofDisproof::LoopDetection();
  if (total_node_count>total_node_limit)
    return ProofDisproof::Unknown();
  search_node_limit=std::min(search_node_limit,total_node_limit-total_node_count);
  this->search_node_limit = search_node_limit;
  unsigned int proof_number = record->proof();
  unsigned int disproof_number = record->disproof();
  if ((ROOT_PROOF_TOL <= proof_number)
      || (ROOT_DISPROOF_TOL <= disproof_number))
    return record->proofDisproof();
  size_t continuousNoExpandLoop = 0;
  CheckmateRecorder::rootLog("hasCheckmateMove start", table.size(),
			     continuousNoExpandLoop);
  // bug 96 でpathencodingを使った対策にすれば，assertに戻せる
  if (record->isVisited)
    return ProofDisproof::LoopDetection();
  for (;;)
  {
    assert(path == root_path);
    const size_t nodeSizeAtRoot = table.size();
    CheckmateRecorder::setNextMove(0);
    attack<P>(ROOT_PROOF_TOL, ROOT_DISPROOF_TOL, record->parent, record);
    record->isVisited = false;
    if (record->findLoop(path, table.getTwinTable()))
    {
      clearNodeCount();
      return ProofDisproof::LoopDetection();
    }
    proof_number=record->proof();
    disproof_number=record->disproof();
    check_assert(!(proof_number==0 && disproof_number==0));
    if (proof_number == 0 || disproof_number==0 
	|| exceedNodeCount(proof_number)
	|| exceedRootTolerance(proof_number, disproof_number, continuousNoExpandLoop))
    {
      if (record->hasBestMove())
      {
	best_move = record->bestMove->move;
	check_assert((disproof_number == 0) || best_move.isValid());
      }
      check_assert((proof_number != 0) || (record->hasBestMove() && best_move.isValid()));
      clearNodeCount();
      return record->proofDisproof();
    }
    if (nodeSizeAtRoot == table.size())
      ++continuousNoExpandLoop;
    else
      continuousNoExpandLoop = 0;
    CheckmateRecorder::rootLog("hasCheckmateMove", table.size(), continuousNoExpandLoop);
  }
}

template <class Table,class HEstimator,class CostEstimator>
template<osl::Player P>
const osl::checkmate::ProofDisproof osl::checkmate::CheckmateSearcher<Table,HEstimator,CostEstimator>::
hasEscapeMove(state_t& s, const HashKey& root_key, const PathEncoding& root_path,
	      size_t search_node_limit, Move last_move)
{
  assert(P == attacker);
  assert(s.turn()==alt(P));
  assert(node_count == 0);
  state = &s;
  path = root_path;
  key = root_key;
  CheckmateRecorder::setState(state);
  fixed_searcher.setState(*state);
  check_assert(state->hasEffectAt(P,(*state).template kingSquare<PlayerTraits<P>::opponent>()));

  CheckHashRecord *record = table.find(key);
  if (! record)
  {
    const PieceStand white_stand(WHITE, *state);
    CheckHashRecord *root = table.root();
    root->distance = path.getDepth()-1;
    root->moves.setOne(CheckMove(last_move), table.listProvider());
    root->bestMove = &*root->moves.begin();
    CheckTableUtil::allocate(record, table, key, white_stand, path, root);
  }
  if (record->proofDisproof().isFinal())
  {
    if (record->proofDisproof().isPawnDropFoul(last_move))
      return ProofDisproof::PawnCheckmate();
    return record->proofDisproof();
  }
  if (record->findLoop(path, table.getTwinTable()))
    return ProofDisproof::LoopDetection();
  if (total_node_count>total_node_limit)
    return ProofDisproof::Unknown();
  search_node_limit=std::min(search_node_limit,total_node_limit-total_node_count);
  this->search_node_limit = search_node_limit;

  unsigned int proof_number = record->proof();
  unsigned int disproof_number = record->disproof();
  if ((ROOT_PROOF_TOL <= proof_number)
      || (ROOT_DISPROOF_TOL <= disproof_number))
    return record->proofDisproof();
  size_t continuousNoExpandLoop = 0;
  CheckmateRecorder::rootLog("hasEscapeMove start", table.size(),
			     continuousNoExpandLoop);
  // bug 96 でpathencodingを使った対策にすれば，assertに戻せる
  if (record->isVisited)
    return ProofDisproof::LoopDetection();
  for (;;)
  {
    const size_t nodeSizeAtRoot = table.size();
    check_assert(path == root_path);
    CheckmateRecorder::setNextMove(0);
    defense<P>(ROOT_PROOF_TOL, ROOT_DISPROOF_TOL, record->parent, record);
    record->isVisited = false;
    proof_number=record->proof();
    disproof_number=record->disproof();
    check_assert(!(proof_number==0 && disproof_number==0));
    if (record->findLoop(path, table.getTwinTable()))
    {
      clearNodeCount();
      return ProofDisproof::LoopDetection();
    }
    if (record->proofDisproof().isFinal() 
	|| exceedNodeCount(proof_number)
	|| exceedRootTolerance(proof_number, disproof_number, continuousNoExpandLoop))
    {
      clearNodeCount();
      if (record->proofDisproof().isPawnDropFoul(last_move))
	return ProofDisproof::PawnCheckmate();
      return record->proofDisproof();
    }
    if (nodeSizeAtRoot == table.size())
      ++continuousNoExpandLoop;
    else
      continuousNoExpandLoop = 0;
    CheckmateRecorder::rootLog("hasEscapeMove", table.size(), continuousNoExpandLoop);
  }
}

#endif /* _CHECKMATE_SEARHCER_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
