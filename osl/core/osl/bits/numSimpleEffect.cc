/* numSimpleEffect.cc
 */
#include "osl/bits/numSimpleEffect.h"
#if (defined(__i386__) || defined(__x86_64__)) && !defined(OSL_NO_SSE)
#include <emmintrin.h>
typedef __v2di v2di;
#endif
#include <iostream>

void osl::effect::
NumSimpleEffectTable::init(const SimpleState& state)
{
  std::fill(effects.begin(), effects.end(),NumBitmapEffect());
  for(int num=0;num<40;num++){
    if (state.isOnBoard(num)){
      Piece p=state.pieceOf(num);
      doEffect<NumBitmapEffect::Add,true>(state,p);
    }
  }
}

void osl::effect::
NumSimpleEffectTable::copyFrom(const NumSimpleEffectTable& src)
{
  this->effected_mask = src.effected_mask;
  this->mobilityTable = src.mobilityTable;
  this->changed_effects = src.changed_effects;
  this->changed_effect_pieces = src.changed_effect_pieces;
  this->effected_changed_mask = src.effected_changed_mask;

#if (defined(__i386__) || defined(__x86_64__)) && !defined(OSL_NO_SSE)
  {  
    v2di en0=*((v2di*)&src.effectedNumTable[0]);
    v2di en2=*((v2di*)&src.effectedNumTable[2]);
    v2di en4=*((v2di*)&src.effectedNumTable[4]);
    v2di en6=*((v2di*)&src.effectedNumTable[6]);
    v2di en8=*((v2di*)&src.effectedNumTable[8]);
    v2di en10=*((v2di*)&src.effectedNumTable[10]);
    v2di en12=*((v2di*)&src.effectedNumTable[12]);
    v2di en14=*((v2di*)&src.effectedNumTable[14]);
    v2di en16=*((v2di*)&src.effectedNumTable[16]);
    v2di en18=*((v2di*)&src.effectedNumTable[18]);

    *((v2di*)&(*this).effectedNumTable[0])=en0;
    *((v2di*)&(*this).effectedNumTable[2])=en2;
    *((v2di*)&(*this).effectedNumTable[4])=en4;
    *((v2di*)&(*this).effectedNumTable[6])=en6;
    *((v2di*)&(*this).effectedNumTable[8])=en8;
    *((v2di*)&(*this).effectedNumTable[10])=en10;
    *((v2di*)&(*this).effectedNumTable[12])=en12;
    *((v2di*)&(*this).effectedNumTable[14])=en14;
    *((v2di*)&(*this).effectedNumTable[16])=en16;
    *((v2di*)&(*this).effectedNumTable[18])=en18;

    v2di en20=*((v2di*)&src.effectedNumTable[20]);
    v2di en22=*((v2di*)&src.effectedNumTable[22]);
    v2di en24=*((v2di*)&src.effectedNumTable[24]);
    v2di en26=*((v2di*)&src.effectedNumTable[26]);
    v2di en28=*((v2di*)&src.effectedNumTable[28]);
    v2di en30=*((v2di*)&src.effectedNumTable[30]);
    v2di en32=*((v2di*)&src.effectedNumTable[32]);
    v2di en34=*((v2di*)&src.effectedNumTable[34]);
    v2di en36=*((v2di*)&src.effectedNumTable[36]);
    v2di en38=*((v2di*)&src.effectedNumTable[38]);

    *((v2di*)&(*this).effectedNumTable[20])=en20;
    *((v2di*)&(*this).effectedNumTable[22])=en22;
    *((v2di*)&(*this).effectedNumTable[24])=en24;
    *((v2di*)&(*this).effectedNumTable[26])=en26;
    *((v2di*)&(*this).effectedNumTable[28])=en28;
    *((v2di*)&(*this).effectedNumTable[30])=en30;
    *((v2di*)&(*this).effectedNumTable[32])=en32;
    *((v2di*)&(*this).effectedNumTable[34])=en34;
    *((v2di*)&(*this).effectedNumTable[36])=en36;
    *((v2di*)&(*this).effectedNumTable[38])=en38;
  }
#else
  for(int i=0;i<40;i++)
    (*this).effectedNumTable[i]=src.effectedNumTable[i];
#endif

#if (defined(__i386__) || defined(__x86_64__)) && !defined(OSL_NO_SSE)
  {  
    v2di e18=*((v2di*)&src.effects[18]);
    v2di e20=*((v2di*)&src.effects[20]);
    v2di e22=*((v2di*)&src.effects[22]);
    v2di e24=*((v2di*)&src.effects[24]);
    v2di e26=*((v2di*)&src.effects[26]);

    v2di e34=*((v2di*)&src.effects[34]);
    v2di e36=*((v2di*)&src.effects[36]);
    v2di e38=*((v2di*)&src.effects[38]);
    v2di e40=*((v2di*)&src.effects[40]);
    v2di e42=*((v2di*)&src.effects[42]);

    *((v2di*)&(*this).effects[18])=e18;
    *((v2di*)&(*this).effects[20])=e20;
    *((v2di*)&(*this).effects[22])=e22;
    *((v2di*)&(*this).effects[24])=e24;
    *((v2di*)&(*this).effects[26])=e26;

    *((v2di*)&(*this).effects[34])=e34;
    *((v2di*)&(*this).effects[36])=e36;
    *((v2di*)&(*this).effects[38])=e38;
    *((v2di*)&(*this).effects[40])=e40;
    *((v2di*)&(*this).effects[42])=e42;

    v2di e50=*((v2di*)&src.effects[50]);
    v2di e52=*((v2di*)&src.effects[52]);
    v2di e54=*((v2di*)&src.effects[54]);
    v2di e56=*((v2di*)&src.effects[56]);
    v2di e58=*((v2di*)&src.effects[58]);

    v2di e66=*((v2di*)&src.effects[66]);
    v2di e68=*((v2di*)&src.effects[68]);
    v2di e70=*((v2di*)&src.effects[70]);
    v2di e72=*((v2di*)&src.effects[72]);
    v2di e74=*((v2di*)&src.effects[74]);

    *((v2di*)&(*this).effects[50])=e50;
    *((v2di*)&(*this).effects[52])=e52;
    *((v2di*)&(*this).effects[54])=e54;
    *((v2di*)&(*this).effects[56])=e56;
    *((v2di*)&(*this).effects[58])=e58;

    *((v2di*)&(*this).effects[66])=e66;
    *((v2di*)&(*this).effects[68])=e68;
    *((v2di*)&(*this).effects[70])=e70;
    *((v2di*)&(*this).effects[72])=e72;
    *((v2di*)&(*this).effects[74])=e74;

    v2di e82=*((v2di*)&src.effects[82]);
    v2di e84=*((v2di*)&src.effects[84]);
    v2di e86=*((v2di*)&src.effects[86]);
    v2di e88=*((v2di*)&src.effects[88]);
    v2di e90=*((v2di*)&src.effects[90]);

    v2di e98=*((v2di*)&src.effects[98]);
    v2di e100=*((v2di*)&src.effects[100]);
    v2di e102=*((v2di*)&src.effects[102]);
    v2di e104=*((v2di*)&src.effects[104]);
    v2di e106=*((v2di*)&src.effects[106]);

    *((v2di*)&(*this).effects[82])=e82;
    *((v2di*)&(*this).effects[84])=e84;
    *((v2di*)&(*this).effects[86])=e86;
    *((v2di*)&(*this).effects[88])=e88;
    *((v2di*)&(*this).effects[90])=e90;

    *((v2di*)&(*this).effects[98])=e98;
    *((v2di*)&(*this).effects[100])=e100;
    *((v2di*)&(*this).effects[102])=e102;
    *((v2di*)&(*this).effects[104])=e104;
    *((v2di*)&(*this).effects[106])=e106;

    v2di e114=*((v2di*)&src.effects[114]);
    v2di e116=*((v2di*)&src.effects[116]);
    v2di e118=*((v2di*)&src.effects[118]);
    v2di e120=*((v2di*)&src.effects[120]);
    v2di e122=*((v2di*)&src.effects[122]);

    v2di e130=*((v2di*)&src.effects[130]);
    v2di e132=*((v2di*)&src.effects[132]);
    v2di e134=*((v2di*)&src.effects[134]);
    v2di e136=*((v2di*)&src.effects[136]);
    v2di e138=*((v2di*)&src.effects[138]);

    *((v2di*)&(*this).effects[114])=e114;
    *((v2di*)&(*this).effects[116])=e116;
    *((v2di*)&(*this).effects[118])=e118;
    *((v2di*)&(*this).effects[120])=e120;
    *((v2di*)&(*this).effects[122])=e122;

    *((v2di*)&(*this).effects[130])=e130;
    *((v2di*)&(*this).effects[132])=e132;
    *((v2di*)&(*this).effects[134])=e134;
    *((v2di*)&(*this).effects[136])=e136;
    *((v2di*)&(*this).effects[138])=e138;

    v2di e146=*((v2di*)&src.effects[146]);
    v2di e148=*((v2di*)&src.effects[148]);
    v2di e150=*((v2di*)&src.effects[150]);
    v2di e152=*((v2di*)&src.effects[152]);
    v2di e154=*((v2di*)&src.effects[154]);

    *((v2di*)&(*this).effects[146])=e146;
    *((v2di*)&(*this).effects[148])=e148;
    *((v2di*)&(*this).effects[150])=e150;
    *((v2di*)&(*this).effects[152])=e152;
    *((v2di*)&(*this).effects[154])=e154;
  }
#else
  for(int x=1;x<=9;x++)
    for(int y=1;y<=9;y++)
      this->effects[Square(x,y).index()]=src.effects[Square(x,y).index()];
#endif
}

