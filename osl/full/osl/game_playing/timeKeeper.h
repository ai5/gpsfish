/* timeKeeper.h
 */
#ifndef GAME_PLAYING_TIMEKEEPER_H
#define GAME_PLAYING_TIMEKEEPER_H

#include "osl/basic_type.h"
#include <memory>
namespace osl
{
  namespace game_playing
  {
    class TimeKeeper
    {
      struct Stack;
      std::unique_ptr<Stack> seconds;
    public:
      TimeKeeper();
      TimeKeeper(int black_time, int white_time);
      ~TimeKeeper();

      void reset(int black_time, int white_time);

      void pushMove(Player, int seconds);
      void popMove();
      int timeLeft(Player) const;
      int timeElapsed(Player) const;
      int timeLimit(Player) const;
    };
  } // namespace game_playing
} // namespace osl

#endif /* GAME_PLAYING_TIMEKEEPER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
