#include "osl/effect_util/neighboring8Direct.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;

BOOST_AUTO_TEST_CASE(Neighboring8DirectTestBishop)
{
  const NumEffectState state(CsaString(
			    "P1-KY+HI *  *  *  *  * -KE-KY\n"
			    "P2 *  * +UM-FU *  *  *  *  * \n"
			    "P3-KE-KI *  * +UM-GI-FU+FU-FU\n"
			    "P4 * -GI-OU * -FU *  * -FU * \n"
			    "P5-FU *  *  *  * +GI *  *  * \n"
			    "P6 * +FU-FU *  *  *  *  *  * \n"
			    "P7+FU *  *  *  *  * +FU * +FU\n"
			    "P8+KY+KI+KI *  *  *  *  *  * \n"
			    "P9+OU+KE *  *  *  *  * +KE+KY\n"
			    "P+00KI\n"
			    "P-00HI00GI00FU00FU00FU00FU00FU00FU\n"
			    "-\n").initialState());
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,PBISHOP), Square(7,2), Square(7,4)));
  
}


BOOST_AUTO_TEST_CASE(Neighboring8DirectTestRook)
{
  const NumEffectState state(CsaString("P1-KY-KE *  *  *  *  * -KE-KY\n"
				    "P2 *  *  *  *  *  * -KI * -OU\n"
				    "P3 *  * -FU *  * -KI *  * -KA\n"
				    "P4-FU-HI *  * -FU * -FU * -GI\n"
				    "P5 * -FU+FU * +GI-FU * -FU * \n"
				    "P6+FU * +HI *  *  *  *  * -FU\n"
				    "P7 * +FU * -TO * +FU+FU+FU * \n"
				    "P8+KY *  *  *  * +KI+KI+GI+KY\n"
				    "P9 * +KE *  * +KA *  * +KE+OU\n"
				    "P-00FU00FU00FU\n"
				    "P+00GI\n"
				    "+").initialState());
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,ROOK), Square(1,6), Square(1,9)));
  
}

