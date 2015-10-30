#include "osl/bits/quadInt.h"
#include <boost/test/unit_test.hpp>
using namespace osl;

BOOST_AUTO_TEST_CASE(QuadIntTestAdd){
  QuadInt a, b;
  BOOST_CHECK_EQUAL(a[0], 0);
  BOOST_CHECK_EQUAL(a[1], 0);
  BOOST_CHECK_EQUAL(a[2], 0);
  BOOST_CHECK_EQUAL(b[0], 0);
  BOOST_CHECK_EQUAL(b[1], 0);
  BOOST_CHECK_EQUAL(b[2], 0);

  a[0] = 1; a[1] = 2; a[2] = 3;
  b[0] = 7; b[1] = 16; b[2] = 27;
  
  QuadInt c = a + b, d = a - b;
  BOOST_CHECK_EQUAL(c[0], 8);
  BOOST_CHECK_EQUAL(c[1], 18);
  BOOST_CHECK_EQUAL(c[2], 30);

  BOOST_CHECK_EQUAL(d[0], -6);
  BOOST_CHECK_EQUAL(d[1], -14);
  BOOST_CHECK_EQUAL(d[2], -24);
}

BOOST_AUTO_TEST_CASE(QuadIntTestMul){
  QuadInt a, b;
  BOOST_CHECK_EQUAL(a[0], 0);
  BOOST_CHECK_EQUAL(a[1], 0);
  BOOST_CHECK_EQUAL(a[2], 0);
  BOOST_CHECK_EQUAL(b[0], 0);
  BOOST_CHECK_EQUAL(b[1], 0);
  BOOST_CHECK_EQUAL(b[2], 0);

  a[0] = 1; a[1] = 2; a[2] = 3;
  b[0] = 7; b[1] = 16; b[2] = 27;
  
  QuadInt a7 = a * 7, a_13 = a * -13;
  QuadInt b1 = b * 1, b_1 = b * -1, b0 = b * 0;
  BOOST_CHECK_EQUAL(a7[0], 7);
  BOOST_CHECK_EQUAL(a7[1], 14);
  BOOST_CHECK_EQUAL(a7[2], 21);

  BOOST_CHECK_EQUAL(a_13[0], -13);
  BOOST_CHECK_EQUAL(a_13[1], -26);
  BOOST_CHECK_EQUAL(a_13[2], -39);

  BOOST_CHECK_EQUAL(b1[0], 7);
  BOOST_CHECK_EQUAL(b1[1], 16);
  BOOST_CHECK_EQUAL(b1[2], 27);

  BOOST_CHECK_EQUAL(b_1[0], -7);
  BOOST_CHECK_EQUAL(b_1[1], -16);
  BOOST_CHECK_EQUAL(b_1[2], -27);

  BOOST_CHECK_EQUAL(b0[0], 0);
  BOOST_CHECK_EQUAL(b0[1], 0);
  BOOST_CHECK_EQUAL(b0[2], 0);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
