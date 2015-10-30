/* dominanceTable.t.cc
 */
#include "dominanceTableTestCommons.h"
#include "osl/checkmate/dominanceTable.h"

class DominanceTableTest
  : public DominanceTableTestCommons<DominanceTable>,
    public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(DominanceTableTest);
  CPPUNIT_TEST(testAllocateBlack);
  CPPUNIT_TEST(testAllocateWhite);
  CPPUNIT_TEST_SUITE_END();
public:
};
CPPUNIT_TEST_SUITE_REGISTRATION(DominanceTableTest);

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
