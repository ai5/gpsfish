/* ptypeOSquareVector.t.cc
 */
#include "osl/container.h"
#include <boost/test/unit_test.hpp>
using namespace osl;

BOOST_AUTO_TEST_CASE(PtypeOSquareVectorTestSort)
{
  Square dummy(1,1);
  {
    PtypeOSquareVector v;
    v.push_back(std::make_pair(newPtypeO(BLACK, PAWN), dummy));
    v.push_back(std::make_pair(newPtypeO(BLACK, PPAWN), dummy));
    v.push_back(std::make_pair(newPtypeO(BLACK, PAWN), dummy));
    v.sort();
    BOOST_CHECK_EQUAL(PPAWN, getPtype(v[2].first));
  }
  {
    PtypeOSquareVector v;
    v.push_back(std::make_pair(newPtypeO(BLACK, PPAWN), dummy));
    v.push_back(std::make_pair(newPtypeO(BLACK, PPAWN), dummy));
    v.push_back(std::make_pair(newPtypeO(BLACK, PAWN), dummy));
    v.push_back(std::make_pair(newPtypeO(BLACK, SILVER), dummy));
    v.sort();
    BOOST_CHECK_EQUAL(PAWN, getPtype(v[0].first));
    BOOST_CHECK_EQUAL(SILVER, getPtype(v[3].first));
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
