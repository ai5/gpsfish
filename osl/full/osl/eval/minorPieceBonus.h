/* minorPieceBonus.h
 */
#ifndef EVAL_MINORPIECEBONUS_H
#define EVAL_MINORPIECEBONUS_H

#include "osl/eval/pieceEval.h"
#include "osl/progress.h"

namespace osl
{
  namespace eval
  {
    struct MinorPieceDebugInfo
    {
      int pawn_bonus, lance_bonus, knight_bonus, gold_bonus;
    };
    /**
     * 歩切れなどの評価
     */
    class MinorPieceBonus
    {
      CArray<int,2> pawn_on_stand;
      CArray<int,2> lance_on_stand;
      CArray<int,2> knight_on_stand;
      CArray<int,2> pawns;
      CArray<int,2> golds;
    private:
      int pawnBonus(Progress16 progress16) const
      {
	const int black_pawn = pawn_on_stand[BLACK];
	const int white_pawn = pawn_on_stand[WHITE];
	const int black_pawn_total = pawns[BLACK];
	const int white_pawn_total =  pawns[WHITE];
	int result = 0;
	if (black_pawn > 1)
	{
	  result -= (black_pawn - 1) * progress16.value() *
	    PtypeEvalTraits<PAWN>::val / 32;
	}
	else if (black_pawn == 0 && black_pawn_total < white_pawn_total)
	{
	  result -= PtypeEvalTraits<PAWN>::val / 2;
	}
	if (black_pawn >= 9)
	{
	  result -= (black_pawn - 8) * progress16.value() *
	    PtypeEvalTraits<PAWN>::val / 8;
	}
	if (white_pawn > 1)
	{
	  result += (white_pawn - 1) * progress16.value() * 
	    PtypeEvalTraits<PAWN>::val / 32;
	}
	else if (white_pawn == 0 && white_pawn_total < black_pawn_total)
	{
	  result += PtypeEvalTraits<PAWN>::val / 2;
	}
	if (white_pawn >= 9)
	{
	  result += (white_pawn - 8) * progress16.value() *
	    PtypeEvalTraits<PAWN>::val / 8;
	}
	return result;
      }
      int lanceBonus(Progress16 progress16) const
      {
	const int black_lance = lance_on_stand[BLACK];
	const int white_lance = lance_on_stand[WHITE];
	int result = 0;
	if (black_lance > 1)
	{
	  result -= (black_lance - 1) * progress16.value() * PtypeEvalTraits<LANCE>::val / 32;
	}
	if (black_lance > 2)
	{
	  result -= (black_lance - 2) * progress16.value() * PtypeEvalTraits<PAWN>::val / 24;
	}
	if (white_lance > 1)
	{
	  result += (white_lance - 1) * progress16.value() * PtypeEvalTraits<LANCE>::val / 32;
	}
	if (white_lance > 2)
	{
	  result += (white_lance - 2) * progress16.value() * PtypeEvalTraits<PAWN>::val / 24;
	}
	return result;
      }
      int knightBonus(Progress16 progress16) const
      {
	const int black_knight = knight_on_stand[BLACK];
	const int white_knight = knight_on_stand[WHITE];
	int result = 0;
	if (black_knight > 1)
	{
	  result -= (black_knight - 1) * progress16.value() * PtypeEvalTraits<KNIGHT>::val / 32;
	}
	if (black_knight > 2)
	{
	  result -= (black_knight - 2) * progress16.value() * PtypeEvalTraits<PAWN>::val / 8;
	}
	if (white_knight > 1)
	{
	  result += (white_knight - 1) * progress16.value() * PtypeEvalTraits<KNIGHT>::val / 32;
	}
	if (white_knight > 2)
	{
	  result += (white_knight - 2) * progress16.value() * PtypeEvalTraits<PAWN>::val / 8;
	}
	return result;
      }
      int goldBonus(Progress16 black, Progress16 white) const
      {
	const int black_gold = golds[BLACK];
	const int white_gold = golds[WHITE];
	if (black_gold >= 3)
	{
	  return white.value() * PtypeEvalTraits<GOLD>::val
	    * (black_gold - 2);
	}
	else if (white_gold >= 3)
	{
	  return -black.value() * PtypeEvalTraits<GOLD>::val
	    * (white_gold - 2);
	}
	
	return 0;
      }

