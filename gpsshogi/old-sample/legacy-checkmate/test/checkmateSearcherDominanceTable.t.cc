/* checkmateSearcherDominanceTable.t.cc
 */
#include "checkmateSearcherTest.h"
#include "osl/state/hashEffectState.h"
#include "osl/checkmate/dominanceTable.h"

typedef HashEffectState state_t;

typedef CheckmateSearcher<DominanceTable> searcher_t;
/**
 * ArrayCheckHashTable を使うテスト
 */
struct CheckmateSearcherTestDominance
  : public CheckmateSearcherTest<searcher_t>,
    public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(CheckmateSearcherTestDominance);
  CPPUNIT_TEST(testPawnCheckmate);
  CPPUNIT_TEST(testFilesCheck);
  CPPUNIT_TEST(testFilesNoCheck);
  CPPUNIT_TEST(testNoCheck);
  CPPUNIT_TEST(testLimit);
  CPPUNIT_TEST(test30c);
  CPPUNIT_TEST_SUITE_END();
#if 0
  CheckmateSearcherTestDominance()
  {
    CheckmateRecorder::DepthTracer::maxVerboseLogDepth = 30;
  }
#endif
};
CPPUNIT_TEST_SUITE_REGISTRATION(CheckmateSearcherTestDominance);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
