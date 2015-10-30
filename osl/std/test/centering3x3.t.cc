/**
 * centering3x3.t.cc
 */
#include "osl/centering3x3.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test.hpp>
using namespace osl;

BOOST_AUTO_TEST_CASE(Centering3x3Test11)
{
  BOOST_CHECK_EQUAL(Square(2,2), 
		       Centering3x3::adjustCenter(Square(1,1)));
}
BOOST_AUTO_TEST_CASE(Centering3x3Test22)
{
  BOOST_CHECK_EQUAL(Square(2,2), 
		       Centering3x3::adjustCenter(Square(2,2)));
}


BOOST_AUTO_TEST_CASE(Centering3x3TestXY)
{
  for (int y=0; y<=10; ++y)
  {
    for (int x=0; x<=10; ++x)
    {
      const Square target = Square(x,y);
      if (target.isOnBoard())
      {
	BOOST_CHECK_EQUAL(Centering3x3::adjustCenterNaive(target),
			     Centering3x3::adjustCenter(target));
      }
      else
      {
	BOOST_CHECK(Centering3x3::adjustCenter(target).isPieceStand());
      }
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
