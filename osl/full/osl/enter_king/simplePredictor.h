/* simplePredictor.h
 */
#ifndef _SIMPLE_PREDICTOR_H
#define _SIMPLE_PREDICTOR_H
#include "osl/numEffectState.h"

namespace osl
{
  namespace enter_king
  {
    // 入玉を予測/判定
    // 宣言法での入玉予測/判定は名前の末尾に27 をつけている
    static const int winning_threshold_black = 24;
    static const int winning_threshold_white = 24;
    static const int winning_threshold_black_27 = 28;
    static const int winning_threshold_white_27 = 27;
  
    class SimplePredictor {
    public:
      template<Player Turn>
      double getProbability(const osl::NumEffectState& state);
      double getProbability(const osl::NumEffectState& state, const Player Turn);

      template<Player Turn>
      double getProbability27(const osl::NumEffectState& state);
      double getProbability27(const osl::NumEffectState& state, const Player Turn);

      template <Player Turn>
      bool predict(const osl::NumEffectState& state, double threshold=0.5);
      bool predict(const osl::NumEffectState& state, const Player Turn, double threshold=0.5);

      template <Player Turn>
      bool predict27(const osl::NumEffectState& state, double threshold=0.5);
      bool predict27(const osl::NumEffectState& state, const Player Turn, double threshold=0.5);
    };
  } //namespace enter_king
} //namespace osl
#endif /* _SIMPLE_PREDICTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
