/* printPiece.cc
 */
#include "osl/basic_type.h"
#include <iostream>

using namespace osl;
int main()
{
  unsigned long position;
  while (std::cin >> position)
  {
    Square p = Square::makeDirect(position);
    std::cout << p.x() << " " << p.y() << " " << std::boolalpha << p.isOnBoard() << std::endl;
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
