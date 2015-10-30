/* iconvConvert.cc
 */
#include "osl/misc/iconvConvert.h"
#include <cstdlib>
#include <cstring>
#ifndef _WIN32
#include <iconv.h>

struct osl::misc::IconvConvert::IconvCD
{
  iconv_t cd;
  IconvCD(const std::string& fromcode, const std::string& tocode)
  {
    cd = iconv_open(tocode.c_str(), fromcode.c_str());
  }
  ~IconvCD()
  {
    iconv_close(cd);
  }
};


std::string osl::misc::
IconvConvert::langToIconvCode(const std::string& lang)
{
  if (lang.empty())
    return "";
  const bool euc_jp =
    (lang.find("jp") != lang.npos || lang.find("JP") != lang.npos)
    && (lang.find("euc") != lang.npos || lang.find("EUC") != lang.npos);
  if (euc_jp)
    return "EUC-JP";
  const bool shift_jis =
    (lang.find("sjis") != lang.npos || lang.find("SJIS") != lang.npos);
  if (shift_jis)
    return "SHIFT_JIS";
  const bool utf8 =
    (lang.find("UTF-8") != lang.npos || lang.find("UTF8") != lang.npos);
  if (utf8)
    return "UTF-8";
  return "";
}

std::string osl::misc::
IconvConvert::eucToLang(const std::string& src)
{
  static const char *lang = getenv("LANG");
  if (! lang)
    return "";
  static const std::string tocode = langToIconvCode(lang);
  if (tocode.empty())
    return "";
  if ("EUC-JP" == tocode)
    return src;
  IconvCD cd("EUC-JP", tocode);
  return convert(cd, src);
}

std::string osl::misc::
IconvConvert::convert(const std::string& fromcode,
		      const std::string& tocode,
		      const std::string& src)
{
  if (fromcode == tocode)
    return src;
  IconvCD cd(fromcode, tocode);
  return convert(cd, src);
}

std::string osl::misc::
IconvConvert::convert(IconvCD& cd, const std::string& src)
{
  const char * inbuf = src.c_str();
  char outbuf[1024], *outbufptr = outbuf;
  size_t inbytesleft = src.size(), outbytesleft = 1024;
#if (defined __linux__ || defined __APPLE__)
  typedef char ** iconv_inbuf_t;
#else
  typedef const char ** iconv_inbuf_t;
#endif
  std::string ret;
  int success;
  while ((success = iconv(cd.cd, (iconv_inbuf_t)&inbuf, &inbytesleft, 
			  &outbufptr, &outbytesleft)) >= 0
	 && inbytesleft > 0)
    if (outbufptr - outbuf >= 512)
    {
      ret += std::string(outbuf, outbufptr);
      outbufptr = outbuf;
    }
  if (success == -1)
    return "";
  ret += std::string(outbuf, outbufptr);
  return ret;
}

#endif /* _WIN32 */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
