#include "osl/book/bookInMemory.h"
#include "osl/numEffectState.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;

BOOST_AUTO_TEST_CASE(BookInMemoryTestInstance)
{
  const BookInMemory& book = BookInMemory::instance();
  BOOST_CHECK(book.size() < 100000); // do not consume too much memory
}

BOOST_AUTO_TEST_CASE(BookInMemoryTestFind)
{
  const BookInMemory& book = BookInMemory::instance();
  NumEffectState initial;
  HashKey key(initial);
  MoveVector moves;
  book.find(key, moves);
  BOOST_CHECK(! moves.empty());
  const Move m76fu(Square(7,7), Square(7,6), PAWN, PTYPE_EMPTY, false, BLACK);
  BOOST_CHECK_EQUAL(m76fu, moves[0]);
  
  moves.clear();
  book.find(key.newMakeMove(m76fu), moves);
  BOOST_CHECK(! moves.empty());
  const Move m34fu(Square(3,3), Square(3,4), PAWN, PTYPE_EMPTY, false, WHITE);
  BOOST_CHECK_EQUAL(m34fu, moves[0]);
}

// ;;; local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
