/* effect5x3.t.cc
 */
#include "osl/progress/effect5x3.h"
#include "consistencyTest.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <string>
#include <iostream>

using namespace osl;
using namespace osl::progress;

BOOST_AUTO_TEST_CASE(ProgressEffect5x3WithBonusTestConsistentUpdate)
{
  consistencyTestUpdate<Effect5x3WithBonus>();
}

static int slowMakeProgressArea(Player attack, const NumEffectState& state,
				  Square king)
{  
  const Square center = Centering5x3::adjustCenter(king);

  const int min_x = center.x() - 2;
  const int max_x = center.x() + 2;
  const int min_y = center.y() - 1;
  const int max_y = center.y() + 1;


  // 利き
  int sum_effect = 0;

  for (int x=min_x; x<=max_x; ++x)
    {
      for (int y=min_y; y<=max_y; ++y)
	{
	  const Square target(x,y);
	  sum_effect += state.countEffect(attack, target) * 16;
	  sum_effect -= (state.countEffect(attack, target) *
			 std::abs(king.x() - x));
	}
    }
  return sum_effect / 2;
}

BOOST_AUTO_TEST_CASE(ProgressEffect5x3WithBonusTestMakeProgress)
{
  using namespace osl;

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const auto record=CsaFileMinimal(file_name).load();
    NumEffectState state(record.initialState());
    
    for (auto m:record.moves) {
      state.makeMove(m);
      const Square bk=state.kingSquare(BLACK);
      const Square wk=state.kingSquare(WHITE);
      int w0=osl::progress::Effect5x3WithBonus::makeProgress(WHITE,state);
      int w1=slowMakeProgressArea(BLACK,state,wk)+
	osl::progress::Effect5x3WithBonus::makeProgressStand(BLACK,state);
      BOOST_CHECK_EQUAL(w0,w1);
      int b0=osl::progress::Effect5x3WithBonus::makeProgress(BLACK,state);
      int b1=slowMakeProgressArea(WHITE,state,bk)+
	osl::progress::Effect5x3WithBonus::makeProgressStand(WHITE,state);
      BOOST_CHECK_EQUAL(b0,b1);
    }
  }
}

BOOST_AUTO_TEST_CASE(ProgressEffect5x3WithBonusTestSymmetry)
{
  using namespace osl;

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const auto record=CsaFileMinimal(file_name).load();

    NumEffectState state(record.initialState());
    progress::Effect5x3WithBonus progress(state);
    for (const Move m: record.moves) {
      state.makeMove(m);
      progress.update(state, m);

      NumEffectState state_r(state.rotate180());
      progress::Effect5x3WithBonus progress_r(state_r);
      BOOST_CHECK_EQUAL(progress.progress16(BLACK).value(), progress_r.progress16(WHITE).value());
      BOOST_CHECK_EQUAL(progress.progress16(WHITE).value(), progress_r.progress16(BLACK).value());
      if (progress.progress16bonus(BLACK).value() != progress_r.progress16bonus(WHITE).value()
	  || progress.progress16bonus(WHITE).value() != progress_r.progress16bonus(BLACK).value()) 
      {
	std::cerr << "unsymmetry found\n" << state
		  << progress.progress16bonus(BLACK).value() << " != " << progress_r.progress16bonus(WHITE).value()
		  << " || "
		  << progress.progress16bonus(WHITE).value() << " != " << progress_r.progress16bonus(BLACK).value()
		  << "\n";
      }
      BOOST_CHECK_EQUAL(progress.progress16bonus(BLACK).value(), progress_r.progress16bonus(WHITE).value());
      BOOST_CHECK_EQUAL(progress.progress16bonus(WHITE).value(), progress_r.progress16bonus(BLACK).value());
    }
  }
}

BOOST_AUTO_TEST_CASE(ProgressEffect5x3WithBonusTestCountEffectPieces)
{
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 *  *  *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			   "P4 *  *  *  *  *  * -FU *  * \n"
			   "P5 *  *  * -HI * +FU *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU * +FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "+\n").initialState());
    Effect5x3WithBonus progress(state);
    BOOST_CHECK_EQUAL(0, progress.countEffectPieces(state, BLACK));
    BOOST_CHECK_EQUAL(2, progress.countEffectPieces(state, WHITE));
  }

  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-KI-KI * +RY *  *  * \n"
			   "P2-KY-GI-KI *  *  *  * -FU-KY\n"
			   "P3-FU-FU * -KY * +TO *  *  * \n"
			   "P4 *  * -FU-FU * +KA *  * -FU\n"
			   "P5 *  *  *  * +GI *  * -KE * \n"
			   "P6+FU+UM+FU * +FU *  *  * +FU\n"
			   "P7 * +FU * +FU *  *  *  *  * \n"
			   "P8+KY+GI+GI *  *  *  *  *  * \n"
			   "P9+OU+KE+KI *  *  * -RY *  * \n"
			   "P+00KE00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    Effect5x3WithBonus progress(state);
    BOOST_CHECK_EQUAL(3, progress.countEffectPieces(state, BLACK));
    BOOST_CHECK_EQUAL(1, progress.countEffectPieces(state, WHITE));
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
