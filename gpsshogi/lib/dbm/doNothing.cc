#include "gpsshogi/dbm/doNothing.h"

bool gpsshogi::dbm::DoNothingWrapper::hasKey(const std::string &key)
{
  return false;
}

bool gpsshogi::dbm::DoNothingWrapper::put(const std::string &key,
					  const std::string &value)
{
  return false;
}

bool gpsshogi::dbm::DoNothingWrapper::get(const std::string &key,
					  std::string &value)
{
  return false;
}

uint64_t gpsshogi::dbm::DoNothingWrapper::size() 
{
  return 0;
}
bool gpsshogi::dbm::DoNothingWrapper::optimize()
{
  return true;
}
