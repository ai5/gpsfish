/* piecePairRawEval.h
 */
#ifndef EVAL_PPAIR_PIECEPAIRRAWEVAL_H
#define EVAL_PPAIR_PIECEPAIRRAWEVAL_H

#include "osl/eval/ppair/piecePairEval.h"
#include "osl/eval/ppair/piecePairTable.h"

namespace osl
{
  namespace eval
  {
    namespace ppair
    {
      class PiecePairRawTable : public PiecePairTable<signed char>
      {
      public:
	PiecePairRawTable();
	~PiecePairRawTable();
	/**
	 * 一度だけ読み込む
	 * @return successful load
	 */
	bool setUp(const char *filename) const;
	/**
	 * バイナリファイルから読み込む.
	 * r2246以降ではファイルイメージとメモリイメージとは，PiecePairIndex
	 * が違うので，変換が必要.
	 * @return successful load
	 */
	bool loadFromBinaryFile(const char *filename) const;
	/**
	 * バイナリファイルに書き出す.
	 * r2246以降ではファイルイメージとメモリイメージとは，PiecePairIndex
	 * が違うので，変換が必要.
	 * @return successful load
	 */
	void writeInBinaryFile(const char *filename) const;

	/** user must initialize this before use */
	static PiecePairRawTable Table;
      };

      /**
       * 関係の価値は[-127,127]点の評価関数.
       */
      class PiecePairRawEval : public PiecePairEval<PiecePairRawEval,PiecePairRawTable>
      {
      public:
	typedef PiecePairEval<PiecePairRawEval,PiecePairRawTable> base_t;
	explicit PiecePairRawEval(const SimpleState& state) 
	  : base_t(state)
	{
	}
      };
    } // namespace ppair
    using ppair::PiecePairRawTable;
    using ppair::PiecePairRawEval;
  } // namespace eval
} // namespace osl


#endif /* EVAL_PPAIR_PIECEPAIRRAWEVAL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
