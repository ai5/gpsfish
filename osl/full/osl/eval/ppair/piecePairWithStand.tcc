/* piecePairWithStand.tcc
 */
#ifndef EVAL_PPAIR_PIECEPAIRWITHSTAND_TCC
#define EVAL_PPAIR_PIECEPAIRWITHSTAND_TCC

#include "osl/eval/ppair/piecePairWithStand.h"
#include "osl/container/pieceValues.h"

template <class Table>
void osl::eval::ppair::PiecePairWithStand<Table>::
setValues(const SimpleState& state, container::PieceValues& values)
{
  base_t::setValues(state, values);
  // 速度は無視
  for (int i=0; i<Piece::SIZE; i++) 
  {
    const Piece piece = state.pieceOf(i);
    values[i] += Table::Piece_Value.value(piece.ptypeO());
    if (piece.isOnBoard())
      continue;
    assert(isBasic(piece.ptype()));
    if (isMajorBasic(piece.ptype()))
    {
      values[i] += Table::Piece_Value.value(newPtypeO(piece.owner(), PAWN));
    }
  }
}

template <class Table>
int osl::eval::ppair::PiecePairWithStand<Table>::
standBonus(const SimpleState& state)
{
  int result = 0;
  for (int i=0; i<Piece::SIZE; i++) 
  {
    const Piece piece = state.pieceOf(i);
    if (piece.isOnBoard())
      continue;
    result += standBonus(piece.ptypeO());
  }
  return result;
}

template <class Table>
osl::eval::ppair::PiecePairWithStand<Table>::
PiecePairWithStand(const SimpleState& state) 
  : base_t(state)
{
  for (int i=0; i<Piece::SIZE; i++) 
  {
    const Piece piece = state.pieceOf(i);
    base_t::val += Table::Piece_Value.value(piece.ptypeO());
  }
  base_t::val += standBonus(state);
}

#endif /* EVAL_PPAIR_PIECEPAIRWITHSTAND_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
