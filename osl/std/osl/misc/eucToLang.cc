#include "osl/misc/eucToLang.h"

#ifdef _WIN32
#include <windows.h>
#include <cassert>
#define CP_EUCJP 20932
//#define CP_EUCJP 51932 not supported by MultiByteToWideChar
#define CP_SJIS  932
#else
#include "osl/misc/iconvConvert.h"
#endif

std::string osl::misc::
eucToLang(const std::string& src) {
#ifdef _WIN32
  const int wlen = MultiByteToWideChar(CP_EUCJP, 0,
                                       src.c_str(), src.size(),
                                       NULL, 0);
  assert(wlen>0);
  wchar_t wbuf[wlen];
  const int wret = MultiByteToWideChar(CP_EUCJP, 0,
                                       src.c_str(), src.size(),
                                       wbuf, wlen);
  if (!wret || wlen != wret) {
    return "";
  }

  const int len = WideCharToMultiByte(CP_SJIS, 0,
                                      wbuf, wret,
                                      NULL, 0,
                                      NULL, NULL);
  assert(len>0);
  char buf[len];
  const int ret = WideCharToMultiByte(CP_SJIS, 0,
                                      wbuf, wret,
                                      buf, len,
                                      NULL, NULL);
  if (!ret || len != ret) {
    return "";
  }

  return std::string(buf, ret);
#else
  return IconvConvert::eucToLang(src);
#endif
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
