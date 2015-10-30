/* checkHashRecord.cc
 */
#include "checkHashRecord.h"
#include "sameBoardList.h"
// TODO: PAWN_CHECKMATE_SENSITIVE, DELAY_SACRIFICE のためだけなので分離
#include "checkMoveGenerator.h"
#ifdef CHECKMATE_DEBUG
#  include "osl/stat/ratio.h"
#endif

// #define TWINLIST_STAT

#ifdef TWINLIST_STAT
#  include "osl/stat/histogram.h"
#endif
#include <iostream>

// /** 定義すると局面の優越関係を記憶し，詰/不詰だけでなく証明数を伝播させる */
// #define PROOF_NUMBER_PROPAGATE_BY_DOMINANCE

namespace osl
{
  namespace checkmate
  {
    // explicit instantiation
    template void CheckHashRecord::propagateCheckmateRecursive<BLACK>();
    template void CheckHashRecord::propagateCheckmateRecursive<WHITE>();
    template void CheckHashRecord::propagateNoCheckmateRecursive<BLACK>();
    template void CheckHashRecord::propagateNoCheckmateRecursive<WHITE>();

    // これを越えるようだとallocator の調整が必要
    BOOST_STATIC_ASSERT(sizeof(CheckHashRecord) + sizeof(int)*6 
			+ sizeof(void*) <= 192);
  }
}

bool osl::checkmate::CheckHashRecord::
isConsistent() const
{
  assert(! proofDisproof().isLoopDetection());
  static int last_type = UNSET;
  if (proof_pieces_type != UNSET)
  {
    if (! proof_disproof.isFinal())
    {
      std::cerr << "record->proof_pieces_type " << last_type
		<< " => " << (int)proof_pieces_type << "\n";
    }
    check_assert(proof_disproof.isFinal());
  }
  last_type = proof_pieces_type;
  return true;
}

osl::checkmate::CheckHashRecord::
~CheckHashRecord()
{
#if 0
  // final_by_dominance が dangling pointer になっている可能性があるため
  // さわってはいけない
  if (final_by_dominance)
    assert(final_by_dominance->final_by_dominance == 0);
#endif
  assert(isConsistent());

#ifdef TWINLIST_STAT
  static stat::Histogram t_hist(1,300,0,true);
  t_hist.add(twins.size());
#endif
}

unsigned int osl::checkmate::CheckHashRecord::
selectBestAttackMove(const PathEncoding& path,
		     const TwinTable& table,
		     unsigned int& min_proof,
		     unsigned int& min_proof2,
		     unsigned int& sum_disproof,
		     unsigned int& disproof_of_bestchild,
		     ProofDisproof& bestResultInSolved,
		     CheckMove *& best_child)
{
#ifdef DELAY_SACRIFICE
  if ((! filter.isTarget(MoveFlags::Sacrifice))
      && (getProof(proofDisproof()) > 8))	// ここの数字次第で208辺りがこける
    filter.addTarget(MoveFlags::Sacrifice);
#endif
  while (true)
  {
    const unsigned int result = 
      selectBestAttackMoveMain(path, table, min_proof,
			       min_proof2,
			       sum_disproof,
			       disproof_of_bestchild, 
			       bestResultInSolved, best_child);
    if (! best_child)
    {
#ifdef DELAY_SACRIFICE
      if (! filter.isTarget(MoveFlags::Sacrifice))
      {
	filter.addTarget(MoveFlags::Sacrifice);
	continue;
      }
#endif
      if ((this->bestResultInSolved == ProofDisproof::PawnCheckmate())
	  && (! filter.isTarget(MoveFlags::NoPromote)))
      {
	filter.addTarget(MoveFlags::NoPromote);
	continue;
      }
#ifdef CHECK_DELAY_UPWARDS
      if (! filter.isTarget(MoveFlags::Upward))
      {
	filter.addTarget(MoveFlags::Upward);
	useMaxInsteadOfSum = true;
	continue;
      }
#endif
    }
    else
    {
      if (best_child->record)
	check_assert(! best_child->findLoop(path, table));
    }
    return result;
  }
}

