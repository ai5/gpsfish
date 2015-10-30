/* iconvConvert.h
 */
#ifndef OSL_ICONVCONVERT_H
#define OSL_ICONVCONVERT_H

#include <string>

namespace osl
{
  namespace misc
  {
    // Since IconvConvert uses iconv, this does not work on Windows.
    // If you just want to use eucToLang, use osl/misc/eucToLang.h instead.
    struct IconvConvert
    {
      static std::string eucToLang(const std::string& src);
      static std::string convert(const std::string& fromcode,
				 const std::string& tocode,
				 const std::string& src);

      static std::string langToIconvCode(const std::string& lang);
    private:
      struct IconvCD;
      static std::string convert(IconvCD& cd,
				 const std::string& src);
    };    
  }
  using misc::IconvConvert;
}

#endif /* OSL_ICONVCONVERT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
