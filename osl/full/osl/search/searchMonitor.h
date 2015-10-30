/* searchMonitor.h
 */
#ifndef OSL_SEARCHMONITOR_H
#define OSL_SEARCHMONITOR_H
#include "osl/basic_type.h"
namespace osl
{
  namespace search
  {
    class SearchMonitor
    {
    public:
      virtual ~SearchMonitor();
      
      virtual void newDepth(int depth);
      virtual void showPV(int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last, 
			  const bool *threatmate_first, const bool *threatmate_last);
      virtual void showFailLow(int depth, size_t node_count, double elapsed, int value, Move cur);
      virtual void rootMove(Move cur);
      virtual void rootFirstMove(Move cur);
      virtual void timeInfo(size_t node_count, double elapsed);
      virtual void hashInfo(double ratio);
      virtual void rootForcedMove(Move the_move);
      virtual void rootLossByCheckmate();
      virtual void depthFinishedNormally(int depth);
      virtual void searchFinished();
    };

    class CerrMonitor : public SearchMonitor
    {
    public:
      void showPV(int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last, 
		  const bool *threatmate_first, const bool *threatmate_last);
    };
  }
  using search::SearchMonitor;
}

#endif /* OSL_SEARCHMONITOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
