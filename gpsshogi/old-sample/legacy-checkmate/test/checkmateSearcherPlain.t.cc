/* checkmateSearcherPlain.t.cc
 */
#include "checkmateSearcherTest.h"
#include "osl/state/hashEffectState.h"
#include "osl/checkmate/simpleCheckHashTable.h"
#include "osl/checkmate/nullEstimator.h"
#include "osl/checkmate/nullCost.h"

typedef HashEffectState state_t;
typedef CheckmateSearcher<SimpleCheckHashTable,NullEstimator,NullCost> 
searcher_t;

/**
 * plain df-pn のテスト
 */
struct CheckmateSearcherTestPlain
  : public CheckmateSearcherTest<searcher_t>,
    public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(CheckmateSearcherTestPlain);
  CPPUNIT_TEST(testNoCheck);
  CPPUNIT_TEST(test30c);
  CPPUNIT_TEST(testFilesCheck);
  CPPUNIT_TEST(testFilesNoCheck);
  CPPUNIT_TEST(testPawnCheckmate);
  CPPUNIT_TEST(testLimit);
  CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_REGISTRATION(CheckmateSearcherTestPlain);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
