/* sennichite.cc
 */
#include "osl/sennichite.h"
#include <stdexcept>
#include <iostream>

osl::Player osl::Sennichite::winner() const
{
  if (value == Result::BLACK_LOSE)
    return WHITE;
  else if (value == Result::WHITE_LOSE)
    return BLACK;
  throw std::runtime_error("no winner");
}

std::ostream& osl::operator<<(std::ostream& os, const Sennichite& s)
{
  if (s.isNormal())
    os << "sennichite normal";
  else if (s.isDraw())
    os << "sennichite draw";
  else 
    os << "sennichite " << s.winner() << " win";
  return os;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
