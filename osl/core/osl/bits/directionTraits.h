/* directionTraits.h
 */
#ifndef OSL_DIRECTIONTRAITS_H
#define OSL_DIRECTIONTRAITS_H

#include "osl/basic_type.h"

namespace osl
{
  template<Direction Dir>
  struct DirectionTraitsGen;
  
  template<>
  struct DirectionTraitsGen<UL>{
      static const int blackDx=1;
      static const int blackDy=-1;
      static const bool canPromoteTo=true;
      static const Direction altDir=DR;
      static const Direction longDir=LONG_UL;
      static const Direction primDir=UL;
      static const int ptypeMask=
	(1<<PPAWN)|(1<<PLANCE)|(1<<PKNIGHT)|(1<<PSILVER)|(1<<PBISHOP)|
        (1<<PROOK)|(1<<KING)|(1<<GOLD)|(1<<SILVER)|(1<<BISHOP);
    };
  
  template<>
    struct DirectionTraitsGen<U>{
      static const int blackDx=0;
      static const int blackDy=-1;
      static const bool canPromoteTo=true;
      static const Direction altDir=D;
      static const Direction longDir=LONG_U;
      static const Direction primDir=U;
      static const int ptypeMask=
	(1<<PPAWN)|(1<<PLANCE)|(1<<PKNIGHT)|(1<<PSILVER)|(1<<PBISHOP)|
        (1<<PROOK)|(1<<KING)|(1<<GOLD)|(1<<PAWN)|(1<<LANCE)|(1<<SILVER)|(1<<ROOK);
    };
  
  template<>
    struct DirectionTraitsGen<UR>{
      static const int blackDx=-1;
      static const int blackDy=-1;
      static const bool canPromoteTo=true;
      static const Direction altDir=DL;
      static const Direction longDir=LONG_UR;
      static const Direction primDir=UR;
      static const int ptypeMask=
	(1<<PPAWN)|(1<<PLANCE)|(1<<PKNIGHT)|(1<<PSILVER)|(1<<PBISHOP)|
        (1<<PROOK)|(1<<KING)|(1<<GOLD)|(1<<SILVER)|(1<<BISHOP);
    };
  template<>
    struct DirectionTraitsGen<L>{
      static const int blackDx=1;
      static const int blackDy=0;
      static const bool canPromoteTo=false;
      static const Direction altDir=R;
      static const Direction longDir=LONG_L;
      static const Direction primDir=L;
      static const int ptypeMask=
	(1<<PPAWN)|(1<<PLANCE)|(1<<PKNIGHT)|(1<<PSILVER)|(1<<PBISHOP)|
        (1<<PROOK)|(1<<KING)|(1<<GOLD)|(1<<ROOK);
    };
  template<>
    struct DirectionTraitsGen<R>{
      static const int blackDx=-1;
      static const int blackDy=0;
      static const bool canPromoteTo=false;
      static const Direction altDir=L;
      static const Direction longDir=LONG_R;
      static const Direction primDir=L;
      static const int ptypeMask=
	(1<<PPAWN)|(1<<PLANCE)|(1<<PKNIGHT)|(1<<PSILVER)|(1<<PBISHOP)|
        (1<<PROOK)|(1<<KING)|(1<<GOLD)|(1<<ROOK);
    };
  template<>
    struct DirectionTraitsGen<DL>{
      static const int blackDx=1;
      static const int blackDy=1;
      static const bool canPromoteTo=false;
      static const Direction altDir=UR;
      static const Direction longDir=LONG_DL;
      static const Direction primDir=UR;
      static const int ptypeMask=
	(1<<PBISHOP)|(1<<PROOK)|(1<<KING)|(1<<SILVER)|(1<<BISHOP);
    };
  template<>
    struct DirectionTraitsGen<D>{
      static const int blackDx=0;
      static const int blackDy=1;
      static const bool canPromoteTo=false;
      static const Direction altDir=U;
      static const Direction longDir=LONG_D;
      static const Direction primDir=U;
      static const int ptypeMask=
	(1<<PPAWN)|(1<<PLANCE)|(1<<PKNIGHT)|(1<<PSILVER)|(1<<PBISHOP)|
        (1<<PROOK)|(1<<KING)|(1<<GOLD)|(1<<ROOK);
    };
  template<>
    struct DirectionTraitsGen<DR>{
      static const int blackDx=-1;
      static const int blackDy=1;
      static const bool canPromoteTo=false;
      static const Direction altDir=UL;
      static const Direction longDir=LONG_DR;
      static const Direction primDir=UL;
      static const int ptypeMask=
	(1<<PBISHOP)|(1<<PROOK)|(1<<KING)|(1<<SILVER)|(1<<BISHOP);
    };
  template<>
    struct DirectionTraitsGen<UUL>{
      static const int blackDx=1;
      static const int blackDy=-2;
      static const bool canPromoteTo=true;
      // no meaning
      static const Direction altDir=UUL;
      static const Direction longDir=UUL;
      static const Direction primDir=UUL;
      static const int ptypeMask=
	(1<<KNIGHT);
    };
  template<>
    struct DirectionTraitsGen<UUR>{
      static const int blackDx=-1;
      static const int blackDy=-2;
      static const bool canPromoteTo=true;
      // no meaning
      static const Direction altDir=UUR;
      static const Direction longDir=UUR;
      static const Direction primDir=UUR;
      static const int ptypeMask=
	(1<<KNIGHT);
    };
  
