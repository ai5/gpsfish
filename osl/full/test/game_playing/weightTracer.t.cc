#include "osl/game_playing/weightTracer.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::game_playing;

BOOST_AUTO_TEST_CASE(WeightTracerTestTop1) 
{
  osl::book::WeightedBook book(osl::OslConfig::openingBook());
  osl::game_playing::DeterminateWeightTracer tracer(book, false, 1);
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);

  for (int i=0; i<100; ++i)
  {
    BOOST_CHECK_EQUAL(m76fu, tracer.selectMove());
  }
}

BOOST_AUTO_TEST_CASE(WeightTracerTestTop2) 
{
  osl::book::WeightedBook book(osl::OslConfig::openingBook());
  osl::game_playing::DeterminateWeightTracer tracer(book, false, 2);
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  const Move m26fu(Square(2,7),Square(2,6),PAWN,PTYPE_EMPTY,false,BLACK);

  for (int i=0; i<100; ++i)
  {
    const Move selected = tracer.selectMove();
    BOOST_CHECK(selected == m76fu || selected == m26fu);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
