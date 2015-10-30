/* miniBoardChar50.cc
 */
#include "osl/book/miniBoardChar50.h"
#include "osl/book/compactBoard.h"
#include "osl/bits/ptypeTable.h"
#include "osl/bits/pieceTable.h"
#include <tuple>
#include <algorithm>
#include <stdexcept>

osl::book::
MiniBoardChar50::MiniBoardChar50()
{
  data.fill(0);
}

osl::book::
MiniBoardChar50::MiniBoardChar50(const SimpleState& org)
{
  data.fill(0);
  SimpleState board = (org.turn() == BLACK) ? org : org.rotate180();
  CArray<std::tuple<int/* =Ptype*/,bool,int /* =Player*/,Square>, Piece::SIZE> pieces;
  for (int i=0; i<Piece::SIZE; ++i) 
  {
    const Piece p = board.pieceOf(i);
    const int ptype_index = Ptype_Table.getIndexMin(unpromote(p.ptype()));
    pieces[i] = std::make_tuple(ptype_index, p.isPromoted(), p.owner(), p.square());
  }
  std::sort(pieces.begin(), pieces.end());
  for (int i=0; i<Piece::SIZE; ++i) 
  {
    data[i] = OPiece::position2Bits(std::get<3>(pieces[i]));
    data[Piece::SIZE + i/8] |= playerToIndex(static_cast<Player>(std::get<2>(pieces[i]))) << (i%8);
    data[Piece::SIZE + i/8 + 5] |= std::get<1>(pieces[i]) << (i%8);
  }
}

osl::book::
MiniBoardChar50::MiniBoardChar50(const std::string& src)
{
  if (src.size() != data.size())
    throw std::runtime_error("bad argument in MiniBoardChar50::MiniBoardChar50(const std::string&)");
  std::copy(src.begin(), src.end(), data.begin());
}

const osl::SimpleState osl::book::
MiniBoardChar50::toSimpleState(Player turn) const
{
  SimpleState state;
  state.init();

  for (int i = 0; i<Piece::SIZE; i++)
  {
    const Square position = OPiece::bits2Square(data[i]);
    const Player owner = indexToPlayer((data[40+i/8] >> (i%8)) & 1);
    const bool promoted = (data[40+i/8+5] >> (i%8)) & 1;
    Ptype ptype = Piece_Table.getPtypeOf(i);
    if (promoted)
      ptype = promote(ptype);
    state.setPiece(owner, position, ptype);
  }
  state.setTurn(BLACK);
  state.initPawnMask();
  if (turn != BLACK)
    state = state.rotate180();
  assert(state.turn() == turn);
  return state;
}

const std::string osl::book::
MiniBoardChar50::toString() const
{
  return std::string(data.begin(), data.end());
}

bool osl::book::operator<(const MiniBoardChar50& l, const MiniBoardChar50& r)
{
  return std::lexicographical_compare(l.data.begin(), l.data.end(), 
				      r.data.begin(), r.data.end());
}
bool osl::book::operator==(const MiniBoardChar50& l, const MiniBoardChar50& r)
{
  return std::equal(l.data.begin(), l.data.end(), r.data.begin());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
