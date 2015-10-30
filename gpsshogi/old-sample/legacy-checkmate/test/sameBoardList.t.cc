/* sameBoardList.t.cc
 */
#include "osl/checkmate/sameBoardList.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl;
using namespace osl::checkmate;

struct SameBoardListTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(SameBoardListTest);
  CPPUNIT_TEST(testFindIneffectiveDropLoop);
  CPPUNIT_TEST(testAllocateBlack);
  CPPUNIT_TEST(testAllocateWhite);
  CPPUNIT_TEST_SUITE_END();

  void testFindIneffectiveDropLoop();
  void testAllocateBlack();
  void testAllocateWhite();
};
CPPUNIT_TEST_SUITE_REGISTRATION(SameBoardListTest);

void SameBoardListTest::testFindIneffectiveDropLoop()
{
  SameBoardList l;
  PieceStand empty, black_silver, white_stand;
  black_silver.add(SILVER);
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<BLACK>(black_silver) == 0);
  PathEncoding path;
  size_t counter;
  CheckHashRecord *black_silverRecord
    = l.allocate<BLACK>(black_silver, white_stand, path, counter);
  CPPUNIT_ASSERT(black_silverRecord);

  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<BLACK>(black_silver) == 0);
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<BLACK>(empty) == 0);
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<WHITE>(black_silver) == 0);
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<WHITE>(empty) == 0);

  black_silverRecord->isVisited = true;

  CPPUNIT_ASSERT(black_silverRecord
		 == (l.findIneffectiveDropLoop<BLACK>(empty)));
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<BLACK>(black_silver) == 0);
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<WHITE>(black_silver) == 0);
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<WHITE>(empty) == 0);

  black_silverRecord->isVisited = false;
  CheckHashRecord *emptyRecord
    = l.allocate<BLACK>(empty, white_stand, path, counter);
  emptyRecord->isVisited = true;
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<BLACK>(black_silver) == 0);
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<BLACK>(empty) == 0);
  CPPUNIT_ASSERT(emptyRecord
		 == (l.findIneffectiveDropLoop<WHITE>(black_silver)));
  CPPUNIT_ASSERT(l.findIneffectiveDropLoop<WHITE>(empty) == 0);
}

static const ProofDisproof normalPdp(3,5);

