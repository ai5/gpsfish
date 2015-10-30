/* bookPlayer.cc
 */
#include "osl/game_playing/bookPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/openingBookTracer.h"
#include "osl/book/compactBoard.h"
#include "osl/container/moveStack.h"
#include <iostream>
#include <stdexcept>

osl::game_playing::
BookPlayer::
BookPlayer(OpeningBookTracer *b, ComputerPlayer *s)
  : book(b), searcher(s), book_limit(-1), current_moves(0), valid_initial_position(true)
{
}

osl::game_playing::
BookPlayer::~BookPlayer()
{
}

osl::game_playing::ComputerPlayer* osl::game_playing::
BookPlayer::clone() const
{
  return new BookPlayer(book->clone(), searcher->clone());
}

void osl::game_playing::
BookPlayer::setBookLimit(int new_limit)
{
  book_limit = new_limit;
}

void osl::game_playing::
BookPlayer::setInitialState(const NumEffectState& state)
{
  SimpleState usual(HIRATE);
  valid_initial_position = (book::CompactBoard(state) == book::CompactBoard(usual));
  if (book->isVerbose() && !valid_initial_position)
    std::cerr << "book: end" << "\n";
}

void osl::game_playing::
BookPlayer::pushMove(Move m)
{
  ++current_moves;
  if (valid_initial_position)
    book->update(m);
  searcher->pushMove(m);
}
void osl::game_playing::
BookPlayer::popMove()
{
  --current_moves;
  if (valid_initial_position)
    book->popMove();
  searcher->popMove();
}

bool osl::game_playing::
BookPlayer::bookAvailable() const
{
  return valid_initial_position 
    && (! book->isOutOfBook())
    && (book_limit < 0 || current_moves < book_limit);
}

const osl::Move osl::game_playing::
BookPlayer::moveByBook(const GameState& state)
{
  if (bookAvailable())
  {
    const Move best_move = book->selectMove();
    if (best_move.isNormal()
	&& (! state.isIllegal(best_move)))
      return best_move;
  }
  return Move::INVALID();
}

const osl::search::MoveWithComment osl::game_playing::
BookPlayer::selectBestMove(const GameState& state, int limit, int elapsed, int byoyomi)
{
  const Move move = moveByBook(state);
  if (move.isNormal())
    return MoveWithComment(move);
  return searcher->selectBestMove(state, limit, elapsed, byoyomi);
}

const osl::search::MoveWithComment osl::game_playing::
BookPlayer::selectBestMoveInTime(const GameState& state, const search::TimeAssigned& msec)
{
  const Move move = moveByBook(state);
  if (move.isNormal())
    return MoveWithComment(move);
  if (ComputerPlayerSelectBestMoveInTime *p
      = dynamic_cast<ComputerPlayerSelectBestMoveInTime *>(searcher.get()))
    return p->selectBestMoveInTime(state, msec);
  throw std::runtime_error("type error in BookPlayer::selectBestMoveInTime");
}

void osl::game_playing::
BookPlayer::allowSpeculativeSearch(bool value)
{
  ComputerPlayer::allowSpeculativeSearch(value);
  searcher->allowSpeculativeSearch(value);
}

void osl::game_playing::
BookPlayer::setRootIgnoreMoves(const MoveVector *rim, bool prediction)
{
  ComputerPlayer::setRootIgnoreMoves(rim, prediction);
  searcher->setRootIgnoreMoves(rim, prediction);
}

bool osl::game_playing::
BookPlayer::stopSearchNow()
{
  return searcher->stopSearchNow();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
