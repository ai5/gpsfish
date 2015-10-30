#include "osl/hashKey.h"
#include "osl/random.h"
#include <iomanip>
#include <cstdlib>
#include <iostream>
#include <sstream>

static_assert(sizeof(osl::HashKey) == sizeof(int)*4, "hash key size");

void osl::hash::HashKey128::setRandom()
{
  board64 = misc::Random<unsigned long long>::newValue() & ~static_cast<uint64_t>(1);
  board32 = misc::Random<unsigned int>::newValue();
}

osl::hash::HashKey128::StandHash::StandHash()
{
  for (uint64_t& value: HashMajorPawn)
    value = misc::Random<unsigned long long>::newValue() & ~1ull;
  for (uint64_t& value: HashPiece)
    value = misc::Random<unsigned long long>::newValue() & ~1ull;
}

#ifndef MINIMAL
std::ostream& osl::hash::operator<<(std::ostream& os,const osl::hash::HashKey& h)
{
  os << h.pieceStand();
  const BoardKey& board_key = h.boardKey();
  for (size_t i=0; i<board_key.size(); ++i)
  {
    os << ':' 
       << std::setfill('0') << std::setbase(16) << std::setw(8)
       << board_key[i];
  }
  return os << ':' << std::setbase(10);
}

void osl::hash::HashKey::dumpContents(std::ostream& os) const
{
  os << pieceStand().getFlags();
  for (size_t i=0; i<size(); ++i) {
    os << ' ' << operator[](i);
  }
}

void osl::hash::HashKey::dumpContentsCerr() const
{
  dumpContents(std::cerr);
}
#endif

osl::hash::HashKey::HashKey(const SimpleState& state)
{
  for(int num=0;num<40;num++){
    Piece p=state.pieceOf(num);
    if(state.usedMask().test(num))
      HashGenTable::addHashKey(*this, p.square(),p.ptypeO());
  }
  setPlayer(state.turn());
}

const osl::hash::HashKey osl::hash::HashKey::
newHashWithMove(Move move) const
{
  return newMakeMove(move);
}

const osl::hash::HashKey osl::hash::HashKey::
newMakeMove(Move move) const
{
  HashKey ret(*this);
  if (! move.isPass())
  {
    assert(move.isValid());
    Square from=move.from();
    Square to=move.to();
    Ptype capturePtype=move.capturePtype();
    PtypeO ptypeO=move.ptypeO();
    PtypeO oldPtypeO=move.oldPtypeO();
    if (capturePtype!=PTYPE_EMPTY)
    {
      PtypeO capturePtypeO=newPtypeO(alt(move.player()),capturePtype);
      PtypeO capturedPtypeO=captured(capturePtypeO);

      HashGenTable::subHashKey(ret,to,capturePtypeO);
      HashGenTable::addHashKey(ret,Square::STAND(),capturedPtypeO);
    }
    HashGenTable::subHashKey(ret,from,oldPtypeO);
    HashGenTable::addHashKey(ret,to,ptypeO);
  }
  ret.changeTurn();
  return ret;
}

const osl::hash::HashKey osl::hash::HashKey::
newUnmakeMove(Move move) const
{
  HashKey ret(*this);
  if (! move.isPass())
  {
    assert(move.isValid());
    Square from=move.from();
    Square to=move.to();
    Ptype capturePtype=move.capturePtype();
    PtypeO ptypeO=move.ptypeO();
    PtypeO oldPtypeO=move.oldPtypeO();
    if (capturePtype!=PTYPE_EMPTY)
    {
      PtypeO capturePtypeO=newPtypeO(alt(move.player()),capturePtype);
      PtypeO capturedPtypeO=captured(capturePtypeO);

      HashGenTable::addHashKey(ret,to,capturePtypeO);
      HashGenTable::subHashKey(ret,Square::STAND(),capturedPtypeO);
    }
    HashGenTable::addHashKey(ret,from,oldPtypeO);
    HashGenTable::subHashKey(ret,to,ptypeO);
  }
  ret.changeTurn();
  return ret;
}

namespace osl
{
  const CArray2d<hash::HashKey128Layout,Square::SIZE,PTYPEO_SIZE>
  hash::HashGenTable::key = {
#include "bits/hash.txt"
    };
}
  
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
