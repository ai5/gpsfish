/* categoryMoveVector.cc
 */
#include "osl/search/analyzer/categoryMoveVector.h"

osl::search::analyzer::
CategoryMoves::CategoryMoves(const MoveLogProbVector& v, const std::string& n)
  : moves(v), category(n)
{
}
osl::search::analyzer::
CategoryMoves::~CategoryMoves()
{
}

osl::search::analyzer::
CategoryMoveVector::CategoryMoveVector()
{
}

osl::search::analyzer::
CategoryMoveVector::~CategoryMoveVector()
{
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
