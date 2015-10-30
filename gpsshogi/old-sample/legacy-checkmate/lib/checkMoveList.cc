/* checkMoveList.cc
 */
#include "checkMoveList.h"
#include "checkMoveListProvider.h"
#include "checkHashRecord.h"
#include <iostream>

#ifndef MINIMAL
std::ostream& osl::checkmate::
operator<<(std::ostream& os, const CheckMoveList& l)
{
  for (CheckMoveList::const_iterator p=l.begin(); p!=l.end(); ++p)
  {
    os << p->move << " ";
  }
  return os << "\n";
}
#endif
/* ------------------------------------------------------------------------- */

void osl::checkmate::
CheckMoveListBase::setSize(size_t length, CheckMoveListProvider& src)
{
  first = src.alloc(length);
  last = first + length;
}

void osl::checkmate::
CheckMoveListBase::setOne(const CheckMove& data, CheckMoveListProvider& src)
{
  if (first == last)
    setSize(1, src);
  *first = data;
}

/* ------------------------------------------------------------------------- */

const osl::checkmate::CheckMove* osl::checkmate::
CheckMoveList::find(Move move) const
{
  for (const_iterator p=begin(); p!=end(); ++p)
  {
    if (p->move == move)
      return &*p;
  }
  return 0;
}

osl::checkmate::CheckMove* osl::checkmate::
CheckMoveList::find(Move move)
{
  for (iterator p=begin(); p!=end(); ++p)
  {
    if (p->move == move)
      return &*p;
  }
  return 0;
}

#ifndef MINIMAL
void osl::checkmate::
CheckMoveList::dump(std::ostream& os) const
{
  os << "moves size " << size() << "\n";
  for (const_iterator p=begin(); p!=end(); ++p)
  {
    os << "moves " << p->flags << " " << p->move;
    if (p->record)
    {
      os << " " << p->record->proofDisproof()
	 << " " << p->record << " parent " << p->record->parent;
    }
    else
    {
      os << " h " << (int)p->h_proof << " " << (int)p->h_disproof
	 << " c " << (int)p->cost_proof << " " << (int)p->cost_disproof;
    }
    os << "\n";
  }
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
