#ifndef GPSSHOGI_DBM_DO_NOTHING_H
#define GPSSHOGI_DBM_DO_NOTHING_H
#include "gpsshogi/dbm/dbm.h"

namespace gpsshogi
{
  namespace dbm
  {
    class DoNothingWrapper : public DBMWrapper
    {
    public:
      DoNothingWrapper() { };
      ~DoNothingWrapper() { };
      bool hasKey(const std::string &key);
      bool get(const std::string &key, std::string &value);
      bool put(const std::string &key, const std::string &value);
      void initIterator() {}
      bool next(std::string &key, std::string &value) { return false; }
      uint64_t size();
      bool optimize();
    };
  }
}
#endif
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

