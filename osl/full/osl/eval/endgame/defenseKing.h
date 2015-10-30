/**
 * defenseKing.h
 */
#ifndef EVAL_ENDGAME_DEFENSEKING_H
#define EVAL_ENDGAME_DEFENSEKING_H

#include "osl/eval/endgame/kingPieceTable.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace eval
  {
    namespace endgame
    {
      /**
       * 玉の位置*守備駒の位置*ptype
       */
      class DefenseKing
      {
	struct Table : public KingPieceTable
	{
	  void init();
	};
	static Table table;
      public:
	static int valueOf(const Piece king, const Piece defender)
	{
	  return valueOf(king, defender.ptypeO(), defender.square());
	}
	static int valueOf(Piece king, PtypeO ptypeo, Square position)
	{
	  assert(king.ptype() == KING);
	  if (getOwner(ptypeo) != king.owner())
	    return 0;
	  return table.valueOf(king.square(), king.owner(), 
			       position, getPtype(ptypeo));
	}
	static void saveText(const char *filename) { table.saveText(filename); }
	static void loadText(const char *filename) { table.loadText(filename); }
	static void resetWeights(const int *w) { table.resetWeights(w); }
	static void init() { table.init(); }
      };
    } // namespace endgame
  } // namespace endgame
} // namespace osl

#endif /* EVAL_ENDGAME_DEFENSEKING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
