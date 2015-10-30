#ifndef OSL_SHADOW_EFFECT_H
#define OSL_SHADOW_EFFECT_H
#include "osl/numEffectState.h"
namespace osl
{
  namespace effect_util
  {
    /**
     * 影利きを求める
     */
    struct ShadowEffect
    {
    private:
      template <int count_max>
      static int count(const NumEffectState&, Square target, 
		       Player attack);
    public:
      /**
       * target に attack の影利きが一つでもあるか．
       * 相手の追加利きが先にある場合は対象としない．
       */
      static bool hasEffect(const NumEffectState&, Square target, 
			    Player attack);
      /**
       * target に attack の影利きを二つまで数える.
       * 相手の追加利きの駒以降は対象としない．
       */
      static int count2(const NumEffectState&, Square target, 
			      Player attack);
    };
  } // namespace effect_util
  using effect_util::ShadowEffect;
} // namespace osl

#endif /* OSL_SHADOW_EFFECT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
