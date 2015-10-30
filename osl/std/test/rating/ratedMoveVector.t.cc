/* ratedMoveVector.t.cc
 */
#include "osl/rating/ratedMoveVector.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::rating;

BOOST_AUTO_TEST_CASE(RatedMoveVectorTestSort)
{
  RatedMove a(Move(Square(5,5),GOLD,BLACK), 200, 200);
  RatedMove b(Move(Square(5,5),SILVER,BLACK), 100, 100);
  {
    RatedMoveVector v;
    v.push_back(a);
    v.push_back(b);
    v.sort();
    BOOST_CHECK_EQUAL(v[0], a);
  }
  {
    RatedMoveVector v;
    v.push_back(b);
    v.push_back(a);
    v.sort();
    BOOST_CHECK_EQUAL(v[0], a);
  }
  b.setRating(300);
  {
    RatedMoveVector v;
    v.push_back(b);
    v.push_back(a);
    v.sort();
    BOOST_CHECK_EQUAL(v[0], b);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
