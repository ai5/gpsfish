#include "osl/eval/ptypeEval.h"
osl::eval::PtypeEvalTable::PtypeEvalTable()
{ 
  init();
}
void osl::eval::PtypeEvalTable::init()
{ 
  const CArray<int, PTYPE_SIZE> values = {
      0, 0,
      PtypeEvalTraits<PPAWN>::val, PtypeEvalTraits<PLANCE>::val,
      PtypeEvalTraits<PKNIGHT>::val, PtypeEvalTraits<PSILVER>::val,
      PtypeEvalTraits<PBISHOP>::val, PtypeEvalTraits<PROOK>::val,
      //
      PtypeEvalTraits<KING>::val, PtypeEvalTraits<GOLD>::val,
      //
      PtypeEvalTraits<PAWN>::val, PtypeEvalTraits<LANCE>::val,
      PtypeEvalTraits<KNIGHT>::val, PtypeEvalTraits<SILVER>::val, 
      PtypeEvalTraits<BISHOP>::val, PtypeEvalTraits<ROOK>::val,
    };
  reset(values);
}
osl::eval::PtypeEvalTable::~PtypeEvalTable()
{ 
}

void osl::eval::PtypeEvalTable::reset(const CArray<int,PTYPE_SIZE>& values)
{
  ptypeO2Val.fill(0);
  std::copy(values.begin(), values.end(), ptypeO2Val.begin()+16);
  for(int i=PTYPE_MIN;i<=PTYPE_MAX;i++)
  {
    Ptype ptype=static_cast<Ptype>(i);
    ptypeO2Val[newPtypeO(WHITE,ptype)-PTYPEO_MIN]=
      -ptypeO2Val[newPtypeO(BLACK,ptype)-PTYPEO_MIN];
  }
  for(int i=PTYPEO_MIN;i<=PTYPEO_MAX;i++)
  {
    PtypeO ptypeO=static_cast<PtypeO>(i);
    PtypeO basicPtypeO=unpromote(ptypeO);
    // note: value() depends on ptypeO2Val
    promoteVal[i-PTYPEO_MIN]=this->value(ptypeO)-this->value(basicPtypeO);
  }
  // EMPTYのcapture
  captureVal[0]=0;
  for(int i=PTYPEO_MIN;i<=PTYPEO_MAX;i++)
  {
    PtypeO ptypeO=static_cast<PtypeO>(i);
    // note: value() depends on ptypeO2Val
    if(isPiece(ptypeO))
      captureVal[i-PTYPEO_MIN]=this->value(captured(ptypeO))-
	this->value(ptypeO);
    else
      captureVal[i-PTYPEO_MIN]=0;
  }
}



/* ------------------------------------------------------------------------- */
osl::CArray<int, osl::PTYPEO_SIZE> osl::eval::ml::PieceEval::table;

void osl::eval::ml::
PieceEval::setUp(const Weights &weights)
{
  table.fill(0);
  // WHITE 0-15, BLACK 16-31
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i - (size_t)PTYPEO_MIN] = weights.value(i);
    table[i] = -weights.value(i);
  }
  table[newPtypeO(BLACK,KING)-PTYPEO_MIN] = osl::eval::Ptype_Eval_Table.value(KING);
  table[newPtypeO(WHITE,KING)-PTYPEO_MIN] = -osl::eval::Ptype_Eval_Table.value(KING);
}

int osl::eval::ml::
PieceEval::eval(const NumEffectState &state)
{
  int value = 0;
  for (int i = 0; i < Piece::SIZE; ++i)
  {
    const Piece piece = state.pieceOf(i);
    value += table[piece.ptypeO() - PTYPEO_MIN];
  }
  return value;
}

int osl::eval::ml::
PieceEval::value(PtypeO ptypeO)
{
  return table[ptypeO - PTYPEO_MIN];
}
/* ------------------------------------------------------------------------- */
