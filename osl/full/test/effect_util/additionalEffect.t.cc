#include "osl/additionalEffect.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>
using namespace osl;
using namespace osl::effect_util;

BOOST_AUTO_TEST_CASE(AdditionalEffectTestHasEffect)
{
  NumEffectState state(CsaString(
			 "P1+UM *  *  *  *  * -OU * -KY\n"
			 "P2 *  * -HI-KI * -GI-KI *  * \n"
			 "P3 *  *  * -FU-GI-FU *  * -KE\n"
			 "P4-FU *  *  * -FU * -FU-FU-FU\n"
			 "P5 * +KE * +FU * +FU *  *  * \n"
			 "P6+FU * -FU+KI+FU * +GI * +FU\n"
			 "P7 * +FU *  *  *  * +KE *  * \n"
			 "P8 * +OU+KI *  *  *  *  * +HI\n"
			 "P9+KY *  *  *  *  *  *  * +KY\n"
			 "P+00KA00KE00KY00FU\n"
			 "P-00GI00FU00FU00FU\n"
			 "-\n").initialState());
  BOOST_CHECK_EQUAL(true, AdditionalEffect::hasEffect
		       (state, Square(7,7), WHITE));
  BOOST_CHECK_EQUAL(false, AdditionalEffect::hasEffect
		       (state, Square(7,7), BLACK));

  BOOST_CHECK_EQUAL(false, AdditionalEffect::hasEffect
		       (state, Square(8,8), WHITE));
  BOOST_CHECK_EQUAL(true, AdditionalEffect::hasEffect
		       (state, Square(8,8), BLACK));
}

static void testFile(const std::string& filename)
{
  const auto record = CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  const auto& moves = record.moves;
  for (unsigned int i=0; i<moves.size(); ++i)
  {
    const Player turn = state.turn();
    const Square target = state.kingSquare(turn);

    // 8近傍の利きの数をカウント
    int kagekiki = 0;
    for (int x=-1; x<2; x++)
    {
      for (int y=-1; y<2; y++){
	Square pos = Square(target.x()+x, target.y()+y);
	if (pos.isOnBoard())
	  kagekiki += AdditionalEffect::count(state, turn, pos);
      }
    }

    for (int x=1; x<=9; x++) {
      for (int y=1; y<=9; y++) {
	const Square p(x,y);
	BOOST_CHECK_EQUAL(AdditionalEffect::hasEffect(state, p, BLACK),
			     AdditionalEffect::hasEffectStable(state, p, BLACK));
	BOOST_CHECK_EQUAL(AdditionalEffect::hasEffect(state, p, WHITE),
			     AdditionalEffect::hasEffectStable(state, p, WHITE));
      }
    }
    state.makeMove(moves[i]);
  }
}

BOOST_AUTO_TEST_CASE(AdditionalEffectTestFiles)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  const int count = OslConfig::inUnitTestShort() ? 10 : 100;
  std::string filename;
  while ((ifs >> filename) && (++i<count)) {
    if (filename == "") 
      break;
    testFile(OslConfig::testCsaFile(filename));
  }
}

BOOST_AUTO_TEST_CASE(AdditionalEffectTestZero)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI * +OU * -GI-KE * \n"
			 "P2 * -HI-KI *  *  * -KI-KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  * -KY *  *  *  * \n"
			 "P6 *  *  *  *  *  *  *  *  * \n"
			 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA+KI *  *  * +KI+HI * \n"
			 "P9+KY+KE+GI * -OU * +GI+KE+KY\n"
			 "+\n").initialState());
  BOOST_CHECK_EQUAL(0, AdditionalEffect::count(state, WHITE, 
						Square(5,2)));
  BOOST_CHECK_EQUAL(0, AdditionalEffect::count(state, BLACK, 
						Square(5,2)));
  BOOST_CHECK_EQUAL(0, AdditionalEffect::count(state, BLACK, 
						Square(6,6)));
  BOOST_CHECK_EQUAL(0, AdditionalEffect::count(state, BLACK, 
						Square(7,6)));
  BOOST_CHECK_EQUAL(0, AdditionalEffect::count(state, BLACK,
						Square(8,4)));
}

BOOST_AUTO_TEST_CASE(AdditionalEffectTestOne)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI * +OU * -GI-KE * \n"
			 "P2 * -HI-KI *  *  * -KI-KA * \n"
			 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  * -KY *  *  *  * \n"
			 "P6 *  * +FU *  *  *  *  *  * \n"
			 "P7+FU+FU+KI+FU+FU+FU+FU+FU+FU\n"
			 "P8 * +KA *  *  *  * +KI+HI * \n"
			 "P9+KY+KE+GI * -OU * +GI+KE+KY\n"
			 "+\n").initialState());
  BOOST_CHECK_EQUAL(1, AdditionalEffect::count(state, BLACK, 
						Square(6,6)));
  BOOST_CHECK_EQUAL(1, AdditionalEffect::count(state, WHITE, 
						Square(6,2)));
}

