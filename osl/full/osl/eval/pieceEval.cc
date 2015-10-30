#include "osl/eval/pieceEval.h"
#include "osl/eval/pieceEval.tcc"
#include "osl/eval/evalTraits.h"
#include "osl/numEffectState.tcc"
#include "osl/effect_util/effectUtil.tcc"
#include "osl/bits/pieceTable.h"
#include "osl/oslConfig.h"

osl::eval::PtypeEvalTable osl::eval::PieceEval::Piece_Value;
static osl::SetUpRegister _initializer([](){ 
  osl::eval::PieceEval::Piece_Value.init();
});

namespace osl
{
  // explicit template instantiation
  template int PieceEval::computeDiffAfterMove<BLACK>
  (const NumEffectState&, Move);
  template int PieceEval::computeDiffAfterMove<WHITE>
  (const NumEffectState&, Move);

#ifndef DFPNSTATONE
#ifndef MINIMAL
  template void
  EffectUtil::findThreat<PieceEval>(const NumEffectState& state,
				    Square position,
				    PtypeO ptypeo,
				    PieceVector& out);
#endif
#endif
}

osl::PieceEval::PieceEval(const NumEffectState& state)
{
  int ret=0;
  for (int num=0;num<Piece::SIZE;num++) {
    if (state.standMask(BLACK).test(num))
    {
      ret+=Ptype_Eval_Table.value(newPtypeO(BLACK,Piece_Table.getPtypeOf(num)));
    }
    else if (state.standMask(WHITE).test(num))
    {
      ret+=Ptype_Eval_Table.value(newPtypeO(WHITE,Piece_Table.getPtypeOf(num)));
    }
    else{
      assert(state.isOnBoard(num));
      Piece p=state.pieceOf(num);
      ret+=Ptype_Eval_Table.value(p.ptypeO());
    }
  }
  val=ret;
}

osl::eval::
PieceEval::PieceEval()
{
  *this = PieceEval(NumEffectState());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
