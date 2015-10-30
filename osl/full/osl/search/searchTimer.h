/* hasTimer.h
 */
#ifndef OSL_SEARCHTIMER_H
#define OSL_SEARCHTIMER_H
#include "osl/misc/milliSeconds.h"
#include "osl/misc/lightMutex.h"
#include "osl/oslConfig.h"
#include <limits>
#include <vector>

namespace osl
{
  namespace search
  {
    struct TimeAssigned
    {
      milliseconds standard, max;
      TimeAssigned() 
	: standard(milliseconds::max()),
	  max(milliseconds::max())
      {
      }
      explicit TimeAssigned(milliseconds assign) 
	: standard(assign), max(assign)
      {
      }
      TimeAssigned(milliseconds s, milliseconds m) 
	: standard(s), max(m)
      {
      }
    };
    class SearchMonitor;
    struct SearchTimerCommon
    {
      enum StopReason { NotStopped, NoMoreTime, NoMoreMemory, StopByOutside };
      /** 探索開始時刻 */
      time_point start_time;
      /** 探索予定時間 */
      TimeAssigned assigned;
      /** 時間が何倍残っていたら次のiteration に進むか */
      volatile double next_iteration_coefficient;
      volatile bool stop_all;
      volatile StopReason stop_reason;
      uint64_t node_count_hard_limit;
      volatile int last_memory_use1000;
      
      // work area
      time_point last_tested;
      uint64_t next_node_count;
      double nps;
      volatile bool stable;
      std::vector<std::shared_ptr<SearchMonitor> > monitors;
      typedef LightMutex Mutex;
      mutable Mutex mutex;

      SearchTimerCommon() 
	: start_time(clock::now()), 
	  next_iteration_coefficient(4.0), stop_all(0), stop_reason(NotStopped),
	  node_count_hard_limit(std::numeric_limits<uint64_t>::max()),
	  last_memory_use1000(0), stable(true)
      {
      }
    };
    class SearchTimer
    {
      std::shared_ptr<SearchTimerCommon> shared_timer;
      typedef SearchTimerCommon::Mutex Mutex;
    public:
      SearchTimer() : shared_timer(new SearchTimerCommon) {}
      SearchTimer(const SearchTimer& src) : shared_timer(src.shared_timer) {}
      virtual ~SearchTimer();
      void setTimeAssign(const TimeAssigned& a) {
	SCOPED_LOCK(lk,shared_timer->mutex);
	shared_timer->assigned = a;
      }
      void setStartTime(time_point start) {
	SCOPED_LOCK(lk,shared_timer->mutex);
	shared_timer->start_time = start;
	shared_timer->next_node_count = 0;
	shared_timer->nps = 0.0;
	shared_timer->last_tested = start;
	shared_timer->stop_all = false;
      }
      void setStable(bool new_stable) { shared_timer->stable = new_stable; }
      bool isStableNow() const { return shared_timer->stable; }
      bool hasSchedule() const {
	SCOPED_LOCK(lk,shared_timer->mutex);
	return shared_timer->assigned.standard < milliseconds::max();
      }
      const TimeAssigned& timeAssigned() const
      {
	SCOPED_LOCK(lk,shared_timer->mutex);
	return shared_timer->assigned;
      }
      const time_point startTime() const {
	SCOPED_LOCK(lk,shared_timer->mutex);
	return shared_timer->start_time;
      }
      double elapsed(time_point now) const 
      {
	return toSeconds(now - shared_timer->start_time);
      }
      double elapsed() const { return elapsed(clock::now()); }

      void setNextIterationCoefficient(double new_value) {
	SCOPED_LOCK(lk,shared_timer->mutex);
	shared_timer->next_iteration_coefficient = new_value;
      }
      void setNodeCountHardLimit(uint64_t new_value) {
	SCOPED_LOCK(lk,shared_timer->mutex);
	shared_timer->node_count_hard_limit = new_value;
      }
      double nextIterationCoefficient() const {
	SCOPED_LOCK(lk,shared_timer->mutex);
	return shared_timer->next_iteration_coefficient;
      }

      bool stopping() const { return shared_timer->stop_all; }
      void stopNow() 
      {
	shared_timer->stop_reason = SearchTimerCommon::StopByOutside;
	shared_timer->stop_all = true; 
      }
      SearchTimerCommon::StopReason stopReason() { return shared_timer->stop_reason; }
      void throwIfNoMoreTime(uint64_t node_count)
      {
	SearchTimerCommon& shared = *shared_timer;
	if (! shared.stop_all) {
	  uint64_t next_node_count;
	  {
#ifdef OSL_USE_RACE_DETECTOR
	    SCOPED_LOCK(lk,shared_timer->mutex);
#endif
	    next_node_count = shared.next_node_count;
	  }
	  if (next_node_count > node_count || ! hasSchedule())
	    return;
	}
	testAndUpdateNextTimeTest(node_count);
      }
      int nodeAffordable() const
      {
	const time_point now = clock::now();
#ifdef OSL_USE_RACE_DETECTOR
	SCOPED_LOCK(lk,shared_timer->mutex);
#endif
	const double nps = shared_timer->nps;
	const double left 
	  = toSeconds(shared_timer->start_time + shared_timer->assigned.max - now);
	return std::max(0, static_cast<int>(nps * left));
      }
      /** メモリとノード数の関係を調整.　探索中は利用不可．*/
      static void adjustMemoryUseLimit(double scale=0.9);
      void addMonitor(const std::shared_ptr<SearchMonitor>&);
      bool hasMonitor() const 
      { 
	return ! shared_timer->monitors.empty();
      }
      const std::vector<std::shared_ptr<SearchMonitor> >& monitors() const
      {
	return shared_timer->monitors;
      }
      int lastMemoryUseRatio1000() const 
      {
	return shared_timer->last_memory_use1000;
      }
    private:
      void testAndUpdateNextTimeTest(uint64_t node_count);
      void throwStop();
    };
  } // namespace search
} // namespace osl


#endif /* OSL_SEARCHTIMER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
