/**
 * attackDefense.h
 */
#ifndef EVAL_ENDGAME_ATTACKtDEFENSE_H
#define EVAL_ENDGAME_ATTACKtDEFENSE_H

#include "osl/eval/endgame/attackKing.h"
#include "osl/eval/endgame/defenseKing.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/evalTraits.h"

namespace osl
{
  namespace container
  {
    class PieceValues;
  } // namespace container
  namespace eval
  {
    namespace endgame
    {
      /**
       * max(AttackKing, DefenseKing).
       * うまく動くようなら統合した表を作る
       */
      class AttackDefense
      {
	CArray<int,2> values;
	void reset() { values.fill(0); }
	void addValue(Player owner, int value)
	{
	  values[playerToIndex(owner)] += value;
	}
	void addValue(Piece king_black, Piece king_white, Piece target)
	{
	  assert(king_black.ptype() == KING);
	  assert(king_white.ptype() == KING);
	  assert(king_black.owner() == BLACK);
	  assert(king_white.owner() == WHITE);
	  addValue(target.owner(), valueOf(king_black, king_white, target));
	}
      public:
	explicit AttackDefense(const SimpleState&);
	void changeTurn() {}
	static bool initialized() { return true; }

	int value() const { return values[0] + values[1]; }
	int value(Player p) const { return values[playerToIndex(p)]; }

	void update(const SimpleState& new_state, Move last_move);
	
	int expect(const SimpleState& state, Move move) const;
      private:
	void updateKingMove(const SimpleState&, Square from, Square to);
	void updateKingMove(const SimpleState&, Square from, Square to,
			    Piece target);
      public:
	static int infty()
	{
	  return PieceEval::infty()*2; // 2倍未満のボーナス
	}

	static int valueOf(Piece black_king, Piece white_king,
			   Piece target)
	{
	  return valueOf(black_king, white_king,
			 target.ptypeO(), target.square());
	}
	static int valueOf(Piece black_king, Piece white_king,
			   PtypeO ptypeo, Square position)
	{
	  assert(black_king.owner() == BLACK);
	  assert(white_king.owner() == WHITE);
	  
	  const Player player = getOwner(ptypeo);
	  const Piece my_king 
	    = (player == BLACK) ? black_king : white_king;
	  const Piece op_king 
	    = (player == BLACK) ? white_king : black_king;

	  const int attack = AttackKing::valueOf(op_king, ptypeo, position);
	  const int defense = DefenseKing::valueOf(my_king, ptypeo, position);

	  return max(player, attack, defense);
	}
	static void setValues(const SimpleState&, container::PieceValues&);
	static void resetWeights(const int *w);
      };
    } // namespace endgame
  } // namespace endgame
} // namespace osl

#endif /* EVAL_ENDGAME_ATTACKKING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
