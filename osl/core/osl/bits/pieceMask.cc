#include "osl/bits/pieceMask.h"
#include <iostream>
#include <iomanip>
#include <bitset>

static_assert(sizeof(osl::PieceMask) == 8, "piecemask size");

#ifndef MINIMAL
std::ostream& osl::operator<<(std::ostream& os,const PieceMask& pieceMask){
  os << '(' << std::setbase(16) << std::setfill('0') 
	    << std::setw(12) << pieceMask.getMask(0).value()
	    << std::setbase(10) << ')';
  os << std::bitset<64>(pieceMask.getMask(0).value());
  return os;
}
#endif

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
