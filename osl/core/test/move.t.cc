#include "osl/numEffectState.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(MoveTestValid) {
  const Move drop(Square(5,5), KNIGHT, WHITE);
  BOOST_CHECK(drop.isValid()); 

  const Move bug = Move::makeDirect(-58698240);
  BOOST_CHECK(! bug.isValid()); // KINGを取る55桂打
}
BOOST_AUTO_TEST_CASE(MoveTestNormal)
{
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  BOOST_CHECK(! m76fu.isPass());
  BOOST_CHECK(m76fu.isNormal());
  BOOST_CHECK(! m76fu.isInvalid());

  const Move m34fu(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE);
  BOOST_CHECK(! m34fu.isPass());
  BOOST_CHECK(m34fu.isNormal());
  BOOST_CHECK(! m34fu.isInvalid());
}
BOOST_AUTO_TEST_CASE(MoveTestPass)
{
  const Move pass_black = Move::PASS(BLACK);
  BOOST_CHECK_EQUAL(PTYPE_EMPTY, pass_black.ptype());
  BOOST_CHECK_EQUAL(PTYPE_EMPTY, pass_black.oldPtype());
  BOOST_CHECK_EQUAL(Square::STAND(), pass_black.from());
  BOOST_CHECK_EQUAL(Square::STAND(), pass_black.to());
  BOOST_CHECK_EQUAL(BLACK, pass_black.player());

  BOOST_CHECK(pass_black.isPass());
  BOOST_CHECK(! pass_black.isNormal());
  BOOST_CHECK(! pass_black.isInvalid());

  const Move pass_white = Move::PASS(WHITE);
  BOOST_CHECK_EQUAL(PTYPE_EMPTY, pass_white.ptype());
  BOOST_CHECK_EQUAL(PTYPE_EMPTY, pass_white.oldPtype());
  BOOST_CHECK_EQUAL(Square::STAND(), pass_white.from());
  BOOST_CHECK_EQUAL(Square::STAND(), pass_white.to());
  BOOST_CHECK_EQUAL(WHITE, pass_white.player());

  BOOST_CHECK(pass_white.isPass());
  BOOST_CHECK(! pass_white.isNormal());
  BOOST_CHECK(! pass_white.isInvalid());
}
BOOST_AUTO_TEST_CASE(MoveTestDeclareWin)
{
  BOOST_CHECK(Move::DeclareWin().isInvalid());
  BOOST_CHECK(! Move::DeclareWin().isNormal());
}
BOOST_AUTO_TEST_CASE(MoveTestSetFrom)
{
  const Square from(7,7);
  const Square to(7,6);
  const Ptype ptype = GOLD;
  const Player player = BLACK;
  const Ptype capture_ptype = PTYPE_EMPTY;
  const bool promote = false;
  Move m(from, to, ptype, capture_ptype, promote, player);
  Move m_copy(m);
  m=m.newFrom(Square(8,7));
  BOOST_CHECK_EQUAL(Square(8,7), m.from());
  BOOST_CHECK_EQUAL(to, m.to());
  BOOST_CHECK_EQUAL(ptype, m.ptype());
  BOOST_CHECK_EQUAL(capture_ptype, m.capturePtype());
  BOOST_CHECK_EQUAL(promote, m.isPromotion());
  BOOST_CHECK_EQUAL(player, m.player());

  m=m.newFrom(Square(7,7));
  BOOST_CHECK_EQUAL(m_copy, m);
}
BOOST_AUTO_TEST_CASE(MoveTestCapturePtpeO)
{
  const Move drop_b(Square(5,5), KNIGHT, BLACK);
  const Move drop_w(Square(5,5), KNIGHT, WHITE);
  BOOST_CHECK_EQUAL(PTYPEO_EMPTY, drop_w.capturePtypeOSafe());
  BOOST_CHECK_EQUAL(PTYPEO_EMPTY, drop_b.capturePtypeOSafe());
}
BOOST_AUTO_TEST_CASE(MoveTestIgnoreUnpromote)
{
  // pawn
  BOOST_CHECK(Move(Square(4,4),Square(4,3),PAWN,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,4),Square(4,3),PPAWN,PTYPE_EMPTY,true,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,3),PAWN,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,4),PAWN,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(Move(Square(4,6),Square(4,7),PAWN,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,6),Square(4,7),PPAWN,PTYPE_EMPTY,true,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,7),PAWN,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,6),PAWN,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  // lance
  BOOST_CHECK(!Move(Square(4,4),Square(4,3),LANCE,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(Move(Square(4,4),Square(4,2),LANCE,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,4),Square(4,3),PLANCE,PTYPE_EMPTY,true,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,3),LANCE,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,4),LANCE,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,6),Square(4,7),LANCE,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  BOOST_CHECK(Move(Square(4,6),Square(4,8),LANCE,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,6),Square(4,7),PLANCE,PTYPE_EMPTY,true,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,7),LANCE,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,6),LANCE,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  // bishop
  BOOST_CHECK(Move(Square(4,4),Square(2,2),BISHOP,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(Move(Square(4,2),Square(8,6),BISHOP,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,4),Square(2,2),PBISHOP,PTYPE_EMPTY,true,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,2),Square(8,6),PBISHOP,PTYPE_EMPTY,true,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,3),BISHOP,BLACK).ignoreUnpromote());

  BOOST_CHECK(Move(Square(6,6),Square(8,8),BISHOP,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  BOOST_CHECK(Move(Square(6,8),Square(2,4),BISHOP,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(6,6),Square(8,8),PBISHOP,PTYPE_EMPTY,true,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(6,8),Square(2,4),PBISHOP,PTYPE_EMPTY,true,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(6,7),BISHOP,WHITE).ignoreUnpromote());
  // ROOK
  BOOST_CHECK(Move(Square(4,4),Square(4,2),ROOK,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(Move(Square(4,2),Square(4,6),ROOK,PTYPE_EMPTY,false,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,4),Square(4,2),PROOK,PTYPE_EMPTY,true,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,2),Square(4,6),PROOK,PTYPE_EMPTY,true,BLACK).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(4,3),ROOK,BLACK).ignoreUnpromote());

  BOOST_CHECK(Move(Square(6,6),Square(6,8),ROOK,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  BOOST_CHECK(Move(Square(6,8),Square(6,4),ROOK,PTYPE_EMPTY,false,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(6,6),Square(6,8),PROOK,PTYPE_EMPTY,true,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(6,8),Square(6,4),PROOK,PTYPE_EMPTY,true,WHITE).ignoreUnpromote());
  BOOST_CHECK(!Move(Square(6,7),ROOK,WHITE).ignoreUnpromote());
}
BOOST_AUTO_TEST_CASE(MoveTestHasIgnoredUnpromote)
{
  // pawn
  BOOST_CHECK(Move(Square(4,4),Square(4,3),PPAWN,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(Move(Square(4,3),Square(4,2),PPAWN,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,2),Square(4,1),PPAWN,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,3),PAWN,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,4),PAWN,PTYPE_EMPTY,false,BLACK).hasIgnoredUnpromote());

  BOOST_CHECK(Move(Square(4,6),Square(4,7),PPAWN,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(Move(Square(4,7),Square(4,8),PPAWN,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,8),Square(4,9),PPAWN,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,7),PAWN,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,6),PAWN,PTYPE_EMPTY,false,WHITE).hasIgnoredUnpromote());
  // lance
  BOOST_CHECK(Move(Square(4,4),Square(4,2),PLANCE,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,4),Square(4,3),PLANCE,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,4),Square(4,1),PLANCE,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,3),LANCE,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,4),LANCE,PTYPE_EMPTY,false,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(Move(Square(4,6),Square(4,8),PLANCE,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,6),Square(4,7),PLANCE,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,6),Square(4,9),PLANCE,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,7),LANCE,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,5),Square(4,6),LANCE,PTYPE_EMPTY,false,WHITE).hasIgnoredUnpromote());
  // bishop
  BOOST_CHECK(Move(Square(4,4),Square(2,2),PBISHOP,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(Move(Square(4,2),Square(8,6),PBISHOP,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,4),Square(2,2),PBISHOP,PTYPE_EMPTY,false,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,3),BISHOP,BLACK).hasIgnoredUnpromote());

  BOOST_CHECK(Move(Square(6,6),Square(8,8),PBISHOP,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(Move(Square(6,8),Square(2,4),PBISHOP,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(6,6),Square(8,8),PBISHOP,PTYPE_EMPTY,false,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(6,7),BISHOP,WHITE).hasIgnoredUnpromote());
  // ROOK
  BOOST_CHECK(Move(Square(4,4),Square(4,2),PROOK,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(Move(Square(4,2),Square(4,6),PROOK,PTYPE_EMPTY,true,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,2),Square(4,6),PROOK,PTYPE_EMPTY,false,BLACK).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(4,3),ROOK,BLACK).hasIgnoredUnpromote());

  BOOST_CHECK(Move(Square(6,6),Square(6,8),PROOK,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(Move(Square(6,8),Square(6,4),PROOK,PTYPE_EMPTY,true,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(6,8),Square(6,4),PROOK,PTYPE_EMPTY,false,WHITE).hasIgnoredUnpromote());
  BOOST_CHECK(!Move(Square(6,7),ROOK,WHITE).hasIgnoredUnpromote());
}
BOOST_AUTO_TEST_CASE(MoveTestRotate180)
{
  const Move m76(Square(7,7), Square(7,6), PAWN, PTYPE_EMPTY, false, BLACK);
  const Move m34(Square(3,3), Square(3,4), PAWN, PTYPE_EMPTY, false, WHITE);
  BOOST_CHECK_EQUAL(m76.rotate180(), m34);

  const Move m41(Square(4,1), SILVER, BLACK);
  const Move m69(Square(6,9), SILVER, WHITE);
  BOOST_CHECK_EQUAL(m41.rotate180(), m69);

  BOOST_CHECK_EQUAL(Move::PASS(BLACK).rotate180(), Move::PASS(WHITE));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
