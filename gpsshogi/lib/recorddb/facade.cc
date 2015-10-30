/* facade.cc
 */
#include "gpsshogi/recorddb/facade.h"
#ifdef USE_TOKYO_CABINET
#  include "gpsshogi/recorddb/recordDb.h"
#  include "gpsshogi/dbm/tokyoCabinet.h"
#  include "osl/oslConfig.h"

bool gpsshogi::recorddb::
query(const osl::SimpleState& state, int &pro_win, int& pro_loss, 
      int &gps_win, int& gps_loss, int &bonanza_win, int& bonanza_loss)
{
  bool found = false;
  pro_win = pro_loss = gps_win = gps_loss = bonanza_win = bonanza_loss = 0;
  try
  {
    std::string filename = osl::OslConfig::home()+"/data/recorddb/pro.tch";
    RecordDB db(filename.c_str(), true);
    std::string key = db.makeKey(state);
    SquareInfo info;
    if (db.get(key, info))
    {
      pro_win = info.win();
      pro_loss = info.loss();
      found = true;
      if (state.turn() == osl::WHITE) 
	std::swap(pro_win, pro_loss);
    }
  }
  catch (dbm::NotOpenedException&)
  {
  }
  try
  {
    std::string filename = osl::OslConfig::home()+"/data/recorddb/floodgate.tch";
    RecordDB db(filename.c_str(), true);
    std::string key = db.makeKey(state);
    SquareInfo info;
    if (db.get(key, info))
    {
      gps_win = info.win_by_gps(), gps_loss = info.loss_by_gps();
      bonanza_win = info.win() - info.win_by_gps(), bonanza_loss = info.loss() - info.loss_by_gps();
      if (state.turn() == osl::WHITE) 
      {
	std::swap(gps_win, bonanza_loss);
	std::swap(gps_loss, bonanza_win);
      }
      found = true;
    }
  }
  catch (dbm::NotOpenedException&)
  {
  }
  return found;
}
#endif

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
