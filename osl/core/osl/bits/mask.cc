#include "osl/bits/mask.h"
#include <iostream>
#include <iomanip>

namespace osl
{
  static_assert(sizeof(mask_t) == 8, "64bit");
}

std::ostream& osl::misc::operator<<(std::ostream& os,const osl::mask_t& mask)
{
  return os << "mask(0x" << std::setbase(16) <<
    mask.value() << std::setbase(10) << ')';
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

