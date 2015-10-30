/* piecePairPieceTable.cc
 */
#include "osl/eval/ppair/piecePairPieceEval.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/ppair/piecePairEval.tcc"
#include "osl/eval/ppair/piecePairWithStand.tcc"
#include "osl/eval/pieceEval.h"

osl::eval::ppair::PiecePairPieceTable osl::eval::ppair::PiecePairPieceTable::Table;
osl::eval::PtypeEvalTable osl::eval::ppair::PiecePairPieceTable::Piece_Value;
static osl::SetUpRegister _initializer([](){ 
  // The parent, PtypeEvalTable's constructor has initialized before main.
  // However, due to the initialization order problem outside tables.cc
  // the initilized value may be zero.
  osl::eval::ppair::PiecePairPieceTable::Piece_Value.init();
});

namespace osl
{
  namespace eval
  {
    namespace ppair
    {
      template class PiecePairEvalTableBase<PiecePairPieceTable>;
      template class PiecePairEval<PiecePairWithStand<PiecePairPieceTable>,PiecePairPieceTable>;
      template class PiecePairWithStand<PiecePairPieceTable>;
    } // namespace ppair
  } // namespace eval
} // namespace osl

osl::eval::ppair::
PiecePairPieceTable::~PiecePairPieceTable()
{
}


bool osl::eval::ppair::
PiecePairPieceTable::
setUp(const char *filename) const
{
  if (! PiecePairRawEval::setUp(filename))
    return false;

  for (unsigned int i=0; i<maxPairIndex; ++i)
  {
    values[i] = PiecePairRawTable::Table.value(i)
      *128/100; // scale up according to pawn value change
  }

  // add once, decrease later
  for (int y=1; y<=9; ++y)
  {
    for (int x=1; x<=9; ++x)
    {
      const Square position(x,y);
      for (int p=PTYPEO_MIN; p<=PTYPEO_MAX; ++p)
      {
	const PtypeO ptypeo = static_cast<PtypeO>(p);
	if (! isPiece(ptypeo))
	  continue;
	const unsigned int index = indexOf(position, ptypeo);
	values[indexOf(index, index)] += Ptype_Eval_Table.value(ptypeo);
      }
    }
  }

  // for gold, silver
  static const CArray<Ptype,2> gold_silver = {{ GOLD, SILVER }};
  for (size_t i=0; i<gold_silver.size(); ++i) {
    const Ptype ptype = gold_silver[i];
    
    for (int y=7; y<=9; ++y) {
      Square right(1,y), left(9,y);
      unsigned int index_r = indexOf(right, newPtypeO(BLACK, ptype));
      unsigned int index_l = indexOf(left,  newPtypeO(BLACK, ptype));
      values[selfIndexOf(index_r)] = values[selfIndexOf(index_r)]*4/5;
      values[selfIndexOf(index_l)] = values[selfIndexOf(index_l)]*4/5;

      right = right.rotate180();
      left = left.rotate180();
      index_r = indexOf(right, newPtypeO(WHITE, ptype));
      index_l = indexOf(left,  newPtypeO(WHITE, ptype));
      values[selfIndexOf(index_r)] = values[selfIndexOf(index_r)]*4/5;
      values[selfIndexOf(index_l)] = values[selfIndexOf(index_l)]*4/5;    
    }
    for (int x=1; x<=9; ++x) {
      Square top(x,1);
      unsigned int index = indexOf(top, newPtypeO(BLACK, ptype));
      values[selfIndexOf(index)] = values[selfIndexOf(index)] * 2/3;

      top = top.rotate180();
      index = indexOf(top, newPtypeO(WHITE, ptype));
      values[selfIndexOf(index)] = values[selfIndexOf(index)] * 2/3;      
    }
  }
  for (int y=1; y<=6; ++y) {
    for (int x=1; x<=9; ++x) {
      Square out(x,y);
      unsigned int index = indexOf(out, newPtypeO(BLACK, GOLD));
      values[selfIndexOf(index)] = values[selfIndexOf(index)] * 6/7;

      out = out.rotate180();
      index = indexOf(out, newPtypeO(WHITE, GOLD));
      values[selfIndexOf(index)] = values[selfIndexOf(index)] * 6/7;
    }
  }
  for (int y=7; y<=9; ++y) {
    Square right(2,y), left(8,y);
    unsigned int index_r = indexOf(right, newPtypeO(BLACK, GOLD));
    unsigned int index_l = indexOf(left,  newPtypeO(BLACK, GOLD));
    values[selfIndexOf(index_r)] = values[selfIndexOf(index_r)]*7/8;
    values[selfIndexOf(index_l)] = values[selfIndexOf(index_l)]*7/8;

    right = right.rotate180();
    left = left.rotate180();
    index_r = indexOf(right, newPtypeO(WHITE, GOLD));
    index_l = indexOf(left,  newPtypeO(WHITE, GOLD));
    values[selfIndexOf(index_r)] = values[selfIndexOf(index_r)]*7/8;
    values[selfIndexOf(index_l)] = values[selfIndexOf(index_l)]*7/8;
  }

  // undo piece values
  for (int y=1; y<=9; ++y)
  {
    for (int x=1; x<=9; ++x)
    {
      const Square position(x,y);
      for (int p=PTYPEO_MIN; p<=PTYPEO_MAX; ++p)
      {
	const PtypeO ptypeo = static_cast<PtypeO>(p);
	if (! isPiece(ptypeo))
	  continue;
	const unsigned int index = indexOf(position, ptypeo);
	values[indexOf(index, index)] -= Ptype_Eval_Table.value(ptypeo);
      }
    }
  }

  return true;
}

void osl::eval::ppair::
PiecePairPieceEval::resetWeights(const int *w)
{
  CArray<int, PTYPE_SIZE> values;
  std::copy(w, w+(int)PTYPE_SIZE, values.begin());
  PiecePairPieceTable::Piece_Value.reset(values);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
