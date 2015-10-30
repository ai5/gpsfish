/* mobilityTable.cc
 */
#include "osl/mobility/mobilityTable.h"
#include "osl/bits/ptypeTable.h"
#include "osl/bits/boardTable.h"
#include <iostream>

osl::mobility::MobilityTable::MobilityTable(osl::SimpleState const& state)
{
  for(int num=32;num<=39;num++){
    osl::Piece p=state.pieceOf(num);
    if(!p.isOnBoard()) continue;
    int moveMask=Ptype_Table.getMoveMask(p.ptype());
    for(int i=0;i<8;i++){
      Direction d=static_cast<Direction>(i);
      if(p.owner()==WHITE) d=inverse(d);
      d=shortToLong(d);
      if((moveMask&dirToMask(d))==0) continue;
      Offset o=Board_Table.getOffsetForBlack(static_cast<Direction>(i));
      Square pos=p.square()+o;
      for(;state.pieceAt(pos).isEmpty();pos+=o) ;
      if(state.pieceAt(pos)==Piece::EDGE()) pos-=o;
      this->set(static_cast<Direction>(i),num,pos);
    }
  }
}

std::ostream& osl::mobility::operator<<(std::ostream& os,osl::mobility::MobilityContent const& mc)
{
  os << "[";
  for(int i=0;i<7;i++) os << mc.get(static_cast<Direction>(i)) << ",";
  return os <<  mc.get(static_cast<Direction>(7)) << "]";
}

std::ostream& osl::mobility::operator<<(std::ostream& os,osl::mobility::MobilityTable const& mt)
{
  os << "MobilityTable(\n";
  for(int num=32;num<=39;num++){
    os << "num=" << num << ",[";
    for(int i=0;i<8;i++){
      Direction d=static_cast<Direction>(i);
      os << " " << mt.get(d,num);
    }
    os << "]\n";
  }
  return os << ")" << std::endl;
}

bool osl::mobility::operator==(MobilityTable const& mt1, MobilityTable const& mt2)
{
  for(int num=32;num<=39;num++){
    for(int i=0;i<8;i++){
      Direction d=static_cast<Direction>(i);
      if(mt1.get(d,num)!=mt2.get(d,num) ) return false;
    }
  }
  return true;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
