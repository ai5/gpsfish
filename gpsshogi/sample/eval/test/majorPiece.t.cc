#include "eval/majorPiece.h"
#include "osl/record/csaRecord.h"

#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(NumPiecesBetweenBishopAndKingTestCountBetween)
{
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI * -KI * -KE-OU\n"
				 "P2 * -HI *  *  *  *  * -GI-KY\n"
				 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
				 "P4 *  *  *  *  *  *  *  *  * \n"
				 "P5-KA *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    BOOST_REQUIRE_EQUAL(1,
	      gpsshogi::NumPiecesBetweenBishopAndKingAll::countBetween(
		state,
		state.kingSquare<BLACK>(),
		state.pieceAt(Square(9, 5))));
    BOOST_REQUIRE_EQUAL(0,
	      gpsshogi::NumPiecesBetweenBishopAndKingSelf::countBetween(
		state,
		state.kingSquare<BLACK>(),
		state.pieceAt(Square(9, 5))));
    BOOST_REQUIRE_EQUAL(1,
	      gpsshogi::NumPiecesBetweenBishopAndKingOpp::countBetween(
		state,
		state.kingSquare<BLACK>(),
		state.pieceAt(Square(9, 5))));
    BOOST_REQUIRE_EQUAL(3,
	      gpsshogi::NumPiecesBetweenBishopAndKingAll::countBetween(
		state,
		state.kingSquare<WHITE>(),
		state.pieceAt(Square(8, 8))));
    BOOST_REQUIRE_EQUAL(1,
	      gpsshogi::NumPiecesBetweenBishopAndKingSelf::countBetween(
		state,
		state.kingSquare<WHITE>(),
		state.pieceAt(Square(8, 8))));
    BOOST_REQUIRE_EQUAL(2,
	      gpsshogi::NumPiecesBetweenBishopAndKingOpp::countBetween(
		state,
		state.kingSquare<WHITE>(),
		state.pieceAt(Square(8, 8))));
  }
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI * -KI * -KE-OU\n"
				 "P2 * -HI *  *  *  *  * -GI-KY\n"
				 "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
				 "P4 * -KA *  *  *  *  *  *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 * +KA *  *  *  *  *  *  * \n"
				 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				 "P8 *  *  *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    BOOST_REQUIRE_EQUAL(8,
	      gpsshogi::NumPiecesBetweenBishopAndKingAll::countBetween(
		state,
		state.kingSquare<WHITE>(),
		state.pieceAt(Square(8, 6))));
    BOOST_REQUIRE_EQUAL(8,
	      gpsshogi::NumPiecesBetweenBishopAndKingSelf::countBetween(
		state,
		state.kingSquare<WHITE>(),
		state.pieceAt(Square(8, 6))));
    BOOST_REQUIRE_EQUAL(8,
	      gpsshogi::NumPiecesBetweenBishopAndKingOpp::countBetween(
		state,
		state.kingSquare<WHITE>(),
		state.pieceAt(Square(8, 6))));
  }
}

BOOST_AUTO_TEST_CASE(BishopBishopPieceTestFeature)
{
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
				 "P4 *  *  *  *  *  * -FU *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::BishopBishopPiece bbp;
    gpsshogi::IndexCacheI<gpsshogi::MaxActiveWithDuplication> features;
    bbp.featuresOneNonUniq(state, features);
    BOOST_REQUIRE_EQUAL(1, features.size());
    BOOST_REQUIRE_EQUAL(bbp.index(osl::newPtypeO(BLACK, PAWN), true, true),
	      features[0].first);
  }
  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
				 "P4 *  *  *  *  *  * -FU *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  *  *  *  *  *  *  * \n"
				 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				 "P8 * +KA+GI *  *  *  * +HI * \n"
				 "P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::BishopBishopPiece bbp;
    gpsshogi::IndexCacheI<gpsshogi::MaxActiveWithDuplication> features;
    bbp.featuresOneNonUniq(state, features);
    BOOST_REQUIRE_EQUAL(1, features.size());
    BOOST_REQUIRE_EQUAL(features[0].first,
	      bbp.index(osl::newPtypeO(BLACK, PAWN), false, true));
  }

  {
    const NumEffectState state(CsaString(
				 "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
				 "P2 * -HI *  *  *  *  * -KA * \n"
				 "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
				 "P4 *  *  *  *  *  * -FU *  * \n"
				 "P5 *  *  *  *  *  *  *  *  * \n"
				 "P6 *  *  * +GI *  *  *  *  * \n"
				 "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				 "P8 * +KA *  *  *  *  * +HI * \n"
				 "P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
				 "+\n").initialState());
    gpsshogi::BishopBishopPiece bbp;
    gpsshogi::IndexCacheI<gpsshogi::MaxActiveWithDuplication> features;
    bbp.featuresOneNonUniq(state, features);
    BOOST_REQUIRE_EQUAL(0, features.size());
  }
}

