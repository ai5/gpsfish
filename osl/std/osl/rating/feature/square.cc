/* square.cc
 */
#include "osl/rating/feature/square.h"
#include "osl/bits/ptypeTable.h"
#include <sstream>

const std::string osl::rating::
RelativeKingX::name(int x, int old_x, bool /*attack*/, Ptype ptype)
{
  std::ostringstream os;
  os << "X";
  if (old_x == 9)
      os << "d";
  else 
      os << old_x << "->";
  os << x << Ptype_Table.getCsaName(ptype);
  return os.str();
}

const std::string osl::rating::
RelativeKingY::name(int y, int old_y, bool /*attack*/, Ptype ptype)
{
  std::ostringstream os;
  os << "Y";
  if (old_y == 9)
      os << "d";
  else 
      os << old_y << "->";
  os << y << Ptype_Table.getCsaName(ptype);
  return os.str();
}

const std::string osl::rating::SquareX::name(int x) 
{
  std::ostringstream os;
  os << "PX" << x << "-";
  return os.str();
}
const std::string osl::rating::SquareY::name(int y) 
{
  std::ostringstream os;
  os << "PY" << y << "-";
  return os.str();
}

/* ------------------------------------------------------------------------- */
