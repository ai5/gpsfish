/* perfmon.cc
 */
#include "osl/misc/perfmon.h"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <iomanip>

#ifndef _MSC_VER
void osl::misc::PerfMon::message(unsigned long long cycles,
				 const char *message,long long int loop)
{
#ifdef HAVE_TSC
  const char *unit = "clocks";
#else
  const char *unit = "microSecs";
#endif  
  std::cerr << std::dec << message << " : take " << cycles 
	    << " " << unit << ", loop= " << loop;
  if (loop)
    std::cerr << " clocks/loop= " <<  (cycles/loop) << "." 
	      << std::setfill('0') << std::setw(2) 
	      << (cycles*100/loop)-(cycles/loop)*100;
  std::cerr << std::endl;
}

osl::misc::CounterPair::
CounterPair(const char *file, const char *function, int line)
  : counter1(0), counter2(0),
    message(std::string(file)+":"+(function)+":"+boost::lexical_cast<std::string>(line))
{
}

osl::misc::CounterPair::~CounterPair()
{
  std::cerr << message << " " << counter1 << "/" << counter2;
  if(counter2!=0) std::cerr << " = " << (double)counter1/(double)counter2;
  std::cerr << std::endl;
}

osl::misc::MeasureTimeLock::~MeasureTimeLock ()
{
  timeval end;
  gettimeofday(&end, NULL);

  end.tv_usec -= start.tv_usec;

  if (end.tv_usec < 0)
  {
    end.tv_usec += 1000000;
    --end.tv_sec;
  }
  end.tv_sec -= start.tv_sec;

  os << message << "\t"
     << end.tv_sec << ":" << end.tv_usec
     << std::endl;
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