unsigned int osl::checkmate::CheckHashRecord::
selectBestAttackMoveMain(const PathEncoding& path,
			 const TwinTable& table,
			 unsigned int& min_proof,
			 unsigned int& min_proof2,
			 unsigned int& sum_disproof,
			 unsigned int& disproof_of_bestchild,
			 ProofDisproof& bestResultInSolved,
			 CheckMove*& best_child)
{
  min_proof=ProofDisproof::BigProofNumber;
  min_proof2=ProofDisproof::BigProofNumber;
  sum_disproof=0;
  disproof_of_bestchild=ProofDisproof::BigProofNumber;
  bestResultInSolved = this->bestResultInSolved;
  best_child = 0;
  unsigned int sum_frontier_proofs = 0;
  unsigned int num_frontiers = 0;

  // TODO: targetUpwards でtemplate にして分けた方が良いか
  const bool targetUpwards = filter.isTarget(MoveFlags::Upward);
  for (CheckMoveList::iterator p=moves.begin(); p!=moves.end(); ++p)
  {
#ifdef CHECK_DELAY_UPWARDS
    if ((! targetUpwards) && (p->flags.isSet(MoveFlags::Upward)))
    {
      check_assert(p->record);
      const ProofDisproof& pdp = p->record->proofDisproof();
      if (pdp.proof()==0)
      {
	if (! pdp.isPawnDropFoul(p->move))
	{
	  best_child = &*p;
	  return 0;
	}
	else
	{
	  addToSolvedInAttack(*p, ProofDisproof::PawnCheckmate());
	}
      }
      continue;
    }
#endif
    if (! filter.isTarget(p->flags))
      continue;

    unsigned int child_proof, child_disproof;
    if (p->record)
    {
#ifdef PROOF_NUMBER_PROPAGATE_BY_DOMINANCE
      if (p->record->sameBoards && (! p->record->proofDisproof().isFinal())
	  && (! p->record->isVisited) && (! p->findLoop(path, table)))
	p->record->sameBoards->updateSlow<false>(p->move.player(), *p->record, 
						 PathEncoding(path, p->move));
#endif      
      const TwinEntry *loopDetected = p->findLoop(path, table);
      if (loopDetected)
      {
#ifdef PAWN_CHECKMATE_SENSITIVE
	if ((this->bestResultInSolved != ProofDisproof::PawnCheckmate())
	    && loopDetected->move.record
	    && (loopDetected->move.record->bestResultInSolved
		== ProofDisproof::PawnCheckmate()))
	{
	  this->bestResultInSolved 
	    = this->bestResultInSolved.betterForAttack(ProofDisproof::PawnCheckmate());
	}
#endif
#ifdef CHECK_ABSORB_LOOP
	if (loopDetected->loopTo == this)
	{
	  p->flags.set(MoveFlags::Solved);
	  this->bestResultInSolved = betterForAttack(this->bestResultInSolved, NoCheckmate);
	  continue;
	}
#endif
	bestResultInSolved 
	  = bestResultInSolved.betterForAttack(ProofDisproof::LoopDetection());
	continue;
      }
      if (p->record->isVisited)
      {
	if (p->record->proofDisproof() == ProofDisproof::PawnCheckmate())
	  this->bestResultInSolved 
	    = this->bestResultInSolved.betterForAttack(ProofDisproof::PawnCheckmate());
	bestResultInSolved = bestResultInSolved.betterForAttack(ProofDisproof::LoopDetection());
	continue;
      }
      if (targetUpwards && (distance > p->record->distance))
	distance = p->record->distance;

      const ProofDisproof& child_pdp = p->record->proofDisproof();
      child_proof=child_pdp.proof();
      child_disproof=child_pdp.disproof();
      if (child_proof == 0)
      {
	if (! child_pdp.isPawnDropFoul(p->move))
	{
	  best_child = &*p;
	  return 0;
	}
	else
	{
	  addToSolvedInAttack(*p, ProofDisproof::PawnCheckmate());
	  continue;
	}
      }
      else if (child_disproof == 0)
      {
	check_assert(! child_pdp.isLoopDetection());
	addToSolvedInAttack(*p, child_pdp);
	continue;
      }
      else {
#ifdef CHECK_DELAY_UPWARDS
	if ((!targetUpwards) && (p->record->distance <= distance)) {
	  p->flags.set(MoveFlags::Upward);
	  continue;
	}
	child_proof += p->cost_proof;
	child_disproof += p->cost_disproof;
      }
#endif
    }
    else
    {
      child_proof = p->h_proof + p->cost_proof;
      child_disproof = p->h_disproof + p->cost_proof;
      sum_frontier_proofs += p->h_proof;
      ++num_frontiers;
    }
    
    check_assert(child_proof != 0);
    check_assert(child_disproof != 0);

    p->addCost(child_proof,child_disproof);

    if (child_proof <= min_proof2)
    {
      if ((child_proof < min_proof)
	  || ((child_proof == min_proof)
	      && (child_disproof < disproof_of_bestchild)))
      {
	best_child=&*p;
	min_proof2=min_proof;
	min_proof=child_proof;
	disproof_of_bestchild=child_disproof;
      }
      else
      {
	min_proof2=child_proof;
      }
      sum_disproof 
	= add(child_disproof, sum_disproof);
      continue;
    }
    sum_disproof 
      = add(child_disproof, sum_disproof);
  }

  bestResultInSolved
    = bestResultInSolved.betterForAttack(this->bestResultInSolved);
#ifdef CHECK_EXTRA_LIMIT_PROOF
  // (分母が)いい加減な平均
  const unsigned int proof_average
    = num_frontiers ? (sum_frontier_proofs / num_frontiers) : 1;
  assert(proof_average >= 1);
  return proof_average;
#else
  return 0;
#endif
}


