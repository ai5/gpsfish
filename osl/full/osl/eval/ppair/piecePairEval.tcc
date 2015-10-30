/* piecePairEval.tcc
 */
#ifndef EVAL_PIECE_PAIR_EVAL_TCC
#define EVAL_PIECE_PAIR_EVAL_TCC

#include "osl/eval/ppair/piecePairEval.h"
#include "osl/eval/ppair/piecePairAssert.h"
#include "osl/container/pieceValues.h"
#include "osl/bits/pieceTable.h"
#include "osl/oslConfig.h"

template <class Table>
osl::eval::ppair::
PiecePairEvalTableBase<Table>::PiecePairEvalTableBase(const SimpleState& state) 
{
  for (int i=0; i<Piece::SIZE; i++) {
    for (int j=i; j<Piece::SIZE; j++) {
      val += Table::Table.valueOf
	(PiecePairIndex::indexOfPieceNum(state, i), 
	 PiecePairIndex::indexOfPieceNum(state, j));
    }
  }
}

template <class Table>
bool& osl::eval::ppair::
PiecePairEvalTableBase<Table>::initializationFlag()
{
  static bool flag = false;
  return flag;
}

template <class Table>
bool osl::eval::ppair::
PiecePairEvalTableBase<Table>::setUp(const char *filename)
{
  bool& result = initializationFlag();
  result = Table::Table.setUp(filename);
  return result;
}

template <class Table>
bool osl::eval::ppair::
PiecePairEvalTableBase<Table>::setUp()
{
  std::string filename = OslConfig::home();
  filename += "/data/sibling-attack.pair";
  return setUp(filename.c_str());
}

template <class Table>
int osl::eval::ppair::PiecePairEvalTableBase<Table>::
adjustPairs(const SimpleState& state,
	    unsigned int new_index)
{
  int diff = 0;
  for (int i=0; i<Piece::SIZE; i++) 
  {
    const Piece p=state.pieceOf(i);
    if(p.isOnBoard()){
      const unsigned int target = 
	PiecePairIndex::indexOf(p);
      // 以下は target==old_index で1回以上(駒台)マッチ
      diff += Table::Table.valueOf(target, new_index);
    }
  }
  diff += Table::Table.valueOf(new_index, new_index);
  return diff;
}

template <class Table>
int osl::eval::ppair::PiecePairEvalTableBase<Table>::
adjustPairs(const SimpleState& state,
	    unsigned int old_index, unsigned int new_index)
{
  int diff = 0;
  for (int i=0; i<Piece::SIZE; i++) 
  {
    const Piece p=state.pieceOf(i);
    if(p.isOnBoard()){
      const unsigned int target = 
	PiecePairIndex::indexOf(p);
      diff -= Table::Table.valueOf(target, old_index);
      // 以下は target==old_index で1回以上(駒台)マッチ
      diff += Table::Table.valueOf(target, new_index);
    }
  }
  diff -= Table::Table.valueOf(old_index, new_index); // 足しすぎた分
  diff += Table::Table.valueOf(new_index, new_index);
  return diff;
}

template <class Table>
int osl::eval::ppair::PiecePairEvalTableBase<Table>::
adjustPairs(const SimpleState& state, 
	    unsigned int old_index, unsigned int old_index2, 
	    unsigned int new_index)
{
  int diff = 0;
  for (int i=0;i<Piece::SIZE; i++) 
  {
    const Piece p=state.pieceOf(i);
    if(p.isOnBoard()){
      const unsigned int target = 
	PiecePairIndex::indexOf(p);
      diff += Table::Table.valueOf(target, new_index);
      diff -= Table::Table.valueOf(target, old_index);
      diff -= Table::Table.valueOf(target, old_index2);
    }
  }

  diff -= Table::Table.valueOf(old_index, new_index);
  diff -= Table::Table.valueOf(old_index2, new_index);
  diff += Table::Table.valueOf(old_index, old_index2);

  diff += Table::Table.valueOf(new_index, new_index);
  return diff;
}

// update
template <class Table>
int osl::eval::ppair::PiecePairEvalTableBase<Table>::
adjustPairsAfterMove(const SimpleState& state,
		     unsigned int new_index)
{
  int diff = 0;
  for (int i=0; i<Piece::SIZE; i++) 
  {
    const Piece p=state.pieceOf(i);
    if (p.isOnBoard()) {
      const unsigned int target = 
	PiecePairIndex::indexOf(p);
      diff += Table::Table.valueOf(target, new_index);
    }
  }
  return diff;
}

