/* quiescenceRecord.cc
 */
#include "osl/search/quiescenceRecord.h"
#include "osl/csa.h"
#include <map>
#include <iostream>

#ifndef MINIMAL
void osl::search::
QuiescenceRecord::dump(std::ostream& os) const
{
  os << "QuiescenceRecord " << this << "\n";
  os << lower_bound << " (" << (int)lower_depth << ")"
     << upper_bound << " (" << (int)upper_depth << ")";
  if (hasStaticValue())
  {
    os << " s " << static_value << toString(staticValueType());
    os << " t1 " << threat1.value << " " << csa::show(threat1.move)
       << " t2 " << threat2.value << " " << csa::show(threat2.move);
  }
  os << "\n";
  os << "checkmate read " << checkmate_nodes << "\t"
     << "threatmate read " << threatmate_nodes << "\n";
  os << "best move " <<  csa::show(bestMove()) << "\n";
  os << "threatmate " <<  threatmate << "\n";
  os << "sendoffs " << (unsigned int)threatmate.sendoffs << "\n";
  os << "moves " << moves_size();
  size_t i=0;
  MoveVector moves_copy;
  loadMoves(moves_copy);
  for (MoveVector::const_iterator p=moves_copy.begin(); p!=moves_copy.end(); ++p, ++i)
  {
    os << " " << csa::show(*p);
    if (i % 8 == 7)
      os << "\n";
  }
  if (i % 8 != 7)
    os << "\n";
}

const char *osl::search::
QuiescenceRecord::toString(StaticValueType type)
{
  switch (type)
  {
  case UNKNOWN:
    return "?";
  case UPPER_BOUND:
    return ">";
  case EXACT:
    return "=";
  default:
    assert(0);
  }
  return "!";			// should not occur
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
