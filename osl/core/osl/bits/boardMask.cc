/* boardMask.cc
 */
#include "osl/bits/boardMask.h"
#include "osl/bits/centering5x3.h"
#include <iostream>

#ifndef MINIMAL
std::ostream& osl::container::operator<<(std::ostream& os, const BoardMask& mask)
{
  for(int y=1; y<=9; ++y) {
    for(int x=9; x>=1; --x) {
      const Square p(x,y);
      os << mask.test(p);
    }
    os << std::endl;
  }
  return os;
}
#endif

osl::container::
BoardMaskTable5x5::BoardMaskTable5x5()
{
  for (int cx=1; cx<=9; ++cx) {
    for (int cy=1; cy<=9; ++cy) {
      const int min_x = std::max(1, cx - 2);
      const int max_x = std::min(9, cx + 2);
      const int min_y = std::max(1, cy - 2);
      const int max_y = std::min(9, cy + 2);
      BoardMask mask;
      mask.clear();
      for (int x=min_x; x<=max_x; ++x) {
	for (int y=min_y; y<=max_y; ++y) {
	  mask.set(Square(x,y));
	}
      }
      data[Square(cx,cy).index()] = mask;
    }
  }
}

osl::container::
BoardMaskTable3x3::BoardMaskTable3x3()
{
  for (int cx=1; cx<=9; ++cx) {
    for (int cy=1; cy<=9; ++cy) {
      const int min_x = std::max(1, cx - 1);
      const int max_x = std::min(9, cx + 1);
      const int min_y = std::max(1, cy - 1);
      const int max_y = std::min(9, cy + 1);
      BoardMask mask;
      mask.clear();
      for (int x=min_x; x<=max_x; ++x) {
	for (int y=min_y; y<=max_y; ++y) {
	  mask.set(Square(x,y));
	}
      }
      data[Square(cx,cy).index()] = mask;
    }
  }
}

osl::container::
BoardMaskTable5x3Center::BoardMaskTable5x3Center()
{
  for (int cx=1; cx<=9; ++cx) {
    for (int cy=1; cy<=9; ++cy) {
      const Square center = Centering5x3::adjustCenter(Square(cx, cy));
      const int min_x = std::max(1, center.x() - 2);
      const int max_x = std::min(9, center.x() + 2);
      const int min_y = std::max(1, center.y() - 1);
      const int max_y = std::min(9, center.y() + 1);
      BoardMask mask;
      mask.clear();
      for (int x=min_x; x<=max_x; ++x) {
	for (int y=min_y; y<=max_y; ++y) {
	  mask.set(Square(x,y));
	}
      }
      data[Square(cx,cy).index()] = mask;
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
