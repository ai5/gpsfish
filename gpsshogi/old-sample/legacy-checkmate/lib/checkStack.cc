/* checkStack.cc
 */
#include "checkStack.h"
#include "checkHashRecord.h"
#include "osl/record/csa.h"
#include <iostream>

osl::checkmate::CheckStackEntry::
CheckStackEntry(const CheckMove *nextMove, const char *name, 
		const CheckHashRecord *r, 
		const HashKey& h, const PathEncoding& path, 
		unsigned int proofLimit, unsigned int disproofLimit)
  : move(nextMove ? *nextMove : CheckMove()),
    name(name), record(r), hash(h), path(path), 
    proofLimit(proofLimit), disproofLimit(disproofLimit)
{
}


osl::checkmate::CheckStack::
~CheckStack()
{
}

osl::checkmate::CheckStack::const_iterator osl::checkmate::CheckStack::
findLoop(const CheckHashRecord *record) const
{
  for (const_iterator p=begin(); p!=end(); ++p)
  {
    if (p->record == record)
      return p;
  }
  return end();
}

osl::checkmate::CheckStack::const_iterator osl::checkmate::CheckStack::
findCover(Player attacker, const HashKey& hash, const CheckHashRecord *record) const
{
  for (const_iterator p=begin(); p!=end(); ++p)
  {
    assert(p->record);
    if (! p->hash.isSameBoard(hash))
      continue;
    if (p->record->stand(BLACK).hasMoreThan(attacker, record->stand(BLACK))
	&& (p->record->stand(BLACK) != record->stand(BLACK)))
      return p;
  }
  return end();
}

#ifndef MINIMAL
std::ostream& osl::checkmate::
operator<<(std::ostream& os, const CheckStack& s)
{
  for (size_t i=0; i<s.size(); ++i)
  {
    std::cerr << i << " " << s[i] << "\n";
  }
  return os;
}

std::ostream& osl::checkmate::
operator<<(std::ostream& os, const CheckStackEntry& e)
{
  csaShow(os, e.move.move);
  // os << e.move.move;
  os << " " << e.move.flags << " " << e.name << " " << e.record 
     << " " << e.path;
  if (e.proofLimit)
    os << " pl" << e.proofLimit;
  if (e.disproofLimit)
    os << " dl" << e.disproofLimit;
  if (e.record)
  {
    os << " depth " << e.record->distance
      // << " " << e.record->proofDisproof()
       << " m " << e.record->moves.size();
#if 0
    // TODO: 内訳
    << " s " << e.record->solvedMoves.size()
    << " u " << e.record->upwardMoves.size();
    if (e.record->interposeMoves.size())
      os << " i " << e.record->interposeMoves.size();
    if (e.record->noPromoteMoves.size())
      os << " n " << e.record->noPromoteMoves.size();
#endif
    os << " p " << e.record->proof() 
       << " d " << e.record->disproof()
       << " " << e.record->filter;
    if (e.record->finalByDominance())
      os << " dom by " << e.record->finalByDominance();
    if (! e.record->twins.empty())
    {
      os << " twins " << e.record->twins.size();
      for (TwinList::const_iterator p=e.record->twins.begin();
	   p!=e.record->twins.end(); ++p)
      {
	os << " " << p->path << " ";
	csaShow(os, p->move.move);
      }
    }
  }
  return os;
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
