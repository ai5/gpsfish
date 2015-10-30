/* valarrayMatrix.t.cc
 */
#include "osl/stat/valarrayMatrix.h"
#include "osl/stat/valarrayMatrixIO.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>

using namespace osl;
using namespace osl::stat;

typedef std::valarray<double> valarray_t;
typedef ValarrayMatrix<double> matrix_t;

class ValarrayMatrixTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(ValarrayMatrixTest);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testCol);
  CPPUNIT_TEST(testAssign);
  CPPUNIT_TEST(testProd);
  CPPUNIT_TEST(testProdVector);
  CPPUNIT_TEST(testTranspose);
  CPPUNIT_TEST_SUITE_END();
public:
  std::string toString(const matrix_t& m)
  {
    std::ostringstream os;
    os << m;
    return os.str();
  }
  void testCreation() 
  {
    matrix_t m(3,3); 
    const std::string expected =
      "3 3\n"
      "0 0 0 \n"
      "0 0 0 \n"
      "0 0 0 \n";
    CPPUNIT_ASSERT_EQUAL(expected, toString(m));
  }
  void testCol() 
  {
    matrix_t m(3,3); 
    m.array()[m.col(1)] = -1;
    const std::string expected =
      "3 3\n"
      "0 -1 0 \n"
      "0 -1 0 \n"
      "0 -1 0 \n";
    CPPUNIT_ASSERT_EQUAL(expected, toString(m));
  }
  void testAssign() 
  {
    matrix_t m(3,3); 
    m(0,0) = 2;
    m(1,0) = 3;

    const std::string expected =
      "3 3\n"
      "2 0 0 \n"
      "3 0 0 \n"
      "0 0 0 \n";
    CPPUNIT_ASSERT_EQUAL(expected, toString(m));
  }
  void testProd()
  {
    std::istringstream is("3 3  1 2 3  4 5 6  7 8 9");
    matrix_t m;
    is >> m;
    const matrix_t m2 = m*m;
    
    const std::string expected =
      "3 3\n"
      "30 36 42 \n"
      "66 81 96 \n"
      "102 126 150 \n";
    CPPUNIT_ASSERT_EQUAL(expected, toString(m2));
  }
  void testProdVector()
  {
    std::istringstream is("3 3  1 2 3  4 5 6  7 8 9" " 3 1 2 3");
    matrix_t m;
    is >> m;
    valarray_t x;
    is >> x;

    const valarray_t y = m*x;
    CPPUNIT_ASSERT_EQUAL(14.0, y[0]);
    CPPUNIT_ASSERT_EQUAL(32.0, y[1]);
    CPPUNIT_ASSERT_EQUAL(50.0, y[2]);
  }
  void testTranspose()
  {
    std::istringstream is("3 3  1 2 3  4 5 6  7 8 9" " 3 1 2 3");
    matrix_t m;
    is >> m;
    
    matrix_t mt = m.transpose();
    const std::string expected =
      "3 3\n"
      "1 4 7 \n"
      "2 5 8 \n"
      "3 6 9 \n";
    CPPUNIT_ASSERT_EQUAL(expected, toString(mt));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValarrayMatrixTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