BOOST_AUTO_TEST_CASE(Neighboring8DirectTestLance)
{
  {
    const NumEffectState state(CsaString(
			      "P1-KY-KE *  *  *  *  *  * -KY\n"
			      "P2 *  *  *  *  *  * -KI *  * \n"
			      "P3 *  * -FU *  * -KI *  * -OU\n"
			      "P4-FU-HI *  * -FU * -FU *  * \n"
			      "P5 * -FU+FU * +GI-FU * -FU+KY\n"
			      "P6+FU *  *  *  *  * +KE *  * \n"
			      "P7 * +FU * -TO * +FU+FU+FU * \n"
			      "P8+KY *  *  *  * +KI+KI+GI * \n"
			      "P9 * +KE-UM * +KA *  * +KE+OU\n"
			      "P+00GI00FU\n"
			      "P-00HI00GI00FU00FU00FU\n"
			      "-\n").initialState());
    BOOST_CHECK(Neighboring8Direct::hasEffect
		   (state, newPtypeO(BLACK,LANCE), Square(1,5), Square(1,3)));
  }
  {
    const NumEffectState state(CsaString(
			      "P1-KY-KE *  *  *  *  *  * -KY\n"
			      "P2 *  *  *  *  *  * -KI *  * \n"
			      "P3 *  * -FU *  * -KI *  * -OU\n"
			      "P4-FU-HI *  * -FU * -FU *  * \n"
			      "P5 * -FU+FU * +GI-FU * -FU+KY\n"
			      "P6+FU *  *  *  *  * +KE *  * \n"
			      "P7 * +FU * -TO * +FU+FU+FU * \n"
			      "P8+KY *  *  *  * +KI+KI+GI * \n"
			      "P9 * +KE-UM * +KA *  * +KE+OU\n"
			      "P+00GI00FU\n"
			      "P-00HI00GI00FU00FU00FU\n"
			      "-\n").initialState());
    BOOST_CHECK(! Neighboring8Direct::hasEffect
		   (state, newPtypeO(BLACK,LANCE), Square(1,5), Square(1,6)));
  }
  {
    const NumEffectState state(CsaString(
			      "P1-KY-KE *  *  *  *  *  * -KY\n"
			      "P2 *  *  *  *  *  * -KI * -OU\n"
			      "P3 *  * -FU *  * -KI *  *  * \n"
			      "P4-FU-HI *  * -FU * -FU *  * \n"
			      "P5 * -FU+FU * +GI-FU * -FU+KY\n"
			      "P6+FU *  *  *  *  * +KE *  * \n"
			      "P7 * +FU * -TO * +FU+FU+FU * \n"
			      "P8+KY *  *  *  * +KI+KI+GI * \n"
			      "P9 * +KE-UM * +KA *  * +KE+OU\n"
			      "P+00GI00FU\n"
			      "P-00HI00GI00FU00FU00FU\n"
			      "-\n").initialState());
    BOOST_CHECK(Neighboring8Direct::hasEffect
		   (state, newPtypeO(BLACK,LANCE), Square(1,5), Square(1,2)));
  }
  {
    const NumEffectState state(CsaString(
			      "P1-KY-KE *  *  *  *  *  * -KY\n"
			      "P2 *  *  *  *  *  * -KI-OU * \n"
			      "P3 *  * -FU *  * -KI *  *  * \n"
			      "P4-FU-HI *  * -FU * -FU *  * \n"
			      "P5 * -FU+FU * +GI-FU * -FU+KY\n"
			      "P6+FU *  *  *  *  * +KE *  * \n"
			      "P7 * +FU * -TO * +FU+FU+FU * \n"
			      "P8+KY *  *  *  * +KI+KI+GI * \n"
			      "P9 * +KE-UM * +KA *  * +KE+OU\n"
			      "P+00GI00FU\n"
			      "P-00HI00GI00FU00FU00FU\n"
			      "-\n").initialState());
    BOOST_CHECK(Neighboring8Direct::hasEffect
		   (state, newPtypeO(BLACK,LANCE), Square(1,5), Square(2,2)));
  }
  
}

BOOST_AUTO_TEST_CASE(Neighboring8DirectTestFindNearest)
{
  {
    const NumEffectState state(CsaString(
			      "P1-KY-KE-KI-GI * +HI-FU *  * \n"
			      "P2 * -OU-GI * -FU *  *  *  * \n"
			      "P3 *  * -FU-FU-KI *  *  * -FU\n"
			      "P4 *  *  *  *  *  *  *  *  * \n"
			      "P5-FU * +FU *  *  *  *  *  * \n"
			      "P6 *  *  *  * +UM *  *  *  * \n"
			      "P7+FU+FU+KE *  * +FU *  * +FU\n"
			      "P8 *  * +OU * +FU *  *  *  * \n"
			      "P9+KY * +GI+KI *  *  *  * +KY\n"
			      "P+00KA00KI00GI00FU00FU00FU00FU00FU\n"
			      "P-00HI00KE00KE00KY00FU\n"
			      "+\n").initialState());
    BOOST_CHECK_EQUAL(Square(8,3),
			 Neighboring8Direct::findNearest
			 (state, newPtypeO(BLACK,BISHOP), Square(6,5), Square(8,2)));
    BOOST_CHECK_EQUAL(Square::STAND(),
			 Neighboring8Direct::findNearest
			 (state, newPtypeO(BLACK,BISHOP), Square(3,6), Square(8,2)));
  }
}

