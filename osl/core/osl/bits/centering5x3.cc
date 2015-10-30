/* centering5x3.cc
 */
#include "osl/bits/centering5x3.h"
#include "osl/basic_type.h"

osl::Centering5x3::
Table::Table()
{
  centers.fill(Square::STAND());
  for (int y=1; y<=9; ++y)
  {
    for (int x=1; x<=9; ++x)
    {
      const Square src = Square(x,y);
      centers[src.index()] = adjustCenterNaive(src);
    }
  }
}

namespace
{
  int adjustCenterX(int x)
  {
    if (x < 3)
      return 3;
    else if (x > 7)
      return 7;
    return x;
  }
  int adjustCenterY(int y)
  {
    if (y == 1)
      return y+1;
    else if (y == 9)
      return y-1;
    return y;
  }
} // anonymous namespace

const osl::Square osl::
Centering5x3::adjustCenterNaive(Square src)
{
  const int x = adjustCenterX(src.x());
  const int y = adjustCenterY(src.y());
  return Square(x, y);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
