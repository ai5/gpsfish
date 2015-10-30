#include "arrayCheckHashTable.h"
#include <iostream>

#ifndef MINIMAL
osl::checkmate::ArrayCheckHashTable::ArrayCheckHashTable(Player a)
  : buckets(new ElementList[bucketSize]), numElements(0), attacker(a)
{
}
osl::checkmate::ArrayCheckHashTable::~ArrayCheckHashTable()
{
#ifdef CHECKMATE_DEBUG
  confirmNoVisitedRecords();
#endif
}

void osl::checkmate::ArrayCheckHashTable::clear()
{
  TwinTableHolder::clear();
  for (unsigned int i=0; i<bucketSize; ++i)
  {
    ElementList& e = buckets[i];
    e.elem.colleagues.clear();
    e.list.clear();
  }
  numElements = 0;
}

void osl::checkmate::ArrayCheckHashTable::confirmNoVisitedRecords() const
{
  int visited = 0;
  for (unsigned int i=0; i<bucketSize; ++i)
  {
    const ElementList& e = buckets[i];
    visited += e.elem.colleagues.confirmNoVisitedRecords();
    for (list_t::const_iterator p=e.list.begin(); p!=e.list.end(); ++p)
    {
      const BoardEntry& b = *p;
      int curVisited = b.colleagues.confirmNoVisitedRecords();
      if (curVisited)
	std::cerr << b.board_key[0]
		  << " " << b.board_key[1]
		  << "\n";
      visited += curVisited;
    }
  }
  if (visited)
  {
    std::cerr << "ArrayCheckHashTable::confirmNoVisitedRecords " 
	      << visited << " visited nodes found\n";
  }
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
