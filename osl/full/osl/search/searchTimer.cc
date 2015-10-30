/* hasTimer.cc
 */
#include "osl/search/searchTimer.h"
#include "osl/search/usiReporter.h"
#include <iostream>

osl::search::SearchTimer::~SearchTimer()
{
}

void osl::search::SearchTimer::throwStop()
{
  assert(shared_timer->stop_all);
  if (shared_timer->stop_reason == SearchTimerCommon::NoMoreMemory)
    throw NoMoreMemory();
  throw misc::NoMoreTime();
}

static uint64_t maximum_node_count = 0;
static double maximum_memory_use_ratio = 0.0;
void osl::search::
SearchTimer::testAndUpdateNextTimeTest(uint64_t node_count)
{
  SCOPED_LOCK(lk,shared_timer->mutex);
  if (shared_timer->stop_all) 
    throwStop();
  if (shared_timer->next_node_count > node_count)
    return;
  const double elapsed = this->elapsed();
  
  if (elapsed > toSeconds(shared_timer->assigned.max)
      || (shared_timer->stable && elapsed > toSeconds(shared_timer->assigned.standard))
      || node_count > shared_timer->node_count_hard_limit) {
    shared_timer->stop_reason = SearchTimerCommon::NoMoreTime;
    shared_timer->stop_all = true;
    throwStop();
  }
  time_point now = clock::now();
  shared_timer->nps = node_count / (0.1+toSeconds(now - shared_timer->start_time));
  const int period100 =
    (shared_timer->node_count_hard_limit != std::numeric_limits<uint64_t>::max())
    ? 1 : 25;
  shared_timer->next_node_count = node_count + static_cast<int>(shared_timer->nps * period100 / 100.0);
  shared_timer->last_tested = now;
  static int skip = 0;
  if (++skip % (100 / period100) == 0) {
    const double memory_use_ratio = OslConfig::memoryUseRatio();
    shared_timer->last_memory_use1000 = static_cast<int>(1000*memory_use_ratio);
    if (memory_use_ratio > 0.85) {
      if (maximum_node_count > 0) {
	if (node_count > maximum_node_count 
	    || (memory_use_ratio > maximum_memory_use_ratio
		&& node_count > 0.85*maximum_node_count)
	    || (memory_use_ratio > 0.87
		&& node_count > 0.8*maximum_node_count))
	{
	  if (memory_use_ratio > 0.87 && memory_use_ratio > maximum_memory_use_ratio) {
	    maximum_memory_use_ratio = std::min(0.88, memory_use_ratio);
	    maximum_node_count = maximum_node_count * 0.9;
	  }
	  shared_timer->stop_reason = SearchTimerCommon::NoMoreMemory;
	  shared_timer->stop_all = true;
	  std::cerr << "stop by memory full " << memory_use_ratio << " " << node_count << "\n";
	  throwStop();
	}
      }
      maximum_memory_use_ratio = std::max(memory_use_ratio, maximum_memory_use_ratio);
    } else if (memory_use_ratio < 0.82) {
      maximum_node_count = std::max(maximum_node_count, node_count);
      maximum_memory_use_ratio = 0.82;
    }
#ifndef GPSONE
    if (elapsed > 1.0) {
      std::lock_guard<std::mutex> lk(OslConfig::lock_io);
      for (const std::shared_ptr<SearchMonitor>& monitor:
		    this->monitors()) {
	monitor->timeInfo(node_count, elapsed);
	monitor->hashInfo(memory_use_ratio);
      }
    }
#endif
  }
}

void osl::search::SearchTimer::adjustMemoryUseLimit(double scale)
{
  maximum_node_count *= scale;
}

void osl::search::SearchTimer::
addMonitor(const std::shared_ptr<SearchMonitor>& monitor)
{
  shared_timer->monitors.push_back(monitor);
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
