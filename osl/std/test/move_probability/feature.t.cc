#include "osl/move_probability/feature.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::move_probability;

BOOST_AUTO_TEST_CASE(MPFeatureTestPatternCommon)
{
  BOOST_CHECK_EQUAL(48, (int)PatternCommon::PromotionBase);
  BOOST_CHECK_EQUAL(64, (int)PatternCommon::SquareDim);
}

BOOST_AUTO_TEST_CASE(MPFeatureTestSupportAttack)
{  
  MoveStack history;
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-GI *  *  *  * -KE-KY\n"
			   "P2-KY * -KI * -KI *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-GI-HI-FU-FU\n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  * -FU+GI+FU * \n"
			   "P6+FU * +FU * +FU *  *  *  * \n"
			   "P7 * +FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +OU+GI+KI *  * +HI * \n"
			   "P9+KY+KE * +KI *  *  * +KE+KY\n"
			   "P+00KA00FU\n"
			   "P-00KA00FU\n"
			   "+\n").initialState());
    Move m36fu(Square(3,6),PAWN,BLACK);
    int index = ToEffect::supportAttack
      (StateInfo(state,Progress16(0),history), Square(3,5), m36fu);
    BOOST_CHECK_EQUAL(PTYPE_EMPTY*PTYPE_SIZE + ROOK, index);
  }
}

