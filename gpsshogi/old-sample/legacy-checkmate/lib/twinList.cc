/* twinList.cc
 */
#include "twinList.h"
#include "checkHashRecord.h"
#include <iostream>

void osl::checkmate::
TwinList::clear()
{
  slist<TwinEntry>::clear();
  size_cache = 0;
}

#ifndef MINIMAL
void osl::checkmate::
TwinList::dump(std::ostream& os) const
{
  os << "twin list " << size() << "\n";
  for (const_iterator p=begin(); p!=end(); ++p)
  {
    os << "twins " << p->path << " " << p->move.move 
       << " loop to " << p->loopTo;
    if (p->move.record)
    {
      os << " " << p->move.record->proofDisproof()
	 << " " << p->move.record
	 << " #next-twins " << p->move.record->twins.size();
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
