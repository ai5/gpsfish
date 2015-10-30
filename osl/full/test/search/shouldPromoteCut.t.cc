#include "osl/move_classifier/shouldPromoteCut.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(ShouldPromoteCutTestPlayer) {
  const Square p39(3,9);
  const Square p66(6,6);

  // WHITE
  const Move w3966ump(p39,p66,PBISHOP,PTYPE_EMPTY,true,WHITE);
  BOOST_CHECK(! ShouldPromoteCut::canIgnoreAndNotDrop(w3966ump));

  const Move w3966um(p39,p66,PBISHOP,PTYPE_EMPTY,false,WHITE);
  BOOST_CHECK(! ShouldPromoteCut::canIgnoreAndNotDrop(w3966um));

  const Move w3966ka(p39,p66,BISHOP,PTYPE_EMPTY,false,WHITE);
  BOOST_CHECK(ShouldPromoteCut::canIgnoreAndNotDrop(w3966ka));

  // BLACK
  const Move b3966um(p39,p66,PBISHOP,PTYPE_EMPTY,false,BLACK);
  BOOST_CHECK(! ShouldPromoteCut::canIgnoreAndNotDrop(b3966um));

  const Move b3966ka(p39,p66,BISHOP,PTYPE_EMPTY,false,BLACK);
  BOOST_CHECK(! ShouldPromoteCut::canIgnoreAndNotDrop(b3966ka));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
