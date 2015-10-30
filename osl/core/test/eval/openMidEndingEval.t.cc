/* midgameEval.t.cc
 */
#include "osl/eval/openMidEndingEval.h"
#include "osl/eval/kingTable.h"
#include "osl/progress.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::ml;

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestInit)
{
  BOOST_CHECK(OpenMidEndingEval::setUp());
  BOOST_CHECK(osl::progress::ml::NewProgress::setUp());
  BOOST_CHECK(OpenMidEndingEval::infty() < EvalTraits<BLACK>::MAX_VALUE - 10); 
}

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestConsistentUpdate)
{
  OpenMidEndingEval::setRandom();

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 100 : 900) && (ifs >> filename) ; i++)
  {
    if ((i % 100) == 0)
      std::cerr << '.';
    if (filename == "") 
      break;
    filename = OslConfig::testCsaFile(filename);

    const RecordMinimal record=CsaFileMinimal(filename).load();
    NumEffectState state(record.initial_state);
    OpenMidEndingEval eval(state);
    
    for (auto move:record.moves)
    {
      state.makeMove(move);
      eval.update(state, move);	// update with new state
      const OpenMidEndingEval new_eval(state);
      if (new_eval.value() != eval.value()) {
	std::cerr << state << std::endl << move << std::endl;
      }
      BOOST_CHECK_EQUAL(new_eval.value(), eval.value());
    }
  }
}

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestSymmetry)
{
  using namespace osl;
  OpenMidEndingEval::setRandom();

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> filename) ; i++)
  {
    if (filename == "") 
      break;
    filename = OslConfig::testCsaFile(filename);

    const RecordMinimal record=CsaFileMinimal(filename).load();
    NumEffectState state(record.initial_state);
    OpenMidEndingEval eval(state);
    for (auto m:record.moves) {
      state.makeMove(m);
      eval.update(state, m);

      NumEffectState state_r(state.rotate180());
      OpenMidEndingEval eval_r(state_r);
      if (abs(eval.value() - -eval_r.value()) > 2)
      {
	OpenMidEndingEval eval_l(state);
	std::cerr << state << state_r
		  << m << std::endl
		  << eval.value() << " " << eval_r.value() << " " << eval_l.value() <<std::endl;
      }
      BOOST_CHECK(abs(eval.value() - -eval_r.value()) <= 2);
#if 0
      // King8 is not horizontally symmetrical.
      NumEffectState state_h(state.flipHorizontal());
      OpenMidEndingEval eval_h(state_h);
      if (eval.value() != eval_h.value())
      {
	std::cerr << state << state_h << "\n";
      }
      BOOST_CHECK_EQUAL(eval.value(), eval_h.value());
#endif
    }
  }
}

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestIcc)
{
  OpenMidEndingEval::setRandom();
  const NumEffectState state(CsaString(
			       "P1-KY-KE * -KI *  * -GI+RY-KY\n"
			       "P2 *  *  * -OU *  * +NK *  * \n"
			       "P3-FU * -GI-FU-FU-FU *  * -FU\n"
			       "P4 *  * -FU *  *  *  *  *  * \n"
			       "P5 *  *  *  * +KA *  *  *  * \n"
			       "P6 * -KE *  *  *  *  *  *  * \n"
			       "P7+FU * +FU+FU+FU+FU+FU * +FU\n"
			       "P8 *  * -TO+KI *  *  *  *  * \n"
			       "P9+KY+KE *  * +OU+KI+GI * +KY\n"
			       "P+00KA00KI00FU00FU\n"
			       "P-00HI00GI00FU00FU\n"
			       "+\n").initialState());
  const NumEffectState state_r(CsaString(
				 "P1-KY * -GI-KI-OU *  * -KE-KY\n"
				 "P2 *  *  *  *  * -KI+TO *  * \n"
				 "P3-FU * -FU-FU-FU-FU-FU * -FU\n"
				 "P4 *  *  *  *  *  *  * +KE * \n"
				 "P5 *  *  *  * -KA *  *  *  * \n"
				 "P6 *  *  *  *  *  * +FU *  * \n"
				 "P7+FU *  * +FU+FU+FU+GI * +FU\n"
				 "P8 *  * -NK *  * +OU *  *  * \n"
				 "P9+KY-RY+GI *  * +KI * +KE+KY\n"
				 "P+00HI00GI00FU00FU\n"
				 "P-00KA00KI00FU00FU\n"
				 "-\n").initialState());
  OpenMidEndingEval eval(state), eval_r(state_r);
  BOOST_CHECK(abs(eval.value() - -eval_r.value()) <= 2);
}

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestBasic)
{
  OpenMidEndingEval::setUp();
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
    OpenMidEndingEval eval(state);
    BOOST_CHECK(eval.value() > 0);
  }
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE *  * -OU *  * -KE-KY\n"
				 "P2 *  *  *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 * +GI *  *  *  *  * +GI * \n"
				 "P7+FU+KI+FU+FU+FU+FU+FU+KI+FU\n"
				 "P8 * +GI+KA * +OU * +HI+KI * \n"
				 "P9+KY+KI+KE *  *  * +KE+GI+KY\n"
				 "P+00HI00FU00FU\n"
				 "+\n").initialState());
    OpenMidEndingEval eval(state);
    BOOST_CHECK(eval.value() > 0);
  }
}

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestKing25EmptySquareNoEffect)
{
  OpenMidEndingEval::setUp();
  
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> filename) ; i++)
  {
    if (filename == "") 
      break;
    filename = OslConfig::testCsaFile(filename);

    const RecordMinimal record=CsaFileMinimal(filename).load();
    NumEffectState state(record.initial_state);
    CArray<int,2> last_opening, last_ending;
    for (auto m:record.moves) {
      state.makeMove(m);

      if (i == 0) 
      {
	std::pair<CArray<int,2>, CArray<int,2> > oe = King25EmptySquareNoEffect::eval
	  (state, King25EmptySquareNoEffectOpening::weights(),
	   King25EmptySquareNoEffectEnding::weights());
	BOOST_CHECK(oe.first == King25EmptySquareNoEffectOpening::eval(state));
	BOOST_CHECK(oe.second == King25EmptySquareNoEffectEnding::eval(state));
	last_opening = oe.first;
	last_ending = oe.second;
      } 
      else 
      {
	std::pair<CArray<int,2>, CArray<int,2> > oe = King25EmptySquareNoEffect::evalWithUpdate
	  (state, m, 
	   King25EmptySquareNoEffectOpening::weights(),
	   King25EmptySquareNoEffectEnding::weights(),
	   last_opening, last_ending);
	BOOST_CHECK(oe.first == King25EmptySquareNoEffectOpening::eval(state));
	BOOST_CHECK(oe.second == King25EmptySquareNoEffectEnding::eval(state));
	last_opening = oe.first;
	last_ending = oe.second;
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestCaptureValue)
{
  OpenMidEndingEval::setUp();
  for (int ptype=PTYPE_PIECE_MIN; ptype<=PTYPE_MAX; ++ptype) {
    PtypeO ptypeo = newPtypeO(BLACK, static_cast<Ptype>(ptype));
    BOOST_CHECK_EQUAL(OpenMidEndingEval::Piece_Value.captureValue(ptypeo)
			 * OpenMidEndingEval::seeScale(),
			 OpenMidEndingEval::captureValue(ptypeo));
    ptypeo = newPtypeO(WHITE, static_cast<Ptype>(ptype));
    BOOST_CHECK_EQUAL(OpenMidEndingEval::Piece_Value.captureValue(ptypeo)
			 * OpenMidEndingEval::seeScale(),
			 OpenMidEndingEval::captureValue(ptypeo));
  }
}

BOOST_AUTO_TEST_CASE(OpenMidEndingEvalTestArray)
{
  OpenMidEndingEval evals[3];
  (void)evals[0];
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
