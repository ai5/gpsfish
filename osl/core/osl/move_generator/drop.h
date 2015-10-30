#ifndef _GENERATE_DROP_MOVES_H
#define _GENERATE_DROP_MOVES_H

#include "osl/numEffectState.h"
#include "osl/move_generator/move_action.h"

namespace osl
{
  namespace move_generator
  {
    /**
     * 打つ手を生成
     */
    template<class Action>
    class Drop
    {
    public:
      template<Player P>
      static void generate(const NumEffectState& state,Action& action);
    };
  } // namespace move_generator
} // namespace osl

#endif /* _GENERATE_DROP_MOVES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
