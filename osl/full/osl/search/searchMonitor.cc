/* searchMonitor.cc
 */
#include "osl/search/searchMonitor.h"
#include "osl/csa.h"
#include <iostream>
#include <iomanip>
osl::search::SearchMonitor::
~SearchMonitor()
{
}

void osl::search::SearchMonitor::newDepth(int)
{
}

void osl::search::SearchMonitor::showPV(int, size_t, double, int, Move, const Move *, const Move *, const bool*, const bool*)
{
}

void osl::search::SearchMonitor::showFailLow(int, size_t, double, int, Move)
{
}

void osl::search::SearchMonitor::rootMove(Move)
{
}

void osl::search::SearchMonitor::rootFirstMove(Move cur)
{
  rootMove(cur);
}

void osl::search::SearchMonitor::timeInfo(size_t, double)
{
}

void osl::search::SearchMonitor::hashInfo(double)
{
}

void osl::search::SearchMonitor::rootForcedMove(Move)
{
}

void osl::search::SearchMonitor::rootLossByCheckmate()
{
}

void osl::search::SearchMonitor::depthFinishedNormally(int)
{
}

void osl::search::SearchMonitor::searchFinished()
{
}

//
void osl::search::CerrMonitor::
showPV(int depth, size_t node_count, double elapsed, int value, Move cur,
       const Move *first, const Move *last, 
       const bool */*threatmate_first*/, const bool */*threatmate_last*/)
{
  std::cerr << " " << csa::show(cur) << " "
	    << std::setw(6) << value
	    << " " << std::setw(2) << last -first
	    << "/" << std::setw(2) << depth << " ";
  for (int i=0; i<last-first; ++i) {
    std::cerr << csa::show(first[i]);
  }
  std::cerr << " " << elapsed << 's'
	    << " nps=" << node_count/elapsed << std::endl;
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
