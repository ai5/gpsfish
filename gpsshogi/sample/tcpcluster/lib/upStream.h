/* upStream.h
 */
#ifndef GPSSHOGI_UPSTREAM_H
#define GPSSHOGI_UPSTREAM_H
#include "usiStatus.h"

#include <boost/asio/io_service.hpp>
#include <mutex>
#include <condition_variable>
namespace gpsshogi
{
  class Coordinator;
  class UpStream
  {
  protected:
    Coordinator *coordinator;
    boost::asio::io_service *io;
    volatile UsiStatus current_status;
  public:
    UpStream();
    virtual ~UpStream();
    void bind(Coordinator &coordinator, boost::asio::io_service &io);

    virtual void start()=0;
    virtual void outputSearchProgress(int position_id, const std::string& msg)=0;
    virtual void outputSearchResult(int position_id, const std::string& msg)=0;

    UsiStatus status() const
    {
      return current_status;
    }

    mutable std::mutex mutex;
    std::condition_variable condition;
  };
}

#endif /* GPSSHOGI_UPSTREAM_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
