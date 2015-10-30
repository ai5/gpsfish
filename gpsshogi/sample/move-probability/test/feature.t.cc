#include "feature.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>

using namespace osl;

using gpsshogi::StateInfo;
using gpsshogi::PinnedGeneral;
BOOST_AUTO_TEST_CASE(FeatureTestPatternCommon)
{
  BOOST_CHECK_EQUAL(48, gpsshogi::PatternCommon::PromotionBase);
  BOOST_CHECK_EQUAL(64, gpsshogi::PatternCommon::SquareDim);
}

BOOST_AUTO_TEST_CASE(FeatureTestSupportAttack)
{  
  std::vector<Move> moves;
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
    size_t index = gpsshogi::ToEffect::supportAttack
      (StateInfo(state,moves,-1), Square(3,5), m36fu);
    BOOST_CHECK_EQUAL(PTYPE_EMPTY*PTYPE_SIZE + ROOK, index);
  }
}

BOOST_AUTO_TEST_CASE(FeatureTestBlockLong)
{
  using gpsshogi::BlockLong;
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
  std::vector<Move> moves;
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
    gpsshogi::index_list_t index;
    Move m55(Square(5,5), BISHOP, BLACK);
    BlockLong::findAll(StateInfo(state,moves,-1), m55, index);
    const int base = BlockLong::ptypeSupport(BISHOP, true);
    int add = (BlockLong::longAttackIndex(newPtypeO(WHITE,BISHOP))*PTYPEO_SIZE
		+ ptypeOIndex(newPtypeO(BLACK,ROOK)))*8;
    BOOST_CHECK_EQUAL(6, index.size());
    BOOST_CHECK_EQUAL(base+add, index[0].first);
    BOOST_CHECK_EQUAL(base+add+2, index[1].first);
    BOOST_CHECK_EQUAL(base+add+3, index[2].first);
    BOOST_CHECK_EQUAL(base+add+4, index[3].first);
    BOOST_CHECK_EQUAL(base+add+5, index[4].first);
    BOOST_CHECK_EQUAL(base+add+6, index[5].first);
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
    gpsshogi::index_list_t index;
    Move m55(Square(5,5), BISHOP, BLACK);
    BlockLong::findAll(StateInfo(state,moves,-1), m55, index);
    const int base = BlockLong::ptypeSupport(BISHOP, true);
    int add = (BlockLong::longAttackIndex(newPtypeO(WHITE,BISHOP))*PTYPEO_SIZE
		+ ptypeOIndex(newPtypeO(BLACK,ROOK)))*8;
    BOOST_CHECK_EQUAL(5, index.size());
    BOOST_CHECK_EQUAL(base+add, index[0].first);
    BOOST_CHECK_EQUAL(base+add+3, index[1].first);
    BOOST_CHECK_EQUAL(base+add+4, index[2].first);
    BOOST_CHECK_EQUAL(base+add+5, index[3].first);
    BOOST_CHECK_EQUAL(base+add+6, index[4].first);
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
    gpsshogi::index_list_t index;
    Move m55(Square(5,5), BISHOP, BLACK);
    BlockLong::findAll(StateInfo(state,moves,-1), m55, index);
    const int base = BlockLong::ptypeSupport(BISHOP, true);
    int add = (BlockLong::longAttackIndex(newPtypeO(WHITE,BISHOP))*PTYPEO_SIZE
		+ ptypeOIndex(newPtypeO(WHITE,ROOK)))*8;
    BOOST_CHECK_EQUAL(6, index.size());
    BOOST_CHECK_EQUAL(base+add, index[0].first);
    BOOST_CHECK_EQUAL(base+add+2, index[1].first);
    BOOST_CHECK_EQUAL(base+add+3, index[2].first);
    BOOST_CHECK_EQUAL(base+add+4, index[3].first);
    BOOST_CHECK_EQUAL(base+add+5, index[4].first);
    BOOST_CHECK_EQUAL(base+add+6, index[5].first);
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
    gpsshogi::index_list_t index;
    Move m55(Square(5,5), BISHOP, WHITE);
    BlockLong::findAll(StateInfo(state,moves,-1), m55, index);
    const int base = BlockLong::ptypeSupport(BISHOP, true);
    int add = (BlockLong::longAttackIndex(newPtypeO(BLACK,BISHOP))*PTYPEO_SIZE // turn's
	       + ptypeOIndex(newPtypeO(WHITE,ROOK)))*8;     // opponent's
    BOOST_CHECK_EQUAL(1, index.size());
    BOOST_CHECK_EQUAL(base+add, index[0].first);
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
    gpsshogi::index_list_t index;
    Move m55(Square(5,5), BISHOP, WHITE);
    BlockLong::findAll(StateInfo(state,moves,-1), m55, index);
    const int base = BlockLong::ptypeSupport(BISHOP, false);
    int add = (BlockLong::longAttackIndex(newPtypeO(WHITE,BISHOP))*PTYPEO_SIZE // opponent's
	       + ptypeOIndex(newPtypeO(BLACK,ROOK)))*8;     // turn's
    BOOST_CHECK_EQUAL(2, index.size());
    BOOST_CHECK_EQUAL(base+add, index[0].first);
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
    gpsshogi::index_list_t index;
    Move m55(Square(5,5), BISHOP, BLACK);
    BlockLong::findAll(StateInfo(state,moves,-1), m55, index);
    const int base = BlockLong::ptypeSupport(BISHOP, true);
    int add = (BlockLong::longAttackIndex(newPtypeO(WHITE,BISHOP))*PTYPEO_SIZE
	       + ptypeOIndex(PTYPEO_EMPTY))*8;
    BOOST_CHECK_EQUAL(3, index.size());
    BOOST_CHECK_EQUAL(base+add, index[0].first);
    BOOST_CHECK_EQUAL(base+add+2, index[1].first);
    BOOST_CHECK_EQUAL(base+add+3, index[2].first);
  } 
}

