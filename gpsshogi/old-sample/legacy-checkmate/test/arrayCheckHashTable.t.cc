/* arrayCheckHashTableTest.cc
 */
#include "dominanceTableTestCommons.h"
#include "osl/checkmate/arrayCheckHashTable.h"

class arrayCheckHashTableTest 
  : public DominanceTableTestCommons<ArrayCheckHashTable>,
    public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(arrayCheckHashTableTest);
  CPPUNIT_TEST(testAllocateBlack);
  CPPUNIT_TEST(testAllocateWhite);
  CPPUNIT_TEST_SUITE_END();
public:
};
CPPUNIT_TEST_SUITE_REGISTRATION(arrayCheckHashTableTest);

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
