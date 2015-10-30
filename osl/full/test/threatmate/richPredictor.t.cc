#include "osl/threatmate/richPredictor.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::threatmate;

BOOST_AUTO_TEST_CASE(RichPredictorTestBeginning)
{
  const NumEffectState state((SimpleState(HIRATE)));
  const Move move = Move(Square(3,3), PAWN, WHITE);
  RichPredictor Rich;
  BOOST_CHECK( Rich.predict(state, move) < 100.0); 
}

BOOST_AUTO_TEST_CASE(RichPredictorTestAddEffect)
{
  const NumEffectState state(CsaString(
				       "P1-KY *  * +TO * +KA-OU-KE-KY\n"
				       "P2-KY *  *  * +HI-KI *  *  * \n"
				       "P3 *  * -KE *  * -FU-FU-FU * \n"
				       "P4 * -FU-FU *  *  *  * -KI-FU\n"
				       "P5-FU *  *  * +FU *  *  *  * \n"
				       "P6 *  * +FU-HI *  *  * +KI * \n"
				       "P7+FU+FU+KE * -TO-NK+FU *  * \n"
				       "P8+OU * +GI *  *  *  *  *  * \n"
				       "P9+KY * -TO *  *  *  *  *  * \n"
				       "P+00KI00FU\n"
				       "P-00KA00GI00GI00GI00FU00FU\n"
				       "-\n").initialState());
  const Move move = Move(Square(5,2), ROOK, BLACK);
  RichPredictor Rich;
  BOOST_CHECK( Rich.predict(state, move) > 0.0);
}

BOOST_AUTO_TEST_CASE(RichPredictorTestNotAddEffect)
{
  const NumEffectState state(CsaString(
				       "P1+RY-KE-FU * +NK *  * -OU-FU\n"
				       "P2 *  * -GI *  * +KI *  *  * \n"
				       "P3 *  *  *  *  *  *  *  * +GI\n"
				       "P4+FU *  *  * -KY+GI-KY * +FU\n"
				       "P5 * -FU *  *  *  *  *  * -KE\n"
				       "P6 *  * +FU *  *  * +KA-UM * \n"
				       "P7 * +FU *  *  *  * -KY+KY-GI\n"
				       "P8 *  *  *  *  * -KI *  *  * \n"
				       "P9-RY+KE+KI *  *  *  * +KI+OU\n"
				       "P+00FU00FU00FU00FU00FU\n"
				       "P-00FU00FU00FU00FU00FU00FU\n"
				       "-\n").initialState());

  const Move move = Move(Square(3,6), BISHOP, BLACK);
  RichPredictor Rich;
  BOOST_CHECK( Rich.predict(state, move) > 0.0);
}

BOOST_AUTO_TEST_CASE(RichPredictorTestCheck)
{
  const NumEffectState state(CsaString(
				       "P1+RY-KE-FU * +NK *  * -OU-FU\n"
				       "P2 *  * -GI *  * +KI *  *  * \n"
				       "P3 *  *  *  *  *  *  * +KY+GI\n"
				       "P4+FU *  *  * -UM+GI-KY * +FU\n"
				       "P5 * -FU *  *  *  *  *  * -KE\n"
				       "P6 *  * +FU *  *  *  *  *  * \n"
				       "P7 * +FU *  *  *  * -KY * -GI\n"
				       "P8 *  *  *  *  * -KI *  *  * \n"
				       "P9-RY+KE+KI *  *  *  * +KI+OU\n"
				       "P+00FU00FU00FU00FU00FU\n"
				       "P-00KA00KY00FU00FU00FU00FU00FU00FU\n"
				       "-\n").initialState());

  const Move move = Move(Square(2,3), LANCE, BLACK);
  RichPredictor Rich;
  BOOST_CHECK( Rich.predict(state, move) > 0.0);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
