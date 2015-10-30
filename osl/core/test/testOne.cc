#include "osl/oslConfig.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>

bool init_unit_test()
{
  osl::OslConfig::setInUnitTest(2);
  osl::OslConfig::showOslHome();
  osl::OslConfig::setUp();
  osl::OslConfig::setVerbose(true);
  return true;
}

int main( int argc, char* argv[] )
{
    return boost::unit_test::unit_test_main( init_unit_test, argc, argv );
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
