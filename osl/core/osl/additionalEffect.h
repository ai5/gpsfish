#ifndef OSL_ADDITIONAL_EFFECT_H
#define OSL_ADDITIONAL_EFFECT_H
#include "osl/numEffectState.h"

namespace osl
{
  namespace effect_util
  {
    /**
     * 追加利きを求める
     */
    struct AdditionalEffect
    {
    private:
      static void find(const NumEffectState&, Square target, 
		       const PieceVector& direct_effects,
		       PieceVector& black, PieceVector& white);
      template <int count_max>
      static int count(const NumEffectState&, Square target, 
		       Player attack);
    public:
      /**
       * target に attack の追加利きが一つでもあるか．
       * 相手の影利きが先にある場合は対象としない．
       */
      static bool hasEffect(const NumEffectState&, Square target, 
			    Player attack);
      static bool hasEffectStable(const NumEffectState&, Square target, 
				  Player attack);
      /**
       * target に attack の追加利きを二つまで数える.
       * 相手の影利きの駒以降は対象としない．
       */
      static int count2(const NumEffectState&, Square target, 
			      Player attack);
      /**
       * target に対する追加利きのある Piece を black, white に求める.
       * [*] +KI -HI +HI の場合，-HIも+HIもカウント.
       */
      static void find(const NumEffectState&, Square target, 
		       PieceVector& black, PieceVector& white);
      static void count(const NumEffectState&, Square target,
			int& black, int& white);
      static int count(const NumEffectState& state, Player pl, Square target)
      {
	int black, white;
	count(state, target, black, white);
	return (pl == BLACK) ? black : white;
      }
    };
  } // namespace effect_util
  using effect_util::AdditionalEffect;
} // namespace osl

#endif /* OSL_ADDITIONAL_EFFECT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
