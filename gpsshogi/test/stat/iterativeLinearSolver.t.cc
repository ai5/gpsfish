#include "osl/stat/iterativeLinearSolver.h"
#include "osl/stat/valarrayMatrixMultiplier.h"
#include "osl/stat/diagonalPreconditioner.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cassert>

using namespace osl;
using namespace osl::stat;

typedef std::valarray<double> valarray_t;
typedef ValarrayMatrix<double> matrix_t;

class IterativeLinearSolverTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(IterativeLinearSolverTest);
  CPPUNIT_TEST(testMakeVector);
  CPPUNIT_TEST(testCG);
  CPPUNIT_TEST_SUITE_END();
  static const int max_loop = 10;
  static const double eps = 0.01;
public:
  static valarray_t makeVector(const char *Y)
  {
    size_t length;
    std::istringstream is(Y);
    is >> length;
    valarray_t result(length);
    for (size_t i=0; i<length; ++i)
      is >> result[i];
    assert(is);
    return result;
  }
  void testMakeVector() 
  { 
    const valarray_t y = makeVector("3 1 2 3");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, y[0], eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, y[1], eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, y[2], eps);
  }
  void testCG(const char *x_str, const char *y_str, const char *w_str)
  {
    ValarrayMatrixMultiplier prodA(x_str);

    const valarray_t y = makeVector(y_str);
    const valarray_t w = makeVector(w_str);

    const size_t dim = prodA.dim();
    
    IterativeLinearSolver solver(prodA, 0, max_loop, 0.001);
    int iter;
    double tol;
    valarray_t result(0.0, dim);
    const int err = solver.solve_by_CG(y, result, &iter, &tol, false);
    CPPUNIT_ASSERT_EQUAL(0, err);
    for (size_t i=0; i<dim; ++i)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(w[i], result[i], eps);

    result = 0.0;
    const int err_bicgstab
      = solver.solve_by_BiCGSTAB(y, result, &iter, &tol, false);  
    CPPUNIT_ASSERT_EQUAL(0, err_bicgstab);
    for (size_t i=0; i<dim; ++i)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(w[i], result[i], eps);
  }
  void testCG()
  {
    testCG(" 3 3"
	   " 13  4 -1"
	   "  4 15  1"
	   " -1  1 14",
	   " 3 29 28 11",
	   "3 1.892 1.307 0.828");
    testCG(" 3 3"
	   " 101 71 53"
	   "  71 83 71"
	   "  53 71 101",
	   " 3 402 450 498",
	   " 3 1 2 3");
    testCG(" 4 4"
	   " 101 71 53  0"
	   "  71 83 71  0"
	   "  53 71 101 0"
	   "   0  0  0  0",
	   " 4 402 450 498 0",
	   " 4 1 2 3 0");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(IterativeLinearSolverTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
