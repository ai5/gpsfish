#include "osl/move_generator/addEffect8Table.h"
#include "osl/oslConfig.h"
#include <cstdlib>
osl::move_generator::addeffect8::AddEffect8Table osl::move_generator::Add_Effect8_Table;
static osl::SetUpRegister _initializer([](){ 
  osl::move_generator::Add_Effect8_Table.init();
});

namespace osl
{
  namespace move_generator
  {
namespace addeffect8{
  bool
#ifdef __GNUC__
  __attribute__ ((const))
#endif
  sameDirection(int dx0, int dy0, int dx1, int dy1)
  {
    return dx0*dy1==dx1*dy0;
  }
  /**
   * targetから(+dx,+dy)の位置にある黒の種類ptypeの駒がdirectな利きを持つか.
   */
  bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
  hasUnblockableEffect(Ptype ptype,int dx,int dy)
  {
    if(std::abs(dx)>8 || std::abs(dy)>8) return false;
    const EffectContent effect
      =Ptype_Table.getEffect(newPtypeO(BLACK,ptype),Offset32(dx,dy));
    return effect.hasUnblockableEffect();
  }
  bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
  hasShortEffect(Ptype ptype,int dx,int dy)
  {
    if(std::abs(dx)>8 || std::abs(dy)>8) return false;
    const EffectContent effect
      =Ptype_Table.getEffect(newPtypeO(BLACK,ptype),Offset32(dx,dy));
    return effect.hasEffect() && effect.offset().zero();
  }
  bool
#ifdef __GNUC__
  __attribute__ ((pure))
#endif
  hasEffect(Ptype ptype,int dx,int dy)
  {
    if(std::abs(dx)>8 || std::abs(dy)>8) return false;
    return Ptype_Table.getEffect(newPtypeO(BLACK,ptype),Offset32(dx,dy)).hasEffect();
  }
}
  } // move_generator
}

void
osl::move_generator::addeffect8::AddEffect8Table::initDropSquare()
{
  for(int i=PTYPE_BASIC_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    if(ptype==KING) continue;
    if(Ptype_Table.hasLongMove(ptype)) continue;
    for(int x=1;x<=9;x++)
      for(int y=1;y<=9;y++){
	Square pos(x,y);
	int index=0;
	for(int x1=1;x1<=9;x1++)
	  for(int y1=1;y1<=9;y1++){
	    Square pos1(x1,y1);
	    if(pos==pos1)continue;
	    // 直接利きがある時は除く
	    if(hasUnblockableEffect(ptype,x-x1,y-y1)) continue;
	    for(int dx0=-1;dx0<=1;dx0++)
	      for(int dy0=-1;dy0<=1;dy0++){
		int x2=x+dx0,y2=y+dy0;
		Square pos2(x2,y2);
		if(!pos2.isOnBoard()) continue;
		if(hasUnblockableEffect(ptype,x2-x1,y2-y1))
		  goto found;
	      }
	    continue;
	  found:
	    dropSquare[ptype][pos.index()][index++]=pos1;
	  }
      }
  }
}

void
osl::move_generator::addeffect8::AddEffect8Table::initLongDropSquare()
{
  for(int i=PTYPE_BASIC_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    if(!Ptype_Table.hasLongMove(ptype)) continue;
    assert(ptype==ROOK || ptype==BISHOP || ptype==LANCE);
    for(int x=1;x<=9;x++)
      for(int y=1;y<=9;y++){
	const Square pos(x,y);
	int sIndex=0,dIndex=0,index1=0,index2=0;
	for(int x1=1;x1<=9;x1++)
	  for(int y1=1;y1<=9;y1++){
	    Square pos1(x1,y1);
	    if(pos==pos1)continue;
	    // 直接利きがある時はdirectにチャレンジ
	    if(hasEffect(ptype,x-x1,y-y1)){
	      if(!hasUnblockableEffect(ptype,x-x1,y-y1)) continue;
	      Square pos2(x+(x1-x)*2,y+(y1-y)*2);
	      if(!pos2.isOnBoard()) continue;
	      longDropDirect[ptype][pos.index()][dIndex++]=Offset(x1-x,y1-y);
	      continue;
	    }
	    int count=0;
	    CArray<int,2> dxs, dys;
	    for(int dx0=-1;dx0<=1;dx0++)
	      for(int dy0=-1;dy0<=1;dy0++){
		int x2=x+dx0,y2=y+dy0;
		Square pos2(x2,y2);
		if(pos2==pos) continue;
		if(!pos2.isOnBoard()) continue;
		if(hasUnblockableEffect(ptype,x2-x1,y2-y1)){
		  dxs[count]=x1-x2;
		  dys[count++]=y1-y2;
		}
	      }
	    if(count>0){
	      if(abs(x-x1)<=1 && abs(y-y1)<=1)
		dropSquare[ptype][pos.index()][sIndex++]=pos1;
	      else if(count==1){
		longDropSquare[ptype][pos.index()][index1++]=
		  PO(pos1,Offset(dxs[0],dys[0]));
	      }
	      else if(count==2){
		longDrop2Square[ptype][pos.index()][index2++]=
		  POO(pos1,OffsetPair(Offset(dxs[0],dys[0]),
				      Offset(dxs[1],dys[1])));
	      }
	    }
	  }
      }
  }
}

