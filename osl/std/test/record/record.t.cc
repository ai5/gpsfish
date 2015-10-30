#include "osl/record/record.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
using namespace osl;

BOOST_AUTO_TEST_CASE(RecordTestShow){
  Record r;
  if (OslConfig::verbose())
  {
    std::cout << r << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(RecordTestParseDate)
{
  using namespace boost::gregorian;
  {
    Record r;
    r.setDate("2010/11/13");
    BOOST_CHECK(date(2010, Nov, 13) == r.start_date);
  }
  {
    Record r;
    r.setDate("2010/1/1");
    BOOST_CHECK(date(2010, Jan, 1) == r.start_date);
  }
  {
    Record r;
    r.setDate("2010/01/02");
    BOOST_CHECK(date(2010, Jan, 2) == r.start_date);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
