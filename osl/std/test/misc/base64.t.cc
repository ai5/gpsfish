#include "osl/misc/base64.h"
#include "osl/numEffectState.h"
#include "osl/book/compactBoard.h"

#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::book;

BOOST_AUTO_TEST_CASE(Base64TestEncodeA) {
  boost::dynamic_bitset<> orig(8); // A
  orig[0] = 1;
  orig[1] = 0;
  orig[2] = 0;
  orig[3] = 0;
  orig[4] = 0;
  orig[5] = 0;
  orig[6] = 1;
  orig[7] = 0;
  const std::string base64 = "QQ==";

  BOOST_CHECK_EQUAL(base64, osl::misc::base64Encode(orig));
  BOOST_CHECK_EQUAL(orig,   osl::misc::base64Decode(base64));
}

BOOST_AUTO_TEST_CASE(Base64TestEncodea) {
  boost::dynamic_bitset<> orig(8); // a
  orig[0] = 1;
  orig[1] = 1;
  orig[2] = 1;
  orig[3] = 0;
  orig[4] = 1;
  orig[5] = 0;
  orig[6] = 0;
  orig[7] = 1;
  const std::string base64 = "lw==";

  BOOST_CHECK_EQUAL(base64, osl::misc::base64Encode(orig));
  BOOST_CHECK_EQUAL(orig,   osl::misc::base64Decode(base64));
}

BOOST_AUTO_TEST_CASE(Base64TestEncode0) {
  boost::dynamic_bitset<> orig(8); // 0
  orig[0] = 0;
  orig[1] = 0;
  orig[2] = 1;
  orig[3] = 0;
  orig[4] = 0;
  orig[5] = 0;
  orig[6] = 0;
  orig[7] = 1;
  const std::string base64 = "hA==";

  BOOST_CHECK_EQUAL(base64, osl::misc::base64Encode(orig));
  BOOST_CHECK_EQUAL(orig,   osl::misc::base64Decode(base64));
}

BOOST_AUTO_TEST_CASE(Base64TestEncode62) {
  boost::dynamic_bitset<> orig(8); // -
  orig[0] = 1;
  orig[1] = 0;
  orig[2] = 1;
  orig[3] = 0;
  orig[4] = 0;
  orig[5] = 0;
  orig[6] = 1;
  orig[7] = 0;
  const std::string base64 = "RQ==";

  BOOST_CHECK_EQUAL(base64, osl::misc::base64Encode(orig));
  BOOST_CHECK_EQUAL(orig,   osl::misc::base64Decode(base64));
}

BOOST_AUTO_TEST_CASE(Base64TestEncode63) {
  boost::dynamic_bitset<> orig(8); // _
  orig[0] = 1;
  orig[1] = 0;
  orig[2] = 1;
  orig[3] = 0;
  orig[4] = 1;
  orig[5] = 0;
  orig[6] = 0;
  orig[7] = 1;
  const std::string base64 = "lQ==";

  BOOST_CHECK_EQUAL(base64, osl::misc::base64Encode(orig));
  BOOST_CHECK_EQUAL(orig,   osl::misc::base64Decode(base64));
}

BOOST_AUTO_TEST_CASE(Base64TestEncodeQUJDREVGRw) {
  boost::dynamic_bitset<> bits(7*8); // ABCDEFG
  bits = boost::dynamic_bitset<>(7*8, 0x41ul) << 6*8 |
         boost::dynamic_bitset<>(7*8, 0x42ul) << 5*8 |
         boost::dynamic_bitset<>(7*8, 0x43ul) << 4*8 |
         boost::dynamic_bitset<>(7*8, 0x44ul) << 3*8 |
         boost::dynamic_bitset<>(7*8, 0x45ul) << 2*8 |
         boost::dynamic_bitset<>(7*8, 0x46ul) << 1*8 |
         boost::dynamic_bitset<>(7*8, 0x47ul);

  const std::string ret = osl::misc::base64Encode(bits);
  BOOST_CHECK_EQUAL(std::string("QUJDREVGRw=="), ret);
}

BOOST_AUTO_TEST_CASE(Base64TestEncodeleasure) {
  std::string src = "bGVhc3VyZS4=";
  const boost::dynamic_bitset<> ret = osl::misc::base64Decode(src);
  const std::string src2 = osl::misc::base64Encode(ret);
  BOOST_CHECK_EQUAL(src, src2);
}

BOOST_AUTO_TEST_CASE(CompactBoardTestBase64Encode){
  const SimpleState state(HIRATE);
  const CompactBoard cb(state);
  const std::string ret = misc::toBase64(cb);
  const std::string answer = "__sAEf_6ABMACgAXAAsAGf_8ACH__gAi__oAIwAKACcADwAoAAwAKf_9ADH_-gAzAAoANwANADn_-QBB__oAQwAKAEcACQBJ__gAUf_6AFMACgBXAAgAWf_5AGH_-gBjAAoAZwAJAGn__QBx__oAcwAKAHcADQB5__wAgf__AIL_-gCDAAoAhwAOAIgADACJ__sAkf_6AJMACgCXAAsAmQAAAAA=";
  BOOST_CHECK_EQUAL(answer, ret);
}

BOOST_AUTO_TEST_CASE(CompactBoardTestBase64Decode){
  const std::string str = "__sAEf_6ABMACgAXAAsAGf_8ACH__gAi__oAIwAKACcADwAoAAwAKf_9ADH_-gAzAAoANwANADn_-QBB__oAQwAKAEcACQBJ__gAUf_6AFMACgBXAAgAWf_5AGH_-gBjAAoAZwAJAGn__QBx__oAcwAKAHcADQB5__wAgf__AIL_-gCDAAoAhwAOAIgADACJ__sAkf_6AJMACgCXAAsAmQAAAAA=";
  const CompactBoard cb = misc::toCompactBoard(str);

  const SimpleState answer(HIRATE);
  const SimpleState state = cb.state();
  BOOST_CHECK_EQUAL(answer, state);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
