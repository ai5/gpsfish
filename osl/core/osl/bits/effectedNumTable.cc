/* effectedNumTable.cc
 */
#include "osl/bits/effectedNumTable.h"
#include "osl/bits/ptypeTable.h"
#include "osl/bits/boardTable.h"
#include <iostream>

void
osl::effect::EffectedNumTable::clear()
{
  for(int i=0;i<40;i++) contents[i].clear();
}

osl::effect::EffectedNumTable::EffectedNumTable(SimpleState const& state)
{
  clear();
  for(int num=32;num<=39;num++){
    osl::Piece p=state.pieceOf(num);
    if(!p.isOnBoard()) continue;
    int moveMask=Ptype_Table.getMoveMask(p.ptype());
    for(int i=0;i<8;i++){
      Direction d=static_cast<Direction>(i);
      if(p.owner()==WHITE) d=inverse(d);
      Direction longD=shortToLong(d);
      if((moveMask&dirToMask(longD))==0) continue;
      Offset o=Board_Table.getOffsetForBlack(static_cast<Direction>(i));
      Square pos=p.square()+o;
      Piece p1;
      for(;(p1=state.pieceAt(pos)).isEmpty();pos+=o) ;
      if(pos.isEdge()) continue;
      int num1=p1.number();
      contents[num1][static_cast<Direction>(i)]=num1;
    }
  }
}

std::ostream& osl::effect::operator<<(std::ostream& os,osl::EffectedNumTable const& et)
{
  os << "[\n";
  for(int num=0;num<=39;num++){
    os << " [";
    for(int d=0;d<7;d++) os << et[num][static_cast<Direction>(d)] << ",";
    os << et[num][static_cast<Direction>(7)] << "],\n";
  }
  return os <<  "]\n";
}
bool osl::effect::operator==(EffectedNumTable const& e1, EffectedNumTable const& e2)
{
  for(int i=0;i<8;i++){
    for(int num=0;num<=39;num++){
      if(e1[num][static_cast<Direction>(i)]!=e2[num][static_cast<Direction>(i)]) return false;
    }
  }
  return true;
}
