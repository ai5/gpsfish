#include "osl/oslConfig.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>

bool long_test = false;
bool init_unit_test()
{
  osl::OslConfig::setInUnitTest(long_test ? 2 : 1);
  osl::OslConfig::showOslHome();
  osl::OslConfig::setVerbose(false);
  osl::OslConfig::setUp();
  return true;
}

int main( int argc, char* argv[] )
{
  nice(20);
  if (argc >= 2 && argv[1] == std::string("--long"))
    long_test = true;
  return boost::unit_test::unit_test_main( init_unit_test, argc, argv );
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

