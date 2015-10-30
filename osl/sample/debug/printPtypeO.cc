/* printPtypeO.cc
 */
#include "osl/basic_type.h"
#include <iostream>

using namespace osl;
int main()
{
  long ptypeo;
  while (std::cin >> ptypeo)
  {
    PtypeO p = static_cast<PtypeO>(ptypeo);
    std::cout << p << std::endl;
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
