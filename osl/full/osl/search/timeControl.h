/* timeControl.h
 */
#ifndef _SEARCH_TIMECONTROL_H
#define _SEARCH_TIMECONTROL_H

namespace osl
{
  namespace search
  {
    struct TimeControl
    {
      static int secondsForThisMove(int total_seconds);
    };
  } // namespace search
} // namespace osl

#endif /* _SEARCH_TIMECONTROL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
