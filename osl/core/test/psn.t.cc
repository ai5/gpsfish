/* psn.t.cc
 */
#include "osl/usi.h"
#include "osl/numEffectState.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

BOOST_AUTO_TEST_CASE(PsnTestMove)
{
  NumEffectState state;
  const Move m76fu = Move(Square(7,7), Square(7,6), PAWN, 
			     PTYPE_EMPTY, false, BLACK);
  const std::string psn76fu = "7g7f";
  const Move m76fu2 = psn::strToMove(psn76fu, state);
  BOOST_CHECK_EQUAL(m76fu, m76fu2);

  const std::string psn76fu2 = psn::show(m76fu);
  BOOST_CHECK_EQUAL(psn76fu, psn76fu2);
}

BOOST_AUTO_TEST_CASE(PsnTestShowXP)
{
  const Move m11kakunari = Move(Square(7,7), Square(1,1), PBISHOP, 
				LANCE, true, BLACK);
  const std::string psn11kakunari = "7gx1a+";
  BOOST_CHECK_EQUAL(psn11kakunari, psn::showXP(m11kakunari));

  const Move m11kakunarazu = Move(Square(7,7), Square(1,1), BISHOP, 
				LANCE, false, BLACK);
  const std::string psn11kakunarazu = "7gx1a=";
  BOOST_CHECK_EQUAL(psn11kakunarazu, psn::showXP(m11kakunarazu));

  const Move m11kin = Move(Square(2,2), Square(1,1), GOLD, 
			   LANCE, false, BLACK);
  const std::string psn11kin = "2bx1a";
  BOOST_CHECK_EQUAL(psn11kin, psn::showXP(m11kin));

  const Move m11ou = Move(Square(2,2), Square(1,1), KING, 
			   LANCE, false, BLACK);
  const std::string psn11ou = "2bx1a";
  BOOST_CHECK_EQUAL(psn11ou, psn::showXP(m11ou));

  const Move bpass = Move::PASS(BLACK);
  const std::string psnpass = "pass";
  BOOST_CHECK_EQUAL(psnpass, psn::showXP(bpass));

  const Move wpass = Move::PASS(WHITE);
  BOOST_CHECK_EQUAL(psnpass, psn::showXP(wpass));
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
