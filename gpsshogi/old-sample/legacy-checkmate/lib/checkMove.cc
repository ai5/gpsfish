/* checkMove.cc
 */
#include "checkMove.h"
#include <iostream>

#ifndef MINIMAL
namespace osl
{
  using namespace checkmate;
namespace
{
  void show_flags(std::ostream& os, int flags)
  {
    os << ((flags & MoveFlags::ImmediateCheckmate) ? 'I' : '-')
       << ((flags & MoveFlags::Upward) ? 'u' : '-')
       << ((flags & MoveFlags::NoPromote) ? 'n' : '-')
       << ((flags & MoveFlags::SacrificeAttack) ? 's' : '-')
       << ((flags & MoveFlags::BlockingBySacrifice) ? 'b' : '-')
       << ((flags & MoveFlags::Solved) ? 'S' : '-');
  }
}
}

std::ostream& osl::checkmate::operator<<(std::ostream& os, MoveFlags f)
{
  show_flags(os, f.getFlags());
  return os;
}

std::ostream& osl::checkmate::operator<<(std::ostream& os, MoveFilter f)
{
  show_flags(os, ~f.getMask());
  return os;
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
