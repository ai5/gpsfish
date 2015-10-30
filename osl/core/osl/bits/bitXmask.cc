/* bitXmask.cc
 */
#include "osl/bits/bitXmask.h"
#include <iostream>

std::ostream& osl::container::operator<<(std::ostream& os,const BitXmask mask){
  for(int i=1;i<=9;i++)
    os << ((mask.intValue()>>i)&1);
  return os;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
