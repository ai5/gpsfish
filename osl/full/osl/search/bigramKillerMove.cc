/* bigramKillerMove.cc
 */
#include "osl/search/bigramKillerMove.h"
#include <iostream>

osl::search::
BigramKillerMove::BigramKillerMove()
{
  clear();
}

osl::search::
BigramKillerMove::~BigramKillerMove()
{
}

void osl::search::
BigramKillerMove::clear()
{
  for (size_t i=0; i<killer_moves.size(); ++i)
    for (size_t j=0; j<killer_moves[i].size(); ++j)
      killer_moves[i][j].clear();
}

void osl::search::
BigramKillerMove::getMove(const NumEffectState& state, Move last_move,
			  MoveVector& out) const
{
  if (last_move.isInvalid())
    return;
  const LRUMoves& moves = operator[](last_move);

  for (size_t i=0; i<moves.size(); ++i)
  {
    Move bigram_move = moves[i];
    if (bigram_move.isInvalid())
      return;
    const Ptype ptype = bigram_move.oldPtype();
    const Square from = bigram_move.from();
    if ((! from.isPieceStand())
	&& (state.pieceOnBoard(from).ptype() != ptype))
    {
      if (! Ptype_Table.hasLongMove(ptype))
	return;
      assert(isPiece(ptype));
      const Square to = bigram_move.to();
      const Player player = state.turn();
      const PieceMask pieces 
	= state.piecesOnBoard(player) & state.effectSetAt(to);
      // to に同じptype の利きがあれば拾う
      for (int i=Ptype_Table.getIndexMin(unpromote(ptype)); 
	   i<Ptype_Table.getIndexLimit(unpromote(ptype)); ++i)
      {
	if (pieces.test(i))
	{
	  const Piece moving = state.pieceOf(i);
	  assert(moving.owner() == player);
	  if (moving.ptype() != ptype)
	    continue;
	  const Square new_from = moving.square();
	  const bool promote = (! isPromoted(ptype))
	    && (to.canPromote(player) || new_from.canPromote(player));
	  bigram_move = Move(new_from, to, 
			     (promote
			      ? osl::promote(moving.ptype()) : moving.ptype()),
			     bigram_move.capturePtype(), 
			     promote, player);
	  assert(state.isValidMoveByRule(bigram_move,false));
	  break;
	}
      }
    }
    if (state.isAlmostValidMove<false>(bigram_move))
      out.push_back(bigram_move);
  }
}

void osl::search::
BigramKillerMove::dump() const
{
  for (int y=1; y<=9; ++y)
  {
    for (int x=1; x<=9; ++x)
    {
      const Square position(x,y);
      for (int p=PTYPEO_MIN; p<=PTYPEO_MAX; ++p)
      {
	const PtypeO ptypeo = static_cast<PtypeO>(p);
	const LRUMoves& moves
	  = killer_moves[position.index()][ptypeOIndex(ptypeo)];
	if (moves[0].isNormal())
	{
	  std::cerr << position << " " << moves[0];
	  if (moves[1].isNormal())
	    std::cerr << " " << moves[1];
	  std::cerr << "\n";
	}
      }
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
