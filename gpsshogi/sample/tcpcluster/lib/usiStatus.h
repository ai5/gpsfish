/* usiStatus.h
 */
#ifndef GPSHSOGI_USISTATUS_H
#define GPSHSOGI_USISTATUS_H
#include <iosfwd>
namespace gpsshogi
{
  enum UsiStatus {
    WaitConnection, Initializing, Idle,
    Go, SearchStop, GoMate, CheckmateStop, 
    Quitting, Disconnected, Sentinel 
  };
  std::ostream& operator<<(std::ostream&, UsiStatus);

  const int usi_keep_alive = 30;
  const int usi_pawn_value = 100;
  const int usi_win_value = usi_pawn_value*300;
  const int probe_msec = 1000;
}


#endif /* GPSHSOGI_USISTATUS_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
