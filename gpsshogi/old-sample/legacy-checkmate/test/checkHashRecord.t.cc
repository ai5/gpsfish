/* checkHashRecordTest.cc
 */
#include "osl/checkmate/checkHashRecord.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

class checkHashRecordTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(checkHashRecordTest);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testTwins);
  CPPUNIT_TEST(testFindDagOrigin);
  CPPUNIT_TEST_SUITE_END();
public:
  void testTwins();
  void testSize();
  void testFindDagOrigin();
};
CPPUNIT_TEST_SUITE_REGISTRATION(checkHashRecordTest);

using namespace osl;
using namespace osl::checkmate;

void checkHashRecordTest::testTwins()
{
  CheckHashRecord record;
  PathEncoding pathBefore(BLACK);
  CheckMove move(Move(Square(7,3),GOLD,BLACK), &record);
  PathEncoding pathAfter = pathBefore;
  pathAfter.pushMove(move.move);

  CPPUNIT_ASSERT(! record.findLoopInList(pathBefore));
  CPPUNIT_ASSERT(! record.findLoopInList(pathAfter));
  CPPUNIT_ASSERT(! move.findLoopInList(pathBefore));
  // CPPUNIT_ASSERT(! move.findLoopInList(pathAfter)); // 手番エラー

  record.setLoopDetection(pathAfter, 0);
  // CPPUNIT_ASSERT(! record.findLoopInList(pathBefore));
  CPPUNIT_ASSERT(record.findLoopInList(pathAfter));
  CPPUNIT_ASSERT(move.findLoopInList(pathBefore));
  // CPPUNIT_ASSERT(! move.findLoopInList(pathAfter));
}

extern bool isShortTest;
void checkHashRecordTest::testSize()
{
  if (! isShortTest)
    std::cerr << sizeof(CheckHashRecord) <<"\n";
  CPPUNIT_ASSERT(sizeof(CheckHashRecord) <= 128u);
}

void checkHashRecordTest::testFindDagOrigin()
{
#if 0
  // 2003-12-07 現在は使っていないがテストケースはとっておく
  {
    // 相手が 0 なら 0
    CheckHashRecord record;
    CPPUNIT_ASSERT_EQUAL((CheckHashRecord*)0, record.findDagOrigin(0,true));
  }
  {
    //   a    
    //  / \   これは対策不要
    // b   c  
    //  \ /   
    //   d    
    CheckHashRecord a, b, c, d;
    a.moves.push_back(CheckMove(Move::INVALID(), &b));
    a.moves.push_back(CheckMove(Move::INVALID(), &c));
    b.moves.push_back(CheckMove(Move::INVALID(), &d));
    b.parent = &a;
    c.moves.push_back(CheckMove(Move::INVALID(), &d));
    c.parent = &a;
    CPPUNIT_ASSERT_EQUAL((CheckHashRecord*)0, b.findDagOrigin(&c,true));
  }
  {
    //   a    
    //  / \   対策必要
    // b   c  
    // |   |
    // d   e
    //  \ /   
    //   f  
    CheckHashRecord a, b, c, d, e, f;
    a.moves.push_back(CheckMove(Move::INVALID(), &b));
    a.moves.push_back(CheckMove(Move::INVALID(), &c));
    b.moves.push_back(CheckMove(Move::INVALID(), &d));
    b.parent = &a;
    c.moves.push_back(CheckMove(Move::INVALID(), &e));
    c.parent = &a;
    d.parent = &b;
    e.parent = &c;
    d.setProofDisproof(b.proofDisproof());
    e.setProofDisproof(c.proofDisproof());
    CPPUNIT_ASSERT_EQUAL(&a, d.findDagOrigin(&e,true));
    CPPUNIT_ASSERT_EQUAL(&a, d.findDagOrigin(&e,false));
    CPPUNIT_ASSERT_EQUAL((CheckHashRecord*)0, a.findDagOrigin(0,true));

    b.setProofDisproof(PawnCheckmate);
    CPPUNIT_ASSERT_EQUAL((CheckHashRecord*)0, d.findDagOrigin(&e,true));
    CPPUNIT_ASSERT_EQUAL((CheckHashRecord*)0, d.findDagOrigin(&e,false));
  }
#endif
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
