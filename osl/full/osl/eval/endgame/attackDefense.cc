/* attackDefense.cc
 */
#include "osl/eval/endgame/attackDefense.h"
#include "osl/container/pieceValues.h"

void osl::eval::endgame::
AttackDefense::setValues(const SimpleState& state, PieceValues& values)
{
  values.fill(0);
  // 速度は無視
  const Piece king_black = state.kingPiece(BLACK);
  const Piece king_white = state.kingPiece(WHITE);
  
  for (int i=0; i<Piece::SIZE; i++) {
    const Piece target = state.pieceOf(i);
    values[i] = valueOf(king_black, king_white, target);
  }
}

osl::eval::endgame::
AttackDefense::AttackDefense(const SimpleState& state)
{
  values.fill(0);
  const Piece king_black = state.kingPiece(BLACK);
  const Piece king_white = state.kingPiece(WHITE);
  for (int i=0; i<Piece::SIZE; i++) {
    const Piece target = state.pieceOf(i);
    addValue(king_black, king_white, target);
  }
}

void osl::eval::endgame::
AttackDefense::update(const SimpleState& new_state, Move last_move)
{
  if (last_move.isPass())
    return;

  const Piece black_king = new_state.kingPiece<BLACK>();
  const Piece white_king = new_state.kingPiece<WHITE>();
  const Square to = last_move.to();
  const Player player = new_state.turn();

  if (last_move.isDrop()) {
    assert(last_move.ptype() != KING);
    const int inc = valueOf(black_king, white_king, last_move.ptypeO(), to);
    const int dec = valueOf(black_king, white_king, last_move.ptypeO(), 
			    Square::STAND());
    addValue(player, inc - dec);
    return;
  }
  const Square from = last_move.from();

  if (last_move.ptype() != KING) {
    const int inc = valueOf(black_king, white_king, 
			    last_move.ptypeO(), to);
    const int dec = valueOf(black_king, white_king, 
			    last_move.oldPtypeO(), from);
    addValue(player, inc - dec);

    if (last_move.capturePtype() != PTYPE_EMPTY) {
      const int inc_capture
	= valueOf(black_king, white_king, captured(last_move.capturePtypeO()), 
		  Square::STAND());
      const int dec_capture
	= valueOf(black_king, white_king, last_move.capturePtypeO(), to);
      addValue(player, inc_capture);
      addValue(alt(player), -dec_capture);
    }
    return;
  }
  // KING
  reset();

  for (int i=0; i<Piece::SIZE; i++) {
    const Piece target = new_state.pieceOf(i);
    addValue(black_king, white_king, target);
  }
}

void osl::eval::endgame::
AttackDefense::updateKingMove(const SimpleState& state, 
			      Square from, Square to)
{
  reset();
  
  const Piece old_king = state.pieceOnBoard(from);
  const Player player = old_king.owner();
  assert(old_king.ptype() == KING);
  const Piece new_king = Piece::makeKing(player, to);
  
  const Piece king_black
    = (player == BLACK) ? new_king : state.kingPiece(BLACK);
  const Piece king_white
    = (player == WHITE) ? new_king : state.kingPiece(WHITE);

  for (int i=0; i<Piece::SIZE; i++) {
    const Piece target = state.pieceOf(i);
    if (target == old_king)
      addValue(king_black, king_white, new_king);
    else
      addValue(king_black, king_white, target);
  }
}

void osl::eval::endgame::
AttackDefense::updateKingMove(const SimpleState& state, 
			      Square from, Square to, Piece captured)
{
  reset();

  const Piece old_king = state.pieceOnBoard(from);
  const Player player = old_king.owner();
  assert(old_king.ptype() == KING);
  const Piece new_king = Piece::makeKing(player, to);
  
  const Piece king_black
    = (player == BLACK) ? new_king : state.kingPiece(BLACK);
  const Piece king_white
    = (player == WHITE) ? new_king : state.kingPiece(WHITE);

  for (int i=0; i<Piece::SIZE; i++) {
    const Piece target = state.pieceOf(i);
    if (target == old_king)
      addValue(king_black, king_white, new_king);
    else if (target == captured)
      addValue(king_black, king_white, captured.captured());
    else 
      addValue(king_black, king_white, target);
  }
}

int osl::eval::endgame::
AttackDefense::expect(const SimpleState& state, Move move) const 
{
  const Piece black_king = state.kingPiece<BLACK>();
  const Piece white_king = state.kingPiece<WHITE>();
  const Square to = move.to();
  if (move.isDrop()) {
    const PtypeO ptypeO = move.ptypeO();
    assert(getPtype(ptypeO) != KING);
    const int inc = valueOf(black_king, white_king, ptypeO, to);
    const int dec = valueOf(black_king, white_king, ptypeO, 
			    Square::STAND());
    return value() + inc - dec;
  }
  const Square from = move.from();
  const Piece old_piece = state.pieceOnBoard(from);
  const PtypeO new_ptypeo = move.ptypeO();
  if (old_piece.ptype() == KING) {
    AttackDefense new_eval = *this;
    if (move.capturePtype() == PTYPE_EMPTY)
      new_eval.updateKingMove(state, from, to);
    else
      new_eval.updateKingMove(state, from, to, state.pieceOnBoard(to));
    return new_eval.value();
  }
  const int inc = valueOf(black_king, white_king, new_ptypeo, to);
  const int dec = valueOf(black_king, white_king, old_piece.ptypeO(), from);
  if (move.capturePtype() == PTYPE_EMPTY)
    return value() + inc - dec;
  const int inc_capture
    = valueOf(black_king, white_king, captured(move.capturePtypeO()), 
	      Square::STAND());
  const int dec_capture
    = valueOf(black_king, white_king, move.capturePtypeO(), to);
  return value() + inc - dec + inc_capture - dec_capture;
}

void osl::eval::endgame::
AttackDefense::resetWeights(const int *w)
{
  AttackKing::resetWeights(w);
  DefenseKing::resetWeights(w+KingPieceTable::dimension());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