bool osl::effect::operator==(const NumSimpleEffectTable& et1,const NumSimpleEffectTable& et2)
{
  for(int y=1;y<=9;y++)
    for(int x=9;x>0;x--){
      Square pos(x,y);
      if (!(et1.effectSetAt(pos)==et2.effectSetAt(pos))) return false;
    }
  if (! (et1.effected_mask == et2.effected_mask))
    return false;
  if(!(et1.mobilityTable==et2.mobilityTable)) return false;
  if(!(et1.effectedNumTable==et2.effectedNumTable)) return false;
  // intentionally ignore history dependent members: changed_effects, changed_effect_pieces, effected_changed_mask
  return true;
}

#ifndef MINIMAL
std::ostream& osl::effect::operator<<(std::ostream& os,const NumSimpleEffectTable& effectTable)
{
  os << "Effect" << std::endl;
  for(int y=1;y<=9;y++){
    for(int x=9;x>0;x--){
      Square pos(x,y);
      os << effectTable.effectSetAt(pos) << " ";
    }
    os << std::endl;
  }
  os << "Effect" << std::endl;
  for(int y=1;y<=9;y++){
    for(int x=9;x>0;x--){
      Square pos(x,y);
      os << effectTable.effectSetAt(pos) << " ";
    }
    os << std::endl;
  }
  return os;
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
