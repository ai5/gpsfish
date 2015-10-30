#ifndef _REGIST_CSA_EXCEPTION_H
#define _REGIST_CSA_EXCEPTION_H

#include <boost/format.hpp>
#include <exception>
#include <string>

const static int ER_DUP_ENTRY = 1062;

struct GpsException : std::exception
{
  std::string msg;

  GpsException(const std::string& _msg)
    : msg(_msg)
  {}

  GpsException(const boost::format& _msg)
    : msg(_msg.str())
  {}

  GpsException(const std::exception& _e)
    : msg(_e.what())
  {}

  const char* what() const noexcept
  {
    return msg.c_str();
  }
};

#endif /* _REGIST_CSA_EXCEPTION_H */
