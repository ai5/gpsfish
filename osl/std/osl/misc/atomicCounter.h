/* atomicCounter.h
 */
#ifndef OSL_ATOMICCOUNTER_H
#define OSL_ATOMICCOUNTER_H

#include "osl/config.h"
#include <atomic>
#include <algorithm>

namespace osl
{
  namespace misc
  {
    template <class Counter>
    struct IncrementLock
    {
      Counter& counter;
      explicit IncrementLock(Counter& c) : counter(c) 
      {
	counter.inc();
      }
      ~IncrementLock() 
      {
	counter.dec();
      }
      IncrementLock(const IncrementLock&) = delete;
      IncrementLock& operator=(const IncrementLock&) = delete;
    };
    class AtomicCounter
    {
      std::atomic<int> count;
    public:
      explicit AtomicCounter(int count_=0) {
	this->count=count_;
      }
      void inc(){
	count.fetch_add(1);
      }
      void inc(int value){
	count.fetch_add(value);
      }
      int valueAndinc(){
	return count.fetch_add(1);
      }
      void dec(){
	count.fetch_sub(1);
      }
      void max(int val){
	int x=count;
	if(x<val){
	  while(! count.compare_exchange_weak(x,val) && x<val)
	    ;
	}
      }
      int value() const{ 
	return count; 
      }
      void setValue(int value) { 
	count = value; 
      }
      typedef IncrementLock<AtomicCounter> IncLock;
    };
  }
  using misc::AtomicCounter;
}

#endif /* OSL_ATOMICCOUNTER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

