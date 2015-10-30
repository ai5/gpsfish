/* effectUtil.cc
 */
#include "osl/effect_util/effectUtil.h"
#include "osl/effect_util/effectUtil.tcc"
#include "osl/eval/openMidEndingEval.h"
#include "osl/numEffectState.tcc"

#ifndef DFPNSTATONE
namespace osl
{
  template void
  EffectUtil::findThreat<osl::eval::ml::OpenMidEndingEval>(
    const NumEffectState& state,
    Square position,
    PtypeO ptypeo,
    PieceVector& out);
}
#endif

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
