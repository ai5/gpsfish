/* simpleHashRecord.cc
 */
#include "osl/search/simpleHashRecord.h"
#include "osl/csa.h"
#include <map>
#include <iostream>
#include <iomanip>

#ifndef MINIMAL
void osl::search::
SimpleHashRecord::dump(std::ostream& os) const
{
  os << "SimpleHashRecord " << this 
     << " node_count " << nodeCount() << "\n";
  os << "best move " << csa::show(best_move.move())
	    << " " << best_move.logProb()
     << "\t";
  os << "limit: l " << lower_limit << " u " << upper_limit << "\n";
  os << "in_check " << inCheck() << "\n";
  if (hasLowerBound(0))
    os <<  lowerBound();
  else 
    os << "*";
  os << " < ";
  if (hasUpperBound(0))
    os <<  upperBound();
  else 
    os << "*";
  os << "\n";
  qrecord.dump(os);
}


#endif

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
