/* historyTable.cc
 */
#include "osl/search/historyTable.h"
#include <algorithm>
#include <functional>
#include <iostream>

void osl::search::
HistoryTable::extractTopN(Player p, std::vector<OutputEntry>& out, size_t limit) const
{
  out.clear();
  for (size_t i=0; i<=Square(9,9).uintValue(); ++i)
    for (size_t j=Square(1,1).uintValue(); j<=Square(9,9).uintValue(); ++j)
      if (table[p][i][j].value > 100)
	out.push_back(OutputEntry(i, j, table[p][i][j].value));
  std::sort(out.begin(), out.end(), std::greater<OutputEntry>());
  if (limit < out.size())
    out.resize(limit);
}

std::ostream& osl::search::operator<<(std::ostream& os, const HistoryTable::OutputEntry& e)
{
  os << '(';
  if (e.from_or_ptype < PTYPE_SIZE)
    os << Ptype(e.from_or_ptype);
  else
    os << Square::makeDirect(e.from_or_ptype);
  return os << " => " << e.to << " " << e.value << ")";
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
