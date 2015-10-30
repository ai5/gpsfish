#include "osl/search/sacrificeCheck.h"
#include "osl/search/searchState2.h"
#include <boost/test/unit_test.hpp>
using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(SacrificeCheckTestCount2)
{
  MoveStack history;
  RecordStack2 record_stack;
  BOOST_CHECK_EQUAL(0, SacrificeCheck::count2(record_stack, history, 100));
  SimpleHashRecord *record0 = new SimpleHashRecord();
  record0->setInCheck(true);
  record_stack.push(record0);
  history.push(Move(Square(5,5),SILVER,BLACK));

  BOOST_CHECK_EQUAL(0, SacrificeCheck::count2(record_stack, history, 100));

  SimpleHashRecord *record1 = new SimpleHashRecord();
  record_stack.push(record1);
  history.push(Move(Square(5,4),Square(5,5),KING,SILVER,false,WHITE));

  BOOST_CHECK_EQUAL(1, SacrificeCheck::count2(record_stack, history, 100));

  SimpleHashRecord *record2 = new SimpleHashRecord();
  record2->setInCheck(true);
  record_stack.push(record2);
  history.push(Move(Square(5,6),GOLD,BLACK));

  BOOST_CHECK_EQUAL(0, SacrificeCheck::count2(record_stack, history, 100));

  SimpleHashRecord *record3 = new SimpleHashRecord();
  record_stack.push(record3);
  history.push(Move(Square(5,5),Square(5,6),KING,GOLD,false,WHITE));

  BOOST_CHECK_EQUAL(2, SacrificeCheck::count2(record_stack, history, 100));
  BOOST_CHECK_EQUAL(1, SacrificeCheck::count2(record_stack, history, 2));

  history.pop();
  history.push(Move(Square(5,5),Square(4,4),KING,PTYPE_EMPTY,false,WHITE));

  BOOST_CHECK_EQUAL(0, SacrificeCheck::count2(record_stack, history, 100));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
