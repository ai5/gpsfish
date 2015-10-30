/* additionalLance.h
 */
#ifndef OSL_ADDITIONALLANCE_H
#define OSL_ADDITIONALLANCE_H

#include "osl/numEffectState.h"
namespace osl
{
  namespace move_generator
  {
    template <Player P>
    struct AdditionalLance
    {
      /**
       * pawn に香車を打って追加利きをつける指手を生成.
       * 敵の利きがあるところで生成をやめる.
       */
      static void generate(const NumEffectState&, Square pawn, MoveVector& out);
      static void generateIfHasLance(const NumEffectState&, Square pawn, 
				     MoveVector& out);
    };
  } // namespace move_generator
} // namespace osl

#endif /* OSL_ADDITIONALLANCE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
