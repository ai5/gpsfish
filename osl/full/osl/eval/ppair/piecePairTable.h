/* piecePairTable.h
 */
#ifndef EVAL_PPAIR_PIECEPAIRTABLE_H
#define EVAL_PPAIR_PIECEPAIRTABLE_H

#include "osl/eval/ppair/piecePairIndex.h"

namespace osl
{
  namespace eval
  {
    namespace ppair
    {
      /**
       * 駒の関係毎の評価値を格納した表.
       * [Square*(Player*Ptype==PtypeO)] ^2
       *
       * @param T signed な char, int などが想定されている
       */
      template <class T>
      class PiecePairTable : public PiecePairIndex
      {
      public:
	typedef T value_type;
      protected:
	/** 
	 * const object に対して，データの変更は不可，読み込みは許可するために
	 * mutable にする．
	 */
	mutable CArray<value_type, maxPairIndex> values;
	PiecePairTable() { values.fill(); }
	~PiecePairTable() {}
      public:
	int value(unsigned int i) const
	{
	  return values[i];
	}
	value_type& valueOf(unsigned int i1, unsigned int i2)
	{
	  const unsigned int index = indexOf(i1,i2);
	  return values[index];
	}
	value_type& valueOf(Piece p1, Piece p2)
	{
	  const unsigned int index = indexOf(p1,p2);
	  return values[index];
	}
	int valueOf(unsigned int i1, unsigned int i2) const
	{
	  const unsigned int index = indexOf(i1,i2);
	  return values[index];
	}
	int valueOf(Piece p1, Piece p2) const
	{
	  const unsigned int index = indexOf(p1,p2);
	  return values[index];
	}

      };

    } // namespace ppair
  } // namespace eval
} // namespace osl


#endif /* EVAL_PPAIR_PIECEPAIRTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
