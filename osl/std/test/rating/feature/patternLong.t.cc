#include "osl/rating/feature/pattern.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::rating;

BOOST_AUTO_TEST_CASE(PatternLongTestNextPieceOrEnd) 
{
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			 "P2 * -HI *  *  *  *  * -KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			 "P4 *  *  *  *  *  * -FU *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  * +FU *  *  *  *  *  * \n"
			 "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA *  *  *  *  * +HI * \n"
			 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			 "+\n").initialState());
  const Piece piece 
    = PatternLong::nextPieceOrEnd(state, Square(6,6), 
				  Board_Table.getShortOffset(Offset32(Square(6,6), Square(2,2)))).first;
  BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(8,8)), piece);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