unsigned int osl::checkmate::CheckHashRecord::
selectBestDefenseMove(const PathEncoding& path,
		      const TwinTable& table,
		      unsigned int& min_disproof,
		      unsigned int& min_disproof2,
		      unsigned int& sum_proof,
		      unsigned int& proof_of_bestchild,
		      ProofDisproof& bestResultInSolved,
		      CheckMove *&best_child)
{
  // TODO: targetUpwards でtemplate にして分けた方が良いか
  const bool targetUpwards = filter.isTarget(MoveFlags::Upward);
  min_disproof=ProofDisproof::BigProofNumber;
  min_disproof2=ProofDisproof::BigProofNumber;
  sum_proof=0;
  proof_of_bestchild=ProofDisproof::BigProofNumber;
  best_child = 0;
  bestResultInSolved = this->bestResultInSolved;
  unsigned int sum_frontier_disproofs = 0;
  unsigned int num_frontiers=0;
  // 柿木2005
  unsigned int simple_king_move_proofmax = 0;
  const CheckHashRecord *converge_candidate = 0;

  for (CheckMoveList::iterator p=moves.begin(); p!=moves.end(); ++p)
  {
#ifdef CHECK_DELAY_UPWARDS
    if ((! targetUpwards) && (p->flags.isSet(MoveFlags::Upward)))
    {
      check_assert(p->record);
      const ProofDisproof& pdp = p->record->proofDisproof();
      if ((pdp.disproof()==0) || p->record->isVisited // 不詰
	  || p->findLoop(path, table))
      {
	best_child = &*p;
	return 0;
      }
      continue;
    }
#endif
    if (! filter.isTarget(p->flags))
      continue;

    unsigned int child_proof, child_disproof;
    if (p->record)
    {
#ifdef PROOF_NUMBER_PROPAGATE_BY_DOMINANCE
      if (p->record->sameBoards && (! p->record->proofDisproof().isFinal())
	  && (! p->record->isVisited) && (! p->findLoop(path, table)))
      {
	p->record->sameBoards->updateSlow<true>(alt(p->move.player()), *p->record, 
						PathEncoding(path, p->move));
      }
#endif      
      if (targetUpwards && (distance > p->record->distance))
	distance = p->record->distance;

      const ProofDisproof& child_pdp = p->record->proofDisproof();
      child_proof=child_pdp.proof();
      child_disproof=child_pdp.disproof();

      if ((child_disproof==0) || p->record->isVisited // 不詰
	  || p->findLoop(path, table))
      {
	best_child = &*p;
	return 0;
      }
      else if (child_proof == 0) // 詰
      {
	addToSolvedInDefense(*p, child_pdp);
	continue;
      }
      else {
#ifdef CHECK_DELAY_UPWARDS
	if ((! targetUpwards) && (p->record->distance <= distance)) {
	  p->flags.set(MoveFlags::Upward);
	  continue;
	}
#endif
	child_proof += p->cost_proof;
	child_disproof += p->cost_disproof;
      }
    }
    else
    {
      child_proof = p->h_proof + p->cost_proof;
      child_disproof = p->h_disproof + p->cost_disproof;
      sum_frontier_disproofs += p->h_disproof;
      ++num_frontiers;
    }
    assert(child_proof);
    assert(child_disproof);
    
    p->addCost(child_proof, child_disproof);

    if ((child_disproof < min_disproof)
	|| ((child_disproof == min_disproof)
	    && (child_proof < proof_of_bestchild)))
    {
      best_child = &*p;

      min_disproof2=min_disproof;
      min_disproof=child_disproof;
      proof_of_bestchild=child_proof;
    }
    else if (child_disproof < min_disproof2)
    {
      min_disproof2=child_disproof;
    }
    if (false_branch_candidate && p->record && p->move.ptype() == KING 
	&& p->move.capturePtype() == PTYPE_EMPTY) {
      if (! false_branch
	  && p->record->hasBestMove() && p->record->getBestMove()->record) {
	if (converge_candidate) {
	  if (converge_candidate == p->record->getBestMove()->record)
	    false_branch = true;
	}
	else {
	  converge_candidate = p->record->getBestMove()->record;
	}
      }
      if (false_branch) {
	if (simple_king_move_proofmax < child_proof)
	  simple_king_move_proofmax = child_proof;
      }
      else {
	simple_king_move_proofmax = add(simple_king_move_proofmax, child_proof);
      }
    }
    else {
      sum_proof = add(sum_proof, child_proof);
    }
  }
  sum_proof += simple_king_move_proofmax;
  bestResultInSolved 
    = bestResultInSolved.betterForDefense(this->bestResultInSolved);
#ifdef CHECK_EXTRA_LIMIT_DISPROOF
  const unsigned int disproof_average
    = num_frontiers ? (sum_frontier_disproofs / num_frontiers) : 1;
  assert(disproof_average >= 1);
  return disproof_average;
#else
  return 0;
#endif
}

