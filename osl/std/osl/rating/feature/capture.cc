/* capture.cc
 */
#include "osl/rating/feature/capture.h"
#include <sstream>

const std::string osl::rating::Capture::name(int first, int /*last*/) 
{
  std::ostringstream os;
  os << "(";
  if (first == -INF)
    os << "-inf";
  else
    os << first;
  os << " <= )";
  return os.str();
}

osl::rating::DropCaptured::DropCaptured(Ptype p)
  : Feature(Ptype_Table.getCsaName(p)), ptype(p)
{
}

/* ------------------------------------------------------------------------- */
