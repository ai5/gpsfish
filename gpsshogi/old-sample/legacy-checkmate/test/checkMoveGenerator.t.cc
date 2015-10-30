/* checkMoveGenerator.t.cc
 */
#include "osl/checkmate/checkMoveGenerator.h"
#include "osl/checkmate/checkMoveList.h"
#include "osl/checkmate/checkMoveListProvider.h"
#include "osl/record/csaString.h"
#include "osl/state/hashEffectState.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

class CheckMoveGeneratorTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(CheckMoveGeneratorTest);
  CPPUNIT_TEST(testEscape);
  CPPUNIT_TEST(testAttack);
  CPPUNIT_TEST(testBug);
  CPPUNIT_TEST_SUITE_END();
public:
  void testEscape();
  void testAttack();
  void testBug();
};
CPPUNIT_TEST_SUITE_REGISTRATION(CheckMoveGeneratorTest);

using namespace osl;
using namespace osl::checkmate;
extern bool isShortTest;

void CheckMoveGeneratorTest::testBug()
{
  SimpleState state = CsaString(
    "P1-KY *  *  *  *  *  *  * +UM\n"
    "P2 * +KI *  * +HI *  *  * -KY\n"
    "P3 *  * -KE *  *  *  *  *  * \n"
    "P4-FU-OU *  * -KI *  * -FU-FU\n"
    "P5 * +KE-GI-FU *  *  * +KE * \n"
    "P6+FU+OU *  * +FU+FU *  * +FU\n"
    "P7 *  *  *  * +GI *  *  *  * \n"
    "P8 *  *  *  *  *  *  * +HI * \n"
    "P9+KY *  * -UM *  *  *  * +KY\n"
    "P+00FU00FU00FU00FU00FU00FU00FU00FU00KI\n"
    "P-00FU00FU00KE00GI00GI00KI\n"
    "+\n").getInitialState();
  HashEffectState hState(state);
  const Move last_move = Move(Square(7,5),SILVER,WHITE);
  CheckMoveList moves;

  CheckMoveListProvider src;
  CheckMoveGenerator<WHITE>::generateEscape(hState, src, moves);
  if (! isShortTest)
    std::cerr << moves;

  moves.clear();
  CheckMoveGenerator<BLACK>::generateAttack(hState, src, moves);
  if (! isShortTest)
    std::cerr << moves;
}

void CheckMoveGeneratorTest::testEscape()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-FU-KY\n"
    "P3 *  *  *  *  *  *  * +KE * \n"
    "P4 *  *  *  *  * +HI *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KE\n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState hState(state);
  CheckMoveList moves;
  CheckMoveListProvider src;
  CheckMoveGenerator<BLACK>::generateEscape(hState, src, moves);
  CPPUNIT_ASSERT_EQUAL((size_t)1u, moves.size());

  SimpleState state2=CsaString(
    "P1 *  *  *  *  *  *  * -KE-OU\n"
    "P2 *  *  *  *  *  * -FU-FU-KY\n"
    "P3 *  *  *  *  *  *  * +KE * \n"
    "P4 *  *  *  *  * +KA *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  * +OU *  *  *  *  *  * \n"
    "P+00KE\n"
    "P-00AL\n"
    "-\n").getInitialState();
  HashEffectState hState2(state2);
  moves.clear();
  CheckMoveGenerator<BLACK>::generateEscape(hState2, src, moves);
  CPPUNIT_ASSERT_EQUAL((size_t)0u, moves.size());


  SimpleState state3=CsaString(
    "P1-KY-KE * -KI *  *  * +RY-KY\n"
    "P2 * -GI-OU-FU *  *  *  *  * \n"
    "P3-FU-FU * -KI *  *  *  *  * \n"
    "P4 *  *  *  * +FU *  *  * -FU\n"
    "P5 *  *  * +FU *  *  *  *  * \n"
    "P6 *  * +FU *  *  * +GI-FU+FU\n"
    "P7+FU *  *  *  *  * +KI-KE * \n"
    "P8 *  *  *  *  * -RY+FU+OU * \n"
    "P9+KY+KE *  *  * -GI-KA+KE+KY\n"
    "P+00FU00FU00KI00KA\n"
    "P-00FU00FU00FU00FU00FU00GI\n"
    "+\n").getInitialState();
  HashEffectState hState3(state3);
  moves.clear();
  CheckMoveGenerator<WHITE>::generateEscape(hState3, src, moves);
  CPPUNIT_ASSERT_EQUAL((size_t)1u, moves.size());
}

void CheckMoveGeneratorTest::testAttack()
{
  SimpleState state=CsaString(
    "P1 *  *  *  *  *  *  *  *  * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  * -OU *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  * +FU *  *  * \n"
    "P7 *  *  *  * +OU *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState hState(state);
  CheckMoveList moves;
  CheckMoveListProvider src;
  CheckMoveGenerator<BLACK>::generateAttack(hState, src, moves);
  CPPUNIT_ASSERT_EQUAL((size_t)1u, moves.size());

  SimpleState state2=CsaString(
    "P1 *  *  *  *  *  *  *  *  * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  * -OU *  *  * \n"
    "P5 *  *  *  *  *  * -KA *  * \n"
    "P6 *  *  *  *  * +FU *  *  * \n"
    "P7 *  *  *  * +OU *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P-00AL\n"
    "+\n").getInitialState();
  HashEffectState hState2(state2);
  moves.clear();
  CheckMoveGenerator<BLACK>::generateAttack(hState2, src, moves);
  CPPUNIT_ASSERT_EQUAL((size_t)0u, moves.size());

  SimpleState state3=CsaString(
    "P1 *  *  *  *  *  *  *  *  * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4-OU *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8+OU *  *  *  * -RY *  *  * \n"
    "P9+KY+KE *  *  *  *  *  *  * \n"
    "P+00AL\n"
    "+\n").getInitialState();
  HashEffectState hState3(state3);
  moves.clear();
  CheckMoveGenerator<BLACK>::generateAttack(hState3, src, moves);
  CPPUNIT_ASSERT_EQUAL((size_t)2u, moves.size());
  const Move counter1 = Move(Square(9,8),Square(8,7),KING,
			       PTYPE_EMPTY,false,BLACK);
  const Move counter2 = Move(Square(5,8),BISHOP,BLACK);
  CPPUNIT_ASSERT_EQUAL(counter1, moves[0].move);
  CPPUNIT_ASSERT_EQUAL(counter2, moves[1].move);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