BOOST_AUTO_TEST_CASE(Neighboring8DirectTestTable) 
{
  const NumEffectState state;
  // gold
  // direct
  const Square white_king = Square(5,1);
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,GOLD), Square(4,2), white_king));
  // 3x3
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,GOLD), Square(3,2), white_king));
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(WHITE,GOLD), Square(4,2), white_king));
  // out
  BOOST_CHECK(!Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,GOLD), Square(2,2), white_king));
  BOOST_CHECK(!Neighboring8Direct::hasEffect
		 (state, newPtypeO(WHITE,GOLD), Square(3,3), white_king));

  // black king
  const Square black_king = Square(5,9);
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,GOLD), Square(4,8), black_king));
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(WHITE,GOLD), Square(3,8), black_king));
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(WHITE,GOLD), Square(4,8), black_king));
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(WHITE,GOLD), Square(3,7), black_king));
  BOOST_CHECK(!Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,GOLD), Square(3,7), black_king));
  
  // lance
  BOOST_CHECK(! Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,LANCE), Square(4,7), black_king));
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(WHITE,LANCE), Square(4,7), black_king));
  BOOST_CHECK(! Neighboring8Direct::hasEffect
		 (state, newPtypeO(WHITE,LANCE), Square(4,6), black_king));

  // bishop
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,BISHOP), Square(6,3), white_king));
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,BISHOP), Square(8,3), white_king));
  BOOST_CHECK(!Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,BISHOP), Square(9,4), white_king));

  // rook
  BOOST_CHECK(Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,ROOK), Square(1,4), Square(1,7)));

  // nearestがはみだす
  BOOST_CHECK(!Neighboring8Direct::hasEffect
		 (state, newPtypeO(BLACK,BISHOP), Square(8,8), black_king));
}

BOOST_AUTO_TEST_CASE(Neighboring8DirectTestEqual)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int max=100;
  if (OslConfig::inUnitTestShort())
      max=10;
  std::string filename;
  int i=0;
  while((ifs >> filename) && (++i<max)) 
  {
    std::string full_filename = OslConfig::testCsaFile(filename);

    auto record=CsaFileMinimal(full_filename).load();
    NumEffectState state(record.initialState());
    const auto& moves=record.moves;
    for (size_t i=0; i<moves.size(); ++i)
    {
      const PtypeO ptypeo = moves[i].ptypeO();
      const Square from = moves[i].to();
      const Square to = state.kingSquare(BLACK);

      const bool stable 
	= Neighboring8Direct::hasEffectNaive(state, ptypeo, from, to);
      const bool current 
	= Neighboring8Direct::hasEffect(state, ptypeo, from, to);
      if (stable != current)
      {
	// 以前の実装は to だけに利きをつける手を除いていた．例穴熊の銀の守り
	if (! state.hasEffectIf(ptypeo, from, to))
	{
	  std::cerr << state << moves[i] << "\n";
	  BOOST_CHECK_EQUAL(stable, current);
	}
      }
      state.makeMove(moves[i]);
    }
  }
}

BOOST_AUTO_TEST_CASE(Neighboring8DirectTestAdditional)
{
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5 *  *  *  * +GI *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				 "P8 *  *  *  *  *  *  * +HI * \n"
				 "P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
				 "P+00KA\n"
				 "+\n").initialState());
    BOOST_CHECK(Neighboring8Direct::hasEffectOrAdditional
		   (state, newPtypeO(BLACK,BISHOP), Square(6,6), Square(4,4)));
    BOOST_CHECK(Neighboring8Direct::hasEffectOrAdditional
		   (state, newPtypeO(BLACK,BISHOP), Square(6,6), Square(4,5)));
    BOOST_CHECK(Neighboring8Direct::hasEffectOrAdditional
		   (state, newPtypeO(BLACK,BISHOP), Square(6,6), Square(8,8)));

    BOOST_CHECK(Neighboring8Direct::hasEffectOrAdditional
		   (state, newPtypeO(BLACK,BISHOP), Square(6,6), Square(3,3)));
    BOOST_CHECK(! Neighboring8Direct::hasEffectOrAdditional
		   (state, newPtypeO(BLACK,BISHOP), Square(6,6), Square(2,3)));
    BOOST_CHECK(! Neighboring8Direct::hasEffectOrAdditional
		   (state, newPtypeO(BLACK,BISHOP), Square(6,6), Square(9,9)));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