void
osl::move_generator::addeffect8::AddEffect8Table::initMoveOffset()
{
  for(int i=PTYPE_PIECE_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    for(int dx=-8;dx<=8;dx++)
      for(int dy=-8;dy<=8;dy++){
	if(dx==0 && dy==0) continue;
	Offset32 o32(dx,dy); // targetから見た駒の位置
	// 最初から王手になっている場合は除く
	if(hasUnblockableEffect(ptype,-dx,-dy)) continue;
	// 最初から8近傍にあたる長い利きを持つ場合
	for(int dx1=-1;dx1<=1;dx1++){
	  for(int dy1=-1;dy1<=1;dy1++){
	    if(dx1==0 && dy1==0) continue;
	    if(hasEffect(ptype,dx1-dx,dy1-dy) &&
	       !hasUnblockableEffect(ptype,dx1-dx,dy1-dy)){
	      int div=std::max(std::abs(dx1-dx),std::abs(dy1-dy));
	      // 8近傍の端の場合しか興味がない．
	      if(abs(dx1+(dx-dx1)/div)>1 ||
		 abs(dy1+(dy-dy1)/div)>1){
		betweenOffset[ptype][o32.index()]=
		  OffsetPair(Offset(dx1,dy1),
			     Offset((dx1-dx)/div,(dy1-dy)/div));
	      }
	    }
	  }
	}
	int sIndex=0,lIndex=0,spIndex=0;
	for(int dx0=-8;dx0<=8;dx0++)
	  for(int dy0=-8;dy0<=8;dy0++){
	    if(dx0==0 && dy0==0) continue;
	    // 移動できない場合は問題外
	    if(!hasEffect(ptype,dx0-dx,dy0-dy)) continue;
	    int effectDx=9,effectDy=9;
	    bool unblockableEffect=false;
	    bool effect=false;
	    bool promotedUnblockableEffect=false;
	    for(int dx1=-1;dx1<=1;dx1++){
	      for(int dy1=-1;dy1<=1;dy1++){
		if(dx1==0 && dy1==0) continue;
		if(hasUnblockableEffect(ptype,dx1-dx0,dy1-dy0)){
		  // 元々利きがあったなら数えない
		  if(!hasUnblockableEffect(ptype,dx1-dx,dy1-dy) &&
		     (!hasEffect(ptype,dx1-dx,dy1-dy) ||
		      !sameDirection(dx1-dx0,dy1-dy0,dx1-dx,dy1-dy) ||
		      (abs(dx0)<=1 && abs(dy0)<=1)
		      )){
		    unblockableEffect=true;
		    effect=true;
		  }
		}
		else if(hasEffect(ptype,dx1-dx0,dy1-dy0) &&
			!hasEffect(ptype,dx1-dx,dy1-dy)){
		  if(std::abs(effectDx)>=std::abs(dx1-dx0) &&
		     std::abs(effectDy)>=std::abs(dy1-dy0)){
		    effectDx=dx1-dx0; effectDy=dy1-dy0;
		    effect=true;
		  }
		  else{
		    effect=true;
		  }
		}
		if(canPromote(ptype) && 
		   hasUnblockableEffect(osl::promote(ptype),dx1-dx0,dy1-dy0) &&
		   !hasUnblockableEffect(ptype,dx1-dx,dy1-dy)
		   ){
		  promotedUnblockableEffect=true;
		}
	      }
	    }
	    if(unblockableEffect 
	       //	       && !hasUnblockableEffect(ptype,-dx0,-dy0)
	       ){
	      shortMoveOffset[ptype][o32.index()][sIndex++]=Offset(dx0,dy0);
	    }
	    else if(effect){
	      // 同じ方向の時は，betweenにする
	      longMoveOffset[ptype][o32.index()][lIndex++]=
		OffsetPair(Offset(dx0,dy0),Offset(dx0+effectDx,dy0+effectDy));
	    }
	    if(promotedUnblockableEffect 
	       //	       &&!hasUnblockableEffect(promote(ptype),-dx0,-dy0)
	       ){
	      shortPromoteMoveOffset[ptype][o32.index()][spIndex++]=
		Offset(dx0,dy0);
	    }
	  }
      }
  }
}
void osl::move_generator::addeffect8::AddEffect8Table::init()
{
  initDropSquare();
  initLongDropSquare();
  initMoveOffset();
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

