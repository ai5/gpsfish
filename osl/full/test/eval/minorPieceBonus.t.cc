/* minorPieceBonus.t.cc
 */
#include "osl/eval/minorPieceBonus.h"
#include "osl/progress/effect5x3.h"
#include "consistencyTest.h"
#include <boost/test/unit_test.hpp>
using namespace osl;
using namespace osl::eval;

struct MinorPieceWithProgress
{
  typedef progress::Effect5x3WithBonus progress_t;
  progress_t current_progress;
  MinorPieceBonus minor_piece_bonus;

  MinorPieceWithProgress(const NumEffectState& state) 
    : current_progress(state), minor_piece_bonus(state)
  {
  }

  int value() const
  {
    return minor_piece_bonus.value(current_progress.progress16(),
				   current_progress.progress16bonus(BLACK),
				   current_progress.progress16bonus(WHITE));
  }
  void changeTurn() {}
  int expect(const NumEffectState& state, Move move) const
  {
    NumEffectState new_state = state;
    new_state.makeMove(move);
    progress_t next = current_progress;
    next.update(new_state, move);
    return minor_piece_bonus.expect(state, move,
				    next.progress16(),
				    next.progress16bonus(BLACK),
				    next.progress16bonus(WHITE));
  }
  void update(const NumEffectState& new_state, Move last_move)
  {
    current_progress = progress_t(new_state);
    minor_piece_bonus.update(new_state, last_move);
  }
};

BOOST_AUTO_TEST_CASE(MinorPieceBonusTestConsistentUpdate)
{
  consistencyTestUpdate<MinorPieceWithProgress>();
}

BOOST_AUTO_TEST_CASE(MinorPieceBonusTestConsistentExpect)
{
  consistencyTestExpect<MinorPieceWithProgress>(0);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
