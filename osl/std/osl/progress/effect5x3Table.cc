/* effect5x3Table.cc
 */

#include "osl/progress/effect5x3Table.h"
#include "osl/bits/centering5x3.h"
#include "osl/basic_type.h"
#include "osl/bits/ptypeTable.h"
#include "osl/bits/boardTable.h"
#include "osl/oslConfig.h"

#include <iostream>

osl::progress::Effect5x3Table osl::progress::Effect5x3_Table;
static osl::SetUpRegister _initializer([](){ osl::progress::Effect5x3_Table.init(); });

namespace osl
{
  namespace progress
  {
    namespace
    {
      /**
       * 駒の短い利きを計算.
       * @param ptypeO - 利きをつけようとする駒の種類
       * @param center_dx - centering済みの敵玉と駒とX座標の差
       * @param center_dy - centering済みの敵玉と駒とY座標の差
       */
      int countShort(PtypeO ptypeO,int center_dx, int center_dy){
	int ret=0;
	for(int dy=std::max(-8,center_dy-1);
	    dy<=std::min(8,center_dy+1);dy++)
	  for(int dx=std::max(-8,center_dx-2);
	      dx<=std::min(8,center_dx+2);dx++){
	    EffectContent ec=Ptype_Table.getEffect(ptypeO,Offset32(dx,dy));
	    // unBlockableではなくて，ほんとのshortが欲しい
	    if(ec == EffectContent::DIRECT()){
	      ret+=8;
	    }
	  }
	return ret;
      }
      /**
       * 5x3領域への長い利きの始まりと終わりを計算.
       * @param d - 利きをつけようとする方向
       * @param center_dx - centering済みの敵玉と駒とX座標の差
       * @param center_dy - centering済みの敵玉と駒とY座標の差
       */
      LongEffect calcLong(Direction d,int center_dx, int center_dy){
	LongEffect ret;
	ret.minIndex=0;
	ret.maxIndex=0;
	int dy_min=std::max(-8,center_dy-1);
	int dy_max=std::min(8,center_dy+1);
	int dx_min=std::max(-8,center_dx-2);
	int dx_max=std::min(8,center_dx+2);
	int dx=Board_Table.getDxForBlack(d);
	int dy=Board_Table.getDyForBlack(d);
	assert(dx!=0 || dy!=0);
	int i;
	
	for(i=1;i<9;i++){
	  if(dx_min<=dx*i && dx*i <=dx_max &&
	     dy_min<=dy*i && dy*i <=dy_max) break;
	}
	if(i==9) return ret;
	ret.minIndex=i;
	for(;i<9;i++){
	  if(!(dx_min<=dx*i && dx*i <=dx_max &&
	       dy_min<=dy*i && dy*i <=dy_max)) break;
	}
	ret.maxIndex=i-1;
	ret.offset=Offset(dx,dy);
	assert(!ret.offset.zero());
	return ret;
      }
      /**
       * 駒の長い利きを計算するためのLongEffect4の計算.
       * @param ptypeO - 利きをつけようとする駒の種類
       * @param center_dx - centering済みの敵玉と駒とX座標の差
       * @param center_dy - centering済みの敵玉と駒とY座標の差
       */
      LongEffect4 calcLong4(PtypeO ptypeO,int center_dx, int center_dy){
	LongEffect4 ret;
	int index=0;
	
	Player pl=getOwner(ptypeO);
	for(int i=0;i<8;i++){
	  Direction d=static_cast<Direction>(i);
	  /**
	   * 白の場合は方向を反転
	   */
	  Offset32 o32(Board_Table.getDxForBlack(d)*sign(pl),
		       Board_Table.getDyForBlack(d)*sign(pl));
	  Direction longD=Board_Table.getLongDirection<BLACK>(o32);
	  if((Ptype_Table.getMoveMask(getPtype(ptypeO))&
	      dirToMask(shortToLong(d)))!=0){
	    assert(index<4);
	    ret[index]=calcLong(longD,center_dx,center_dy);
	    if(!ret[index].offset.zero())index++;
	  }
	}
	return ret;
      }
    }
#ifndef MINIMAL
    std::ostream& operator<<(std::ostream& os,LongEffect const& longEffect){
      return os << "(" << longEffect.offset << "," << longEffect.minIndex 
		<< "," << longEffect.maxIndex << ")";
    }
#endif
  }
}

