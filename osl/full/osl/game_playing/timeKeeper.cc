/* timeKeeper.cc
 */
#include "osl/game_playing/timeKeeper.h"
#include <vector>
struct osl::game_playing::TimeKeeper::Stack
  : public std::vector<std::pair<int,int> >
{
};

osl::game_playing::
TimeKeeper::TimeKeeper() : seconds(new Stack())
{
  reset(1500, 1500); // default: 25 min
}

osl::game_playing::
TimeKeeper::TimeKeeper(int black_time, int white_time) 
  : seconds(new Stack())
{
  reset(black_time, white_time);
}

osl::game_playing::
TimeKeeper::~TimeKeeper()
{
}

void osl::game_playing::
TimeKeeper::reset(int black_time, int white_time)
{
  seconds->clear();
  seconds->push_back(std::make_pair(black_time, white_time));
}

void osl::game_playing::
TimeKeeper::pushMove(Player turn, int consumed)
{
  std::pair<int,int> time_left = seconds->back();
  if (turn == BLACK)
    time_left.first -= consumed;
  else
    time_left.second -= consumed;
  seconds->push_back(time_left);
}

void osl::game_playing::
TimeKeeper::popMove()
{
  assert(! seconds->empty());
  seconds->pop_back();
}

int osl::game_playing::
TimeKeeper::timeLeft(Player player) const
{
  const std::pair<int,int>& time_left = seconds->back();
  return (player == BLACK) ? time_left.first : time_left.second;
}

int osl::game_playing::
TimeKeeper::timeElapsed(Player player) const
{
  return timeLimit(player) - timeLeft(player);
}

int osl::game_playing::
TimeKeeper::timeLimit(Player player) const
{
  const std::pair<int,int>& time_left = seconds->front();
  return (player == BLACK) ? time_left.first : time_left.second;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