BOOST_AUTO_TEST_CASE(MPFeatureTestBlockLong)
{
  BOOST_CHECK_EQUAL(0, BlockLong::longAttackIndex(newPtypeO(BLACK, LANCE)));
  BOOST_CHECK_EQUAL(1, BlockLong::longAttackIndex(newPtypeO(BLACK, BISHOP)));
  BOOST_CHECK_EQUAL(2, BlockLong::longAttackIndex(newPtypeO(BLACK, ROOK)));
  BOOST_CHECK_EQUAL(3, BlockLong::longAttackIndex(newPtypeO(BLACK, PROOK)));
  BOOST_CHECK_EQUAL(3, BlockLong::longAttackIndex(newPtypeO(BLACK, PBISHOP)));
  BOOST_CHECK_EQUAL(4, BlockLong::longAttackIndex(newPtypeO(WHITE, LANCE)));
  BOOST_CHECK_EQUAL(5, BlockLong::longAttackIndex(newPtypeO(WHITE, BISHOP)));
  BOOST_CHECK_EQUAL(6, BlockLong::longAttackIndex(newPtypeO(WHITE, ROOK)));
  BOOST_CHECK_EQUAL(7, BlockLong::longAttackIndex(newPtypeO(WHITE, PROOK)));
  BOOST_CHECK_EQUAL(7, BlockLong::longAttackIndex(newPtypeO(WHITE, PBISHOP)));
  MoveStack history;
  CArray<double,BlockLong::DIM> weight;
  weight.fill(1.0);
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-GI *  *  *  * -KE-KY\n"
			   "P2-KY * -KI * -KI *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-GI-HI-FU-FU\n"
			   "P4 *  *  * -KA *  *  *  *  * \n"
			   "P5 *  *  *  *  * -FU+GI+FU * \n"
			   "P6+FU * +FU * +FU * +FU *  * \n"
			   "P7 * +FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +OU+GI+KI *  * +HI * \n"
			   "P9+KY+KE * +KI *  *  * +KE+KY\n"
			   "P+00KA\n"
			   "P-00FU\n"
			   "+\n").initialState());
    Move m55(Square(5,5), BISHOP, BLACK);
    double value 
      = BlockLong::findAll(StateInfo(state,Progress16(0),history), m55, &weight[0]);
    BOOST_CHECK(value != 0);
  } 
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-GI *  *  *  * -KE-KY\n"
			   "P2-KY * -KI * -KI *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-GI-HI-FU-FU\n"
			   "P4 *  *  * -KA *  *  *  *  * \n"
			   "P5 *  *  *  *  * -FU+GI+FU * \n"
			   "P6+FU * +FU * +FU * +FU *  * \n"
			   "P7 * +FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +OU+GI+KI *  * +HI * \n"
			   "P9+KY+KE * +KI *  *  * +KY+KE\n" // KY
			   "P+00KA\n"
			   "P-00FU\n"
			   "+\n").initialState());
    Move m55(Square(5,5), BISHOP, BLACK);
    double value
      = BlockLong::findAll(StateInfo(state,Progress16(0),history), m55, &weight[0]);
    BOOST_CHECK(value != 0);
  } 
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-GI *  *  *  * -KE-KY\n"
			   "P2-KY * -KI * -KI *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-GI-HI-FU-FU\n"
			   "P4 *  *  * -KA *  *  *  *  * \n"
			   "P5 *  *  *  *  * -FU+GI+FU * \n"
			   "P6+FU * +FU * +FU * +FU *  * \n"
			   "P7 * +FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +OU+GI+KI *  * -HI * \n"
			   "P9+KY+KE * +KI *  *  * +KE+KY\n"
			   "P+00KA\n"
			   "P-00FU\n"
			   "+\n").initialState());
    Move m55(Square(5,5), BISHOP, BLACK);
    double value
      = BlockLong::findAll(StateInfo(state,Progress16(0),history), m55, &weight[0]);
    BOOST_CHECK(value != 0);
  } 
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-GI *  *  *  * -KE-KY\n"
			   "P2-KY * -KI * -KI *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-GI-HI-FU-FU\n"
			   "P4 *  *  * -KA *  *  *  *  * \n"
			   "P5 *  *  *  *  * -FU+GI+FU * \n"
			   "P6+FU * +FU * +FU * +FU *  * \n"
			   "P7 * +FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +OU+GI+KI *  * +HI * \n"
			   "P9+KY+KE * +KI *  *  * +KE+KY\n"
			   "P+00FU\n"
			   "P-00KA\n"
			   "-\n").initialState());
    Move m55(Square(5,5), BISHOP, WHITE);
    double value = 
      BlockLong::findAll(StateInfo(state,Progress16(0),history), m55, &weight[0]);
    BOOST_CHECK(value != 0);
  } 
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-GI *  *  *  * -KE-KY\n"
			   "P2-KY * -KI * -KI *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-GI-HI-FU-FU\n"
			   "P4 *  *  * +KA *  *  *  *  * \n"
			   "P5 *  *  *  *  * -FU+GI+FU * \n"
			   "P6+FU * +FU * +FU * +FU *  * \n"
			   "P7 * +FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +OU+GI+KI *  * -HI * \n"
			   "P9+KY+KE * +KI *  *  * +KE+KY\n"
			   "P+00FU\n"
			   "P-00KA\n"
			   "-\n").initialState());
    Move m55(Square(5,5), BISHOP, WHITE);
    double value
      = BlockLong::findAll(StateInfo(state,Progress16(0),history), m55, &weight[0]);
    BOOST_CHECK(value != 0);
  } 
  {
    NumEffectState state(CsaString(
			   "P1-OU-KE-GI *  *  *  * -KE-KY\n"
			   "P2-KY * -KI * -KI *  *  *  * \n"
			   "P3-FU-FU-FU-FU-FU-GI-HI-FU-FU\n"
			   "P4 *  *  * -KA *  *  *  *  * \n"
			   "P5 *  *  *  *  * -FU+GI+FU * \n"
			   "P6+FU * +FU * +FU * +FU *  * \n"
			   "P7 * +FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +OU+GI+KI * +HI *  * \n"
			   "P9+KY+KE * +KI *  * +KE+KY * \n" // edge
			   "P+00KA\n"
			   "P-00FU\n"
			   "+\n").initialState());
    Move m55(Square(5,5), BISHOP, BLACK);
    double value
      = BlockLong::findAll(StateInfo(state,Progress16(0),history), m55, &weight[0]);
    BOOST_CHECK(value != 0);
  } 
}

