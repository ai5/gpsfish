/**
 * centering5x3.t.cc
 */
#include "osl/bits/centering5x3.h"
#include "osl/centering3x3.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

static bool inSafeRegion5x3(Square center)
{
  if (! center.isOnBoard())
    return false;
  if (center != Centering3x3::adjustCenter(center))
    return false;
  const Square tl(center.x()+2, center.y()-1);
  const Square tr(center.x()-2, center.y()-1);
  const Square bl(center.x()+2, center.y()+1);
  const Square br(center.x()-2, center.y()+1);
  return tl.isOnBoard()
    && tr.isOnBoard()
    && bl.isOnBoard()
    && br.isOnBoard();
}

BOOST_AUTO_TEST_CASE(Centering5x3Test11)
{
  BOOST_CHECK_EQUAL(Square(3,2), 
		       Centering5x3::adjustCenter(Square(1,1)));
}
BOOST_AUTO_TEST_CASE(Centering5x3Test22)
{
  BOOST_CHECK_EQUAL(Square(3,2), 
		       Centering5x3::adjustCenter(Square(2,2)));
}


BOOST_AUTO_TEST_CASE(Centering5x3TestXY)
{
  for (int y=0; y<=10; ++y)
  {
    for (int x=0; x<=10; ++x)
    {
      const Square target = Square(x,y);
      if (target.isOnBoard())
      {
	const Square center = Centering5x3::adjustCenter(target);
	BOOST_CHECK_EQUAL(Centering5x3::adjustCenterNaive(target),
			     center);
	BOOST_CHECK(inSafeRegion5x3(center));
	BOOST_CHECK((! inSafeRegion5x3(target))
		       || (target == center)); // 移動の必要がないものはそのまま
      }
      else
      {
	BOOST_CHECK(Centering5x3::adjustCenter(target).isPieceStand());
      }
    }
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
