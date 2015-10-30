#include "osl/ntesuki/ntesukiTable.h"
#include "osl/hash/hashKey.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_action/store.h"
#include "osl/apply_move/applyMove.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/safeMove.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace osl;
using namespace osl::ntesuki;
extern int isShortTest;

class NtesukiTableTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(NtesukiTableTest);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testAllocate);
  CPPUNIT_TEST(testFind);
  CPPUNIT_TEST(testAllocateWithMove);
  CPPUNIT_TEST(testErase);
  CPPUNIT_TEST(testGC);
#if 0
  CPPUNIT_TEST(testPass);
#endif
  CPPUNIT_TEST_SUITE_END();
public:
  void testCreation();
  void testSize();
  void testAllocate();
  void testFind();
  void testAllocateWithMove();
  void testErase();
  void testGC();
#if 0
  void testPass();
#endif
};

void NtesukiTableTest::testCreation()
{
  NtesukiTable table(1, 0, false);
}

void NtesukiTableTest::testSize()
{
  NtesukiTable table(1,0, false);
  BOOST_CHECK_EQUAL(0u, table.size());
}

void NtesukiTableTest::testAllocate()
{
  NtesukiTable table(100,0, false);
  HashKey k;
  BOOST_CHECK_EQUAL(0u, table.size());

  NtesukiRecord *record = table.allocateRoot(k, PieceStand(), 0);
  BOOST_CHECK(record);
  BOOST_CHECK_EQUAL(1u, table.size());
}

void NtesukiTableTest::testFind()
{
  NtesukiTable table(100,0, false);
  const HashKey k;
  BOOST_CHECK_EQUAL(0u, table.size());

  NtesukiRecord *record = table.allocateRoot(k, PieceStand(), 0);
  BOOST_CHECK(record);

  NtesukiRecord *newrecord = table.find(k);
  BOOST_CHECK(newrecord);
  BOOST_CHECK_EQUAL(record, newrecord);

  const HashKey kb = k.newHashWithMove(Move::PASS(BLACK));
  NtesukiRecord *brecord = table.allocateRoot(kb, PieceStand(), 0);
  BOOST_CHECK(brecord);
  BOOST_CHECK(record != brecord);
  BOOST_CHECK(newrecord != brecord);

  NtesukiRecord *newrecord2 = table.find(k);
  BOOST_CHECK_EQUAL(newrecord, newrecord2);
#if 0
  newrecord = table.find(k);
  BOOST_CHECK(!newrecord);
#endif
}

void NtesukiTableTest::testAllocateWithMove()
{
  NtesukiTable table(100,0, false);
  const HashKey k;

  BOOST_CHECK_EQUAL(0u, table.size());
  NtesukiRecord *record = table.allocateRoot(k, PieceStand(), 0);
  BOOST_CHECK(record);
  BOOST_CHECK_EQUAL(1u, table.size());

  NtesukiMove move(Move(Square(7, 7), Square(7, 6),
			PAWN, PTYPE_EMPTY,
			false, BLACK));
  NtesukiRecord *record_move = table.allocateWithMove(record, move);
  BOOST_CHECK(record_move);
  BOOST_CHECK_EQUAL(record_move, table.findWithMove(record, move));
  BOOST_CHECK_EQUAL(2u, table.size());
}

void NtesukiTableTest::testErase()
{
  NtesukiTable table(10,0, false);
  const HashKey k;

  for (int i = 0; i < 100; ++i)
  {
    BOOST_CHECK_EQUAL(0u, table.size());
    NtesukiRecord *record = table.allocateRoot(k, PieceStand(), 0);
    BOOST_CHECK(record);
    BOOST_CHECK_EQUAL(1u, table.size());

    table.erase(k);
    BOOST_CHECK_EQUAL(0u, table.size());

    record = table.find(k);
    BOOST_CHECK_EQUAL((NtesukiRecord *)NULL, record);
  }
}

void generate_valid_moves(const NumEffectState& state,
			  MoveVector& moves)
{
  if (state.inCheck(state.turn()))
  {
    GenerateEscapeKing::generate(state, moves);
    return;
  }
  GenerateAllMoves::generate(state.turn(), state, moves);

}
void NtesukiTableTest::testGC()
{
  NtesukiTable table(100, 10, !isShortTest);
  NtesukiTable::Table::largeGCCount = 1000;
  SimpleState sstate(HIRATE);
  NumEffectState state(sstate);
  HashKey key;

  unsigned int correct_size = 0;
  for (int i = 0; i < 1000; ++i)
  {
    BOOST_CHECK_EQUAL(correct_size, table.size());
    MoveVector moves;
    generate_valid_moves(state, moves);
    unsigned int j;
    HashKey new_key;
    for (j = 0; j < moves.size(); j++)
    {
      using namespace osl::move_classifier;
      if (!PlayerMoveAdaptor<SafeMove>::isMember(state, moves[j]))
	continue;
      new_key = key.newHashWithMove(moves[j]);
      if(!table.find(new_key))
	break;
    }
    if (j == moves.size())
    { 
     return;
    }
      
    key = new_key;
    ApplyMoveOfTurn::doMove(state, moves[j]);

    NtesukiRecord *record = table.allocateRoot(key, PieceStand(), 0);

    BOOST_CHECK(record);
    record->addChildCount(i * 4);//Must be multiple of 4
    ++correct_size;
    if (correct_size > 100) correct_size = 11;
    BOOST_CHECK_EQUAL(correct_size, table.size());
  }
}


#if 0
void NtesukiTableTest::testPass()
{
  NtesukiTable table(100, 0, false);
  const HashKey k;

  BOOST_CHECK_EQUAL(0u, table.size());
  NtesukiRecord *record = table.allocateRoot(k, PieceStand(), 0);
  BOOST_CHECK(record);
  BOOST_CHECK_EQUAL(1u, table.size());

  NtesukiRecord *record_pass = table.allocatePass<BLACK>(record, 0);
  BOOST_CHECK(record_pass);
  BOOST_CHECK_EQUAL(2u, table.size());
  BOOST_CHECK_EQUAL(record_pass, record->getPass());

  NtesukiRecord *record_pass_pass = table.allocatePass<WHITE>(record_pass, 0);
  BOOST_CHECK(record_pass_pass);
  BOOST_CHECK_EQUAL(2u, table.size());
  BOOST_CHECK_EQUAL(record, record_pass_pass);
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(NtesukiTableTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
