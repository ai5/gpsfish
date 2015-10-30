#include "osl/csa.h"
#include "osl/oslConfig.h"
#include "osl/enter_king/simplePredictor.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <unistd.h>

using namespace osl;
using namespace enter_king;

BOOST_AUTO_TEST_CASE(SimplePredictorTestBeginning)
{
  const NumEffectState state((SimpleState(HIRATE)));
  SimplePredictor Predictor;
  const double pb = Predictor.getProbability<BLACK>(state);
  const double pw = Predictor.getProbability<WHITE>(state);
  BOOST_CHECK( !Predictor.predict(state,BLACK) );
  BOOST_CHECK( !Predictor.predict(state, WHITE) );
  BOOST_CHECK( pb == pw );
  const double pb27 = Predictor.getProbability27<BLACK>(state);
  const double pw27 = Predictor.getProbability27<WHITE>(state);
  BOOST_CHECK( pb27 < 0.1);
  BOOST_CHECK( pw27 < 0.1);
  // BOOST_CHECK( pb27 != pw27 );
}

BOOST_AUTO_TEST_CASE(SimplePredictorTestBlackWin)
{
  const NumEffectState state(CsaString(
				       "P1 *  * +RY *  * +KI * -KE * \n"
				       "P2+OU+TO+UM+UM+TO * -KI-OU * \n"
				       "P3-FU+FU+FU+FU+FU-GI-GI *  * \n"
				       "P4 *  *  *  * -FU-FU-FU-FU * \n"
				       "P5 *  * -RY-NY-GI *  *  *  * \n"
				       "P6 *  *  *  *  * +FU+FU+FU * \n"
				       "P7 *  *  *  *  * +KI+KE * -NY\n"
				       "P8 *  *  *  *  * +KI+GI *  * \n"
				       "P9 *  *  *  * -NY *  *  *  * \n"
				       "P+00FU00FU00FU00FU00KY00KE00KE\n"
				       "+\n").initialState());
  SimplePredictor Predictor;
  BOOST_CHECK( Predictor.predict(state,BLACK) );
  BOOST_CHECK( Predictor.predict27(state,BLACK) );
}

BOOST_AUTO_TEST_CASE(SimplePredictorTestWhiteWin)
{
  const NumEffectState state(CsaString(
					"P1 *  *  *  * +NY *  *  *  * \n"
					"P2 *  *  *  *  * -KI-GI *  * \n"
					"P3 *  *  *  *  * -KI-KE * +NY\n"
					"P4 *  *  *  *  * -FU-FU-FU * \n"
					"P5 *  * +RY+NY+GI *  *  *  * \n"
					"P6 *  *  *  * +FU+FU+FU+FU * \n"
					"P7+FU-FU-FU-FU-FU+GI+GI *  * \n"
					"P8-OU-TO-UM-UM-TO * +KI+OU * \n"
					"P9 *  * -RY *  * -KI * +KE * \n"
					"P-00FU00FU00FU00FU00KY00KE00KE\n"
					"-\n").initialState());
  SimplePredictor Predictor;
  BOOST_CHECK( Predictor.predict(state, WHITE) );
  BOOST_CHECK( Predictor.predict27(state, WHITE) );
}

