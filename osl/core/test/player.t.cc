#include "osl/basic_type.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
using namespace osl;

BOOST_AUTO_TEST_CASE(PlayerTestAlt){
  BOOST_CHECK( alt(BLACK)==WHITE );
  BOOST_CHECK( alt(WHITE)==BLACK );
}

BOOST_AUTO_TEST_CASE(PlayerTestShow){
  BOOST_CHECK_EQUAL(boost::lexical_cast<std::string>(BLACK), "+");
  BOOST_CHECK_EQUAL(boost::lexical_cast<std::string>(WHITE), "-");
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