template <osl::Player Attacker>
void osl::checkmate::CheckHashRecord::propagateCheckmateRecursive()
{
  assert(proof_pieces_type == PROOF);
#ifdef CHECKMATE_DEBUG
  static stat::Ratio ratio("propagateCheckmate");
  if ((! moves.empty()) && (moves.begin()->move.player() == Attacker))
  {
    // or-node
    check_assert(hasBestMove());
  }
#endif
  check_assert(final_by_dominance == 0);
  for (SameBoardList::iterator p=sameBoards->begin(); p!=sameBoards->end(); ++p)
  {
    if (p->stand(BLACK) == black_stand)
      continue;
    const PieceStand& attack_stand 
      = (Attacker == BLACK) ? p->black_stand : p->white_stand;
    if (attack_stand.template hasMoreThan<BLACK>(proofPieces()))
    {
#ifdef CHECKMATE_DEBUG
      ratio.add(p->proof());
#endif    
      check_assert(p->disproof());
      if (p->proof())
      {
	check_assert(p->final_by_dominance == 0);
	p->setProofPieces(proofPieces());
	p->setFinalByDominance(this);
	p->bestMove = bestMove;
	p->proof_disproof = proofDisproof();
	check_assert(p->isConsistent());
      }
    }
  }
}

template <osl::Player Attacker>
void osl::checkmate::CheckHashRecord::propagateNoCheckmateRecursive()
{
#ifdef CHECKMATE_DEBUG
  static stat::Ratio ratio("propagateNoCheckmate");
#endif
  check_assert(final_by_dominance == 0);
  check_assert(hasDisproofPieces());
  check_assert(! proofDisproof().isLoopDetection());
  for (SameBoardList::iterator p=sameBoards->begin(); p!=sameBoards->end(); ++p)
  {
    if (p->stand(BLACK) == black_stand)
      continue;
    const PieceStand& defense_stand 
      = (Attacker == BLACK) ? p->white_stand : p->black_stand;
    if (defense_stand.template hasMoreThan<BLACK>(disproofPieces()))
    {
#ifdef CHECKMATE_DEBUG
      ratio.add(p->disproof());
#endif    
      check_assert(p->proof()
		   || (p->dump(), dump(), std::cerr << Attacker << "\n", 0));
      if (p->disproof())
      {
	check_assert((p->final_by_dominance == 0) || (p->dump(), 0));
	p->setDisproofPieces(disproofPieces());
	p->setFinalByDominance(this);
	p->bestMove = bestMove;
	p->proof_disproof = proofDisproof();
	p->twins.clear();
      }
    }
  }
}