BOOST_AUTO_TEST_CASE(FeatureTestLureDefender)
{  
  // gold
  std::vector<Move> moves;
  {
    NumEffectState state(CsaString(
			   "P1 * -OU-FU * -GI * +RY  *  * \n"
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
    StateInfo info(state,moves,-1);
    BOOST_CHECK_EQUAL(5, info.exchange_pins[BLACK].size());
    BOOST_CHECK_EQUAL(2, info.exchange_pins[WHITE].size());
    BOOST_CHECK_EQUAL(Square(5,3), info.exchange_pins[WHITE][0].attack);
    BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(5,1)),
	      info.exchange_pins[WHITE][0].covered);

    BOOST_CHECK_EQUAL(Square(4,7), info.exchange_pins[BLACK][0].attack);
    BOOST_CHECK_EQUAL(Square(3,7), info.exchange_pins[BLACK][1].attack);
    BOOST_CHECK_EQUAL(Square(2,7), info.exchange_pins[BLACK][2].attack);
    BOOST_CHECK_EQUAL(state.pieceOnBoard(Square(3,9)),
	      info.exchange_pins[BLACK][0].covered);
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
    StateInfo info(state,moves,-1);
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
    StateInfo info(state,moves,-1);
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
    StateInfo info(state,moves,-1);
    BOOST_CHECK(info.exchange_pins[BLACK].isMember
		(PinnedGeneral(state[Square(5,8)], state[Square(4,9)], Square(5,7))));
    BOOST_CHECK(info.exchange_pins[BLACK].isMember
		(PinnedGeneral(state[Square(5,8)], state[Square(4,9)], Square(4,7))));
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(5,2)], state[Square(4,1)], Square(6,3))));
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(5,2)], state[Square(4,1)], Square(4,3))));
    BOOST_CHECK_EQUAL(2, info.exchange_pins[BLACK].size());
    BOOST_CHECK_EQUAL(3, info.exchange_pins[WHITE].size());
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
    StateInfo info(state,moves,-1);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(6,2)], state[Square(5,1)], Square(6,3))));
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(6,2)], state[Square(5,1)], Square(7,3))));
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
    StateInfo info(state,moves,-1);
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
    StateInfo info(state,moves,-1);
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
    StateInfo info(state,moves,-1);
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
    StateInfo info(state,moves,-1);
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
    StateInfo info(state,moves,-1);
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
    StateInfo info(state,moves,-1);
    BOOST_CHECK(info.exchange_pins[WHITE].isMember
		(PinnedGeneral(state[Square(6,2)], state[Square(7,3)], Square(7,1))));
  }
}

