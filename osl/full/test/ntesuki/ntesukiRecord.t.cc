#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/move.h"
#include "osl/record/csaString.h"
#include "osl/record/csaRecord.h"
#include "osl/ntesuki/ntesukiMoveGenerator.h"
//#include "osl/ntesuki/ntesukiRecordPtr.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl;
using namespace osl::ntesuki;

class NtesukiRecordTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NtesukiRecordTest);
  CPPUNIT_TEST(testGenerate);
  CPPUNIT_TEST(testModify);
  CPPUNIT_TEST(testDominanceSet_P);
  //CPPUNIT_TEST(testDominanceSet_Q);

  //CPPUNIT_TEST(testDominanceSetWhite_P);
  //CPPUNIT_TEST(testDominanceSetWhite_Q);
  //CPPUNIT_TEST(testSmartPtr);
  CPPUNIT_TEST_SUITE_END();
public:
  void testGenerate()
  {
    NtesukiRecord::RecordList list;
    NtesukiRecord nr(0u, HashKey(), PieceStand(), &list);

    for (size_t i = 0; i < NtesukiRecord::SIZE; ++i)
    {
      BOOST_CHECK_EQUAL(ProofDisproof(1, 1), nr.getValue<BLACK>(i));
      BOOST_CHECK_EQUAL(ProofDisproof(1, 1), nr.getValue<WHITE>(i));
    }
    BOOST_CHECK(!nr.isVisited());

    SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -OU * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  * +FU * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n").initialState();
    NumEffectState nState(state);
  
    osl::Square lastTo = Square::STAND();

    NtesukiMoveGenerator mg(OslConfig::verbose());
    NtesukiRecord::mg = &mg;
    NtesukiRecord::state = &nState;
    NtesukiMoveList movelist;
    mg.generate<BLACK>(nState, movelist);
  }

  void testModify()
  {
    NtesukiRecord::RecordList list;
    NtesukiRecord nr(0, HashKey(), PieceStand(), &list);
    const NtesukiRecord& nrc = nr;

    nr.setResult<BLACK>(0, ProofDisproof(), NtesukiMove::INVALID(), false);
    BOOST_CHECK_EQUAL(ProofDisproof(),
			 nrc.getValue<BLACK>(0));

    nr.setResult<BLACK>(1, ProofDisproof(), NtesukiMove::INVALID(), false);
    BOOST_CHECK_EQUAL(ProofDisproof(),
			 nrc.getValue<BLACK>(1));
  }

  /* Initial settings
   *  P-b P-w Q-b Q-w
   *    1   0   0   1
   *    0   1   1   0
   */

  /* BLACK_P P has an extra pawn for BLACK compared to Q
   *
   *  Before: both P and Q is unknown
   *  - L_B^0(P) is success -> nothing
   *  - L_B^0(P) is fail    -> L_B^0(Q) is fail
   *  - L_W^0(P) is success -> L_W^0(Q) is success
   *  - L_W^0(P) is fail    -> nothing
   *  - L_B^1(P) is success -> nothing
   *  - L_B^1(P) is fail    -> L_B^1(Q) is fail
   *  - L_W^1(P) is success -> L_W^1(Q) is success
   *  - L_W^1(P) is fail    -> nothing
   */

  /* BLACK_Q Q has an extra pawn for BLACK compared to P
   *
   *  - L_B^0(P) is success -> L_B^0(Q) is success
   *  - L_B^0(P) is fail    -> nothing
   *  - L_W^0(P) is success -> L_W^0(Q) is success
   *  - L_W^0(P) is fail    -> L_W^0(Q) is fail
   *  - L_B^1(P) is success -> L_B^1(Q) is success
   *  - L_B^1(P) is fail    -> nothing
   *  - L_W^1(P) is success -> nothing
   *  - L_W^1(P) is fail    -> L_W^1(Q) is fail
   */


#define SUCC ProofDisproof::Checkmate()
#define FAIL ProofDisproof::NoCheckmate()
#define AB ProofDisproof::AttackBack()
#define NONE ProofDisproof()

  //for experiments with positions where black is to play
