/* quiescenceRecord.t.cc
 */
#include "osl/search/quiescenceRecord.h"
#include <boost/test/unit_test.hpp>


using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(QuiescenceRecordTestBestMoves)
{
  QuiescenceRecord r;
  BOOST_CHECK_EQUAL(Move(), r.bestMove());

  BestMoves best_moves;
  BOOST_CHECK(best_moves.sizeFilled() == 0);
  
  MoveVector moves;
  for (int x=1; x<=9; ++x)
    moves.push_back(Move(Square(x,5), GOLD, BLACK));

  best_moves.add(moves[0]);
  BOOST_CHECK_EQUAL(moves[0], best_moves[0]);
  
  best_moves.add(moves[1]);
  BOOST_CHECK_EQUAL(moves[1], best_moves[0]);
  BOOST_CHECK_EQUAL(moves[0], best_moves[1]);

  MoveVector test;
  test.push_back(moves[2]);
  best_moves.addSecondary(test);
  
  BOOST_CHECK_EQUAL(moves[1], best_moves[0]); // best move remains
  BOOST_CHECK_EQUAL(moves[2], best_moves[1]); // inserted here
  BOOST_CHECK_EQUAL(moves[0], best_moves[2]);

  test.push_back(moves[3]);
  test.push_back(moves[4]);
  best_moves.addSecondary(test);

  BOOST_CHECK_EQUAL((size_t)4, best_moves.size()); // best move remains
  BOOST_CHECK_EQUAL(moves[1], best_moves[0]); // best move remains
  BOOST_CHECK_EQUAL(moves[2], best_moves[1]); // same contents stored
  BOOST_CHECK_EQUAL(moves[3], best_moves[2]);
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