BOOST_AUTO_TEST_CASE(MPFeatureTestLureDefender)
{  
  // gold
  MoveStack history;
  {
    NumEffectState state(CsaString(
			   "P1 * -OU *  * -GI * +RY *  * \n"
			   "P2 *  *  *  * -KI *  *  *  * \n"
			   "P3 *  *  * +FU *  *  *  *  * \n"
			   "P4 *  *  *  *  *  * -GI *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  *  * +KI *  * +KI *  * \n"
			   "P9 * +OU * +GI *  * +FU-TO * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK_EQUAL(5, (int)info.exchange_pins[BLACK].size());
    BOOST_CHECK_EQUAL(2, (int)info.exchange_pins[WHITE].size());
    BOOST_CHECK_EQUAL(Square(5,3), info.exchange_pins[WHITE][0].attack);
    BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(5,1)),
			 info.exchange_pins[WHITE][0].covered);

    BOOST_CHECK_EQUAL(Square(4,7), info.exchange_pins[BLACK][0].attack);
    BOOST_CHECK_EQUAL(Square(3,7), info.exchange_pins[BLACK][1].attack);
    BOOST_CHECK_EQUAL(Square(2,7), info.exchange_pins[BLACK][2].attack);
    BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(3,9)),
			 info.exchange_pins[BLACK][1].covered);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  * -HI-KI * -KE * \n"
			   "P2-KY *  * +GI *  * -KI * -OU\n"
			   "P3 *  *  * +UM * +FU * -FU-FU\n"
			   "P4 *  *  * -FU * -FU *  *  * \n"
			   "P5 * -FU *  *  *  * +FU *  * \n"
			   "P6-FU * +FU+FU *  * +GI+FU * \n"
			   "P7 * +FU+GI * +FU *  *  *  * \n"
			   "P8+FU+OU+KI+KI *  *  *  *  * \n"
			   "P9+KY+KE * +KE *  * -HI * -NY\n"
			   "P-00KA00GI00KE00FU00FU00FU00FU\n"
			   "-\n").initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[BLACK].isMember
		   (PinnedGeneral(state[Square(6,8)],state[Square(6,9)],Square(5,8))));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  * +KA *  * -OU-KE-KY\n"
			   "P2 *  *  * -HI *  * -KI *  * \n"
			   "P3 *  *  *  *  * -KI *  *  * \n"
			   "P4 * -FU * -FU * -FU * +HI-FU\n"
			   "P5 * -KE+KA+FU *  *  *  *  * \n"
			   "P6+FU * +FU *  *  *  *  * +FU\n"
			   "P7+KY+FU *  *  * +FU *  *  * \n"
			   "P8 *  *  * +GI *  *  *  *  * \n"
			   "P9 * +OU+GI-GI *  *  * +KE+KY\n"
			   "P+00KI00GI00KE00FU00FU00FU00FU00FU00FU00FU\n"
			   "P-00KI00FU\n"
			   "+\n").initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		   (PinnedGeneral(state[Square(3,2)],state[Square(4,3)],Square(2,2))));
  }

  // silver
  {
    NumEffectState state(CsaString(
			   "P1-OU-TO *  *  * -KI * +RY * \n"
			   "P2 *  *  *  * -GI *  *  *  * \n"
			   "P3 *  *  *  *  * -KI *  *  * \n"
			   "P4 *  *  *  *  *  *  *  *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7 *  *  *  *  *  *  *  *  * \n"
			   "P8 *  * +GI * +GI *  *  *  * \n"
			   "P9+OU+TO *  *  * +KI * -RY * \n"
			   "P-00AL\n"
			   "+\n").initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[BLACK].isMember
		   (PinnedGeneral(state[Square(5,8)], state[Square(4,9)], Square(5,7))));
    BOOST_CHECK(info.exchange_pins[BLACK].isMember
		   (PinnedGeneral(state[Square(5,8)], state[Square(4,9)], Square(4,7))));
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(5,2)], state[Square(4,1)], Square(6,3))));
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(5,2)], state[Square(4,1)], Square(4,3))));
    BOOST_CHECK_EQUAL(2, (int)info.exchange_pins[BLACK].size());
    BOOST_CHECK_EQUAL(3, (int)info.exchange_pins[WHITE].size());
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  * -KI * -OU-KE-KY\n"
			   "P2 * -HI * -GI+KI-FU * -KI * \n"
			   "P3-FU *  *  * -FU-KA * -KA-FU\n"
			   "P4 *  *  * +GI *  * -FU-FU * \n"
			   "P5 *  * +FU *  *  *  *  *  * \n"
			   "P6+FU * -FU+KI+FU * +KE *  * \n"
			   "P7 * +FU *  *  * +RY+FU * +FU\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9+KY *  * +OU *  *  * +KE+KY\n"
			   "P+00GI00GI00KE00FU\n"
			   "P-00FU00FU00FU00FU\n"
			   "+\n").initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		   (PinnedGeneral(state[Square(6,2)], state[Square(5,1)], Square(6,3))));
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		   (PinnedGeneral(state[Square(6,2)], state[Square(5,1)],Square(7,3))));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE * -KI *  *  *  * +RY\n"
			   "P2 *  * -GI-OU *  * -KI *  * \n"
			   "P3-FU * -FU * -FU-GI-FU * -KE\n"
			   "P4 *  *  * -FU * -FU *  * -FU\n"
			   "P5 *  * +FU *  *  *  *  *  * \n"
			   "P6+FU-RY *  *  *  *  *  *  * \n"
			   "P7 *  * +KE+FU+FU+FU+FU * +FU\n"
			   "P8+KA+KA *  *  *  *  *  *  * \n"
			   "P9+KY * +GI * +OU+KI * +KE+KY\n"
			   "P+00KY00FU00FU00FU00FU\n"
			   "P-00KI00GI\n"
			   "-\n").initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[BLACK].isMember
		   (PinnedGeneral(state[Square(7,9)], state[Square(8,8)], Square(7,8))));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  * -KE-KY\n"
			   "P2 * -HI *  *  *  *  * -OU * \n"
			   "P3 *  *  *  *  *  *  * -FU * \n"
			   "P4-FU-KI * -FU *  * +FU * -FU\n"
			   "P5 *  * -FU * -FU * -GI *  * \n"
			   "P6+FU+HI *  *  * -GI *  * +FU\n"
			   "P7 *  * +KE+FU *  * +KE *  * \n"
			   "P8 *  * +KI *  *  * +GI+OU * \n"
			   "P9+KY *  *  *  * +KI *  * +KY\n"
			   "P+00KA00GI00FU00FU\n"
			   "P-00KA00KI00FU00FU00FU00FU00FU00FU\n"
			   "+\n").initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		   (PinnedGeneral(state[Square(3,5)], state[Square(4,6)], Square(3,6))));
  }

  // others
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  * -KE-KY\n"
			   "P2 * -HI *  *  *  * -KI-OU * \n"
			   "P3-FU *  *  *  * -KI-GI-FU-FU\n"
			   "P4 * -KA *  *  * -FU-FU *  * \n"
			   "P5 *  *  * +FU+KI *  * +FU+FU\n"
			   "P6 * +FU-FU *  *  * +FU *  * \n"
			   "P7+FU-FU * +GI+GI+FU+KE *  * \n"
			   "P8 *  * +KI+KA *  *  * +HI * \n"
			   "P9+KY+KE+OU *  *  *  *  * +KY\n"
			   "P+00KE00FU00FU00FU\n"
			   "P-00GI00FU\n"
			   "-\n"
			   ).initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[BLACK].isMember
		(PinnedGeneral(state[Square(6,8)], state[Square(5,7)], Square(5,9))));
  }

  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  * +TO-KY\n"
			   "P2 *  *  * -KI-KI * -KI *  * \n"
			   "P3 *  *  *  * -FU-FU-OU-FU * \n"
			   "P4 *  *  *  * -GI * +KE * -FU\n"
			   "P5-FU * +OU *  *  *  *  *  * \n"
			   "P6+FU-HI *  * +FU *  *  * +FU\n"
			   "P7 * -TO * +FU-NK+FU+FU *  * \n"
			   "P8 *  *  *  *  *  * +GI *  * \n"
			   "P9+KY * -TO *  * +KI * +KE+KY\n"
			   "P+00HI00KA00KA00GI00KE00FU00FU00FU\n"
			   "P-00GI00FU\n"
			   "+\n"
			   ).initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(8,7)], state[Square(8,6)], Square(7,7))));
  }

  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-OU-KE+HI *  * -KE * \n"
			   "P2 *  * -GI *  *  *  *  * -KY\n"
			   "P3 * -FU-FU * -KY+KA * -FU * \n"
			   "P4-FU *  * -FU *  * -FU * -FU\n"
			   "P5 *  *  *  *  *  *  * +FU * \n"
			   "P6+FU *  *  * +GI * +FU * +FU\n"
			   "P7 * +FU * +FU *  *  *  *  * \n"
			   "P8 *  * +OU+UM+KI+KI *  *  * \n"
			   "P9+KY+KE *  * +FU * -RY *  * \n"
			   "P+00KI00FU00FU\n"
			   "P-00KI00GI00GI00FU00FU\n"
			   "+\n"
			   ).initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(7,2)], state[Square(6,1)], Square(6,3))));
  }
  
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * -OU * -KE-KY\n"
			   "P2 * -HI * -GI-KI *  * -KI * \n"
			   "P3-FU * -KE * -FU *  * -FU-FU\n"
			   "P4 *  * +HI-FU * -FU+FU *  * \n"
			   "P5 *  *  *  *  * -GI *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+FU+FU * +FU * +FU *  * +FU\n"
			   "P8 *  * +KI+GI+KI *  *  *  * \n"
			   "P9+KY+KE+OU *  *  *  *  * +KY\n"
			   "P+00KA00GI00FU00FU00FU00FU\n"
			   "P-00KA00KE00FU00FU\n"
			   "+\n"
			   ).initialState());
    StateInfo info(state,Progress16(0),history);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(6,2)], state[Square(7,3)], Square(7,1))));
  }
}

