/* effectUtil.t.cc
 */
#include "osl/effect_util/effectUtil.h"
#include "osl/effect_util/effectUtil.tcc"
#include "osl/numEffectState.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;

struct SquareVector : public FixedCapacityVector<Square,81>
{
  
};

std::ostream& operator<<(std::ostream& os, const SquareVector& vec)
{
  for (size_t i=0; i<vec.size(); ++i)
  {
    os << vec[i] << "\n";
  }
  return os;
}

struct StoreSquare
{
  SquareVector& out;
  StoreSquare(SquareVector& o) : out(o)
  {
  }
  void operator()(Square pos)
  {
    out.push_back(pos);
  }
};

BOOST_AUTO_TEST_CASE(EffectUtilTestCheapest)
{
  const char *endGameState =
  "P1+NY * +TO *  *  * -OU-KE-KY\n"
  "P2 *  *  *  *  * -GI-KI *  *\n"
  "P3 * +RY *  * +UM * -KI-FU-FU\n"
  "P4 *  * +FU-FU *  *  *  *  *\n"
  "P5 *  * -KE * +FU *  * +FU *\n"
  "P6-KE *  * +FU+GI-FU *  * +FU\n"
  "P7 *  * -UM *  *  *  *  *  *\n"
  "P8 *  *  *  *  *  *  *  *  * \n"
  "P9 * +OU * -GI *  *  *  * -NG\n"
  "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU00FU\n"
  "P-00KI00KY00FU\n"
  "P-00AL\n"
  "+\n";
  const NumEffectState state(CsaString(endGameState).initialState());
  BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(8,3)),
		       state.findCheapAttack
		       (BLACK, Square(9,3)));
  BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(9,6)),
		       state.findCheapAttack
		       (WHITE, Square(8,8)));
  BOOST_CHECK_EQUAL(Piece::EMPTY(),
		       state.findCheapAttack
		       (BLACK, Square(2,8)));
  BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(3,2)),
		       state.findCheapAttack
		       (WHITE, Square(2,2)));
}

BOOST_AUTO_TEST_CASE(EffectUtilTestCheapestPtype)
{
  const char *endGameState =
  "P1+NY * +TO *  *  * -OU-KE-KY\n"
  "P2 *  *  *  *  * -GI-KI *  *\n"
  "P3 * +RY *  * +UM * -KI-FU-FU\n"
  "P4 *  * +FU-FU *  *  *  *  *\n"
  "P5 *  * -KE * +FU *  * +FU *\n"
  "P6-KE *  * +FU+GI-FU *  * +FU\n"
  "P7 *  * -UM *  *  *  *  *  *\n"
  "P8 *  *  *  *  *  *  *  *  * \n"
  "P9 * +OU * -GI *  *  *  * -NG\n"
  "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU00FU\n"
  "P-00KI00KY00FU\n"
  "P-00AL\n"
  "+\n";
  const NumEffectState state(CsaString(endGameState).initialState());
  BOOST_CHECK_EQUAL(PROOK, state.findCheapAttack(BLACK, Square(9,3)).ptype());
  BOOST_CHECK_EQUAL(KNIGHT, state.findCheapAttack(WHITE, Square(8,8)).ptype());
  BOOST_CHECK_EQUAL(PTYPE_EMPTY, state.findCheapAttack(BLACK, Square(2,8)).ptype());
  BOOST_CHECK_EQUAL(GOLD, state.findCheapAttack(WHITE, Square(2,2)).ptype());
}



/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
