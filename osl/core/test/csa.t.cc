#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <fstream>

using namespace osl;

BOOST_AUTO_TEST_CASE(CsaTestShow){
  NumEffectState s;
  std::vector<Move> ms;
  
  BOOST_CHECK_EQUAL(std::string(""),
                       csa::show(&*ms.begin(), &*ms.end()));

  ms.push_back(csa::strToMove("+7776FU", s));
  s.makeMove(ms.back());
  BOOST_CHECK_EQUAL(std::string("+7776FU"),
                       csa::show(&*ms.begin(), &*ms.end()));

  ms.push_back(csa::strToMove("-3334FU", s));
  s.makeMove(ms.back());

  BOOST_CHECK_EQUAL(std::string("+7776FU-3334FU"),
                       csa::show(&*ms.begin(), &*ms.end()));
}

BOOST_AUTO_TEST_CASE(CsaTestLoadKatagyoku){
  CsaString csa(
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  * +FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n");
  SimpleState sstate=csa.initialState();
  BOOST_CHECK_EQUAL(BLACK, sstate.kingPiece(BLACK).owner());
  BOOST_CHECK_EQUAL(Square::STAND(),
		       sstate.kingPiece(BLACK).square());
}

BOOST_AUTO_TEST_CASE(CsaTestLoadKatagyokuFile){
  const char* test_file_name = "osl_record_csa_testLoadKatagyokuFile.csa";
  std::ofstream os(test_file_name);
  os <<
    "P1 *  *  *  *  *  *  *  * -OU\n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 *  *  *  *  *  *  *  * +FU\n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  *  * \n"
    "P+00KI\n"
    "P-00AL\n"
    "+\n";
  os.close();

  osl::RecordMinimal myrec=CsaFileMinimal(test_file_name).load();
  auto mystate=myrec.initialState();
  BOOST_CHECK_EQUAL(BLACK, mystate.kingPiece(BLACK).owner());
  BOOST_CHECK_EQUAL(Square::STAND(),
		       mystate.kingPiece(BLACK).square());
  unlink(test_file_name);
}

BOOST_AUTO_TEST_CASE(CsaTestSpeed){
  auto record=CsaString(
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
"+\n").load();
  SimpleState state0=record.initialState();
  {
    const Square pos0=csa::strToPos("55"),pos1=csa::strToPos("54");
    const Move move=Move(pos0,pos1,
			    PAWN,PTYPE_EMPTY,false,BLACK);
    const SimpleState stateCopy(state0);
    BOOST_CHECK_EQUAL(stateCopy, state0);
  }
  {
    const Square pos1=csa::strToPos("99");
    const Move move=Move(pos1,PAWN,BLACK);
    const SimpleState stateCopy(state0);
    BOOST_CHECK_EQUAL(stateCopy, state0);
  }
  {
    const Square pos0=csa::strToPos("53"),pos1=csa::strToPos("42");
    const Move move=Move(pos0,pos1,
			     PBISHOP,SILVER,false,BLACK);
    const SimpleState stateCopy(state0);
    BOOST_CHECK_EQUAL(stateCopy, state0);
  }
}

BOOST_AUTO_TEST_CASE(CsaTestStr2Move)
{
  NumEffectState state;
  BOOST_CHECK_EQUAL(Move(), csa::strToMove("%TORYO", state));
  BOOST_CHECK_EQUAL(Move::DeclareWin(), csa::strToMove("%KACHI", state));
  BOOST_CHECK_EQUAL(Move::PASS(BLACK), csa::strToMove("%PASS", state));
}

BOOST_AUTO_TEST_CASE(CsaTestError)
{
  try
  {
    // missing P1
    const auto record(CsaString("P2 * -HI *  *  *  *  * -KA * \n"
				  "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n"
				  "P4 *  *  *  *  *  *  *  *  * \n"
				  "P5 *  *  *  *  *  *  *  *  * \n"
				  "P6 *  *  *  *  *  *  *  *  * \n"
				  "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n"
				  "P8 * +KA *  *  *  *  * +HI * \n"
				  "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
				  "+\n").load());
    BOOST_CHECK("failed to detect errors" == 0);
  }
  catch (CsaIOError& e) {
    BOOST_CHECK(e.what()
		== std::string("incomplete position description in csaParseLine"));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
