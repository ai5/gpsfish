/* killerMoveTable.cc
 */
#include "osl/search/killerMoveTable.h"

osl::search::
KillerMoveTable::KillerMoveTable()
{
  clear();
}

osl::search::
KillerMoveTable::~KillerMoveTable()
{
}

void osl::search::
KillerMoveTable::clear()
{
  for (LRUMoves& l: killer_moves)
    l.clear();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
