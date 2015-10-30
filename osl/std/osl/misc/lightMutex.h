/* lightMutex.h
 */
#ifndef OSL_LIGHT_MUTEX_H
#define OSL_LIGHT_MUTEX_H

#include "osl/oslConfig.h"
#ifdef PROFILE_MUTEX
#  include "osl/misc/perfmon.h"
#endif
#include <thread>
//#include <boost/utility.hpp>

namespace osl
{
  namespace misc
  {
#if defined OSL_USE_RACE_DETECTOR || defined _MSC_VER
    typedef std::mutex LightMutex;
    typedef std::mutex LightMutexChar;
#else
    template <class Mutex>
    class LightScopedLock {
      LightScopedLock(const LightScopedLock&) = delete;
      LightScopedLock& operator=(const LightScopedLock&) = delete;

      Mutex& m;
    public:
#ifdef PROFILE_MUTEX
      LightScopedLock(osl::misc::CounterPair &c,Mutex& m) :m(m){
	c.count2();
	while(!m.tryLock()){
	  for(int i=0;i<2;i++){
	    if(!m.waitLock(100)) break;
	    if(m.tryLock()) return;
	  }
	  c.count1();
	  std::this_thread::yield();
	}
      }
#else
      LightScopedLock(Mutex& m) :m(m){
	m.lock();
      }
#endif
      ~LightScopedLock(){
	m.unlock();
      }
    };


    class LightMutex {
      LightMutex(const LightMutex&) = delete;
      LightMutex& operator=(const LightMutex&) = delete;
      
      volatile int data;
    public:
      typedef LightScopedLock<LightMutex> scoped_lock;
      class unlockable_lock;
      LightMutex() :data(0) {}
      bool tryLock(){
	if(data!=0) return false;
	int dummy;
#ifdef __GNUC__
	asm __volatile__(" movl $1,%0" "\n\t"
			 " xchgl (%1),%0" "\n\t"
			 : "=&q"(dummy)
			 : "q"(&data)
			 : "cc");
#else
#  error "not supported"
#endif
	return dummy==0;
      }
      bool waitLock(int counter){
	for(int i=0;i<counter;i++){
#ifdef __GNUC__
	  asm __volatile__(" pause" "\n\t");
#endif
	  if(data==0)
	    return true;
	}
	return false;
      }
      void lock(){
	while(!tryLock()){
	  for(int i=0;i<2;i++){
	    if(!waitLock(100)) break;
	    if(tryLock()) return;
	  }
	  std::this_thread::yield();
	}
      }
      void unlock(){
	data=0;
      }
    };
    
    /** requirement: thread local */
    class LightMutex::unlockable_lock {
      unlockable_lock(const unlockable_lock&);
      unlockable_lock& operator=(const unlockable_lock&);
      
      LightMutex& m;
      bool locked;
    public:
      unlockable_lock(LightMutex& m) :m(m), locked(true) {
	m.lock();
      }
      ~unlockable_lock(){
	unlock();
      }
      void unlock()
      {
	if (locked) {
	  locked = false;
	  m.unlock();
	}
      }
    };

    class LightMutexChar {
      LightMutexChar(const LightMutexChar&) = delete;
      LightMutexChar& operator=(const LightMutexChar&) = delete;
      
      volatile char data;
    public:
      typedef LightScopedLock<LightMutexChar> scoped_lock;
      LightMutexChar() :data(0) {}
      bool tryLock(){
	if(data!=0) return false;
	char dummy;
#ifdef __GNUC__
	asm __volatile__(" movb $1,%0" "\n\t"
			 " xchgb (%1),%0" "\n\t"
			 : "=&q"(dummy)
			 : "q"(&data)
			 : "cc");
#else
#  error "not supported"
#endif
	return dummy==0;
      }
      bool waitLock(int counter){
	for(int i=0;i<counter;i++){
#ifdef __GNUC__
	  asm __volatile__(" pause" "\n\t");
#endif
	  if(data==0)
	    return true;
	}
	return false;
      }
      void lock(){
	while(!tryLock()){
	  for(int i=0;i<2;i++){
	    if(!waitLock(100)) break;
	    if(tryLock()) return;
	  }
	  std::this_thread::yield();
	}
      }
      void unlock(){
	data=0;
      }
    };
#endif

#ifdef PROFILE_MUTEX
#  define SCOPED_LOCK(lock,m) \
    static osl::misc::CounterPair c(__FILE__, __FUNCTION__, __LINE__);	\
    osl::misc::LightMutex::scoped_lock lock(c,m);
#  define SCOPED_LOCK_CHAR(lock,m) \
    static osl::misc::CounterPair c(__FILE__, __FUNCTION__, __LINE__);	\
    osl::misc::LightMutexChar::scoped_lock lock(c,m);
#else
#  define SCOPED_LOCK(lock,m) \
    osl::misc::LightMutex::scoped_lock lock(m);
#  define SCOPED_LOCK_CHAR(lock,m) \
    osl::misc::LightMutexChar::scoped_lock lock(m);
#endif
  }
  using misc::LightMutex;
}
#endif /* OSL_LIGHT_MUTEX_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

