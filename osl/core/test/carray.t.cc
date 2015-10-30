#include "osl/container.h"
#include <boost/test/unit_test.hpp>
using namespace osl;

BOOST_AUTO_TEST_CASE(CArrayTestPlayerAccess)
{
  CArray<int, 2> array = {{ 100, 200 }};
  BOOST_CHECK_EQUAL(100, array[BLACK]);
  BOOST_CHECK_EQUAL(200, array[WHITE]);
}

BOOST_AUTO_TEST_CASE(CArrayTestCallConstructor)
{
  CArray<int,4> a;
  BOOST_CHECK_EQUAL(4, a.size());
}

BOOST_AUTO_TEST_CASE(CArray2dTestExhaustive)
{
  const size_t c1 = 4, c2 = 3;
  CArray2d<int,c1,c2> a;
  {
    int c = 0;
    for (size_t i=0; i<c1; ++i)
    {
      for (size_t j=0; j<c2; ++j)
      {
	a[i][j] = c++;
      }
    }
  }
  const CArray2d<int,c1,c2>& c = a;
  for (size_t i=0; i<c1; ++i)
  {
    for (size_t j=0; j<c2; ++j)
    {
      for (size_t i2=0; i2<c1; ++i2)
      {
	for (size_t j2=0; j2<c2; ++j2)
	{
	  if ((i==i2) && (j == j2))
	  {
	    BOOST_CHECK_EQUAL(a[i][j], c[i2][j2]);
	    BOOST_CHECK(&a[i][j] == &c[i2][j2]);
	  }
	  else
	  {
	    BOOST_CHECK(a[i][j] != c[i2][j2]);
	    BOOST_CHECK(&a[i][j] != &c[i2][j2]);
	  }
	}
      }
    }
  }
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
