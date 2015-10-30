#include "osl/effect_util/pin.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::effect_util;

BOOST_AUTO_TEST_CASE(PinTestNoPin) 
{
  {
    NumEffectState state((SimpleState(HIRATE)));
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK(pins.none());
  }
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
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK(pins.none());
  }
}
BOOST_AUTO_TEST_CASE(PinTestUp)
{
  {
    NumEffectState state(CsaString(	
			  "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			  "P2 *  *  *  *  *  *  * -KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  *  * -HI *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI * \n"
			  "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			  "+\n").initialState());
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK_EQUAL(1, pins.countBit2());
    const Piece p57fu = state.pieceOnBoard(Square(5,7));
    BOOST_CHECK(pins.test(p57fu.number()));
  }
  {
    NumEffectState state(CsaString(	
			  "P1-KY-KE-GI-KI-OU-KI-GI-KE * \n"
			  "P2 * -HI *  *  *  *  * -KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  *  * -KY *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI * \n"
			  "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			  "+\n").initialState());
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK_EQUAL(1, pins.countBit2());
    const Piece p57fu = state.pieceOnBoard(Square(5,7));
    BOOST_CHECK(pins.test(p57fu.number()));
  }
}
BOOST_AUTO_TEST_CASE(PinTestDown)
{
  {
    NumEffectState state(CsaString(
			  "P1-KY-KE-GI * +OU * -GI-KE-KY\n"
			  "P2 *  * -KI *  *  * -KI-KA * \n"
			  "P3-FU-FU-FU-FU+FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  *  * -HI *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU-FU+FU+FU+FU+FU\n"
			  "P8 * +KA+KI *  *  * +KI+HI * \n"
			  "P9+KY+KE+GI * -OU * +GI+KE+KY\n"
			  "+\n").initialState());
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK_EQUAL(1, pins.countBit2());
    const Piece p53fu = state.pieceOnBoard(Square(5,3));
    BOOST_CHECK(pins.test(p53fu.number()));
  }
}
BOOST_AUTO_TEST_CASE(PinTestNeighbor)
{
  // 駒が隣にある場合のテスト
  {
    NumEffectState state(CsaString(	
			  "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			  "P2 *  *  *  *  *  *  * -KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  *  * -HI+FU+OU *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU * +FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI * \n"
			  "P9+KY+KE+GI+KI * +KI+GI+KE+KY\n"
			  "+\n").initialState());
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK_EQUAL(1, pins.countBit2());
    const Piece p45fu = state.pieceOnBoard(Square(4,5));
    BOOST_CHECK(pins.test(p45fu.number()));
  }
  {
    NumEffectState state(CsaString(	
			  "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			  "P2 *  *  *  *  *  *  * -KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  * -HI * +FU+OU *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU * +FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI * \n"
			  "P9+KY+KE+GI+KI * +KI+GI+KE+KY\n"
			  "+\n").initialState());
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK_EQUAL(1, pins.countBit2());
    const Piece p45fu = state.pieceOnBoard(Square(4,5));
    BOOST_CHECK(pins.test(p45fu.number()));
  }
  {
    NumEffectState state(CsaString(	
			  "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			  "P2 *  *  *  *  *  *  * -KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  *  * -HI+FU * +OU * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU * +FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI * \n"
			  "P9+KY+KE+GI+KI * +KI+GI+KE+KY\n"
			  "+\n").initialState());
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK_EQUAL(1, pins.countBit2());
    const Piece p45fu = state.pieceOnBoard(Square(4,5));
    BOOST_CHECK(pins.test(p45fu.number()));
  }
}
BOOST_AUTO_TEST_CASE(PinTestDiagonal)
{
  {
    NumEffectState state(CsaString(
			  "P1-KY-KE-GI * -OU * -GI-KE-KY\n"
			  "P2 * -HI-KI *  *  * -KI-KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			  "P4 *  *  *  *  *  * -FU *  * \n"
			  "P5 *  *  *  *  *  *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			  "P8 * +OU+KI *  *  * +KI+HI * \n"
			  "P9+KY+KE+GI * +KA * +GI+KE+KY\n"
			  "+\n").initialState());
    const PieceMask pins = Pin::make(state, BLACK);
    BOOST_CHECK_EQUAL(1, pins.countBit2());
    const Piece pin = state.pieceOnBoard(Square(7,7));
    BOOST_CHECK(pins.test(pin.number()));
  }
}
static void testEquality(const std::string& filename)
{
  auto record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  const auto& moves=record.moves;

  size_t i=0;
  while (true)
  {
    const PieceMask black_pins = Pin::makeNaive(state, BLACK);
    const PieceMask black_pins2 = Pin::makeByPiece(state, BLACK);
    if (black_pins != black_pins2)
	std::cerr << state;
    BOOST_CHECK_EQUAL(black_pins, black_pins2);
    const PieceMask black_pins3 = Pin::makeStep(state, state.kingSquare<BLACK>(), BLACK);
    if (black_pins != black_pins3)
	std::cerr << state;
    BOOST_CHECK_EQUAL(black_pins, black_pins3);

    const PieceMask white_pins = Pin::makeNaive(state, WHITE);
    const PieceMask white_pins2 = Pin::makeByPiece(state, WHITE);
    if (white_pins != white_pins2)
	std::cerr << state;
    BOOST_CHECK_EQUAL(white_pins, white_pins2);

    const PieceMask white_pins3 = Pin::makeStep(state, state.kingSquare<WHITE>(), WHITE);
    if (white_pins != white_pins3)
	std::cerr << state;
    BOOST_CHECK_EQUAL(white_pins, white_pins3);

    if (i >= moves.size())
	break;
    const Move move = moves[i++];
    state.makeMove(move);
  } 
}
BOOST_AUTO_TEST_CASE(PinTestEquality)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=100;
  if (OslConfig::inUnitTestShort())
    count=10;
  std::string file_name;
  while ((ifs >> file_name) && (++i<count)) {
    if (file_name == "") 
	break;
    if (OslConfig::verbose())
	std::cerr << file_name << " ";
    testEquality(OslConfig::testCsaFile(file_name));
  }
}

BOOST_AUTO_TEST_CASE(PinTestCount) {
  {
    NumEffectState state(CsaString(	
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 *  *  *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * -HI *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(1, Pin::count(state, BLACK));
  }
  {
    NumEffectState state(CsaString(	
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 *  *  *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * -HI *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA *  *  *  *  * +GI * \n"
			   "P9+KY+KE+GI+KI+OU+KI-HI+KE+KY\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(2, Pin::count(state, BLACK));
  }
  {
    // target は玉でなくても良い
    NumEffectState state(CsaString(	
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 *  *  *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  * -HI *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			   "P8 * +KA+OU *  *  *  * +GI * \n"
			   "P9+KY+KE+GI+KI * +KI-HI+KE+KY\n"
			   "+\n").initialState());
    BOOST_CHECK_EQUAL(2, Pin::count(state, Square(5,9), BLACK));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
