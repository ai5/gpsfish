#include "osl/search/simpleHashRecord.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(SimpleHashRecordTestSize)
{
  BOOST_CHECK(sizeof(SimpleHashRecord) <= 256);
}
BOOST_AUTO_TEST_CASE(SimpleHashRecordTestAbsoluteBound)
{
  search::SimpleHashRecord record;
  const int limit = 100;
  const int value = 300;
  BOOST_CHECK_EQUAL(false, record.hasLowerBound(limit));
  BOOST_CHECK_EQUAL(false, record.hasUpperBound(limit));

  record.setLowerBound(BLACK, limit, MoveLogProb(), value);
  BOOST_CHECK_EQUAL(true, record.hasLowerBound(limit));
  BOOST_CHECK_EQUAL(false, record.hasUpperBound(limit));
    
  BOOST_CHECK_EQUAL(true, record.hasAbsoluteLowerBound(BLACK, limit));
  BOOST_CHECK_EQUAL(false, record.hasAbsoluteLowerBound(WHITE, limit));
  BOOST_CHECK_EQUAL(false, record.hasAbsoluteUpperBound(BLACK, limit));
  BOOST_CHECK_EQUAL(true, record.hasAbsoluteUpperBound(WHITE, limit));

  BOOST_CHECK_EQUAL(value, record.absoluteLowerBound(BLACK));
  BOOST_CHECK_EQUAL(value, record.absoluteUpperBound(WHITE));

  record = SimpleHashRecord();
  record.setUpperBound(BLACK, 100, MoveLogProb(), -value);
  BOOST_CHECK_EQUAL(false, record.hasAbsoluteLowerBound(BLACK, limit));
  BOOST_CHECK_EQUAL(true, record.hasAbsoluteLowerBound(WHITE, limit));
  BOOST_CHECK_EQUAL(true, record.hasAbsoluteUpperBound(BLACK, limit));
  BOOST_CHECK_EQUAL(false, record.hasAbsoluteUpperBound(WHITE, limit));

  BOOST_CHECK_EQUAL(-value, record.absoluteLowerBound(WHITE));
  BOOST_CHECK_EQUAL(-value, record.absoluteUpperBound(BLACK));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
