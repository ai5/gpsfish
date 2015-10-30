#include "osl/move_order/captureEstimation.h"
#include "osl/move_order/moveSorter.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::move_order;

BOOST_AUTO_TEST_CASE(CaptureEstimationTestSort) 
{ 
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI * -OU * -GI-KE-KY\n"
			 "P2 *  *  *  *  *  * -KI-KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5-KI *  *  *  *  *  *  *  * \n"
			 "P6+FU *  *  * +HI *  * +HI * \n"
			 "P7 * +FU+FU+FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA *  *  *  *  *  *  * \n"
			 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			 "+\n").initialState());

  const Move m1 = Move(Square(2,6), Square(2,3), PROOK, 
		       PAWN, true, BLACK);
  const Move m2 = Move(Square(5,6), Square(5,3), PROOK, 
		       PAWN, true, BLACK);
  MoveVector moves;
  moves.push_back(m1);
  moves.push_back(m2);
  MoveSorter::sort(moves, CaptureEstimation(state));
  BOOST_CHECK(moves[0] == m2); 
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
