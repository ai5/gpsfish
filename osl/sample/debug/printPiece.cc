/* printPiece.cc
 */
#include "osl/basic_type.h"
#include <iostream>

using namespace osl;
int main()
{
  int piece;
  while (std::cin >> piece)
  {
    Piece p = Piece::makeDirect(piece);
    std::cout << p << std::endl;
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
