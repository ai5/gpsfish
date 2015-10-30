/**
 * attackKing.h
 */
#ifndef EVAL_ENDGAME_ATTACKKING_H
#define EVAL_ENDGAME_ATTACKKING_H

#include "osl/eval/endgame/kingPieceTable.h"
#include "osl/numEffectState.h"

namespace osl
{
  namespace eval
  {
    namespace endgame
    {
      /**
       * 玉の位置*攻撃駒の位置*ptype
       */
      class AttackKing
      {
	struct Table : public KingPieceTable
	{
	  void init();
	private:
	  void adhoc_edge_king_1(const Player player,
				 const Square king,
				 const Square attack);
	  void adhoc_edge_king_2(const Player player,
				 const Square king,
				 const Square attack);
	};
	static Table table;
      public:
	static int valueOf(const Piece king, const Piece attacker)
	{
	  return valueOf(king, attacker.ptypeO(), attacker.square());
	}
	static int valueOf(Piece king, PtypeO ptypeo, Square position)
	{
	  assert(king.ptype() == KING);
	  if (getOwner(ptypeo) == king.owner())
	    return 0;
	  return table.valueOf(king.square(), king.owner(), 
			       position, getPtype(ptypeo));
	}
	static void saveText(const char *filename);
	static void loadText(const char *filename) { table.loadText(filename); }
	static void resetWeights(const int *w) { table.resetWeights(w); }
	static void init() { table.init(); }
      };
    } // namespace endgame
  } // namespace endgame
} // namespace osl

#endif /* EVAL_ENDGAME_ATTACKKING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
