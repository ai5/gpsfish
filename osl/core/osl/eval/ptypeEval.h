/* pieceEval.h
 */
#ifndef OSL_PTYPEEVAL_H
#define OSL_PTYPEEVAL_H

#include "osl/eval/ptypeEvalTraits.h"
#include "osl/eval/evalTraits.h"
#include "osl/numEffectState.h"
#include <cassert>

namespace osl
{
  namespace eval
  {
    class PtypeEvalTable
    {
    protected:
      CArray<int, PTYPEO_SIZE> ptypeO2Val;
      CArray<int, PTYPEO_SIZE> promoteVal;
      CArray<int, PTYPEO_SIZE> captureVal;
    public:
      PtypeEvalTable();
      ~PtypeEvalTable();
      void init();
    public:
      /**
       * 先手から見たptypeOの駒の価値
       */
      int value(PtypeO ptypeO) const{
	assert(isValidPtypeO(ptypeO));
	return ptypeO2Val[ptypeO-PTYPEO_MIN];
      }
      /**
       * ptypeの駒の価値
       */
      int value(Ptype ptype) const{
	assert(isValid(ptype));
	return ptypeO2Val[ptype-PTYPEO_MIN];
      }
      /**
       * ptypeOにpromoteした時の評価値の増減
       */
      int promoteValue(PtypeO ptypeO) const{
	assert(isPromoted(ptypeO));
	return promoteVal[ptypeO-PTYPEO_MIN];
      }
      /**
       * ownerのptypeOがcaptureされた時の評価値の増減
       */
      int captureValue(PtypeO ptypeO) const{
	assert(isValidPtypeO(ptypeO));
	return captureVal[ptypeO-PTYPEO_MIN];
      }
      int diffWithMove(const NumEffectState&, Move move) const {
	int ret = 0;
	if (move.capturePtype() != PTYPE_EMPTY)
	  ret += captureValue(move.capturePtypeO());
	if (move.isPromotion())
	  ret+=promoteValue(move.ptypeO());
	return ret;
      }

      void reset(const CArray<int, PTYPE_SIZE>& values);
    };
    extern const PtypeEvalTable Ptype_Eval_Table;
  } // namespace eval
} // namespace osl

#endif /* OSL_PTYPEEVAL_H */

#ifndef EVAL_ML_PIECEEVAL_H
#define EVAL_ML_PIECEEVAL_H

#include "osl/eval/weights.h"

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class PieceEval
      {
	static CArray<int, PTYPEO_SIZE> table;
      public:
	static void setUp(const Weights &weights);
	static int eval(const NumEffectState &state);
	template<Player P>
	static int evalWithUpdate(const NumEffectState &,
			   Move moved, int last_value)
	{
	  assert(moved.player()==P);
	  int value = last_value;
	  if (moved.isPass() || moved.isDrop())
	    return last_value;
	  if (moved.isPromotion())
	  {
	    value -= table[moved.oldPtypeO() - PTYPEO_MIN];
	    value += table[moved.ptypeO() - PTYPEO_MIN];
	  }
	  Ptype captured = moved.capturePtype();
	  if (captured != PTYPE_EMPTY)
	  {
	    value -= table[newPtypeO(alt(P), captured) - PTYPEO_MIN];
	    value += table[newPtypeO(P, unpromote(captured)) - PTYPEO_MIN];
	  }
	  return value;
	}
	static int value(PtypeO ptypeO);
      };
    }
  }
}
#endif // EVAL_ML_PIECEEVAL_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