/** 先手番 */
void SameBoardListTest::testAllocateBlack()
{
  HashEffectState state(CsaString(
			  "P1+NY+TO *  *  *  * -OU-KE-KY\n"
			  "P2 *  *  *  *  * -GI-KI *  *\n"
			  "P3 * +RY *  * +UM * -KI-FU-FU\n"
			  "P4 *  * +FU-FU *  *  *  *  *\n"
			  "P5 *  * -KE * +FU *  * +FU *\n"
			  "P6-KE *  * +FU+GI-FU *  * +FU\n"
			  "P7 *  * -UM *  *  *  *  *  *\n"
			  "P8 *  *  *  *  *  *  *  *  * \n"
			  "P9 * +OU * -GI *  *  *  * -NG\n"
			  "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			  "P-00KI00KY00FU00FU\n"
			  "P-00AL\n"
			  "+\n").getInitialState());
  const PieceStand white_stand(WHITE, state);
  const PathEncoding path_black(BLACK);
  size_t counter = 0;
  SameBoardList table;
  CheckHashRecord *record 
    = table.allocate<BLACK>(state.getHash().blackStand(), white_stand,
			    path_black, counter);
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT_EQUAL(1u, record->proof());
  CPPUNIT_ASSERT_EQUAL(1u, record->disproof());
  CPPUNIT_ASSERT_EQUAL((size_t)1u, counter);

  record->setProofDisproof(normalPdp);

  HashEffectState state2(CsaString( // 先手の歩が1枚少ない
			   "P1+NY+TO *  *  *  * -OU-KE-KY\n"
			   "P2 *  *  *  *  * -GI-KI *  *\n"
			   "P3 * +RY *  * +UM * -KI-FU-FU\n"
			   "P4 *  * +FU-FU *  *  *  *  *\n"
			   "P5 *  * -KE * +FU *  * +FU *\n"
			   "P6-KE *  * +FU+GI-FU *  * +FU\n"
			   "P7 *  * -UM *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU * -GI *  *  *  * -NG\n"
			   "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU00FU\n"
			   "P-00AL\n"
			   "+\n").getInitialState());
  const PieceStand white_stand2(WHITE, state2);
  CheckHashRecord *record2 
    = table.allocate<BLACK>(state2.getHash().blackStand(), white_stand2,
			    path_black, counter);
  CPPUNIT_ASSERT(record2);
  CPPUNIT_ASSERT(record2 != record);
  CPPUNIT_ASSERT_EQUAL((size_t)2u, counter);

  CPPUNIT_ASSERT_EQUAL(3u, record2->proof());
  CPPUNIT_ASSERT_EQUAL(1u, record2->disproof());

  HashEffectState state3(CsaString( // 後手の歩が1枚少ない
			   "P1+NY+TO *  *  *  * -OU-KE-KY\n"
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
			   "+\n").getInitialState());
  const PieceStand white_stand3(WHITE, state3);
  CheckHashRecord *record3 
    = table.allocate<BLACK>(state3.getHash().blackStand(), white_stand3,
			    path_black, counter);
  CPPUNIT_ASSERT(record3);
  CPPUNIT_ASSERT(record3 != record);
  CPPUNIT_ASSERT_EQUAL(1u, record3->proof());
  CPPUNIT_ASSERT_EQUAL(5u, record3->disproof());
  CPPUNIT_ASSERT_EQUAL((size_t)3u, counter);

  // propagate test

  record3->setProofPieces(record3->stand(BLACK));
  record3->propagateCheckmate<BLACK>(ProofDisproof::Checkmate());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record3->proofDisproof());
  CPPUNIT_ASSERT(record2->proofDisproof() != ProofDisproof::Checkmate());
  CPPUNIT_ASSERT(record->proofDisproof()  != ProofDisproof::Checkmate());
  
  record2->setProofPieces(record2->stand(BLACK));
  record2->propagateCheckmate<BLACK>(ProofDisproof::Checkmate());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record2->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record->proofDisproof());

  // reset
  SameBoardList *same_boards = record->sameBoards;
  *record  = CheckHashRecord(record->stand(BLACK),  record->stand(WHITE));
  *record2 = CheckHashRecord(record2->stand(BLACK), record2->stand(WHITE));
  *record3 = CheckHashRecord(record3->stand(BLACK), record3->stand(WHITE));
  record->setProofDisproof(normalPdp);
  record2->setProofDisproof(normalPdp);
  record3->setProofDisproof(normalPdp);
  record->sameBoards = record2->sameBoards = record->sameBoards
    = same_boards;
  
  record->setProofPieces(record->stand(BLACK));
  record->propagateCheckmate<BLACK>(ProofDisproof::Checkmate());
  CPPUNIT_ASSERT_EQUAL(normalPdp, record2->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record3->proofDisproof());

  *record  = CheckHashRecord(record->stand(BLACK),  record->stand(WHITE));
  *record2 = CheckHashRecord(record2->stand(BLACK), record2->stand(WHITE));
  *record3 = CheckHashRecord(record3->stand(BLACK), record3->stand(WHITE));
  record->setProofDisproof(normalPdp);
  record2->setProofDisproof(normalPdp);
  record3->setProofDisproof(normalPdp);
  record->sameBoards = record2->sameBoards = record->sameBoards
    = same_boards;

  record->setDisproofPieces(record->stand(WHITE));
  record->propagateNoCheckmate<BLACK>(ProofDisproof::NoCheckmate());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::NoCheckmate(), record->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::NoCheckmate(), record2->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(normalPdp, record3->proofDisproof());
}

