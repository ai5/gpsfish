/* timeCondition.h
 */
#ifndef GPSSHOGI_TIMECONDITION_H
#define GPSSHOGI_TIMECONDITION_H
#include "osl/basic_type.h"
namespace gpsshogi
{
  struct TimeCondition
  {
    int total, byoyomi_msec, used, opponent_used;
    bool allow_ponder;
    osl::Player my_turn; // for ponder
    TimeCondition() : total(0), byoyomi_msec(0), used(0), opponent_used(0),
		      allow_ponder(false), my_turn(osl::BLACK)
    {
    }
    int byoyomi() const { return byoyomi_msec/1000; }
  };
}

#endif /* GPSSHOGI_TIMECONDITION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