template <class Table>
int osl::eval::ppair::PiecePairEvalTableBase<Table>::
adjustPairsAfterMove(const SimpleState& state,
		     unsigned int old_index, unsigned int new_index)
{
  int diff = 0;
  for (int i=0; i<Piece::SIZE; i++) 
  {
    const Piece p=state.pieceOf(i);
    if(p.isOnBoard()){
      const unsigned int target = 
	PiecePairIndex::indexOf(p);
      diff -= Table::Table.valueOf(target, old_index);
      diff += Table::Table.valueOf(target, new_index);
    }
  }
  diff -= Table::Table.valueOf(old_index, old_index);
  diff += Table::Table.valueOf(new_index, old_index); // 引きすぎた分
  return diff;
}

template <class Table>
int osl::eval::ppair::PiecePairEvalTableBase<Table>::
adjustPairsAfterMove(const SimpleState& state, 
		     unsigned int old_index, unsigned int old_index2, 
		     unsigned int new_index)
{
  int diff = 0;
  for (int i=0;i<Piece::SIZE; i++) 
  {
    const Piece p=state.pieceOf(i);
    if(p.isOnBoard()){
      const unsigned int target = 
	PiecePairIndex::indexOf(p);
      diff += Table::Table.valueOf(target, new_index);
      diff -= Table::Table.valueOf(target, old_index);
      diff -= Table::Table.valueOf(target, old_index2);
    }
  }

  diff += Table::Table.valueOf(new_index, old_index);
  diff += Table::Table.valueOf(new_index, old_index2);

  diff -= Table::Table.valueOf(old_index, old_index2);
  diff -= Table::Table.valueOf(old_index, old_index);
  diff -= Table::Table.valueOf(old_index2, old_index2);
  return diff;
}

template <class Table>
void osl::eval::ppair::PiecePairEvalTableBase<Table>::
setValues(const SimpleState& state, container::PieceValues& values) 
{
  values.fill(0);
  // 速度は無視
  for (int i=0; i<Piece::SIZE; i++) {
    const bool i_is_king = (Piece_Table.getPtypeOf(i) == KING);
    for (int j=0; j<Piece::SIZE; j++) {
      if (i==j)
	continue;
      const bool j_is_king = (Piece_Table.getPtypeOf(j) == KING);
      
      const int relation_value = Table::Table.valueOf
	(PiecePairIndex::indexOfPieceNum(state, i), 
	 PiecePairIndex::indexOfPieceNum(state, j));
      if (i_is_king && (! j_is_king))
      {
	values[j] += relation_value;
      }
      else
      {
	values[i] += relation_value;
      }
    }
  }
  for (int i=0; i<Piece::SIZE; i++) 
  {
    values[i] /= 2;

    const unsigned int index = PiecePairIndex::indexOfPieceNum(state, i);
    const int self_value = Table::Table.valueOf(index, index);
    values[i] += self_value;
  }
}

/* ------------------------------------------------------------------------- */

template <class Eval, class Table>
osl::eval::ppair::
PiecePairEval<Eval,Table>::PiecePairEval(const SimpleState& state) 
  : PiecePairEvalTableBase<Table>(state)
{
}

template <class Eval, class Table>
int osl::eval::ppair::PiecePairEval<Eval,Table>::
expect(const SimpleState& state, Move m) const
{
  const Ptype ptype = m.ptype();
  const Square to = m.to();
  const Player player = state.turn();
  if (m.isDrop()) {
    piece_pair_assert(state.pieceAt(to) == Piece::EMPTY());    
    return this->roundUp(this->val + Eval::diffAfterDropMove(state, to, newPtypeO(player, ptype)));
  }
  
  const Square from = m.from();
  piece_pair_assert(state.pieceAt(from) != Piece::EMPTY());
  if (m.capturePtype() == PTYPE_EMPTY) {
    piece_pair_assert(state.pieceAt(to) == Piece::EMPTY());
    return this->roundUp(this->val + Eval::diffAfterSimpleMove(state, from, to, m.promoteMask()));
  }
  piece_pair_assert(state.pieceAt(to) != Piece::EMPTY());
  return this->roundUp(this->val + Eval::diffAfterCaptureMove(state, from, to, m.capturePtypeO(), m.promoteMask()));
}

#endif /* EVAL_PIECE_PAIR_EVAL_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
