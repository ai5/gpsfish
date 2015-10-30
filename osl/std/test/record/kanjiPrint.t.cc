#include "osl/record/kanjiPrint.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::record;

BOOST_AUTO_TEST_CASE(KanjiPrintTestShow){
  SimpleState state=CsaString(
"P1+NY+TO *  *  *  * -OU-KE-KY\n"
"P2 *  *  *  *  * -GI-KI *  *\n"
"P3 * +RY *  * +UM * -KI-FU-FU\n"
"P4 *  * +FU-FU *  *  *  *  *\n"
"P5 *  * -KE * +FU *  * +FU *\n"
"P6-KE *  * +FU+GI-FU *  * +FU\n"
"P7 *  * -UM *  *  *  *  *  *\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 * +OU * -GI *  *  *  * -NG\n"
"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
"P-00KI00KY00FU00FU\n"
"P-00AL\n"
"+\n").initialState();

  KanjiPrint printer(std::cerr);
  if (OslConfig::verbose())
    printer.print(state);
  SimpleState state1(HIRATE);
  if (OslConfig::verbose())
    printer.print(state1);
}

BOOST_AUTO_TEST_CASE(KanjiPrintTestShowVKanji){
  SimpleState state=CsaString(
"P1+NY+TO *  *  *  * -OU-KE-KY\n"
"P2 *  *  *  *  * -GI-KI *  *\n"
"P3 * +RY *  * +UM * -KI-FU-FU\n"
"P4 *  * +FU-FU *  *  *  *  *\n"
"P5 *  * -KE * +FU *  * +FU *\n"
"P6-KE *  * +FU+GI-FU *  * +FU\n"
"P7 *  * -UM *  *  *  *  *  *\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 * +OU * -GI *  *  *  * -NG\n"
"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
"P-00KI00KY00FU00FU\n"
"P-00AL\n"
"+\n").initialState();

  KanjiPrint printer(std::cerr, 
                     std::shared_ptr<KIFCharacters>(new KIFCharacters()));
  if (OslConfig::verbose())
    printer.print(state);
  SimpleState state1(HIRATE);
  if (OslConfig::verbose())
    printer.print(state1);
}

BOOST_AUTO_TEST_CASE(KanjiPrintTestShowVKanjiColor){
  SimpleState state=CsaString(
"P1+NY+TO *  *  *  * -OU-KE-KY\n"
"P2 *  *  *  *  * -GI-KI *  *\n"
"P3 * +RY *  * +UM * -KI-FU-FU\n"
"P4 *  * +FU-FU *  *  *  *  *\n"
"P5 *  * -KE * +FU *  * +FU *\n"
"P6-KE *  * +FU+GI-FU *  * +FU\n"
"P7 *  * -UM *  *  *  *  *  *\n"
"P8 *  *  *  *  *  *  *  *  * \n"
"P9 * +OU * -GI *  *  *  * -NG\n"
"P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
"P-00KI00KY00FU00FU\n"
"P-00AL\n"
"+\n").initialState();

  KanjiPrint printer(std::cerr, 
                     std::shared_ptr<KIFCharacters>(new KIFCharacters()));
  printer.setBlackColor(Color::Yellow);
  printer.setWhiteColor(Color::Blue);
  printer.setLastMoveColor(Color::Red);
  if (OslConfig::verbose())
    printer.print(state);
  SimpleState state1(HIRATE);
  if (OslConfig::verbose())
    printer.print(state1);
}

BOOST_AUTO_TEST_CASE(KanjiPrintTestColor){
    BOOST_CHECK_EQUAL(Color::NONE, Color::colorFor("None"));
    BOOST_CHECK_EQUAL(Color::Black, Color::colorFor("Black"));
    BOOST_CHECK_EQUAL(Color::Red, Color::colorFor("Red"));
    BOOST_CHECK_EQUAL(Color::Green, Color::colorFor("Green"));
    BOOST_CHECK_EQUAL(Color::Brown, Color::colorFor("Brown"));
    BOOST_CHECK_EQUAL(Color::Blue, Color::colorFor("Blue"));
    BOOST_CHECK_EQUAL(Color::Purple, Color::colorFor("Purple"));
    BOOST_CHECK_EQUAL(Color::Cyan, Color::colorFor("Cyan"));
    BOOST_CHECK_EQUAL(Color::LightGray, Color::colorFor("LightGray"));
    BOOST_CHECK_EQUAL(Color::DarkGray, Color::colorFor("DarkGray"));
    BOOST_CHECK_EQUAL(Color::LightRed, Color::colorFor("LightRed"));
    BOOST_CHECK_EQUAL(Color::LightGreen, Color::colorFor("LightGreen"));
    BOOST_CHECK_EQUAL(Color::Yellow, Color::colorFor("Yellow"));
    BOOST_CHECK_EQUAL(Color::LightBlue, Color::colorFor("LIghtBlue"));
    BOOST_CHECK_EQUAL(Color::LightPurple, Color::colorFor("LightPurple"));
    BOOST_CHECK_EQUAL(Color::LightCyan, Color::colorFor("LightCyan"));
    BOOST_CHECK_EQUAL(Color::White, Color::colorFor("White"));

    BOOST_CHECK_EQUAL(Color::Blue, Color::colorFor("BLUE"));
    BOOST_CHECK_EQUAL(Color::Blue, Color::colorFor("blue"));
}

BOOST_AUTO_TEST_CASE(KanjiPrintTestKanjiNumber)
{
  for (int i=1; i<=10; ++i) {
    const std::string str = kanjiNumber(i);
    BOOST_CHECK_EQUAL(2, (int)str.size());
  }
  for (int i=11; i<=18; ++i) {
    const std::string str = kanjiNumber(i);
    BOOST_CHECK_EQUAL(4, (int)str.size());
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
