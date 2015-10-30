#include "osl/move_order/promotion.h"
#include "osl/move_order/moveSorter.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::move_order;

BOOST_AUTO_TEST_CASE(MoveOrderPromotionTestSort) 
{ 
  const Square from = Square(2,8);
  const Square to   = Square(2,2);
    
  const Move m1 = Move(from, to, ROOK, PTYPE_EMPTY, false, BLACK);
  const Move m2 = Move(from, to, PROOK, PTYPE_EMPTY, true, BLACK);
  MoveVector moves;
  moves.push_back(m1);
  moves.push_back(m2);
  MoveSorter::sort(moves, Promotion());
  BOOST_CHECK(moves[0] == m2); 
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
