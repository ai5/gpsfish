/* inaniwa.h
 */
#ifndef INANIWA_H
#define INANIWA_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace inaniwa
  {
    struct InaniwaDetection {
      static bool IsInaniwa(const osl::NumEffectState& state, int my_total_sec, int op_total_sec, int my_remain_sec, int op_remain_sec);
    };
  } // namespace inaniwa
} // namespace osl
#endif /* INANIWA_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
