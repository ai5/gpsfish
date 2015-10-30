/* immediateCheckmateTable.cc
 */
#include "osl/checkmate/immediateCheckmateTable.h"
#include "osl/bits/boardTable.h"
#include "osl/bits/ptypeTable.h"
namespace
{
  bool canCheckmate(osl::Ptype ptype,osl::Direction dir,unsigned int mask)
  {
    // 王はdropできない, 打ち歩詰め
    if(ptype==osl::KING || ptype==osl::PAWN) return false;
    // ptypeがdir方向に利きを持たない == 王手をかけられない
    if(!(osl::Ptype_Table.getMoveMask(ptype)&
	 (osl::dirToMask(dir) | osl::dirToMask(osl::shortToLong(dir))))) return false;
    int dx=osl::Board_Table.getDxForBlack(dir);
    int dy=osl::Board_Table.getDyForBlack(dir);
    for(int l=0;l<8;l++){
      if((mask&(1<<l))==0) continue;
      osl::Direction dir1=static_cast<osl::Direction>(l);
      int dx1=osl::Board_Table.getDxForBlack(dir1);
      int dy1=osl::Board_Table.getDyForBlack(dir1);
      osl::Offset32 o32(dx-dx1,dy-dy1);
      if(!osl::Ptype_Table.getEffect(osl::newPtypeO(osl::BLACK,ptype),o32).hasEffect())
	return false;
    }
    return true;
  }
}

osl::checkmate::ImmediateCheckmateTable::ImmediateCheckmateTable()
{
  // ptypeDropMaskの初期化
  for(int i=0;i<0x100;i++){
    for(int k=PTYPE_BASIC_MIN;k<=PTYPE_MAX;k++){
      unsigned char mask=0;
      Ptype ptype=static_cast<Ptype>(k);
      for(int j=0;j<8;j++){
	// 玉の逃げ道がある
	if((i&(0x1<<j))!=0)continue;
	Direction dir=static_cast<Direction>(j);
	if(canCheckmate(ptype,dir,i))
	  mask|=(1<<j);
      }
      ptypeDropMasks[i][ptype]=mask;
    }
  }
  // dropPtypeMaskの初期化
  for(int i=0;i<0x10000;i++){
    unsigned char ptypeMask=0;
    for(int k=PTYPE_BASIC_MIN;k<=PTYPE_MAX;k++){
      Ptype ptype=static_cast<Ptype>(k);
      for(int j=0;j<8;j++){
	// 空白でない
	if((i&(0x1<<j))==0) continue;
	// 玉の逃げ道がある
	if((i&(0x100<<j))!=0)continue;
	Direction dir=static_cast<Direction>(j);
	if(canCheckmate(ptype,dir,(i>>8)&0xff)){
	  ptypeMask|=1u<<(k-PTYPE_BASIC_MIN);
	  goto nextPtype;
	}
      }
    nextPtype:;
    }
    dropPtypeMasks[i]=ptypeMask;
  }
  // blockingMaskの初期化
  for(int k=PTYPE_BASIC_MIN;k<=PTYPE_MAX;k++){
    Ptype ptype=static_cast<Ptype>(k);
    for(int j=0;j<8;j++){
      unsigned int mask=0;
      Direction dir=static_cast<Direction>(j);
      if(Ptype_Table.getMoveMask(ptype)&
	 (dirToMask(dir) | dirToMask(shortToLong(dir)))){
	int dx=Board_Table.getDxForBlack(dir);
	int dy=Board_Table.getDyForBlack(dir);
	for(int l=0;l<8;l++){
	  Direction dir1=static_cast<Direction>(l);
	  int dx1=Board_Table.getDxForBlack(dir1);
	  int dy1=Board_Table.getDyForBlack(dir1);
	  Offset32 o32(dx-dx1,dy-dy1);
	  if(!Ptype_Table.getEffect(newPtypeO(BLACK,ptype),o32).hasEffect()){
	    if(!Board_Table.getShortOffsetNotKnight(o32).zero() &&
	       !(dx==-dx1 && dy==-dy1)
	       ){
	      mask|=1<<l;
	    }
	  }
	}
      }
      blockingMasks[ptype][dir]=mask;
    }
  }
  // effectMaskの初期化
  for(int k=PTYPE_PIECE_MIN;k<=PTYPE_MAX;k++){
    Ptype ptype=static_cast<Ptype>(k);
    for(int j=0;j<8;j++){
      unsigned int mask=0x1ff;
      Direction dir=static_cast<Direction>(j);
      if(Ptype_Table.getMoveMask(ptype)&
	 (dirToMask(dir) | dirToMask(shortToLong(dir)))){ // 王手をかけられる
	mask=0;
	int dx=Board_Table.getDxForBlack(dir);
	int dy=Board_Table.getDyForBlack(dir);
	for(int l=0;l<8;l++){
	  Direction dir1=static_cast<Direction>(l);
	  int dx1=Board_Table.getDxForBlack(dir1);
	  int dy1=Board_Table.getDyForBlack(dir1);
	  Offset32 o32(dx-dx1,dy-dy1);
	  if(dir!= dir1 &&
	     !Ptype_Table.getEffect(newPtypeO(BLACK,ptype),o32).hasEffect()){
	    mask|=1<<l;
	  }
	}
      }
      noEffectMasks[ptype][dir]=mask;
    }
  }
}



/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


