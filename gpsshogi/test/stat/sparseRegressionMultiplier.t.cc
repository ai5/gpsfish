#include "osl/stat/iterativeLinearSolver.h"
#include "osl/stat/sparseRegressionMultiplier.h"
#include "osl/stat/valarrayMatrixMultiplier.h"
#include "osl/stat/valarrayMatrixIO.h"
#include "osl/stat/diagonalPreconditioner.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <valarray>
#include <cassert>
#include <iostream>

using namespace osl;
using namespace osl::stat;

typedef std::valarray<double> valarray_t;
typedef ValarrayMatrix<double> matrix_t;
/**
 * テスト用掛け算クラス.
 */
class StringMultiplier : public SparseRegressionMultiplierSeq
{
  matrix_t matrix;
  mutable size_t cur;
  mutable bool next_instanace_is_end_flag;
public:
  StringMultiplier(const char *contents)
    : cur(0), next_instanace_is_end_flag(0)
  {
    std::istringstream is(contents);
    is >> matrix;
    init(matrix.ncols());
  }
  void 	newIteration () const
  {
  }
  void getVectorX(int& num_elements, unsigned int *non_zero_indices, 
		  double *non_zero_values) const
  {
    if (next_instanace_is_end_flag) {
      num_elements = -1;
      next_instanace_is_end_flag = false;
      return;
    }
    num_elements = dim();
    for (size_t i=0; i<num_elements; ++i)
    {
      non_zero_indices[i] = i;
      non_zero_values[i] = matrix(cur % matrix.nrows(), i);
    }
    if (++cur % matrix.nrows() == 0)
      next_instanace_is_end_flag = true;
  }
};


class SparseRegressionMultiplierTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(SparseRegressionMultiplierTest);
  CPPUNIT_TEST(testReader);
  CPPUNIT_TEST(testMakeVector);
  CPPUNIT_TEST(testInstance);
  CPPUNIT_TEST(testProd2);
  CPPUNIT_TEST(testProd);
  CPPUNIT_TEST(testCG);
  CPPUNIT_TEST_SUITE_END();
  static const int max_loop = 10;
  static const double eps = 0.01;
public:
  void testReader()
  {
    const double array[] = {1,2,3};
    DoubleArrayReader reader(array);
    CPPUNIT_ASSERT_EQUAL(1.0, reader.read());
    CPPUNIT_ASSERT_EQUAL(2.0, reader.read());
    CPPUNIT_ASSERT_EQUAL(3.0, reader.read());
  }
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
  void testInstance() 
  { 
    StringMultiplier  prodA("1 1 1.0");
  }
  void testProd(const char *X, const char *XTX,
		const char *y_str, const char *w_str)
  {
    valarray_t y = makeVector(y_str);
    valarray_t w = makeVector(w_str);
    ValarrayMatrixMultiplier m1(XTX);
    StringMultiplier m2(X);

    valarray_t r1(m1.dim());
    valarray_t r2(m2.dim());
    m1.prod(&w[0], &r1[0]);
    m2.prod(&w[0], &r2[0]);

    CPPUNIT_ASSERT_EQUAL(r1.size(), r2.size());
    for (size_t i=0; i<r1.size(); ++i)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(y[i], r1[i], eps);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(y[i], r2[i], eps);
    }
  }
  void testProd2()
  {
    const char *X = 
      " 2 2"
      " 1 0"
      " 0 1";
    const char *XTX = 
      " 2 2"
      " 1 0"
      " 0 1";
    testProd(X, XTX, " 2 2 3", " 2 2 3");
  }
  void testProd()
  {
    const char *X = 
      " 3 3"
      " 8 3 4"
      " 1 5 9"
      " 6 7 2";
    const char *XTX = 
      " 3 3"
      " 101 71 53"
      "  71 83 71"
      "  53 71 101";
    testProd(X, XTX, " 3 402 450 498", " 3 1 2 3");
  }
  void testCG(const char *x_str, const char *y_str, const char *w_str)
  {
    StringMultiplier prodA(x_str);

    valarray_t y = makeVector(y_str);
    const valarray_t w = makeVector(w_str);
    const size_t dim = prodA.dim();

    valarray_t xty(y);
    valarray_t diag(y);
    prodA.computeXtY(&y[0], &xty[0], &diag[0]);
    // std::cerr << xty;
    DiagonalPreconditioner preconditioner(dim);
    preconditioner.setInverseDiagonals(&diag[0]);
    
    int iter;
    double tol;
    {
      // CG
      IterativeLinearSolver solver(prodA, 0, max_loop, 0.001);
      valarray_t result(0.0, dim);
      const int err = solver.solve_by_CG(xty, result, &iter, &tol, false);
      CPPUNIT_ASSERT_EQUAL(0, err);
      for (size_t i=0; i<dim; ++i)
	CPPUNIT_ASSERT_DOUBLES_EQUAL(w[i], result[i], eps);
    }
    {
      // CG + preconditioner
      IterativeLinearSolver solver(prodA, &preconditioner, max_loop, 0.001);
      valarray_t result(0.0, dim);
      const int err = solver.solve_by_CG(xty, result, &iter, &tol, false);
      CPPUNIT_ASSERT_EQUAL(0, err);
      for (size_t i=0; i<dim; ++i)
	CPPUNIT_ASSERT_DOUBLES_EQUAL(w[i], result[i], eps);
    }
    {
      // BiCGSTAB
      IterativeLinearSolver solver(prodA, 0, max_loop, 0.001);
      valarray_t result(0.0, dim);
      const int err_bicgstab
	= solver.solve_by_BiCGSTAB(xty, result, &iter, &tol, false);  
      CPPUNIT_ASSERT_EQUAL(0, err_bicgstab);
      for (size_t i=0; i<dim; ++i)
	CPPUNIT_ASSERT_DOUBLES_EQUAL(w[i], result[i], eps);
    }
    {
      // BiCGSTAB + preconditioner
      IterativeLinearSolver solver(prodA, &preconditioner, max_loop, 0.001);
      valarray_t result(0.0, dim);
      const int err_bicgstab
	= solver.solve_by_BiCGSTAB(xty, result, &iter, &tol, false);  
      CPPUNIT_ASSERT_EQUAL(0, err_bicgstab);
      for (size_t i=0; i<dim; ++i)
	CPPUNIT_ASSERT_DOUBLES_EQUAL(w[i], result[i], eps);
    }
  }
  void testCG()
  {
    testCG("3 3"
	   " 8 3 4"
	   " 1 5 9"
	   " 6 7 2",
	   " 3 26 38 26",
	   " 3 1 2 3");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SparseRegressionMultiplierTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
