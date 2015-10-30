/* facade.h
 */
#ifndef GPSSHOGI_RECORDDB_FACADE_H
#define GPSSHOGI_RECORDDB_FACADE_H
#include "osl/state/simpleState.h"
namespace gpsshogi
{
  namespace recorddb
  {
#ifdef USE_TOKYO_CABINET
    bool query(const osl::SimpleState& state, int &pro_win, int& pro_loss, 
	       int &gps_win, int& gps_loss, int &bonanza_win, int& bonanza_loss);
#else
    bool query(const osl::SimpleState& state, int &pro_win, int& pro_loss, 
	       int &gps_win, int& gps_loss, int &bonanza_win, int& bonanza_loss)
    {
      return false;
    }
#endif
  }
}


#endif /* GPSSHOGI_RECORDDB_FACADE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
