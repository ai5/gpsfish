/* centering3x3.cc
 */
#include "osl/centering3x3.h"
#include "osl/oslConfig.h"

osl::Centering3x3::Table osl::Centering3x3::table;

static osl::SetUpRegister _initializer([](){ osl::Centering3x3::table.init(); });

void osl::Centering3x3::
Table::init()
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
  int adjustCenterXY(int xy)
  {
    if (xy == 1)
      return xy+1;
    else if (xy == 9)
      return xy-1;
    return xy;
  }
} // anonymous namespace

const osl::Square osl::
Centering3x3::adjustCenterNaive(Square src)
{
  const int x = adjustCenterXY(src.x());
  const int y = adjustCenterXY(src.y());
  return Square(x, y);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
