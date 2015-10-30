/* csaStopwatch.h
 */
#ifndef GAME_PLAYING_CSASTOPWATCH_H
#define GAME_PLAYING_CSASTOPWATCH_H
#include "osl/misc/milliSeconds.h"
#include <cmath>
namespace osl
{
  namespace game_playing
  {
    class CsaStopwatch
    {
      time_point start;
    public:
      CsaStopwatch() : start(clock::now())
      {
      }
      int read() {
	double elapsed = elapsedSeconds(start);
	int ret = (int)floor(elapsed);
	return std::max(1, ret);
      }
    };
  } // namespace game_playing
} // namespace osl


#endif /* GAME_PLAYING_CSASTOPWATCH_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
