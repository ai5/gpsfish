#include "osl/misc/math.h"
#include <boost/test/unit_test.hpp>
#include <vector>

using namespace osl;

BOOST_AUTO_TEST_CASE(MathTestNthPower) {
  BOOST_CHECK_EQUAL(1, (osl::misc::nthPower<0>(3)));
  BOOST_CHECK_EQUAL(3, (osl::misc::nthPower<1>(3)));
  BOOST_CHECK_EQUAL(3*3, (osl::misc::nthPower<2>(3)));
  BOOST_CHECK_EQUAL(3*3*3*3*3, (osl::misc::nthPower<5>(3)));
}

BOOST_AUTO_TEST_CASE(MathTestComputeStdDev) {
  typedef std::vector<double> data_t;
  data_t data;
  data.push_back(154.0);
  data.push_back(150.0);
  data.push_back(140.0);
  data_t::const_iterator first = data.begin();
  data_t::const_iterator last  = data.end();
  const size_t cnt  = distance(first, last);
  const double sum  = accumulate(first, last, double());
  const double mean = sum / cnt;
  const double result = misc::computeStdDev(data.begin(), data.end(), mean);
  BOOST_CHECK_CLOSE(5.89, result, 1);
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
