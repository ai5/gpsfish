#include "osl/hash/hashRandom.h"
#include "osl/stat/variance.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace osl;
using namespace osl::hash;

BOOST_AUTO_TEST_CASE(HashRandomTestSetUp)
{
    double sigma = 10.0;
    HashRandom::setUp(sigma);
    stat::Variance var;
    
    for (size_t i=0; i<HashRandom::Length; ++i)
	var.add(HashRandom::value(i));
    BOOST_CHECK_SMALL(var.average(), 0.25);
    BOOST_CHECK_CLOSE(sigma, sqrt(var.variance()), 20);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
