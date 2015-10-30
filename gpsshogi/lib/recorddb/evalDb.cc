/* evalDb.cc
 */
#include "gpsshogi/recorddb/evalDb.h"
#include "gpsshogi/recorddb/recordDb.h"
#ifdef USE_TOKYO_CABINET
#  include "gpsshogi/dbm/tokyoCabinet.h"
#else
#  include "gpsshogi/dbm/doNothing.h"
#endif

gpsshogi::
EvalDB::EvalDB(const char *filename, bool readonly)
{
#ifdef USE_TOKYO_CABINET
  db.reset(new dbm::TokyoCabinetWrapper(filename, readonly ? HDBOREADER : (HDBOWRITER | HDBOCREAT | HDBOREADER)));
#else
  db.reset(new dbm::DoNothingWrapper());
#endif
}

gpsshogi::
EvalDB::~EvalDB()
{
}

bool gpsshogi::
EvalDB::get(const std::string& key, EvalInfo& info)
{
  std::string value;
  if (! db->get(key, value))
    return false;
  info.ParseFromString(value);
  return true;
}
bool gpsshogi::
EvalDB::get(const osl::SimpleState& state, EvalInfo& info) 
{
  return get(RecordDB::makeKey(state), info);
}
bool gpsshogi::
EvalDB::put(const std::string& key, const EvalInfo& info) 
{
  std::string value;
  info.SerializeToString(&value);
  return db->put(key, value);
}
bool gpsshogi::
EvalDB::put(const osl::SimpleState& state, const EvalInfo& info) 
{
  return put(RecordDB::makeKey(state), info);
}
bool gpsshogi::
EvalDB::optimize()
{
  return db->optimize();
}
void gpsshogi::
EvalDB::initIterator() 
{
  db->initIterator(); 
}

bool gpsshogi::
EvalDB::next(osl::record::MiniBoardChar50 &state, EvalInfo &info)
{
  std::string key, value;
  if (! db->next(key, value))
    return false;
  state = osl::record::MiniBoardChar50(key);
  info.ParseFromString(value);
  return true;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
