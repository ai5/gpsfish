// pieceValues.cc
#include "osl/container/pieceValues.h"
#include "osl/numEffectState.h"
#include <iostream>
#include <iomanip>

osl::container::
PieceValues::PieceValues()
{
}

osl::container::
PieceValues::~PieceValues()
{
}

int osl::container::
PieceValues::sum() const
{
  int result = 0;
  for (int v: *this)
  {
    result += v;
  }
  return result;
}

#ifndef MINIMAL
void osl::container::
PieceValues::showValues(std::ostream& os, const NumEffectState& state) const
{
  for (int y=1;y<=9;y++) {
    os << y;  
    for (int x=9;x>0;x--) {
      const Piece piece = state.pieceOnBoard(Square(x,y));
      os << std::setw(7);
      if (piece.isEmpty())
	os << 0;
      else
	os << (*this)[piece.number()];
    }
    os << std::endl;
  }
  os << "black stand: ";
  for (int i=0; i<Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if ((piece.owner() == BLACK)
	&& (piece.square().isPieceStand()))
      os << piece.ptype() << " " << (*this)[piece.number()] << " ";
  }
  os << "\n";
  os << "white stand: ";
  for (int i=0; i<Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    if ((piece.owner() == WHITE)
	&& (piece.square().isPieceStand()))
      os << piece.ptype() << " " << (*this)[piece.number()] << " ";
  }
  os << "\n";
  os << "total: " << sum() << "\n";
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
