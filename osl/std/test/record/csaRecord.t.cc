#include "osl/record/csaRecord.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
using namespace osl;

BOOST_AUTO_TEST_CASE(CsaTestSearchInfo)
{
  // XXX
  const int old_unit_test_level = OslConfig::inUnitTest();
  OslConfig::setInUnitTest(0);
  std::istringstream is(
    "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
    "P2 * -HI *  *  *  *  * -KA * \n"
    "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
    "P8 * +KA *  *  *  *  * +HI * \n"
    "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
    "+\n"
    "+7776FU\n"
    "'** 3\n"
    "-8384FU\n"
    "'** -5\n");
  const Record record = CsaFile(is).load();

  BOOST_CHECK_EQUAL((size_t)2, record.move_info.size());
  BOOST_CHECK_EQUAL( 3, record.move_info[0].value);
  BOOST_CHECK_EQUAL(-5, record.move_info[1].value);

  OslConfig::setInUnitTest(old_unit_test_level);
}

BOOST_AUTO_TEST_CASE(CsaTestResult)
{
  {
    std::istringstream is(
      "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
      "P2 * -HI *  *  *  *  * -KA * \n"
      "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
      "P4 *  *  *  *  *  *  *  *  * \n"
      "P5 *  *  *  *  *  *  *  *  * \n"
      "P6 *  *  *  *  *  *  *  *  * \n"
      "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
      "P8 * +KA *  *  *  *  * +HI * \n"
      "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
      "+\n"
      "+7776FU\n"
      "-8384FU\n");
    const Record record = CsaFile(is).load();
    BOOST_CHECK_EQUAL(Record::Unknown, record.result);
  }
  {
    std::istringstream is("P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			  "P2 * -HI *  *  *  *  * -KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  *  *  *  *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI * \n"
			  "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			  "+\n"
			  "+7776FU\n"
			  "%TORYO\n");
    const Record record = CsaFile(is).load();
    BOOST_CHECK_EQUAL(Record::BlackWin, record.result);
  }
  {
    std::istringstream is("P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			  "P2 * -HI *  *  *  *  * -KA * \n"
			  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
			  "P4 *  *  *  *  *  *  *  *  * \n"
			  "P5 *  *  *  *  *  *  *  *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
			  "P8 * +KA *  *  *  *  * +HI * \n"
			  "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			  "+\n"
			  "%TORYO\n");
    const Record record = CsaFile(is).load();
    BOOST_CHECK_EQUAL(Record::WhiteWin, record.result);
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
