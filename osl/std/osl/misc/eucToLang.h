/* eucToLang.h
 */
#ifndef OSL_MISC_EUCTOLANG_H
#define OSL_MISC_EUCTOLANG_H

#include <string>

namespace osl
{
  namespace misc
  {
    // Converts character encoding from EUC-JP to a native encoding.
    // This works on both Linux and Windows.
    std::string eucToLang(const std::string& src);
  }
}

#endif /* OSL_MISC_EUCTOLANG_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
