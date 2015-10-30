/* enterKing.h
 */
#ifndef OSl_ENTERKING_H
#define OSl_ENTERKING_H

#include "osl/numEffectState.h"

namespace osl
{
  namespace enter_king
  {
    struct EnterKing
    {
      static bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      canDeclareWin(const NumEffectState& state);
      template <Player P>
      static bool
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      canDeclareWin(const NumEffectState& state);

      static bool canDeclareWin(const NumEffectState& state, int &drops);
      template <Player P>
      static bool canDeclareWin(const NumEffectState& state, int &drops);
    };

  } // namespace enter_king
  using enter_king::EnterKing;
} // namespace osl

#endif /* OSl_ENTERKING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
