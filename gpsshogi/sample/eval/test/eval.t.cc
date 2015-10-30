#include "moveData.h"
#include "eval/eval.h"
#include "eval/progress.h"
#include "eval/progressFeature.h"
#include "eval/openMidEnding.h"
#include "eval/progressEval.h"
#include "analyzer.h"
#include "osl/eval/pieceEval.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/eval/progressEval.h"
#include "osl/progress.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <valarray>

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::eval;
extern bool long_test;

static  int Gold() { return gpsshogi::PieceEval().value(GOLD); }
static  int Rook() { return gpsshogi::PieceEval().value(ROOK); }
static  int total(const gpsshogi::Eval& eval,
		  const std::vector<std::pair<int, double> >& diff) 
{
  int sum = 0;
  for (size_t i=0; i<diff.size(); ++i) {
    sum += (int)(eval.flatValue(diff[i].first)*diff[i].second);
  }
  return sum;
}
static  void testConsistentBase(boost::ptr_vector<gpsshogi::Eval> &evals,
				double error)
{
  for (size_t j=0; j<evals.size(); ++j) {
    evals[j].setRandom();
  }
  
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_REQUIRE(ifs);
  std::string file_name;
  for (int i=0;i<900 && (ifs >> file_name) ; i++)
  {
    if ((i % 100) == 0)
      std::cerr << '.';
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const auto record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;

    NumEffectState state(record.initialState());
    
    for (unsigned int i=0; i<moves.size(); i++) {
      const Move m = moves[i];
      state.makeMove(m);

      for (size_t j=0; j<evals.size(); ++j) {
	const int value = evals[j].eval(state);
	
	gpsshogi::MoveData data;
	const int& features_value = data.value;
	const auto& diffs = data.diffs;
	evals[j].features(state, data, 0);
#ifndef L1BALL_NO_SORT
	BOOST_REQUIRE(__gnu_cxx::is_sorted(diffs.begin(), diffs.end()));
#endif
	if (abs(value - total(evals[j], diffs)) > evals[j].roundUp()) {
	  std::cerr << "eval " << j << "\n" << state << m << " " << features_value << "\n";
	  for (size_t k=0; k<diffs.size(); ++k)
	    std::cerr << diffs[k].first << " "
		      << std::get<0>(evals[j].findFeature(diffs[k].first))
		      << " " << diffs[k].second
		      << "  " << evals[j].flatValue(diffs[k].first) << "\n";
	  std::cerr << "inequality " << value << " " << total(evals[j], diffs)
		    << ' ' << features_value << "\n";
	}
	BOOST_CHECK_SMALL(value - total(evals[j], diffs), std::max(2,evals[j].roundUp()));
	BOOST_CHECK_SMALL((double)(value - features_value), error);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(EvalTestStableOpenMidEndingEval)
{  
  gpsshogi::StableOpenMidEnding eval;
  {
    const char *filename = "../stable-eval.txt";
    eval.load(filename);
  }
  using namespace osl;
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_REQUIRE(ifs);
  std::string file_name;

  osl::eval::ml::OpenMidEndingEval::setUp();
  osl::progress::ml::NewProgress::setUp();

  BOOST_REQUIRE_EQUAL(osl::progress::ml::NewProgress::maxProgress(), eval.maxProgress());
  for (int i=0;i<(!long_test ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "")
      break;
    file_name = OslConfig::testCsaFile(file_name);
    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;
    NumEffectState state(record.initialState());
    osl::eval::ml::OpenMidEndingEval test_eval(state, false);
    for (unsigned int i=0; i<moves.size(); i++){
      const Move m = moves[i];
      state.makeMove(m);
      test_eval.update(state, m);
      // test_eval.debug();
      if (test_eval.openingValue() != eval.openingValue(state)) {
	std::cerr << "osl\n";
	test_eval.debug();
	std::cerr << "gpsshogi\n";
	eval.debug(state);
      }
      BOOST_REQUIRE_EQUAL(test_eval.openingValue(), eval.openingValue(state));
#if 0
      << state << m << " progress " << eval.progress(state)
      << " progress max " << eval.maxProgress();
#endif
      BOOST_REQUIRE_EQUAL(test_eval.midgameValue(), eval.midgameValue(state));
      // << state << m;
#ifdef EVAL_QUAD
      BOOST_REQUIRE_EQUAL(test_eval.midgame2Value(), eval.midgame2Value(state));
      // << state << m;
#endif
      BOOST_REQUIRE_EQUAL(test_eval.endgameValue(), eval.endgameValue(state));
      // << state << m;
      BOOST_REQUIRE_EQUAL(test_eval.progressIndependentValue(), eval.progressIndependentValue(state));
      // << state << m;
      if (abs(test_eval.value() - eval.eval(state)) > 2.0) {
	const int progress = eval.progress(state);
	MultiInt stage_value;
	stage_value[0] = eval.openingValue(state);
	stage_value[1] = eval.midgameValue(state);
	stage_value[2] = eval.midgame2Value(state);
	stage_value[3] = eval.endgameValue(state);
	int flat = test_eval.progressIndependentValueAdjusted
	  (eval.progressIndependentValue(state),progress,eval.maxProgress());
	BOOST_CHECK_SMALL(test_eval.value()
			  - eval.compose(flat, stage_value, progress),
			  2);
	//<< state << m;
      }
      else
	BOOST_CHECK_SMALL(test_eval.value() - eval.eval(state), 2);
      // << state << m;
    }
  }
  // additional positions
  {
    const NumEffectState state(CsaString(
				 "P1-KY *  *  *  * -OU * -KE-KY\n"
				 "P2 * -HI *  *  *  * -KI *  * \n"
				 "P3 *  * -KE-FU * -KI-GI-FU * \n"
				 "P4 * -GI-FU-KA-FU-FU-FU * -FU\n"
				 "P5-FU-FU *  *  *  *  * +FU * \n"
				 "P6 *  * +FU+FU+FU * +FU * +FU\n"
				 "P7+FU+FU+GI+KI+KA+FU+GI * +KY\n"
				 "P8 * +OU+KI *  *  *  *  * +HI\n"
				 "P9+KY+KE *  *  *  *  * +KE * \n"
				 "+\n").initialState());
    osl::eval::ml::OpenMidEndingEval test_eval(state, false);
    BOOST_CHECK_SMALL(test_eval.value() - eval.eval(state), 2);
    // << state;
  }
  {
    const NumEffectState state(CsaString(
				 "P1-KY-HI *  *  *  *  *  *  * \n"
				 "P2 *  *  *  * +GI *  *  *  * \n"
				 "P3-FU * -FU *  * +FU-KE * -KY\n"
				 "P4 *  * +FU *  * -KI-OU-FU-FU\n"
				 "P5 *  *  *  *  * -FU *  *  * \n"
				 "P6 * +GI * +KI+FU-GI-UM-KA * \n"
				 "P7+FU+FU *  * -FU * -TO * +KE\n"
				 "P8+OU+KI * +FU *  * -FU *  * \n"
				 "P9+KY+KE *  *  *  *  * +HI+KY\n"
				 "P+00KI00GI00KE00FU00FU\n"
				 "P-00FU00FU\n"
				 "+\n").initialState());
    osl::eval::ml::OpenMidEndingEval test_eval(state, false);
    BOOST_CHECK_SMALL(test_eval.value()- eval.eval(state), 2);
  }
  {
    const NumEffectState state(CsaString(
				 "P1 *  *  * +UM *  *  * -OU-KY\n"
				 "P2+TO *  *  *  *  * +UM *  * \n"
				 "P3 *  *  *  *  *  *  * -FU * \n"
				 "P4 *  * -FU *  * -FU-FU * -FU\n"
				 "P5 *  *  * -GI * -KE *  *  * \n"
				 "P6-KY+FU+FU *  *  * +FU+FU+FU\n"
				 "P7 *  *  *  *  *  * -KE *  * \n"
				 "P8 *  *  *  * +GI *  * +OU * \n"
				 "P9 *  *  *  *  * +KI * +KE+KY\n"
				 "P+00HI00HI00KI00KI00GI\n"
				 "P-00KI00GI00KE00KY00FU00FU00FU00FU00FU00FU00FU\n"
				 "-\n").initialState());
    osl::eval::ml::OpenMidEndingEval test_eval(state, false);
    BOOST_CHECK_SMALL(test_eval.value()- eval.eval(state), 2);
  }
}

BOOST_AUTO_TEST_CASE(EvalTestAllDifferential)
{
  gpsshogi::PieceEval eval;
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  * -KI *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::MoveData md;
    eval.features(state, md, 0);
  }
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU-FU * -FU-FU-FU\n"
				 "P4 *  *  *  *  * +KA *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  * +FU *  *  *  *  *  * \n"
				 "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
				 "P8 *  *  *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "P+00FU\n"
				 "-\n").initialState());
    gpsshogi::MoveData md;
    eval.features(state, md, 0);
  }
}

