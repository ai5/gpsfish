#include "osl/game_playing/recordTracer.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::game_playing;

BOOST_AUTO_TEST_CASE(RecordTracerTestEmpty)
{
  std::vector<Move> moves;
  RecordTracer tracer(moves);
  BOOST_CHECK_EQUAL(true, tracer.isOutOfBook());
  BOOST_CHECK_EQUAL(Move::INVALID(), tracer.selectMove());
}
BOOST_AUTO_TEST_CASE(RecordTracerTestTrace) 
{
  std::vector<Move> moves;
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  const Move m26fu(Square(2,7),Square(2,6),PAWN,PTYPE_EMPTY,false,BLACK);
  const Move m34fu(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE);
  moves.push_back(m76fu);
  moves.push_back(m34fu);
    
  RecordTracer tracer(moves);
  moves.clear();		// copy されたはず
    
  BOOST_CHECK_EQUAL(false, tracer.isOutOfBook());
  BOOST_CHECK_EQUAL(m76fu, tracer.selectMove());

  tracer.update(m76fu);

  BOOST_CHECK_EQUAL(false, tracer.isOutOfBook());
  BOOST_CHECK_EQUAL(m34fu, tracer.selectMove());

  tracer.update(m34fu);
    
  BOOST_CHECK_EQUAL(true, tracer.isOutOfBook());
  BOOST_CHECK_EQUAL(Move::INVALID(), tracer.selectMove());

  tracer.popMove();
    
  BOOST_CHECK_EQUAL(false, tracer.isOutOfBook());
  BOOST_CHECK_EQUAL(m34fu, tracer.selectMove());

  tracer.popMove();

  BOOST_CHECK_EQUAL(false, tracer.isOutOfBook());
  BOOST_CHECK_EQUAL(m76fu, tracer.selectMove());

  tracer.update(m26fu);

  BOOST_CHECK_EQUAL(true, tracer.isOutOfBook());
  BOOST_CHECK_EQUAL(Move::INVALID(), tracer.selectMove());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
