#include "eval/minorPiece.h"
#include "osl/record/csaRecord.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl;
using namespace osl::record;

typedef osl::FixedCapacityVector<std::pair<int,int>,gpsshogi::MaxActiveWithDuplication> features_t;
class PromotedMinorPiecesTest : public gpsshogi::EvalComponentStages
{
public:
  PromotedMinorPiecesTest() : gpsshogi::EvalComponentStages(new gpsshogi::PromotedMinorPieces)
  {
  }
  void getFeatures(const osl::NumEffectState &state,
		   gpsshogi::index_list_t &features) const
  {
    featuresOneNonUniq(state, features);
  }
};

BOOST_AUTO_TEST_CASE(PromotedMinorPiecesTestFeatures)
{
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU+TO-FU-FU-FU-FU-FU+TO-FU\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU-TO+FU+FU+FU+FU+FU-TO+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::index_list_t features;
    PromotedMinorPiecesTest test;
    test.getFeatures(state, features);
    BOOST_REQUIRE_EQUAL(0, features.size());
  }

  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU * +NY-FU-FU+TO\n"
				 "P4 *  *  *  *  * -FU *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 * +FU *  *  *  *  *  *  * \n"
				 "P7+FU-TO+FU+FU+FU+FU+FU-TO+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE * \n"
				 "+\n").initialState());
    gpsshogi::index_list_t features;
    PromotedMinorPiecesTest test;
    test.getFeatures(state, features);
    BOOST_REQUIRE_EQUAL(1, features.size());
    BOOST_CHECK_EQUAL(4, features[0].first);
    BOOST_CHECK_EQUAL(1, features[0].second);
  }

  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU+TO-FU-FU+TO-FU-FU-FU+TO\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU-TO+FU+FU+FU+FU+FU-TO+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::index_list_t features;
    PromotedMinorPiecesTest test;
    test.getFeatures(state, features);
    BOOST_REQUIRE_EQUAL(2, features.size());
    BOOST_CHECK_EQUAL(3, features[0].first);
    BOOST_CHECK_EQUAL(1, features[0].second);
    BOOST_CHECK_EQUAL(4, features[1].first);
    BOOST_CHECK_EQUAL(1, features[1].second);
  }

  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU+TO-FU-FU+TO-FU-FU-FU * \n"
				 "P4 *  *  *  * +TO *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU-TO+FU+FU+FU+FU+FU-TO+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::index_list_t features;
    PromotedMinorPiecesTest test;
    test.getFeatures(state, features);
    BOOST_REQUIRE_EQUAL(2, features.size());
#ifdef L1BALL_NO_SORT
    BOOST_CHECK_EQUAL(0, features[1].first);
    BOOST_CHECK_EQUAL(1, features[1].second);
#else
    BOOST_CHECK_EQUAL(0, features[0].first);
    BOOST_CHECK_EQUAL(1, features[0].second);
#endif
  }

  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU *  * -FU-FU-FU\n"
				 "P4 *  *  *  *  * -FU *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 * -TO *  *  *  *  *  *  * \n"
				 "P7+FU-TO+FU+FU+FU+FU+FU-TO+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::index_list_t features;
    PromotedMinorPiecesTest test;
    test.getFeatures(state, features);
    BOOST_REQUIRE_EQUAL(1, features.size());
    BOOST_CHECK_EQUAL(3, features[0].first);
    BOOST_CHECK_EQUAL(-1, features[0].second);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
