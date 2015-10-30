/* pieceTable.cc
 */
#include "osl/bits/pieceTable.h"
#include "osl/bits/ptypeTraits.h"

template<osl::Ptype T>
void osl::PieceTable::initPtype()
{
  for (int num=PtypeTraits<T>::indexMin; num<PtypeTraits<T>::indexLimit; num++)
  {
    ptypes[num]=T;
  }
}

osl::PieceTable::PieceTable() 
{
  initPtype<PAWN>();
  initPtype<LANCE>();
  initPtype<KNIGHT>();
  initPtype<SILVER>();
  initPtype<GOLD>();
  initPtype<KING>();
  initPtype<BISHOP>();
  initPtype<ROOK>();
}

