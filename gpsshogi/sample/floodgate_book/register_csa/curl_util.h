#ifndef _GPS_CURL_UTIL_H
#define _GPS_CURL_UTIL_H

#include <curl/curl.h>
#include <functional>
#include <memory>
#include <string>

struct CurlDeleter : public std::unary_function<CURL*, void>
{
  void operator()(CURL *p) const {
    if (p) curl_easy_cleanup(p);
  }
};

struct CurlCharDeleter : public std::unary_function<char*, void>
{
  void operator()(char *p) const {
    if (p) curl_free(p);
  }
};

typedef std::unique_ptr<CURL, CurlDeleter> CurlPtr;
typedef std::unique_ptr<char, CurlCharDeleter> CurlCharPtr;

std::string urlEncode(const std::string& str);

#endif /* _GPS_CURL_UTIL_H */
