/* timeControl.cc
 */
#include "osl/search/timeControl.h"

int osl::search::TimeControl::
secondsForThisMove(int totalSeconds)
{
  if (totalSeconds < 90)
    return 1;
  if (totalSeconds < 2*60)
    return 4;
  if (totalSeconds < 3*60)
    return 8;
  if (totalSeconds < 4*60)
    return 15;
  if (totalSeconds < 6*60)
    return 22;
  if (totalSeconds < 8*60)
    return 30;
  if (totalSeconds < 10*60)
    return 42;
  if (totalSeconds < 12*60)
    return 55;
  if (totalSeconds < 25*60)
    return 67;
  if (totalSeconds < 40*60)
    return 90;
  if (totalSeconds < 60*60)	// 1h
    return 135;
  if (totalSeconds < 90*60)	// 1.5h
    return 197;
  if (totalSeconds < 2*60*60)	// 2h
    return 270;
  return 600;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
