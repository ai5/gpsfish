/* ratingEnv.h
 */
#ifndef _RATINGENV_H
#define _RATINGENV_H

#include "osl/numEffectState.h"
#include "osl/container/moveStack.h"
#include "osl/container/square8.h"
#include "osl/progress.h"

namespace osl
{
  namespace rating
  {
    class RatingEnv
    {
    public:
      MoveStack history;
      Square8 sendoffs;
      PieceMask my_pin, op_pin;
      int attack_count_for_turn;
      Progress16 progress;
      mutable CArray<signed char,Square::SIZE> counteffect2_cache;
      mutable CArray<int,Square::SIZE> pattern_cache;

      void update(const NumEffectState& new_state, Move last_move);
      void make(const NumEffectState& new_state);
      void make(const NumEffectState& new_state, 
		const PieceMask& my_pin, const PieceMask& op_pin, Progress16);
    };
  }
  using rating::RatingEnv;
}

#endif /* _RATINGENV_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
