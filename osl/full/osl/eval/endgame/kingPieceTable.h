/* kingPieceTable.h
 */
#ifndef ENDGAME_KINGPIECETABLE_H
#define ENDGAME_KINGPIECETABLE_H

#include "osl/container.h"

namespace osl
{
  namespace container
  {
    class PieceValues;
  }
  namespace eval
  {
    namespace endgame
    {
      class KingPieceTable;
      bool operator==(const KingPieceTable& l, KingPieceTable& r);
      /**
       * 玉と他の駒の関係を保持
       */
      class KingPieceTable
      {
      public:
	enum { EffectiveDimension = 81*2*82*PTYPE_SIZE };
      protected:
	CArray2d<int,Square::SIZE*2,Square::SIZE*PTYPE_SIZE> data;
	KingPieceTable() { data.fill(0); }
      public:
	static int otherIndex(Square other, Ptype ptype)
	{
	  return other.index()*PTYPE_SIZE + ptype;
	}
	static int kingIndex(Square king, Player defense)
	{
	  return king.index()*2+playerToIndex(defense);
	}
	int& valueOf(Square king, Player defense, Square other, Ptype ptype)
	{
	  return data[kingIndex(king,defense)][otherIndex(other,ptype)];
	}
	int valueOf(Square king, Player defense, Square other, Ptype ptype) const
	{
	  return data[kingIndex(king,defense)][otherIndex(other, ptype)];
	}
	static int effectiveIndexOf(Square king, Player defense, Square other, Ptype ptype) 
	{
	  int base = (((king.x()-1)*9+king.y()-1)*2+playerToIndex(defense));
	  int s = other.isPieceStand() ? 0 : ((other.x()-1)*9+other.y());
	  return base*82*PTYPE_SIZE + s*PTYPE_SIZE + ptype;
	}
	void saveText(const char *filename) const;
	void loadText(const char *filename);
	void resetWeights(const int *w);
	void randomize();
	void clear();
	static int dimension() { return EffectiveDimension; }
	friend bool operator==(const KingPieceTable& l, KingPieceTable& r);
      };
    } // namespace endgame
  } // namespace endgame
} // namespace osl


#endif /* ENDGAME_KINGPIECETABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
