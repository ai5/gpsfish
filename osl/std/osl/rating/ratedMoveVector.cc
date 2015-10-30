/* ratedMoveVector.cc
 */
#include "osl/rating/ratedMoveVector.h"
#include <algorithm>
#include <functional>
#include <iostream>

#ifndef MINIMAL
std::ostream& osl::rating::operator<<(std::ostream& os, RatedMoveVector const& mv)
{
  os<< "RatedMoves" << std::endl;
  for (const auto& move: mv) {
    os << move << std::endl;
  }
  return os << std::endl;
}
#endif

bool osl::rating::operator==(const RatedMoveVector& l, const RatedMoveVector& r)
{
  return l.size() == r.size()
    && std::equal(l.begin(), l.end(), r.begin());
}

void osl::rating::RatedMoveVector::sort()
{
  std::sort(begin(), end(), std::greater<RatedMove>());
}

const osl::rating::RatedMove* osl::rating::RatedMoveVector::find(Move m) const
{
  for (const_iterator p=begin(); p!=end(); ++p)
    if (p->move() == m)
      return &*p;
  return 0;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

