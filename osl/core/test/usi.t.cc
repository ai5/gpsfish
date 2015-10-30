/* usi.t.cc
 */
#include "osl/usi.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(UsiTestMove)
{
  NumEffectState state;
  const Move m76fu = Move(Square(7,7), Square(7,6), PAWN, 
			  PTYPE_EMPTY, false, BLACK);
  const std::string usi76fu = "7g7f";
  const std::string usi76fu2 = usi::show(m76fu);
  BOOST_CHECK_EQUAL(usi76fu, usi76fu2);

  const Move m76fu2 = usi::strToMove(usi76fu, state);
  BOOST_CHECK_EQUAL(m76fu, m76fu2);

  const std::string usi_win = "win";
  const Move win = usi::strToMove(usi_win, state);
  BOOST_CHECK_EQUAL(win, Move::DeclareWin());
}

BOOST_AUTO_TEST_CASE(UsiTestParseBoard)
{
  NumEffectState state;
  const std::string hirate = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL";

  NumEffectState state2;
  usi::parseBoard(hirate, state2);
  BOOST_CHECK_EQUAL(HashKey(state), HashKey(state2));
}

BOOST_AUTO_TEST_CASE(UsiTestParseBoard2)
{
  NumEffectState state;
  const std::string hirate = "8l/1l+R2P3/p2pBG1pp/kps1p4/Nn1P2G2/P1P1P2PP/1PS6/1KSG3+r1/LN2+p3L";

  NumEffectState state2;
  BOOST_CHECK_NO_THROW(usi::parseBoard(hirate, state2));
}