#ifndef MINIMAL
void osl::checkmate::CheckHashRecord::dump(int dump_depth) const
{
  dump(std::cerr, dump_depth);
}

void osl::checkmate::CheckHashRecord::dump(std::ostream& os, int dump_depth) const
{
  if (dump_depth <= 0)
    return;
  os << "+ CheckHashRecord::dump(" << dump_depth << ") "
     << this << " distance " << distance << " ";
  os << proofDisproof() << " " << filter;
  if (proofDisproof().isFinal())
    os << " " << bestResultInSolved;
  os << "\n"
     << " black " << stand(BLACK) << " white " << stand(WHITE) << "\n"
     << " bestMove " << &*(bestMove);
  if (bestMove)
    os << " " << bestMove->move << " " << bestMove->record;
  if (hasProofPieces())
    os << " pp " << proofPieces();
  else if (hasDisproofPieces())
    os << " dp " << disproofPieces();
  os << "\n";
  if (final_by_dominance)
  {
    os << "--- final_by_dominance " << final_by_dominance << "\n";
    final_by_dominance->dump(os, 1);
    os << "+++ final_by_dominance\n";
  }
  if (parent)
  {
    os << "parent " << parent << " " << parent->proofDisproof();
    os << " " << parent->filter;
    if (parent->bestMove)
      os << " " << parent->bestMove->move << " " << parent->bestMove->record
	 << parent->bestMove->flags;
    if (parent->final_by_dominance)
      os << " f " << final_by_dominance;
    os << "\n";
  }
  moves.dump(os);
  twins.dump(os);
  if (bestMove)
  {
    os << "depth " << dump_depth << " " << bestMove->record << "\n";
    if ((dump_depth > 0) && (bestMove->record))
      bestMove->record->dump(os, dump_depth-1);
  }
}
#endif

#ifdef USE_CHECKMATE_FIND_DAG_ORIGIN
osl::CheckHashRecord*
osl::CheckHashRecord::getGrandParentForDag(bool isAttackNode,
					   osl::CheckHashRecord* child)
{
  if ((! parent) || (! parent->parent))
    return NULL;
  // loop の疑い
  if (parent->useMaxInsteadOfSum || parent->parent->useMaxInsteadOfSum)
    return NULL;
  if ((!isAttackNode)
      ? (parent->proof() != proof())
      : (parent->disproof() != disproof()))
    return NULL;

  child = parent;
  return parent->parent;
}


osl::CheckHashRecord *
osl::CheckHashRecord::findDagOrigin(CheckHashRecord *other, bool isAttackNode)
{
  if (! other)
    return 0;
  assert(this != other);

  CheckHashRecord *cur_child = NULL;
  CheckHashRecord *cur = getGrandParentForDag(isAttackNode, cur_child);
  while (cur)
  {
    CheckHashRecord *work_child = NULL;
    CheckHashRecord *work = other->getGrandParentForDag(isAttackNode,
							work_child);
    while (work)
    {
      if (cur == work)
      {
	
      }
      work = work->getGrandParentForDag(isAttackNode, work_child);
    } 
    cur = cur->getGrandParentForDag(isAttackNode, cur_child);
  }
  return 0;
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
