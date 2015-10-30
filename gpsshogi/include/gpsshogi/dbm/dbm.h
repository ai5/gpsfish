#ifndef GPSSHOGI_DBM_DBM_H
#define GPSSHOGI_DBM_DBM_H
#include <boost/cstdint.hpp>
#include <string>

namespace gpsshogi
{
  namespace dbm
  {
    class DBMWrapper
    {
    public:
      DBMWrapper () { }
      virtual ~DBMWrapper() { }
      virtual bool hasKey(const std::string &key) = 0;
      virtual bool get(const std::string &key, std::string &value) = 0;
      virtual bool put(const std::string &key, const std::string &value) = 0;
      virtual void initIterator() = 0;
      virtual bool next(std::string &key, std::string &value) = 0;
      virtual uint64_t size() = 0;
      virtual bool optimize() = 0;
    };
  }
}

#endif // GPSSHOGI_DBM_DBM_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
