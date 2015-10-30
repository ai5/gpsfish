/* computerPlayer.cc
 */
#include "osl/game_playing/computerPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/openingBookTracer.h"
#include "osl/random.h"

osl::game_playing::
ComputerPlayer::~ComputerPlayer()
{
}

bool osl::game_playing::
ComputerPlayer::isReasonableMove(const GameState&, Move, int)
{
  return true;
}

void osl::game_playing::
ComputerPlayer::allowSpeculativeSearch(bool value)
{
  speculative_search_allowed = value;
}

void osl::game_playing::
ComputerPlayer::setInitialState(const NumEffectState&)
{
}

bool osl::game_playing::
ComputerPlayer::stopSearchNow()
{
  return true;
}

void osl::game_playing::
ComputerPlayer::setRootIgnoreMoves(const MoveVector */*rim*/, bool) 
{
}

/* ------------------------------------------------------------------------- */
osl::game_playing::
ComputerPlayerSelectBestMoveInTime::~ComputerPlayerSelectBestMoveInTime()
{
}

/* ------------------------------------------------------------------------- */

osl::game_playing::
ResignPlayer::~ResignPlayer()
{
}

void osl::game_playing::
ResignPlayer::pushMove(Move)
{
}
void osl::game_playing::
ResignPlayer::popMove()
{
}
const osl::search::MoveWithComment osl::game_playing::
ResignPlayer::selectBestMove(const GameState&, int, int, int)
{
  return MoveWithComment(Move::INVALID());
}

/* ------------------------------------------------------------------------- */

osl::game_playing::
RandomPlayer::~RandomPlayer()
{
}

void osl::game_playing::
RandomPlayer::pushMove(Move)
{
}
void osl::game_playing::
RandomPlayer::popMove()
{
}
const osl::search::MoveWithComment osl::game_playing::
RandomPlayer::selectBestMove(const GameState& state, int, int, int)
{
  MoveVector moves;
  state.state().generateLegal(moves);
  if (moves.empty())
    return MoveWithComment(Move::INVALID());
  return MoveWithComment(moves[time_seeded_random() % moves.size()]);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
