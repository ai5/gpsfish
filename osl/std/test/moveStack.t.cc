/* moveStackTest.cc
 */
#include "osl/container/moveStack.h"
#include <boost/test/unit_test.hpp>
#include <set>

using namespace osl;

BOOST_AUTO_TEST_CASE(MoveStackTestPush)
{
  MoveStack s;
  BOOST_CHECK_EQUAL(static_cast<size_t>(0u), s.size());
  BOOST_CHECK_EQUAL(false, s.hasLastMove());
  BOOST_CHECK_EQUAL(false, s.hasLastMove(1));
  BOOST_CHECK_EQUAL(false, s.hasLastMove(2));
  BOOST_CHECK_EQUAL(Move::INVALID(), s.lastMove());
  
  const Move move71rook = 
    Move(Square::STAND(),Square(7,1),ROOK,PTYPE_EMPTY,false,BLACK);
  s.push(move71rook);

  BOOST_CHECK_EQUAL(static_cast<size_t>(1u), s.size());
  BOOST_CHECK_EQUAL(true, s.hasLastMove());
  BOOST_CHECK_EQUAL(true, s.hasLastMove(1));
  BOOST_CHECK_EQUAL(false, s.hasLastMove(2));
  BOOST_CHECK_EQUAL(move71rook, s.lastMove());
  BOOST_CHECK_EQUAL(move71rook, s.lastMove(1));

  s.pop();

  BOOST_CHECK_EQUAL(static_cast<size_t>(0u), s.size());
  BOOST_CHECK_EQUAL(false, s.hasLastMove());
  BOOST_CHECK_EQUAL(false, s.hasLastMove(1));
  BOOST_CHECK_EQUAL(false, s.hasLastMove(2));
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
