#include "osl/move_probability/featureSet.h"
#include "osl/progress.h"
#include "osl/hashKey.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::move_probability;

BOOST_AUTO_TEST_CASE(MPFeatureSetTestInitialize)
{
  /* const osl::move_probability::StandardFeatureSet& osl_feature_set
     = */ osl::move_probability::StandardFeatureSet::instance();
}

BOOST_AUTO_TEST_CASE(MPFeatureSetTestPass)
{
  const StandardFeatureSet& feature_set
    = StandardFeatureSet::instance();

  NumEffectState state;
  const Move pass_b = Move::PASS(BLACK);
  const Move pass_w = Move::PASS(WHITE);
  state.makeMove(pass_b);
  state.makeMove(pass_w);

  MoveStack history;
  history.push(pass_b);
  history.push(pass_w);

  typedef progress::ml::NewProgress progress_t;
  progress_t progress(state);
  StateInfo info(state, progress.progress16(), history);

  MoveLogProbVector a;
  feature_set.generateLogProb(info, a);
  BOOST_CHECK(! a.empty());
}

BOOST_AUTO_TEST_CASE(MPFeatureSetTestGenerateLogProb)
{
  const StandardFeatureSet& feature_set
    = StandardFeatureSet::instance();
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string csafilename;
  int i=0;
  while((ifs >> csafilename) && csafilename != "" && ++i<128){
    RecordMinimal record=CsaFileMinimal(OslConfig::testCsaFile(csafilename)).load();
    NumEffectState state(record.initial_state);
    auto& moves=record.moves;
    typedef osl::progress::ml::NewProgress progress_t;
    progress_t progress(state);
    MoveStack history;
    StateInfo info(state, progress.progress16(), history);
    for (size_t i=0; i<moves.size(); i++) {
      MoveLogProbVector a;
      feature_set.generateLogProb(info, a);
      BOOST_CHECK(!a.empty());

      state.makeMove(moves[i]);
      progress.update(state, moves[i]);
      history.push(moves[i]);
      info.reset(state, progress.progress16(), history);
      StateInfo info2(state, progress.progress16(), history);

      {
	const StateInfo& l = info;
	const StateInfo& r = info2;
	for (int x=1; x<=9; ++x) {
	  for (int y=1; y<=9; ++y) {
	    const Square position(x,y);
	    if (! (l.pattern_cache[position.index()]
		   == r.pattern_cache[position.index()])) {
	      std::cerr << state << position
			<< " " << moves[i] << "\n";
	    }
	    BOOST_CHECK((l.pattern_cache[position.index()]
			    == r.pattern_cache[position.index()]));
	  }
	}
	BOOST_CHECK(HashKey(*l.state) == HashKey(*r.state));
	BOOST_CHECK(*l.history == *r.history);
	BOOST_CHECK(l.pin_by_opposing_sliders == r.pin_by_opposing_sliders);
	BOOST_CHECK(l.king8_long_pieces == r.king8_long_pieces);
	BOOST_CHECK(l.threatened == r.threatened);
	BOOST_CHECK(l.long_attack_cache == r.long_attack_cache);
	BOOST_CHECK(l.attack_shadow == r.attack_shadow);
	BOOST_CHECK(l.progress16 == r.progress16);
	BOOST_CHECK(l.last_move_ptype5 == r.last_move_ptype5);
	BOOST_CHECK(l.last_add_effect == r.last_add_effect);
	BOOST_CHECK(l.pin == r.pin);
	BOOST_CHECK(l.threatmate_move == r.threatmate_move);
	BOOST_CHECK(l.sendoffs == r.sendoffs);
	BOOST_CHECK(l.exchange_pins == r.exchange_pins);
	BOOST_CHECK(l.move_candidate_exists == r.move_candidate_exists);
	BOOST_CHECK(HashKey(l.copy) == HashKey(r.copy));
      }
      BOOST_CHECK(info == info2);
    }
  }  
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
