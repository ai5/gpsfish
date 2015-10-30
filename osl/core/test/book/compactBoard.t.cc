#include "osl/book/compactBoard.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <string>

using namespace osl;
using namespace osl::book;

BOOST_AUTO_TEST_CASE(CompactBoardTestisEqual)
{
  NumEffectState state1;
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  state1.makeMove(m76fu);

  NumEffectState state2(CsaString(
			 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			 "P2 * -HI *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  * +FU *  *  *  *  *  * \n"
			 "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA *  *  *  *  * +HI * \n"
			 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			 "-\n").initialState());
  
  const CompactBoard cb1(state1);
  const CompactBoard cb2(state2);
  BOOST_CHECK(cb1 == cb2);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

