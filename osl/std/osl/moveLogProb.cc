#include "osl/moveLogProb.h"
#include <iostream>
#include <cassert>

#ifndef MINIMAL
namespace osl{
  std::ostream& operator<<(std::ostream& os,MoveLogProb const& moveLogProb){
    return os << "MoveLogProb("<< moveLogProb.move() << "," <<
      moveLogProb.logProb() << ")";
  }
}
#endif
