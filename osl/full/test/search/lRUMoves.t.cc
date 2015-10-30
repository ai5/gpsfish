#include "osl/search/lRUMoves.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::search;
BOOST_AUTO_TEST_CASE(LRUMovesTestSetMove)
{
  LRUMoves moves;
  BOOST_CHECK_EQUAL(Move::INVALID(), moves[0]);
  BOOST_CHECK_EQUAL(Move::INVALID(), moves[1]);

  const Move m24ki(Square(2,4), GOLD, BLACK);
  moves.setMove(m24ki);
  BOOST_CHECK_EQUAL(m24ki, moves[0]);
  BOOST_CHECK_EQUAL(Move::INVALID(), moves[1]);

  const Move m24fu(Square(2,4), PAWN, BLACK);
  moves.setMove(m24fu);
  BOOST_CHECK_EQUAL(m24fu, moves[0]);
  BOOST_CHECK_EQUAL(m24ki, moves[1]);

  moves.setMove(m24fu);
  BOOST_CHECK_EQUAL(m24fu, moves[0]);
  BOOST_CHECK_EQUAL(m24ki, moves[1]);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
