/* usiReporter.h
 */
#ifndef OSL_USIREPORTER_H
#define OSL_USIREPORTER_H
#include "osl/search/searchMonitor.h"
#include "osl/misc/milliSeconds.h"
#include <boost/asio/ip/udp.hpp>
#include <iosfwd>

namespace osl
{
  namespace search
  {
    struct UsiReporter
    {
      static void newDepth(std::ostream& os, int depth);
      static void showPV(std::ostream& os, int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last, bool ignore_silent=false);
      static void showPVExtended(std::ostream& os, int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last,
				 const bool *threatmate_first, const bool *threatmate_last);
      static void rootMove(std::ostream& os, Move cur, bool allow_frequent_display=false);
      static void timeInfo(std::ostream& os, size_t node_count, double elapsed);
      static void hashInfo(std::ostream& os, double ratio);
    };

    class UsiMonitor : public SearchMonitor
    {
      Move last_root_move;
      std::string deferred;
      double silent_period;
      bool extended;
      time_point depth0;
      std::ostream& os;
      boost::asio::ip::udp::socket *udp_socket;
      boost::asio::ip::udp::endpoint *udp_endpoint;
      std::string client_id;
    public:
      UsiMonitor(bool extended, std::ostream& os, double silent=0.5);
      ~UsiMonitor();
      void setUdpLogging(std::string& udp_client_id,
			 boost::asio::ip::udp::socket *,
			 boost::asio::ip::udp::endpoint *);
      void newDepth(int depth);
      void showPV(int depth, size_t node_count, double elapsed, int value, Move cur, const Move *first, const Move *last,
		  const bool *threatmate_first, const bool *threatmate_last);
      void showFailLow(int depth, size_t node_count, double elapsed,
		       int value, Move cur);
      void rootMove(Move cur);
      void rootFirstMove(Move cur);
      void timeInfo(size_t node_count, double elapsed);
      void hashInfo(double ratio);
      void rootForcedMove(Move the_move);
      void rootLossByCheckmate();
      void searchFinished();
    private:
      void showDeferred(bool forced=false);
    };
  }
  using search::UsiMonitor;
}


#endif /* OSL_USIREPORTER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