BOOST_AUTO_TEST_CASE(AdditionalEffectTestPile)
{
  NumEffectState state(CsaString(
			 "P1-KY-KE-GI * +OU * -GI-KE * \n"
			 "P2 *  * -KI *  *  * -KI * -KY\n"
			 "P3-FU-FU-FU-FU-FU-FU-FU-FU * \n"
			 "P4 *  *  *  *  * +GI *  * -FU\n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  * +FU+KA *  *  *  * +FU\n"
			 "P7+FU+FU * +FU+FU+FU+FU+FU+HI\n"
			 "P8 * +KA *  *  *  * +KI * +KY\n"
			 "P9+KY+KE * +KI-OU * +GI+KE-HI\n"
			 "+\n").initialState());
  // KYO, HI
  BOOST_CHECK_EQUAL(2, AdditionalEffect::count(state, BLACK, 
						Square(1,5)));
  BOOST_CHECK_EQUAL(2, AdditionalEffect::count(state, WHITE, 
						Square(1,5)));
  BOOST_CHECK_EQUAL(2, AdditionalEffect::count(state, BLACK, 
						Square(3,3)));
}

BOOST_AUTO_TEST_CASE(AdditionalEffectTestCount2)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  *  * -KY\n"
			   "P2 * -HI *  *  * -GI-KI-OU * \n"
			   "P3 *  * -KE * -KA-KI-KE-FU+FU\n"
			   "P4 * -GI-FU-FU-FU-FU *  *  * \n"
			   "P5-FU-FU *  *  *  * -FU+FU+HI\n"
			   "P6 *  * +FU+FU+FU+FU *  *  * \n"
			   "P7+FU+FU+GI+KI+GI * +KE *  * \n"
			   "P8 * +OU+KI *  *  *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00FU\n"
			   "P-00KA00FU\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(2, AdditionalEffect::count2(state, Square(1,2),
						     BLACK));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI *  *  *  * +RY\n"
			   "P2 * -OU-GI-KI *  *  *  *  * \n"
			   "P3 *  * -FU-FU * -FU * -FU * \n"
			   "P4-FU+FU *  * -FU-GI *  * +FU\n"
			   "P5 *  *  *  *  *  * -FU+FU * \n"
			   "P6+FU *  *  * +FU+FU *  *  * \n"
			   "P7 * +GI+KE+FU+KA * +FU *  * \n"
			   "P8 * +KY+OU+GI *  *  *  *  * \n"
			   "P9+KY *  *  * +KI *  *  * -RY\n"
			   "P+00KE\n"
			   "P-00KA00KI00KE00KY00FU00FU00FU\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(0, AdditionalEffect::count2(state, Square(8,3),
						     BLACK));
  }
}
BOOST_AUTO_TEST_CASE(AdditionalEffectTestCount)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  *  *  * \n"
			   "P2 * -OU-GI-KI-KI *  *  *  * \n"
			   "P3+FU-FU-FU *  *  * -KE *  * \n"
			   "P4-FU *  * -FU *  *  *  * -HI\n"
			   "P5 *  *  * +KE * -FU * -GI-KY\n"
			   "P6 *  * +FU+GI+HI * +FU-FU-FU\n"
			   "P7 * +FU+KA *  * +FU *  *  * \n"
			   "P8 *  *  *  * +KI * +GI+OU+FU\n"
			   "P9-UM *  *  *  * +KI * +KE+KY\n"
			   "P+00FU\n"
			   "P-00KY00FU00FU00FU00FU\n"
			   "-\n"
			   ).initialState());
    BOOST_CHECK_EQUAL(2, AdditionalEffect::count(state, WHITE,Square(1,7)));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  *  *  * \n"
			   "P2 * -OU-GI-KI-KI *  *  *  * \n"
			   "P3+FU-FU-FU *  *  * -KE * -KY\n"
			   "P4-FU *  * -FU *  *  *  * -HI\n"
			   "P5 *  *  * +KE * -FU * -GI-KY\n"
			   "P6 *  * +FU+GI+HI * +FU-FU-FU\n"
			   "P7 * +FU+KA *  * +FU *  *  * \n"
			   "P8 *  *  *  * +KI * +GI+OU+FU\n"
			   "P9-UM *  *  *  * +KI * +KE+KY\n"
			   "P+00FU\n"
			   "P-00FU00FU00FU00FU\n"
			   "-\n"
			   ).initialState());
    BOOST_CHECK_EQUAL(3, AdditionalEffect::count(state, WHITE,Square(1,7)));
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
