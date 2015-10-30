#include "featureSet.h"
#include "osl/move_probability/featureSet.h"
#include "osl/progress.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

using namespace osl;

using gpsshogi::StateInfo;
using gpsshogi::PredictionModelLight;
BOOST_AUTO_TEST_CASE(FeatureSetTestinitialize)
{
  gpsshogi::StandardFeatureSet feature_set;
  std::valarray<double> weights;
  const char *filename = "../stable-move-order";
  feature_set.load(filename, weights);
  /* const osl::move_probability::StandardFeatureSet& osl_feature_set
     = */ osl::move_probability::StandardFeatureSet::instance();
}

using gpsshogi::StateInfo;
BOOST_AUTO_TEST_CASE(FeatureSetTestPass)
{
  osl::OslConfig::setUp();
  gpsshogi::StandardFeatureSet feature_set;
  std::valarray<double> weights;
  const char *filename = "../stable-move-order";
  feature_set.load(filename, weights);
  const osl::move_probability::StandardFeatureSet& osl_feature_set
    = osl::move_probability::StandardFeatureSet::instance();

  NumEffectState state;
  const Move pass_b = Move::PASS(BLACK);
  const Move pass_w = Move::PASS(WHITE);
  state.makeMove(pass_b);
  state.makeMove(pass_w);

  std::vector<osl::Move> moves;
  moves.push_back(pass_b);
  moves.push_back(pass_w);
  MoveStack history;
  history.push(pass_b);
  history.push(pass_w);

  gpsshogi::StateInfo info(state, moves, 1);
  typedef osl::progress::ml::NewProgress progress_t;
  progress_t progress(state);
  osl::move_probability::StateInfo osl_info(state, progress.progress16(),
					    history, info.threatmate_move);

  MoveLogProbVector a, b;
  feature_set.generateLogProb(info, a, weights);
  osl_feature_set.generateLogProb(osl_info, b);
  BOOST_CHECK_EQUAL(a, b);
}

using gpsshogi::StateInfo;
BOOST_AUTO_TEST_CASE(FeatureSetTestgenerateLogProb)
{
  osl::OslConfig::setUp();
  gpsshogi::StandardFeatureSet feature_set;
  std::valarray<double> weights, tweights;
  const char *filename = "../stable-move-order";
  feature_set.load(filename, weights);
  {
    std::string tfilename = osl::OslConfig::home();
    tfilename += "/data/move-tactical.txt";
    const int tactical_dimension = 8*4;
    tweights.resize(tactical_dimension);
    std::ifstream is(tfilename.c_str());
    for (int i=0; i<tactical_dimension; ++i)
      is >> tweights[i];
    BOOST_CHECK(is);
  }
  const osl::move_probability::StandardFeatureSet& osl_feature_set
    = osl::move_probability::StandardFeatureSet::instance();
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string csafilename;
  int i=0;
  while((ifs >> csafilename) && csafilename != "" && ++i<128){
    CsaFileMinimal csa(OslConfig::testCsaFile(csafilename));
    NumEffectState state(csa.initialState());
    std::vector<osl::Move> moves=csa.moves();
    gpsshogi::StateInfo info(state, moves, -1);
    info.use_adhoc_adjust = true;
    typedef osl::progress::ml::NewProgress progress_t;
    progress_t progress(state);
    MoveStack history;
    osl::move_probability::StateInfo osl_info(state, progress.progress16(), 
					      history, info.threatmate_move);
    for (size_t i=0; i<moves.size(); i++) {
      BOOST_CHECK_EQUAL(info.progress8(), osl_info.progress8());
      BOOST_CHECK(info.pin_by_opposing_sliders == osl_info.pin_by_opposing_sliders);
      BOOST_CHECK(info.threatened == osl_info.threatened);
      BOOST_CHECK(info.long_attack_cache == osl_info.long_attack_cache);
      // << state << info.threatmate_move << "\n";
      for (int y=1; y<=9; ++y) {
	for (int x=1; x<=9; ++x) {
	  Square square(x,y);
	  BOOST_CHECK(info.pattern_cache[square.index()]
		      == osl_info.pattern_cache[square.index()]);
	}
      }
      BOOST_CHECK(info.last_add_effect == osl_info.last_add_effect);
      MoveLogProbVector a, b;
      feature_set.generateLogProb(info, a, weights);
      osl_feature_set.generateLogProb(osl_info, b);
      BOOST_CHECK_EQUAL(a, b); // << state << info.threatmate_move << "\n";

      for  (MoveLogProb move: a) {
	gpsshogi::index_list_t dummy;
	double sum = feature_set.matchLight(info, move.move(), dummy, weights);
	BOOST_CHECK_SMALL(sum - 
			  osl_feature_set.matchLight(osl_info, move.move()),
			  0.001);
	if (move.move().capturePtype() != PTYPE_EMPTY) {
	  BOOST_CHECK_EQUAL(PredictionModelLight::logProbTakeBack(info, sum, tweights),
		    osl_feature_set.logProbTakeBack(osl_info, move.move()));
	  BOOST_CHECK_EQUAL(PredictionModelLight::logProbSeePlus(info, sum, tweights),
		    osl_feature_set.logProbSeePlus(osl_info, move.move()));
	}
      }

      state.makeMove(moves[i]);
      progress.update(state, moves[i]);
      history.push(moves[i]);
      info.update(moves[i]);
      osl_info.reset(state, progress.progress16(), history,
		     info.threatmate_move);
    }
  }  
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
