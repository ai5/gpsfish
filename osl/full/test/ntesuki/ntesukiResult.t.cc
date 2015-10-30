#include "osl/ntesuki/ntesukiResult.h"

#include <boost/test/unit_test.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <sstream>

using namespace osl;
using namespace osl::ntesuki;

class NtesukiResultTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE(NtesukiResultTest);
  CPPUNIT_TEST(testGenerate);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testGenerate()
  {
    NtesukiResult nr;
    nr = ProofDisproof::NoCheckmate();
    nr = ProofDisproof::Checkmate();
    nr = ProofDisproof(1, 1);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NtesukiResultTest);

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
