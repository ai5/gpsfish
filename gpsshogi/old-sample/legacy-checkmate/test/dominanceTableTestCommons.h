/* dominanceTableTestCommons.h
 */
#ifndef _DOMINANCETABLETESTCOMMONS_H
#define _DOMINANCETABLETESTCOMMONS_H

#include "osl/checkmate/checkHashRecord.h"
#include "osl/state/hashEffectState.h"
#include "osl/record/csaString.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl;
using namespace osl::checkmate;

template <class Table>
struct DominanceTableTestCommons
{
  void testAllocateBlack();
  void testAllocateWhite();
};

/** 先手番 */
template <class Table>
void DominanceTableTestCommons<Table>::testAllocateBlack()
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
  Table table(BLACK);
  const PathEncoding path_black(BLACK);
  const PieceStand white_stand(WHITE, state);
  CheckHashRecord *record = table.allocate(state.getHash(), white_stand,
					   path_black);
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT_EQUAL(1u, record->proofDisproof().proof());
  CPPUNIT_ASSERT_EQUAL(1u, record->proofDisproof().disproof());
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
			   "+\n").getInitialState());
  const PieceStand white_stand2(WHITE, state2);
  CheckHashRecord *record2 = table.allocate(state2.getHash(), white_stand2,
					    path_black);
  CPPUNIT_ASSERT(record2);
  CPPUNIT_ASSERT(record2 != record);
  CPPUNIT_ASSERT_EQUAL(3u, record2->proofDisproof().proof());
  CPPUNIT_ASSERT_EQUAL(1u, record2->proofDisproof().disproof());

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
  CheckHashRecord *record3 = table.allocate(state3.getHash(), white_stand3,
					    path_black);
  CPPUNIT_ASSERT(record3);
  CPPUNIT_ASSERT(record3 != record);
  CPPUNIT_ASSERT_EQUAL(1u, record3->proofDisproof().proof());
  CPPUNIT_ASSERT_EQUAL(5u, record3->proofDisproof().disproof());
}

/** 後手番 */
template <class Table>
void DominanceTableTestCommons<Table>::testAllocateWhite()
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
  const PathEncoding path_white(WHITE);
  const PieceStand white_stand(WHITE, state);
  Table table(WHITE);
  CheckHashRecord *record = table.allocate(state.getHash(), white_stand,
					   path_white);
  CPPUNIT_ASSERT(record);
  CPPUNIT_ASSERT_EQUAL(1u, record->proofDisproof().proof());
  CPPUNIT_ASSERT_EQUAL(1u, record->proofDisproof().disproof());
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
  CheckHashRecord *record2 = table.allocate(state2.getHash(), white_stand2,
					    path_white);
  CPPUNIT_ASSERT(record2);
  CPPUNIT_ASSERT(record2 != record);
  CPPUNIT_ASSERT_EQUAL(1u, record2->proofDisproof().proof());
  CPPUNIT_ASSERT_EQUAL(5u, record2->proofDisproof().disproof());

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
  CheckHashRecord *record3 = table.allocate(state3.getHash(), white_stand3,
					    path_white);
  CPPUNIT_ASSERT(record3);
  CPPUNIT_ASSERT(record3 != record);
  CPPUNIT_ASSERT_EQUAL(3u, record3->proofDisproof().proof());
  CPPUNIT_ASSERT_EQUAL(1u, record3->proofDisproof().disproof());
}

#endif /* _DOMINANCETABLETESTCOMMONS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
