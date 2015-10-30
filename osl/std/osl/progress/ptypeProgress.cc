#include "osl/progress/ptypeProgress.h"
#include "osl/bits/pieceTable.h"
#include "osl/oslConfig.h"
#include <iostream>

osl::progress::PtypeProgressTable osl::progress::Ptype_Progress_Table;
static osl::SetUpRegister _initializer([](){ 
  osl::progress::Ptype_Progress_Table.init();
});
/** y 座標に対応した進行度の係数、最初は0 (使用しない)*/
const osl::CArray<int,10> osl::progress::PtypeProgressTable::yVals =
{{   
  0,4,4,4,3,2,1,0,0,0
}};

namespace osl
{
  namespace progress
  {
    /** ゲームの進行度を測る駒の種類別の係数 */
    template<Ptype T>
    struct PtypeProgressTraits;
    // 歩
    template<>
    struct PtypeProgressTraits<PAWN>{
      static const int val=2;
    };
    template<>
    struct PtypeProgressTraits<PPAWN>{
      static const int val=2;
    };
    //
    template<>
    struct PtypeProgressTraits<LANCE>{
      static const int val=5;
    };
    template<>
    struct PtypeProgressTraits<PLANCE>{
      static const int val=5;
    };
    //
    template<>
    struct PtypeProgressTraits<KNIGHT>{
      static const int val=5;
    };
    template<>
    struct PtypeProgressTraits<PKNIGHT>{
      static const int val=5;
    };
    //
    template<>
    struct PtypeProgressTraits<SILVER>{
      static const int val=7;
    };
    template<>
    struct PtypeProgressTraits<PSILVER>{
      static const int val=7;
    };
    //
    template<>
    struct PtypeProgressTraits<GOLD>{
      static const int val=8;
    };
    //
    template<>
    struct PtypeProgressTraits<BISHOP>{
      static const int val=7;
    };
    template<>
    struct PtypeProgressTraits<PBISHOP>{
      static const int val=7;
    };
    //
    template<>
    struct PtypeProgressTraits<ROOK>{
      static const int val=10;
    };
    template<>
    struct PtypeProgressTraits<PROOK>{
      static const int val=10;
    };
    //
    template<>
    struct PtypeProgressTraits<KING>{
      static const int val=15;
    };
  } // namespace progress
} // namespace osl

osl::progress::
PtypeProgress::PtypeProgress(SimpleState const& state)
{
  int ret=0;
  for (int num=0; num<Piece::SIZE; num++)
  {
    if(state.standMask(BLACK).test(num)){
      ret+=Ptype_Progress_Table.progress(newPtypeO(BLACK,Piece_Table.getPtypeOf(num)),
				     Square::STAND());
    }
    else if(state.standMask(WHITE).test(num)){
      ret+=Ptype_Progress_Table.progress(newPtypeO(WHITE,Piece_Table.getPtypeOf(num)),
				     Square::STAND());
    }
    else{
      assert(state.isOnBoard(num));
      const Piece p=state.pieceOf(num);
      ret+=Ptype_Progress_Table.progress(p.ptypeO(),p.square());
    }
  }
  val=ret;
}

void osl::progress::PtypeProgressTable::init()
{
  ptype2Val[PAWN]=PtypeProgressTraits<PAWN>::val;
  ptype2Val[PPAWN]=PtypeProgressTraits<PPAWN>::val;
  ptype2Val[LANCE]=PtypeProgressTraits<LANCE>::val;
  ptype2Val[PLANCE]=PtypeProgressTraits<PLANCE>::val;
  ptype2Val[KNIGHT]=PtypeProgressTraits<KNIGHT>::val;
  ptype2Val[PKNIGHT]=PtypeProgressTraits<PKNIGHT>::val;
  ptype2Val[SILVER]=PtypeProgressTraits<SILVER>::val;
  ptype2Val[PSILVER]=PtypeProgressTraits<PSILVER>::val;
  ptype2Val[GOLD]=PtypeProgressTraits<GOLD>::val;
  ptype2Val[KING]=PtypeProgressTraits<KING>::val;
  ptype2Val[BISHOP]=PtypeProgressTraits<BISHOP>::val;
  ptype2Val[PBISHOP]=PtypeProgressTraits<PBISHOP>::val;
  ptype2Val[ROOK]=PtypeProgressTraits<ROOK>::val;
  ptype2Val[PROOK]=PtypeProgressTraits<PROOK>::val;
  for(int i=PTYPE_MIN;i<=PTYPE_MAX;i++){
    Ptype ptype=static_cast<Ptype>(i);
    pos2Val[newPtypeO(BLACK,ptype)-PTYPEO_MIN][Square::STAND().index()]=ptype2Val[i]*yVals[5];
    pos2Val[newPtypeO(WHITE,ptype)-PTYPEO_MIN][Square::STAND().index()]=ptype2Val[i]*yVals[5];
    for(int y=1;y<10;y++)
    {
      for(int x=9;x>0;x--)
      {
	pos2Val[newPtypeO(BLACK,ptype)-PTYPEO_MIN][Square(x,y).index()]
	  = ptype2Val[i]*yVals[y];
	pos2Val[newPtypeO(WHITE,ptype)-PTYPEO_MIN][Square(x,10-y).index()]
	  = ptype2Val[i]*yVals[y];
      }
    }
  }
}

osl::progress::PtypeProgressTable::~PtypeProgressTable() {
}

#ifndef MINIMAL
std::ostream& osl::progress::operator<<(std::ostream& os, PtypeProgress prog)
{
  return os << "progress " << prog.progress();
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
