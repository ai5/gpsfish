/* king8.cc
 */
#include "osl/rating/feature/king8.h"

const std::string osl::rating::AttackKing8::name(Ptype self, Ptype target, bool same) 
{
  return std::string(Ptype_Table.getCsaName(self)) + "-" 
    + Ptype_Table.getCsaName(target) + (same ? "=" : "!");
}

const std::string osl::rating::DefenseKing8::name(Ptype self, bool drop, int danger) 
{
  return std::string(Ptype_Table.getCsaName(self)) + "-" 
    + (drop ? "d" : "m") + (char)('0' + danger);
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
