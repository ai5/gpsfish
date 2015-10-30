#ifndef OSL_CSA_TIME_H
#define OSL_CSA_TIME_H
#include "osl/misc/milliSeconds.h"
#include <string>
#include <cmath>

namespace osl
{
  namespace game_playing
  {
  class CsaTime
  {
    time_point start, opmove, mymove;
    long mytimeleft, optimeleft;
  public:
    explicit CsaTime(long timeleft) 
      : mytimeleft(timeleft), optimeleft(timeleft)
    {
      mymove = opmove = start = clock::now();
    }
    CsaTime(long myTimeLeft, long opTimeLeft) 
      : mytimeleft(myTimeLeft), optimeleft(opTimeLeft)
    {
      mymove = opmove = start = clock::now();
    }
    long makeOpMove() {
      opmove = clock::now();
      long ret = (long)floor(toSeconds(opmove - mymove));
      if (ret == 0) { ret = 1; }
      optimeleft -= ret;
      return ret;
    }
    long makeMyMove() {
      mymove = clock::now();
      long ret = (long)floor(toSeconds(mymove - opmove));
      if(ret == 0) { ret = 1; }
      mytimeleft -= ret;
      return ret;
    }
    long getMyLeft() const { return mytimeleft; }
    long getOpLeft() const { return optimeleft; }
    const std::string getStart() const;
    static const std::string curruntTime();
  };
} // namespace game_playing
} // namespace osl
#endif // OSL_CSA_TIME
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