void osl::progress::Effect5x3Table::setupOnStand()
{
  onStand[PAWN]  =StandPAWN;
  onStand[LANCE] =StandLANCE;
  onStand[KNIGHT]=StandKNIGHT;
  onStand[SILVER]=StandSILVER;
  onStand[GOLD]  =StandGOLD;
  onStand[BISHOP]=StandBISHOP;
  onStand[ROOK]  =StandROOK;
  for(int i=PTYPE_PIECE_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    if(unpromote(ptype)!=ptype)
      onStand[ptype]=onStand[unpromote(ptype)];
  }
}


void osl::progress::Effect5x3Table::setupShortEffect()
{
  Player pl=BLACK;
  for(int i=0;i<2;i++,pl=alt(pl)){
    for(int j=PTYPE_PIECE_MIN;j<=PTYPE_MAX;j++){
      PtypeO ptypeO=newPtypeO(pl,static_cast<Ptype>(j));
      for(int dy= -8;dy<=8;dy++)
	for(int dx= -8;dx<=8;dx++){
	  Offset32 o32(dx,dy);
	  shortEffect[ptypeOIndex(ptypeO)][o32.index()]=
	    countShort(ptypeO,dx,dy);
	}
    }
  }
}
void osl::progress::Effect5x3Table::setupBlockEffect()
{
  Player pl=BLACK;
  for(int i=0;i<2;i++,pl=alt(pl)){
    for(int j=0;j<8;j++){
      Direction d=static_cast<Direction>(j);
      for(int dy= -8;dy<=8;dy++)
	for(int dx= -8;dx<=8;dx++){
	  Offset32 o32(dx,dy);
	  blockEffect[j][o32.index()]=
	    calcLong(d,dx,dy);
	}
    }
  }
}
void osl::progress::Effect5x3Table::setupLongEffect()
{
  Player pl=BLACK;
  for(int i=0;i<2;i++,pl=alt(pl)){
    for(int j=PTYPE_PIECE_MIN;j<=PTYPE_MAX;j++){
      PtypeO ptypeO=newPtypeO(pl,static_cast<Ptype>(j));
      for(int dy= -8;dy<=8;dy++)
	for(int dx= -8;dx<=8;dx++){
	  Offset32 o32(dx,dy);
	  longEffect[ptypeOIndex(ptypeO)][o32.index()]=
	    calcLong4(ptypeO,dx,dy);
	}
    }
  }
}

void osl::progress::Effect5x3Table::setupAttackEffect()
{
  // 黒の攻める際のテーブルを作成
  // 玉は白
  for(int x=1;x<=9;x++)
    for(int y=1;y<=9;y++){
      const Square king(x,y);
      const Square center = Centering5x3::adjustCenter(king);
      for(int dx=0;dx<5;dx++){
	for(int dy=0;dy<3;dy++){
	  const Square p(center.x()+dx-2,center.y()+dy-1);
	  int val=16-std::abs(p.x()-king.x());
	  attackEffect[BLACK][king.index()][dx*3+dy]=val;
	}
      }
    }
  for(int x=1;x<=9;x++)
    for(int y=1;y<=9;y++){
      const Square king(x,y);
      const Square r_king=king.rotate180();
      for(int dx=0;dx<5;dx++){
	for(int dy=0;dy<3;dy++){
	  int r_dx=4-dx,r_dy=2-dy;
	  attackEffect[WHITE][king.index()][dx*3+dy]=
	    attackEffect[BLACK][r_king.index()][r_dx*3+r_dy];
	}
      }
    }
}
void osl::progress::Effect5x3Table::setupDefenseEffect()
{
  // 白が守る際のテーブルを作成
  // 玉は白
  for(int x=1;x<=9;x++)
    for(int y=1;y<=9;y++){
      const Square king(x,y);
      const Square center = Centering5x3::adjustCenter(king);
      for(int dx=0;dx<5;dx++){
	for(int dy=0;dy<3;dy++){
	  const Square p(center.x()+dx-2,center.y()+dy-1);
	  int val=16-std::abs(p.x()-king.x());
	  defenseEffect[WHITE][king.index()][dx*3+dy]=val;
	}
      }
    }
  for(int x=1;x<=9;x++)
    for(int y=1;y<=9;y++){
      const Square king(x,y);
      const Square r_king=king.rotate180();
      for(int dx=0;dx<5;dx++){
	for(int dy=0;dy<3;dy++){
	  int r_dx=4-dx,r_dy=2-dy;
	  defenseEffect[BLACK][king.index()][dx*3+dy]=
	    defenseEffect[WHITE][r_king.index()][r_dx*3+r_dy];
	}
      }
    }
}

void osl::progress::Effect5x3Table::init()
{
  setupOnStand();
  setupShortEffect();
  setupBlockEffect();
  setupLongEffect();
  setupAttackEffect();
  setupDefenseEffect();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