BOOST_AUTO_TEST_CASE(AttackMajorsInBaseTestFeature)
{
  {
    const NumEffectState state(CsaString(
				 "P1-KY *  *  *  *  * -OU-KE-KY\n"
				 "P2 * -HI *  *  *  * -KI *  * \n"
				 "P3-FU * -KE * -GI * -GI-FU-FU\n"
				 "P4 * -KA+KA * -FU-FU *  *  * \n"
				 "P5 * -FU-FU *  *  *  *  *  * \n"
				 "P6 *  *  * +FU *  *  * +FU * \n"
				 "P7+FU+FU+GI+KI * +FU *  * +FU\n"
				 "P8 *  * +KI *  *  *  * +HI * \n"
				 "P9+KY+KE * +OU *  *  * +KE+KY\n"
				 "P+00KI00FU00FU\n"
				 "P-00GI00FU00FU00FU\n"
				 "+\n").initialState());
    gpsshogi::AttackMajorsInBase amib;
    gpsshogi::IndexCacheI<gpsshogi::MaxActiveWithDuplication> features;
    amib.featuresOneNonUniq(state, features);
    BOOST_REQUIRE_EQUAL(2, features.size());
    BOOST_REQUIRE_EQUAL(amib.index(ROOK, BISHOP, true, false, false), features[0].first);
    BOOST_REQUIRE_EQUAL(-1, features[0].second);
    BOOST_REQUIRE_EQUAL( 0, features[1].first);
    BOOST_REQUIRE_EQUAL(-1, features[1].second);
  }

  {
    const NumEffectState state(CsaString(
				 "P1-KY *  *  *  *  * -OU-KE-KY\n"
				 "P2 *  * -HI *  * -KI-KI *  * \n"
				 "P3-FU * -KE * -GI * -GI-FU-FU\n"
				 "P4 * -KA+KA * -FU-FU *  *  * \n"
				 "P5 * -FU-FU *  *  *  *  *  * \n"
				 "P6 *  *  * +FU *  *  * +FU * \n"
				 "P7+FU+FU+GI+KI * +FU *  * +FU\n"
				 "P8 *  * +KI *  *  *  * +HI * \n"
				 "P9+KY+KE * +OU *  *  * +KE+KY\n"
				 "P+00FU00FU\n"
				 "P-00GI00FU00FU00FU\n"
				 "+\n").initialState());
    gpsshogi::AttackMajorsInBase amib;
    gpsshogi::IndexCacheI<gpsshogi::MaxActiveWithDuplication> features;
    amib.featuresOneNonUniq(state, features);
    BOOST_REQUIRE_EQUAL(2, features.size());
    BOOST_REQUIRE_EQUAL(amib.index(PTYPE_EMPTY, BISHOP, false, false, false), features[0].first);
    BOOST_REQUIRE_EQUAL(-1, features[0].second);
    BOOST_REQUIRE_EQUAL( 0, features[1].first);
    BOOST_REQUIRE_EQUAL(-1, features[1].second);
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
