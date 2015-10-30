/* ptypeTable.cc
 */
#include "osl/bits/ptypeTable.h"
#include "osl/bits/boardTable.h"

osl::PtypeTable::PtypeTable()
{
  init();
  
  {
    // guard
    assert(&effect(newPtypeO(WHITE,ROOK), Offset32(2,8))
	   == &getEffect(newPtypeO(WHITE,ROOK), Offset32(2,8)));
    assert(getEffect(newPtypeO(BLACK,PPAWN),
		     Offset32(Square(7,1),Square(8,1)))
	   == EffectContent::DIRECT());
  }
}

template<osl::Ptype T>
void osl::PtypeTable::initPtypeSub(Int2Type<true>)
{
  initPtypeSub<T>(Int2Type<false>());
  numMaskLows[static_cast<int>(T)]=mask_t::makeDirect(PtypeFuns<T>::indexMask);
  numIndices[static_cast<int>(T)]=PtypeFuns<T>::indexNum;
  if(canPromote(T)){
    numMaskLows[static_cast<int>(promote(T))]=mask_t::makeDirect(PtypeFuns<T>::indexMask);
    numIndices[static_cast<int>(promote(T))]=PtypeFuns<T>::indexNum;
  }
  /** 黒 */
  canDropLimit[0][static_cast<int>(T)]=PtypeTraits<T>::dropBlackFromY;
  /** 白 */
  canDropLimit[1][static_cast<int>(T)]=Square::reverseY(PtypeTraits<T>::dropBlackFromY);
  indexMins[static_cast<int>(T)]=PtypeTraits<T>::indexMin;
  indexLimits[static_cast<int>(T)]=PtypeTraits<T>::indexLimit;
}

template<osl::Ptype T>
void osl::PtypeTable::initPtypeSub(Int2Type<false> /*is_basic*/)
{
  names[static_cast<int>(T)]=PtypeTraits<T>::name();
  csaNames[static_cast<int>(T)]=PtypeTraits<T>::csaName();
  moveMasks[static_cast<int>(T)]=PtypeTraits<T>::moveMask;
  betterToPromote[static_cast<int>(T)]=PtypeTraits<T>::betterToPromote;
}

template<osl::Ptype T>
void osl::PtypeTable::initPtype()
{
  initPtypeSub<T>(Int2Type<PtypeTraits<T>::isBasic>());
}

void osl::PtypeTable::init()
{
  numMaskLows.fill();
  numIndices.fill();

  initPtype<PTYPE_EMPTY>(); 
  initPtype<PTYPE_EDGE>(); 
  initPtype<PPAWN>(); 
  initPtype<PLANCE>(); 
  initPtype<PKNIGHT>(); 
  initPtype<PSILVER>(); 
  initPtype<PBISHOP>(); 
  initPtype<PROOK>(); 
  initPtype<GOLD>(); 
  initPtype<KING>(); 
  initPtype<PAWN>(); 
  initPtype<LANCE>(); 
  initPtype<KNIGHT>(); 
  initPtype<SILVER>(); 
  initPtype<BISHOP>(); 
  initPtype<ROOK>(); 
  effectTable.fill();
  effectTableNotLongU.fill();
  shortMoveMask.fill();
  static_assert(sizeof(EffectContent) == 4, "size");
  assert(&effect(newPtypeO(WHITE,ROOK), Offset32(2,8))
 	 == &getEffect(newPtypeO(WHITE,ROOK), Offset32(2,8)));
  assert(! getEffect(newPtypeO(BLACK,ROOK), Offset32(-1,8)).hasEffect());

  for(int ptype=PTYPE_MIN;ptype<=PTYPE_MAX;ptype++){
    for(int j=DIRECTION_MIN;j<=DIRECTION_MAX;j++){
      Direction dir=static_cast<Direction>(j);

      if((moveMasks[ptype]&(1<<dir))!=0){
	int dx=Board_Table.getDxForBlack(dir);
	int dy=Board_Table.getDyForBlack(dir);
	Offset32 offset32=Offset32(dx,dy);
	Offset offset=newOffset(dx,dy);
	if(isLong(dir)){
	  shortMoveMask[0][dir-10]|=1<<(ptype-PTYPEO_MIN);
	  shortMoveMask[1][dir-10]|=1<<(ptype-16-PTYPEO_MIN);

	  effectTable[ptype-PTYPEO_MIN][offset32.index()]=EffectContent::DIRECT(offset);
	  effectTable[ptype-16-PTYPEO_MIN][(-offset32).index()]=EffectContent::DIRECT(-offset);

	  for(int i=2;i<9;i++){
	    offset32=Offset32(dx*i,dy*i);
	    effectTable[ptype-PTYPEO_MIN][offset32.index()]=EffectContent(offset);
	    effectTable[ptype-16-PTYPEO_MIN][(-offset32).index()]=EffectContent(-offset);

	    if(static_cast<int>(dir)!=LONG_U){
	      effectTableNotLongU[ptype-PTYPEO_MIN][offset32.index()]=effectTable[ptype-PTYPEO_MIN][offset32.index()];
	      effectTableNotLongU[ptype-16-PTYPEO_MIN][(-offset32).index()]=effectTable[ptype-16-PTYPEO_MIN][(-offset32).index()];
	    }
	  }
	}
	else{
	  shortMoveMask[0][dir]|=1<<(ptype-PTYPEO_MIN);
	  shortMoveMask[1][dir]|=1<<(ptype-16-PTYPEO_MIN);
	  effectTable[ptype-PTYPEO_MIN][offset32.index()]=EffectContent::DIRECT();
	  effectTable[ptype-16-PTYPEO_MIN][(-offset32).index()]=EffectContent::DIRECT();

	  assert(! getEffect(newPtypeO(BLACK,ROOK),Offset32(-1,8)).hasEffect());
	}
      }
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
