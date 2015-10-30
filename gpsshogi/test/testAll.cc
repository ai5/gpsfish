#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <iostream>
#include <unistd.h>

// No need to include header files of every test classes.
// Instead, place CPPUNIT_TEST_SUITE_REGISTRATION(fooTest); in each fooTest.cc

bool isShortTest=true;
int main(int /* argc */, char ** /* argv */)
{
  nice(20);
  
  CppUnit::TextUi::TestRunner runner;

  CppUnit::TestFactoryRegistry &registry 
    = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest(registry.makeTest());

  runner.run();
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

