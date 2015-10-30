/* sameBoardList.cc
 */
#include "sameBoardList.h"
#include <iostream>
osl::checkmate::
SameBoardList::~SameBoardList()
{
}

void osl::checkmate::
SameBoardList::clear() 
{
  colleagues.clear();
}

size_t osl::checkmate::
SameBoardList::confirmNoVisitedRecords() const
{
  size_t visited = 0;
  for (list_t::const_iterator q=colleagues.begin(); q!=colleagues.end(); ++q)
  {
    if (q->isVisited)
    {
      ++visited;
#ifdef CHECKMATE_EXTRA_DEBUG
      std::cerr << q->stand(BLACK) << " " << &*q << "\n";
#endif
    }
  }
  return visited;
}

void osl::checkmate::
SameBoardList::setMoreProvable(unsigned int& proofLL, unsigned int& disproofUL,
			       const CheckHashRecord *& final_by_dominance, 
			       CheckHashRecord& record)
{
  // key の方が record より詰みにくい
#ifdef SAMEBOARDLIST_STAT
  static stat::Ratio ratioDom("SameBoardList: disproof by dominance");
#endif
  const unsigned int p = record.proof();
  const unsigned int d = record.disproof();
#ifdef SAMEBOARDLIST_STAT
  ratioDom.add(d == 0);
#endif
  if (d == 0)
  {
    disproofUL = d;	// 不詰
    if (proofLL <= p)
    {
      proofLL = p;
      final_by_dominance = &record;
    }
    check_assert(final_by_dominance);
  }
  else
  {
    proofLL    = std::max(p, proofLL);
    disproofUL = std::min(d, disproofUL);
  }
}

void osl::checkmate::
SameBoardList::setLessProvable(unsigned int& proofUL, unsigned int& disproofLL, 
			       const CheckHashRecord *& final_by_dominance,
			       CheckHashRecord& record)
{
#ifdef SAMEBOARDLIST_STAT
  static stat::Ratio ratioDom("SameBoardList: proof by dominance");
#endif	
  // key の方が record より詰みやすい
  const unsigned int p = record.proof();
  const unsigned int d = record.disproof();
#ifdef SAMEBOARDLIST_STAT
  ratioDom.add(p == 0);
#endif
  if (p == 0)
  {
    proofUL = p;		// 詰
    if (disproofLL <= d)
    {
      disproofLL = d;
      final_by_dominance = &record;
    }
    check_assert(final_by_dominance);
  }
  else
  {
    proofUL    = std::min(p, proofUL);
    disproofLL = std::max(d, disproofLL);
  }
}

osl::checkmate::CheckHashRecord * osl::checkmate::
SameBoardList::allocateSlow(Player attacker,
			    const PieceStand& black_stand,
			    const PieceStand& white_stand,
			    const PathEncoding& path,
			    size_t& counter)      
{
  if (attacker == BLACK)
    return allocate<BLACK>(black_stand, white_stand, path, counter);
  else
    return allocate<WHITE>(black_stand, white_stand, path, counter);
}
      

template <osl::Player Attacker>
osl::checkmate::CheckHashRecord * osl::checkmate::
SameBoardList::allocate(const PieceStand& black_stand,
			const PieceStand& white_stand,
			const PathEncoding& path,
			size_t& counter)
{
  CheckHashRecord *result=0;
  unsigned int proofUL=ProofDisproof::PROOF_MAX; // upper limit
  unsigned int proofLL=1;	// lower limit
  unsigned int disproofUL=ProofDisproof::DISPROOF_MAX;
  unsigned int disproofLL=1;
  const CheckHashRecord *final_by_dominance = 0;
  const CheckHashRecord *ineffectiveDropLoop = 0;

  const PieceStand& attack_stand
    = (Attacker == BLACK) ? black_stand : white_stand;

  for (list_t::iterator p=begin(); p!=end(); ++p)
  {
    if (p->stand(BLACK) == black_stand)
    {
      result = &*p;
      if (result->proofDisproof().isFinal())
	return result;
      if (result->findLoopInList(path))
	return result;
      if ((proofUL == 0) || (disproofUL == 0))
	break;
      
      continue;
    }
    // 証明駒
    if (p->hasProofPieces()
	&& attack_stand.hasMoreThan<BLACK>(p->proofPieces()))
    {
      assert(p->proofDisproof().isCheckmateSuccess());
      proofUL = 0;		// 詰
      disproofLL = p->disproof();
      final_by_dominance = &*p;
      if (result)
	break;
    }
    // 反証駒
    // 普通の持駒
    if (p->stand(BLACK).template hasMoreThan<Attacker>(black_stand))
    {
      if (p->isVisited)	// わざわざ持駒を少なくした
      {
	ineffectiveDropLoop = &*p;
	continue;
      }
      setMoreProvable(proofLL, disproofUL, final_by_dominance, *p);
    }
    else if ((! p->isVisited)
	     && (black_stand.template hasMoreThan<Attacker>(p->stand(BLACK))))
    {
      setLessProvable(proofUL, disproofLL, final_by_dominance, *p);
    }
  }
  if (result)
  {
    if (ineffectiveDropLoop)
      result->setLoopDetection(path, ineffectiveDropLoop);
    if (! ((proofUL == 0) || (disproofUL == 0)))
      return result;
  }
  if (! result)
  {
    ++counter; 
    colleagues.push_front(CheckHashRecord(black_stand, white_stand));
    result = &colleagues.front();
    result->sameBoards = this;
  }
  if (proofUL == 0)
  {
    assert(disproofUL);
    result->setProofByDominance(disproofLL, final_by_dominance);
    return result;
  }
  if (disproofUL == 0)
  {
    assert(proofUL);
    result->setDisproofByDominance(proofLL, final_by_dominance);
    return result;
  }
  assert(proofLL);
  assert(disproofLL);
  // TODO: loop がらみのlower limit は不当に高い場合がある?
  result->setProofDisproof(ProofDisproof(proofLL, disproofLL));
  if (ineffectiveDropLoop)
    result->setLoopDetection(path, ineffectiveDropLoop);
  return result;
}

