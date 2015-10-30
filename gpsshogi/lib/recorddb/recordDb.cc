/* recordDb.cc
 */
#include "gpsshogi/recorddb/recordDb.h"
#include "gpsshogi/recorddb/recordDb.pb.cc"
#ifdef USE_TOKYO_CABINET
#  include "gpsshogi/dbm/tokyoCabinet.h"
#else
#  include "gpsshogi/dbm/doNothing.h"
#endif

gpsshogi::
RecordDB::RecordDB(const char *filename, bool readonly)
{
#ifdef USE_TOKYO_CABINET
  db.reset(new dbm::TokyoCabinetWrapper(filename, readonly ? HDBOREADER : (HDBOWRITER | HDBOCREAT | HDBOREADER)));
#else
  db.reset(new dbm::DoNothingWrapper());
#endif
}

gpsshogi::
RecordDB::~RecordDB()
{
}

const std::string gpsshogi::
RecordDB::makeKey(const osl::SimpleState& state) 
{
  osl::record::MiniBoardChar50 board(state);
  return board.toString();
}
bool gpsshogi::
RecordDB::get(const std::string& key, SquareInfo& info)
{
  std::string value;
  if (! db->get(key, value))
    return false;
  info.ParseFromString(value);
  return true;
}
bool gpsshogi::
RecordDB::get(const osl::SimpleState& state, SquareInfo& info) 
{
  return get(makeKey(state), info);
}
bool gpsshogi::
RecordDB::put(const std::string& key, const SquareInfo& info) 
{
  std::string value;
  info.SerializeToString(&value);
  return db->put(key, value);
}
bool gpsshogi::
RecordDB::put(const osl::SimpleState& state, const SquareInfo& info) 
{
  return put(makeKey(state), info);
}
bool gpsshogi::
RecordDB::optimize()
{
  return db->optimize();
}
void gpsshogi::
RecordDB::initIterator() 
{
  db->initIterator(); 
}

bool gpsshogi::
RecordDB::next(osl::record::MiniBoardChar50 &state, SquareInfo &info)
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