BOOST_AUTO_TEST_CASE(SimplePredictorTestBothEnterBlackWin) {
  const NumEffectState state(CsaString(
				       "P1+NK+OU *  *  * +RY *  *  * \n"
				       "P2 * +TO+NY *  *  * +KI *  * \n"
				       "P3 *  *  *  *  *  *  * +GI * \n"
				       "P4 *  *  *  *  *  *  *  *  * \n"
				       "P5 *  *  *  *  *  *  *  *  * \n"
				       "P6 *  *  *  *  *  *  *  * -KY\n"
				       "P7 *  *  *  *  *  * -TO *  * \n"
				       "P8 *  *  *  *  *  *  *  * -OU\n"
				       "P9+KY-HI *  *  *  *  * -NG * \n"
				       "P+00KA00KA00KI00KI00GI00GI00KE00KE00FU00FU00FU00FU00FU\n"
				       "P-00KI00KE00KY00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
				       "-\n").initialState());

  SimplePredictor Predictor;
  BOOST_CHECK(Predictor.predict(state,BLACK));
  BOOST_CHECK(! Predictor.predict(state, WHITE));
  BOOST_CHECK(Predictor.predict27(state,BLACK));
  BOOST_CHECK(! Predictor.predict27(state, WHITE));
}
BOOST_AUTO_TEST_CASE(SimplePredictorTestBothEnterWhiteWin) {
  const NumEffectState state(CsaString(
				       "P1-KY+HI *  *  *  *  * +NG * \n"
				       "P2 *  *  *  *  *  *  *  * +OU\n"
				       "P3 *  *  *  *  *  * +TO *  * \n"
				       "P4 *  *  *  *  *  *  *  * +KY\n"
				       "P5 *  *  *  *  *  *  *  *  * \n"
				       "P6 *  *  *  *  *  *  *  *  * \n"
				       "P7 *  *  *  *  *  *  * -GI * \n"
				       "P8 * -TO-NY *  *  * -KI *  * \n"
				       "P9-NK-OU *  *  * -RY *  *  * \n"
				       "P-00KA00KA00KI00KI00GI00GI00KE00KE00FU00FU00FU00FU00FU\n"
				       "P+00KI00KE00KY00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU00FU\n"
				       "+\n").initialState());

  SimplePredictor Predictor;
  BOOST_CHECK(! Predictor.predict(state,BLACK));
  BOOST_CHECK(Predictor.predict(state, WHITE));
  BOOST_CHECK(! Predictor.predict27(state,BLACK));
  BOOST_CHECK(Predictor.predict27(state, WHITE));
}

BOOST_AUTO_TEST_CASE(SimplePredictorTestBothEnter) {
  const NumEffectState state(CsaString(
				       "P1 *  *  *  *  *  * +RY * -KY\n"
				       "P2+TO+OU * +FU+UM *  *  *  * \n"
				       "P3 *  * +FU+KI *  *  * -FU-FU\n"
				       "P4+KI * +GI *  *  *  *  *  * \n"
				       "P5+KY+KI *  *  *  *  *  * +FU\n"
				       "P6 *  *  *  *  * -UM *  *  * \n"
				       "P7 * +FU *  *  * -NG-NK *  * \n"
				       "P8 *  *  *  * -TO *  *  *  * \n"
				       "P9 *  *  * -TO *  * -TO-OU-NG\n"
				       "P+00KI00KE00KY00FU00FU00FU00FU\n"
				       "P-00HI00GI00KE00KE00KY00FU00FU00FU00FU\n"
				       "+\n").initialState());
  SimplePredictor Predictor;
  BOOST_CHECK(Predictor.predict(state,BLACK));
  BOOST_CHECK(Predictor.predict(state,WHITE));
}

