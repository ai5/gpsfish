#include "osl/rating/ratedMove.h"
#include <iostream>
#include <cassert>

#ifndef MINIMAL
std::ostream& osl::rating::operator<<(std::ostream& os, RatedMove const& move){
  return os << move.move() << "+rate " << move.rating() << " " << move.optimisticRating();
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
