#ifndef OSL_NUM_SIMPLE_EFFECT_TCC
#define OSL_NUM_SIMPLE_EFFECT_TCC

#include "osl/bits/numSimpleEffect.h"
#include "osl/bits/king8Info.h"
#include "osl/bits/pieceTable.h"
#include "osl/csa.h"
#include <iostream>

template<osl::effect::NumBitmapEffect::Op OP,bool UC>
void  osl::effect::
NumSimpleEffectTable::doEffect(const SimpleState& state,PtypeO ptypeo,Square pos,int num)
{
  switch((int)ptypeo){
  case NEW_PTYPEO(WHITE,PAWN): doEffectBy<WHITE,PAWN,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,LANCE): doEffectBy<WHITE,LANCE,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,KNIGHT): doEffectBy<WHITE,KNIGHT,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,SILVER): doEffectBy<WHITE,SILVER,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,PPAWN):
  case NEW_PTYPEO(WHITE,PLANCE):
  case NEW_PTYPEO(WHITE,PKNIGHT):
  case NEW_PTYPEO(WHITE,PSILVER):
  case NEW_PTYPEO(WHITE,GOLD): doEffectBy<WHITE,GOLD,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,BISHOP): doEffectBy<WHITE,BISHOP,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,PBISHOP): doEffectBy<WHITE,PBISHOP,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,ROOK): doEffectBy<WHITE,ROOK,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,PROOK): doEffectBy<WHITE,PROOK,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(WHITE,KING): doEffectBy<WHITE,KING,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,PAWN): doEffectBy<BLACK,PAWN,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,LANCE): doEffectBy<BLACK,LANCE,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,KNIGHT): doEffectBy<BLACK,KNIGHT,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,SILVER): doEffectBy<BLACK,SILVER,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,PPAWN):
  case NEW_PTYPEO(BLACK,PLANCE):
  case NEW_PTYPEO(BLACK,PKNIGHT):
  case NEW_PTYPEO(BLACK,PSILVER):
  case NEW_PTYPEO(BLACK,GOLD): doEffectBy<BLACK,GOLD,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,BISHOP): doEffectBy<BLACK,BISHOP,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,PBISHOP): doEffectBy<BLACK,PBISHOP,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,ROOK): doEffectBy<BLACK,ROOK,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,PROOK): doEffectBy<BLACK,PROOK,OP,UC>(state,pos,num); break;
  case NEW_PTYPEO(BLACK,KING): doEffectBy<BLACK,KING,OP,UC>(state,pos,num); break;
  default: assert(0);
  }
  return;
}

template<osl::Player P, osl::Ptype T, osl::effect::NumBitmapEffect::Op OP,bool UC>
void  osl::effect::
NumSimpleEffectTable::doEffectBy(const SimpleState& state,Square pos,int num)
{
  if(UC){
    if(T==LANCE || T==BISHOP || T==PBISHOP || T==ROOK || T==PROOK)
      setChangedPieces(NumBitmapEffect::makeLongEffect<P>(num));
    else
      setChangedPieces(NumBitmapEffect::makeEffect<P>(num));
  }
  doEffectShort<P,T,UL,OP,UC>(state,pos,num);
  doEffectShort<P,T,U,OP,UC>(state,pos,num);
  doEffectShort<P,T,UR,OP,UC>(state,pos,num);
  doEffectShort<P,T,L,OP,UC>(state,pos,num);
  doEffectShort<P,T,R,OP,UC>(state,pos,num);
  doEffectShort<P,T,DL,OP,UC>(state,pos,num);
  doEffectShort<P,T,D,OP,UC>(state,pos,num);
  doEffectShort<P,T,DR,OP,UC>(state,pos,num);
  doEffectShort<P,T,UUL,OP,UC>(state,pos,num);
  doEffectShort<P,T,UUR,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_UL,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_U,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_UR,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_L,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_R,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_DL,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_D,OP,UC>(state,pos,num);
  doEffectLong<P,T,LONG_DR,OP,UC>(state,pos,num);
}

#endif /* OSL_NUM_SIMPLE_EFFECT_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
