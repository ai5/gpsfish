#include "osl/real_time.h"
#include <iostream>

int
main ()
{
  osl::RealTime realTime (10);

  while (realTime.timeLeft ())
    {
      long tlis = realTime.getTimeLeftApprox ();
      std::cout << tlis << std::endl;
      sleep (1);
    }

  return 0;
}
