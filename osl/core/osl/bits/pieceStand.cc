/* pieceStand.cc
 */
#include "osl/bits/pieceStand.h"
#include "osl/bits/ptypeTable.h"
#include "osl/simpleState.h"
#include <iostream>

namespace osl
{
  static_assert(sizeof(unsigned int)*/*CHARBITS*/8>=32, "PieceStand");

  const CArray<Ptype,7> PieceStand::order =
  {{
    ROOK, BISHOP, GOLD, SILVER, KNIGHT, LANCE, PAWN,
  }};

  const CArray<unsigned char,PTYPE_MAX+1> PieceStand::shift =
  {{
    0,0,0,0,0,0,0,0,
    28, 24, 18, 14, 10, 6, 3, 0,
  }};
  const CArray<unsigned char,PTYPE_MAX+1> PieceStand::mask =
  {{
    0,0,0,0,0,0,0,0,
    (1<<2)-1, (1<<3)-1, (1<<5)-1, (1<<3)-1, (1<<3)-1, (1<<3)-1, (1<<2)-1, (1<<2)-1
  }};
  
  const unsigned int PieceStand::carryMask;
}

osl::PieceStand::
PieceStand(Player pl, const SimpleState& state)
  : flags(0)
{
  for (Ptype ptype: PieceStand::order)
    add(ptype, state.countPiecesOnStand(pl, ptype));
}

bool osl::PieceStand::canAdd(Ptype type) const
{
  const int max 
    = Ptype_Table.getIndexLimit(type) - Ptype_Table.getIndexMin(type);
  assert(max >= 0);
  return (static_cast<int>(get(type)) != max);
}

void osl::PieceStand::tryAdd(Ptype type)
{
  if (canAdd(type))
    add(type);
}

bool osl::PieceStand::atMostOneKind() const
{
  return misc::BitOp::countBit(getFlags()) <= 1;
}

#ifndef MINIMAL
bool osl::PieceStand::
carryUnchangedAfterAdd(const PieceStand& original, const PieceStand& other) const
{
  if (original.testCarries() == testCarries())
    return true;
  std::cerr << original << " + " << other << " = " << *this << "\n";
  return false;
}

bool osl::PieceStand::
carryUnchangedAfterSub(const PieceStand& original, const PieceStand& other) const
{
  if (original.testCarries() == testCarries())
    return true;
  std::cerr << original << " - " << other << " = " << *this << "\n";
  return false;
}

std::ostream& osl::operator<<(std::ostream& os, osl::PieceStand stand)
{
  os << "(stand";
  for (Ptype ptype: PieceStand::order)
  {
    os << ' ' << stand.get(ptype);
  }
  return os << ")";
}
#endif

std::ostream& osl::
PieceStandIO::writeNumbers(std::ostream& os, const PieceStand& stand)
{
  for (Ptype ptype: PieceStand::order) {
    os << stand.get(ptype) << " ";
  }
  return os;
}
std::istream& osl::
PieceStandIO::readNumbers(std::istream& is, PieceStand& stand)
{
  stand  = PieceStand();
  for (Ptype ptype: PieceStand::order) {
    int val;
    if (is >> val) 
      stand.add(ptype, val);
  }
  return is;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