  template<>
    struct DirectionTraitsGen<LONG_UL>{
      static const bool canPromoteTo=true;
      static const Direction altDir=LONG_DR;
      static const Direction longDir=LONG_UL;
      static const Direction primDir=UL;
      static const int blackDx=DirectionTraitsGen<UL>::blackDx;
      static const int blackDy=DirectionTraitsGen<UL>::blackDy;
      static const int ptypeMask=
	(1<<PBISHOP)|(1<<BISHOP);
    };
  
  template<>
    struct DirectionTraitsGen<LONG_U>{
      static const bool canPromoteTo=true;
      static const Direction altDir=LONG_D;
      static const Direction longDir=LONG_U;
      static const Direction primDir=U;
      static const int blackDx=DirectionTraitsGen<U>::blackDx;
      static const int blackDy=DirectionTraitsGen<U>::blackDy;
      static const int ptypeMask=
	(1<<LANCE)|(1<<ROOK)|(1<<PROOK);
    };
  
  template<>
    struct DirectionTraitsGen<LONG_UR>{
      static const bool canPromoteTo=true;
      static const Direction altDir=LONG_DL;
      static const Direction longDir=LONG_UR;
      static const Direction primDir=UR;
      static const int blackDx=DirectionTraitsGen<UR>::blackDx;
      static const int blackDy=DirectionTraitsGen<UR>::blackDy;
      static const int ptypeMask=
	(1<<PBISHOP)|(1<<BISHOP);
    };
  template<>
    struct DirectionTraitsGen<LONG_L>{
      static const bool canPromoteTo=false;
      static const Direction altDir=LONG_R;
      static const Direction longDir=LONG_L;
      static const Direction primDir=L;
      static const int blackDx=DirectionTraitsGen<L>::blackDx;
      static const int blackDy=DirectionTraitsGen<L>::blackDy;
      static const int ptypeMask=
	(1<<ROOK)|(1<<PROOK);
    };
  template<>
    struct DirectionTraitsGen<LONG_R>{
      static const bool canPromoteTo=false;
      static const Direction altDir=LONG_L;
      static const Direction longDir=LONG_R;
      static const Direction primDir=L;
      static const int blackDx=DirectionTraitsGen<R>::blackDx;
      static const int blackDy=DirectionTraitsGen<R>::blackDy;
      static const int ptypeMask=
	(1<<ROOK)|(1<<PROOK);
    };
  template<>
    struct DirectionTraitsGen<LONG_DL>{
      static const bool canPromoteTo=false;
      static const Direction altDir=LONG_UR;
      static const Direction longDir=LONG_DL;
      static const Direction primDir=UR;
      static const int blackDx=DirectionTraitsGen<DL>::blackDx;
      static const int blackDy=DirectionTraitsGen<DL>::blackDy;
      static const int ptypeMask=
	(1<<PBISHOP)|(1<<BISHOP);
    };
  template<>
    struct DirectionTraitsGen<LONG_D>{
      static const bool canPromoteTo=false;
      static const Direction altDir=LONG_U;
      static const Direction longDir=LONG_D;
      static const Direction primDir=U;
      static const int blackDx=DirectionTraitsGen<D>::blackDx;
      static const int blackDy=DirectionTraitsGen<D>::blackDy;
      static const int ptypeMask=
	(1<<ROOK)|(1<<PROOK);
    };
  template<>
    struct DirectionTraitsGen<LONG_DR>{
      static const bool canPromoteTo=false;
      static const Direction altDir=LONG_UL;
      static const Direction longDir=LONG_DR;
      static const Direction primDir=UL;
      static const int blackDx=DirectionTraitsGen<DR>::blackDx;
      static const int blackDy=DirectionTraitsGen<DR>::blackDy;
      static const int ptypeMask=
	(1<<PBISHOP)|(1<<BISHOP);
    };
  template<Direction Dir>
  struct DirectionTraits{
    // これらを関数にすると押し込められる
    static const unsigned int mask=1<<static_cast<int>(Dir);
    static const bool isLong=(static_cast<int>(Dir) >= LONG_UL);
    static const int blackDx=DirectionTraitsGen<Dir>::blackDx;
    static const int blackDy=DirectionTraitsGen<Dir>::blackDy;
    static const Offset blackOffset() { return Offset(blackDx,blackDy); }
      static const bool canPromoteTo=DirectionTraitsGen<Dir>::canPromoteTo;
      static const Direction longDir=DirectionTraitsGen<Dir>::longDir;
    static const int ptypeMask=DirectionTraitsGen<Dir>::ptypeMask;
    static const int ptypeMaskNotKing=DirectionTraitsGen<Dir>::ptypeMask &
				      ~(1<<KING);
    static const Direction primDir=DirectionTraitsGen<Dir>::primDir;
    static const Direction altDir=DirectionTraitsGen<Dir>::altDir;
    };
  
  template<Direction Dir,Player P>
    struct DirectionPlayerTraits;
  
  template<Direction Dir>
    struct DirectionPlayerTraits<Dir,BLACK>{
      static const Offset offset() { 
	return DirectionTraits<Dir>::blackOffset(); 
      }
      static const Direction directionByBlack=Dir;
    };
  template<Direction Dir> 
  const Direction DirectionPlayerTraits<Dir,BLACK>::directionByBlack;
  
  template<Direction Dir>
    struct DirectionPlayerTraits<Dir,WHITE>{
      static const Offset offset() { 
	return -DirectionTraits<Dir>::blackOffset();
      }
      static const Direction directionByBlack=DirectionTraitsGen<Dir>::altDir;
    };
  template<Direction Dir> 
  const Direction DirectionPlayerTraits<Dir,WHITE>::directionByBlack;

  template <Player P, Direction D>
  Offset Offset::make() {
    return DirectionPlayerTraits<D,P>::offset();
  }
} // namespace osl

#endif /* OSL_DIRECTIONTRAITS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