    public:
      MinorPieceBonus(const SimpleState& state) 
      {
	pawn_on_stand[BLACK] = state.countPiecesOnStand(BLACK, PAWN);
	pawn_on_stand[WHITE] = state.countPiecesOnStand(WHITE, PAWN);
	lance_on_stand[BLACK] = state.countPiecesOnStand(BLACK, LANCE);
	lance_on_stand[WHITE] = state.countPiecesOnStand(WHITE, LANCE);
	knight_on_stand[BLACK] = state.countPiecesOnStand(BLACK, KNIGHT);
	knight_on_stand[WHITE] = state.countPiecesOnStand(WHITE, KNIGHT);
	pawns[BLACK] = 0;
	pawns[WHITE] = 0;
	golds[BLACK] = 0;
	golds[WHITE] = 0;
	for (int i = PtypeTraits<PAWN>::indexMin;
	     i < PtypeTraits<PAWN>::indexLimit; i++)
	{
	  const Piece pawn = state.pieceOf(i);
	  if (pawn.owner() == BLACK)
	    pawns[BLACK]++;
	  else
	    pawns[WHITE]++;
	}

	for (int i = PtypeTraits<GOLD>::indexMin;
	     i < PtypeTraits<GOLD>::indexLimit; i++)
	{
	  const Piece gold = state.pieceOf(i);
	  golds[gold.owner()]++;
	}
      }

      int value(Progress16 progress16,
		Progress16 black,
		Progress16 white) const
      {
	return pawnBonus(progress16) + lanceBonus(progress16) +
	  knightBonus(progress16) + goldBonus(black, white);
      }

      void update(const SimpleState& /*new_state*/, Move last_move)
      {
	const Player player = last_move.player();
	const Ptype ptype = last_move.ptype();
	if (last_move.isDrop()) {
	  if (ptype == PAWN) {
	    pawn_on_stand[player]--;
	    assert(pawn_on_stand[BLACK] >= 0);
	    assert(pawn_on_stand[WHITE] >= 0);
	  }
	  if (ptype == LANCE) {
	    lance_on_stand[player]--;
	  }
	  if (ptype == KNIGHT) {
	    knight_on_stand[player]--;
	  }
	  return;
	}
	const Ptype captured = last_move.capturePtype();
	if (captured != PTYPE_EMPTY) {
	  switch (unpromote(captured)) {
	  case PAWN:
	    pawn_on_stand[player]++;
	    pawns[player]++;
	    pawns[alt(player)]--;
	    assert(pawns[BLACK] + pawns[WHITE] == 18);
	    assert(pawn_on_stand[BLACK] >= 0);
	    assert(pawn_on_stand[WHITE] >= 0);
	    break;
	  case LANCE:
	    lance_on_stand[player]++;
	    break;
	  case KNIGHT:
	    knight_on_stand[player]++;
	    break;
	  case GOLD:
	    golds[player]++;
	    golds[alt(player)]--;
	    assert(golds[BLACK] + golds[WHITE] == 4);
	    break;
	  default:
	    ;
	  }
	}
      }

      int expect(const SimpleState& state, Move move, Progress16 progress16,
		 Progress16 black,
		 Progress16 white) const
      {
	MinorPieceBonus new_eval = *this;
	if (move.isDrop()){
	  const Ptype ptype = move.ptype();
	  if (ptype == PAWN) {
	    new_eval.pawn_on_stand[state.turn()]--;
	    assert(new_eval.pawn_on_stand[BLACK] >= 0);
	    assert(new_eval.pawn_on_stand[WHITE] >= 0);
	  }
	  else if (ptype == LANCE) {
	    new_eval.lance_on_stand[state.turn()]--;
	  }
	  else if (ptype == KNIGHT) {
	    new_eval.knight_on_stand[state.turn()]--;
	  }
	  return new_eval.value(progress16, black, white);
	}
	Ptype ptype = move.capturePtype();
	if (ptype != PTYPE_EMPTY) {
	  if (unpromote(ptype) == PAWN) {
	    new_eval.pawn_on_stand[state.turn()]++;
	    new_eval.pawns[state.turn()]++;
	    new_eval.pawns[alt(state.turn())]--;
	    assert(new_eval.pawns[BLACK] + new_eval.pawns[WHITE] == 18);
	    assert(new_eval.pawn_on_stand[BLACK] >= 0);
	    assert(new_eval.pawn_on_stand[WHITE] >= 0);
	  }
	  else if (unpromote(ptype) == LANCE) {
	    new_eval.lance_on_stand[state.turn()]++;
	  }
	  else if (unpromote(ptype) == KNIGHT) {
	    new_eval.knight_on_stand[state.turn()]++;
	  }
	  else if (unpromote(ptype) == GOLD) {
	    new_eval.golds[state.turn()]++;
	    new_eval.golds[alt(state.turn())]--;
	    assert(new_eval.golds[BLACK] + new_eval.golds[WHITE] == 4);
	  }
	}
	return new_eval.value(progress16, black, white);
      }

      MinorPieceDebugInfo debugInfo(Progress16 progress16,
				    Progress16 black, Progress16 white) const
      {
	MinorPieceDebugInfo debug_info;
	debug_info.pawn_bonus = pawnBonus(progress16);
	debug_info.lance_bonus = lanceBonus(progress16);
	debug_info.knight_bonus = knightBonus(progress16);
	debug_info.gold_bonus = goldBonus(black, white);

	return debug_info;
      }
    };
  } // namespace eval
} // namespace osl

#endif /* EVAL_MINORPIECEBONUS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
