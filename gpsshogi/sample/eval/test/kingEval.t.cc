#include "eval/kingEval.h"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"

#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(KingEffect3CountPieces)
{
  const NumEffectState state(CsaString(
			       "P1-KY-KE-GI-KI * +KI * -KE-OU\n"
			       "P2 * -HI *  *  *  * +NK-GI-KY\n"
			       "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			       "P4 *  *  *  *  *  *  *  *  * \n"
			       "P5-KA *  *  *  *  *  *  *  * \n"
			       "P6+FU *  *  *  *  *  *  *  * \n"
			       "P7 * +FU+FU+FU+FU+FU+FU+FU * \n"
			       "P8 * +KA *  *  *  *  * +HI * \n"
			       "P9 *  * +GI+KI+OU+KI+GI * +KY\n"
			       "P+00KE00KY00FU\n"
			       "+\n").initialState());
  int piece_count, stand_count, attacked_count;
  bool with_knight, stand_with_knight;
  gpsshogi::King25Effect3::countPieces<BLACK>(state, piece_count, with_knight,
					      stand_count, stand_with_knight,
					      attacked_count);
  BOOST_REQUIRE_EQUAL(2, piece_count);
  BOOST_REQUIRE_EQUAL(0, stand_count);
  BOOST_REQUIRE_EQUAL(1, attacked_count);
  ASSERT_FALSE(with_knight);
  BOOST_REQUIRE(stand_with_knight);
}