BOOST_AUTO_TEST_CASE(FeatureTestLongRecapture)
{
  std::vector<Move> moves;
  { 
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  *  * -OU * +UM\n"
			   "P2 * -HI * -FU+NG-KE * -KI * \n"
			   "P3 *  * -KE * +NG *  * -FU * \n"
			   "P4-FU *  *  *  * -RY-GI * -FU\n"
			   "P5 *  *  *  * -FU *  *  *  * \n"
			   "P6+FU *  * +OU * -FU *  * +FU\n"
			   "P7 * +FU * +FU+FU-KI *  *  * \n"
			   "P8 *  * +KI+GI *  *  *  *  * \n"
			   "P9+KY+KE+FU *  *  *  *  * -NY\n"
			   "P+00KA00KI00KE00FU00FU00FU\n"
			   "P-00KY00FU00FU00FU\n"
			   "+\n").initialState());
    StateInfo info(state,moves,-1);
    Move m21ka(Square(2,1),BISHOP,BLACK);
    gpsshogi::LongRecapture feature;
    gpsshogi::index_list_t out;
    feature.match(info, gpsshogi::MoveInfo(info, m21ka), 0, out);
    BOOST_CHECK(! out.empty());
  }
}

BOOST_AUTO_TEST_CASE(FeatureTestKingBlockade)
{
  std::vector<Move> moves;
  { 
    NumEffectState state(CsaString(
			   "P1-OU * +UM-GI *  *  * -KE-KY\n"
			   "P2-KY * -GI *  *  *  * -HI * \n"
			   "P3-FU-FU-GI *  *  *  *  *  * \n"
			   "P4 *  * +FU * -FU *  * +FU-FU\n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6+FU *  * +GI+FU *  *  * +FU\n"
			   "P7 * +FU * +FU *  * -TO * +KE\n"
			   "P8 * +OU * +KI * -TO * -TO * \n"
			   "P9+KY+KE *  *  *  *  *  * +KY\n"
			   "P+00KI00KE00FU00FU\n"
			   "P-00HI00KA00KI00KI00FU00FU\n"
			   "-\n").initialState());
    StateInfo info(state,moves,-1);
    Move m76hi(Square(7,6),ROOK,WHITE);
    gpsshogi::KingBlockade feature;
    gpsshogi::index_list_t out;
    feature.match(info, gpsshogi::MoveInfo(info, m76hi), 0, out);
    BOOST_CHECK_EQUAL(2, out.size());
    BOOST_CHECK_EQUAL(gpsshogi::KingBlockade::BlockSideWide, out[0].first % 5);
    BOOST_CHECK_EQUAL(gpsshogi::KingBlockade::BlockBack, out[1].first % 5);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE *  *  *  *  * -KE-KY\n"
			   "P2 * -HI *  *  *  * -KI-OU * \n"
			   "P3-FU-FU-FU-FU *  * +FU-FU-FU\n"
			   "P4 *  *  *  *  * +GI *  *  * \n"
			   "P5 *  *  *  *  *  *  * +FU * \n"
			   "P6 *  * +FU+FU * -UM *  *  * \n"
			   "P7+FU+FU+GI *  *  *  *  * +FU\n"
			   "P8 *  * +OU * -NY *  *  *  * \n"
			   "P9+KY+KE *  *  *  *  * +KE * \n"
			   "P+00HI00KA00KI00GI00FU00FU\n"
			   "P-00KI00KI00GI00FU00FU00FU\n"
			   "+\n").initialState());
    StateInfo info(state,moves,-1);
    Move m86ka(Square(8,6),BISHOP,BLACK);
    gpsshogi::KingBlockade feature;
    gpsshogi::index_list_t out;
    feature.match(info, gpsshogi::MoveInfo(info, m86ka), 0, out);
    BOOST_CHECK_EQUAL(2, out.size());
    BOOST_CHECK_EQUAL(gpsshogi::KingBlockade::BlockSideWide, out[0].first % 5);
    BOOST_CHECK_EQUAL(gpsshogi::KingBlockade::BlockBack, out[1].first % 5);
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE+RY *  *  *  *  * -KY\n"
			   "P2 *  *  *  * -KI+TO *  *  * \n"
			   "P3-FU-FU-FU-OU-FU-FU * +NG-FU\n"
			   "P4 *  *  * -FU *  *  * -FU * \n"
			   "P5 *  *  *  * +FU-UM *  *  * \n"
			   "P6 *  * -KI *  *  *  *  *  * \n"
			   "P7+FU+FU-KY+FU+KI+FU *  * +FU\n"
			   "P8 *  *  * +OU * +KI *  *  * \n"
			   "P9 * -UM+GI * +KE *  *  * +KY\n"
			   "P+00HI00GI00KE00FU00FU\n"
			   "P-00GI00KE00FU\n"
			   "+\n").initialState());
    StateInfo info(state,moves,-1);
    Move m86ke(Square(8,6),KNIGHT,BLACK);
    gpsshogi::KingBlockade feature;
    gpsshogi::index_list_t out;
    feature.match(info, gpsshogi::MoveInfo(info, m86ke), 0, out);
    BOOST_CHECK_EQUAL(3, out.size());
    BOOST_CHECK_EQUAL(gpsshogi::KingBlockade::BlockLastOne, out[0].first % 5);
    BOOST_CHECK_EQUAL(gpsshogi::KingBlockade::BlockFront, out[1].first % 5);
    BOOST_CHECK_EQUAL(gpsshogi::KingBlockade::BlockSideOther, out[2].first % 5);
  }
}

