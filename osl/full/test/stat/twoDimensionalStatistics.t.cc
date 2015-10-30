#include "osl/stat/twoDimensionalStatistics.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::stat;

BOOST_AUTO_TEST_CASE(TwoDimensionalStatisticsTestCorrelation)
{
    TwoDimensionalStatistics t;
    t.add(0,0);
    t.add(1,1);
    BOOST_CHECK_EQUAL(1.0, t.correlation());
    t.add(1,2);
    BOOST_CHECK(t.correlation() < 1.0);
    BOOST_CHECK(0.0 < t.correlation());
}

BOOST_AUTO_TEST_CASE(TwoDimensionalStatisticsTestFitting)
{
    {
	TwoDimensionalStatistics t;
	t.add(0,0);
	t.add(1,1);
	double a, b, res;
	t.fitting(a, b, res);
	BOOST_CHECK_EQUAL(1.0, a);
	BOOST_CHECK_EQUAL(0.0, b);
	BOOST_CHECK_EQUAL(0.0, res);
    }
    {
	TwoDimensionalStatistics t;
	t.add(0,0);
	t.add(1,-1);
	double a, b, res;
	t.fitting(a, b, res);
	BOOST_CHECK_EQUAL(-1.0, a);
	BOOST_CHECK_EQUAL(0.0, b);
	BOOST_CHECK_EQUAL(0.0, res);
    }

    {
	TwoDimensionalStatistics t;
	t.add(0,1);
	t.add(1,2);
	double a, b, res;
	t.fitting(a, b, res);
	BOOST_CHECK_EQUAL(1.0, a);
	BOOST_CHECK_EQUAL(1.0, b);
	BOOST_CHECK_EQUAL(0.0, res);
    }
    {
	TwoDimensionalStatistics t;
	t.add(0,1);
	t.add(1,0);
	double a, b, res;
	t.fitting(a, b, res);
	BOOST_CHECK_EQUAL(-1.0, a);
	BOOST_CHECK_EQUAL(1.0, b);
	BOOST_CHECK_EQUAL(0.0, res);
    }

    {
	TwoDimensionalStatistics t;
	t.add(0,0);
	t.add(1,1);
	t.add(1,-1);
	double a, b, res;
	t.fitting(a, b, res);
	BOOST_CHECK_EQUAL(0.0, a);
	BOOST_CHECK_EQUAL(0.0, b);
	BOOST_CHECK(res > 0.0);
    }
}
