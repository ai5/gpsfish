/* positionGroup.cc
 */
#include "osl/rating/group/squareGroup.h"

osl::rating::
RelativeKingXGroup::RelativeKingXGroup(bool a) : Group(a ? "KingRelAX" : "KingRelDX"), attack(a)
{
  for (int x=0; x<9; ++x) {
    for (int old_x=0; old_x<10; ++old_x) {
      for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s) {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new RelativeKingX(x, old_x, a, static_cast<Ptype>(s)));
      }
    }
  }
}

osl::rating::
RelativeKingYGroup::RelativeKingYGroup(bool a) : Group(a ? "KingRelAY" : "KingRelDY"), attack(a)
{
  for (int y=-8; y<9; ++y) {
    for (int old_y=-8; old_y<10; ++old_y) {
      for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s) {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new RelativeKingY(y, old_y, a, static_cast<Ptype>(s)));
      }
    }
  }
}

osl::rating::SquareXGroup::SquareXGroup() : Group("SquareX")
{
  for (int x=1; x<=5; ++x) {
    for (int s=PTYPE_PIECE_MIN; s<=PTYPE_BASIC_MIN; ++s) { // we cannot drop KING 
      for (int p=0; p<8; ++p)	// progress8
	push_back(new SquareX(x, static_cast<Ptype>(s), false));
    }
    for (int s=PTYPE_BASIC_MIN+1; s<=PTYPE_MAX; ++s) {
      for (int p=0; p<8; ++p)	// progress8
	push_back(new SquareX(x, static_cast<Ptype>(s), false));
      for (int p=0; p<8; ++p)	// progress8
	push_back(new SquareX(x, static_cast<Ptype>(s), true));
    }
  }
}

osl::rating::SquareYGroup::SquareYGroup() : Group("SquareY")
{
  for (int y=1; y<=9; ++y) {
    for (int s=PTYPE_PIECE_MIN; s<=PTYPE_BASIC_MIN; ++s) { // we cannot drop KING 
      for (int p=0; p<8; ++p)	// progress8
	push_back(new SquareY(y, static_cast<Ptype>(s), false));
    }
    for (int s=PTYPE_BASIC_MIN+1; s<=PTYPE_MAX; ++s) {
      for (int p=0; p<8; ++p)	// progress8
	push_back(new SquareY(y, static_cast<Ptype>(s), false));
      for (int p=0; p<8; ++p)	// progress8
	push_back(new SquareY(y, static_cast<Ptype>(s), true));
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
