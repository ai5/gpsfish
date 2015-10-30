#include "curl_util.h"

std::string urlEncode(const std::string& str)
{
  CurlPtr curl(curl_easy_init());
  const CurlCharPtr encoded(curl_easy_escape(curl.get(), str.c_str(), str.size()));
  return std::string(encoded.get());
}

