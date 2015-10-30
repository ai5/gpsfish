#include "osl/book/compactBoard.h"
#include <iostream>
#include <algorithm>
#include <sstream>

int osl::book::
OPiece::position2Bits(const Square& pos)
{
  return pos.isPieceStand() ? 0 : ((pos.x() << 4) | pos.y()); // 8 bits
}

osl::Square osl::book::
OPiece::bits2Square(const int bit_position)
{
  if ((bit_position & 0xff) == 0)
    return Square::STAND();
  else
    return Square((bit_position >> 4) & 0xf, bit_position & 0xf);
}

namespace osl
{
  namespace book
  {

    struct opiece_sort
    { 
      bool operator()(const OPiece& l, const OPiece& r) {
	if (l.square() == Square::STAND() || r.square() == Square::STAND()) {
	  if (l.square() != r.square())
	    return l.square() == Square::STAND();
	  if (l.owner() != r.owner())
	    return l.owner() == WHITE;
	  return l.ptype() < r.ptype();
	}
	if (l.square().x() != r.square().x())
	  return l.square().x() < r.square().x();
	return l.square().y() < r.square().y();
      }
    };
  }
}

osl::book::
CompactBoard::CompactBoard(const SimpleState& state)
{
  piece_vector.reserve(40);
  for (int i = 0; i < 40; i++)
  {
    if(state.usedMask().test(i))
      piece_vector.push_back(OPiece(state.pieceOf(i)));
  }
  std::sort(piece_vector.begin(), piece_vector.end(), opiece_sort());
  player_to_move = state.turn();
}

osl::SimpleState osl::book::
CompactBoard::state() const
{

  SimpleState state;
  state.init();

  for (const OPiece& p: piece_vector) {
    state.setPiece(p.owner(), p.square(), p.ptype());
  }
  state.setTurn(turn());
  state.initPawnMask();
  return state;
}

bool osl::book::
operator==(const CompactBoard& lhs, const CompactBoard& rhs)
{
  return (lhs.turn() == rhs.turn()) && (lhs.pieces() == rhs.pieces());
}

std::ostream& osl::book::
operator<<(std::ostream& os, const CompactBoard& c)
{

  for (unsigned int i = 0; i < c.pieces().size(); i++)
  {
    writeInt(os, static_cast<int>(c.pieces()[i]));
  }
  writeInt(os, static_cast<int>(c.turn()));
  return os;
}

std::istream& osl::book::
operator>>(std::istream& is, CompactBoard& c)
{
  assert(c.piece_vector.size() == 0);

  for (unsigned int i = 0; i < 40; i++)
  {
    c.piece_vector.push_back(OPiece(readInt(is)));
  }
  c.player_to_move = static_cast<Player>(readInt(is));
  return is;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