BOOST_AUTO_TEST_CASE(UsiTestParse)
{
  {
    NumEffectState state(CsaString(
			"P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			"P2 * -HI *  *  *  *  * -KA * \n"
			"P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			"P4 *  *  *  *  *  * -FU *  * \n"
			"P5 *  *  *  *  *  *  *  *  * \n"
			"P6 *  * +FU *  *  *  * +FU * \n"
			"P7+FU+FU * +FU+FU+FU+FU * +FU\n"
			"P8 * +KA *  *  *  *  * +HI * \n"
			"P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			"-\n").initialState());

    const std::string s763426 = "startpos moves 7g7f 3c3d 2g2f";

    NumEffectState state2;
    usi::parse(s763426, state2);
    BOOST_CHECK_EQUAL(HashKey(state), HashKey(state2));
  }
  {
    NumEffectState state(CsaString(
			"P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			"P2 * -HI *  *  *  *  *  *  * \n"
			"P3 *  *  * -FU-FU-FU-FU-FU-FU\n"
			"P4 *  *  *  *  *  *  *  *  * \n"
			"P5 *  *  *  *  *  *  *  *  * \n"
			"P6 *  *  *  *  *  *  *  *  * \n"
			"P7+FU+FU+FU+FU+FU+FU+FU *  * \n"
			"P8 * +KA *  *  *  *  * +HI * \n"
			"P9+KY+KE * +KI+OU+KI+GI+KE+KY\n"
			"P+00GI00FU00FU\n"
			"P-00KA00FU00FU00FU\n"
			"-\n").initialState());

    const std::string stest = "sfen lnsgkgsnl/1r7/3pppppp/9/9/9/PPPPPPP2/1B5R1/LN1GKGSNL w S2Pb3p 1";

    NumEffectState state2;
    usi::parse(stest, state2);
    BOOST_CHECK_EQUAL(state, static_cast<const NumEffectState&>(state2));

    const std::string stest3 = "sfen lnsgkgsnl/1r7/3pppppp/9/9/9/PPPPPPP2/1B5R1/LN1GKGSNL w S2Pb3p";
    NumEffectState state3;
    usi::parse(stest3, state3);
    BOOST_CHECK_EQUAL(state, static_cast<const NumEffectState&>(state3));
  }
}

BOOST_AUTO_TEST_CASE(UsiTestParseBoardError)
{
  NumEffectState state;

  BOOST_CHECK_THROW(usi::parseBoard("lnsgkgsna/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL", state), // invalid ascii character a
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("", state), // empty
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL+", state), // ends with +
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("8l/1l+R2P3/p2pBG1pp/kps1p4/Nn1P2G2/P1P1P2PP/1PS6/1KSG3+r1/LN2+3L", state), // should be ...+p3L
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("lnsg+kgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL", state), // k can not be promoted
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("lnsgkgsnl/1r5b2/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL", state), // too many columns at the second row
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("lnsgkgsnl/1r5b1/pppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL", state), // too many columns at the third row
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("lnsgkgsnl/1r5b10/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL", state), // 0 is invalid
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parseBoard("lnsgkgsn?/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL", state), // ? is invalid
    osl::usi::ParseError);
}

BOOST_AUTO_TEST_CASE(UsiTestParseError)
{
  NumEffectState state;

  BOOST_CHECK_THROW(usi::parse("lnsgkgsnl/1r7/3pppppp/9/9/9/PPPPPPP2/1B5R1/LN1GKGSNL w S2Pb3p 1", state), // should start with sfen
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parse("sfen lnsgkgsnl/1r7/3pppppp/9/9/9/PPPPPPP2/1B5R1/LN1GKGSNL l S2Pb3p 1", state), // should be black or white
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parse("sfen lnsgkgsnl/1r7/3pppppp/9/9/9/PPPPPPP2/1B5R1/LN1GKGSNL w S2Ab3p 1", state), // invalid ascii character in hands
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parse("sfen lnsgkgsnl/1r7/3pppppp/9/9/9/PPPPPPP2/1B5R1/LN1GKGSNL w S2#b3p 1", state), // invalid character in hands
    osl::usi::ParseError);
  BOOST_CHECK_THROW(usi::parse("sfen lnsgkgsnl/1r7/3pppppp/9/9/9/PPPPPPP2/1B5R1/LN1GKGSNL w S0Pb3p 1", state), // 0 is invalid
    osl::usi::ParseError);
  BOOST_CHECK_NO_THROW(usi::parse("sfen lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL w - a", state)); // invalid moves are not respected
}

BOOST_AUTO_TEST_CASE(UsiTestKingAbsence)
{
#ifdef ALLOW_KING_ABSENCE
  NumEffectState state;
  usi::parse("sfen 3l+R4/1l1g5/2k1n4/1p+p2s3/4+r+B3/1G1s+p4/9/2N2+p3/9 b BGS2NPgs2l13p 1", state);
  usi::parse("sfen 6n2/+PS1+Ps1k2/3s+B1l2/1Lp1G1p1S/3p1n1+R+p/3+p1+nG2/4Pp1G1/6G+r1/5L1N+B b 2Pl7p 1", state);
#endif
}

BOOST_AUTO_TEST_CASE(UsiTestShowSquare)
{
  {
    NumEffectState state;
    BOOST_CHECK_EQUAL(std::string("startpos"), usi::show(state));
  }
  {
    std::string str = "sfen kng5l/ls1S2r2/pppPp2Pp/7+B1/7p1/2Pp2P2/PP3P2P/L2+n5/K1G5L b B2S2N2Pr2gp 1";
    NumEffectState state(CsaString(
			"P1-OU-KE-KI *  *  *  *  * -KY\n"
			"P2-KY-GI * +GI *  * -HI *  * \n"
			"P3-FU-FU-FU+FU-FU *  * +FU-FU\n"
			"P4 *  *  *  *  *  *  * +UM * \n"
			"P5 *  *  *  *  *  *  * -FU * \n"
			"P6 *  * +FU-FU *  * +FU *  * \n"
			"P7+FU+FU *  *  * +FU *  * +FU\n"
			"P8+KY *  * -NK *  *  *  *  * \n"
			"P9+OU * +KI *  *  *  *  * +KY\n"
			"P+00KA00GI00GI00KE00KE00FU00FU\n"
			"P-00HI00KI00KI00FU\n"
			"+").initialState());
    BOOST_CHECK_EQUAL(str, usi::show(state));
    NumEffectState test;
    usi::parse(str, test);
    BOOST_CHECK_EQUAL(state, static_cast<const NumEffectState&>(test));
  }
  {
    std::string str = "sfen lnsg4l/2k2R+P2/pppp1s2p/9/4p1P2/2P2p+n2/PP1P4P/2KSG1p2/LN1G4L w BG2Prbsnp 1";
    NumEffectState state(CsaString(
			"P1-KY-KE-GI-KI *  *  *  * -KY\n"
			"P2 *  * -OU *  * +HI+TO *  * \n"
			"P3-FU-FU-FU-FU * -GI *  * -FU\n"
			"P4 *  *  *  *  *  *  *  *  * \n"
			"P5 *  *  *  * -FU * +FU *  * \n"
			"P6 *  * +FU *  * -FU-NK *  * \n"
			"P7+FU+FU * +FU *  *  *  * +FU\n"
			"P8 *  * +OU+GI+KI * -FU *  * \n"
			"P9+KY+KE * +KI *  *  *  * +KY\n"
			"P+00KA00KI00FU00FU\n"
			"P-00HI00KA00GI00KE00FU\n"
			"-\n").initialState());
    BOOST_CHECK_EQUAL(str, usi::show(state));
    NumEffectState test;
    usi::parse(str, test);
    BOOST_CHECK_EQUAL(state, static_cast<const NumEffectState&>(test));
  }
}

BOOST_AUTO_TEST_CASE(UsiTestHandicap)
{
  {
    const std::string sfen = "position sfen lnsgkgsnl/9/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL w - 1 moves 5a6b 2g2f 6b7b 2f2e 7b8b 2e2d 2c2d 2h2d";
    NumEffectState state;
    std::vector<Move> moves;
    BOOST_CHECK_NO_THROW(usi::parse(sfen, state, moves));

    for (Ptype ptype: PieceStand::order) {
      BOOST_CHECK_EQUAL(0, state.countPiecesOnStand(BLACK, ptype));
      BOOST_CHECK_EQUAL(0, state.countPiecesOnStand(WHITE, ptype));
    }

    for (Move move: moves) {
      MoveVector all;
      state.generateLegal(all);
      BOOST_CHECK(all.isMember(move));
      state.makeMove(move);

      for (int i = 0; i < Piece::SIZE; ++i) {
	if (! state.usedMask().test(i)) {
	  BOOST_CHECK(state.pieceOf(i).owner() == WHITE);
	  BOOST_CHECK(! state.pieceOf(i).isOnBoard());
	}
      }
    }
  }
  {
    const std::string sfen = "position sfen lnsg1gsnl/1k7/ppppppp1p/7R1/9/9/PPPPPPP1P/1B7/LNSGKGSNL w Pp 1";
    NumEffectState state;
    BOOST_CHECK_NO_THROW(usi::parse(sfen, state));
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
