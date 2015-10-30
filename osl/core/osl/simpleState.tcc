/* simpleState.tcc
 */
#ifndef OSL_SIMPLE_STATE_TCC
#define OSL_SIMPLE_STATE_TCC

#include "osl/simpleState.h"
#include <iostream>

template <bool show_error>
bool osl::SimpleState::isAlmostValidDrop(Move move) const
{
  assert(move.from().isPieceStand());
  const Square to=move.to();
  const Piece to_piece=pieceAt(to);
  const Ptype ptype=move.ptype();
  const Player turn = move.player();
  // ターゲットが空白か
  if (! to_piece.isEmpty()) {
    if (show_error) std::cerr << "drop on to piece : " << move << std::endl;
    return false;
  }
  // そもそもその駒を持っているか?
  if (! hasPieceOnStand(turn,ptype)) {
    if (show_error) std::cerr << turn << " don't have : " << ptype << std::endl;
    return false;
  }
  // 二歩のチェック
  if (ptype==PAWN && isPawnMaskSet(turn, to.x())) {
    if (show_error) std::cerr << " Double Pawn : " << move << std::endl;
    return false;
  }
  return true;
}

template <bool show_error>
bool
osl::SimpleState::testValidityOtherThanEffect(Move move) const
{
  const Square from=move.from();
  const Piece from_piece = pieceAt(from);
  const Square to=move.to();
  const Piece to_piece=pieceAt(to);
  // fromにあるのがその駒か
  if (from_piece.isEmpty() 
      || (from_piece.owner() != turn()))
  {
    if (show_error) 
      std::cerr << " No such piece0 : " << move << std::endl;
    return false;
  }
  // promoteしている時にpromote可能か
  if (move.isPromotion())
  {
    // fromにあるのがその駒か
    if (from_piece.ptype() != unpromote(move.ptype()))
    {
      if (show_error) 
	std::cerr << " No such piece1  : " << move << std::endl;
      return false;
    }
    if (from_piece.isPromotedNotKingGold())
    {
      if (show_error) 
	std::cerr << " can't promote promoted piece : " << move << std::endl;
      return false;
    }
  }
  else
  {
    // fromにあるのがその駒か
    if (from_piece.ptype() != move.ptype())
    {
      if (show_error) 
	std::cerr << " No such piece2  : " << move << std::endl;
      return false;
    }
  }
  // toにあるのが，相手の駒か空白か?
  if (!to_piece.isEmpty() && to_piece.owner()==turn()) {
    if (show_error) std::cerr << " No move on  : " << move << std::endl;
    return false;
  }
  // capturePtypeが一致しているか?
  if (to_piece.ptype()!=move.capturePtype()) {
    if (show_error) std::cerr << " Not such capture : " << move 
			      << std::endl << *this;
    return false;
  }
  return true;
}


#endif /* _SIMPLE_STATE_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
