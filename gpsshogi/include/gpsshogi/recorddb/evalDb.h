/* evalDb.h
 */
#ifndef GPSSHOGI_EVALDB_H
#define GPSSHOGI_EVALDB_H

#include "recordDb.pb.h"
#include "gpsshogi/dbm/dbm.h"
#include "osl/record/miniBoardChar50.h"
#include <boost/scoped_ptr.hpp>

namespace gpsshogi
{
  class EvalDB
  {
    std::unique_ptr<dbm::DBMWrapper> db;
  public:
    EvalDB(const char *filename, bool readonly);
    ~EvalDB();

    bool get(const std::string& key, EvalInfo& info);
    bool get(const osl::SimpleState& state, EvalInfo& info);
    bool put(const std::string& key, const EvalInfo& info);
    bool put(const osl::SimpleState& state, const EvalInfo& info);
    bool optimize();
    size_t size() { return db->size(); }

    void initIterator();
    bool next(osl::record::MiniBoardChar50 &state, EvalInfo &info);
  };
}

#endif /* GPSSHOGI_EVALDB_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
