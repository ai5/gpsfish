#include "osl/book/miniBoard.h"
#include "osl/numEffectState.h"
#include <boost/test/unit_test.hpp>
#include <string>


using namespace osl;

BOOST_AUTO_TEST_CASE(MiniBoardTestBase64Encode){
  const NumEffectState state;
  const book::MiniBoard mb(state);
  const std::string ret = mb.toBase64();
  const std::string answer = "RMF0jCdMw3UMR1TFdYxnXMd2DIdkyXREGWRJlIQpYEiUxDlcR5SIiAoYKgkmwmlZUQA=";
  BOOST_CHECK_EQUAL(answer, ret);
  book::MiniBoard mb2;
  const int result = fromBase64(answer, mb2);
  BOOST_CHECK_EQUAL(0, result);
  BOOST_CHECK_EQUAL(mb.getState(), mb2.getState());
  
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

