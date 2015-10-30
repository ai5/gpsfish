#ifndef _EFFECTUTIL_H
#define _EFFECTUTIL_H

#include "osl/numEffectState.h"
#include "osl/container.h"

#include <iosfwd>
#include <cassert>

namespace osl
{
  namespace effect_util
  {
  /**
   * EffectState を活用するためのメソッド
   * NumSimpleEffect などの公開インターフェースで
   * 使って書けるコード
   */
  struct EffectUtil
  {
    /**
     * state の position に ptypeo があった場合を仮定して，脅威をoutに集める
     */
    template <class EvalT>
    static void findThreat(const NumEffectState& state, Square position,
			   PtypeO ptypeo, PieceVector& out);
    template <class EvalT>
    struct FindThreat;
    struct SafeCapture;
  };

  } // namespace effect_util
  using effect_util::EffectUtil;
} // namespace osl

#endif /* _EFFECTUTIL_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
