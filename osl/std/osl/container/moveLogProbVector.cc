#include "osl/container/moveLogProbVector.h"
#include <algorithm>
#include <iostream>

// 厳密な整列は特に必要ではない
// #define RIGID_SORT_OF_MOVE

#ifndef MINIMAL
std::ostream& osl::container::operator<<(std::ostream& os,MoveLogProbVector const& mv)
{
  os<< "LogProbVector" << std::endl;
  for (const MoveLogProb& move: mv)
  {
    os << move << std::endl;
  }
  return os << std::endl;
}
#endif
bool osl::container::operator==(const MoveLogProbVector& l, const MoveLogProbVector& r)
{
  return l.size() == r.size()
    && std::equal(l.begin(), l.end(), r.begin());
}

namespace osl
{
  template <bool isLess>
  struct LogProbCompare
  {
    bool operator()(const MoveLogProb& l, const MoveLogProb& r) const
    {
#ifdef RIGID_SORT_OF_MOVE
      if (l.logProb() != r.logProb())
      {
#endif
	if (isLess)
	  return l.logProb() < r.logProb();
	else
	  return l.logProb() > r.logProb();
#ifdef RIGID_SORT_OF_MOVE
      }
      return l.move() > r.move();
#endif
    }
  };
}

void osl::container::MoveLogProbVector::sortByProbability()
{
  std::sort(begin(), end(), LogProbCompare<true>());
}
void osl::container::MoveLogProbVector::sortByProbabilityReverse()
{
  std::sort(begin(), end(), LogProbCompare<false>());
}

const osl::MoveLogProb* osl::container::MoveLogProbVector::find(Move m) const
{
  for (const_iterator p=begin(); p!=end(); ++p)
    if (p->move() == m)
      return &*p;
  return 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
