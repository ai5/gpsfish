/* pawnCheckmateMoves.t.cc
 */
#include "osl/checkmate/pawnCheckmateMoves.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(PawnCheckmateMovesTestPair)
{
  const Square from = Square(2,4);
  const Square to = Square(2,3);
  
  const Move p = Move(from, to, PPAWN, PTYPE_EMPTY, true, BLACK);
  const Move np = Move(from, to, PAWN, PTYPE_EMPTY, false, BLACK);
  BOOST_CHECK_EQUAL(np, p.unpromote());
  BOOST_CHECK(PawnCheckmateMoves::hasParingNoPromote(p));
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
