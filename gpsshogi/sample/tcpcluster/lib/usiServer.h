/* usiServer.h
 */
#ifndef GPSHSOGI_USISERVER_H
#define GPSHSOGI_USISERVER_H
#include "upStream.h"
#include <boost/asio/io_service.hpp>
#include <mutex>
#include <condition_variable>
#include <vector>
namespace gpsshogi
{
  class Coordinator;

  class UsiServer : public UpStream
  {
  private:
    std::ostream& os;
    std::istream& is;
    std::vector<std::string> positions;
    std::vector<std::pair<int,bool> > go_history;
    std::vector<int> stop_history;
    std::vector<std::string> bestmove_history;
    bool verbose;
  public:
    UsiServer();
    ~UsiServer();
    void start();
    void watchInput();

    const std::string position(int position_id) const;
    void outputSearchProgress(int position_id, const std::string& msg);
    void outputSearchResult(int position_id, const std::string& msg);
  };

}

#endif /* GPSSHOGI_USISERVER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
