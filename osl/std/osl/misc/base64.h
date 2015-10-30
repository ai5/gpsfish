#ifndef OSL_MISC_BASE64ENCODE_H
#define OSL_MISC_BASE64ENCODE_H

#include <string>
#include <boost/dynamic_bitset.hpp>

namespace osl
{
  namespace book
  {
    class CompactBoard;
  }
  namespace misc
  {
    // http://en.wikipedia.org/wiki/Base64
    // http://ja.wikipedia.org/wiki/Base64
    std::string base64Encode(boost::dynamic_bitset<> src);

    boost::dynamic_bitset<> base64Decode(std::string src);

    std::string toBase64(const book::CompactBoard&);
    book::CompactBoard toCompactBoard(const std::string& str);

  } // namespace misc

} // namespace CQ

#endif /* _MISC_UUENCODE_H */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
