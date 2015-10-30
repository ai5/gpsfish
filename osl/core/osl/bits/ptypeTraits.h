/* ptypeTraits.h
 */
#ifndef OSL_PTYPETRAITS_H
#define OSL_PTYPETRAITS_H

#include "osl/basic_type.h"
#include "osl/bits/directionTraits.h"

namespace osl
{
  template<Ptype T>
  struct PtypeTraits;
  
  template <>
  struct PtypeTraits<PTYPE_EMPTY>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    /** 打ち歩詰を除いて考えると promoteは常に得 */
    static const bool betterToPromote=false;
    static const char *name() { return "PTYPE_EMPTY";}
    static const char *csaName() { return "..";}
    static const int moveMask=0;
  };
  
  template <>
  struct PtypeTraits<PTYPE_EDGE>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    static const bool betterToPromote=false;
    static const char *name() { return "PTYPE_EDGE";}
    static const char *csaName() { return "XX";}
    static const int moveMask=0;
  };
  
  template <>
  struct PtypeTraits<GOLD>
  {
    static const bool isBasic=true;
    static const bool canPromote=false;
    static const bool betterToPromote=false;
    static const Ptype moveType=GOLD;
    static const char *name() { return "GOLD";}
    static const char *csaName() { return "KI";}
    static const int indexMin=26;
    static const int indexLimit=30;
    static const int dropBlackFromY=1;
    static const int dropBlackToY=9;
    static const Ptype basicType=GOLD;
    static const int moveMask=
    DirectionTraits<UL>::mask|DirectionTraits<U>::mask
    |DirectionTraits<UR>::mask|DirectionTraits<L>::mask
    |DirectionTraits<R>::mask|DirectionTraits<D>::mask;
  };
  
  template <>
  struct PtypeTraits<PAWN>
  {
    static const bool isBasic=true;
    static const bool canPromote=true;
    static const bool betterToPromote=true;
    static const Ptype moveType=PAWN;
    static const Ptype basicType=PAWN;
    static const char *name() { return "PAWN";}
    static const char *csaName() { return "FU";}
    static const int indexMin=0;
    static const int indexLimit=18;
    static const int dropBlackFromY=2;
    static const int dropBlackToY=9;
    static const int mayPromoteToY=4;
    static const int moveMask=DirectionTraits<U>::mask;
  };
  
  template <>
  struct PtypeTraits<PPAWN>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    // 疑問 falseの方がよいのでは?
    static const bool betterToPromote=true;
    static const Ptype moveType=GOLD;
    static const char *name() { return "PPAWN";}
    static const char *csaName() { return "TO";}
    static const int moveMask=PtypeTraits<GOLD>::moveMask;
    static const Ptype basicType=PAWN;
    static const int indexMin=PtypeTraits<basicType>::indexMin;
  };
  
  template <>
  struct PtypeTraits<LANCE>
  {
    static const bool isBasic=true;
    static const bool canPromote=true;
    static const bool betterToPromote=false;
    static const Ptype moveType=LANCE;
    static const Ptype basicType=LANCE;
    static const char *name() { return "LANCE";}
    static const char *csaName() { return "KY";}
    static const int indexMin=32;
    static const int indexLimit=36;
    static const int dropBlackFromY=2;
    static const int dropBlackToY=9;
    static const int mayPromoteToY=9;
    static const int moveMask=DirectionTraits<LONG_U>::mask;
  };
  
  template <>
  struct PtypeTraits<PLANCE>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    static const bool betterToPromote=false;
    static const Ptype moveType=GOLD;
    static const char *name() { return "PLANCE";}
    static const char *csaName() { return "NY";}
    static const int moveMask=PtypeTraits<GOLD>::moveMask;
    static const Ptype basicType=LANCE;
    static const int indexMin=PtypeTraits<basicType>::indexMin;
  };
  
  template <>
  struct PtypeTraits<KNIGHT>
  {
    static const bool isBasic=true;
    static const bool canPromote=true;
    static const bool betterToPromote=false;
    static const Ptype moveType=KNIGHT;
    static const Ptype basicType=KNIGHT;
    static const char *name() { return "KNIGHT";}
    static const char *csaName() { return "KE";}
    static const int indexMin=18;
    static const int indexLimit=22;
    static const int dropBlackFromY=3;
    static const int dropBlackToY=9;
    static const int mayPromoteToY=5;
    static const int moveMask=DirectionTraits<UUL>::mask|DirectionTraits<UUR>::mask;
  };
  
  template <>
  struct PtypeTraits<PKNIGHT>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    static const bool betterToPromote=false;
    static const Ptype moveType=GOLD;
    static const char *name() { return "PKNIGHT";}
    static const char *csaName() { return "NK";}
    static const int moveMask=PtypeTraits<GOLD>::moveMask;
    static const Ptype basicType=KNIGHT;
    static const int indexMin=PtypeTraits<basicType>::indexMin;
  };
  
  template <>
  struct PtypeTraits<SILVER>
  {
    static const bool isBasic=true;
    static const bool canPromote=true;
    static const bool betterToPromote=false;
    static const Ptype moveType=SILVER;
    static const Ptype basicType=SILVER;
    static const char *name() { return "SILVER";}
    static const char *csaName() { return "GI";}
    static const int indexMin=22;
    static const int indexLimit=26;
    static const int dropBlackFromY=1;
    static const int dropBlackToY=9;
    static const int mayPromoteToY=4;
    static const int moveMask=
    DirectionTraits<UL>::mask|DirectionTraits<U>::mask
    |DirectionTraits<UR>::mask|DirectionTraits<DL>::mask
    |DirectionTraits<DR>::mask;
  };
  
  template <>
  struct PtypeTraits<PSILVER>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    static const bool betterToPromote=false;
    static const Ptype moveType=GOLD;
    static const char *name() { return "PSILVER";}
    static const char *csaName() { return "NG";}
    static const int moveMask=PtypeTraits<GOLD>::moveMask;
    static const Ptype basicType=SILVER;
    static const int indexMin=PtypeTraits<basicType>::indexMin;
  };
  
  template <>
  struct PtypeTraits<BISHOP>
  {
    static const bool isBasic=true;
    static const bool canPromote=true;
    static const bool betterToPromote=true;
    static const Ptype moveType=BISHOP;
    static const Ptype basicType=BISHOP;
    static const char *name() { return "BISHOP";}
    static const char *csaName() { return "KA";}
    static const int indexMin=36;
    static const int indexLimit=38;
    static const int dropBlackFromY=1;
    static const int dropBlackToY=9;
    static const int mayPromoteToY=9;
    static const int moveMask=
    DirectionTraits<LONG_UL>::mask|DirectionTraits<LONG_UR>::mask
    |DirectionTraits<LONG_DL>::mask|DirectionTraits<LONG_DR>::mask;
  };
  
  template <>
  struct PtypeTraits<PBISHOP>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    // 疑問 falseの方がよいのでは?
    static const bool betterToPromote=true;
    static const Ptype moveType=PBISHOP;
    static const char *name() { return "PBISHOP";}
    static const char *csaName() { return "UM";}
    static const int moveMask=
    DirectionTraits<LONG_UL>::mask|DirectionTraits<LONG_UR>::mask
    |DirectionTraits<LONG_DL>::mask|DirectionTraits<LONG_DR>::mask
    |DirectionTraits<U>::mask|DirectionTraits<L>::mask
    |DirectionTraits<R>::mask|DirectionTraits<D>::mask;
    static const Ptype basicType=BISHOP;
    static const int indexMin=PtypeTraits<basicType>::indexMin;
  };
  
  template <>
  struct PtypeTraits<ROOK>
  {
    static const bool isBasic=true;
    static const bool canPromote=true;
    static const bool betterToPromote=true;
    static const Ptype moveType=ROOK;
    static const Ptype basicType=ROOK;
    static const char *name() { return "ROOK";}
    static const char *csaName() { return "HI";}
    static const int indexMin=38;
    static const int indexLimit=40;
    static const int dropBlackFromY=1;
    static const int dropBlackToY=9;
    static const int mayPromoteToY=9;
    static const int moveMask=
    DirectionTraits<LONG_U>::mask|DirectionTraits<LONG_L>::mask
    |DirectionTraits<LONG_R>::mask|DirectionTraits<LONG_D>::mask;
  };
  
  template <>
  struct PtypeTraits<PROOK>
  {
    static const bool isBasic=false;
    static const bool canPromote=false;
    // 疑問 falseの方がよいのでは?
    static const bool betterToPromote=true;
    static const Ptype moveType=PROOK;
    static const char *name() { return "PROOK";}
    static const char *csaName() { return "RY";}
    static const int moveMask=
    DirectionTraits<LONG_U>::mask|DirectionTraits<LONG_L>::mask
    |DirectionTraits<LONG_R>::mask|DirectionTraits<LONG_D>::mask
    |DirectionTraits<UL>::mask|DirectionTraits<UR>::mask
    |DirectionTraits<DL>::mask|DirectionTraits<DR>::mask;
    static const Ptype basicType=ROOK;
    static const int indexMin=PtypeTraits<basicType>::indexMin;
  };
  
  
  template <>
  struct PtypeTraits<KING>
  {
    static const bool isBasic=true;
    static const bool canPromote=false;
    static const bool betterToPromote=false;
    static const Ptype moveType=KING;
    static const Ptype basicType=KING;
    static const char *name() { return "KING";}
    static const char *csaName() { return "OU";}
    static const int indexMin=30;
    static const int indexLimit=32;
    static const int dropBlackFromY=1;
    static const int dropBlackToY=9;
    static const int moveMask=
    DirectionTraits<U>::mask|DirectionTraits<L>::mask
    |DirectionTraits<R>::mask|DirectionTraits<D>::mask
    |DirectionTraits<UL>::mask|DirectionTraits<UR>::mask
    |DirectionTraits<DL>::mask|DirectionTraits<DR>::mask;
  };

  template<Ptype T,bool IsBasic>
  struct PtypeFunsSub;
  
  template<Ptype T>
  struct PtypeFunsSub<T,true>
  {
    static const uint64_t indexMask=(-1LL<<(PtypeTraits<T>::indexMin))^(-1LL<<(PtypeTraits<T>::indexLimit));
    static const Ptype promotePtype=static_cast<Ptype>(static_cast<int>(T)-8);
    static const Ptype basicType = T;
  };
  
  template<Ptype T>
  struct PtypeFunsSub<T,false>
  {
    static const uint64_t indexMask=static_cast<uint64_t>(0);
    //    static const Ptype promotePtype=PTYPE_EMPTY;
    static const Ptype promotePtype=T;
    static const Ptype basicType = PtypeTraits<T>::basicType;
  };
  
  template<Ptype T>
  struct PtypeFuns
  {
#if OSL_WORDSIZE == 64
    static const unsigned int indexNum=0;
#elif OSL_WORDSIZE == 32
    static const unsigned int indexNum=(PtypeTraits<T>::indexMin >> 5);
#endif
    static const bool hasLongMove=(PtypeTraits<T>::indexMin>=32);
    static const uint64_t indexMask=PtypeFunsSub<T,PtypeTraits<T>::isBasic>::indexMask;
    static const Ptype promotePtype=PtypeFunsSub<T,PtypeTraits<T>::canPromote>::promotePtype;
    static const Ptype basicType=PtypeFunsSub<T,PtypeTraits<T>::isBasic>::basicType;
  };

  /**
   * ある方向にある駒が移動可能かを表す.
   * (basicTypeだけは確定しているが，promote済みかどうかはわからない場合)
   */
  enum MoveConstraint { 
    /** 可能でない */
    CannotMove,
    /** promote済みの駒の場合にのみ可能 */
    OnlyPromoted,
    /** promoteしていない駒の場合にのみ可能 */
    OnlyBasic,
    /** promoteしていようが，いまいが可能 */
    NoConstraint,
  };

  template<Ptype T,Direction D>
  struct PtypeDirectionTraits
  {
    static const bool hasMove=(PtypeTraits<T>::moveMask & DirectionTraits<D>::mask)!=0;
    static const bool canMove=
    (PtypeTraits<T>::moveMask & DirectionTraits<D>::mask)!=0 ||
    (PtypeTraits<T>::moveMask & 
     DirectionTraits<DirectionTraits<D>::longDir>::mask)!=0;
    static const MoveConstraint moveConstraint =
    (PtypeDirectionTraits<T,D>::canMove 
     ? (PtypeDirectionTraits<PtypeFuns<T>::promotePtype,D>::canMove 
	? NoConstraint : OnlyBasic ) 
     : (PtypeDirectionTraits<PtypeFuns<T>::promotePtype,D>::canMove 
	? OnlyPromoted : CannotMove));
  };
  
  
  template<Player T>
  struct KingTraits
  {
    static const int index=PtypeTraits<KING>::indexMin+playerToIndex(T);
  };

  template<Ptype T,Player P>
  struct PtypePlayerTraits
  {
    static bool canDropTo(Square pos)
    {
      static_assert(PtypeTraits<T>::isBasic, "canDropTo");
      if (PtypeTraits<T>::dropBlackFromY == 1)
	return true; 

      if (P==BLACK)
	return pos.y() >= PtypeTraits<T>::dropBlackFromY;
      else
	return pos.y() <= Square::reverseY(PtypeTraits<T>::dropBlackFromY);
    }
    /**
     * posにある駒がpromoteする可能性があるか?
     * 先手BISHOPが49,58,59,69などにいる場合は可能性がないが，この時点では排除しない
     */
    static bool mayPromote(Square pos)
    {
      static_assert(PtypeTraits<T>::isBasic&&PtypeTraits<T>::canPromote, "mayPromote");
      if (PtypeTraits<T>::mayPromoteToY == 9)
	return true; 

      if (P==BLACK)
	return pos.y() <= PtypeTraits<T>::mayPromoteToY;
      else
	return pos.y() >= Square::reverseY(PtypeTraits<T>::mayPromoteToY);
    }
    /**
     * posにあるTの駒がpromoteする手しかない
     */
    static bool mustPromote(Square pos)
    {
      if(P==BLACK){
	if(T==PAWN || T==LANCE) return pos.yEq<2>();
	else if(T==KNIGHT) return pos.yLe<4>();
	else return false;
      }
      else{
	if(T==PAWN || T==LANCE) return pos.yEq<8>();
	else if(T==KNIGHT) return pos.yGe<6>();
	else return false;
      }
    }
    /**
     * posにあるTの駒がどの方向に動いてもpromote可能
     */
    static bool canPromote(Square pos)
    {
      if(P==BLACK){
	if(T==PAWN || T==LANCE) return pos.yLe<4>();
	else if(T==KNIGHT) return pos.yLe<5>();
	else return pos.yLe<3>();
      }
      else{
	if(T==PAWN || T==LANCE) return pos.yGe<6>();
	else if(T==KNIGHT) return pos.yGe<5>();
	else return pos.yGe<7>();
      }
    }
    /**
     * posにあるTの駒がpromote可能なdirectionに動く時だけpromote可能
     * shortの時はその時のみYES
     */
    static bool checkPromote(Square pos)
    {
      if(P==BLACK){
	if(T==SILVER) return pos.yEq<4>();
	else if(T==LANCE || T==ROOK || T==BISHOP)
	  return true;
	else return false;
      }
      else{
	if(T==SILVER) return pos.yEq<6>();
	else if(T==LANCE || T==ROOK || T==BISHOP)
	  return true;
	else return false;
      }
    }
    /**
     * posにあるTの駒は次に絶対にpromoteできない
     */
    static bool noPromote(Square pos)
    {
      if(P==BLACK){
	if(T==PAWN || T==SILVER) return pos.yGe<5>();
	else if(T==KNIGHT) return pos.yGe<6>();
	else if(T==LANCE || T==ROOK || T==BISHOP) return false;
	else return true;
      }
      else{
	if(T==PAWN || T==SILVER) return pos.yLe<5>();
	else if(T==KNIGHT) return pos.yLe<4>();
	else if(T==LANCE || T==ROOK || T==BISHOP) return false;
	else return true;
      }
    }
  };
  
  
} // namespace osl

#endif /* OSL_PTYPETRAITS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