BOOST_AUTO_TEST_CASE(EvalTestConstruct)
{
  BOOST_REQUIRE(osl::eval::ml::OpenMidEndingEval::setUp());
  BOOST_REQUIRE(osl::progress::ml::NewProgress::setUp());
  gpsshogi::PieceEval eval;
  gpsshogi::RichEval reval(0);
  {
    const NumEffectState state((SimpleState(HIRATE)));
    BOOST_CHECK_EQUAL(0, eval.eval(state));
    BOOST_CHECK_EQUAL(0, reval.eval(state));
  }
  
  {
    const NumEffectState state(CsaString(
			      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			      "P2 * -HI *  *  *  *  * -KA * \n"
			      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			      "P4 *  *  *  *  *  *  *  *  * \n"
			      "P5 *  *  * -KI *  *  *  *  * \n"
			      "P6 *  *  *  *  *  *  *  *  * \n"
			      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			      "P8 * +KA *  *  *  *  * +HI * \n"
			      "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
			      "+\n").initialState());
    BOOST_CHECK_EQUAL(-Gold()*2, eval.eval(state));
    BOOST_CHECK_EQUAL(-Gold()*2, reval.eval(state));

    gpsshogi::MoveData data;
    reval.features(state, data, 0);
    BOOST_CHECK_EQUAL(-Gold()*2, static_cast<int>(data.value));
  }

  {
    const NumEffectState state(CsaString(
			      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			      "P2 *  *  *  *  *  *  * -KA * \n"
			      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			      "P4 *  *  *  *  *  *  *  *  * \n"
			      "P5 *  *  *  *  *  *  *  *  * \n"
			      "P6 *  *  *  *  *  *  *  *  * \n"
			      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			      "P8 * +KA *  *  *  *  * +HI * \n"
			      "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			      "P+00HI\n"
			      "+\n").initialState());
    BOOST_CHECK_EQUAL(Rook()*2, eval.eval(state));
    BOOST_CHECK_EQUAL(Rook()*2, reval.eval(state));
  }
}

