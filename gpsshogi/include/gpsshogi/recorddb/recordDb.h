/* recordDb.h
 */
#ifndef GPSSHOGI_RECORDDB_H
#define GPSSHOGI_RECORDDB_H

#include "recordDb.pb.h"
#include "gpsshogi/dbm/dbm.h"
#include "osl/record/miniBoardChar50.h"
#include <boost/scoped_ptr.hpp>

namespace gpsshogi
{
  class RecordDB
  {
    std::unique_ptr<dbm::DBMWrapper> db;
  public:
    RecordDB(const char *filename, bool readonly);
    ~RecordDB();

    static const std::string makeKey(const osl::SimpleState& state);
    bool get(const std::string& key, SquareInfo& info);
    bool get(const osl::SimpleState& state, SquareInfo& info);
    bool put(const std::string& key, const SquareInfo& info);
    bool put(const osl::SimpleState& state, const SquareInfo& info);
    bool optimize();
    size_t size() { return db->size(); }

    void initIterator();
    bool next(osl::record::MiniBoardChar50 &state, SquareInfo &info);
  };
}

#endif /* GPSSHOGI_RECORDDB_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
