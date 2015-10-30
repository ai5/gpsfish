/* piecePairIndex.h
 */
#ifndef EVAL_PIECEPAIRINDEX_H
#define EVAL_PIECEPAIRINDEX_H

#include "osl/numEffectState.h"
#include "osl/bits/squareCompressor.h"

namespace osl
{
  namespace eval
  {
    namespace ppair
    {
      /**
       * PiecePairEvalTable の添字計算.
       * キャッシュのヒット率を上げるために，r2246から計算を変更する．
       * r2246以降では片方あるいは両方がSquare::STAND()の場合は
       * tableの中身が0であるとして，差分計算の対象から外す．
       */
      struct PiecePairIndex
      {
	static const unsigned int maxSquareIndex = 82;
	static const unsigned int maxPtypeOIndex = PTYPEO_SIZE;
	static const unsigned int maxPieceIndex = maxSquareIndex*maxPtypeOIndex;
	static const unsigned int maxPairIndex = maxPieceIndex*maxPieceIndex;

	static unsigned int selfIndexOf(unsigned int i) 
	{
	  return indexOf(i, i);
	}

	static unsigned int indexOf(unsigned int i1, unsigned int i2)
	{
	  assert(i1 < maxPieceIndex);
	  assert(i2 < maxPieceIndex);
	  return i1*maxPieceIndex + i2;
	}
	static unsigned int canonicalIndexOf(unsigned int i1, unsigned int i2)
	{
	  if (i1 > i2)
	    std::swap(i1,i2);
	  return indexOf(i1,i2);
	}
	/** 逆変換 */
	static void meltIndex(size_t index, 
			      size_t& i1, size_t& i2)
	{
	  i1 = index / maxPieceIndex;
	  i2 = index % maxPieceIndex;
	}
	static unsigned int positionIndexOf(Square pos)
	{
	  unsigned int result = SquareCompressor::compress(pos);
	  assert(result < maxSquareIndex);
	  return result;
	}
	static unsigned int ptypeOIndexOf(PtypeO ptypeo)
	{
	  return ptypeo - PTYPEO_MIN;
	}
	static unsigned int indexOf(Square pos, PtypeO ptypeo)
	{
	  const int result = maxSquareIndex*ptypeOIndexOf(ptypeo)
	    + positionIndexOf(pos);
	  return result;
	}
	/** 逆変換 */
	static void meltIndex(size_t index, Square& pos, PtypeO& ptypeo)
	{
	  ptypeo = static_cast<PtypeO>(static_cast<int>(index / maxSquareIndex)+PTYPEO_MIN);
	  pos    = SquareCompressor::melt(index % maxSquareIndex);
	}

	static unsigned int indexOf(Piece piece)
	{
	  return indexOf(piece.square(), piece.ptypeO());
	}
	static unsigned int indexOf(Piece p1, Piece p2)
	{
	  return indexOf(indexOf(p1), indexOf(p2));
	}

	static unsigned int indexOfPieceNum(const SimpleState& s, int id)
	{
	  return indexOf(s.pieceOf(id));
	}


	/** 全ての関係についてfを実行する．重複する関係は訪れない */
	template <class F>
	static void forEachRelation(F f);
      };
    
    } // namespace ppair
    using ppair::PiecePairIndex;
  } // namespace eval
} // namespace osl


template <class F>
void osl::eval::ppair::
PiecePairIndex::forEachRelation(F f)
{
  for (int x=1; x<=9; ++x)
  {
    for (int y=1; y<=9; ++y)
    {
      const Square pos1(x,y);
      for (int ptype=PPAWN; ptype<=PTYPE_MAX; ++ptype)
      {
	const Ptype p1 = static_cast<Ptype>(ptype);
	const unsigned int i1 = 
	  indexOf(pos1, newPtypeO(BLACK, p1));
	const unsigned int i1w = 
	  indexOf(pos1, newPtypeO(WHITE, p1));
	f(indexOf(i1,i1));
	f(indexOf(i1w,i1w));
	
	for (int x2=x; x2<=9; ++x2)
	{
	  for (int y2=((x2 == x) ? y+1 : 1); y2<=9; ++y2)
	  {
	    const Square pos2(x2,y2);
	    
	    for (int ptype2=PPAWN; ptype2<=PTYPE_MAX; ++ptype2)
	    {
	      const Ptype p2 = static_cast<Ptype>(ptype2);
	      const unsigned int i2 = 
		indexOf(pos2, newPtypeO(BLACK, p2));
	      const unsigned int i2w = 
		indexOf(pos2, newPtypeO(WHITE, p2));

	      f(indexOf(i1, i2));
	      f(indexOf(i1, i2w));
	      f(indexOf(i1w, i2));
	      f(indexOf(i1w, i2w));
	    }
	  }
	}

	for (int ptype2=KING; ptype2<=PTYPE_MAX; ++ptype2)
	{
	  const Ptype p2 = static_cast<Ptype>(ptype2);
	  const unsigned int i2 = 
	    indexOf(Square::STAND(), newPtypeO(BLACK, p2));
	  const unsigned int i2w = 
	    indexOf(Square::STAND(), newPtypeO(WHITE, p2));

	  f(indexOf(i1, i2));
	  f(indexOf(i1, i2w));
	  f(indexOf(i1w, i2));
	  f(indexOf(i1w, i2w));
	}
      }	// ptype
    } // x
  } // y

  // 持駒同士は最後に
  for (int ptype=KING; ptype<=PTYPE_MAX; ++ptype)
  {
    const Ptype p1 = static_cast<Ptype>(ptype);
    const unsigned int i1 = 
      indexOf(Square::STAND(), newPtypeO(BLACK, p1));
    const unsigned int i1w = 
      indexOf(Square::STAND(), newPtypeO(WHITE, p1));
    f(indexOf(i1, i1));
    f(indexOf(i1, i1w));
    f(indexOf(i1w, i1w));
    for (int ptype2=ptype+1; ptype2<=PTYPE_MAX; ++ptype2)
    {
      const Ptype p2 = static_cast<Ptype>(ptype2);
      const unsigned int i2 = 
	indexOf(Square::STAND(), newPtypeO(BLACK, p2));
      const unsigned int i2w = 
	indexOf(Square::STAND(), newPtypeO(WHITE, p2));

      f(indexOf(i1, i2));
      f(indexOf(i1, i2w));
      f(indexOf(i1w, i2));
      f(indexOf(i1w, i2w));
    }
  }
}

#endif /* EVAL_PIECEPAIRINDEX_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
