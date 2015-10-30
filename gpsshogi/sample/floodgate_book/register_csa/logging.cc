#include "logging.h"
#include <cppconn/exception.h>
#include <iostream>

namespace {

void log(const std::string& level, const std::string& method, const std::string& msg)
{
  std::cerr << level << " [" << method << "] " << msg << std::endl;
}

} // annonymous namespace

void logInfo(const std::string& method, const std::string& msg)
{
  log("INFO", method, msg);
}

void logWarn(const std::string& method, const std::string& msg)
{
  log("WARN", method, msg);
}

void logError(const std::string& method, const std::string& msg)
{
  log("ERROR", method, msg);
}

void logInfo(const std::string& method, const boost::format& msg)
{
  logInfo(method, msg.str());
}

void logWarn(const std::string& method, const boost::format& msg)
{
  logWarn(method, msg.str());
}

void logError(const std::string& method, const boost::format& msg)
{
  logError(method, msg.str());
}

void logInfo(const std::string& method, const std::exception& e)
{
  logInfo(method, e.what());
}

void logWarn(const std::string& method, const std::exception& e)
{
  logWarn(method, e.what());
}

void logError(const std::string& method, const sql::SQLException& e)
{
  logError(method, boost::format("[%s] %s") % e.getErrorCode() % e.what());
}

void logError(const std::string& method, const std::exception& e)
{
  logError(method, e.what());
}