BOOST_AUTO_TEST_CASE(MPFeatureTestBreakThreatmate)
{
  MoveStack history;
  { 
    NumEffectState state(CsaString(
			   "P1+HI *  *  * -GI *  * -KE-KY\n"
			   "P2 *  *  *  *  * -OU * -KA * \n"
			   "P3 *  * -FU-FU * -FU-KI-FU-FU\n"
			   "P4 *  *  *  *  *  * -FU *  * \n"
			   "P5 * -FU+FU *  *  *  *  *  * \n"
			   "P6 *  *  *  *  *  *  *  *  * \n"
			   "P7+KY *  * +FU+FU+FU+FU * +FU\n"
			   "P8 *  * +GI-HI *  *  * +KY * \n"
			   "P9 * +KI *  * -KI+GI+OU+KE+KY\n"
			   "P+00KA00GI00KE00FU00FU00FU00FU\n"
			   "P-00KI00KE00FU\n"
			   "+\n").initialState());
    StateInfo info(state,Progress16(0),history,Move(Square(2,7),KNIGHT,WHITE));
    BOOST_CHECK_EQUAL(std::string("-0027KE"),  // "-5949KI", 
		      csa::show(info.threatmate_move));
    Move m58ke(Square(5,8),KNIGHT,BLACK);
    BOOST_CHECK(BreakThreatmate::isDefendingKing8
		(m58ke, Square(3,9), state));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  * -FU *  * -KE-KY\n"
			   "P2 *  * +HI *  * -GI-OU *  * \n"
			   "P3-FU * -KE * +TO-FU * -FU-FU\n"
			   "P4 *  * -FU *  * -KI-KA *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 * +FU+FU+KA *  *  *  *  * \n"
			   "P7+FU+OU+GI *  *  * -NG * +FU\n"
			   "P8 * +KI * +KI *  *  *  *  * \n"
			   "P9+KY-RY *  *  *  *  * +KE+KY\n"
			   "P+00KI00FU00FU00FU00FU\n"
			   "P-00GI00KE00FU00FU00FU\n"
			   "-\n").initialState());
    Move m72ry(Square(7,2),Square(4,2),PROOK,SILVER,true,BLACK);
    StateInfo info(state,Progress16(0),history,m72ry);
    BOOST_CHECK_EQUAL(csa::show(info.threatmate_move),
			 std::string("+7242RY"));
    Move m52gi(Square(5,2),SILVER,WHITE);
    BOOST_CHECK(BreakThreatmate::isDefendingKing8
		(m52gi, Square(3,2), state));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY+NK+KA * -KI *  *  * -KY\n"
			   "P2 *  *  *  * +TO *  * +FU * \n"
			   "P3 * +TO-FU+NK * -GI * -FU * \n"
			   "P4-FU *  *  * -FU *  *  * -FU\n"
			   "P5 *  *  * -FU-OU-UM+FU *  * \n"
			   "P6 * +HI *  * -KE *  *  * +FU\n"
			   "P7+FU-FU+KI+FU * +FU *  *  * \n"
			   "P8 *  *  *  *  * +KI *  *  * \n"
			   "P9+KY * +GI+OU+KI *  *  * +KY\n"
			   "P+00GI\n"
			   "P-00HI00GI00KE00FU00FU00FU\n"
			   "-\n").initialState());
    StateInfo info(state,Progress16(0),history,Move(Square(6,4),SILVER,BLACK));
    BOOST_CHECK_EQUAL(csa::show(info.threatmate_move),
			 std::string("+0064GI"));
    Move m62gi(Square(6,2),SILVER,WHITE);
    BOOST_CHECK(BreakThreatmate::isDefendingKing8
		(m62gi, Square(5,5), state));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY+TO *  *  *  *  *  * -KY\n"
			   "P2 *  *  *  *  *  *  * +RY * \n"
			   "P3-OU * -FU-FU *  *  *  * -FU\n"
			   "P4-FU-FU *  * -FU *  *  *  * \n"
			   "P5 *  *  *  *  * -HI *  *  * \n"
			   "P6 *  * +FU+UM *  *  *  *  * \n"
			   "P7+FU+FU * +FU * -UM *  * +FU\n"
			   "P8 *  * +OU+GI * -FU *  *  * \n"
			   "P9+KY+KE *  * +KI * -TO * +KY\n"
			   "P+00KI00GI00GI00KE00KE00FU00FU\n"
			   "P-00KI00KI00GI00KE00FU00FU\n"
			   "-\n").initialState());
    Move m82ry(Square(2,2),Square(8,2),PROOK,PTYPE_EMPTY,false,BLACK);
    StateInfo info(state,Progress16(0),history,m82ry);
    BOOST_CHECK_EQUAL(csa::show(info.threatmate_move),
			 std::string("+2282RY"));
    Move m42ke(Square(4,2),KNIGHT,WHITE);
    BOOST_CHECK(BreakThreatmate::isDefendingKing8
		(m42ke, Square(9,3), state));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  *  * -KE-KY\n"
			   "P2 *  *  *  *  * -KI * -OU * \n"
			   "P3-FU+GI+UM * -GI-KI-FU * -FU\n"
			   "P4 *  * -FU * -FU *  * -FU * \n"
			   "P5 *  *  *  *  * -FU+FU *  * \n"
			   "P6 * +GI+FU+KA+FU *  *  *  * \n"
			   "P7+FU *  *  *  * +FU *  * +FU\n"
			   "P8 * +OU-GI-TO *  * +HI *  * \n"
			   "P9+KY+KE *  *  *  *  * +KE+KY\n"
			   "P+00HI00KE00FU00FU\n"
			   "P-00KI00KI00FU00FU\n"
			   "+\n").initialState());
    StateInfo info(state,Progress16(0),history,Move(Square(8,7),GOLD,WHITE));
    BOOST_CHECK_EQUAL(csa::show(info.threatmate_move),
			 std::string("-0087KI"));
    Move m87hi(Square(8,7),ROOK,BLACK);
    BOOST_CHECK(BreakThreatmate::isDefendingKing8
		(m87hi, Square(8,8), state));
  }
  {
    NumEffectState state(CsaString(
			   "P1 *  *  *  *  *  * -KI-KE * \n"
			   "P2 *  *  *  * +NK *  * -OU * \n"
			   "P3 * +NY * -FU *  * -KE-FU-FU\n"
			   "P4 *  * -FU * +GI * -FU *  * \n"
			   "P5-GI *  *  * +KA-FU *  * +FU\n"
			   "P6 *  * +FU *  *  * +FU+FU * \n"
			   "P7 *  * +KI *  * +FU *  *  * \n"
			   "P8-KI+GI *  * -GI *  *  * +HI\n"
			   "P9 *  * +OU * -UM *  *  *  * \n"
			   "P+00HI00KI00KE00KY00KY00KY00FU00FU00FU00FU00FU\n"
			   "P-00FU00FU\n"
			   "+\n").initialState());
    Move m69um(Square(5,9),Square(6,9),PBISHOP,PTYPE_EMPTY,false,WHITE);
    StateInfo info(state,Progress16(0),history,m69um);
    BOOST_CHECK_EQUAL(info.threatmate_move, m69um);
    Move m19hi(Square(1,8),Square(1,9),ROOK,PTYPE_EMPTY,false,BLACK);
    BOOST_CHECK(BreakThreatmate::isDefendingThreatmate
		(m19hi, m69um, state));
    BOOST_CHECK(BreakThreatmate::isDefendingThreatmate
		(Move(Square(4,9),ROOK,BLACK), m69um, state));
    BOOST_CHECK(! BreakThreatmate::isDefendingThreatmate
		(Move(Square(8,9),ROOK,BLACK), m69um, state));
    BOOST_CHECK(! BreakThreatmate::isDefendingThreatmate
		(Move(Square(3,7),BISHOP,BLACK), m69um, state));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY * +RY *  *  *  *  * +KI\n"
			   "P2 *  *  *  *  * -FU-FU *  * \n"
			   "P3-FU-FU+TO-FU * -KI * -FU-FU\n"
			   "P4 *  *  *  *  * +UM-GI * -OU\n"
			   "P5 * -KE * +FU *  *  *  *  * \n"
			   "P6 *  *  *  * +FU * +KE+FU * \n"
			   "P7+FU+FU+KE * +KI * +GI * +FU\n"
			   "P8 *  * +OU+KI *  *  *  * -RY\n"
			   "P9+KY *  *  *  *  *  *  *  * \n"
			   "P+00GI00KY00FU00FU00FU\n"
			   "P-00KA00GI00KE00KY00FU\n"
			   "-\n").initialState());
    Move m15gi(Square(1,5),SILVER,BLACK);
    StateInfo info(state,Progress16(0),history,m15gi);
    BOOST_CHECK_EQUAL(info.threatmate_move, m15gi);
    Move m15ke(Square(1,5),KNIGHT,WHITE);
    BOOST_CHECK(BreakThreatmate::isDefendingThreatmate
		(m15ke, m15gi, state));
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
