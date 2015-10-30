/* piecePairPieceEval.h
 */
#ifndef EVAL_PPAIR_PIECEPAIRPIECEEVAL_H
#define EVAL_PPAIR_PIECEPAIRPIECEEVAL_H

#include "osl/eval/ppair/piecePairWithStand.h"
#include "osl/eval/ppair/piecePairTable.h"

namespace osl
{
  namespace eval
  {
    namespace ppair
    {
      class PiecePairPieceTable : public PiecePairTable<signed short>
      {
      public:
	~PiecePairPieceTable();
	/**
	 * @return successful load
	 * @param filename PiecePairRawTable 用のファイル
	 */
	bool setUp(const char *filename) const;

	/** user must initialize this before use */
	static PiecePairPieceTable Table;
	static PtypeEvalTable Piece_Value;
      };

      /**
       * 評価関数: PiecePairRawEval + PieceEval の点数を加えたもの
       */
      class PiecePairPieceEval
	: public PiecePairWithStand<PiecePairPieceTable>
      {
      public:
	typedef PiecePairWithStand<PiecePairPieceTable> base_t;
	explicit PiecePairPieceEval(const SimpleState& state) 
	  : base_t(state)
	{
	}
	static int adjustableDimension() { return PTYPE_SIZE; }
	static void resetWeights(const int *w);
      };
      
    } // namespace ppair
    using ppair::PiecePairPieceTable;
    using ppair::PiecePairPieceEval;
  } // namespace eval
} // namespace osl


#endif /* EVAL_PPAIR_PIECEPAIRPIECEEVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
