#include "pvVector.h"
#include "osl/numEffectState.h"
#include "osl/eval/ptypeEval.h"
#include <iostream>

int gpsshogi::PVVector::pieceValue() const
{
  static const osl::NumEffectState dummy;
  int sum = 0;
  for (osl::Move move: *this)
    if (move.isNormal())
      sum += osl::eval::Ptype_Eval_Table.diffWithMove(dummy, move);
  return sum;
}

int gpsshogi::PVVector::pieceValueOfTurn() const
{
  if (empty())
    return 0;
  return pieceValue() * sign(front().player());
}

std::ostream& gpsshogi::operator<<(std::ostream& os, const PVVector& moves)
{
  os<< "PVVector" << std::endl;
  for (osl::Move m: moves) {
    os << m << std::endl;
  }
  return os<<std::endl;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
