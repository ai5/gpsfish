/* characterEncodingConvertWin32.h
 */
#ifndef OSL_CHARACTER_ENCODING_CONVERT_WIN32_H
#define OSL_CHARACTER_ENCODING_CONVERT_WIN32_H

#ifdef _WIN32
#include <string>

namespace osl
{
  namespace misc
  {
    std::string eucToLang(const std::string& src);
  }
  using misc::eucToLang;
}
#endif

#endif /* OSL_CHARACTER_ENCODING_CONVERT_WIN32_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