#define TEST_DOMINANCE_BEFORE(I,PL,PV,BV, WV) \
    {\
      if (OslConfig::verbose()) std::cerr << "\nBEFORE " << I << "\t" << PL << "\t" << PV << "\t" << BV << "\t" << WV;\
      NumEffectState state(sstate);\
      NtesukiRecord::RecordList list;\
      NtesukiRecord best_record(0, HashKey(), PieceStand(), &list);\
      NtesukiMove best_move = Move(Square(7,7), Square(7,6), PAWN, PTYPE_EMPTY, false, PL);\
\
      list.push_front(NtesukiRecord(0, key_P, ps_P_W, &list));\
      NtesukiRecord& record_P = list.front();\
      NtesukiRecord::state = &state;\
\
      if (PL == BLACK && PV == SUCC)\
        record_P.setResult<PL>(I, PV, best_move, false, &pp_B);\
      else if (PL == WHITE && PV == SUCC)\
	record_P.setResult<PL>(I, PV, NtesukiMove::INVALID(), false, &pp_W);\
      else if (PL == BLACK && PV == FAIL)\
        record_P.setResult<PL>(I, PV, NtesukiMove::INVALID(), false, &pp_W);\
      else if (PL == WHITE && PV == FAIL)\
	record_P.setResult<PL>(I, PV, best_move, false, &pp_B);\
      else\
        record_P.setResult<PL>(I, PV, NtesukiMove::INVALID(), false, NULL);\
\
      list.push_front(NtesukiRecord(0, key_Q, ps_Q_W, &list));\
      NtesukiRecord& record_Q = list.front();\
\
      BOOST_CHECK_EQUAL(BV, record_Q.getValue<BLACK>(I));\
      BOOST_CHECK_EQUAL(WV, record_Q.getValue<WHITE>(I));\
    }\

