/* ratingEnv.cc
 */
#include "osl/rating/ratingEnv.h"
#include "osl/rating/feature/king8.h"
#include "osl/effect_util/sendOffSquare.h"
#include "osl/progress/effect5x3.h"

void osl::rating::
RatingEnv::update(const NumEffectState& new_state, Move last_move)
{
  history.push(last_move);
  make(new_state);
}

void osl::rating::
RatingEnv::make(const NumEffectState& state, 
		const PieceMask& my_pin, const PieceMask& op_pin, Progress16 progress)
{
  sendoffs.clear();
  const Square king_position = state.kingSquare(alt(state.turn()));
  effect_util::SendOffSquare::find(state.turn(), state, king_position, 
				     sendoffs);
  this->my_pin = my_pin;
  this->op_pin = op_pin;
  this->progress = progress;
  attack_count_for_turn = DefenseKing8::count(state);
  counteffect2_cache.fill(-1);
  pattern_cache.fill(-1);
}

void osl::rating::
RatingEnv::make(const NumEffectState& state)
{
  progress::Effect5x3 progress(state);
  make(state, 
       state.pin(state.turn()),
       state.pin(alt(state.turn())),
       progress.progress16());
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
