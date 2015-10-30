/* openingBookTracer.cc
 */
#include "osl/game_playing/openingBookTracer.h"

osl::game_playing::
OpeningBookTracer::~OpeningBookTracer()
{
}

/* ------------------------------------------------------------------------- */

osl::game_playing::
NullBook::~NullBook()
{
}

void osl::game_playing::
NullBook::update(Move)
{
}

const osl::Move osl::game_playing::
NullBook::selectMove() const
{
  return Move::INVALID();
}

bool osl::game_playing::
NullBook::isOutOfBook() const
{
  return true;
}

void osl::game_playing::
NullBook::popMove()
{
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
