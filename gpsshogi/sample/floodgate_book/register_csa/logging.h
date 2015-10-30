#ifndef _REGIST_CSA_LOGGING_H
#define _REGIST_CSA_LOGGING_H

#include <boost/format.hpp>
#include <exception>
#include <string>

namespace sql
{
  class SQLException;
}

void logInfo(const std::string& method, const std::string& msg);
void logInfo(const std::string& method, const boost::format& msg);
void logInfo(const std::string& method, const std::exception& e);

void logWarn(const std::string& method, const std::string& msg);
void logWarn(const std::string& method, const boost::format& msg);
void logWarn(const std::string& method, const std::exception& e);

void logError(const std::string& method, const std::string& msg);
void logError(const std::string& method, const boost::format& msg);
void logError(const std::string& method, const sql::SQLException& e);
void logError(const std::string& method, const std::exception& e);

#endif /* _REGIST_CSA_LOGGING_H */