void osl::checkmate::
SameBoardList::updateSlow(bool is_attack, Player attacker, 
			  CheckHashRecord& record, const PathEncoding& path)
{
  if (is_attack)
    updateSlow<true>(attacker, record, path);
  else
    updateSlow<false>(attacker, record, path);
}

template <bool isAttack, osl::Player Attacker>
void osl::checkmate::
SameBoardList::update(CheckHashRecord& record, const PathEncoding& path)
{
#ifdef CHECKMATE_DEBUG
  static stat::Ratio ratio("SameBoardList: skip update by ancestor");
#endif
  unsigned int proofUL=ProofDisproof::PROOF_MAX; // upper limit
  unsigned int proofLL=record.proof();	// lower limit
  unsigned int disproofUL=ProofDisproof::DISPROOF_MAX;
  unsigned int disproofLL=record.disproof();
  check_assert(proofLL);
  check_assert(disproofLL);
  const PieceStand black_stand = record.stand(BLACK);
#ifdef CHECKMATE_DEBUG
  bool found_myself = false;
#endif
  const PieceStand attack_stand = record.stand(Attacker);
  const CheckHashRecord *final_by_dominance = record.finalByDominance();

  for (list_t::iterator p=begin(); p!=end(); ++p)
  {
    if (p->stand(BLACK) == black_stand)
    {
      check_assert(&*p == &record);
#ifdef CHECKMATE_DEBUG
      found_myself = true;
#endif
      continue;
    }
    // 証明駒
    if (p->hasProofPieces()
	&& attack_stand.hasMoreThan<BLACK>(p->proofPieces()))
    {
      assert(p->proofDisproof().isCheckmateSuccess());
      proofUL = 0;		// 詰
      disproofLL = p->disproof();
      final_by_dominance = &*p;
      break;
    }
    // 反証駒
    // 普通の持駒
    const bool skip_ancestor = (p->distance < record.distance)
      || ((p->distance == record.distance) && p->useMaxInsteadOfSum);
#ifdef CHECKMATE_DEBUG
    ratio.add(skip_ancestor);
#endif
    if (skip_ancestor)
      continue; // loopと合流で無限ポンプができるため，TODO: 条件を絞れるかも
    if (p->stand(BLACK).template hasMoreThan<Attacker>(black_stand))
    {
      if (p->isVisited)	// わざわざ持駒を少なくした
      {
	record.setLoopDetection(path, &*p);
	return;
      }
      setMoreProvable(proofLL, disproofUL, final_by_dominance, *p);
    }
    else if ((! p->isVisited)
	     && (black_stand
		 .template hasMoreThan<Attacker>(p->stand(BLACK))))
    {
      setLessProvable(proofUL, disproofLL, final_by_dominance, *p);
    }
  }
#ifdef CHECKMATE_DEBUG
  assert(final_by_dominance || found_myself);
#endif
  if (proofUL == 0)
  {
    check_assert(disproofUL);
    record.setProofByDominance(disproofLL, final_by_dominance);
    return;
  }
  if (disproofUL == 0)
  {
    check_assert(proofUL);
    record.setDisproofByDominance(proofLL, final_by_dominance);
    return;
  }
  check_assert(proofUL);
  check_assert(disproofUL);
  // TODO: loop がらみのlower limit は不当に高い場合がある?
  record.setProofDisproof(proofLL, disproofLL);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