/** 後手番 */
void SameBoardListTest::testAllocateWhite()
{
  HashEffectState state(CsaString(
			  "P1+NY+TO *  *  *  * -OU-KE-KY\n"
			  "P2 *  *  *  *  * -GI-KI *  *\n"
			  "P3 * +RY *  * +UM * -KI-FU-FU\n"
			  "P4 *  * +FU-FU *  *  *  *  *\n"
			  "P5 *  * -KE * +FU *  * +FU *\n"
			  "P6-KE *  * +FU+GI-FU *  * +FU\n"
			  "P7 *  * -UM *  *  *  *  *  *\n"
			  "P8 *  *  *  *  *  *  *  *  * \n"
			  "P9 * +OU * -GI *  *  *  * -NG\n"
			  "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			  "P-00KI00KY00FU00FU\n"
			  "P-00AL\n"
			  "-\n").getInitialState());
  const PieceStand white_stand(WHITE, state);
  const PathEncoding path_white(WHITE);
  SameBoardList table;
  size_t counter = 0;
  CheckHashRecord *record 
    = table.allocate<WHITE>(state.getHash().blackStand(), white_stand,
			    path_white, counter);
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT_EQUAL(1u, record->proof());
  CPPUNIT_ASSERT_EQUAL(1u, record->disproof());
  CPPUNIT_ASSERT_EQUAL((size_t)1u, counter);
  record->setProofDisproof(3,5);

  HashEffectState state2(CsaString( // 先手の歩が1枚少ない
			   "P1+NY+TO *  *  *  * -OU-KE-KY\n"
			   "P2 *  *  *  *  * -GI-KI *  *\n"
			   "P3 * +RY *  * +UM * -KI-FU-FU\n"
			   "P4 *  * +FU-FU *  *  *  *  *\n"
			   "P5 *  * -KE * +FU *  * +FU *\n"
			   "P6-KE *  * +FU+GI-FU *  * +FU\n"
			   "P7 *  * -UM *  *  *  *  *  *\n"
			   "P8 *  *  *  *  *  *  *  *  * \n"
			   "P9 * +OU * -GI *  *  *  * -NG\n"
			   "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU\n"
			   "P-00KI00KY00FU00FU00FU\n"
			   "P-00AL\n"
			   "-\n").getInitialState());
  const PieceStand white_stand2(WHITE, state2);
  CheckHashRecord *record2
    = table.allocate<WHITE>(state2.getHash().blackStand(), white_stand2,
			    path_white, counter);
  CPPUNIT_ASSERT(record2);
  CPPUNIT_ASSERT(record2 != record);
  CPPUNIT_ASSERT_EQUAL(1u, record2->proof());
  CPPUNIT_ASSERT_EQUAL(5u, record2->disproof());
  CPPUNIT_ASSERT_EQUAL((size_t)2u, counter);

  HashEffectState state3(CsaString( // 後手の歩が1枚少ない
			   "P1+NY+TO *  *  *  * -OU-KE-KY\n"
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
			   "-\n").getInitialState());
  const PieceStand white_stand3(WHITE, state3);
  CheckHashRecord *record3 
    = table.allocate<WHITE>(state3.getHash().blackStand(), white_stand3,
			    path_white, counter);
  CPPUNIT_ASSERT(record3);
  CPPUNIT_ASSERT(record3 != record);
  CPPUNIT_ASSERT_EQUAL(3u, record3->proof());
  CPPUNIT_ASSERT_EQUAL(1u, record3->disproof());
  CPPUNIT_ASSERT_EQUAL((size_t)3u, counter);

  // propagate test
  record2->setProofPieces(record2->stand(WHITE));
  record2->propagateCheckmate<WHITE>(ProofDisproof::Checkmate());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record2->proofDisproof());
  CPPUNIT_ASSERT(record3->proofDisproof() != ProofDisproof::Checkmate());
  CPPUNIT_ASSERT(record->proofDisproof()  != ProofDisproof::Checkmate());
  
  record3->setProofPieces(record3->stand(WHITE));
  record3->propagateCheckmate<WHITE>(ProofDisproof::Checkmate());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record3->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record->proofDisproof());

  // reset
  SameBoardList *same_boards = record->sameBoards;
  *record  = CheckHashRecord(record->stand(BLACK),  record->stand(WHITE));
  *record2 = CheckHashRecord(record2->stand(BLACK), record2->stand(WHITE));
  *record3 = CheckHashRecord(record3->stand(BLACK), record3->stand(WHITE));
  record->setProofDisproof(normalPdp);
  record2->setProofDisproof(normalPdp);
  record3->setProofDisproof(normalPdp);
  record->sameBoards = record2->sameBoards = record->sameBoards
    = same_boards;
  
  record->setProofPieces(record->stand(WHITE));
  record->propagateCheckmate<WHITE>(ProofDisproof::Checkmate());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::Checkmate(), record2->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(normalPdp, record3->proofDisproof());

  *record  = CheckHashRecord(record->stand(BLACK),  record->stand(WHITE));
  *record2 = CheckHashRecord(record2->stand(BLACK), record2->stand(WHITE));
  *record3 = CheckHashRecord(record3->stand(BLACK), record3->stand(WHITE));
  record->setProofDisproof(normalPdp);
  record2->setProofDisproof(normalPdp);
  record3->setProofDisproof(normalPdp);
  record->sameBoards = record2->sameBoards = record->sameBoards
    = same_boards;

  record->setDisproofPieces(record->stand(BLACK));
  record->propagateNoCheckmate<WHITE>(ProofDisproof::NoCheckmate());
  CPPUNIT_ASSERT_EQUAL(normalPdp, record2->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::NoCheckmate(), record->proofDisproof());
  CPPUNIT_ASSERT_EQUAL(ProofDisproof::NoCheckmate(), record3->proofDisproof());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
