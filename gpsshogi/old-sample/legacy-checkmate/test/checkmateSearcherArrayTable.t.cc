/* checkmateSearcherArrayTable.t.cc
 */
#include "checkmateSearcherTest.h"
#include "osl/state/hashEffectState.h"
#include "osl/checkmate/arrayCheckHashTable.h"

typedef HashEffectState state_t;

typedef CheckmateSearcher<ArrayCheckHashTable> searcher_t;
/**
 * ArrayCheckHashTable を使うテスト
 */
struct CheckmateSearcherTestArray
  : public CheckmateSearcherTest<searcher_t>,
    public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(CheckmateSearcherTestArray);
  CPPUNIT_TEST(testNoCheck);
  CPPUNIT_TEST(testPawnCheckmate);
  CPPUNIT_TEST(testLimit);
  CPPUNIT_TEST(test30c);
  CPPUNIT_TEST(testFilesCheck);
  CPPUNIT_TEST(testFilesNoCheck);
  CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_REGISTRATION(CheckmateSearcherTestArray);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
