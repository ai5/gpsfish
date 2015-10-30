#ifndef OSL_MILLISECONDS_H
#define OSL_MILLISECONDS_H

#include <chrono>
#include <stdexcept>

namespace osl
{
  namespace misc
  {
    struct NoMoreTime : std::runtime_error
    {
      NoMoreTime() : std::runtime_error("time limit over")
	{
	}
    };
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::time_point<clock> time_point;
    typedef std::chrono::milliseconds milliseconds;
    using std::chrono::duration_cast;
    template <class Duration>
    inline double toSeconds(Duration duration) {
      return duration_cast<std::chrono::duration<double>>(duration).count();
    }
    template <class Duration>
    inline long long msec(Duration duration) {
      return duration_cast<milliseconds>(duration).count();
    }
    inline double elapsedSeconds(time_point start) {
      return toSeconds(clock::now()-start);
    }
  } // namespace misc
  using misc::clock;
  using misc::time_point;
  using misc::milliseconds;
  using misc::elapsedSeconds;
  using misc::toSeconds;
  using misc::duration_cast;
  using misc::msec;
} // namespace osl


#endif // OSL_MILLISECONDS_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