BOOST_AUTO_TEST_CASE(EvalTestConsistent)
{
  boost::ptr_vector<gpsshogi::Eval> evals;
  evals.push_back(new gpsshogi::PieceEval);
  evals.push_back(new gpsshogi::RichEval(2));
  testConsistentBase(evals, 0.1);
}

BOOST_AUTO_TEST_CASE(EvalTestConsistentProgress)
{
  boost::ptr_vector<gpsshogi::Eval> evals;
#ifdef LEARN_TEST_PROGRESS
  evals.push_back(new gpsshogi::HandProgressFeatureEval());
  evals.push_back(new gpsshogi::EffectProgressFeatureEval());
#endif
  evals.push_back(new gpsshogi::OpenMidEndingForTest(1));
  evals.push_back(new gpsshogi::KOpenMidEnding());
  evals.push_back(new gpsshogi::StableOpenMidEnding());
  evals.push_back(new gpsshogi::OslOpenMidEnding());
  //evals.push_back(new gpsshogi::KProgressEval());
  testConsistentBase(evals, 0.1 * 16);
}

BOOST_AUTO_TEST_CASE(EvalTestConsistentUpdate)
{
  boost::ptr_vector<gpsshogi::Eval> evals;
  evals.push_back(new gpsshogi::PieceEval);
  evals.push_back(new gpsshogi::RichEval(2));
#ifdef LEARN_TEST_PROGRESS
  evals.push_back(new gpsshogi::HandProgressFeatureEval());
  evals.push_back(new gpsshogi::EffectProgressFeatureEval());
#endif
  evals.push_back(new gpsshogi::OpenMidEndingForTest(1));
  evals.push_back(new gpsshogi::KOpenMidEnding());
  evals.push_back(new gpsshogi::KProgressEval());

  std::vector<int> values(evals.size());
  for (size_t j=0; j<evals.size(); ++j) {
    evals[j].setRandom();
  }
  
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_REQUIRE(ifs);
  std::string file_name;
  for (int i=0;i<900 && (ifs >> file_name) ; i++)
  {
    if ((i % 100) == 0)
      std::cerr << '.';
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const auto record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;

    NumEffectState state(record.initialState());
    
    boost::ptr_vector<gpsshogi::EvalValueStack> eval_values;
    for (size_t j=0; j<evals.size(); ++j) {
      values[j] = evals[j].eval(state);
      eval_values.push_back(evals[j].newStack(state));
    }
    for (unsigned int i=0; i<moves.size(); i++) {
      const Move m = moves[i];
      state.makeMove(m);

      for (size_t j=0; j<evals.size(); ++j) {
	const int value = evals[j].eval(state);

	eval_values[j].push(state, m);
	BOOST_CHECK_EQUAL(value, eval_values[j].value());
	// << "eval " << j << "\n" << state << m << "\n";
      }
      if (! state.inCheck()) {
	state.changeTurn();
	for (size_t j=0; j<evals.size(); ++j) {
	  eval_values[j].push(state, Move::PASS(alt(state.turn())));
	  const int value = evals[j].eval(state);
	  BOOST_CHECK_EQUAL(value, eval_values[j].value());
	  // << " eval(pass) " << j << "\n" << state << m << "\n";
	  eval_values[j].pop();
	}
	state.changeTurn();
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(EvalTestSaveWeight)
{
  boost::ptr_vector<gpsshogi::Eval> evals;
  evals.push_back(new gpsshogi::PieceEval);
  evals.push_back(new gpsshogi::RichEval(2));
#ifdef LEARN_TEST_PROGRESS
  evals.push_back(new gpsshogi::HandProgressFeatureEval());
  evals.push_back(new gpsshogi::EffectProgressFeatureEval());
#endif
  evals.push_back(new gpsshogi::OpenMidEndingForTest(1));
  evals.push_back(new gpsshogi::KOpenMidEnding());
  evals.push_back(new gpsshogi::KProgressEval());

  std::vector<int> values(evals.size());
  for (size_t j=0; j<evals.size(); ++j) {
    evals[j].setRandom();
    std::valarray<double> weight(evals[j].dimension());
    evals[j].saveWeight(&weight[0]);
    for (size_t i=0; i<evals[j].dimension(); ++i)
      BOOST_REQUIRE_EQUAL(evals[j].flatValue(i), weight[i]);
  }  
}

BOOST_AUTO_TEST_CASE(EvalTestSymmetry)
{  
  boost::ptr_vector<gpsshogi::Eval> evals;
  evals.push_back(new gpsshogi::PieceEval);
  evals.push_back(new gpsshogi::RichEval(2));
#ifdef LEARN_TEST_PROGRESS
  evals.push_back(new gpsshogi::HandProgressFeatureEval());
  evals.push_back(new gpsshogi::EffectProgressFeatureEval());
#endif
  evals.push_back(new gpsshogi::OpenMidEndingForTest(1));
  evals.push_back(new gpsshogi::KOpenMidEnding());
  // evals.push_back(new gpsshogi::KProgressEval()); not symmetric yet

  for (size_t j=0; j<evals.size(); ++j) {
    evals[j].setRandom();
  }

  using namespace osl;
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_REQUIRE(ifs);
  std::string file_name;
  for (int i=0;i<(!long_test ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "")
      break;
    file_name = OslConfig::testCsaFile(file_name);
    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;
    NumEffectState state(record.initialState());
    for (unsigned int i=0; i<moves.size(); i++){
      const Move m = moves[i];
      state.makeMove(m);
      NumEffectState state_r(state.rotate180());
      for (size_t j=0; j<evals.size(); ++j) {
	if (evals[j].eval(state) != -evals[j].eval(state_r)) {
	  std::cerr << "asymmetry found " << j << "\n" << state
		    << m
		    << "\n";
	  evals[j].showEvalSummary(state);
	  std::cerr << "\n";
	  evals[j].showEvalSummary(state_r);
	}
	BOOST_CHECK_SMALL(evals[j].eval(state) - (-evals[j].eval(state_r)),
			  std::max(1,evals[j].roundUp()*2));
      }
    }
  } 
}

#ifdef LEARN_TEST_PROGRESS
BOOST_AUTO_TEST_CASE(EvalTestStableProgress)
{  
  gpsshogi::StableEffectProgressFeatureEval progress;
  {
    std::string filename = OslConfig::home();
    filename += "/data/progress.txt";
    progress.load(filename.c_str());
  }
  using namespace osl;
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_REQUIRE(ifs);
  std::string file_name;

  const int osl_progress_scale = osl::progress::ml::NewProgress::ProgressScale;

  osl::progress::ml::NewProgress::setUp();
  BOOST_REQUIRE_EQUAL(osl::progress::ml::NewProgress::maxProgress(),
		      progress.maxProgress() / osl_progress_scale); 
  for (int i=0;i<(!long_test ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "")
      break;
    file_name = OslConfig::testCsaFile(file_name);
    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;
    NumEffectState state(record.initialState());
    osl::progress::ml::NewProgress test_progress(state);
    for (unsigned int i=0; i<moves.size(); i++){
      const Move m = moves[i];
      state.makeMove(m);
      test_progress.update(state, m);
      BOOST_REQUIRE_EQUAL(test_progress.progress(),
		std::max(std::min(progress.progress(state),
				  progress.maxProgress() - osl_progress_scale), 0) / osl_progress_scale);
    }
  } 

  gpsshogi::StableOpenMidEnding eval;
  {
    const char *filename = "../stable-eval.txt";
    eval.load(filename);
  }
  BOOST_CHECK_SMALL(progress.pawnValue() / osl_progress_scale - eval.pawnValue(),
		    eval.pawnValue()/10);
}
#endif

BOOST_AUTO_TEST_CASE(EvalTestStableProgressEval)
{  
  osl::eval::ProgressEval::setUp();
  gpsshogi::StableProgressEval eval;
  {
    const char *filename = "../stable-progresseval.txt";
    eval.load(filename);
  }
  using namespace osl;
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_REQUIRE(ifs);
  std::string file_name;

  for (int i=0;i<(!long_test ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "")
      break;
    file_name = OslConfig::testCsaFile(file_name);
    const auto record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;
    NumEffectState state(record.initialState());
    osl::eval::ProgressEval osl_eval(state);
    for (unsigned int i=0; i<moves.size(); i++){
      const Move m = moves[i];
      state.makeMove(m);
      osl_eval.update(state, m);
      BOOST_REQUIRE_EQUAL(osl_eval.openingValue(), eval.openingValue(state));
      // << state << m << " progress " << eval.progress(state)
      // << " progress max " << eval.maxProgress();
      BOOST_REQUIRE_EQUAL(osl_eval.endgameValue(), eval.endgameValue(state));
      // << state << m;
      BOOST_CHECK_SMALL(osl_eval.value() - eval.eval(state), 2);
    }
  }
  // additional positions
  {
    const NumEffectState state(CsaString(
				 "P1-KY *  *  *  * -OU * -KE-KY\n"
				 "P2 * -HI *  *  *  * -KI *  * \n"
				 "P3 *  * -KE-FU * -KI-GI-FU * \n"
				 "P4 * -GI-FU-KA-FU-FU-FU * -FU\n"
				 "P5-FU-FU *  *  *  *  * +FU * \n"
				 "P6 *  * +FU+FU+FU * +FU * +FU\n"
				 "P7+FU+FU+GI+KI+KA+FU+GI * +KY\n"
				 "P8 * +OU+KI *  *  *  *  * +HI\n"
				 "P9+KY+KE *  *  *  *  * +KE * \n"
				 "+\n").initialState());
    osl::eval::ProgressEval osl_eval(state);
    BOOST_CHECK_SMALL(osl_eval.value()-eval.eval(state), 2);
  }
  {
    const NumEffectState state(CsaString(
				 "P1-KY-HI *  *  *  *  *  *  * \n"
				 "P2 *  *  *  * +GI *  *  *  * \n"
				 "P3-FU * -FU *  * +FU-KE * -KY\n"
				 "P4 *  * +FU *  * -KI-OU-FU-FU\n"
				 "P5 *  *  *  *  * -FU *  *  * \n"
				 "P6 * +GI * +KI+FU-GI-UM-KA * \n"
				 "P7+FU+FU *  * -FU * -TO * +KE\n"
				 "P8+OU+KI * +FU *  * -FU *  * \n"
				 "P9+KY+KE *  *  *  *  * +HI+KY\n"
				 "P+00KI00GI00KE00FU00FU\n"
				 "P-00FU00FU\n"
				 "+\n").initialState());
    osl::eval::ProgressEval osl_eval(state);
    BOOST_CHECK_SMALL(osl_eval.value() - eval.eval(state), 2);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
