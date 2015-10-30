#include "gpsshogi/dbm/tokyoCabinet.h"
#include <stdint.h>
gpsshogi::dbm::TokyoCabinetWrapper::
TokyoCabinetWrapper(const std::string &filename,
		    int flag)
{
  opened = false;
  hdb = tchdbnew();
  if (!tchdbsetmutex(hdb))
  {
    return;
  }
  if (!tchdbopen(hdb, filename.c_str(), flag))
  {
    return;
  }
  opened = true;
}

gpsshogi::dbm::TokyoCabinetWrapper::
~TokyoCabinetWrapper()
{
  tchdbclose(hdb); // Check error code?
  tchdbdel(hdb);
}

bool gpsshogi::dbm::TokyoCabinetWrapper::hasKey(const std::string &key)
{
  if (!opened)
  {
    throw NotOpenedException();
  }
  int value_len;
  char *value =
    static_cast<char *>(tchdbget(hdb, key.data(), key.length(), &value_len));
  if (value)
  {
    free(value);
    return true;
  }
  return false;  
}

bool gpsshogi::dbm::TokyoCabinetWrapper::get(const std::string &key,
					     std::string &value)
{
  if (!opened)
  {
    throw NotOpenedException();
  }
  int value_len;
  char *value_ptr =
    static_cast<char *>(tchdbget(hdb, key.data(), key.length(), &value_len));
  if (value_ptr)
  {
    value.assign(value_ptr, value_len);
    free(value_ptr);
    return true;
  }
  return false;  
}

bool gpsshogi::dbm::TokyoCabinetWrapper::put(const std::string &key,
					     const std::string &value)
{
  if (!opened)
  {
    throw NotOpenedException();
  }
  if (tchdbput(hdb, key.data(), key.length(),
	       value.data(), value.length()))
  {
    return true;
  }
  return false;  
}

void gpsshogi::dbm::TokyoCabinetWrapper::initIterator()
{
  if (!opened)
  {
    throw NotOpenedException();
  }
  tchdbiterinit(hdb);
}

bool gpsshogi::dbm::TokyoCabinetWrapper::next(std::string &key,
					      std::string &value)
{
  if (!opened)
  {
    throw NotOpenedException();
  }
  int key_len;
  char *key_ptr = static_cast<char *>(tchdbiternext(hdb, &key_len));
  if (key_ptr != NULL)
  {
    key.assign(key_ptr, key_len);
    free(key_ptr);
    return get(key, value);
  }
  return false;
}

bool gpsshogi::dbm::TokyoCabinetWrapper::resize(size_t new_bucket)
{
  return tchdboptimize(hdb, new_bucket, -1, -1, /*UINT8_MAX*/255);
}

uint64_t gpsshogi::dbm::TokyoCabinetWrapper::size() 
{
  return tchdbrnum(hdb);
}
uint64_t gpsshogi::dbm::TokyoCabinetWrapper::bucketSize() 
{
  return tchdbbnum(hdb);
}
bool gpsshogi::dbm::TokyoCabinetWrapper::optimize()
{
  return bucketSize() > size()*2
     || resize(size()*2);
}
