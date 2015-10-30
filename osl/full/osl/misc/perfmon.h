#ifndef MISC_PERFMON_H
#define MISC_PERFMON_H
#if defined(__i386__) || defined(__x86_64__) || defined(_MSC_VER)
#  define HAVE_TSC 1
#endif
#ifndef _MSC_VER
# include <sys/time.h>
# ifndef HAVE_TSC
#  include <sys/resource.h>
# endif
#endif
#include <iosfwd>
#include <string>
#include <cassert>
namespace osl
{
  namespace misc
  {
    class PerfMon
    {
#ifdef HAVE_TSC
      unsigned long long start_time;
#else
      rusage start_time;
#endif
    public:
      void restart()
      {
#ifdef HAVE_TSC
#  ifndef _MSC_VER
	unsigned int ax,dx;
	asm volatile("rdtsc\nmovl %%eax,%0\nmovl %%edx,%1":"=g"(ax),"=g"(dx): :"eax","edx");
	start_time =
	  (static_cast<unsigned long long>(dx)<<32)
	  + static_cast<unsigned long long>(ax);
#  else
	start_time = 0;
#  endif
#else
#ifdef NDEBUG
	getrusage(RUSAGE_SELF, &start_time);
#else
	int ret=getrusage(RUSAGE_SELF, &start_time);
	assert(ret==0);
#endif
#endif
      }
      PerfMon() {
	restart();
      }
      unsigned long long stop(){
#ifdef HAVE_TSC
#  ifndef _MSC_VER
	unsigned int ax,dx;
	asm volatile("rdtsc\nmovl %%eax,%0\nmovl %%edx,%1":"=g"(ax),"=g"(dx): :"eax","edx");
	const unsigned long long end_time
	  = ((static_cast<unsigned long long>(dx)<<32)
	     + static_cast<unsigned long long>(ax));
	return (end_time - PerfMon::start_time);
#  else
	return 0;
#  endif
#else
	rusage end_time;
#ifdef NDEBUG
	getrusage(RUSAGE_SELF,&end_time);
#else
	int ret=getrusage(RUSAGE_SELF,&end_time);
	assert(ret==0);
#endif
	return (end_time.ru_utime.tv_sec - start_time.ru_utime.tv_sec)*1000000
	  +(end_time.ru_utime.tv_usec - start_time.ru_utime.tv_usec);
#endif
      }
      void stop(const char *message,int loop){
	const unsigned long long cycles=stop();
	PerfMon::message(cycles, message, loop);
      }
      static void message(unsigned long long cycles,
			  const char *message,long long int loop);
    };
  
    class TSC
    {
      unsigned long long start_time;
      unsigned long long sum_time;
      long long int counter;
      std::string message;
    public:
      TSC(const char *m) :start_time(0ll),sum_time(0ll),counter(0ll),message(m) {}
      void start()
      {
#ifdef HAVE_TSC
#  ifndef _MSC_VER
	unsigned int ax,dx;
	asm volatile("rdtsc\nmovl %%eax,%0\nmovl %%edx,%1":"=g"(ax),"=g"(dx): :"eax","edx");
	start_time =
	  (static_cast<unsigned long long>(dx)<<32)
	  + static_cast<unsigned long long>(ax);
#  else
	start_time = 0;
#  endif
#endif
	counter++;
      }
      void stop(){
#ifdef HAVE_TSC
#  ifndef _MSC_VER
	unsigned int ax,dx;
	asm volatile("rdtsc\nmovl %%eax,%0\nmovl %%edx,%1":"=g"(ax),"=g"(dx): :"eax","edx");
	const unsigned long long end_time
	  = ((static_cast<unsigned long long>(dx)<<32)
	     + static_cast<unsigned long long>(ax));
	sum_time+=end_time - start_time;
#  else
	sum_time = 0;
#  endif
#endif
      }
      ~TSC()
      {
	PerfMon::message(sum_time,message.c_str(),counter);
      }
    };
    class Counter
    {
      unsigned long long int counter;
      std::string message;
    public:
      Counter(const char *m) :counter(0ll),message(m) {}
      Counter(std::string const& m) :counter(0ll),message(m) {}
      void count()
      {
	counter++;
      }
      ~Counter()
      {
	PerfMon::message(0ll,message.c_str(),counter);
      }
    };
    class CounterPair
    {
      unsigned long long int counter1;
      unsigned long long int counter2;
      std::string message;
    public:
      CounterPair(std::string const& m) :counter1(0ll),counter2(0ll),message(m) {}
      CounterPair(const char *file, const char *function, int line);
      void count1()
      {
	counter1++;
      }
      void count2()
      {
	counter2++;
      }
      ~CounterPair();
    };
#ifndef _MSC_VER
    class MeasureTimeLock
    {
      timeval start;
      std::ostream& os;
      char const* message;
    public:
      MeasureTimeLock (std::ostream& os, char const* message)
	: os (os), message (message)
      {
#ifndef NDEBUG
	int ret =
#endif
	  gettimeofday(&start, NULL);
	assert(ret == 0);
      }

      ~MeasureTimeLock();
    };
#endif
  } // namespace misc
} // namespace osl


#endif /* MISC_PERFMON_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
