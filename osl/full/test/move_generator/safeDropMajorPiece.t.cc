#include "osl/move_generator/safeDropMajorPiece.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace osl;
using namespace osl::move_action;
using namespace osl::move_generator;

BOOST_AUTO_TEST_CASE(TestSafeMajorPieceRookBlack)
{
  NumEffectState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 * +OU *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P+00HI\n"
    "P-00AL\n"
    "-\n").initialState();
  {
    MoveVector moves;
    {
      Store store(moves);
      SafeDropMajorPiece<BLACK>::generate<>(state, store);
    }
    BOOST_CHECK(23u == moves.size());

    for (size_t i = 0; i < moves.size(); i++)
    {
      BOOST_CHECK(moves[i].ptype() == ROOK);
      BOOST_CHECK(moves[i].from().isPieceStand());
      BOOST_CHECK(moves[i].to().y() <= 3);
      BOOST_CHECK(!state.hasEffectAt(alt(state.turn()),
				     moves[i].to()));
    }
  }
}

BOOST_AUTO_TEST_CASE(TestSafeMajorPieceRookWhite)
{
  NumEffectState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 * +OU *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P-00HI\n"
    "P+00AL\n"
    "-\n").initialState();
  {
    MoveVector moves;
    {
      Store store(moves);
      SafeDropMajorPiece<WHITE>::generate<>(state, store);
    }
    BOOST_CHECK(18u == moves.size());
    for (size_t i = 0; i < moves.size(); i++)
    {
      BOOST_CHECK(moves[i].ptype() == ROOK);
      BOOST_CHECK(moves[i].from().isPieceStand());
      BOOST_CHECK(moves[i].to().y() >= 7);
      BOOST_CHECK(!state.hasEffectAt(alt(state.turn()),
				     moves[i].to()));
    }
  }
}

BOOST_AUTO_TEST_CASE(TestSafeMajorPieceBishopBlack)
{
  NumEffectState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 * +OU *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P+00KA\n"
    "P-00AL\n"
    "-\n").initialState();
  {
    MoveVector moves;
    {
      Store store(moves);
      SafeDropMajorPiece<BLACK>::generate<>(state, store);
    }
    BOOST_CHECK(23u == moves.size());
    for (size_t i = 0; i < moves.size(); i++)
    {
      BOOST_CHECK(moves[i].ptype() == BISHOP);
      BOOST_CHECK(moves[i].from().isPieceStand());
      BOOST_CHECK(moves[i].to().y() <= 3);
      BOOST_CHECK(!state.hasEffectAt(alt(state.turn()),
				     moves[i].to()));
    }
  }
}

BOOST_AUTO_TEST_CASE(TestSafeMajorPieceBishopWhite)
{
  NumEffectState state=CsaString(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 * +OU *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P-00KA\n"
    "P+00AL\n"
    "-\n").initialState();
  {
    MoveVector moves;
    {
      Store store(moves);
      SafeDropMajorPiece<WHITE>::generate<>(state, store);
    }
    BOOST_CHECK(18u == moves.size());
    for (size_t i = 0; i < moves.size(); i++)
    {
      BOOST_CHECK(moves[i].ptype() == BISHOP);
      BOOST_CHECK(moves[i].from().isPieceStand());
      BOOST_CHECK(moves[i].to().y() >= 7);
      BOOST_CHECK(!state.hasEffectAt(alt(state.turn()),
				     moves[i].to()));
    }
  }
}



// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
