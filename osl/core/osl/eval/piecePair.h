/* piecePair.h
 */
#ifndef OSL_EVAL_PIECEPAIR_H
#define OSL_EVAL_PIECEPAIR_H

#include "osl/eval/weights.h"
#include "osl/eval/evalTraits.h"
#include "osl/numEffectState.h"
#include <iostream>

namespace osl
{
  namespace eval
  {
    namespace ml
    {
      class PiecePair
      {
      public:
	enum { 
	  plain_table_size = 1338,
	  x_table_size = 4901,
	  y_table_size = 7057,
	  DIM = plain_table_size + x_table_size + y_table_size, // 14 * 12 * PTYPEO_SIZE * PTYPEO_SIZE
	};
	
	static int eval(const NumEffectState&, const Weights&);
	template<int Direction, int Offset>
	static int sum12One(const Piece *basePtr,const int *xbase, const int *ybase);
	static int sum12(NumEffectState const& state,Square base,PtypeO ptypeO);
 	template<int Direction, int Offset>
	static int adjust12One(const Piece *basePtr,const int *xbase1, const int *ybase1,const int *xbase2, const int *ybase2);
	static int adjust12(NumEffectState const& state,Square base,PtypeO pos,PtypeO neg);

	static int evalWithUpdate(const NumEffectState& state, Move moved, int last_value, const Weights& values);
	static int evalWithUpdateCompiled(const NumEffectState& state, Move moved, int last_value);

	static int pieceValue(const NumEffectState& state, Piece p, const Weights& values);
	static int pieceValueDouble(const NumEffectState& state, Piece p, const Weights&);
	static int weight(Player attack, int index, const Weights& values)
	{
	  return osl::eval::delta(attack) * values.value(index);
	}
	typedef CArray<int,3> index_t;
	static index_t index(int offset_id, Piece p, Piece q);
	static index_t index(int offset_id, Square p0, PtypeO o0, Square p1, PtypeO o1);
	
	static int value(int offset_id, Piece p, Piece q, const Weights& values)
	{
	  assert(p.isOnBoard() && q.isOnBoard());
	  return value(offset_id, p.square(), p.ptypeO(), q.square(), q.ptypeO(), values);
	}
	static int value(int offset_id, Piece p, Square p1, PtypeO o1, const Weights& values)
	{
	  return value(offset_id, p.square(), p.ptypeO(), p1, o1, values);
	}
	static int value(int offset_id, Square p0, PtypeO o0, Square p1, PtypeO o1, const Weights& values)
	{
	  assert(p0 != p1);
	  index_t idx = index(offset_id, p0, o0, p1, o1);
	  assert(idx[0] != 0);	// do not forget to call init()
	  int ret = 0;
	  for (int i=0; i<3; ++i)
	    ret += (idx[i] > 0) ? values.value(idx[i]) : -values.value(-idx[i]);
	  return ret;
	}

	static void init();
	static void sanitize(Weights& values);
	/** values を展開してクラス全体で使う */
	static void compile(const Weights& values);
	static int valueCompiled(int offset_id, Piece p, Square p1, PtypeO o1)
	{
	  return valueCompiled(offset_id, p.square(), p.ptypeO(), p1, o1);
	}
	static int valueCompiled(int offset_id, Square p0, PtypeO o0, Square p1, PtypeO o1);

	// 内部用
	struct IndexTable : public CArray3d<signed short, 12, PTYPEO_SIZE, PTYPEO_SIZE>
	{
	  IndexTable();
	  void fillBW(int index, int dir, Ptype p0, Ptype p1);
	  /** for same owner */
	  void fillSame(int index, int dir, Ptype p0, Ptype p1);
	  /** for different owner */
	  void fillDiffer(int index, int dir, Ptype p0, Ptype p1);
	  static int pindex(Player player, Ptype ptype) 
	  {
	    return ptypeOIndex(newPtypeO(player, ptype));
	  }
	  void amplify(int base);
	};
	static IndexTable plain_table;
	static CArray<IndexTable, 10> x_table, y_table;
	static const CArray<Offset, 12> offsets;	// offset_id -> Offset
      };
    }
  }
}


#endif /* OSL_EVAL_ATTACKKING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
