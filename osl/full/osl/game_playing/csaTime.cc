/* csa_time.cc
 */
#include "osl/game_playing/csaTime.h"
#include <cstdio>

const std::string osl::game_playing::CsaTime::
getStart() const
{
  std::string result(128,'\0');
  int sec = start.time_since_epoch().count()/1000;
#ifdef _MSC_VER
  _snprintf(&result[0], result.size(), "%d", sec);
#else
  snprintf(&result[0], result.size(), "%d", sec);
#endif
  return result;
}

const std::string osl::
game_playing::CsaTime::curruntTime()
{
  std::string result(128,'\0');
  int sec = clock::now().time_since_epoch().count()/1000;
#ifdef _MSC_VER
  _snprintf(&result[0], result.size(), "%d", sec);
#else
  snprintf(&result[0], result.size(), "%d", sec);
#endif
  return result;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
