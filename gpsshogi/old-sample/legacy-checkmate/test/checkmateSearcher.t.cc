/* checkmateSearcherTest.cc
 */
#include "checkmateSearcherTest.h"
#include "osl/checkmate/simpleCheckHashTable.h"
#include "osl/state/hashEffectState.h"
typedef osl::HashEffectState state_t;

/**
 * 標準的な設定でのテスト
 */
struct CheckmateSearcherTestStandard 
  : public CheckmateSearcherTest<CheckmateSearcher<SimpleCheckHashTable> >,
    public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(CheckmateSearcherTestStandard);
  CPPUNIT_TEST(testPawnCheckmate);
  CPPUNIT_TEST(testLimit);
  CPPUNIT_TEST(test30c);
  CPPUNIT_TEST(testFilesCheck);
  CPPUNIT_TEST(testFilesNoCheck);
  CPPUNIT_TEST(testNoCheck);
  CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_REGISTRATION(CheckmateSearcherTestStandard);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
