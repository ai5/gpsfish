/* effect5x3.t.cc
 */
#include "osl/progress/effect5x3d.h"
#include "consistencyTest.h"
#include "osl/numEffectState.h"
#include "osl/bits/centering5x3.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <string>
#include <iostream>

using namespace osl;
using namespace osl::progress;

BOOST_AUTO_TEST_CASE(ProgressEffect5x3dTestConsistentUpdate)
{
  consistencyTestUpdate<Effect5x3d>();
}

static int slowMakeProgress(Player defense, const NumEffectState& state,
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
	  sum_effect += state.countEffect(defense, target) * 16;
	  sum_effect -= (state.countEffect(defense, target) *
			 std::abs(king.x() - x));
	}
    }
  return sum_effect / 2;
}

BOOST_AUTO_TEST_CASE(ProgressEffect5x3dTestMakeProgress)
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
    
    for (const Move m: record.moves) {
      state.makeMove(m);
      const Square bk=state.kingSquare(BLACK);
      const Square wk=state.kingSquare(WHITE);
      int w0=osl::progress::Effect5x3d::makeProgress(WHITE,state,wk);
      int w1=slowMakeProgress(WHITE,state,wk);
      BOOST_CHECK_EQUAL(w0,w1);
      int b0=osl::progress::Effect5x3d::makeProgress(BLACK,state,bk);
      int b1=slowMakeProgress(BLACK,state,bk);
      BOOST_CHECK_EQUAL(b0,b1);
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