BOOST_AUTO_TEST_CASE(FeatureTestCoverFork)
{
  using gpsshogi::CoverFork;
  std::vector<Move> moves;
  { 
    NumEffectState state(CsaString(
			   "P1-KY *  *  *  * -KI-GI-KE-KY\n"
			   "P2 *  * +RY *  * -KI-OU-KA * \n"
			   "P3-FU *  *  *  * -FU * -FU * \n"
			   "P4-KA *  * +FU-FU * -FU-KE * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6+FU *  *  * +FU+FU+KI * -FU\n"
			   "P7 *  *  *  * +GI+GI+FU+FU * \n"
			   "P8 *  *  *  *  *  *  * +OU+FU\n"
			   "P9 *  * +FU *  * +KI * +KE+KY\n"
			   "P+00HI00GI00KY00FU00FU\n"
			   "P-00KE00FU00FU\n"
			   "+\n").initialState());
    StateInfo info(state,moves,-1);
    Move m76hi(Square(7,6),ROOK,BLACK);
    BOOST_CHECK(CoverFork::defending(state, m76hi, Square(4,9)));
    BOOST_CHECK(CoverFork::defending(state, m76hi, Square(7,2)));
    CoverFork feature;
    gpsshogi::index_list_t out;
    feature.match(info, gpsshogi::MoveInfo(info, m76hi), 0, out);
    BOOST_CHECK_EQUAL(1*4, out.size());
    BOOST_CHECK_EQUAL(((ROOK*PTYPE_SIZE)+GOLD)*PTYPE_SIZE+PROOK, out[3].first);
  }
}

BOOST_AUTO_TEST_CASE(FeatureTestPromotionBySacrifice)
{
  using gpsshogi::CoverFork;
  std::vector<Move> moves;
  { 
    NumEffectState state(CsaString(
			   "P1-KY * +RY * -KI-OU *  * -KY\n"
			   "P2 *  *  *  *  * -GI-KI *  * \n"
			   "P3-FU * +GI * +FU-FU * -FU-FU\n"
			   "P4 * -HI *  *  *  *  * -KA * \n"
			   "P5 * -KE+FU-FU * -KE-FU *  * \n"
			   "P6 *  *  *  *  * +GI *  *  * \n"
			   "P7+FU *  * +FU * +FU+FU+FU+FU\n"
			   "P8 *  * +KI *  *  *  *  *  * \n"
			   "P9+KY+KE * +OU * +KI+GI+KE+KY\n"
			   "P+00FU00FU00FU\n"
			   "P-00KA00FU\n"
			   "-\n").initialState());
    StateInfo info(state,moves,-1);
    Move m77ke(Square(8,5),Square(7,7),KNIGHT,PTYPE_EMPTY,false,WHITE);
    gpsshogi::PromotionBySacrifice feature;
    gpsshogi::index_list_t out;
    feature.match(info, gpsshogi::MoveInfo(info, m77ke), 0, out);
    BOOST_CHECK(out.size() > 0);
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