#define TEST_DOMINANCE_AFTER(I,PL,PV,BV,WV) \
    {\
      if (OslConfig::verbose()) std::cerr << "\nAFTER  " << I << "\t" << PL << "\t" << PV << "\t" << BV << "\t" << WV;\
      NumEffectState state(sstate);\
      NtesukiRecord::RecordList list;\
      NtesukiRecord best_record(0, HashKey(), PieceStand(), &list);\
      NtesukiMove best_move = Move(Square(7,7), Square(7,6), PAWN, PTYPE_EMPTY, false, PL);\
      NtesukiRecord::state = &state;\
\
      list.push_front(NtesukiRecord(0, key_P, ps_P_W, &list));\
      NtesukiRecord& record_P = list.front();\
      list.push_front(NtesukiRecord(0, key_Q, ps_Q_W, &list));\
      NtesukiRecord& record_Q = list.front();\
\
      if (PL == BLACK && PV == SUCC)\
        record_P.setResult<PL>(I, PV, best_move, false, &pp_B);\
      else if (PL == WHITE && PV == SUCC)\
	record_P.setResult<PL>(I, PV, NtesukiMove::INVALID(), false, &pp_W);\
      else if (PL == BLACK && PV == FAIL)\
        record_P.setResult<PL>(I, PV, NtesukiMove::INVALID(), false, &pp_W);\
      else if (PL == WHITE && PV == FAIL)\
	record_P.setResult<PL>(I, PV, best_move, false, &pp_B);\
      else\
        record_P.setResult<PL>(I, PV, NtesukiMove::INVALID(), false, NULL);\
\
      BOOST_CHECK_EQUAL(BV, record_Q.getValue<BLACK>(I));\
      BOOST_CHECK_EQUAL(WV, record_Q.getValue<WHITE>(I));\
    }\

    
  void testDominanceSet_P()
  {
    NtesukiRecord::use_dominance = true;
    HashKey key_P, key_Q;

    SimpleState sstate = CsaString(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU+FU * \n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P+00FU\n"
    "+\n").initialState();;
    key_P = HashKey(sstate);
    PieceStand ps_P_W(0, 0, 0, 0, 0, 0, 0, 0);
    PieceStand ps_Q_W(1, 0, 0, 0, 0, 0, 0, 0);

    PieceStand pp_B(1, 0, 0, 0, 0, 0, 0, 0);
    PieceStand pp_W(0, 0, 0, 0, 0, 0, 0, 0);

    //  - nothing -> nothing
    TEST_DOMINANCE_BEFORE(0, BLACK, NONE, NONE, NONE);
    TEST_DOMINANCE_AFTER(0, BLACK, NONE, NONE, NONE);
    //  - L_B^0(P) is success -> nothing
    TEST_DOMINANCE_BEFORE(0, BLACK, SUCC, NONE, NONE);
    TEST_DOMINANCE_AFTER(0, BLACK, SUCC, NONE, NONE);
    //  - L_B^0(P) is fail    -> L_B^0(Q) is fail
    TEST_DOMINANCE_BEFORE(0, BLACK, FAIL, FAIL, NONE);
    TEST_DOMINANCE_AFTER(0, BLACK, FAIL, FAIL, NONE);
    //  - L_B^1(P) is success -> nothing
    TEST_DOMINANCE_BEFORE(1, BLACK, SUCC, NONE, NONE);
    TEST_DOMINANCE_AFTER(1, BLACK, SUCC, NONE, NONE);
    //  - L_B^1(P) is fail    -> L_B^1(Q) is fail
    TEST_DOMINANCE_BEFORE(1, BLACK, FAIL, FAIL, NONE);
    TEST_DOMINANCE_AFTER(1, BLACK, FAIL, FAIL, NONE);
    //  - L_W^0(P) is success -> L_W^0(Q) is success
    TEST_DOMINANCE_BEFORE(0, WHITE, SUCC, AB, SUCC);
    TEST_DOMINANCE_AFTER(0, WHITE, SUCC, AB, SUCC);
    //  - L_W^0(P) is fail    -> nothing
    TEST_DOMINANCE_BEFORE(0, WHITE, FAIL, NONE, NONE);
    TEST_DOMINANCE_AFTER(0, WHITE, FAIL, NONE, NONE);
    //  - L_W^1(P) is success -> L_W^1(Q) is success
    TEST_DOMINANCE_BEFORE(1, WHITE, SUCC, AB, SUCC);
    TEST_DOMINANCE_AFTER(1, WHITE, SUCC, AB, SUCC);
    //  - L_W^1(P) is fail    -> nothing
    TEST_DOMINANCE_BEFORE(1, WHITE, FAIL, NONE, NONE);
    TEST_DOMINANCE_AFTER(1, WHITE, FAIL, NONE, NONE);
  }

  void testDominanceSet_Q()
  {
    HashKey key_P, key_Q;

    SimpleState sstate = CsaString(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU+FU * \n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "P-00FU\n"
    "+\n").initialState();;
    key_P = HashKey(sstate);
    PieceStand ps_P_W(1, 0, 0, 0, 0, 0, 0, 0);
    PieceStand ps_Q_W(0, 0, 0, 0, 0, 0, 0, 0);

    PieceStand pp_B(0, 0, 0, 0, 0, 0, 0, 0);
    PieceStand pp_W(1, 0, 0, 0, 0, 0, 0, 0);

    //  - nothing -> nothing
    TEST_DOMINANCE_BEFORE(0, BLACK, NONE, NONE, NONE);
    TEST_DOMINANCE_AFTER(0, BLACK, NONE, NONE, NONE);
    //  - L_B^0(P) is success -> L_B^0(Q) is success
    TEST_DOMINANCE_BEFORE(0, BLACK, SUCC, SUCC, AB);
    TEST_DOMINANCE_AFTER(0, BLACK, SUCC, SUCC, AB);
    //  - L_B^0(P) is fail    -> nothing
    TEST_DOMINANCE_BEFORE(0, BLACK, FAIL, NONE, NONE);
    TEST_DOMINANCE_AFTER(0, BLACK, FAIL, NONE, NONE);
    //  - L_B^1(P) is success -> L_B^1(Q) is success
    TEST_DOMINANCE_BEFORE(1, BLACK, SUCC, SUCC, AB);
    TEST_DOMINANCE_AFTER(1, BLACK, SUCC, SUCC, AB);
    //  - L_B^1(P) is fail    -> nothing
    TEST_DOMINANCE_BEFORE(1, BLACK, FAIL, NONE, NONE);
    TEST_DOMINANCE_AFTER(1, BLACK, FAIL, NONE, NONE);
    //  - L_W^0(P) is success -> nothing
    TEST_DOMINANCE_BEFORE(0, WHITE, SUCC, NONE, NONE);
    TEST_DOMINANCE_AFTER(0, WHITE, SUCC, NONE, NONE);
    //  - L_W^0(P) is fail    -> L_W^0(Q) is fail
    TEST_DOMINANCE_BEFORE(0, WHITE, FAIL, NONE, FAIL);
    TEST_DOMINANCE_AFTER(0, WHITE, FAIL, NONE, FAIL);
    //  - L_W^1(P) is success -> nothing
    TEST_DOMINANCE_BEFORE(1, WHITE, SUCC, NONE, NONE);
    TEST_DOMINANCE_AFTER(1, WHITE, SUCC, NONE, NONE);
    //  - L_W^1(P) is fail    -> L_W^1(Q) is fail
    TEST_DOMINANCE_BEFORE(1, WHITE, FAIL, NONE, FAIL);
    TEST_DOMINANCE_AFTER(1, WHITE, FAIL, NONE, FAIL);
  }


#undef SUCC
#undef FAIL
#undef AB
#undef NONE

#undef TEST_DOMINANCE

#if 0
  void testSmartPtr()
  {
    NtesukiRecord::RecordList list;
    NtesukiRecord nr(0u, HashKey(), &list);

    {
      NtesukiRecordPtr ptr(&nr);
      BOOST_CHECK_EQUAL(nr.key, ptr->key);
      BOOST_CHECK_EQUAL(nr.key, (*ptr).key);

      NtesukiRecordPtr ptr2 = ptr;
      BOOST_CHECK_EQUAL(2, nr.ref_count);
      NtesukiRecordPtr ptr3(ptr);
      BOOST_CHECK_EQUAL(3, nr.ref_count);
    }
    BOOST_CHECK_EQUAL(0, nr.ref_count);
  }
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(NtesukiRecordTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