BOOST_AUTO_TEST_CASE(SimplePredictorTestOppositionBlackCannotEnter) {
  const NumEffectState state(CsaString(
				       "P1-KY *  *  *  *  *  * -KE-OU\n"
				       "P2 *  *  *  *  *  * -KI-KI-KY\n"
				       "P3-FU *  * -FU *  * -FU-FU-FU\n"
				       "P4 *  * -FU * -FU *  *  *  * \n"
				       "P5 *  *  * +FU-KA * +FU * +OU\n"
				       "P6 *  * +FU * +KA * +KI *  * \n"
				       "P7+FU *  *  * +FU+GI+KE-KI+FU\n"
				       "P8 *  *  *  *  *  *  * -GI+KY\n"
				       "P9 *  *  *  *  *  * +RY *  * \n"
				       "P+00HI00KE00KE00KY00FU00FU00FU00FU00FU\n"
				       "P-00GI00GI\n"
				       "+\n").initialState());
  SimplePredictor Predictor;
  BOOST_CHECK(! Predictor.predict(state,BLACK));
  BOOST_CHECK(! Predictor.predict27(state,BLACK));
}
BOOST_AUTO_TEST_CASE(SimplePredictorTestOppositionWhiteCannotEnter) {
  const NumEffectState state(CsaString(
				       "P1 *  *  *  *  *  * -RY *  * \n"
				       "P2 *  *  *  *  *  *  * +GI-KY\n"
				       "P3-FU *  *  * -FU-GI-KE+KI-FU\n"
				       "P4 *  * -FU * -KA * -KI *  * \n"
				       "P5 *  *  * -FU+KA * -FU * -OU\n"
				       "P6 *  * +FU * +FU *  *  *  * \n"
				       "P7+FU *  * +FU *  * +FU+FU+FU\n"
				       "P8 *  *  *  *  *  * +KI+KI+KY\n"
				       "P9+KY *  *  *  *  *  * +KE+OU\n"
				       "P+00GI00GI\n"
				       "P-00HI00KE00KE00KY00FU00FU00FU00FU00FU\n"
				       "-\n").initialState());
  SimplePredictor Predictor;
  BOOST_CHECK(! Predictor.predict(state, WHITE));
  BOOST_CHECK(! Predictor.predict27(state, WHITE));
}

BOOST_AUTO_TEST_CASE(SimplePredictorTestDistantMajorPieceBlackWin){
  // 大駒は1手で敵陣に入れるが、それを考慮していないために失敗するかもしれない
  const NumEffectState state(CsaString(
				       "P1-FU *  *  *  *  * -RY-KE-KY\n"
				       "P2 *  * +NG+TO *  * -KI-OU * \n"
				       "P3+FU+OU * +KI * -KI * -FU * \n"
				       "P4 *  * -FU+FU-FU *  * -KE * \n"
				       "P5 * -NY *  * -KA-GI+FU *  * \n"
				       "P6 *  * -GI *  *  *  * +FU+FU\n"
				       "P7 *  *  *  * -NK * -FU+KY+KY\n"
				       "P8+RY *  *  *  *  *  *  *  * \n"
				       "P9 *  *  *  *  *  *  *  *  * \n"
				       "P+00KA00KI00KE00FU00FU00FU00FU00FU00FU00FU\n"
				       "P-00GI\n"

				       "-\n").initialState());
  SimplePredictor Predictor;
  // std::cerr << Predictor.getProbability<BLACK>(state) << std::endl;
  BOOST_CHECK(Predictor.predict(state,BLACK));
}
BOOST_AUTO_TEST_CASE(SimplePredictorTestDistantMajorPieceWhiteWin){
  // 大駒は1手で敵陣に入れるが、それを考慮していないために失敗するかもしれない
  const NumEffectState state(CsaString(
				       "P1 *  *  *  *  *  *  *  *  * \n"
				       "P2-RY *  *  *  *  *  *  *  * \n"
				       "P3 *  *  *  * +NK * +FU-KY-KY\n"
				       "P4 *  * +GI *  *  *  * -FU-FU\n"
				       "P5 * +NY *  * +KA+GI-FU *  * \n"
				       "P6 *  * +FU-FU+FU *  * +KE * \n"
				       "P7-FU-OU * -KI * +KI * +FU * \n"
				       "P8 *  * -NG-TO *  * +KI+OU * \n"
				       "P9+FU *  *  *  *  * +RY+KE+KY\n"
				       "P+00GI\n"
				       "P-00KA00KI00KE00FU00FU00FU00FU00FU00FU00FU\n"

				       "+\n").initialState());
  SimplePredictor Predictor;
  // std::cerr << Predictor.getProbability<WHITE>(state) << std::endl;
  BOOST_CHECK(Predictor.predict(state, WHITE));
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
